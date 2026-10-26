#pragma once

#include "hms/patient.hpp"

#include <cstddef>
#include <string>
#include <vector>

namespace hms {

class PatientRepository {
public:
    Patient registerPatient(const PatientDraft& draft);
    std::size_t size() const noexcept;

private:
    std::vector<Patient> patients_;
    std::size_t nextPatientNumber_{1};

    static std::string trim(const std::string& value);
    static std::string normalizeText(const std::string& value);
    static std::string normalizePhone(const std::string& value);
    static void validate(const Patient& patient);

    std::string generatePatientId();
    void ensureUniqueContacts(const Patient& patient) const;
};

}  // namespace hms
