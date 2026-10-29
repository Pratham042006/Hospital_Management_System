# Sprint 1 Requirements Traceability

This document maps the Sprint 1 requirements selected from the Software Requirements Specification (SRS) to their implementation locations and verification evidence. The Software Architecture Document (SAD) and Test Plan provide supporting design and test context.

The status and test results below reflect the reported Sprint 1 completion: all eleven automated test suites passed.

| Requirement | Requirement summary | Implementation | Verification | Implementation status | Related test result |
|---|---|---|---|---|---|
| FR-01 | Login and role access | `src/auth.cpp` | `tests/test_auth.cpp` | Completed in Sprint 1 | Passed |
| FR-02 | Patient registration | `src/patient_repository.cpp` | `tests/test_registration.cpp` | Completed in Sprint 1 | Passed |
| FR-03 | Patient update | `src/patient_repository.cpp` | `tests/test_update.cpp` | Completed in Sprint 1 | Passed |
| FR-04 | Patient search | `src/patient_repository.cpp` | `tests/test_search.cpp` | Completed in Sprint 1 | Passed |
| FR-05 | Doctor management | `src/doctor_repository.cpp` | `tests/test_doctors.cpp` | Completed in Sprint 1 | Passed |
| FR-06 | Appointment scheduling | `src/appointment_repository.cpp` | `tests/test_appointment_scheduling.cpp` | Completed in Sprint 1 | Passed |
| FR-07 | Appointment modification and cancellation | `src/appointment_repository.cpp` | `tests/test_appointment_modification.cpp` | Completed in Sprint 1 | Passed |
| NFR-01 | Performance and patient persistence | Patient persistence and performance coverage | `tests/test_performance.cpp`; `tests/test_persistence_performance.cpp` | Completed in Sprint 1 | Passed |
| NFR-02 | Usability and safe input handling | Console workflows and usability guidance | `tests/test_usability.cpp`; `docs/USABILITY_CHECKLIST.md` | Completed in Sprint 1 | Passed |
| NFR-04 | Security and role-based access | Authentication and authorization paths | `tests/test_security.cpp` | Completed in Sprint 1 | Passed |

Only requirements selected for Sprint 1 are included in this matrix. Features deferred to later sprints are recorded in [DEVIATIONS.md](DEVIATIONS.md).
