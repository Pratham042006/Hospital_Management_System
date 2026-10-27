# Hospital Management System

This repository contains the C++17 Hospital Management System project.

## Implemented requirements

- **FR-01 — User Login and Access:** authenticated sessions, useful login failures, four roles, permission checks, and role-specific menus.
- **FR-02 — Patient Registration:** generated patient IDs, required identity/contact/address/emergency fields, validation, and duplicate prevention.
- **FR-03 — Patient Record Update:** authorized updates, immutable patient IDs, atomic validation, and clear unknown-patient errors.
- **FR-04 — Patient Search:** exact ID lookup, case-insensitive partial-name search, phone/email lookup, and empty no-match results.
- **NFR-01 — Performance:** in-memory indexes for patient IDs and contact fields plus a 10,000-patient search test with a three-second limit.
- **NFR-05 — Maintainability:** patient validation and normalization are isolated from repository storage, and the console presentation layer is separate from the application entry point.

## Build and test

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

Run the console application with:

```sh
./build/hms
```

## Demonstration accounts

| Role | Username | Password |
|---|---|---|
| Administrator | `admin` | `admin123` |
| Receptionist | `receptionist` | `reception123` |
| Doctor | `doctor` | `doctor123` |
| Billing staff | `billing` | `billing123` |

These are local demonstration accounts. Passwords are converted to salted digests before storage in memory. A dedicated password KDF and external account store should replace the built-in accounts before production use.

## Project structure

- `include/hms/` — domain models and public service interfaces
- `src/auth.cpp` — authentication, roles, and permission policy
- `src/patient_repository.cpp` — patient storage, indexes, and repository operations
- `src/patient_validation.cpp` — patient input normalization and validation rules
- `src/hospital_system.cpp` — application workflows and authorization checks
- `src/console_app.cpp` — console prompts, menus, and output; `src/main.cpp` is the entry point
- `tests/` — separate tests for each Sprint 1 requirement

The current code implements authentication and patient workflows. Doctor, appointment, admission, clinical, billing, and reporting workflows are not implemented yet; add each as its own domain and service module when that functionality is introduced. Keep persistent storage behind repository interfaces so storage choices do not spread into the console or application layer.

