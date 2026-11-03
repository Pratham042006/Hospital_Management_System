/**
 * Hospital Management System - Medical Records & Billing Module
 * Contributor: Pratik Patil (@Pratix005)
 * Requirements: FR-10 (Medical Record Management), FR-12 (Billing), FR-13 (Payment Recording)
 */

window.RecordsBillingModule = (function () {
    let medicalRecords = [];
    let bills = [];
    let payments = [];

    function init() {
        renderRecordsTab();
        renderBillingTab();
    }

    function createMedicalRecord(record) {
        if (!record.patientId || !record.doctorId || !record.diagnosis) {
            throw new Error("Patient ID, Doctor ID, and Diagnosis are required.");
        }
        const newRecord = {
            id: 'MR-' + String(medicalRecords.length + 1).padStart(5, '0'),
            patientId: record.patientId,
            doctorId: record.doctorId,
            visitDate: record.visitDate || new Date().toISOString().slice(0, 10),
            symptoms: record.symptoms || '',
            diagnosis: record.diagnosis,
            treatmentNotes: record.treatmentNotes || '',
            prescriptions: record.prescriptions || [],
            createdAt: new Date().toISOString().slice(0, 10)
        };
        medicalRecords.push(newRecord);
        return newRecord;
    }

    function createBill(billData) {
        if (!billData.patientId || !billData.items || billData.items.length === 0) {
            throw new Error("Patient ID and at least one bill item are required.");
        }
        const total = billData.items.reduce((sum, item) => {
            if (item.amount <= 0) throw new Error("Item amount must be positive.");
            return sum + Number(item.amount);
        }, 0);

        const newBill = {
            id: 'B-' + String(bills.length + 1).padStart(5, '0'),
            patientId: billData.patientId,
            admissionId: billData.admissionId || null,
            issueDate: billData.issueDate || new Date().toISOString().slice(0, 10),
            dueDate: billData.dueDate || new Date(Date.now() + 14 * 86400000).toISOString().slice(0, 10),
            items: billData.items,
            totalAmount: Math.round(total * 100) / 100,
            paidAmount: 0.0,
            balance: Math.round(total * 100) / 100,
            status: 'Unpaid'
        };
        bills.push(newBill);
        return newBill;
    }

    function recordPayment(paymentData) {
        const bill = bills.find(b => b.id === paymentData.billId);
        if (!bill) throw new Error("Bill ID not found: " + paymentData.billId);
        const amount = Number(paymentData.amount);
        if (amount <= 0) throw new Error("Payment amount must be greater than zero.");
        if (amount > (bill.balance + 0.01)) throw new Error("Payment exceeds outstanding balance.");

        const payment = {
            paymentId: 'PAY-' + String(payments.length + 1).padStart(5, '0'),
            billId: bill.id,
            amount: Math.round(amount * 100) / 100,
            paymentDate: paymentData.paymentDate || new Date().toISOString().slice(0, 10),
            method: paymentData.method || 'Cash',
            status: 'Completed',
            notes: paymentData.notes || ''
        };

        bill.paidAmount = Math.round((bill.paidAmount + payment.amount) * 100) / 100;
        bill.balance = Math.round((bill.totalAmount - bill.paidAmount) * 100) / 100;
        if (bill.balance <= 0.01) {
            bill.balance = 0.0;
            bill.status = 'Paid';
        } else {
            bill.status = 'Partially Paid';
        }

        payments.push(payment);
        return payment;
    }

    function renderRecordsTab() {
        const container = document.getElementById('medical-records-view');
        if (!container) return;
        container.innerHTML = `
            <div class="card header-card">
                <div class="card-title-row">
                    <h3><i class="fas fa-file-medical"></i> Clinical Medical Records</h3>
                    <button class="btn btn-primary" onclick="window.RecordsBillingModule.openAddRecordModal()">
                        <i class="fas fa-plus"></i> Add Medical Record
                    </button>
                </div>
                <p class="subtitle">Record diagnosis, prescriptions, and consult notes compliant with FR-10.</p>
            </div>
            <div class="card table-card">
                <table class="data-table" id="records-table">
                    <thead>
                        <tr>
                            <th>Record ID</th>
                            <th>Patient ID</th>
                            <th>Doctor ID</th>
                            <th>Visit Date</th>
                            <th>Diagnosis</th>
                            <th>Prescriptions</th>
                            <th>Actions</th>
                        </tr>
                    </thead>
                    <tbody id="records-table-body">
                        ${medicalRecords.map(r => `
                            <tr>
                                <td><strong>${r.id}</strong></td>
                                <td>${r.patientId}</td>
                                <td>${r.doctorId}</td>
                                <td>${r.visitDate}</td>
                                <td><span class="badge badge-diagnosis">${r.diagnosis}</span></td>
                                <td>${r.prescriptions.join(', ')}</td>
                                <td>
                                    <button class="btn btn-sm btn-outline" onclick="window.RecordsBillingModule.viewRecordDetails('${r.id}')">View Notes</button>
                                </td>
                            </tr>
                        `).join('')}
                    </tbody>
                </table>
            </div>
        `;
    }

    function renderBillingTab() {
        const container = document.getElementById('billing-view');
        if (!container) return;
        const totalBilled = bills.reduce((acc, b) => acc + b.totalAmount, 0);
        const totalCollected = bills.reduce((acc, b) => acc + b.paidAmount, 0);
        const totalOutstanding = bills.reduce((acc, b) => acc + b.balance, 0);

        container.innerHTML = `
            <div class="stats-grid">
                <div class="stat-card">
                    <div class="stat-label">Total Invoiced</div>
                    <div class="stat-value">$${totalBilled.toFixed(2)}</div>
                </div>
                <div class="stat-card stat-success">
                    <div class="stat-label">Total Collected</div>
                    <div class="stat-value">$${totalCollected.toFixed(2)}</div>
                </div>
                <div class="stat-card stat-warning">
                    <div class="stat-label">Outstanding Balance</div>
                    <div class="stat-value">$${totalOutstanding.toFixed(2)}</div>
                </div>
            </div>
            <div class="card header-card">
                <div class="card-title-row">
                    <h3><i class="fas fa-file-invoice-dollar"></i> Invoicing & Payments</h3>
                    <button class="btn btn-primary" onclick="window.RecordsBillingModule.openCreateBillModal()">
                        <i class="fas fa-plus"></i> Generate Bill
                    </button>
                </div>
            </div>
            <div class="card table-card">
                <table class="data-table" id="bills-table">
                    <thead>
                        <tr>
                            <th>Bill ID</th>
                            <th>Patient ID</th>
                            <th>Issue Date</th>
                            <th>Total</th>
                            <th>Paid</th>
                            <th>Balance</th>
                            <th>Status</th>
                            <th>Actions</th>
                        </tr>
                    </thead>
                    <tbody id="bills-table-body">
                        ${bills.map(b => `
                            <tr>
                                <td><strong>${b.id}</strong></td>
                                <td>${b.patientId}</td>
                                <td>${b.issueDate}</td>
                                <td>$${b.totalAmount.toFixed(2)}</td>
                                <td>$${b.paidAmount.toFixed(2)}</td>
                                <td>$${b.balance.toFixed(2)}</td>
                                <td><span class="badge badge-${b.status.toLowerCase().replace(' ', '-')}">${b.status}</span></td>
                                <td>
                                    ${b.status !== 'Paid' ? `
                                        <button class="btn btn-sm btn-success" onclick="window.RecordsBillingModule.openPaymentModal('${b.id}')">
                                            Pay
                                        </button>
                                    ` : '<span class="text-muted">Settled</span>'}
                                </td>
                            </tr>
                        `).join('')}
                    </tbody>
                </table>
            </div>
        `;
    }

    return {
        init,
        createMedicalRecord,
        createBill,
        recordPayment,
        renderRecordsTab,
        renderBillingTab,
        getMedicalRecords: () => medicalRecords,
        getBills: () => bills,
        getPayments: () => payments
    };
})();
