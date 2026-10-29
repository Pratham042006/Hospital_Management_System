#pragma once

#include "hms/auth.hpp"
#include "hms/appointment_repository.hpp"
#include "hms/doctor_repository.hpp"
#include "hms/patient_repository.hpp"

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
    explicit HospitalSystem(const std::vector<InitialAccount>& initialAccounts);

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
    std::optional<Appointment> findAppointmentById(
        const Session& session,
        const std::string& appointmentId) const;
    std::vector<Appointment> listAppointments(const Session& session) const;

private:
    AuthService auth_;
    PatientRepository patients_;
    DoctorRepository doctors_;
    AppointmentRepository appointments_;

    void require(const Session& session, Permission permission) const;
};

}  // namespace hms
