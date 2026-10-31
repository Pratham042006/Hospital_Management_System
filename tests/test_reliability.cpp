#include "test_support.hpp"

#include <iostream>
#include <stdexcept>
#include <string>

// NFR-03 Reliability & Fault Tolerance Tests
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
    const auto receptionist = *system.login("receptionist", "test-reception-password");

    // 1. Reliability check: Repeated malformed inputs do not corrupt internal state
    hms::PatientDraft invalidName{"", "1990-01-01", "Male", "9876543210", "", "Bangalore", "Kin", "9876543211"};
    for (int i = 0; i < 5; ++i) {
        checkThrows<std::invalid_argument>([&] {
            system.registerPatient(receptionist, invalidName);
        });
    }
    CHECK(system.patientCount() == 0);

    // 2. Reliability check: System remains fully functional after handled exceptions
    hms::PatientDraft validPatient{
        "Vikas Reddy", "1994-08-10", "Male", "9876543210",
        "vikas@example.com", "Whitefield, Bangalore", "Suresh Reddy", "9876543211"
    };
    const auto registered = system.registerPatient(receptionist, validPatient);
    CHECK(!registered.id.empty());
    CHECK(system.patientCount() == 1);

    // 3. Reliability check: Duplicate phone rejection preserves previous state intact
    checkThrows<std::invalid_argument>([&] {
        system.registerPatient(receptionist, validPatient);
    });
    CHECK(system.patientCount() == 1);

    // 4. Reliability check: Extreme string length inputs are handled safely
    const std::string extremeString(2048, 'X');
    hms::DoctorDraft extremeDoctor{
        extremeString, "Orthopedics", "Bone & Joint", "9812345678",
        "doc@hms.org", {"2026-11-15 10:00"}
    };
    const auto doc = system.addDoctor(receptionist, extremeDoctor);
    CHECK(doc.fullName == extremeString);

    // 5. Reliability check: Partial failures in admission rollback room bed state
    hms::RoomDraft singleBedRoom{"R-RELIABLE-1", "Test Ward", hms::RoomType::Private, 1, 500.0};
    system.addRoom(receptionist, singleBedRoom);

    // Normal admission succeeds
    hms::AdmissionDraft adm1{registered.id, doc.id, "R-RELIABLE-1", "2026-11-01", "Checkup"};
    const auto admission1 = system.admitPatient(receptionist, adm1);
    CHECK(admission1.id == "ADM-00001");

    // Room is now full; subsequent admission fails and leaves bed count exactly at 1
    hms::PatientDraft secondPat{
        "Kiran Patel", "1992-04-15", "Female", "9876543299",
        "kiran@example.com", "Koramangala, Bangalore", "A. Patel", "9876543298"
    };
    const auto reg2 = system.registerPatient(receptionist, secondPat);

    hms::AdmissionDraft adm2{reg2.id, doc.id, "R-RELIABLE-1", "2026-11-01", "Overflow test"};
    checkThrows<std::invalid_argument>([&] {
        system.admitPatient(receptionist, adm2);
    });

    const auto roomCheck = system.findRoomById(receptionist, "R-RELIABLE-1");
    CHECK(roomCheck && roomCheck->occupiedBeds == 1);

    if (failures != 0) {
        std::cerr << failures << " reliability assertion(s) failed.\n";
        return 1;
    }
    std::cout << "NFR-03 reliability checks passed successfully.\n";
    return 0;
}
