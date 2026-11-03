#include "hms/safety_manager.hpp"

#include <algorithm>
#include <cstdio>
#include <fstream>
#include <iostream>

namespace hms {

bool SafetyManager::atomicWriteFile(const std::string& destinationPath, const std::string& content) {
    if (destinationPath.empty()) {
        return false;
    }

    const std::string tempPath = destinationPath + ".tmp";
    {
        std::ofstream out(tempPath, std::ios::trunc | std::ios::binary);
        if (!out.is_open()) {
            return false;
        }
        out << content;
        out.flush();
        if (!out.good()) {
            return false;
        }
    }

    // Atomic filesystem swap
    if (std::rename(tempPath.c_str(), destinationPath.c_str()) != 0) {
        // Fallback for cross-device renames or Windows locks
        std::remove(destinationPath.c_str());
        if (std::rename(tempPath.c_str(), destinationPath.c_str()) != 0) {
            std::remove(tempPath.c_str());
            return false;
        }
    }

    return true;
}

bool SafetyManager::safeFlush(const std::vector<std::pair<std::string, std::string>>& entityDumps) {
    bool allSucceeded = true;
    for (const auto& [filePath, data] : entityDumps) {
        if (!atomicWriteFile(filePath, data)) {
            allSucceeded = false;
        }
    }
    return allSucceeded;
}

bool SafetyManager::verifyConsistency(
    const std::vector<std::string>& patientIds,
    const std::vector<std::string>& doctorIds,
    const std::vector<std::string>& roomIds,
    const std::vector<std::pair<std::string, std::string>>& admissionPatientAndRooms) {
    static_cast<void>(doctorIds);
    for (const auto& [patId, roomId] : admissionPatientAndRooms) {
        const bool patExists = std::find(patientIds.begin(), patientIds.end(), patId) != patientIds.end();
        const bool roomExists = std::find(roomIds.begin(), roomIds.end(), roomId) != roomIds.end();
        if (!patExists || !roomExists) {
            return false;
        }
    }
    return true;
}

}  // namespace hms
