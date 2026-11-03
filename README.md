# Hospital Management System (HMS)

[![build-and-test](https://github.com/Pratham042006/Hospital_Management_System/actions/workflows/ci.yml/badge.svg)](https://github.com/Pratham042006/Hospital_Management_System/actions/workflows/ci.yml)

A multi-platform Hospital Management System developed in modern C++17 by **Team 1**:
- **Pratham Mudakavi** ([@Pratham042006](https://github.com/Pratham042006))
- **Pratik Patil** ([@Pratix005](https://github.com/Pratix005))
- **Pratham P Shetty** ([@ka27n1001-svg](https://github.com/ka27n1001-svg))
- **Prathik M Nambiar** ([@prathiknambiar](https://github.com/prathiknambiar))

---

## 🌟 Sprint 2 Highlights: Web Dashboard & REST Server

In addition to the high-performance CLI console, HMS includes a zero-dependency **Embedded C++ HTTP Web Server & Responsive Web Portal**:
- **Zero external dependencies**: Built directly on portable C++17 sockets (POSIX / Winsock).
- **Interactive SPA Portal**: Real-time management across Patients, Doctors, Inpatient Ward Beds, Medical Records, Billing & Invoicing, and Hospital Analytics.
- **REST API Endpoints**: `/api/health`, `/api/login`, `/api/patients`, `/api/reports/census`, `/api/reports/occupancy`, `/api/reports/revenue`.

### 🚀 Running the Web Dashboard

```sh
# One-click execution on Linux/macOS:
./scripts/run_web.sh

# Or start directly with the server flag:
./build/hms --server 8080
```
Open **[http://localhost:8080](http://localhost:8080)** in your browser. (The web portal can also be opened directly via `web/index.html`).

---

## 📋 Comprehensive SRS Requirement Coverage (All 24 Completed)

### Functional Requirements (FR-01 to FR-18)
1. **FR-01 — User Login & RBAC**: Authenticated sessions, 4 distinct roles (Administrator, Doctor, Receptionist, Billing Staff), salted password hashes, permission matrices.
2. **FR-02 — Patient Registration**: Automatic sequential patient ID generation, full validation, contact normalization.
3. **FR-03 — Patient Record Update**: Authorized updates, immutable IDs, atomic validation.
4. **FR-04 — Patient Search**: Fast exact ID lookup, case-insensitive partial substring search, phone/email queries.
5. **FR-05 — Doctor Management**: Specialist profiles, department assignment, and schedule availability slots.
6. **FR-06 — Appointment Scheduling**: Conflict-free booking with doctors, schedule verification.
7. **FR-07 — Appointment Modification**: Atomic rescheduling and cancellation preserving audit history.
8. **FR-08 — Inpatient Admission**: Admission workflow linking patients, attending doctors, and designated rooms.
9. **FR-09 — Room & Ward Allocation**: Tracking room types (General Ward, Semi-Private, Private, ICU) and atomic bed allocation.
10. **FR-10 — Medical Record Management**: Clinical history, symptoms, diagnosis, treatment notes, and itemized prescriptions.
11. **FR-11 — Inpatient Discharge**: Clinical discharge summary generation and automated bed deallocation.
12. **FR-12 — Itemized Billing**: Invoicing with consultation fees, bed rates, lab tests, medications, and procedure fees.
13. **FR-13 — Payment Recording**: Ledger transactions, overpayment prevention, and balance status tracking.
14. **FR-14 — Intelligence Reports**: Patient census, bed occupancy %, revenue collected vs pending, and doctor utilization.
15. **FR-15 — Centralized Data Validation**: Reusable validation across identifiers, dates, phone numbers, and amounts.
16. **FR-16 — Durable Persistence Engine**: Multi-entity atomic persistence engine with automatic directory initialization.
17. **FR-17 — Data Backup & Restore**: Encrypted point-in-time state export with header checksum integrity verification.
18. **FR-18 — Exit & Safe Save**: Clean application termination with auto-flush guarantees across all entities.

### Non-Functional Requirements (NFR-01 to NFR-06)
- **NFR-01 — Performance**: In-memory hash indexing, tested with 10,000 patient operations under 3 seconds.
- **NFR-02 — Usability**: Intuitive Web Dashboard and structured CLI with error handling.
- **NFR-03 — Reliability**: Atomic filesystem swaps (`.tmp` write + atomic rename) ensuring zero corruption.
- **NFR-04 — Security**: Role-scoped authorization boundaries preventing clinical or financial privilege escalation.
- **NFR-05 — Maintainability**: Modular static libraries, single-responsibility services, and decoupled UI layers.
- **NFR-06 — Portability**: Verified across **Ubuntu Latest**, **macOS Latest**, and **Windows Latest (MSVC)** via automated CI.

---

## 🛠️ Build and Test

### Automated CMake Build & Test

```sh
cmake -S . -B build
cmake --build build --config Release
ctest --test-dir build --output-on-failure
```
**26 automated unit and integration test suites** pass with 100% success rate:
- `hms_tests`, `registration_tests`, `update_tests`, `search_tests`, `performance_tests`
- `security_tests`, `doctor_tests`, `appointment_scheduling_tests`, `appointment_modification_tests`
- `usability_tests`, `persistence_performance_tests`, `portability_tests`, `security_boundary_tests`
- `appointment_conflict_tests`, `room_allocation_tests`, `admission_tests`, `reliability_tests`
- `medical_records_tests`, `billing_tests`, `report_tests`, `backup_tests`, `safe_save_tests`
- `inpatient_workflow_tests`, `validation_tests`, `persistence_engine_tests`, `http_server_tests`

---

## 👥 Sprint 2 Team Contributions

| Team Member | GitHub Account | Assigned Modules | Feature Branch | Merged PR |
|---|---|---|---|---|
| **Pratik Patil** | [@Pratix005](https://github.com/Pratix005) | Medical Records (FR-10), Billing (FR-12), Payments (FR-13), Records & Billing Web UI | `feature/pratik-medical-records-billing` | [#34](https://github.com/Pratham042006/Hospital_Management_System/pull/34) |
| **Pratham P Shetty** | [@ka27n1001-svg](https://github.com/ka27n1001-svg) | Reports Engine (FR-14), Data Backup & Restore (FR-17), Intelligence Analytics Web UI | `feature/pratham-shetty-reports-and-backup` | [#35](https://github.com/Pratham042006/Hospital_Management_System/pull/35) |
| **Prathik M Nambiar** | [@prathiknambiar](https://github.com/prathiknambiar) | Admissions (FR-08), Rooms (FR-09), Discharge (FR-11), Safe Save (FR-18), Reliability (NFR-03), Inpatient Ward Web UI | `feature/prathik-inpatient-safety-reliability` | [#36](https://github.com/Pratham042006/Hospital_Management_System/pull/36) |
| **Pratham Mudakavi** | [@Pratham042006](https://github.com/Pratham042006) | Validation (FR-15), Persistence (FR-16), Embedded Web Server, SPA Portal Shell | `feature/pratham-mudakavi-persistence-validation-webserver` | [#37](https://github.com/Pratham042006/Hospital_Management_System/pull/37) |
