#pragma once

#include "hms/room.hpp"

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace hms {

class RoomRepository {
public:
    Room addRoom(const RoomDraft& draft);
    std::optional<Room> findRoomById(const std::string& roomId) const;
    std::vector<Room> listAllRooms() const;
    std::vector<Room> listAvailableRooms() const;

    bool allocateBed(const std::string& roomId);
    bool releaseBed(const std::string& roomId);
    std::size_t totalRooms() const;

private:
    std::unordered_map<std::string, Room> rooms_;
};

}  // namespace hms
