# Sprint 2 Requirement Traceability & Verification Matrix

| Requirement ID | Requirement Name | Planned Owner | Implementation Artifacts | Automated Verification Suite | Status |
|---|---|---|---|---|---|
| **FR-01** | User Login & Role-Based Access | Pratham Mudakavi | `include/hms/auth.hpp`, `src/auth.cpp` | `tests/test_auth.cpp` | **Verified (Passed)** |
| **FR-02** | Patient Registration | Pratham Mudakavi | `include/hms/patient_repository.hpp`, `src/patient_repository.cpp` | `tests/test_registration.cpp` | **Verified (Passed)** |
| **FR-03** | Patient Record Update | Pratham Mudakavi | `src/patient_repository.cpp` | `tests/test_update.cpp` | **Verified (Passed)** |
| **FR-04** | Patient Search | Pratham Mudakavi | `src/patient_repository.cpp` | `tests/test_search.cpp` | **Verified (Passed)** |
| **FR-05** | Doctor Management | Pratham Mudakavi | `include/hms/doctor_repository.hpp`, `src/doctor_repository.cpp` | `tests/test_doctors.cpp` | **Verified (Passed)** |
| **FR-06** | Appointment Scheduling | Pratham Mudakavi | `include/hms/appointment_repository.hpp`, `src/appointment_repository.cpp` | `tests/test_appointment_scheduling.cpp`, `tests/test_appointment_conflicts.cpp` | **Verified (Passed)** |
| **FR-07** | Appointment Modification & Cancellation | Pratham Mudakavi | `src/appointment_repository.cpp` | `tests/test_appointment_modification.cpp` | **Verified (Passed)** |
| **FR-08** | Inpatient Admission Management | Prathik Nambiar | `include/hms/admission_repository.hpp`, `src/admission_repository.cpp`, `web/js/inpatient_management.js` | `tests/test_admissions.cpp`, `tests/test_inpatient_workflow.cpp` | **Verified (Passed)** |
| **FR-09** | Room & Ward Allocation | Prathik Nambiar | `include/hms/room_repository.hpp`, `src/room_repository.cpp`, `web/js/inpatient_management.js` | `tests/test_room_allocation.cpp`, `tests/test_inpatient_workflow.cpp` | **Verified (Passed)** |
| **FR-10** | Clinical Medical Records Management | Pratik Patil | `include/hms/medical_record.hpp`, `src/medical_record_repository.cpp`, `web/js/records_billing.js` | `tests/test_medical_records.cpp` | **Verified (Passed)** |
| **FR-11** | Inpatient Discharge Management | Prathik Nambiar | `src/admission_repository.cpp`, `web/js/inpatient_management.js` | `tests/test_admissions.cpp`, `tests/test_inpatient_workflow.cpp` | **Verified (Passed)** |
| **FR-12** | Itemized Billing & Invoicing | Pratik Patil | `include/hms/bill.hpp`, `src/bill_repository.cpp`, `web/js/records_billing.js` | `tests/test_billing.cpp` | **Verified (Passed)** |
| **FR-13** | Payment Recording & Balance Tracking | Pratik Patil | `src/bill_repository.cpp`, `web/js/records_billing.js` | `tests/test_billing.cpp` | **Verified (Passed)** |
| **FR-14** | Reporting Engine (Census, Occupancy, Revenue) | Pratham P Shetty | `include/hms/report_service.hpp`, `src/report_service.cpp`, `web/js/reports_backup.js` | `tests/test_reports.cpp` | **Verified (Passed)** |
| **FR-15** | Centralized Data Validation | Pratham Mudakavi | `include/hms/validation_service.hpp`, `src/validation_service.cpp` | `tests/test_validation.cpp` | **Verified (Passed)** |
| **FR-16** | Durable Multi-Entity Persistence Engine | Pratham Mudakavi | `include/hms/persistence_engine.hpp`, `src/persistence_engine.cpp` | `tests/test_persistence_engine.cpp`, `tests/test_persistence_performance.cpp` | **Verified (Passed)** |
| **FR-17** | Data Backup & Snapshot Verification | Pratham P Shetty | `include/hms/backup_service.hpp`, `src/backup_service.cpp`, `web/js/reports_backup.js` | `tests/test_backup.cpp` | **Verified (Passed)** |
| **FR-18** | Clean Exit & Safe Save | Prathik Nambiar | `include/hms/safety_manager.hpp`, `src/safety_manager.cpp` | `tests/test_safe_save.cpp` | **Verified (Passed)** |
| **NFR-01** | Sub-Second Performance (< 1.0s) | Pratham Mudakavi | High-throughput in-memory indexes | `tests/test_performance.cpp` | **Verified (Passed)** |
| **NFR-02** | Usability & Role-Based UI | Pratham Mudakavi | Responsive Web Portal & Menus | `tests/test_usability.cpp` | **Verified (Passed)** |
| **NFR-03** | Reliability & Integrity Checks | Prathik Nambiar | `src/safety_manager.cpp` atomic swap | `tests/test_reliability.cpp`, `tests/test_safe_save.cpp` | **Verified (Passed)** |
| **NFR-04** | Security & RBAC Enforcement | Pratham Mudakavi | Role Permission Matrix in `auth.cpp` | `tests/test_security.cpp`, `tests/test_security_boundaries.cpp` | **Verified (Passed)** |
| **NFR-05** | Maintainability & Clean Architecture | Pratik Patil | Refactored modular libraries & single responsibility | `tests/test_portability.cpp` | **Verified (Passed)** |
| **NFR-06** | Cross-Platform Portability | Prathik Nambiar | Pure C++17, MSVC & POSIX Sockets | GitHub Actions Matrix (macOS, Ubuntu, Windows) | **Verified (Passed)** |

---

## Sprint 2 Pull Request Audit Trail

| PR Number | Feature Branch | Author / Contributor | Delivered Requirements | Merged Strategy | CI Matrix Status |
|---|---|---|---|---|---|
| **#34** | `feature/pratik-medical-records-billing` | Pratik Patil (@Pratix005) | FR-10, FR-12, FR-13, Clinical & Billing Web Module | Merge Commit | 6/6 Checks Passed (macOS, Ubuntu, Windows) |
| **#35** | `feature/pratham-shetty-reports-and-backup` | Pratham P Shetty (@ka27n1001-svg) | FR-14, FR-17, Analytics & Backup Web Module | Merge Commit | 6/6 Checks Passed (macOS, Ubuntu, Windows) |
| **#36** | `feature/prathik-inpatient-safety-reliability` | Prathik M Nambiar (@prathiknambiar) | FR-08, FR-09, FR-11, FR-18, NFR-03, Inpatient Ward Web Module | Merge Commit | 6/6 Checks Passed (macOS, Ubuntu, Windows) |
| **#37** | `feature/pratham-mudakavi-persistence-validation-webserver` | Pratham Mudakavi (@Pratham042006) | FR-15, FR-16, Embedded HTTP Server, SPA Shell | Merge Commit | 6/6 Checks Passed (macOS, Ubuntu, Windows) |
