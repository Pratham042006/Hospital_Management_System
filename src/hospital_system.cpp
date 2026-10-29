#include "hms/hospital_system.hpp"

namespace hms {

namespace {

PatientSearchResult projectPatient(const Patient& patient, Role role) {
    PatientSearchResult result;
    result.id = patient.id;
    result.fullName = patient.fullName;

    switch (role) {
        case Role::Administrator:
            result.dateOfBirth = patient.dateOfBirth;
            result.gender = patient.gender;
            result.phone = patient.phone;
            result.email = patient.email;
            result.address = patient.address;
            result.emergencyContactName = patient.emergencyContactName;
            result.emergencyContactPhone = patient.emergencyContactPhone;
            break;
        case Role::Receptionist:
            result.dateOfBirth = patient.dateOfBirth;
            result.phone = patient.phone;
            result.email = patient.email;
            result.address = patient.address;
            break;
        case Role::Doctor:
            result.dateOfBirth = patient.dateOfBirth;
            result.gender = patient.gender;
            result.emergencyContactName = patient.emergencyContactName;
            result.emergencyContactPhone = patient.emergencyContactPhone;
            break;
        case Role::BillingStaff:
            break;
    }
    return result;
}

}  // namespace

HospitalSystem::HospitalSystem(const std::vector<InitialAccount>& initialAccounts)
    : auth_(initialAccounts) {}

std::optional<Session> HospitalSystem::login(const std::string& username,
                                             const std::string& password) const {
    return auth_.authenticate(username, password);
}

std::vector<std::string> HospitalSystem::menuFor(const Session& session) const {
    return auth_.availableActions(session);
}

bool HospitalSystem::addUser(const Session& session,
                             const std::string& username,
                             const std::string& password,
                             Role role) {
    require(session, Permission::ManageUsers);
    return auth_.addUser(username, password, role);
}

Patient HospitalSystem::registerPatient(const Session& session, const PatientDraft& draft) {
    require(session, Permission::RegisterPatient);
    return patients_.registerPatient(draft);
}

Patient HospitalSystem::updatePatient(const Session& session,
                                      const std::string& patientId,
                                      const PatientUpdate& update) {
    require(session, Permission::UpdatePatient);
    return patients_.updatePatient(patientId, update);
}

std::optional<PatientSearchResult> HospitalSystem::searchPatientById(
    const Session& session,
    const std::string& patientId) const {
    require(session, Permission::SearchPatient);
    const auto patient = patients_.findById(patientId);
    if (!patient) return std::nullopt;
    return projectPatient(*patient, session.role);
}

std::vector<PatientSearchResult> HospitalSystem::searchPatientsByName(
    const Session& session,
    const std::string& name) const {
    require(session, Permission::SearchPatient);
    std::vector<PatientSearchResult> results;
    for (const auto& patient : patients_.findByName(name)) {
        results.push_back(projectPatient(patient, session.role));
    }
    return results;
}

std::vector<PatientSearchResult> HospitalSystem::searchPatientsByContact(
    const Session& session,
    const std::string& contact) const {
    require(session, Permission::SearchPatient);
    std::vector<PatientSearchResult> results;
    for (const auto& patient : patients_.findByContact(contact)) {
        results.push_back(projectPatient(patient, session.role));
    }
    return results;
}

std::size_t HospitalSystem::patientCount() const noexcept {
    return patients_.size();
}

Doctor HospitalSystem::addDoctor(const Session& session, const DoctorDraft& draft) {
    require(session, Permission::ManageDoctors);
    return doctors_.addDoctor(draft);
}

Doctor HospitalSystem::updateDoctor(const Session& session,
                                    const std::string& doctorId,
                                    const DoctorUpdate& update) {
    require(session, Permission::ManageDoctors);
    return doctors_.updateDoctor(doctorId, update);
}

std::optional<Doctor> HospitalSystem::findDoctorById(
    const Session& session,
    const std::string& doctorId) const {
    require(session, Permission::ViewDoctors);
    return doctors_.findById(doctorId);
}

std::vector<Doctor> HospitalSystem::searchDoctors(
    const Session& session,
    const std::string& query) const {
    require(session, Permission::ViewDoctors);
    return doctors_.search(query);
}

std::vector<Doctor> HospitalSystem::listDoctors(const Session& session) const {
    require(session, Permission::ViewDoctors);
    return doctors_.list();
}

Appointment HospitalSystem::scheduleAppointment(
    const Session& session,
    const AppointmentDraft& draft) {
    require(session, Permission::ManageAppointments);
    if (!patients_.findById(draft.patientId)) {
        throw std::invalid_argument("Patient ID does not exist.");
    }
    if (!doctors_.findById(draft.doctorId)) {
        throw std::invalid_argument("Doctor ID does not exist.");
    }
    if (!doctors_.isAvailable(draft.doctorId, draft.date, draft.time)) {
        throw std::invalid_argument("The doctor is not available at the requested date and time.");
    }
    return appointments_.schedule(draft);
}

Appointment HospitalSystem::rescheduleAppointment(
    const Session& session,
    const std::string& appointmentId,
    const AppointmentUpdate& update) {
    require(session, Permission::ManageAppointments);
    const auto current = appointments_.findById(appointmentId);
    if (!current) throw std::invalid_argument("Appointment ID was not found: " + appointmentId);
    const std::string doctorId = update.doctorId ? *update.doctorId : current->doctorId;
    const std::string date = update.date ? *update.date : current->date;
    const std::string time = update.time ? *update.time : current->time;
    if (!AppointmentRepository::isValidDate(date)) {
        throw std::invalid_argument("Appointment date must be a real YYYY-MM-DD date.");
    }
    if (AppointmentRepository::isPastDate(date)) {
        throw std::invalid_argument("Appointment date cannot be in the past.");
    }
    if (!AppointmentRepository::isValidTime(time)) {
        throw std::invalid_argument("Appointment time must be a valid HH:MM time.");
    }
    if (!doctors_.findById(doctorId)) throw std::invalid_argument("Doctor ID does not exist.");
    if (!doctors_.isAvailable(doctorId, date, time)) {
        throw std::invalid_argument("The doctor is not available at the requested date and time.");
    }
    return appointments_.reschedule(appointmentId, update);
}

Appointment HospitalSystem::cancelAppointment(const Session& session,
                                              const std::string& appointmentId) {
    require(session, Permission::ManageAppointments);
    return appointments_.cancel(appointmentId);
}

std::optional<Appointment> HospitalSystem::findAppointmentById(
    const Session& session,
    const std::string& appointmentId) const {
    require(session, Permission::ViewAppointments);
    return appointments_.findById(appointmentId);
}

std::vector<Appointment> HospitalSystem::listAppointments(const Session& session) const {
    require(session, Permission::ViewAppointments);
    return appointments_.list();
}

void HospitalSystem::require(const Session& session, Permission permission) const {
    if (!auth_.isAllowed(session, permission)) {
        throw AuthorizationError("The " + toString(session.role) +
                                 " role is not authorized for this operation.");
    }
}

}  // namespace hms
