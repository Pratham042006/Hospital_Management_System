#include "hms/room_repository.hpp"

#include <algorithm>
#include <stdexcept>

namespace hms {

Room RoomRepository::addRoom(const RoomDraft& draft) {
    if (draft.id.empty()) {
        throw std::invalid_argument("Room ID cannot be empty.");
    }
    if (draft.wardName.empty()) {
        throw std::invalid_argument("Ward name cannot be empty.");
    }
    if (draft.totalBeds <= 0) {
        throw std::invalid_argument("Room must have at least one bed.");
    }
    if (draft.dailyRate < 0.0) {
        throw std::invalid_argument("Daily rate cannot be negative.");
    }
    if (rooms_.find(draft.id) != rooms_.end()) {
        throw std::invalid_argument("Room with ID " + draft.id + " already exists.");
    }

    Room room{
        draft.id,
        draft.wardName,
        draft.type,
        draft.totalBeds,
        0,
        draft.dailyRate
    };

    rooms_.emplace(draft.id, room);
    return room;
}

std::optional<Room> RoomRepository::findRoomById(const std::string& roomId) const {
    const auto it = rooms_.find(roomId);
    if (it == rooms_.end()) {
        return std::nullopt;
    }
    return it->second;
}

std::vector<Room> RoomRepository::listAllRooms() const {
    std::vector<Room> result;
    result.reserve(rooms_.size());
    for (const auto& [_, room] : rooms_) {
        result.push_back(room);
    }
    std::sort(result.begin(), result.end(), [](const Room& a, const Room& b) {
        return a.id < b.id;
    });
    return result;
}

std::vector<Room> RoomRepository::listAvailableRooms() const {
    std::vector<Room> result;
    for (const auto& [_, room] : rooms_) {
        if (room.hasAvailableBed()) {
            result.push_back(room);
        }
    }
    std::sort(result.begin(), result.end(), [](const Room& a, const Room& b) {
        return a.id < b.id;
    });
    return result;
}

bool RoomRepository::allocateBed(const std::string& roomId) {
    auto it = rooms_.find(roomId);
    if (it == rooms_.end()) {
        throw std::invalid_argument("Room " + roomId + " does not exist.");
    }
    if (!it->second.hasAvailableBed()) {
        return false;
    }
    it->second.occupiedBeds += 1;
    return true;
}

bool RoomRepository::releaseBed(const std::string& roomId) {
    auto it = rooms_.find(roomId);
    if (it == rooms_.end()) {
        throw std::invalid_argument("Room " + roomId + " does not exist.");
    }
    if (it->second.occupiedBeds <= 0) {
        return false;
    }
    it->second.occupiedBeds -= 1;
    return true;
}

std::size_t RoomRepository::totalRooms() const {
    return rooms_.size();
}

}  // namespace hms
