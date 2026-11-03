#include "hms/bill_repository.hpp"

#include <cmath>
#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace hms {

std::string toString(BillItemCategory category) {
    switch (category) {
        case BillItemCategory::Consultation: return "Consultation";
        case BillItemCategory::RoomCharge:   return "Room Charge";
        case BillItemCategory::Procedure:    return "Procedure";
        case BillItemCategory::Medication:   return "Medication";
        case BillItemCategory::LabTest:      return "Lab Test";
        case BillItemCategory::Other:        return "Other";
    }
    return "Other";
}

std::optional<BillItemCategory> parseBillCategory(const std::string& str) {
    if (str == "Consultation") return BillItemCategory::Consultation;
    if (str == "Room Charge" || str == "RoomCharge") return BillItemCategory::RoomCharge;
    if (str == "Procedure") return BillItemCategory::Procedure;
    if (str == "Medication") return BillItemCategory::Medication;
    if (str == "Lab Test" || str == "LabTest") return BillItemCategory::LabTest;
    if (str == "Other") return BillItemCategory::Other;
    return std::nullopt;
}

std::string toString(BillStatus status) {
    switch (status) {
        case BillStatus::Unpaid:        return "Unpaid";
        case BillStatus::PartiallyPaid: return "Partially Paid";
        case BillStatus::Paid:          return "Paid";
        case BillStatus::Cancelled:     return "Cancelled";
    }
    return "Unknown";
}

std::string toString(PaymentMethod method) {
    switch (method) {
        case PaymentMethod::Cash:         return "Cash";
        case PaymentMethod::CreditCard:   return "Credit Card";
        case PaymentMethod::DebitCard:    return "Debit Card";
        case PaymentMethod::Insurance:    return "Insurance";
        case PaymentMethod::BankTransfer: return "Bank Transfer";
    }
    return "Cash";
}

std::optional<PaymentMethod> parsePaymentMethod(const std::string& str) {
    if (str == "Cash") return PaymentMethod::Cash;
    if (str == "Credit Card" || str == "CreditCard") return PaymentMethod::CreditCard;
    if (str == "Debit Card" || str == "DebitCard") return PaymentMethod::DebitCard;
    if (str == "Insurance") return PaymentMethod::Insurance;
    if (str == "Bank Transfer" || str == "BankTransfer") return PaymentMethod::BankTransfer;
    return std::nullopt;
}

std::string toString(PaymentStatus status) {
    switch (status) {
        case PaymentStatus::Completed: return "Completed";
        case PaymentStatus::Pending:   return "Pending";
        case PaymentStatus::Failed:    return "Failed";
    }
    return "Completed";
}

bool BillRepository::isValidDate(const std::string& date) {
    if (date.size() != 10 || date[4] != '-' || date[7] != '-') {
        return false;
    }
    try {
        int year = std::stoi(date.substr(0, 4));
        int month = std::stoi(date.substr(5, 2));
        int day = std::stoi(date.substr(8, 2));
        if (year < 1900 || year > 2100 || month < 1 || month > 12 || day < 1 || day > 31) {
            return false;
        }
    } catch (...) {
        return false;
    }
    return true;
}

std::string BillRepository::generateBillId() {
    std::ostringstream stream;
    stream << "B-" << std::setw(5) << std::setfill('0') << nextBillId_++;
    return stream.str();
}

std::string BillRepository::generatePaymentId() {
    std::ostringstream stream;
    stream << "PAY-" << std::setw(5) << std::setfill('0') << nextPaymentId_++;
    return stream.str();
}

