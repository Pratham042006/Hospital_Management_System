#pragma once

#include "hms/admission_repository.hpp"
#include "hms/auth.hpp"
#include "hms/appointment_repository.hpp"
#include "hms/doctor_repository.hpp"
#include "hms/patient_repository.hpp"
#include "hms/room_repository.hpp"

#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

namespace hms {

class AuthorizationError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

struct PatientSearchResult {
    std::string id;
    std::string fullName;
    std::optional<std::string> dateOfBirth;
    std::optional<std::string> gender;
    std::optional<std::string> phone;
    std::optional<std::string> email;
    std::optional<std::string> address;
    std::optional<std::string> emergencyContactName;
    std::optional<std::string> emergencyContactPhone;
};

class HospitalSystem {
public:
    explicit HospitalSystem(
        const std::vector<InitialAccount>& initialAccounts,
        const std::optional<std::string>& patientJournalPath = std::nullopt);

    std::optional<Session> login(const std::string& username,
                                 const std::string& password) const;
    std::vector<std::string> menuFor(const Session& session) const;
    bool addUser(const Session& session,
                 const std::string& username,
                 const std::string& password,
                 Role role);
    Patient registerPatient(const Session& session, const PatientDraft& draft);
    Patient updatePatient(const Session& session,
                          const std::string& patientId,
                          const PatientUpdate& update);
    std::optional<PatientSearchResult> searchPatientById(
        const Session& session,
        const std::string& patientId) const;
    std::vector<PatientSearchResult> searchPatientsByName(
        const Session& session,
        const std::string& name) const;
    std::vector<PatientSearchResult> searchPatientsByContact(
        const Session& session,
        const std::string& contact) const;
    std::size_t patientCount() const noexcept;

    Doctor addDoctor(const Session& session, const DoctorDraft& draft);
    Doctor updateDoctor(const Session& session,
                        const std::string& doctorId,
                        const DoctorUpdate& update);
    std::optional<Doctor> findDoctorById(const Session& session,
                                         const std::string& doctorId) const;
    std::vector<Doctor> searchDoctors(const Session& session,
                                      const std::string& query) const;
    std::vector<Doctor> listDoctors(const Session& session) const;

    Appointment scheduleAppointment(const Session& session,
                                    const AppointmentDraft& draft);
    Appointment rescheduleAppointment(const Session& session,
                                      const std::string& appointmentId,
                                      const AppointmentUpdate& update);
    Appointment cancelAppointment(const Session& session,
                                  const std::string& appointmentId);
    std::optional<Appointment> findAppointmentById(
        const Session& session,
        const std::string& appointmentId) const;
    std::vector<Appointment> listAppointments(const Session& session) const;

    // FR-09: Room and Ward Management
    Room addRoom(const Session& session, const RoomDraft& draft);
    std::optional<Room> findRoomById(const Session& session, const std::string& roomId) const;
    std::vector<Room> listAllRooms(const Session& session) const;
    std::vector<Room> listAvailableRooms(const Session& session) const;

    // FR-08 & FR-11: Admission and Discharge Management
    Admission admitPatient(const Session& session, const AdmissionDraft& draft);
    Admission dischargePatient(const Session& session,
                               const std::string& admissionId,
                               const std::string& dischargeDate,
                               const std::string& dischargeSummary);
    std::optional<Admission> findAdmissionById(const Session& session,
                                               const std::string& admissionId) const;
    std::vector<Admission> listActiveAdmissions(const Session& session) const;
    std::vector<Admission> listAllAdmissions(const Session& session) const;

private:
    AuthService auth_;
    PatientRepository patients_;
    DoctorRepository doctors_;
    AppointmentRepository appointments_;
    RoomRepository rooms_;
    AdmissionRepository admissions_;

    void require(const Session& session, Permission permission) const;
};

}  // namespace hms
