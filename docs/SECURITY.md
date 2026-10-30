# Security Architecture & Role-Based Access Control (RBAC)

**Project:** Hospital Management System  
**Author:** Pratik Patil (PES2UG24CS372)  
**SRS Reference:** NFR-04 (Security)  

---

## 1. Overview

The Hospital Management System enforces strict Role-Based Access Control (RBAC) to protect sensitive healthcare information and ensure that users can only perform actions authorized for their assigned role.

---

## 2. Role Permission Matrix

| Operation / Feature | Administrator | Receptionist | Doctor | Billing Staff |
|---|:---:|:---:|:---:|:---:|
| **User Account Management** (`Manage users`) |  Yes | ❌ No | ❌ No | ❌ No |
| **Patient Registration** (`Register patient`) |  Yes |  Yes | ❌ No | ❌ No |
| **Patient Update** (`Update patient`) |  Yes |  Yes | ❌ No | ❌ No |
| **Patient Search** (`Search patients`) |  Yes |  Yes |  Yes |  Yes |
| **Clinical Records** (`View clinical data`) |  Yes | ❌ No |  Yes | ❌ No |
| **Billing Operations** (`Manage billing`) |  Yes | ❌ No | ❌ No |  Yes |
| **Doctor Management** (`Manage doctors`) |  Yes |  Yes | ❌ No | ❌ No |
| **Doctor Directory** (`View doctors`) |  Yes |  Yes |  Yes | ❌ No |
| **Appointment Management** (`Manage appointments`) |  Yes |  Yes | ❌ No | ❌ No |
| **Appointment View** (`View appointments`) |  Yes |  Yes |  Yes | ❌ No |

---

## 3. Authentication & Credential Protection

1. **Non-Plaintext Storage:**
   - Passwords are never stored or logged in plaintext.
   - In-memory credentials utilize a 64-bit salted digest to mitigate brute-force and dictionary attacks.
2. **Hidden Terminal Input:**
   - On Unix (macOS/Linux), standard input disables terminal echo (`termios`) during password entry.
   - On Windows, console input mode disables `ENABLE_ECHO_INPUT`.
3. **Session Token Isolation:**
   - Authentication produces an immutable `Session` token containing the authenticated username and granted `Role`.
   - All internal operations validate the session before executing domain queries.
