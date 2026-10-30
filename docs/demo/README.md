# Sprint 1 Demonstration Video

This directory contains the 1-minute video demonstration of the working Hospital Management System for Sprint 1, satisfying requirements **2.g** and **2.h** of the SE Mini-Project Deliverables (Part 2).

## File Details

- **Video File:** [`sprint1_demo.mp4`](sprint1_demo.mp4)
- **Duration:** 60 seconds (1 minute)
- **Resolution:** 1280x720 HD (20 FPS)
- **Demonstrated By:** Team 1
  - Pratham P Mudakavi (PES2UG24CS369)
  - Pratham P Shetty (PES2UG24CS370)
  - Prathik M Nambiar (PES2UG24CS371)
  - Pratik Patil (PES2UG24CS372)

## Demonstrated Workflows

1. **Automated Verification:**
   - 11 of 11 automated test suites passing (`ctest --output-on-failure`).
2. **User Authentication & Role Menus (FR-01, NFR-04):**
   - Administrator login with password masking and role-specific action menus.
3. **Patient Registration & Search (FR-02, FR-04, NFR-01):**
   - Registering a new patient with contact and emergency info (`P-00001`).
   - Searching patients by name with indexed lookup.
4. **Doctor Management (FR-05):**
   - Adding a doctor (`D-00001`) with specialization, department, and availability slots.
   - Listing active doctors.
5. **Appointment Scheduling & Cancellation (FR-06, FR-07):**
   - Scheduling an appointment for patient `P-00001` with doctor `D-00001`.
   - Cancelling the appointment while retaining historical records.
6. **Safe Exit & Clean Shutdown (NFR-02):**
   - Safe termination with confirmation.
