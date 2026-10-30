#include "test_support.hpp"

#include <iostream>
#include <stdexcept>

// NFR-04 Security Boundary Regression Tests
// Author: Pratik Patil (PES2UG24CS372)

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
    const auto billing = *system.login("billing", "test-billing-password");

    // 1. Verify Role-specific action menu boundaries
    const auto adminMenu = system.menuFor(admin);
    const auto receptionistMenu = system.menuFor(receptionist);
    const auto doctorMenu = system.menuFor(doctor);
    const auto billingMenu = system.menuFor(billing);

    CHECK(!receptionistMenu.empty());
    CHECK(!doctorMenu.empty());
    CHECK(!billingMenu.empty());

    // 2. Enforce Administrator-only user account creation
    checkThrows<hms::AuthorizationError>([&] {
        system.addUser(receptionist, "hacker", "password123", hms::Role::Administrator);
    });
    checkThrows<hms::AuthorizationError>([&] {
        system.addUser(doctor, "hacker", "password123", hms::Role::Administrator);
    });
    checkThrows<hms::AuthorizationError>([&] {
        system.addUser(billing, "hacker", "password123", hms::Role::Administrator);
    });

    // 3. Enforce Doctor and Patient registration boundary (Doctor & Billing cannot register patients or doctors)
    hms::DoctorDraft doctorDraft{
        "Dr. Test", "Neurology", "Neuroscience", "9988776655", "dr.test@hms.org",
        {"2026-11-10 10:00"}
    };
    checkThrows<hms::AuthorizationError>([&] {
        system.addDoctor(doctor, doctorDraft);
    });
    checkThrows<hms::AuthorizationError>([&] {
        system.addDoctor(billing, doctorDraft);
    });

    hms::PatientDraft patientDraft{
        "Restricted Patient", "1992-03-04", "Female", "9998887776", "patient@test.com",
        "Hospital Rd", "Kin", "9998887770"
    };
    checkThrows<hms::AuthorizationError>([&] {
        system.registerPatient(doctor, patientDraft);
    });
    checkThrows<hms::AuthorizationError>([&] {
        system.registerPatient(billing, patientDraft);
    });

    if (failures != 0) {
        std::cerr << failures << " security boundary assertion(s) failed.\n";
        return 1;
    }
    std::cout << "NFR-04 security boundary tests passed successfully.\n";
    return 0;
}
