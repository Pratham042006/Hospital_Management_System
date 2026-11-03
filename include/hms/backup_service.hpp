#pragma once

#include <string>
#include <vector>

namespace hms {

struct BackupSnapshot {
    std::string version{"1.0.0"};
    std::string timestamp;
    std::size_t patientCount{0};
    std::size_t doctorCount{0};
    std::size_t appointmentCount{0};
    std::size_t roomCount{0};
    std::size_t admissionCount{0};
    std::size_t recordCount{0};
    std::size_t billCount{0};
    std::string manifestSummary;
};

class BackupService {
public:
    static bool exportSnapshot(
        const std::string& destinationFilePath,
        const BackupSnapshot& snapshot,
        const std::string& payloadData);

    static std::string loadSnapshotPayload(const std::string& sourceFilePath);
    static bool verifyIntegrity(const std::string& sourceFilePath);
};

}  // namespace hms
