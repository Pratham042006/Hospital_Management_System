#pragma once

#include <string>

namespace hms {

enum class RoomType {
    GeneralWard,
    SemiPrivate,
    Private,
    ICU
};

inline std::string toString(RoomType type) {
    switch (type) {
        case RoomType::GeneralWard: return "General Ward";
        case RoomType::SemiPrivate: return "Semi-Private";
        case RoomType::Private:     return "Private";
        case RoomType::ICU:         return "ICU";
    }
    return "Unknown";
}

struct Room {
    std::string id;
    std::string wardName;
    RoomType type;
    int totalBeds{1};
    int occupiedBeds{0};
    double dailyRate{500.0};

    bool hasAvailableBed() const {
        return occupiedBeds < totalBeds;
    }
};

struct RoomDraft {
    std::string id;
    std::string wardName;
    RoomType type;
    int totalBeds{1};
    double dailyRate{500.0};
};

}  // namespace hms
