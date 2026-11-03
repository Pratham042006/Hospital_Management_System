#include "hms/medical_record_repository.hpp"

#include <chrono>
#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace hms {

bool MedicalRecordRepository::isValidDate(const std::string& date) {
    if (date.size() != 10 || date[4] != '-' || date[7] != '-') {
        return false;
    }
    try {
        int year = std::stoi(date.substr(0, 4));
        int month = std::stoi(date.substr(5, 2));
        int day = std::stoi(date.substr(8, 2));
        if (year < 1900 || year > 2100 || month < 1 || month > 12 || day < 1 || day > 31) {
            return false;
        }
    } catch (...) {
        return false;
    }
    return true;
}

std::string MedicalRecordRepository::generateId() {
    std::ostringstream stream;
    stream << "MR-" << std::setw(5) << std::setfill('0') << nextId_++;
    return stream.str();
}

MedicalRecord MedicalRecordRepository::addRecord(const MedicalRecordDraft& draft) {
    if (draft.patientId.empty()) {
        throw std::invalid_argument("Patient ID is required for a medical record.");
    }
    if (draft.doctorId.empty()) {
        throw std::invalid_argument("Doctor ID is required for a medical record.");
    }
    if (!isValidDate(draft.visitDate)) {
        throw std::invalid_argument("Medical record visit date must be in YYYY-MM-DD format.");
    }
    if (draft.diagnosis.empty()) {
        throw std::invalid_argument("Diagnosis cannot be empty.");
    }

    MedicalRecord record;
    record.id = generateId();
    record.patientId = draft.patientId;
    record.doctorId = draft.doctorId;
    record.visitDate = draft.visitDate;
    record.symptoms = draft.symptoms;
    record.diagnosis = draft.diagnosis;
    record.treatmentNotes = draft.treatmentNotes;
    record.prescriptions = draft.prescriptions;
    record.createdAt = draft.visitDate;

    records_.push_back(record);
    return record;
}

std::optional<MedicalRecord> MedicalRecordRepository::findById(const std::string& recordId) const {
    for (const auto& record : records_) {
        if (record.id == recordId) {
            return record;
        }
    }
    return std::nullopt;
}

std::vector<MedicalRecord> MedicalRecordRepository::findByPatientId(const std::string& patientId) const {
    std::vector<MedicalRecord> matches;
    for (const auto& record : records_) {
        if (record.patientId == patientId) {
            matches.push_back(record);
        }
    }
    return matches;
}

std::vector<MedicalRecord> MedicalRecordRepository::listAll() const {
    return records_;
}

std::size_t MedicalRecordRepository::size() const noexcept {
    return records_.size();
}

}  // namespace hms
