#include "test_support.hpp"

#include <iostream>
#include <stdexcept>

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
    try { callable(); ++failures; } catch (const Exception&) {} catch (...) { ++failures; }
}
}  // namespace

int main() {
    auto system = makeTestSystem();
    const auto administrator = *system.login("admin", "test-admin-password");
    const auto receptionist = *system.login("receptionist", "test-reception-password");
    const auto doctorRole = *system.login("doctor", "test-doctor-password");
    const auto billing = *system.login("billing", "test-billing-password");

    hms::DoctorDraft draft{"Dr Asha Rao", "Cardiology", "Heart Centre", "9111122222",
                           "asha@example.com", {"2026-11-02 09:30", "2026-11-03 10:15"}};
    const auto doctor = system.addDoctor(receptionist, draft);
    CHECK(doctor.id == "D000001");
    CHECK(system.findDoctorById(doctorRole, doctor.id).has_value());
    CHECK(system.searchDoctors(doctorRole, "cardiology").size() == 1);
    CHECK(system.searchDoctors(receptionist, "heart centre").size() == 1);
    CHECK(system.listDoctors(administrator).size() == 1);

    hms::DoctorUpdate update;
    update.department = "Cardiac Sciences";
    update.availability = std::vector<std::string>{"2026-11-04 11:00"};
    const auto updated = system.updateDoctor(administrator, doctor.id, update);
    CHECK(updated.id == doctor.id);
    CHECK(updated.department == "Cardiac Sciences");
    CHECK(updated.availability.size() == 1);

    auto duplicate = draft;
    duplicate.fullName = "Dr Duplicate";
    checkThrows<std::invalid_argument>([&] { system.addDoctor(receptionist, duplicate); });
    auto invalid = draft;
    invalid.phone = "12";
    invalid.email = "invalid";
    checkThrows<std::invalid_argument>([&] { system.addDoctor(receptionist, invalid); });
    checkThrows<hms::AuthorizationError>([&] { system.addDoctor(doctorRole, draft); });
    checkThrows<hms::AuthorizationError>([&] { system.listDoctors(billing); });

    if (failures != 0) return 1;
    std::cout << "FR-05 doctor management tests passed.\n";
    return 0;
}
