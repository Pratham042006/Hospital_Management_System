#pragma once

#include "hms/bill.hpp"

#include <optional>
#include <string>
#include <vector>

namespace hms {

class BillRepository {
public:
    explicit BillRepository(
        const std::optional<std::string>& journalPath = std::nullopt);

    Bill createBill(const BillDraft& draft);
    std::optional<Bill> findBillById(const std::string& billId) const;
    std::vector<Bill> findByPatientId(const std::string& patientId) const;
    std::vector<Bill> listAllBills() const;
    Payment recordPayment(const PaymentDraft& draft);
    std::vector<Payment> listPaymentsForBill(const std::string& billId) const;

    double totalBilled() const noexcept;
    double totalCollected() const noexcept;
    double totalOutstanding() const noexcept;
    std::size_t size() const noexcept;

    static bool isValidDate(const std::string& date);

private:
    std::vector<Bill> bills_;
    std::vector<Payment> payments_;
    std::size_t nextBillId_{1};
    std::size_t nextPaymentId_{1};
    std::optional<std::string> journalPath_;

    std::string generateBillId();
    std::string generatePaymentId();
    void appendBillToJournal(const BillDraft& draft, const Bill& bill) const;
    void appendPaymentToJournal(const PaymentDraft& draft, const Payment& payment) const;
    void loadJournal();
};

}  // namespace hms
