#pragma once

#include "hms/auth.hpp"
#include "hms/patient_repository.hpp"
#include "hms/clinical.hpp"

#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

namespace hms {

class AuthorizationError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

class HospitalSystem {
public:
    std::optional<Session> login(const std::string& username,
                                 const std::string& password) const;
    std::vector<std::string> menuFor(const Session& session) const;
    Patient registerPatient(const Session& session, const PatientDraft& draft);
    Patient updatePatient(const Session& session,
                          const std::string& patientId,
                          const PatientUpdate& update);
    std::optional<Patient> searchPatientById(const Session& session,
                                             const std::string& patientId) const;
    std::vector<Patient> searchPatientsByName(const Session& session,
                                              const std::string& name) const;
    std::vector<Patient> searchPatientsByContact(const Session& session,
                                                 const std::string& contact) const;
    std::size_t patientCount() const noexcept;

    Doctor addDoctor(const Session& session, const DoctorDraft& draft);
    std::vector<Doctor> listDoctors(const Session& session) const;
    Appointment scheduleAppointment(const Session& session, const AppointmentDraft& draft);
    Appointment modifyAppointment(const Session& session, const std::string& appointmentId,
                                  const AppointmentDraft& changes);
    bool cancelAppointment(const Session& session, const std::string& appointmentId);
    std::vector<Appointment> listAppointments(const Session& session) const;

private:
    AuthService auth_;
    PatientRepository patients_;
    std::vector<Doctor> doctors_;
    std::vector<Appointment> appointments_;
    std::size_t nextDoctorNumber_{1};
    std::size_t nextAppointmentNumber_{1};

    void require(const Session& session, Permission permission) const;
};

}  // namespace hms
