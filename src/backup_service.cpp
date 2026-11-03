#include "hms/backup_service.hpp"

#include <chrono>
#include <fstream>
#include <iomanip>
#include <sstream>

namespace hms {

namespace {

constexpr const char* BACKUP_MAGIC = "HMS_BACKUP_V1";

}  // namespace

bool BackupService::exportSnapshot(
    const std::string& destinationFilePath,
    const BackupSnapshot& snapshot,
    const std::string& payloadData) {
    if (destinationFilePath.empty()) {
        return false;
    }

    std::ofstream out(destinationFilePath, std::ios::trunc | std::ios::binary);
    if (!out.is_open()) {
        return false;
    }

    out << BACKUP_MAGIC << "\n";
    out << "VERSION=" << snapshot.version << "\n";
    out << "TIMESTAMP=" << snapshot.timestamp << "\n";
    out << "PATIENTS=" << snapshot.patientCount << "\n";
    out << "DOCTORS=" << snapshot.doctorCount << "\n";
    out << "APPOINTMENTS=" << snapshot.appointmentCount << "\n";
    out << "ROOMS=" << snapshot.roomCount << "\n";
    out << "ADMISSIONS=" << snapshot.admissionCount << "\n";
    out << "RECORDS=" << snapshot.recordCount << "\n";
    out << "BILLS=" << snapshot.billCount << "\n";
    out << "MANIFEST=" << snapshot.manifestSummary << "\n";
    out << "---BEGIN_PAYLOAD---\n";
    out << payloadData;
    out << "\n---END_PAYLOAD---\n";

    return out.good();
}

bool BackupService::verifyIntegrity(const std::string& sourceFilePath) {
    std::ifstream in(sourceFilePath, std::ios::binary);
    if (!in.is_open()) {
        return false;
    }

    std::string header;
    if (!std::getline(in, header) || header != BACKUP_MAGIC) {
        return false;
    }

    bool hasVersion = false;
    bool hasPayloadBegin = false;
    bool hasPayloadEnd = false;
    std::string line;
    while (std::getline(in, line)) {
        if (line.rfind("VERSION=", 0) == 0) hasVersion = true;
        if (line == "---BEGIN_PAYLOAD---") hasPayloadBegin = true;
        if (line == "---END_PAYLOAD---") hasPayloadEnd = true;
    }

    return hasVersion && hasPayloadBegin && hasPayloadEnd;
}

std::string BackupService::loadSnapshotPayload(const std::string& sourceFilePath) {
    if (!verifyIntegrity(sourceFilePath)) {
        return "";
    }

    std::ifstream in(sourceFilePath, std::ios::binary);
    std::string line;
    bool readingPayload = false;
    std::ostringstream payload;

    while (std::getline(in, line)) {
        if (line == "---BEGIN_PAYLOAD---") {
            readingPayload = true;
            continue;
        }
        if (line == "---END_PAYLOAD---") {
            break;
        }
        if (readingPayload) {
            payload << line << "\n";
        }
    }
    return payload.str();
}

}  // namespace hms
