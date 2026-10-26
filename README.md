# Hospital Management System

This branch contains the C++17 Sprint 1 work assigned to Pratham Mudakavi.

## Implemented requirements

- **FR-01 — User Login and Access:** authenticated sessions, useful login failures, four roles, permission checks, and role-specific menus.
- **FR-02 — Patient Registration:** generated patient IDs, required identity/contact/address/emergency fields, validation, and duplicate prevention.

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

