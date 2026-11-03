/**
 * Hospital Management System - Inpatient Admissions & Room Ward Management
 * Contributor: Prathik M Nambiar (@prathiknambiar)
 * Requirements: FR-08 (Admissions), FR-09 (Room Allocation), FR-11 (Discharge), FR-18 (Safe Save), NFR-03 (Reliability)
 */

window.InpatientModule = (function () {
    let admissions = [];
    let rooms = [];

    function init() {
        renderWardTab();
        renderAdmissionsTab();
    }

    function addRoom(roomData) {
        if (!roomData.id || !roomData.wardName) {
            throw new Error("Room ID and Ward Name are required.");
        }
        const room = {
            id: roomData.id,
            wardName: roomData.wardName,
            type: roomData.type || 'General Ward',
            totalBeds: Number(roomData.totalBeds) || 1,
            occupiedBeds: 0,
            dailyRate: Number(roomData.dailyRate) || 500.0
        };
        rooms.push(room);
        return room;
    }

    function admitPatient(admData) {
        if (!admData.patientId || !admData.doctorId || !admData.roomId) {
            throw new Error("Patient ID, Doctor ID, and Room ID are mandatory.");
        }
        const room = rooms.find(r => r.id === admData.roomId);
        if (!room) throw new Error("Room not found: " + admData.roomId);
        if (room.occupiedBeds >= room.totalBeds) {
            throw new Error("No beds available in room " + admData.roomId);
        }

        const admission = {
            id: 'ADM-' + String(admissions.length + 1).padStart(5, '0'),
            patientId: admData.patientId,
            doctorId: admData.doctorId,
            roomId: admData.roomId,
            admissionDate: admData.admissionDate || new Date().toISOString().slice(0, 10),
            reason: admData.reason || '',
            status: 'Admitted',
            dischargeDate: null,
            dischargeSummary: null
        };

        room.occupiedBeds++;
        admissions.push(admission);
        return admission;
    }

    function dischargePatient(admissionId, dischargeDate, summary) {
        const admission = admissions.find(a => a.id === admissionId);
        if (!admission) throw new Error("Admission ID not found: " + admissionId);
        if (admission.status === 'Discharged') throw new Error("Patient already discharged.");

        const room = rooms.find(r => r.id === admission.roomId);
        if (room && room.occupiedBeds > 0) {
            room.occupiedBeds--;
        }

        admission.status = 'Discharged';
        admission.dischargeDate = dischargeDate || new Date().toISOString().slice(0, 10);
        admission.dischargeSummary = summary || 'Discharged in stable condition.';
        return admission;
    }

    function renderWardTab() {
        const container = document.getElementById('ward-view');
        if (!container) return;

        container.innerHTML = `
            <div class="card header-card">
                <div class="card-title-row">
                    <h3><i class="fas fa-bed"></i> Inpatient Ward & Bed Capacity (FR-09)</h3>
                    <button class="btn btn-primary" onclick="window.InpatientModule.openAddRoomModal()">
                        <i class="fas fa-plus"></i> Add Ward / Room
                    </button>
                </div>
                <p class="subtitle">Track bed allocations, ward availability, and daily room charges.</p>
            </div>

            <div class="rooms-grid" id="rooms-grid-container">
                ${rooms.map(r => {
                    const available = r.totalBeds - r.occupiedBeds;
                    const isFull = available <= 0;
                    return `
                        <div class="room-card ${isFull ? 'room-card-full' : 'room-card-available'}">
                            <div class="room-card-header">
                                <span class="room-id">${r.id}</span>
                                <span class="badge ${isFull ? 'badge-danger' : 'badge-success'}">
                                    ${isFull ? 'Full' : available + ' Beds Free'}
                                </span>
                            </div>
                            <div class="room-ward">${r.wardName}</div>
                            <div class="room-type">${r.type}</div>
                            <div class="room-meta">
                                <div><strong>Rate:</strong> $${r.dailyRate.toFixed(2)}/day</div>
                                <div><strong>Occupancy:</strong> ${r.occupiedBeds} / ${r.totalBeds}</div>
                            </div>
                        </div>
                    `;
                }).join('')}
            </div>
        `;
    }

    function renderAdmissionsTab() {
        const container = document.getElementById('admissions-view');
        if (!container) return;

        container.innerHTML = `
            <div class="card header-card">
                <div class="card-title-row">
                    <h3><i class="fas fa-hospital-user"></i> Admissions & Discharges (FR-08, FR-11)</h3>
                    <button class="btn btn-primary" onclick="window.InpatientModule.openAdmitModal()">
                        <i class="fas fa-user-plus"></i> Admit Patient
                    </button>
                </div>
            </div>

            <div class="card table-card">
                <table class="data-table" id="admissions-table">
                    <thead>
                        <tr>
                            <th>Admission ID</th>
                            <th>Patient ID</th>
                            <th>Doctor ID</th>
                            <th>Room ID</th>
                            <th>Admission Date</th>
                            <th>Status</th>
                            <th>Actions</th>
                        </tr>
                    </thead>
                    <tbody id="admissions-table-body">
                        ${admissions.map(a => `
                            <tr>
                                <td><strong>${a.id}</strong></td>
                                <td>${a.patientId}</td>
                                <td>${a.doctorId}</td>
                                <td>${a.roomId}</td>
                                <td>${a.admissionDate}</td>
                                <td>
                                    <span class="badge ${a.status === 'Admitted' ? 'badge-primary' : 'badge-secondary'}">
                                        ${a.status}
                                    </span>
                                </td>
                                <td>
                                    ${a.status === 'Admitted' ? `
                                        <button class="btn btn-sm btn-warning" onclick="window.InpatientModule.openDischargeModal('${a.id}')">
                                            Discharge
                                        </button>
                                    ` : `
                                        <button class="btn btn-sm btn-outline" onclick="window.InpatientModule.viewDischargeSummary('${a.id}')">
                                            Summary
                                        </button>
                                    `}
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
        addRoom,
        admitPatient,
        dischargePatient,
        renderWardTab,
        renderAdmissionsTab,
        getRooms: () => rooms,
        getAdmissions: () => admissions
    };
})();
