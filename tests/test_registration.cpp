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
        "Patient " + std::to_string(number),
        "1995-04-12",
        "Other",
        "90000" + std::to_string(10000 + number),
        "patient" + std::to_string(number) + "@example.com",
        std::to_string(number) + " Test Street",
        "Emergency Contact",
        "80000" + std::to_string(10000 + number),
    };
}

}  // namespace

int main() {
    auto system = makeTestSystem();
    const auto receptionist = *system.login("receptionist", "test-reception-password");
    const auto doctor = *system.login("doctor", "test-doctor-password");

    const auto first = system.registerPatient(receptionist, patientDraft(1));
    const auto second = system.registerPatient(receptionist, patientDraft(2));
    CHECK(first.id == "P000001");
    CHECK(second.id == "P000002");
    CHECK(system.patientCount() == 2);

    auto missingName = patientDraft(3);
    missingName.fullName.clear();
    checkThrows<std::invalid_argument>([&] { system.registerPatient(receptionist, missingName); });

    auto duplicate = patientDraft(4);
    duplicate.phone = first.phone;
    checkThrows<std::invalid_argument>([&] { system.registerPatient(receptionist, duplicate); });
    checkThrows<hms::AuthorizationError>([&] { system.registerPatient(doctor, patientDraft(5)); });
    CHECK(system.patientCount() == 2);

    if (failures != 0) {
        std::cerr << failures << " registration test assertion(s) failed.\n";
        return 1;
    }
    std::cout << "FR-02 patient registration tests passed.\n";
    return 0;
}
