#pragma once

#include <string>
#include <vector>

namespace hms {

class PersistenceEngine {
public:
    explicit PersistenceEngine(std::string baseStorageDirectory);

    bool saveRecord(const std::string& entityType, const std::string& recordId, const std::string& data);
    std::vector<std::string> loadAllRecords(const std::string& entityType) const;
    bool clearStorage(const std::string& entityType);
    std::string getStorageDirectory() const noexcept;

    static bool ensureDirectoryExists(const std::string& path);

private:
    std::string baseDir_;
    std::string getEntityPath(const std::string& entityType) const;
};

}  // namespace hms
