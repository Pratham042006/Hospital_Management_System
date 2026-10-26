#include "hms/hospital_system.hpp"

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
    try {
        callable();
        ++failures;
    } catch (const Exception&) {
    } catch (...) {
        ++failures;
    }
}

hms::PatientDraft patientDraft(int number) {
    return {
        "Patient " + std::to_string(number), "1995-04-12", "Other",
        "90000" + std::to_string(10000 + number),
        "patient" + std::to_string(number) + "@example.com",
        std::to_string(number) + " Test Street", "Emergency Contact",
        "80000" + std::to_string(10000 + number),
    };
}

}  // namespace

int main() {
    hms::HospitalSystem system;
    const auto administrator = *system.login("admin", "admin123");
    const auto billing = *system.login("billing", "billing123");
    const auto original = system.registerPatient(administrator, patientDraft(1));
    const auto other = system.registerPatient(administrator, patientDraft(2));

    hms::PatientUpdate update;
    update.fullName = "Updated Name";
    update.phone = "+91 98765 43210";
    const auto updated = system.updatePatient(administrator, original.id, update);
    CHECK(updated.id == original.id);
    CHECK(updated.fullName == "Updated Name");
    CHECK(updated.phone == "+91 98765 43210");

    hms::PatientUpdate invalid;
    invalid.dateOfBirth = "2026-99-99";
    checkThrows<std::invalid_argument>([&] {
        system.updatePatient(administrator, original.id, invalid);
    });
    const auto afterInvalidUpdate = system.updatePatient(administrator, original.id, {});
    CHECK(afterInvalidUpdate.dateOfBirth == original.dateOfBirth);

    hms::PatientUpdate duplicate;
    duplicate.email = other.email;
    checkThrows<std::invalid_argument>([&] {
        system.updatePatient(administrator, original.id, duplicate);
    });
    const auto afterDuplicateUpdate = system.updatePatient(administrator, original.id, {});
    CHECK(afterDuplicateUpdate.email == original.email);
    checkThrows<std::invalid_argument>([&] {
        system.updatePatient(administrator, "P999999", update);
    });
    checkThrows<hms::AuthorizationError>([&] {
        system.updatePatient(billing, original.id, update);
    });

    if (failures != 0) {
        std::cerr << failures << " update test assertion(s) failed.\n";
        return 1;
    }
    std::cout << "FR-03 patient update tests passed.\n";
    return 0;
}
