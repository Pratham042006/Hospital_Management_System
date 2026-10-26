#include "hms/auth.hpp"

#include <algorithm>
#include <iostream>
#include <string>

namespace {

int failures = 0;

#define CHECK(condition)                                                                    \
    do {                                                                                    \
        if (!(condition)) {                                                                 \
            std::cerr << "CHECK failed at line " << __LINE__ << ": " #condition << '\n'; \
            ++failures;                                                                     \
        }                                                                                   \
    } while (false)

bool contains(const std::vector<std::string>& values, const std::string& value) {
    return std::find(values.begin(), values.end(), value) != values.end();
}

}  // namespace

int main() {
    hms::AuthService auth;
    const auto administrator = auth.authenticate("admin", "admin123");
    const auto receptionist = auth.authenticate("receptionist", "reception123");
    const auto doctor = auth.authenticate("doctor", "doctor123");
    const auto billing = auth.authenticate("billing", "billing123");

    CHECK(administrator && administrator->role == hms::Role::Administrator);
    CHECK(receptionist && receptionist->role == hms::Role::Receptionist);
    CHECK(doctor && doctor->role == hms::Role::Doctor);
    CHECK(billing && billing->role == hms::Role::BillingStaff);
    CHECK(!auth.authenticate("admin", "incorrect"));
    CHECK(!auth.authenticate("missing", "password"));

    CHECK(auth.isAllowed(*administrator, hms::Permission::ManageUsers));
    CHECK(auth.isAllowed(*receptionist, hms::Permission::RegisterPatient));
    CHECK(!auth.isAllowed(*doctor, hms::Permission::RegisterPatient));
    CHECK(!auth.isAllowed(*billing, hms::Permission::UpdatePatient));

    const auto administratorMenu = auth.availableActions(*administrator);
    const auto doctorMenu = auth.availableActions(*doctor);
    CHECK(administratorMenu.size() == 6);
    CHECK(doctorMenu.size() == 2);
    CHECK(contains(doctorMenu, "Search patients"));
    CHECK(contains(doctorMenu, "View clinical data"));

    if (failures != 0) {
        std::cerr << failures << " authentication test assertion(s) failed.\n";
        return 1;
    }
    std::cout << "FR-01 authentication and role tests passed.\n";
    return 0;
}
