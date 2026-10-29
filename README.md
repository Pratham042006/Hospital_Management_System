# Hospital Management System

This branch contains the C++17 Sprint 1 work assigned to Pratham Mudakavi.

## Implemented requirements

- **FR-01 — User Login and Access:** authenticated sessions, useful login failures, four roles, permission checks, and role-specific menus.
- **FR-02 — Patient Registration:** generated patient IDs, required identity/contact/address/emergency fields, validation, and duplicate prevention.
- **FR-03 — Patient Record Update:** authorized updates, immutable patient IDs, atomic validation, and clear unknown-patient errors.
- **FR-04 — Patient Search:** exact ID lookup, case-insensitive partial-name search, phone/email lookup, and empty no-match results.
- **FR-05 — Doctor Management:** add, view, update, and search complete doctor records including department and validated availability.
- **NFR-01 — Performance:** in-memory indexes for patient IDs and contact fields plus a 10,000-patient search test with a three-second limit.
- **NFR-04 — Security:** non-forgeable sessions, administrator-only account creation, hidden password entry, and role-scoped patient views.

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

## Secure first run

Set an administrator password in the environment, then start the program. The password is not stored in the repository or displayed by the terminal.

```sh
export HMS_ADMIN_PASSWORD='choose-a-strong-password'
./build/hms
```

Sign in as `admin`, then use the administrator menu to create the other role accounts. Passwords are converted to salted digests before storage in memory. A dedicated password KDF and external account store should replace the in-memory account store before production use.

## Project structure

- `include/hms/` — public domain and service interfaces
- `src/` — authentication, authorization, patient repository, application service, and CLI
- `tests/` — separate tests for each Sprint 1 requirement

