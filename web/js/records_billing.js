/**
 * Hospital Management System - Medical Records & Billing Module
 * Contributor: Pratik Patil (@Pratix005)
 * Requirements: FR-10 (Medical Record Management), FR-12 (Billing), FR-13 (Payment Recording)
 */

window.RecordsBillingModule = (function () {
    let medicalRecords = [];
    let bills = [];
    let payments = [];

    function escapeHtml(value) {
        return String(value ?? '').replace(/[&<>"']/g, character => ({
            '&': '&amp;', '<': '&lt;', '>': '&gt;', '"': '&quot;', "'": '&#39;'
        })[character]);
    }

    function validDate(value) {
        if (!/^\d{4}-\d{2}-\d{2}$/.test(value)) return false;
        const date = new Date(`${value}T00:00:00Z`);
        return !Number.isNaN(date.getTime()) && date.toISOString().slice(0, 10) === value;
    }

    function init() {
        renderRecordsTab();
        renderBillingTab();
    }

    function createMedicalRecord(record) {
        if (!record.patientId || !record.doctorId || !record.diagnosis ||
            !validDate(record.visitDate || new Date().toISOString().slice(0, 10))) {
            throw new Error("Patient ID, Doctor ID, Diagnosis, and a valid visit date are required.");
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
        renderRecordsTab();
        return newRecord;
    }

    function createBill(billData) {
        if (!billData.patientId || !billData.items || billData.items.length === 0) {
            throw new Error("Patient ID and at least one bill item are required.");
        }
        const issueDate = billData.issueDate || new Date().toISOString().slice(0, 10);
        const dueDate = billData.dueDate || new Date(Date.now() + 14 * 86400000).toISOString().slice(0, 10);
        if (!validDate(issueDate) || !validDate(dueDate)) throw new Error("Bill dates must use YYYY-MM-DD.");
        if (billData.dueDate && (!validDate(billData.dueDate) ||
            dueDate < issueDate)) {
            throw new Error("Due date must be valid and cannot precede the issue date.");
        }
        const total = billData.items.reduce((sum, item) => {
            if (!String(item.description || '').trim()) throw new Error("Item description is required.");
            const amount = Number(item.amount);
            if (!Number.isFinite(amount) || amount <= 0) throw new Error("Item amount must be a positive number.");
            return sum + amount;
        }, 0);
        if (!Number.isFinite(total)) throw new Error("Bill total is too large.");

        const newBill = {
            id: 'B-' + String(bills.length + 1).padStart(5, '0'),
            patientId: billData.patientId,
            admissionId: billData.admissionId || null,
            issueDate,
            dueDate,
            items: billData.items,
            totalAmount: Math.round(total * 100) / 100,
            paidAmount: 0.0,
            balance: Math.round(total * 100) / 100,
            status: 'Unpaid'
        };
        bills.push(newBill);
        renderBillingTab();
        return newBill;
    }

    function recordPayment(paymentData) {
        const bill = bills.find(b => b.id === paymentData.billId);
        if (!bill) throw new Error("Bill ID not found: " + paymentData.billId);
        const amount = Number(paymentData.amount);
        if (!Number.isFinite(amount) || amount <= 0) throw new Error("Payment amount must be a positive number.");
        if (!validDate(paymentData.paymentDate || new Date().toISOString().slice(0, 10))) {
            throw new Error("Payment date must be YYYY-MM-DD.");
        }
        if (!['Cash', 'Credit Card', 'Debit Card', 'Insurance', 'Bank Transfer'].includes(paymentData.method || 'Cash')) {
            throw new Error("Select a supported payment method.");
        }
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
        renderBillingTab();
        return payment;
    }

    function openAddRecordModal() {
        const patientId = prompt('Patient ID:');
        if (patientId === null) return;
        const doctorId = prompt('Doctor ID:');
        if (doctorId === null) return;
        const visitDate = prompt('Visit date (YYYY-MM-DD):', new Date().toISOString().slice(0, 10));
        if (visitDate === null) return;
        const diagnosis = prompt('Diagnosis:');
        if (diagnosis === null) return;
        try {
            createMedicalRecord({ patientId, doctorId, visitDate, diagnosis,
                symptoms: prompt('Symptoms (optional):') || '',
                treatmentNotes: prompt('Treatment notes (optional):') || '',
                prescriptions: (prompt('Prescriptions, separated by commas (optional):') || '')
                    .split(',').map(item => item.trim()).filter(Boolean) });
            renderRecordsTab();
        } catch (error) { alert(error.message); }
    }

    function viewRecordDetails(recordId) {
        const record = medicalRecords.find(item => item.id === recordId);
        if (!record) return;
        alert(`Diagnosis: ${record.diagnosis}\nSymptoms: ${record.symptoms || '—'}\n` +
            `Treatment: ${record.treatmentNotes || '—'}\nPrescriptions: ${record.prescriptions.join(', ') || '—'}`);
    }

    function openCreateBillModal() {
        const patientId = prompt('Patient ID:');
        if (patientId === null) return;
        const description = prompt('Bill item description:');
        if (description === null) return;
        const amount = Number(prompt('Amount:'));
        try {
            createBill({ patientId, items: [{ category: 'Other', description, amount }] });
        } catch (error) { alert(error.message); }
    }

    function openPaymentModal(billId) {
        const amount = Number(prompt('Payment amount:'));
        if (!Number.isFinite(amount) || amount <= 0) return;
        const method = prompt('Payment method (Cash, Credit Card, Debit Card, Insurance, Bank Transfer):', 'Cash');
        if (method === null) return;
        try { recordPayment({ billId, amount, method }); }
        catch (error) { alert(error.message); }
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
                                <td><strong>${escapeHtml(r.id)}</strong></td>
                                <td>${escapeHtml(r.patientId)}</td>
                                <td>${escapeHtml(r.doctorId)}</td>
                                <td>${escapeHtml(r.visitDate)}</td>
                                <td><span class="badge badge-diagnosis">${escapeHtml(r.diagnosis)}</span></td>
                                <td>${escapeHtml(r.prescriptions.join(', '))}</td>
                                <td>
                                    <button class="btn btn-sm btn-outline" onclick='window.RecordsBillingModule.viewRecordDetails(${JSON.stringify(r.id)})'>View Notes</button>
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
                                <td><strong>${escapeHtml(b.id)}</strong></td>
                                <td>${escapeHtml(b.patientId)}</td>
                                <td>${escapeHtml(b.issueDate)}</td>
                                <td>$${b.totalAmount.toFixed(2)}</td>
                                <td>$${b.paidAmount.toFixed(2)}</td>
                                <td>$${b.balance.toFixed(2)}</td>
                                <td><span class="badge badge-${b.status.toLowerCase().replace(' ', '-')}">${b.status}</span></td>
                                <td>
                                    ${b.status !== 'Paid' ? `
                                        <button class="btn btn-sm btn-success" onclick='window.RecordsBillingModule.openPaymentModal(${JSON.stringify(b.id)})'>
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
        openAddRecordModal,
        viewRecordDetails,
        openCreateBillModal,
        openPaymentModal,
        renderRecordsTab,
        renderBillingTab,
        getMedicalRecords: () => medicalRecords,
        getBills: () => bills,
        getPayments: () => payments
    };
})();
