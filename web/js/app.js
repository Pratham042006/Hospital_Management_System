/**
 * Hospital Management System - Core SPA Controller
 * Contributor: Pratham Mudakavi (@Pratham042006)
 * Integrates:
 *   - FR-01 Authentication & Session
 *   - FR-02/03/04 Patient Registry & Search
 *   - FR-05 Doctor Management
 *   - FR-06/07 Appointment Scheduling
 *   - FR-15 Validation & FR-16 Persistence Core
 * Coordinates submodules for FR-08, 09, 10, 11, 12, 13, 14, 17, 18.
 */

window.hmsState = {
    currentUser: { username: 'admin', role: 'Administrator' },
    patients: [],
    doctors: [],
    appointments: [],
    rooms: [],
    admissions: []
};

// Seed realistic healthcare dataset
function seedInitialData() {
    window.hmsState.patients = [
        { id: 'P-00001', fullName: 'Ananya Sharma', dateOfBirth: '1992-04-12', gender: 'Female', phone: '9876543210', email: 'ananya.sharma@example.com', address: 'Indiranagar, Bengaluru', emergencyContactName: 'Rohan Sharma', emergencyContactPhone: '9876543219' },
        { id: 'P-00002', fullName: 'Arjun Mehta', dateOfBirth: '1980-01-15', gender: 'Male', phone: '9876543212', email: 'arjun.mehta@example.com', address: 'Koramangala, Bengaluru', emergencyContactName: 'Priya Mehta', emergencyContactPhone: '9876543299' },
        { id: 'P-00003', fullName: 'Kavya Rao', dateOfBirth: '1984-05-12', gender: 'Female', phone: '9876543214', email: 'kavya.rao@example.com', address: 'Jayanagar, Bengaluru', emergencyContactName: 'Vikram Rao', emergencyContactPhone: '9876543215' },
        { id: 'P-00004', fullName: 'Rahul Nair', dateOfBirth: '1975-02-19', gender: 'Male', phone: '9876543220', email: 'rahul.nair@example.com', address: 'Malleshwaram, Bengaluru', emergencyContactName: 'Lakshmi Nair', emergencyContactPhone: '9876543221' }
    ];

    window.hmsState.doctors = [
        { id: 'D-00001', fullName: 'Dr. Anil Kumar', specialization: 'Diagnostics', department: 'Internal Medicine', phone: '9876543211', email: 'anil.kumar@example.com' },
        { id: 'D-00002', fullName: 'Dr. Priya Menon', specialization: 'General Surgery', department: 'Surgery', phone: '9876543218', email: 'priya.menon@example.com' },
        { id: 'D-00003', fullName: 'Dr. Neha Desai', specialization: 'Critical Care', department: 'ICU & Emergency', phone: '9876543240', email: 'neha.desai@example.com' }
    ];

    window.hmsState.appointments = [
        { id: 'APT-00001', patientId: 'P-00001', doctorId: 'D-00001', date: '2026-11-04', time: '10:00', purpose: 'Diagnostics consultation', status: 'Completed' },
        { id: 'APT-00002', patientId: 'P-00003', doctorId: 'D-00002', date: '2026-11-05', time: '11:00', purpose: 'Pre-op surgical check', status: 'Scheduled' }
    ];

    // Seed Ward Rooms
    const r1 = window.InpatientModule.addRoom({ id: 'ICU-101', wardName: 'Intensive Care Unit', type: 'ICU', totalBeds: 4, dailyRate: 1200.0 });
    const r2 = window.InpatientModule.addRoom({ id: 'PVT-201', wardName: 'Executive Wing', type: 'Private', totalBeds: 1, dailyRate: 850.0 });
    const r3 = window.InpatientModule.addRoom({ id: 'GEN-301', wardName: 'North Ward A', type: 'General Ward', totalBeds: 6, dailyRate: 350.0 });
    window.hmsState.rooms = [r1, r2, r3];

    // Seed Admission
    const adm = window.InpatientModule.admitPatient({
        patientId: 'P-00003',
        doctorId: 'D-00002',
        roomId: 'ICU-101',
        admissionDate: '2026-11-04',
        reason: 'Acute post-op monitoring'
    });
    window.hmsState.admissions = [adm];

    // Seed Medical Records & Billing
    window.RecordsBillingModule.createMedicalRecord({
        patientId: 'P-00001',
        doctorId: 'D-00001',
        visitDate: '2026-11-04',
        symptoms: 'Persistent cough, fever 101F',
        diagnosis: 'Acute Bronchitis',
        treatmentNotes: 'Advised rest and hydration.',
        prescriptions: ['Amoxicillin 500mg', 'Albuterol Inhaler']
    });

    const bill = window.RecordsBillingModule.createBill({
        patientId: 'P-00003',
        admissionId: adm.id,
        issueDate: '2026-11-04',
        dueDate: '2026-11-18',
        items: [
            { category: 'RoomCharge', description: 'ICU Bed Stay', amount: 1200.0 },
            { category: 'Consultation', description: 'Surgeon Fee', amount: 300.0 }
        ]
    });

    window.RecordsBillingModule.recordPayment({
        billId: bill.id,
        amount: 500.0,
        paymentDate: '2026-11-04',
        method: 'Credit Card',
        notes: 'Deposit on admission'
    });
}

