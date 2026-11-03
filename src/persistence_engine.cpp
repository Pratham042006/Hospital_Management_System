#include "hms/persistence_engine.hpp"
#include "hms/safety_manager.hpp"

#include <filesystem>
#include <fstream>
#include <sstream>

namespace hms {

namespace fs = std::filesystem;

PersistenceEngine::PersistenceEngine(std::string baseStorageDirectory)
    : baseDir_(std::move(baseStorageDirectory)) {
    ensureDirectoryExists(baseDir_);
}

bool PersistenceEngine::ensureDirectoryExists(const std::string& path) {
    if (path.empty()) return true;
    std::error_code ec;
    return fs::create_directories(path, ec) || fs::exists(path);
}

std::string PersistenceEngine::getEntityPath(const std::string& entityType) const {
    return (fs::path(baseDir_) / (entityType + ".journal")).string();
}

bool PersistenceEngine::saveRecord(const std::string& entityType,
                                   const std::string& recordId,
                                   const std::string& data) {
    ensureDirectoryExists(baseDir_);
    const std::string filePath = getEntityPath(entityType);

    std::ofstream out(filePath, std::ios::app | std::ios::binary);
    if (!out.is_open()) {
        return false;
    }

    out << recordId << "\t" << data << "\n";
    return out.good();
}

std::vector<std::string> PersistenceEngine::loadAllRecords(const std::string& entityType) const {
    const std::string filePath = getEntityPath(entityType);
    std::vector<std::string> records;

    if (!fs::exists(filePath)) {
        return records;
    }

    std::ifstream in(filePath, std::ios::binary);
    std::string line;
    while (std::getline(in, line)) {
        if (!line.empty()) {
            records.push_back(line);
        }
    }
    return records;
}

bool PersistenceEngine::clearStorage(const std::string& entityType) {
    const std::string filePath = getEntityPath(entityType);
    std::error_code ec;
    return fs::remove(filePath, ec);
}

std::string PersistenceEngine::getStorageDirectory() const noexcept {
    return baseDir_;
}

}  // namespace hms
