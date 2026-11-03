#pragma once

#include <string>
#include <vector>

namespace hms {

struct MedicalRecordDraft {
    std::string patientId;
    std::string doctorId;
    std::string visitDate;       // YYYY-MM-DD
    std::string symptoms;
    std::string diagnosis;
    std::string treatmentNotes;
    std::vector<std::string> prescriptions;
};

struct MedicalRecord {
    std::string id;              // e.g. "MR-00001"
    std::string patientId;
    std::string doctorId;
    std::string visitDate;
    std::string symptoms;
    std::string diagnosis;
    std::string treatmentNotes;
    std::vector<std::string> prescriptions;
    std::string createdAt;
};

}  // namespace hms
