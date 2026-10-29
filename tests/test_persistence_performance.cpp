#include "hms/hospital_system.hpp"

#include <chrono>
#include <filesystem>
#include <iostream>
#include <string>

namespace {

const std::vector<hms::InitialAccount> accounts{
    {"admin", "test-admin-password", hms::Role::Administrator},
};

hms::PatientDraft patientDraft(int number) {
    return {"Persistent Patient " + std::to_string(number), "1995-04-12", "Other",
            "70000" + std::to_string(10000 + number),
            "persistent" + std::to_string(number) + "@example.com",
            std::to_string(number) + " Journal Street", "Emergency Contact",
            "60000" + std::to_string(10000 + number)};
}

}  // namespace

int main() {
    const auto unique = std::to_string(
        std::chrono::steady_clock::now().time_since_epoch().count());
    const auto journal = std::filesystem::temp_directory_path() /
                         ("hms-patient-performance-" + unique + ".journal");
    std::error_code error;
    std::filesystem::remove(journal, error);
    constexpr int patientCount = 10000;

    const auto start = std::chrono::steady_clock::now();
    {
        hms::HospitalSystem system(accounts, journal.string());
        const auto administrator = *system.login("admin", "test-admin-password");
        for (int number = 1; number <= patientCount; ++number) {
            system.registerPatient(administrator, patientDraft(number));
        }
    }
    const auto sizeAfterRegistrations = std::filesystem::file_size(journal);
    {
        hms::HospitalSystem reloaded(accounts, journal.string());
        const auto administrator = *reloaded.login("admin", "test-admin-password");
        if (reloaded.patientCount() != patientCount) return 1;
        const auto patient = reloaded.searchPatientById(administrator, "P010000");
        if (!patient || patient->fullName != "Persistent Patient 10000") return 1;
        hms::PatientUpdate update;
        update.address = "Updated Journal Street";
        reloaded.updatePatient(administrator, "P010000", update);
    }
    const auto sizeAfterUpdate = std::filesystem::file_size(journal);
    const auto elapsed = std::chrono::duration<double>(
        std::chrono::steady_clock::now() - start);

    if (sizeAfterUpdate <= sizeAfterRegistrations) {
        std::cerr << "Persistence must append updates rather than rewrite the journal.\n";
        return 1;
    }
    if (elapsed.count() >= 3.0) {
        std::cerr << "Persistent project-scale workload took " << elapsed.count() << " seconds.\n";
        return 1;
    }
    {
        hms::HospitalSystem finalReload(accounts, journal.string());
        const auto administrator = *finalReload.login("admin", "test-admin-password");
        const auto patient = finalReload.searchPatientById(administrator, "P010000");
        if (!patient || !patient->address || *patient->address != "Updated Journal Street") return 1;
    }
    std::filesystem::remove(journal, error);
    std::cout << "NFR-01 persistent workload passed in " << elapsed.count() << " seconds.\n";
    return 0;
}
