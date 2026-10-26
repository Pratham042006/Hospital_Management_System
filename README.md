# Hospital Management System

This branch contains the C++17 Sprint 1 work assigned to Pratham Mudakavi.

## Implemented requirements

- **FR-01 — User Login and Access:** authenticated sessions, useful login failures, four roles, permission checks, and role-specific menus.
- **FR-02 — Patient Registration:** generated patient IDs, required identity/contact/address/emergency fields, validation, and duplicate prevention.
- **FR-03 — Patient Record Update:** authorized updates, immutable patient IDs, atomic validation, and clear unknown-patient errors.
- **FR-04 — Patient Search:** exact ID lookup, case-insensitive partial-name search, phone/email lookup, and empty no-match results.
- **NFR-01 — Performance:** in-memory indexes for patient IDs and contact fields plus a 10,000-patient search test with a three-second limit.

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
- `tests/` — separate tests for each Sprint 1 requirement

