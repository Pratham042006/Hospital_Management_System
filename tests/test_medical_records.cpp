#include "hms/hospital_system.hpp"
#include "hms/medical_record_repository.hpp"
#include "test_support.hpp"

#include <cassert>
#include <chrono>
#include <filesystem>
#include <iostream>

void testMedicalRecordCreationAndRetrieval() {
    auto system = makeTestSystem();
    const auto doctorSession = *system.login("doctor", "test-doctor-password");
    const auto adminSession = *system.login("admin", "test-admin-password");
    const auto receptionistSession = *system.login("receptionist", "test-reception-password");

    // Register a patient
    hms::PatientDraft patientDraft;
    patientDraft.fullName = "Emily Rose";
    patientDraft.dateOfBirth = "1992-04-12";
    patientDraft.gender = "Female";
    patientDraft.phone = "9876543210";
    patientDraft.email = "emily.rose@example.com";
    patientDraft.address = "742 Evergreen Terrace";
    patientDraft.emergencyContactName = "John Rose";
    patientDraft.emergencyContactPhone = "9876543219";
    const auto patient = system.registerPatient(adminSession, patientDraft);

    // Add a doctor
    hms::DoctorDraft docDraft;
    docDraft.fullName = "Dr. Gregory House";
    docDraft.specialization = "Diagnostics";
    docDraft.department = "Internal Medicine";
    docDraft.phone = "9876543211";
    docDraft.email = "ghouse@hospital.com";
    docDraft.availability = {"2026-11-04 09:00", "2026-11-04 10:00"};
    const auto doctor = system.addDoctor(adminSession, docDraft);

    // Create medical record with doctor session
    hms::MedicalRecordDraft recordDraft;
    recordDraft.patientId = patient.id;
    recordDraft.doctorId = doctor.id;
    recordDraft.visitDate = "2026-11-04";
    recordDraft.symptoms = "Persistent cough, fever of 101F for 3 days";
    recordDraft.diagnosis = "Acute Bronchitis";
    recordDraft.treatmentNotes = "Advised rest, increased fluid intake, and prescribed inhaler.";
    recordDraft.prescriptions = {"Amoxicillin 500mg TDS 5 days", "Albuterol Inhaler 2 puffs PRN"};

    const auto record = system.createMedicalRecord(doctorSession, recordDraft);
    assert(!record.id.empty());
    assert(record.patientId == patient.id);
    assert(record.doctorId == doctor.id);
    assert(record.diagnosis == "Acute Bronchitis");
    assert(record.prescriptions.size() == 2);

    // Retrieve by record ID
    const auto fetched = system.findMedicalRecordById(doctorSession, record.id);
    assert(fetched.has_value());
    assert(fetched->diagnosis == "Acute Bronchitis");
    assert(fetched->symptoms == "Persistent cough, fever of 101F for 3 days");

    // Retrieve by patient ID
    const auto patientRecords = system.getMedicalRecordsByPatient(doctorSession, patient.id);
    assert(patientRecords.size() == 1);
    assert(patientRecords[0].id == record.id);

    // Authorization: Receptionist cannot create medical record
    bool rejected = false;
    try {
        system.createMedicalRecord(receptionistSession, recordDraft);
    } catch (const hms::AuthorizationError&) {
        rejected = true;
    }
    assert(rejected);

    // Authorization: Receptionist cannot view clinical data
    bool viewRejected = false;
    try {
        system.getMedicalRecordsByPatient(receptionistSession, patient.id);
    } catch (const hms::AuthorizationError&) {
        viewRejected = true;
    }
    assert(viewRejected);

    std::cout << "testMedicalRecordCreationAndRetrieval passed\n";
}

void testMedicalRecordValidation() {
    auto system = makeTestSystem();
    const auto adminSession = *system.login("admin", "test-admin-password");

    // Non-existent patient
    hms::MedicalRecordDraft badDraft;
    badDraft.patientId = "P-99999";
    badDraft.doctorId = "D-00001";
    badDraft.visitDate = "2026-11-04";
    badDraft.diagnosis = "Condition";

    bool failedNonExistentPatient = false;
    try {
        system.createMedicalRecord(adminSession, badDraft);
    } catch (const std::invalid_argument&) {
        failedNonExistentPatient = true;
    }
    assert(failedNonExistentPatient);

    std::cout << "testMedicalRecordValidation passed\n";
}

void testMedicalRecordJournalPersistence() {
    const auto folder = std::filesystem::temp_directory_path() /
        ("hms-medical-record-test-" + std::to_string(
            std::chrono::steady_clock::now().time_since_epoch().count()));
    const auto journal = folder / "records.journal";
    hms::MedicalRecordDraft draft{"P-00042", "D-00008", "2024-02-29", "cough\tfever",
                                  "Bronchitis", "Rest and fluids", {"Medicine A", "Medicine B"}};
    std::string recordId;
    {
        hms::MedicalRecordRepository repository(journal.string());
        recordId = repository.addRecord(draft).id;
        assert(recordId == "MR-00001");
    }
    {
        hms::MedicalRecordRepository repository(journal.string());
        const auto restored = repository.findById(recordId);
        assert(restored.has_value());
        assert(restored->symptoms == draft.symptoms);
        assert(restored->prescriptions == draft.prescriptions);
        assert(repository.addRecord(draft).id == "MR-00002");
    }
    std::filesystem::remove_all(folder);
}

int main() {
    testMedicalRecordCreationAndRetrieval();
    testMedicalRecordValidation();
    testMedicalRecordJournalPersistence();
    std::cout << "All medical record tests passed successfully.\n";
    return 0;
}
