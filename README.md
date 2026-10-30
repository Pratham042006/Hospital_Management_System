# Hospital Management System

[![build-and-test](https://github.com/Pratham042006/Hospital_Management_System/actions/workflows/ci.yml/badge.svg)](https://github.com/Pratham042006/Hospital_Management_System/actions/workflows/ci.yml)

This repository contains the C++17 Sprint 1 release developed by Team 1 (Pratham P Mudakavi, Pratham P Shetty, Prathik M Nambiar, and Pratik Patil).

## Implemented requirements

- **FR-01 — User Login and Access:** authenticated sessions, useful login failures, four roles, permission checks, and role-specific menus.
- **FR-02 — Patient Registration:** generated patient IDs, required identity/contact/address/emergency fields, validation, and duplicate prevention.
- **FR-03 — Patient Record Update:** authorized updates, immutable patient IDs, atomic validation, and clear unknown-patient errors.
- **FR-04 — Patient Search:** exact ID lookup, case-insensitive partial-name search, phone/email lookup, and empty no-match results.
- **FR-05 — Doctor Management:** add, view, update, and search complete doctor records including department and validated availability.
- **FR-06 — Appointment Scheduling:** schedule against existing patients, doctors, and availability with real-date, time, and conflict validation.
- **FR-07 — Appointment Modification:** atomically reschedule or cancel active appointments while retaining cancelled history.
- **NFR-01 — Performance:** indexed searches plus an append-only patient journal tested with a 10,000-patient write, reload, update, and search workload under three seconds.
- **NFR-02 — Usability:** role-appropriate menus, consistent navigation, corrective errors, operation confirmations, and a representative workflow checklist.
- **NFR-04 — Security:** non-forgeable sessions, administrator-only account creation, hidden password entry, and role-scoped patient views.
- **NFR-05 — Maintainability:** patient validation is separated from persistence, and console interaction is separated from the application entry point.
- **NFR-06 — Portability:** cross-platform CMake build configuration and automated GitHub Actions verification across Ubuntu, macOS, and Windows.

## Build and test

### Quick build using helper scripts

```sh
# On Linux / macOS:
./scripts/build_unix.sh

# On Windows:
scripts\build_windows.bat
```

### Manual CMake build

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

Run the console application with:

```sh
./build/hms
```

## Secure first run

Set an administrator password in the environment, then start the program. The password is not stored in the repository or displayed by the terminal.

```sh
export HMS_ADMIN_PASSWORD='choose-a-strong-password'
./build/hms
```

Sign in as `admin`, then use the administrator menu to create the other role accounts. Passwords are converted to salted digests before storage in memory. A dedicated password KDF and external account store should replace the in-memory account store before production use.

## Project structure

- `include/hms/` — public domain, service, validation, and console interfaces
- `src/auth.cpp` — authentication, roles, and permission policy
- `src/patient_repository.cpp` — patient storage, indexes, persistence, and repository operations
- `src/patient_validation.cpp` — patient normalization and validation rules
- `src/hospital_system.cpp` — application workflows and authorization checks
- `src/console_app.cpp` — console prompts, menus, and output; `src/main.cpp` is the entry point
- `tests/` — separate tests for each Sprint 1 requirement

