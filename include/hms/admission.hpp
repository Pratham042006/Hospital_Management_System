#pragma once

#include <optional>
#include <string>

namespace hms {

enum class AdmissionStatus {
    Admitted,
    Discharged
};

inline std::string toString(AdmissionStatus status) {
    switch (status) {
        case AdmissionStatus::Admitted:   return "Admitted";
        case AdmissionStatus::Discharged: return "Discharged";
    }
    return "Unknown";
}

struct Admission {
    std::string id;
    std::string patientId;
    std::string doctorId;
    std::string roomId;
    std::string admissionDate;
    std::string reason;
    AdmissionStatus status{AdmissionStatus::Admitted};
    std::optional<std::string> dischargeDate;
    std::optional<std::string> dischargeSummary;
};

struct AdmissionDraft {
    std::string patientId;
    std::string doctorId;
    std::string roomId;
    std::string admissionDate;
    std::string reason;
};

}  // namespace hms
