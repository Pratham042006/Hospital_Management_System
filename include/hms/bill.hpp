#pragma once

#include <optional>
#include <string>
#include <vector>

namespace hms {

enum class BillItemCategory {
    Consultation,
    RoomCharge,
    Procedure,
    Medication,
    LabTest,
    Other
};

std::string toString(BillItemCategory category);
std::optional<BillItemCategory> parseBillCategory(const std::string& str);

struct BillItem {
    BillItemCategory category{BillItemCategory::Other};
    std::string description;
    double amount{0.0};
};

enum class BillStatus {
    Unpaid,
    PartiallyPaid,
    Paid,
    Cancelled
};

std::string toString(BillStatus status);

struct BillDraft {
    std::string patientId;
    std::optional<std::string> admissionId;
    std::string issueDate;       // YYYY-MM-DD
    std::string dueDate;         // YYYY-MM-DD
    std::vector<BillItem> items;
};

enum class PaymentMethod {
    Cash,
    CreditCard,
    DebitCard,
    Insurance,
    BankTransfer
};

std::string toString(PaymentMethod method);
std::optional<PaymentMethod> parsePaymentMethod(const std::string& str);

enum class PaymentStatus {
    Completed,
    Pending,
    Failed
};

std::string toString(PaymentStatus status);

struct PaymentDraft {
    std::string billId;
    double amount{0.0};
    std::string paymentDate;     // YYYY-MM-DD
    PaymentMethod method{PaymentMethod::Cash};
    std::string notes;
};

struct Payment {
    std::string paymentId;       // e.g. "PAY-00001"
    std::string billId;
    double amount{0.0};
    std::string paymentDate;
    PaymentMethod method{PaymentMethod::Cash};
    PaymentStatus status{PaymentStatus::Completed};
    std::string notes;
};

struct Bill {
    std::string id;              // e.g. "B-00001"
    std::string patientId;
    std::optional<std::string> admissionId;
    std::string issueDate;
    std::string dueDate;
    std::vector<BillItem> items;
    double totalAmount{0.0};
    double paidAmount{0.0};
    double balance{0.0};
    BillStatus status{BillStatus::Unpaid};
    std::vector<Payment> payments;
};

}  // namespace hms
