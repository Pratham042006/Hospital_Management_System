#include "test_support.hpp"

#include <iostream>
#include <string>
#include <type_traits>

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
    static_assert(!std::is_constructible_v<hms::Session, std::string, hms::Role>,
                  "Callers must not be able to forge an authenticated session.");

    auto system = makeTestSystem();
    const auto administrator = *system.login("admin", "test-admin-password");
    const auto receptionist = *system.login("receptionist", "test-reception-password");
    const auto doctor = *system.login("doctor", "test-doctor-password");
    const auto billing = *system.login("billing", "test-billing-password");

    CHECK(system.addUser(administrator, "new-user", "secure-password", hms::Role::Doctor));
    CHECK(system.login("new-user", "secure-password").has_value());
    checkThrows<hms::AuthorizationError>([&] {
        system.addUser(receptionist, "forbidden", "secure-password", hms::Role::Administrator);
    });

    hms::PatientDraft draft{"Security Patient", "1995-04-12", "Other", "9000011111",
                            "security@example.com", "1 Secure Street", "Emergency Person",
                            "8000011111"};
    const auto patient = system.registerPatient(administrator, draft);
    const auto adminView = *system.searchPatientById(administrator, patient.id);
    const auto receptionistView = *system.searchPatientById(receptionist, patient.id);
    const auto doctorView = *system.searchPatientById(doctor, patient.id);
    const auto billingView = *system.searchPatientById(billing, patient.id);

    CHECK(adminView.phone && adminView.emergencyContactPhone);
    CHECK(receptionistView.phone && !receptionistView.gender && !receptionistView.emergencyContactPhone);
    CHECK(doctorView.gender && doctorView.emergencyContactPhone && !doctorView.phone);
    CHECK(!billingView.dateOfBirth && !billingView.phone && !billingView.address);

    if (failures != 0) return 1;
    std::cout << "NFR-04 role security tests passed.\n";
    return 0;
}
