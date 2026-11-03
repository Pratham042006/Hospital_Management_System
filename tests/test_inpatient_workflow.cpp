#include "hms/hospital_system.hpp"
#include "test_support.hpp"

#include <cassert>
#include <iostream>

void testInpatientEndToEndWorkflow() {
    auto system = makeTestSystem();
    const auto admin = *system.login("admin", "test-admin-password");

    // Add Doctor
    hms::DoctorDraft docDraft;
    docDraft.fullName = "Dr. Beverly Crusher";
    docDraft.specialization = "Internal Medicine";
    docDraft.department = "Sickbay";
    docDraft.phone = "9876543240";
    docDraft.email = "crusher@enterprise.org";
    docDraft.availability = {"2026-11-06 09:00", "2026-11-06 10:00"};
    const auto doc = system.addDoctor(admin, docDraft);

    // Register 2 Patients
    hms::PatientDraft p1;
    p1.fullName = "Jean-Luc Picard";
    p1.dateOfBirth = "1960-07-13";
    p1.gender = "Male";
    p1.phone = "9876543241";
    p1.email = "picard@starfleet.org";
    p1.address = "La Barre, France";
    p1.emergencyContactName = "Robert Picard";
    p1.emergencyContactPhone = "9876543242";
    const auto pat1 = system.registerPatient(admin, p1);

    hms::PatientDraft p2;
    p2.fullName = "William Riker";
    p2.dateOfBirth = "1964-08-19";
    p2.gender = "Male";
    p2.phone = "9876543243";
    p2.email = "riker@starfleet.org";
    p2.address = "Valdez, Alaska";
    p2.emergencyContactName = "Kyle Riker";
    p2.emergencyContactPhone = "9876543244";
    const auto pat2 = system.registerPatient(admin, p2);

    // Add room with 1 bed (Semi-Private)
    hms::RoomDraft roomDraft;
    roomDraft.id = "WARD-201";
    roomDraft.wardName = "Cardiology Observation";
    roomDraft.type = hms::RoomType::SemiPrivate;
    roomDraft.dailyRate = 800.00;
    roomDraft.totalBeds = 1;
    system.addRoom(admin, roomDraft);

    // 1. Admit patient 1
    hms::AdmissionDraft admDraft1;
    admDraft1.patientId = pat1.id;
    admDraft1.doctorId = doc.id;
    admDraft1.roomId = "WARD-201";
    admDraft1.admissionDate = "2026-11-06";
    admDraft1.reason = "Cardiac arrhythmia evaluation";
    const auto adm1 = system.admitPatient(admin, admDraft1);
    assert(adm1.status == hms::AdmissionStatus::Admitted);

    // Check room is now full
    const auto roomAfterAdm1 = system.findRoomById(admin, "WARD-201");
    assert(roomAfterAdm1.has_value());
    assert(roomAfterAdm1->occupiedBeds == 1);
    assert(!roomAfterAdm1->hasAvailableBed());

    // 2. Try admitting patient 2 into full room -> must fail
    hms::AdmissionDraft admDraft2;
    admDraft2.patientId = pat2.id;
    admDraft2.doctorId = doc.id;
    admDraft2.roomId = "WARD-201";
    admDraft2.admissionDate = "2026-11-06";
    admDraft2.reason = "Observation";

    bool roomFullRejected = false;
    try {
        system.admitPatient(admin, admDraft2);
    } catch (const std::invalid_argument&) {
        roomFullRejected = true;
    }
    assert(roomFullRejected);

    // 3. Discharge patient 1
    const auto dischargedAdm1 = system.dischargePatient(
        admin, adm1.id, "2026-11-07", "Stabilized and discharged with medication.");
    assert(dischargedAdm1.status == hms::AdmissionStatus::Discharged);

    // Check room has available bed again
    const auto roomAfterDischarge = system.findRoomById(admin, "WARD-201");
    assert(roomAfterDischarge->occupiedBeds == 0);
    assert(roomAfterDischarge->hasAvailableBed());

    // 4. Now admit patient 2 -> succeeds!
    const auto adm2 = system.admitPatient(admin, admDraft2);
    assert(adm2.status == hms::AdmissionStatus::Admitted);

    // Verify consistency
    assert(system.checkDataConsistency(admin));

    std::cout << "testInpatientEndToEndWorkflow passed\n";
}

int main() {
    testInpatientEndToEndWorkflow();
    std::cout << "All inpatient workflow tests passed successfully.\n";
    return 0;
}
