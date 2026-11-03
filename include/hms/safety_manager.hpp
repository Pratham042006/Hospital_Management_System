#pragma once

#include <string>
#include <utility>
#include <vector>

namespace hms {

class SafetyManager {
public:
    static bool atomicWriteFile(const std::string& destinationPath, const std::string& content);
    static bool safeFlush(const std::vector<std::pair<std::string, std::string>>& entityDumps);
    static bool verifyConsistency(
        const std::vector<std::string>& patientIds,
        const std::vector<std::string>& doctorIds,
        const std::vector<std::string>& roomIds,
        const std::vector<std::pair<std::string, std::string>>& admissionPatientAndRooms);
};

}  // namespace hms