function switchTab(tabId) {
    document.querySelectorAll('.tab-view').forEach(view => view.classList.remove('active'));
    document.querySelectorAll('.nav-item').forEach(item => item.classList.remove('active'));

    const targetView = document.getElementById(tabId + '-view');
    const targetNav = document.getElementById('nav-' + tabId);

    if (targetView) targetView.classList.add('active');
    if (targetNav) targetNav.classList.add('active');

    // Tab-specific lifecycle renders
    if (tabId === 'overview') renderOverview();
    if (tabId === 'patients') renderPatients();
    if (tabId === 'doctors') renderDoctors();
    if (tabId === 'appointments') renderAppointments();
    if (tabId === 'ward' && window.InpatientModule) window.InpatientModule.renderWardTab();
    if (tabId === 'admissions' && window.InpatientModule) window.InpatientModule.renderAdmissionsTab();
    if (tabId === 'records' && window.RecordsBillingModule) window.RecordsBillingModule.renderRecordsTab();
    if (tabId === 'billing' && window.RecordsBillingModule) window.RecordsBillingModule.renderBillingTab();
    if (tabId === 'reports' && window.ReportsBackupModule) window.ReportsBackupModule.renderReportsTab();
    if (tabId === 'backup' && window.ReportsBackupModule) window.ReportsBackupModule.renderBackupTab();
}

function renderOverview() {
    const container = document.getElementById('overview-view');
    if (!container) return;

    const totalPatients = window.hmsState.patients.length;
    const activeAdmissions = window.InpatientModule.getAdmissions().filter(a => a.status === 'Admitted').length;
    const appointmentsScheduled = window.hmsState.appointments.length;
    const totalRevenue = window.RecordsBillingModule.getBills().reduce((acc, b) => acc + b.paidAmount, 0);

    container.innerHTML = `
        <div class="card header-card">
            <div class="card-title-row">
                <h3><i class="fas fa-hospital-alt"></i> Hospital Operations Dashboard</h3>
                <span class="badge badge-success">System Online (C++ Core v2.0)</span>
            </div>
            <p class="subtitle">Complete Sprint 2 Integrated Healthcare Management Dashboard</p>
        </div>

        <div class="stats-grid">
            <div class="stat-card">
                <div class="stat-label">Registered Patients</div>
                <div class="stat-value">${totalPatients}</div>
                <div class="stat-footnote">Active Electronic Records</div>
            </div>
            <div class="stat-card stat-accent">
                <div class="stat-label">Inpatients in Ward</div>
                <div class="stat-value">${activeAdmissions}</div>
                <div class="stat-footnote">Current Bed Occupants</div>
            </div>
            <div class="stat-card stat-warning">
                <div class="stat-label">Total Appointments</div>
                <div class="stat-value">${appointmentsScheduled}</div>
                <div class="stat-footnote">Consultations Scheduled</div>
            </div>
            <div class="stat-card stat-success">
                <div class="stat-label">Revenue Collected</div>
                <div class="stat-value">₹${totalRevenue.toLocaleString('en-IN', {minimumFractionDigits: 2, maximumFractionDigits: 2})}</div>
                <div class="stat-footnote">Settled Transactions</div>
            </div>
        </div>

    `;
}

