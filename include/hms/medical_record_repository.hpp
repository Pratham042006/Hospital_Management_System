#pragma once

#include "hms/medical_record.hpp"

#include <optional>
#include <string>
#include <vector>

namespace hms {

class MedicalRecordRepository {
public:
    explicit MedicalRecordRepository(
        const std::optional<std::string>& journalPath = std::nullopt);

    MedicalRecord addRecord(const MedicalRecordDraft& draft);
    std::optional<MedicalRecord> findById(const std::string& recordId) const;
    std::vector<MedicalRecord> findByPatientId(const std::string& patientId) const;
    std::vector<MedicalRecord> listAll() const;
    std::size_t size() const noexcept;

    static bool isValidDate(const std::string& date);

private:
    std::vector<MedicalRecord> records_;
    std::size_t nextId_{1};
    std::optional<std::string> journalPath_;

    std::string generateId();
    void appendToJournal(const MedicalRecord& record) const;
    void loadJournal();
};

}  // namespace hms
