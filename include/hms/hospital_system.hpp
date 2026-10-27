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

private:
    AuthService auth_;
    PatientRepository patients_;

    void require(const Session& session, Permission permission) const;
};

}  // namespace hms
