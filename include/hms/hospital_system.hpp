#pragma once

#include "hms/auth.hpp"
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

private:
    AuthService auth_;
    PatientRepository patients_;

    void require(const Session& session, Permission permission) const;
};

}  // namespace hms
