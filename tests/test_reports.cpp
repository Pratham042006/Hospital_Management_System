#include "hms/hospital_system.hpp"
#include "test_support.hpp"

#include <cassert>
#include <cmath>
#include <iostream>

void testReportsEngine() {
    auto system = makeTestSystem();
    const auto admin = *system.login("admin", "test-admin-password");
    const auto billing = *system.login("billing", "test-billing-password");

    // Register 2 patients
    hms::PatientDraft p1;
    p1.fullName = "Sarah Connor";
    p1.dateOfBirth = "1984-05-12";
    p1.gender = "Female";
    p1.phone = "9876543214";
    p1.email = "sarah@cyberdyne.com";
    p1.address = "Los Angeles, CA";
    p1.emergencyContactName = "John Connor";
    p1.emergencyContactPhone = "9876543215";
    const auto pat1 = system.registerPatient(admin, p1);

    hms::PatientDraft p2;
    p2.fullName = "Kyle Reese";
    p2.dateOfBirth = "2002-08-21";
    p2.gender = "Male";
    p2.phone = "9876543216";
    p2.email = "kyle@resistance.org";
    p2.address = "Tech City";
    p2.emergencyContactName = "Sarah Connor";
    p2.emergencyContactPhone = "9876543217";
    const auto pat2 = system.registerPatient(admin, p2);

    // Add doctor
    hms::DoctorDraft docDraft;
    docDraft.fullName = "Dr. Leonard McCoy";
    docDraft.specialization = "General Surgery";
    docDraft.department = "Surgery";
    docDraft.phone = "9876543218";
    docDraft.email = "mccoy@starfleet.org";
    docDraft.availability = {"2026-11-05 10:00", "2026-11-05 11:00"};
    const auto doc = system.addDoctor(admin, docDraft);

    // Schedule appointment
    hms::AppointmentDraft apptDraft;
    apptDraft.patientId = pat1.id;
    apptDraft.doctorId = doc.id;
    apptDraft.date = "2026-11-05";
    apptDraft.time = "10:00";
    apptDraft.purpose = "Pre-op evaluation";
    system.scheduleAppointment(admin, apptDraft);

    // Add room
    hms::RoomDraft roomDraft;
    roomDraft.id = "ICU-101";
    roomDraft.wardName = "Intensive Care Unit";
    roomDraft.type = hms::RoomType::ICU;
    roomDraft.dailyRate = 1200.00;
    roomDraft.totalBeds = 4;
    const auto room = system.addRoom(admin, roomDraft);

    // Admit patient
    hms::AdmissionDraft admDraft;
    admDraft.patientId = pat1.id;
    admDraft.doctorId = doc.id;
    admDraft.roomId = room.id;
    admDraft.admissionDate = "2026-11-04";
    admDraft.reason = "Trauma recovery";
    system.admitPatient(admin, admDraft);

    // Create bill
    hms::BillDraft billDraft;
    billDraft.patientId = pat1.id;
    billDraft.issueDate = "2026-11-04";
    billDraft.dueDate = "2026-11-18";
    billDraft.items = {
        {hms::BillItemCategory::Consultation, "Surgical Consultation", 300.00},
        {hms::BillItemCategory::RoomCharge, "ICU Bed Daily", 1200.00}
    };
    const auto bill = system.createBill(billing, billDraft);

    // Record partial payment
    hms::PaymentDraft pay;
    pay.billId = bill.id;
    pay.amount = 500.00;
    pay.paymentDate = "2026-11-04";
    pay.method = hms::PaymentMethod::CreditCard;
    system.recordPayment(billing, pay);

    // Generate census report
    const auto census = system.getPatientCensusReport(admin);
    assert(census.totalPatients == 2);
    assert(census.activeInpatients == 1);
    assert(census.dischargedPatients == 0);

    // Generate room occupancy report
    const auto occupancy = system.getRoomOccupancyReport(admin);
    assert(occupancy.totalRooms == 1);
    assert(occupancy.totalBeds == 4);
    assert(occupancy.occupiedBeds == 1);
    assert(occupancy.availableBeds == 3);
    assert(std::abs(occupancy.occupancyRate - 25.00) < 0.01);

    // Generate billing report
    const auto billingReport = system.getBillingRevenueReport(admin);
    assert(billingReport.totalBills == 1);
    assert(std::abs(billingReport.totalBilled - 1500.00) < 0.01);
    assert(std::abs(billingReport.totalCollected - 500.00) < 0.01);
    assert(std::abs(billingReport.totalOutstanding - 1000.00) < 0.01);
    assert(billingReport.partiallyPaidBills == 1);

    // Generate doctor utilization report
    const auto doctorReport = system.getDoctorUtilizationReport(admin);
    assert(doctorReport.doctors.size() == 1);
    assert(doctorReport.doctors[0].appointmentCount == 1);
    assert(doctorReport.doctors[0].activeAdmissionCount == 1);

    std::cout << "testReportsEngine passed\n";
}

int main() {
    testReportsEngine();
    std::cout << "All report tests passed successfully.\n";
    return 0;
}
