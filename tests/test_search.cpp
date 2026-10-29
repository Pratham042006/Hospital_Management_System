#include "test_support.hpp"

#include <iostream>

namespace {

int failures = 0;

#define CHECK(condition)                                                                    \
    do {                                                                                    \
        if (!(condition)) {                                                                 \
            std::cerr << "CHECK failed at line " << __LINE__ << ": " #condition << '\n'; \
            ++failures;                                                                     \
        }                                                                                   \
    } while (false)

hms::PatientDraft patientDraft(int number, const std::string& name) {
    return {
        name, "1995-04-12", "Other", "90000" + std::to_string(10000 + number),
        "patient" + std::to_string(number) + "@example.com",
        std::to_string(number) + " Test Street", "Emergency Contact",
        "80000" + std::to_string(10000 + number),
    };
}

}  // namespace

int main() {
    auto system = makeTestSystem();
    const auto receptionist = *system.login("receptionist", "test-reception-password");
    const auto first = system.registerPatient(receptionist, patientDraft(1, "Anita Rao"));
    system.registerPatient(receptionist, patientDraft(2, "Anil Rao"));

    const auto byId = system.searchPatientById(receptionist, first.id);
    CHECK(byId && byId->fullName == "Anita Rao");
    CHECK(system.searchPatientsByName(receptionist, "rao").size() == 2);
    CHECK(system.searchPatientsByName(receptionist, "ANITA").size() == 1);
    CHECK(system.searchPatientsByContact(receptionist, first.phone).size() == 1);
    CHECK(system.searchPatientsByContact(receptionist, first.email).size() == 1);
    CHECK(!system.searchPatientById(receptionist, "P999999"));
    CHECK(system.searchPatientsByName(receptionist, "Nobody").empty());
    CHECK(system.searchPatientsByContact(receptionist, "missing@example.com").empty());

    if (failures != 0) {
        std::cerr << failures << " search test assertion(s) failed.\n";
        return 1;
    }
    std::cout << "FR-04 patient search tests passed.\n";
    return 0;
}