function renderPatients() {
    const tbody = document.getElementById('patients-table-body');
    if (!tbody) return;
    tbody.innerHTML = window.hmsState.patients.map(p => `
        <tr>
            <td><strong>${p.id}</strong></td>
            <td>${p.fullName}</td>
            <td>${p.dateOfBirth}</td>
            <td>${p.gender}</td>
            <td>${p.phone}</td>
            <td>${p.email}</td>
            <td>${p.address}</td>
        </tr>
    `).join('');
}

function renderDoctors() {
    const tbody = document.getElementById('doctors-table-body');
    if (!tbody) return;
    tbody.innerHTML = window.hmsState.doctors.map(d => `
        <tr>
            <td><strong>${d.id}</strong></td>
            <td>${d.fullName}</td>
            <td><span class="badge badge-primary">${d.specialization}</span></td>
            <td>${d.department}</td>
            <td>${d.phone}</td>
            <td>${d.email}</td>
        </tr>
    `).join('');
}

function renderAppointments() {
    const tbody = document.getElementById('appointments-table-body');
    if (!tbody) return;
    tbody.innerHTML = window.hmsState.appointments.map(a => `
        <tr>
            <td><strong>${a.id}</strong></td>
            <td>${a.patientId}</td>
            <td>${a.doctorId}</td>
            <td>${a.date} ${a.time}</td>
            <td>${a.purpose}</td>
            <td><span class="badge badge-${a.status === 'Scheduled' ? 'primary' : 'success'}">${a.status}</span></td>
        </tr>
    `).join('');
}

// Global modal triggers for forms
window.openRegisterPatientModal = function () {
    const dialog = document.createElement('dialog');
    dialog.className = 'patient-dialog';
    dialog.innerHTML = `<form class="patient-form">
        <div class="form-heading"><div><h2>Register a patient</h2><p>Enter the patient's details below.</p></div><button type="button" class="close-dialog" aria-label="Close">&times;</button></div>
        <div class="form-grid">
          <label>Full name <input name="fullName" required autocomplete="name"></label>
          <label>Date of birth <input name="dateOfBirth" type="date" required></label>
          <label>Gender <select name="gender" required><option value="">Select gender</option><option>Female</option><option>Male</option><option>Other</option></select></label>
          <label>Phone number <input name="phone" type="tel" pattern="[0-9]{10}" title="Enter 10 digits" required></label>
          <label>Email <input name="email" type="email"></label>
          <label>Address <input name="address"></label>
          <label>Emergency contact name <input name="emergencyContactName"></label>
          <label>Emergency contact phone <input name="emergencyContactPhone" type="tel" pattern="[0-9]{10}" title="Enter 10 digits"></label>
        </div><p class="form-note">Demo data is kept only for this browser session. Use fictional details.</p>
        <div class="form-actions"><button type="button" class="btn btn-outline cancel-dialog">Cancel</button><button class="btn btn-primary" type="submit">Register patient</button></div>
    </form>`;
    document.body.appendChild(dialog);
    const close = () => dialog.close();
    dialog.addEventListener('close', () => dialog.remove());
    dialog.querySelector('.close-dialog').addEventListener('click', close);
    dialog.querySelector('.cancel-dialog').addEventListener('click', close);
    dialog.querySelector('form').addEventListener('submit', event => {
        event.preventDefault();
        const data = Object.fromEntries(new FormData(event.currentTarget));
        if (window.hmsState.patients.some(p => p.phone === data.phone)) {
            alert('A patient with this phone number already exists.'); return;
        }
        const newId = 'P-' + String(window.hmsState.patients.length + 1).padStart(5, '0');
        window.hmsState.patients.push({ id: newId, ...data });
        renderPatients(); close();
    });
    dialog.showModal();
};

window.openAddDoctorModal = function () {
    const name = prompt("Enter Doctor Full Name (e.g. Dr. Watson):");
    if (!name) return;
    const spec = prompt("Enter Medical Specialization:", "Cardiology");
    const dept = prompt("Enter Department:", "Internal Medicine");
    const phone = prompt("Enter Phone:", "9876543288");

    const newId = 'D-' + String(window.hmsState.doctors.length + 1).padStart(5, '0');
    window.hmsState.doctors.push({
        id: newId,
        fullName: name,
        specialization: spec || 'General',
        department: dept || 'Medicine',
        phone: phone || '9876543200',
        email: name.toLowerCase().replace(/[^a-z]/g, '') + '@hospital.com'
    });
    renderDoctors();
    alert("Doctor " + name + " added with ID: " + newId);
};

