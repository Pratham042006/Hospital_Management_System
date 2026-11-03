#include "hms/hospital_system.hpp"
#include "test_support.hpp"

#include <cassert>
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

int main() {
    testMedicalRecordCreationAndRetrieval();
    testMedicalRecordValidation();
    std::cout << "All medical record tests passed successfully.\n";
    return 0;
}
