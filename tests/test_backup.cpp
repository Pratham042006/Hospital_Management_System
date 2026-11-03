#include "hms/hospital_system.hpp"
#include "test_support.hpp"

#include <cassert>
#include <cstdio>
#include <fstream>
#include <iostream>

void testBackupExportAndVerification() {
    auto system = makeTestSystem();
    const auto admin = *system.login("admin", "test-admin-password");
    const auto receptionist = *system.login("receptionist", "test-reception-password");

    // Register a patient
    hms::PatientDraft p;
    p.fullName = "Bruce Wayne";
    p.dateOfBirth = "1975-02-19";
    p.gender = "Male";
    p.phone = "9876543220";
    p.email = "bruce@wayneenterprises.com";
    p.address = "1007 Mountain Drive, Gotham";
    p.emergencyContactName = "Alfred Pennyworth";
    p.emergencyContactPhone = "9876543221";
    system.registerPatient(admin, p);

    const std::string backupPath = "test_system_backup.hms";

    // Export backup
    bool exported = system.exportSystemBackup(admin, backupPath);
    assert(exported);

    // Verify backup file exists
    std::ifstream file(backupPath);
    assert(file.is_open());
    file.close();

    // Verify integrity check
    bool valid = system.verifyBackupFile(admin, backupPath);
    assert(valid);

    // Non-existent file verification fails
    bool missingValid = system.verifyBackupFile(admin, "non_existent_file.hms");
    assert(!missingValid);

    // Authorization check: Receptionist cannot export backup
    bool unauthorizedExport = false;
    try {
        system.exportSystemBackup(receptionist, "reception_backup.hms");
    } catch (const hms::AuthorizationError&) {
        unauthorizedExport = true;
    }
    assert(unauthorizedExport);

    std::remove(backupPath.c_str());
    std::cout << "testBackupExportAndVerification passed\n";
}

int main() {
    testBackupExportAndVerification();
    std::cout << "All backup tests passed successfully.\n";
    return 0;
}
