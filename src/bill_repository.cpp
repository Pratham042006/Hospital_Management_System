#include "hms/bill_repository.hpp"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <limits>
#include <sstream>
#include <stdexcept>

namespace hms {

namespace {

std::string hexEncode(const std::string& value) {
    static constexpr char digits[] = "0123456789ABCDEF";
    std::string encoded;
    encoded.reserve(value.size() * 2);
    for (const unsigned char ch : value) {
        encoded.push_back(digits[ch >> 4]);
        encoded.push_back(digits[ch & 0x0F]);
    }
    return encoded;
}

int hexValue(char value) {
    if (value >= '0' && value <= '9') return value - '0';
    if (value >= 'A' && value <= 'F') return value - 'A' + 10;
    if (value >= 'a' && value <= 'f') return value - 'a' + 10;
    return -1;
}

std::string hexDecode(const std::string& value) {
    if (value.size() % 2 != 0) throw std::runtime_error("Malformed billing journal field.");
    std::string decoded;
    decoded.reserve(value.size() / 2);
    for (std::size_t index = 0; index < value.size(); index += 2) {
        const int high = hexValue(value[index]);
        const int low = hexValue(value[index + 1]);
        if (high < 0 || low < 0) throw std::runtime_error("Malformed billing journal field.");
        decoded.push_back(static_cast<char>((high << 4) | low));
    }
    return decoded;
}

std::vector<std::string> splitTabs(const std::string& line) {
    std::vector<std::string> fields;
    std::istringstream input(line);
    std::string field;
    while (std::getline(input, field, '\t')) fields.push_back(field);
    return fields;
}

std::size_t parseCount(const std::string& value, std::size_t lineNumber) {
    try {
        std::size_t consumed = 0;
        const auto count = std::stoull(value, &consumed);
        if (consumed != value.size() || count > 10000) throw std::runtime_error("bad count");
        return static_cast<std::size_t>(count);
    } catch (const std::exception&) {
        throw std::runtime_error("Malformed billing journal item count at line " +
                                 std::to_string(lineNumber) + '.');
    }
}

double parseAmount(const std::string& value, std::size_t lineNumber) {
    try {
        std::size_t consumed = 0;
        const double amount = std::stod(value, &consumed);
        if (consumed != value.size() || !std::isfinite(amount)) throw std::runtime_error("bad amount");
        return amount;
    } catch (const std::exception&) {
        throw std::runtime_error("Malformed billing amount at line " +
                                 std::to_string(lineNumber) + '.');
    }
}

void appendLine(const std::string& journalPath, const std::string& line) {
    const std::filesystem::path path(journalPath);
    const auto parent = path.parent_path();
    std::error_code error;
    if (!parent.empty()) std::filesystem::create_directories(parent, error);
    if (error) throw std::runtime_error("Unable to create billing storage directory: " + error.message());

    std::ofstream output(path, std::ios::app | std::ios::binary);
    if (!output) throw std::runtime_error("Unable to open billing journal for writing.");
    output << line << '\n';
    output.flush();
    if (!output) throw std::runtime_error("Unable to save the billing journal.");
}

}  // namespace

BillRepository::BillRepository(const std::optional<std::string>& journalPath)
    : journalPath_(journalPath) {
    if (journalPath_) loadJournal();
}

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
        if (year < 1900 || year > 2100 || month < 1 || month > 12 || day < 1) {
            return false;
        }
        static constexpr int daysInMonth[] = {31, 28, 31, 30, 31, 30,
                                               31, 31, 30, 31, 30, 31};
        const bool leapYear = (year % 4 == 0 && year % 100 != 0) || year % 400 == 0;
        const int maximumDay = daysInMonth[month - 1] + (month == 2 && leapYear ? 1 : 0);
        if (day > maximumDay) return false;
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
    if (draft.dueDate < draft.issueDate) {
        throw std::invalid_argument("Due date cannot be earlier than the issue date.");
    }
    if (draft.items.empty()) {
        throw std::invalid_argument("A bill must contain at least one line item.");
    }

    double total = 0.0;
    std::vector<BillItem> normalizedItems;
    normalizedItems.reserve(draft.items.size());
    for (const auto& item : draft.items) {
        if (item.description.empty()) {
            throw std::invalid_argument("Bill item description cannot be empty.");
        }
        if (!std::isfinite(item.amount) || item.amount <= 0.0) {
            throw std::invalid_argument("Bill item amount must be strictly greater than zero.");
        }
        if (item.amount > std::numeric_limits<double>::max() / 100.0) {
            throw std::invalid_argument("Bill item amount is too large.");
        }
        const double roundedAmount = std::round(item.amount * 100.0) / 100.0;
        if (roundedAmount < 0.01) {
            throw std::invalid_argument("Bill item amount must be at least one cent.");
        }
        normalizedItems.push_back({item.category, item.description, roundedAmount});
        total += roundedAmount;
        if (!std::isfinite(total)) throw std::invalid_argument("Bill total is too large.");
    }

    Bill bill;
    bill.id = generateBillId();
    bill.patientId = draft.patientId;
    bill.admissionId = draft.admissionId;
    bill.issueDate = draft.issueDate;
    bill.dueDate = draft.dueDate;
    bill.items = std::move(normalizedItems);
    if (total > std::numeric_limits<double>::max() / 100.0) {
        throw std::invalid_argument("Bill total is too large.");
    }
    bill.totalAmount = std::round(total * 100.0) / 100.0;
    bill.paidAmount = 0.0;
    bill.balance = bill.totalAmount;
    bill.status = BillStatus::Unpaid;

