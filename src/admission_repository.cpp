#include "hms/admission_repository.hpp"

#include <algorithm>
#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace hms {

namespace {

std::string formatAdmissionId(int number) {
    std::ostringstream stream;
    stream << "ADM-" << std::setw(5) << std::setfill('0') << number;
    return stream.str();
}

}  // namespace

Admission AdmissionRepository::admitPatient(const AdmissionDraft& draft) {
    if (draft.patientId.empty()) {
        throw std::invalid_argument("Patient ID cannot be empty for admission.");
    }
    if (draft.doctorId.empty()) {
        throw std::invalid_argument("Attending doctor ID cannot be empty for admission.");
    }
    if (draft.roomId.empty()) {
        throw std::invalid_argument("Room ID cannot be empty for admission.");
    }
    if (draft.admissionDate.empty()) {
        throw std::invalid_argument("Admission date cannot be empty.");
    }
    if (draft.reason.empty()) {
        throw std::invalid_argument("Reason for admission cannot be empty.");
    }

    // Check if patient is already actively admitted
    if (findActiveAdmissionByPatient(draft.patientId).has_value()) {
        throw std::invalid_argument("Patient " + draft.patientId + " is already currently admitted.");
    }

    const std::string admissionId = formatAdmissionId(nextId_++);
    Admission admission{
        admissionId,
        draft.patientId,
        draft.doctorId,
        draft.roomId,
        draft.admissionDate,
        draft.reason,
        AdmissionStatus::Admitted,
        std::nullopt,
        std::nullopt
    };

    admissions_.emplace(admissionId, admission);
    return admission;
}

Admission AdmissionRepository::dischargePatient(const std::string& admissionId,
                                                const std::string& dischargeDate,
                                                const std::string& dischargeSummary) {
    auto it = admissions_.find(admissionId);
    if (it == admissions_.end()) {
        throw std::invalid_argument("Admission record " + admissionId + " not found.");
    }
    if (it->second.status == AdmissionStatus::Discharged) {
        throw std::invalid_argument("Patient for admission " + admissionId + " is already discharged.");
    }
    if (dischargeDate.empty()) {
        throw std::invalid_argument("Discharge date cannot be empty.");
    }
    if (dischargeSummary.empty()) {
        throw std::invalid_argument("Discharge summary cannot be empty.");
    }

    it->second.status = AdmissionStatus::Discharged;
    it->second.dischargeDate = dischargeDate;
    it->second.dischargeSummary = dischargeSummary;

    return it->second;
}

std::optional<Admission> AdmissionRepository::findAdmissionById(const std::string& admissionId) const {
    const auto it = admissions_.find(admissionId);
    if (it == admissions_.end()) {
        return std::nullopt;
    }
    return it->second;
}

std::optional<Admission> AdmissionRepository::findActiveAdmissionByPatient(const std::string& patientId) const {
    for (const auto& [_, admission] : admissions_) {
        if (admission.patientId == patientId && admission.status == AdmissionStatus::Admitted) {
            return admission;
        }
    }
    return std::nullopt;
}

std::vector<Admission> AdmissionRepository::listActiveAdmissions() const {
    std::vector<Admission> result;
    for (const auto& [_, admission] : admissions_) {
        if (admission.status == AdmissionStatus::Admitted) {
            result.push_back(admission);
        }
    }
    std::sort(result.begin(), result.end(), [](const Admission& a, const Admission& b) {
        return a.id < b.id;
    });
    return result;
}

std::vector<Admission> AdmissionRepository::listAllAdmissions() const {
    std::vector<Admission> result;
    result.reserve(admissions_.size());
    for (const auto& [_, admission] : admissions_) {
        result.push_back(admission);
    }
    std::sort(result.begin(), result.end(), [](const Admission& a, const Admission& b) {
        return a.id < b.id;
    });
    return result;
}

std::vector<Admission> AdmissionRepository::listAdmissionsForDoctor(const std::string& doctorId) const {
    std::vector<Admission> result;
    for (const auto& [_, admission] : admissions_) {
        if (admission.doctorId == doctorId) {
            result.push_back(admission);
        }
    }
    std::sort(result.begin(), result.end(), [](const Admission& a, const Admission& b) {
        return a.id < b.id;
    });
    return result;
}

std::size_t AdmissionRepository::totalAdmissions() const {
    return admissions_.size();
}

}  // namespace hms
