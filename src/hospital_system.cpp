#include "hms/hospital_system.hpp"

namespace hms {

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

Patient HospitalSystem::updatePatient(const Session& session,
                                      const std::string& patientId,
                                      const PatientUpdate& update) {
    require(session, Permission::UpdatePatient);
    return patients_.updatePatient(patientId, update);
}

std::optional<Patient> HospitalSystem::searchPatientById(
    const Session& session,
    const std::string& patientId) const {
    require(session, Permission::SearchPatient);
    return patients_.findById(patientId);
}

std::vector<Patient> HospitalSystem::searchPatientsByName(
    const Session& session,
    const std::string& name) const {
    require(session, Permission::SearchPatient);
    return patients_.findByName(name);
}

std::vector<Patient> HospitalSystem::searchPatientsByContact(
    const Session& session,
    const std::string& contact) const {
    require(session, Permission::SearchPatient);
    return patients_.findByContact(contact);
}

std::size_t HospitalSystem::patientCount() const noexcept {
    return patients_.size();
}

void HospitalSystem::require(const Session& session, Permission permission) const {
    if (!auth_.isAllowed(session, permission)) {
        throw AuthorizationError("The " + toString(session.role) +
                                 " role is not authorized for this operation.");
    }
}

}  // namespace hms
