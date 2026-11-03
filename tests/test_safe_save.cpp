#include "hms/hospital_system.hpp"
#include "hms/safety_manager.hpp"
#include "test_support.hpp"

#include <cassert>
#include <cstdio>
#include <fstream>
#include <iostream>

void testAtomicFileWrite() {
    const std::string filename = "test_atomic_save.dat";
    const std::string testContent = "HEALTHCARE_RECORDS_CONSISTENT_STATE_2026";

    bool written = hms::SafetyManager::atomicWriteFile(filename, testContent);
    assert(written);

    std::ifstream in(filename);
    assert(in.is_open());
    std::string line;
    std::getline(in, line);
    assert(line == testContent);
    in.close();

    std::remove(filename.c_str());
    std::remove((filename + ".tmp").c_str());
    std::cout << "testAtomicFileWrite passed\n";
}

void testSafeShutdownLifecycle() {
    auto system = makeTestSystem();
    const auto admin = *system.login("admin", "test-admin-password");

    // Register patient
    hms::PatientDraft p;
    p.fullName = "Diana Prince";
    p.dateOfBirth = "1980-03-22";
    p.gender = "Female";
    p.phone = "9876543230";
    p.email = "diana@themyscira.gov";
    p.address = "Gateway City";
    p.emergencyContactName = "Hippolyta";
    p.emergencyContactPhone = "9876543231";
    system.registerPatient(admin, p);

    // Perform safe shutdown
    bool shutdownSuccess = system.safeShutdown(admin);
    assert(shutdownSuccess);

    // Verify consistency check
    bool consistent = system.checkDataConsistency(admin);
    assert(consistent);

    std::remove("patients_shutdown.dat");
    std::remove("admissions_shutdown.dat");
    std::remove("rooms_shutdown.dat");

    std::cout << "testSafeShutdownLifecycle passed\n";
}

int main() {
    testAtomicFileWrite();
    testSafeShutdownLifecycle();
    std::cout << "All safe save and shutdown tests passed successfully.\n";
    return 0;
}
