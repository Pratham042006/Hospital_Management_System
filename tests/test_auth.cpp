#include "test_support.hpp"

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
    auto system = makeTestSystem();
    const auto administrator = system.login("admin", "test-admin-password");
    const auto receptionist = system.login("receptionist", "test-reception-password");
    const auto doctor = system.login("doctor", "test-doctor-password");
    const auto billing = system.login("billing", "test-billing-password");

    CHECK(administrator && administrator->role == hms::Role::Administrator);
    CHECK(receptionist && receptionist->role == hms::Role::Receptionist);
    CHECK(doctor && doctor->role == hms::Role::Doctor);
    CHECK(billing && billing->role == hms::Role::BillingStaff);
    CHECK(!system.login("admin", "incorrect"));
    CHECK(!system.login("missing", "password"));

    const auto administratorMenu = system.menuFor(*administrator);
    const auto receptionistMenu = system.menuFor(*receptionist);
    const auto doctorMenu = system.menuFor(*doctor);
    const auto billingMenu = system.menuFor(*billing);
    CHECK(administratorMenu.size() == 7);
    CHECK(contains(receptionistMenu, "Register patient"));
    CHECK(doctorMenu.size() == 3);
    CHECK(contains(doctorMenu, "Search patients"));
    CHECK(contains(doctorMenu, "View clinical data"));
    CHECK(!contains(doctorMenu, "Register patient"));
    CHECK(!contains(billingMenu, "Update patient"));

    if (failures != 0) {
        std::cerr << failures << " authentication test assertion(s) failed.\n";
        return 1;
    }
    std::cout << "FR-01 authentication and role tests passed.\n";
    return 0;
}