    appendBillToJournal(draft, bill);
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
    if (!std::isfinite(draft.amount) || draft.amount <= 0.0) {
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
    if (std::round(draft.amount * 100.0) < 1.0) {
        throw std::invalid_argument("Payment amount must be at least one cent.");
    }

    Payment payment;
    payment.paymentId = generatePaymentId();
    payment.billId = draft.billId;
    if (draft.amount > std::numeric_limits<double>::max() / 100.0) {
        throw std::invalid_argument("Payment amount is too large.");
    }
    payment.amount = std::round(draft.amount * 100.0) / 100.0;
    payment.paymentDate = draft.paymentDate;
    payment.method = draft.method;
    payment.status = PaymentStatus::Completed;
    payment.notes = draft.notes;

    appendPaymentToJournal(draft, payment);
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

void BillRepository::appendBillToJournal(const BillDraft& draft, const Bill& bill) const {
    if (!journalPath_) return;
    std::ostringstream line;
    line << "BILL\t" << hexEncode(bill.id) << '\t' << hexEncode(draft.patientId) << '\t'
         << hexEncode(draft.admissionId.value_or("")) << '\t' << hexEncode(draft.issueDate) << '\t'
         << hexEncode(draft.dueDate) << '\t' << draft.items.size();
    for (const auto& item : bill.items) {
        line << '\t' << hexEncode(toString(item.category)) << '\t' << hexEncode(item.description)
             << '\t' << std::fixed << std::setprecision(2) << item.amount;
    }
    appendLine(*journalPath_, line.str());
}

void BillRepository::appendPaymentToJournal(const PaymentDraft& draft,
                                            const Payment& payment) const {
    if (!journalPath_) return;
    std::ostringstream line;
    line << "PAYMENT\t" << hexEncode(payment.paymentId) << '\t' << hexEncode(draft.billId) << '\t'
         << std::fixed << std::setprecision(2) << payment.amount << '\t'
         << hexEncode(draft.paymentDate) << '\t' << hexEncode(toString(draft.method)) << '\t'
         << hexEncode(draft.notes);
    appendLine(*journalPath_, line.str());
}

void BillRepository::loadJournal() {
    const std::filesystem::path path(*journalPath_);
    if (!std::filesystem::exists(path)) return;
    std::ifstream input(path, std::ios::binary);
    if (!input) throw std::runtime_error("Unable to open billing journal for reading.");

    // Replay events through the same validation and balance-update paths used for live writes.
    const auto journalPath = journalPath_;
    journalPath_.reset();
    std::string line;
    std::size_t lineNumber = 0;
    while (std::getline(input, line)) {
        ++lineNumber;
        const auto fields = splitTabs(line);
        if (fields.empty()) {
            throw std::runtime_error("Malformed billing journal entry at line " +
                                     std::to_string(lineNumber) + '.');
        }
        if (fields[0] == "BILL") {
            if (fields.size() < 7) {
                throw std::runtime_error("Malformed bill journal entry at line " +
                                         std::to_string(lineNumber) + '.');
            }
            const std::size_t itemCount = parseCount(fields[6], lineNumber);
            if (fields.size() != 7 + itemCount * 3) {
                throw std::runtime_error("Malformed bill items in journal at line " +
                                         std::to_string(lineNumber) + '.');
            }
            BillDraft draft;
            draft.patientId = hexDecode(fields[2]);
            const std::string admissionId = hexDecode(fields[3]);
            if (!admissionId.empty()) draft.admissionId = admissionId;
            draft.issueDate = hexDecode(fields[4]);
            draft.dueDate = hexDecode(fields[5]);
            for (std::size_t index = 0; index < itemCount; ++index) {
                const auto offset = 7 + index * 3;
                const auto category = parseBillCategory(hexDecode(fields[offset]));
                if (!category) {
                    throw std::runtime_error("Unknown bill category in journal at line " +
                                             std::to_string(lineNumber) + '.');
                }
                draft.items.push_back({*category, hexDecode(fields[offset + 1]),
                                       parseAmount(fields[offset + 2], lineNumber)});
            }
            const Bill restored = createBill(draft);
            if (restored.id != hexDecode(fields[1])) {
                throw std::runtime_error("Unexpected bill ID sequence in journal at line " +
                                         std::to_string(lineNumber) + '.');
            }
        } else if (fields[0] == "PAYMENT") {
            if (fields.size() != 7) {
                throw std::runtime_error("Malformed payment journal entry at line " +
                                         std::to_string(lineNumber) + '.');
            }
            PaymentDraft draft;
            draft.billId = hexDecode(fields[2]);
            draft.amount = parseAmount(fields[3], lineNumber);
            draft.paymentDate = hexDecode(fields[4]);
            const auto method = parsePaymentMethod(hexDecode(fields[5]));
            if (!method) {
                throw std::runtime_error("Unknown payment method in journal at line " +
                                         std::to_string(lineNumber) + '.');
            }
            draft.method = *method;
            draft.notes = hexDecode(fields[6]);
            const Payment restored = recordPayment(draft);
            if (restored.paymentId != hexDecode(fields[1])) {
                throw std::runtime_error("Unexpected payment ID sequence in journal at line " +
                                         std::to_string(lineNumber) + '.');
            }
        } else {
            throw std::runtime_error("Unknown billing journal event at line " +
                                     std::to_string(lineNumber) + '.');
        }
    }
    if (!input.eof()) throw std::runtime_error("Unable to finish reading the billing journal.");
    journalPath_ = journalPath;
}

}  // namespace hms
