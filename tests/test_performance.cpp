#include "test_support.hpp"

#include <chrono>
#include <iostream>
#include <string>

namespace {

hms::PatientDraft patientDraft(int number) {
    return {
        "Performance Patient " + std::to_string(number),
        "1995-04-12",
        "Other",
        "90000" + std::to_string(10000 + number),
        "performance" + std::to_string(number) + "@example.com",
        std::to_string(number) + " Test Street",
        "Emergency Contact",
        "80000" + std::to_string(10000 + number),
    };
}

std::string patientId(int number) {
    const std::string digits = std::to_string(number);
    return "P" + std::string(6 - digits.size(), '0') + digits;
}

}  // namespace

int main() {
    auto system = makeTestSystem();
    const auto administrator = *system.login("admin", "test-admin-password");
    constexpr int patientCount = 10000;

    for (int number = 1; number <= patientCount; ++number) {
        system.registerPatient(administrator, patientDraft(number));
    }

    const auto start = std::chrono::steady_clock::now();
    for (int number = 1; number <= 1000; ++number) {
        if (!system.searchPatientById(administrator, patientId(number))) return 1;
    }
    if (system.searchPatientsByContact(administrator, "performance9999@example.com").size() != 1) {
        return 1;
    }
    if (system.searchPatientsByName(administrator, "performance patient 9999").size() != 1) {
        return 1;
    }
    const auto elapsed = std::chrono::duration<double>(
        std::chrono::steady_clock::now() - start);

    if (elapsed.count() >= 3.0) {
        std::cerr << "NFR-01 failed: project-scale searches took " << elapsed.count()
                  << " seconds.\n";
        return 1;
    }
    std::cout << "NFR-01 passed in " << elapsed.count() << " seconds.\n";
    return 0;
}
