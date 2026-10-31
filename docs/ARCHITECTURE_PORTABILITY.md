# System Architecture: Portability, Reliability & Inpatient Management

**Author:** Prathik M Nambiar (PES2UG24CS371)  
**SRS Requirements Covered:** FR-08 (Admissions), FR-09 (Room Allocation), FR-11 (Discharge), NFR-03 (Reliability), NFR-06 (Portability)

---

## 1. Cross-Platform Portability Architecture (NFR-06)

The Hospital Management System is engineered to compile and execute deterministically across major operating systems (Ubuntu Linux, macOS, and Microsoft Windows) and toolchains (GCC, Clang, and MSVC).

### 1.1 Toolchain Neutrality
- **C++17 Baseline:** Enforced through compile-time static assertions (`_MSVC_LANG >= 201703L` and `__cplusplus >= 201703L`).
- **Data Type Alignment:** Fixed-width integer types (`std::uint64_t`, `std::size_t`) guarantee identical hashing behavior and byte representations across 32-bit and 64-bit target architectures.
- **Platform-Independent Path Delimiters:** File system paths conform to standard C++17 filesystem guidelines, avoiding hardcoded Unix forward slashes or Windows backslashes in production paths.

---

## 2. Inpatient Admission & Room Allocation Design (FR-08, FR-09, FR-11)

Inpatient workflows link the Patient Repository, Doctor Directory, and Room Infrastructure with atomic consistency.

```
       [ Patient Registry ]             [ Doctor Directory ]
                 \                               /
                  \                             /
                   v                           v
              +-------------------------------------+
              |       Admission Management          |
              |   (Record ID, Date, Reason, Attending) |
              +-------------------------------------+
                                 |
                                 v
                     [ Room & Bed Allocation ]
                   (Total Beds vs Occupied Beds)
```

### 2.1 Room & Bed Tracking Model (FR-09)
- **Room Categories:** `GeneralWard`, `SemiPrivate`, `Private`, and `ICU`.
- **Bed Occupancy Invariants:**
  - $\text{occupiedBeds} \le \text{totalBeds}$ at all times.
  - Bed allocation fails atomically with an exception if $\text{occupiedBeds} == \text{totalBeds}$.

### 2.2 Admission Lifecycle (FR-08, FR-11)
1. **Admission Phase:**
   - Validates existence of both Patient and Attending Doctor.
   - Prevents duplicate concurrent admissions for an already-admitted patient.
   - Allocates one bed in the target room.
2. **Discharge Phase:**
   - Records the discharge timestamp and detailed medical summary.
   - Transitions admission state to `Discharged`.
   - Automatically decrements room bed occupancy to restore availability for incoming patients.

---

## 3. Reliability & Fault-Tolerance Strategy (NFR-03)

- **Rollback on Partial Failures:** If an admission draft fails secondary validation after bed reservation, bed allocation is rolled back immediately.
- **Defensive Input Sanitization:** Non-terminating exception handling ensures that malformed user inputs, oversized payloads, or invalid foreign keys never crash the server or corrupt storage.
- **Idempotent Operations:** Repeated search queries and read-only reports produce zero side effects on the underlying data models.