Bill BillRepository::createBill(const BillDraft& draft) {
    if (draft.patientId.empty()) {
        throw std::invalid_argument("Patient ID is required to create a bill.");
    }
    if (!isValidDate(draft.issueDate)) {
        throw std::invalid_argument("Issue date must be in YYYY-MM-DD format.");
    }
    if (!isValidDate(draft.dueDate)) {
        throw std::invalid_argument("Due date must be in YYYY-MM-DD format.");
    }
    if (draft.items.empty()) {
        throw std::invalid_argument("A bill must contain at least one line item.");
    }

    double total = 0.0;
    for (const auto& item : draft.items) {
        if (item.description.empty()) {
            throw std::invalid_argument("Bill item description cannot be empty.");
        }
        if (item.amount <= 0.0) {
            throw std::invalid_argument("Bill item amount must be strictly greater than zero.");
        }
        total += item.amount;
    }

    Bill bill;
    bill.id = generateBillId();
    bill.patientId = draft.patientId;
    bill.admissionId = draft.admissionId;
    bill.issueDate = draft.issueDate;
    bill.dueDate = draft.dueDate;
    bill.items = draft.items;
    bill.totalAmount = std::round(total * 100.0) / 100.0;
    bill.paidAmount = 0.0;
    bill.balance = bill.totalAmount;
    bill.status = BillStatus::Unpaid;

    bills_.push_back(bill);
    return bill;
}

std::optional<Bill> BillRepository::findBillById(const std::string& billId) const {
    for (const auto& bill : bills_) {
        if (bill.id == billId) {
            return bill;
        }
    }
    return std::nullopt;
}

std::vector<Bill> BillRepository::findByPatientId(const std::string& patientId) const {
    std::vector<Bill> matches;
    for (const auto& bill : bills_) {
        if (bill.patientId == patientId) {
            matches.push_back(bill);
        }
    }
    return matches;
}

std::vector<Bill> BillRepository::listAllBills() const {
    return bills_;
}

Payment BillRepository::recordPayment(const PaymentDraft& draft) {
    if (draft.billId.empty()) {
        throw std::invalid_argument("Bill ID is required to record a payment.");
    }
    if (draft.amount <= 0.0) {
        throw std::invalid_argument("Payment amount must be greater than zero.");
    }
    if (!isValidDate(draft.paymentDate)) {
        throw std::invalid_argument("Payment date must be in YYYY-MM-DD format.");
    }

    Bill* targetBill = nullptr;
    for (auto& b : bills_) {
        if (b.id == draft.billId) {
            targetBill = &b;
            break;
        }
    }

    if (!targetBill) {
        throw std::invalid_argument("Bill ID does not exist: " + draft.billId);
    }

    if (targetBill->status == BillStatus::Paid) {
        throw std::invalid_argument("Bill is already fully paid.");
    }

    if (draft.amount > (targetBill->balance + 0.001)) {
        throw std::invalid_argument("Payment amount exceeds outstanding balance of " +
                                    std::to_string(targetBill->balance));
    }

    Payment payment;
    payment.paymentId = generatePaymentId();
    payment.billId = draft.billId;
    payment.amount = std::round(draft.amount * 100.0) / 100.0;
    payment.paymentDate = draft.paymentDate;
    payment.method = draft.method;
    payment.status = PaymentStatus::Completed;
    payment.notes = draft.notes;

    targetBill->paidAmount = std::round((targetBill->paidAmount + payment.amount) * 100.0) / 100.0;
    targetBill->balance = std::round((targetBill->totalAmount - targetBill->paidAmount) * 100.0) / 100.0;
    if (targetBill->balance <= 0.001) {
        targetBill->balance = 0.0;
        targetBill->status = BillStatus::Paid;
    } else {
        targetBill->status = BillStatus::PartiallyPaid;
    }
    targetBill->payments.push_back(payment);
    payments_.push_back(payment);

    return payment;
}

std::vector<Payment> BillRepository::listPaymentsForBill(const std::string& billId) const {
    std::vector<Payment> billPayments;
    for (const auto& payment : payments_) {
        if (payment.billId == billId) {
            billPayments.push_back(payment);
        }
    }
    return billPayments;
}

double BillRepository::totalBilled() const noexcept {
    double total = 0.0;
    for (const auto& b : bills_) {
        total += b.totalAmount;
    }
    return total;
}

double BillRepository::totalCollected() const noexcept {
    double collected = 0.0;
    for (const auto& b : bills_) {
        collected += b.paidAmount;
    }
    return collected;
}

double BillRepository::totalOutstanding() const noexcept {
    double outstanding = 0.0;
    for (const auto& b : bills_) {
        outstanding += b.balance;
    }
    return outstanding;
}

std::size_t BillRepository::size() const noexcept {
    return bills_.size();
}

}  // namespace hms
