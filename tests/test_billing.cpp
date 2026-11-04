#include "hms/hospital_system.hpp"
#include "hms/bill_repository.hpp"
#include "test_support.hpp"

#include <cassert>
#include <cmath>
#include <iostream>
#include <chrono>
#include <filesystem>

void testBillingCreationAndCalculation() {
    auto system = makeTestSystem();
    const auto adminSession = *system.login("admin", "test-admin-password");
    const auto billingSession = *system.login("billing", "test-billing-password");
    const auto doctorSession = *system.login("doctor", "test-doctor-password");

    // Register patient
    hms::PatientDraft patientDraft;
    patientDraft.fullName = "Robert Paulson";
    patientDraft.dateOfBirth = "1980-01-15";
    patientDraft.gender = "Male";
    patientDraft.phone = "9876543212";
    patientDraft.email = "robert@example.com";
    patientDraft.address = "123 Main St, Springfield";
    patientDraft.emergencyContactName = "Marla Singer";
    patientDraft.emergencyContactPhone = "9876543299";
    const auto patient = system.registerPatient(adminSession, patientDraft);

    // Create bill
    hms::BillDraft billDraft;
    billDraft.patientId = patient.id;
    billDraft.issueDate = "2026-11-04";
    billDraft.dueDate = "2026-11-18";
    billDraft.items = {
        {hms::BillItemCategory::Consultation, "Specialist Consultation Fee", 150.00},
        {hms::BillItemCategory::LabTest, "Comprehensive Metabolic Panel", 85.50},
        {hms::BillItemCategory::Medication, "Prescription Antibiotics", 45.25}
    };

    const auto bill = system.createBill(billingSession, billDraft);
    assert(!bill.id.empty());
    assert(bill.patientId == patient.id);
    assert(std::abs(bill.totalAmount - 280.75) < 0.01);
    assert(std::abs(bill.balance - 280.75) < 0.01);
    assert(bill.paidAmount == 0.0);
    assert(bill.status == hms::BillStatus::Unpaid);

    // Doctor is not allowed to create bills
    bool doctorForbidden = false;
    try {
        system.createBill(doctorSession, billDraft);
    } catch (const hms::AuthorizationError&) {
        doctorForbidden = true;
    }
    assert(doctorForbidden);

    std::cout << "testBillingCreationAndCalculation passed\n";
}

void testPaymentRecordingAndSettlement() {
    auto system = makeTestSystem();
    const auto adminSession = *system.login("admin", "test-admin-password");
    const auto billingSession = *system.login("billing", "test-billing-password");

    hms::PatientDraft patientDraft;
    patientDraft.fullName = "Alice Smith";
    patientDraft.dateOfBirth = "1995-07-20";
    patientDraft.gender = "Female";
    patientDraft.phone = "9876543213";
    patientDraft.email = "alice@example.com";
    patientDraft.address = "456 Oak Avenue";
    patientDraft.emergencyContactName = "Bob Smith";
    patientDraft.emergencyContactPhone = "9876543298";
    const auto patient = system.registerPatient(adminSession, patientDraft);

    hms::BillDraft billDraft;
    billDraft.patientId = patient.id;
    billDraft.issueDate = "2026-11-04";
    billDraft.dueDate = "2026-11-20";
    billDraft.items = {
        {hms::BillItemCategory::Procedure, "Minor Outpatient Procedure", 500.00}
    };
    const auto bill = system.createBill(billingSession, billDraft);

    // Partial payment
    hms::PaymentDraft pay1;
    pay1.billId = bill.id;
    pay1.amount = 200.00;
    pay1.paymentDate = "2026-11-05";
    pay1.method = hms::PaymentMethod::CreditCard;
    pay1.notes = "Initial partial deposit";
    const auto recorded1 = system.recordPayment(billingSession, pay1);
    assert(recorded1.status == hms::PaymentStatus::Completed);

    const auto updatedBill1 = *system.findBillById(billingSession, bill.id);
    assert(std::abs(updatedBill1.paidAmount - 200.00) < 0.01);
    assert(std::abs(updatedBill1.balance - 300.00) < 0.01);
    assert(updatedBill1.status == hms::BillStatus::PartiallyPaid);

    // Overpayment prevention
    hms::PaymentDraft overpay;
    overpay.billId = bill.id;
    overpay.amount = 350.00; // exceeds remaining 300.00
    overpay.paymentDate = "2026-11-05";
    bool overpayRejected = false;
    try {
        system.recordPayment(billingSession, overpay);
    } catch (const std::invalid_argument&) {
        overpayRejected = true;
    }
    assert(overpayRejected);

    // Final settlement payment
    hms::PaymentDraft pay2;
    pay2.billId = bill.id;
    pay2.amount = 300.00;
    pay2.paymentDate = "2026-11-06";
    pay2.method = hms::PaymentMethod::BankTransfer;
    pay2.notes = "Settlement of remaining balance";
    system.recordPayment(billingSession, pay2);

    const auto settledBill = *system.findBillById(billingSession, bill.id);
    assert(settledBill.paidAmount == 500.00);
    assert(settledBill.balance == 0.0);
    assert(settledBill.status == hms::BillStatus::Paid);

    // Check payment history
    const auto payments = system.listPaymentsForBill(billingSession, bill.id);
    assert(payments.size() == 2);
    assert(payments[0].amount == 200.00);
    assert(payments[1].amount == 300.00);

    std::cout << "testPaymentRecordingAndSettlement passed\n";
}

void testBillingJournalPersistence() {
    const auto folder = std::filesystem::temp_directory_path() /
        ("hms-billing-test-" + std::to_string(
            std::chrono::steady_clock::now().time_since_epoch().count()));
    const auto journal = folder / "billing.journal";
    hms::BillDraft draft{"P-00042", std::nullopt, "2026-11-04", "2026-11-18",
                         {{hms::BillItemCategory::LabTest, "Blood panel", 125.50}}};
    std::string billId;
    {
        hms::BillRepository repository(journal.string());
        billId = repository.createBill(draft).id;
        hms::PaymentDraft payment{billId, 25.50, "2026-11-05",
                                  hms::PaymentMethod::Cash, "copay\tnote"};
        assert(repository.recordPayment(payment).paymentId == "PAY-00001");
    }
    {
        hms::BillRepository repository(journal.string());
        const auto restored = repository.findBillById(billId);
        assert(restored.has_value());
        assert(restored->totalAmount == 125.50);
        assert(restored->paidAmount == 25.50);
        assert(restored->balance == 100.00);
        assert(restored->status == hms::BillStatus::PartiallyPaid);
        assert(repository.listPaymentsForBill(billId).front().notes == "copay\tnote");
        assert(repository.totalOutstanding() == 100.00);
    }
    std::filesystem::remove_all(folder);
}

int main() {
    testBillingCreationAndCalculation();
    testPaymentRecordingAndSettlement();
    testBillingJournalPersistence();
    std::cout << "All billing tests passed successfully.\n";
    return 0;
}
