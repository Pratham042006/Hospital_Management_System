#pragma once

#include "hms/patient.hpp"

#include <cstddef>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace hms {

class PatientRepository {
public:
    explicit PatientRepository(
        const std::optional<std::string>& journalPath = std::nullopt);

    Patient registerPatient(const PatientDraft& draft);
    Patient updatePatient(const std::string& patientId, const PatientUpdate& update);
    std::optional<Patient> findById(const std::string& patientId) const;
    std::vector<Patient> findByName(const std::string& name) const;
    std::vector<Patient> findByContact(const std::string& contact) const;
    std::size_t size() const noexcept;

private:
    std::vector<Patient> patients_;
    std::unordered_map<std::string, std::size_t> idIndex_;
    std::unordered_map<std::string, std::string> phoneIndex_;
    std::unordered_map<std::string, std::string> emailIndex_;
    std::size_t nextPatientNumber_{1};
    std::optional<std::string> journalPath_;

    std::string generatePatientId();
    std::size_t indexForId(const std::string& patientId) const;
    void ensureUniqueContacts(
        const Patient& patient,
        const std::optional<std::string>& ignoredPatientId = std::nullopt) const;
    void addToIndexes(const Patient& patient, std::size_t index);
    void removeContactIndexes(const Patient& patient);
    void appendToJournal(const Patient& patient) const;
    void loadJournal();
};

}  // namespace hms

