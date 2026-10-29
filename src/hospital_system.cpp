#include "hms/hospital_system.hpp"

#include <algorithm>
#include <cctype>
#include <iomanip>
#include <sstream>

namespace hms {
namespace {
std::string trim(const std::string& value) {
    const auto first = value.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return {};
    return value.substr(first, value.find_last_not_of(" \t\r\n") - first + 1);
}
bool validDate(const std::string& date) {
    if (date.size() != 10 || date[4] != '-' || date[7] != '-') return false;
    for (std::size_t i = 0; i < date.size(); ++i) {
        if (i != 4 && i != 7 && !std::isdigit(static_cast<unsigned char>(date[i]))) return false;
    }
    const int month = std::stoi(date.substr(5, 2));
    const int day = std::stoi(date.substr(8, 2));
    return month >= 1 && month <= 12 && day >= 1 && day <= 31;
}
bool validTime(const std::string& time) {
    if (time.size() != 5 || time[2] != ':') return false;
    if (!std::isdigit(static_cast<unsigned char>(time[0])) ||
        !std::isdigit(static_cast<unsigned char>(time[1])) ||
        !std::isdigit(static_cast<unsigned char>(time[3])) ||
        !std::isdigit(static_cast<unsigned char>(time[4]))) return false;
    const int hour = std::stoi(time.substr(0, 2));
    const int minute = std::stoi(time.substr(3, 2));
    return hour >= 0 && hour <= 23 && minute >= 0 && minute <= 59;
}
} // namespace

std::string toString(AppointmentStatus status) {
    switch (status) {
        case AppointmentStatus::Scheduled: return "Scheduled";
        case AppointmentStatus::Cancelled: return "Cancelled";
        case AppointmentStatus::Completed: return "Completed";
    }
    return "Unknown";
}

std::optional<Session> HospitalSystem::login(const std::string& username,
                                             const std::string& password) const {
    return auth_.authenticate(username, password);
}
std::vector<std::string> HospitalSystem::menuFor(const Session& session) const {
    return auth_.availableActions(session);
}
Patient HospitalSystem::registerPatient(const Session& session, const PatientDraft& draft) {
    require(session, Permission::RegisterPatient);
    return patients_.registerPatient(draft);
}
Patient HospitalSystem::updatePatient(const Session& session, const std::string& patientId,
                                      const PatientUpdate& update) {
    require(session, Permission::UpdatePatient);
    return patients_.updatePatient(patientId, update);
}
std::optional<Patient> HospitalSystem::searchPatientById(const Session& session,
                                                        const std::string& patientId) const {
    require(session, Permission::SearchPatient);
    return patients_.findById(patientId);
}
std::vector<Patient> HospitalSystem::searchPatientsByName(const Session& session,
                                                          const std::string& name) const {
    require(session, Permission::SearchPatient);
    return patients_.findByName(name);
}
std::vector<Patient> HospitalSystem::searchPatientsByContact(const Session& session,
                                                             const std::string& contact) const {
    require(session, Permission::SearchPatient);
    return patients_.findByContact(contact);
}
std::size_t HospitalSystem::patientCount() const noexcept { return patients_.size(); }

Doctor HospitalSystem::addDoctor(const Session& session, const DoctorDraft& draft) {
    require(session, Permission::ManageDoctors);
    Doctor doctor{"DOC" + std::to_string(nextDoctorNumber_), trim(draft.fullName),
                  trim(draft.specialty), trim(draft.phone), trim(draft.email)};
    if (doctor.fullName.empty() || doctor.specialty.empty() || doctor.phone.empty())
        throw std::invalid_argument("Doctor name, specialty, and phone are required.");
    const auto duplicate = std::find_if(doctors_.begin(), doctors_.end(), [&](const Doctor& item) {
        return item.phone == doctor.phone || (!doctor.email.empty() && item.email == doctor.email);
    });
    if (duplicate != doctors_.end()) throw std::invalid_argument("A doctor with this phone or email already exists.");
    ++nextDoctorNumber_;
    doctors_.push_back(doctor);
    return doctor;
}
std::vector<Doctor> HospitalSystem::listDoctors(const Session& session) const {
    require(session, Permission::ManageDoctors);
    return doctors_;
}
Appointment HospitalSystem::scheduleAppointment(const Session& session, const AppointmentDraft& draft) {
    require(session, Permission::ManageAppointments);
    if (!patients_.findById(trim(draft.patientId))) throw std::invalid_argument("Patient ID does not exist.");
    const auto doctor = std::find_if(doctors_.begin(), doctors_.end(), [&](const Doctor& item) {
        return item.id == trim(draft.doctorId);
    });
    if (doctor == doctors_.end()) throw std::invalid_argument("Doctor ID does not exist.");
    if (!validDate(draft.date) || !validTime(draft.time) || trim(draft.reason).empty())
        throw std::invalid_argument("Use date YYYY-MM-DD, time HH:MM (24-hour), and provide a reason.");
    const auto conflict = std::find_if(appointments_.begin(), appointments_.end(), [&](const Appointment& item) {
        return item.status == AppointmentStatus::Scheduled && item.date == draft.date && item.time == draft.time &&
               (item.doctorId == draft.doctorId || item.patientId == draft.patientId);
    });
    if (conflict != appointments_.end()) throw std::invalid_argument("Doctor or patient already has an appointment at that time.");
    Appointment appointment{"APT" + std::to_string(nextAppointmentNumber_), trim(draft.patientId),
        trim(draft.doctorId), draft.date, draft.time, trim(draft.reason), AppointmentStatus::Scheduled};
    ++nextAppointmentNumber_;
    appointments_.push_back(appointment);
    return appointment;
}
Appointment HospitalSystem::modifyAppointment(const Session& session, const std::string& appointmentId,
                                               const AppointmentDraft& changes) {
    require(session, Permission::ManageAppointments);
    auto found = std::find_if(appointments_.begin(), appointments_.end(), [&](const Appointment& item) {
        return item.id == appointmentId;
    });
    if (found == appointments_.end()) throw std::invalid_argument("Appointment ID does not exist.");
    if (found->status != AppointmentStatus::Scheduled) throw std::invalid_argument("Only scheduled appointments can be modified.");
    AppointmentDraft updated = changes;
    if (trim(updated.patientId).empty()) updated.patientId = found->patientId;
    if (trim(updated.doctorId).empty()) updated.doctorId = found->doctorId;
    if (trim(updated.date).empty()) updated.date = found->date;
    if (trim(updated.time).empty()) updated.time = found->time;
    if (trim(updated.reason).empty()) updated.reason = found->reason;
    if (!patients_.findById(trim(updated.patientId))) throw std::invalid_argument("Patient ID does not exist.");
    const auto doctor = std::find_if(doctors_.begin(), doctors_.end(), [&](const Doctor& item) { return item.id == updated.doctorId; });
    if (doctor == doctors_.end()) throw std::invalid_argument("Doctor ID does not exist.");
    if (!validDate(updated.date) || !validTime(updated.time)) throw std::invalid_argument("Invalid date or time format.");
    const auto conflict = std::find_if(appointments_.begin(), appointments_.end(), [&](const Appointment& item) {
        return item.id != appointmentId && item.status == AppointmentStatus::Scheduled && item.date == updated.date &&
               item.time == updated.time && (item.doctorId == updated.doctorId || item.patientId == updated.patientId);
    });
    if (conflict != appointments_.end()) throw std::invalid_argument("Doctor or patient already has an appointment at that time.");
    found->patientId = trim(updated.patientId); found->doctorId = trim(updated.doctorId);
    found->date = updated.date; found->time = updated.time; found->reason = trim(updated.reason);
    return *found;
}
bool HospitalSystem::cancelAppointment(const Session& session, const std::string& appointmentId) {
    require(session, Permission::ManageAppointments);
    auto found = std::find_if(appointments_.begin(), appointments_.end(), [&](const Appointment& item) { return item.id == appointmentId; });
    if (found == appointments_.end()) return false;
    if (found->status != AppointmentStatus::Scheduled) return false;
    found->status = AppointmentStatus::Cancelled;
    return true;
}
std::vector<Appointment> HospitalSystem::listAppointments(const Session& session) const {
    require(session, Permission::ManageAppointments);
    return appointments_;
}
void HospitalSystem::require(const Session& session, Permission permission) const {
    if (!auth_.isAllowed(session, permission))
        throw AuthorizationError("The " + toString(session.role) + " role is not authorized for this operation.");
}
} // namespace hms
