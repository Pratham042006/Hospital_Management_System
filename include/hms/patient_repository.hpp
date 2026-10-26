#pragma once

#include "hms/patient.hpp"

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

namespace hms {

class PatientRepository {
public:
    Patient registerPatient(const PatientDraft& draft);
    Patient updatePatient(const std::string& patientId, const PatientUpdate& update);
    std::size_t size() const noexcept;

private:
    std::vector<Patient> patients_;
    std::size_t nextPatientNumber_{1};

    static std::string trim(const std::string& value);
    static std::string normalizeText(const std::string& value);
    static std::string normalizePhone(const std::string& value);
    static void validate(const Patient& patient);

    std::string generatePatientId();
    std::size_t indexForId(const std::string& patientId) const;
    void ensureUniqueContacts(
        const Patient& patient,
        const std::optional<std::string>& ignoredPatientId = std::nullopt) const;
};

}  // namespace hms
