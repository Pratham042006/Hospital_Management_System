#include "test_support.hpp"

#include <iostream>
#include <stdexcept>

// FR-09 Room and Bed Allocation Tests
// Author: Prathik Nambiar (PES2UG24CS371)

namespace {

int failures = 0;

#define CHECK(condition)                                                                    \
    do {                                                                                    \
        if (!(condition)) {                                                                 \
            std::cerr << "CHECK failed at line " << __LINE__ << ": " #condition << '\n'; \
            ++failures;                                                                     \
        }                                                                                   \
    } while (false)

template <typename Exception, typename Callable>
void checkThrows(Callable&& callable) {
    try {
        callable();
        ++failures;
    } catch (const Exception&) {
    } catch (...) {
        ++failures;
    }
}

}  // namespace

int main() {
    auto system = makeTestSystem();
    const auto admin = *system.login("admin", "test-admin-password");
    const auto receptionist = *system.login("receptionist", "test-reception-password");
    const auto doctor = *system.login("doctor", "test-doctor-password");

    // 1. Add valid rooms across different wards and room types
    hms::RoomDraft generalWard{"R-101", "North Ward", hms::RoomType::GeneralWard, 4, 350.0};
    hms::RoomDraft icuRoom{"R-ICU-1", "Critical Care", hms::RoomType::ICU, 2, 2500.0};
    hms::RoomDraft privateRoom{"R-201", "Deluxe Ward", hms::RoomType::Private, 1, 1500.0};

    const auto r1 = system.addRoom(admin, generalWard);
    CHECK(r1.id == "R-101");
    CHECK(r1.totalBeds == 4);
    CHECK(r1.occupiedBeds == 0);
    CHECK(r1.hasAvailableBed());

    const auto r2 = system.addRoom(admin, icuRoom);
    CHECK(r2.type == hms::RoomType::ICU);

    const auto r3 = system.addRoom(receptionist, privateRoom);
    CHECK(r3.hasAvailableBed());

    // 2. Reject duplicate room IDs
    checkThrows<std::invalid_argument>([&] {
        system.addRoom(admin, generalWard);
    });

    // 3. Reject invalid bed count or negative daily rate
    hms::RoomDraft invalidBeds{"R-ERR-1", "Test", hms::RoomType::GeneralWard, 0, 500.0};
    checkThrows<std::invalid_argument>([&] {
        system.addRoom(admin, invalidBeds);
    });

    hms::RoomDraft invalidRate{"R-ERR-2", "Test", hms::RoomType::GeneralWard, 2, -100.0};
    checkThrows<std::invalid_argument>([&] {
        system.addRoom(admin, invalidRate);
    });

    // 4. Verify room listing and filtering
    const auto allRooms = system.listAllRooms(doctor);
    CHECK(allRooms.size() == 3);

    const auto available = system.listAvailableRooms(receptionist);
    CHECK(available.size() == 3);

    const auto found = system.findRoomById(doctor, "R-101");
    CHECK(found.has_value());
    CHECK(found->wardName == "North Ward");

    CHECK(!system.findRoomById(doctor, "NONEXISTENT").has_value());

    if (failures != 0) {
        std::cerr << failures << " room allocation assertion(s) failed.\n";
        return 1;
    }
    std::cout << "FR-09 room allocation tests passed successfully.\n";
    return 0;
}
