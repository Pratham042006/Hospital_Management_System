#include "hms/medical_record_repository.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace hms {

namespace {

std::string hexEncode(const std::string& value) {
    static constexpr char digits[] = "0123456789ABCDEF";
    std::string encoded;
    encoded.reserve(value.size() * 2);
    for (const unsigned char ch : value) {
        encoded.push_back(digits[ch >> 4]);
        encoded.push_back(digits[ch & 0x0F]);
    }
    return encoded;
}

int hexValue(char value) {
    if (value >= '0' && value <= '9') return value - '0';
    if (value >= 'A' && value <= 'F') return value - 'A' + 10;
    if (value >= 'a' && value <= 'f') return value - 'a' + 10;
    return -1;
}

std::string hexDecode(const std::string& value) {
    if (value.size() % 2 != 0) throw std::runtime_error("Malformed medical record journal field.");
    std::string decoded;
    decoded.reserve(value.size() / 2);
    for (std::size_t index = 0; index < value.size(); index += 2) {
        const int high = hexValue(value[index]);
        const int low = hexValue(value[index + 1]);
        if (high < 0 || low < 0) throw std::runtime_error("Malformed medical record journal field.");
        decoded.push_back(static_cast<char>((high << 4) | low));
    }
    return decoded;
}

std::vector<std::string> splitTabs(const std::string& line) {
    std::vector<std::string> fields;
    std::istringstream input(line);
    std::string field;
    while (std::getline(input, field, '\t')) fields.push_back(field);
    return fields;
}

std::size_t parseCount(const std::string& value, std::size_t lineNumber) {
    try {
        std::size_t consumed = 0;
        const auto count = std::stoull(value, &consumed);
        if (consumed != value.size() || count > 1000) throw std::runtime_error("bad count");
        return static_cast<std::size_t>(count);
    } catch (const std::exception&) {
        throw std::runtime_error("Malformed prescription count in medical record journal at line " +
                                 std::to_string(lineNumber) + '.');
    }
}

}  // namespace

MedicalRecordRepository::MedicalRecordRepository(
    const std::optional<std::string>& journalPath)
    : journalPath_(journalPath) {
    if (journalPath_) loadJournal();
}

bool MedicalRecordRepository::isValidDate(const std::string& date) {
    if (date.size() != 10 || date[4] != '-' || date[7] != '-') {
        return false;
    }
    try {
        int year = std::stoi(date.substr(0, 4));
        int month = std::stoi(date.substr(5, 2));
        int day = std::stoi(date.substr(8, 2));
        if (year < 1900 || year > 2100 || month < 1 || month > 12 || day < 1) {
            return false;
        }
        static constexpr int daysInMonth[] = {31, 28, 31, 30, 31, 30,
                                               31, 31, 30, 31, 30, 31};
        const bool leapYear = (year % 4 == 0 && year % 100 != 0) || year % 400 == 0;
        const int maximumDay = daysInMonth[month - 1] + (month == 2 && leapYear ? 1 : 0);
        if (day > maximumDay) return false;
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

    appendToJournal(record);
    records_.push_back(record);
    if (record.id.rfind("MR-", 0) == 0) {
        nextId_ = std::max(nextId_, static_cast<std::size_t>(std::stoull(record.id.substr(3))) + 1);
    }
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

void MedicalRecordRepository::appendToJournal(const MedicalRecord& record) const {
    if (!journalPath_) return;
    const std::filesystem::path path(*journalPath_);
    const auto parent = path.parent_path();
    std::error_code error;
    if (!parent.empty()) std::filesystem::create_directories(parent, error);
    if (error) {
        throw std::runtime_error("Unable to create medical record storage directory: " + error.message());
    }

    std::ofstream output(path, std::ios::app | std::ios::binary);
    if (!output) throw std::runtime_error("Unable to open medical record journal for writing.");
    output << "RECORD\t" << hexEncode(record.id) << '\t' << hexEncode(record.patientId) << '\t'
           << hexEncode(record.doctorId) << '\t' << hexEncode(record.visitDate) << '\t'
           << hexEncode(record.symptoms) << '\t' << hexEncode(record.diagnosis) << '\t'
           << hexEncode(record.treatmentNotes) << '\t' << hexEncode(record.createdAt) << '\t'
           << record.prescriptions.size();
    for (const auto& prescription : record.prescriptions) {
        output << '\t' << hexEncode(prescription);
    }
    output << '\n';
    output.flush();
    if (!output) throw std::runtime_error("Unable to save the medical record journal.");
}

void MedicalRecordRepository::loadJournal() {
    const std::filesystem::path path(*journalPath_);
    if (!std::filesystem::exists(path)) return;
    std::ifstream input(path, std::ios::binary);
    if (!input) throw std::runtime_error("Unable to open medical record journal for reading.");

    std::string line;
    std::size_t lineNumber = 0;
    while (std::getline(input, line)) {
        ++lineNumber;
        const auto fields = splitTabs(line);
        if (fields.size() < 10 || fields.front() != "RECORD") {
            throw std::runtime_error("Malformed medical record journal entry at line " +
                                     std::to_string(lineNumber) + '.');
        }
        const std::size_t prescriptionCount = parseCount(fields[9], lineNumber);
        if (fields.size() != 10 + prescriptionCount) {
            throw std::runtime_error("Malformed prescription list in medical record journal at line " +
                                     std::to_string(lineNumber) + '.');
        }

        MedicalRecord record;
        record.id = hexDecode(fields[1]);
        record.patientId = hexDecode(fields[2]);
        record.doctorId = hexDecode(fields[3]);
        record.visitDate = hexDecode(fields[4]);
        record.symptoms = hexDecode(fields[5]);
        record.diagnosis = hexDecode(fields[6]);
        record.treatmentNotes = hexDecode(fields[7]);
        record.createdAt = hexDecode(fields[8]);
        for (std::size_t index = 0; index < prescriptionCount; ++index) {
            record.prescriptions.push_back(hexDecode(fields[10 + index]));
        }
        if (record.id.empty() || record.patientId.empty() || record.doctorId.empty() ||
            record.diagnosis.empty() || !isValidDate(record.visitDate)) {
            throw std::runtime_error("Invalid medical record data in journal at line " +
                                     std::to_string(lineNumber) + '.');
        }

        records_.push_back(std::move(record));
        const std::string& id = records_.back().id;
        if (id.rfind("MR-", 0) == 0) {
            try {
                std::size_t consumed = 0;
                const auto number = std::stoull(id.substr(3), &consumed);
                if (consumed != id.size() - 3) throw std::runtime_error("bad id");
                nextId_ = std::max(nextId_, static_cast<std::size_t>(number) + 1);
            } catch (const std::exception&) {
                throw std::runtime_error("Malformed medical record ID in journal at line " +
                                         std::to_string(lineNumber) + '.');
            }
        } else {
            throw std::runtime_error("Malformed medical record ID in journal at line " +
                                     std::to_string(lineNumber) + '.');
        }
    }
    if (!input.eof()) throw std::runtime_error("Unable to finish reading the medical record journal.");
}

}  // namespace hms