window.openScheduleModal = function () {
    const patId = prompt("Enter Patient ID (e.g. P-00001):", "P-00001");
    const docId = prompt("Enter Doctor ID (e.g. D-00001):", "D-00001");
    const date = prompt("Enter Date (YYYY-MM-DD):", "2026-11-10");
    const time = prompt("Enter Time (HH:MM):", "14:00");
    const purpose = prompt("Enter Reason for Visit:", "Routine Health Checkup");

    const newId = 'APT-' + String(window.hmsState.appointments.length + 1).padStart(5, '0');
    window.hmsState.appointments.push({
        id: newId,
        patientId: patId,
        doctorId: docId,
        date: date,
        time: time,
        purpose: purpose || 'Consultation',
        status: 'Scheduled'
    });
    renderAppointments();
    alert("Appointment confirmed: " + newId);
};

document.addEventListener('DOMContentLoaded', () => {
    seedInitialData();
    switchTab('overview');
});

// Shared forms for the dashboard's demonstration workflows.
function demoForm(title, fields, submit) {
    const dialog = document.createElement('dialog');
    dialog.className = 'patient-dialog';
    const form = document.createElement('form');
    form.className = 'patient-form';
    const heading = document.createElement('div');
    heading.className = 'form-heading';
    const h = document.createElement('h2'); h.textContent = title;
    const close = document.createElement('button'); close.type = 'button'; close.className = 'close-dialog'; close.textContent = '×'; close.setAttribute('aria-label', 'Close');
    heading.append(h, close); form.append(heading);
    const grid = document.createElement('div'); grid.className = 'form-grid';
    fields.forEach(field => {
        const label = document.createElement('label'); label.textContent = field.label;
        let input;
        if (field.options) {
            input = document.createElement('select');
            field.options.forEach(option => { const el = document.createElement('option'); el.value = option.value; el.textContent = option.label; input.append(el); });
        } else { input = document.createElement('input'); input.type = field.type || 'text'; }
        input.name = field.name; input.required = field.required !== false;
        if (field.min !== undefined) input.min = field.min;
        if (field.step) input.step = field.step;
        if (field.value) input.value = field.value;
        label.append(input); grid.append(label);
    });
    form.append(grid);
    const note = document.createElement('p'); note.className = 'form-note'; note.textContent = 'Demonstration only: changes last for this browser session.'; form.append(note);
    const error = document.createElement('p'); error.style.color = '#b42318'; error.setAttribute('role', 'alert'); form.append(error);
    const actions = document.createElement('div'); actions.className = 'form-actions';
    const cancel = document.createElement('button'); cancel.type = 'button'; cancel.className = 'btn btn-outline'; cancel.textContent = 'Cancel';
    const save = document.createElement('button'); save.type = 'submit'; save.className = 'btn btn-primary'; save.textContent = 'Save';
    actions.append(cancel, save); form.append(actions); dialog.append(form); document.body.append(dialog);
    close.onclick = cancel.onclick = () => dialog.close();
    dialog.addEventListener('close', () => dialog.remove());
    form.addEventListener('submit', event => { event.preventDefault(); try { submit(Object.fromEntries(new FormData(form))); dialog.close(); } catch (e) { error.textContent = e.message; } });
    dialog.showModal();
}
const optionList = items => items.map(item => ({value: item.id, label: `${item.id} — ${item.fullName || item.wardName}`}));
const field = (name, label, extra = {}) => ({name, label, ...extra});
const today = () => new Date().toISOString().slice(0, 10);
window.openAddDoctorModal = () => demoForm('Add a specialist', [field('fullName','Full name'),field('specialization','Specialization'),field('department','Department'),field('phone','Phone', {type:'tel'}),field('email','Email',{type:'email',required:false})], data => {
    data.id = 'D-' + String(hmsState.doctors.length + 1).padStart(5,'0'); hmsState.doctors.push(data); renderDoctors();
});
window.openScheduleModal = () => demoForm('Book an appointment', [field('patientId','Patient',{options:optionList(hmsState.patients)}),field('doctorId','Doctor',{options:optionList(hmsState.doctors)}),field('date','Date',{type:'date',value:today()}),field('time','Time',{type:'time'}),field('purpose','Purpose')], data => {
    if (!data.patientId || !data.doctorId) throw Error('Add a patient and doctor first.');
    if (hmsState.appointments.some(a => a.doctorId === data.doctorId && a.date === data.date && a.time === data.time && a.status === 'Scheduled')) throw Error('The doctor already has an appointment at this time.');
    hmsState.appointments.push({id:'APT-'+String(hmsState.appointments.length+1).padStart(5,'0'),...data,status:'Scheduled'}); renderAppointments();
});
window.InpatientModule.openAddRoomModal = () => demoForm('Add a ward or room', [field('id','Room ID'),field('wardName','Ward name'),field('type','Room type'),field('totalBeds','Number of beds',{type:'number',min:1}),field('dailyRate','Daily rate',{type:'number',min:1,step:'0.01'})], data => { if (InpatientModule.getRooms().some(r=>r.id===data.id)) throw Error('Room ID already exists.'); InpatientModule.addRoom(data); hmsState.rooms = InpatientModule.getRooms(); InpatientModule.renderWardTab(); });
window.InpatientModule.openAdmitModal = () => demoForm('Admit a patient', [field('patientId','Patient',{options:optionList(hmsState.patients)}),field('doctorId','Doctor',{options:optionList(hmsState.doctors)}),field('roomId','Room',{options:optionList(InpatientModule.getRooms())}),field('admissionDate','Admission date',{type:'date',value:today()}),field('reason','Reason')], data => { InpatientModule.admitPatient(data); hmsState.admissions = InpatientModule.getAdmissions(); InpatientModule.renderAdmissionsTab(); });
window.InpatientModule.openDischargeModal = id => demoForm('Discharge patient', [field('date','Discharge date',{type:'date',value:today()}),field('summary','Discharge summary')], data => { InpatientModule.dischargePatient(id,data.date,data.summary); InpatientModule.renderAdmissionsTab(); });
window.InpatientModule.viewDischargeSummary = id => { const a = InpatientModule.getAdmissions().find(x=>x.id===id); if (a) alert(a.dischargeSummary || 'No summary available.'); };
window.RecordsBillingModule.openAddRecordModal = () => demoForm('Add medical record', [field('patientId','Patient',{options:optionList(hmsState.patients)}),field('doctorId','Doctor',{options:optionList(hmsState.doctors)}),field('visitDate','Visit date',{type:'date',value:today()}),field('symptoms','Symptoms',{required:false}),field('diagnosis','Diagnosis'),field('treatmentNotes','Treatment notes',{required:false}),field('prescriptions','Prescriptions (comma separated)',{required:false})], data => { data.prescriptions = data.prescriptions.split(',').map(s=>s.trim()).filter(Boolean); RecordsBillingModule.createMedicalRecord(data); RecordsBillingModule.renderRecordsTab(); });
window.RecordsBillingModule.viewRecordDetails = id => { const r = RecordsBillingModule.getMedicalRecords().find(x=>x.id===id); if (r) alert(`Symptoms: ${r.symptoms || '—'}\nDiagnosis: ${r.diagnosis}\nTreatment: ${r.treatmentNotes || '—'}`); };
window.RecordsBillingModule.openCreateBillModal = () => demoForm('Generate a bill', [field('patientId','Patient',{options:optionList(hmsState.patients)}),field('issueDate','Issue date',{type:'date',value:today()}),field('dueDate','Due date',{type:'date',value:today()}),field('description','Charge description'),field('amount','Amount',{type:'number',min:0.01,step:'0.01'})], data => { RecordsBillingModule.createBill({...data,items:[{category:'Other',description:data.description,amount:Number(data.amount)}]}); RecordsBillingModule.renderBillingTab(); });
window.RecordsBillingModule.openPaymentModal = id => demoForm('Record payment', [field('amount','Amount',{type:'number',min:0.01,step:'0.01'}),field('paymentDate','Payment date',{type:'date',value:today()}),field('method','Payment method',{options:['Cash','Card','Other'].map(x=>({value:x,label:x}))})], data => { RecordsBillingModule.recordPayment({...data,billId:id}); RecordsBillingModule.renderBillingTab(); });
