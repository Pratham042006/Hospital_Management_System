# Hospital Management System

This repository contains the C++17 Hospital Management System. Sprint 1 contributions are shared by the project team, including Pratham P Shetty and Pratham Mudakavi.

## Implemented requirements

- **FR-01 — User Login and Access:** authenticated sessions, useful login failures, four roles, permission checks, and role-specific menus.
- **FR-02 — Patient Registration:** generated patient IDs, required identity/contact/address/emergency fields, validation, and duplicate prevention.
- **FR-03 — Patient Record Update:** authorized updates, immutable patient IDs, atomic validation, and clear unknown-patient errors.
- **FR-04 — Patient Search:** exact ID lookup, case-insensitive partial-name search, phone/email lookup, and empty no-match results.
- **FR-05 — Doctor Management:** add and list doctors, validate required fields, prevent duplicate phone/email records, and enforce role permissions.
- **FR-06 — Appointment Scheduling:** schedule appointments against existing patients and doctors, validate date/time formats, and prevent same-time patient/doctor conflicts.
- **FR-07 — Appointment Modification:** modify scheduled appointments, preserve appointment IDs, detect conflicts, and cancel scheduled appointments.
- **NFR-01 — Performance:** in-memory indexes for patient IDs and contact fields plus a 10,000-patient search test with a three-second limit.
- **NFR-02 — Usability:** clear validation errors and stable generated doctor/appointment IDs in the service layer; CLI integration remains a follow-up.

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

- `include/hms/` — public domain and service interfaces
- `src/` — authentication, authorization, patient repository, application service, and CLI
- `tests/` — separate tests for authentication, patient workflows, performance, doctor management, and appointment workflows

