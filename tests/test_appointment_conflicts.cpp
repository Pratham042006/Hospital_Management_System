#include "test_support.hpp"

#include <iostream>
#include <stdexcept>

// FR-06 & FR-07 Appointment Conflict & Validation Regression Tests
// Author: Pratham P Shetty (PES2UG24CS370)

namespace {

int failures = 0;

#define CHECK(condition)                                                                    \
    do {                                                                                    \
        if (!(condition)) {                                                                 \
            std::cerr << "CHECK failed at line " << __LINE__ << ": " #condition << '\n'; \
            ++failures;                                                                     \
        }                                                                                   \
    } while (false)

template <typename Exception, typename Callable>
void checkThrows(Callable&& callable) {
    try {
        callable();
        ++failures;
    } catch (const Exception&) {
    } catch (...) {
        ++failures;
    }
}

}  // namespace

int main() {
    auto system = makeTestSystem();
    const auto receptionist = *system.login("receptionist", "test-reception-password");

    // 1. Seed doctor with specific availability slot
    hms::DoctorDraft doctorDraft{
        "Dr. Rajesh Sharma", "Cardiology", "Cardiovascular Sciences", "9876501234",
        "sharma@hms.org", {"2026-11-15 10:00", "2026-11-15 11:00"}
    };
    const auto doctor = system.addDoctor(receptionist, doctorDraft);

    // 2. Seed patient
    hms::PatientDraft patientDraft{
        "Rahul Verma", "1988-06-20", "Male", "9876543210", "rahul.verma@example.com",
        "MG Road, Bangalore", "Anita Verma", "9876543211"
    };
    const auto patient = system.registerPatient(receptionist, patientDraft);

    // 3. Successfully schedule valid appointment
    hms::AppointmentDraft draft1{
        patient.id, doctor.id, "2026-11-15", "10:00", "Cardiac evaluation"
    };
    const auto appt1 = system.scheduleAppointment(receptionist, draft1);
    CHECK(appt1.status == hms::AppointmentStatus::Scheduled);
    CHECK(appt1.doctorId == doctor.id);
    CHECK(appt1.patientId == patient.id);

    // 4. Reject duplicate appointment slot for same doctor (Conflict)
    hms::AppointmentDraft draftConflict{
        patient.id, doctor.id, "2026-11-15", "10:00", "Second overlapping request"
    };
    checkThrows<std::invalid_argument>([&] {
        system.scheduleAppointment(receptionist, draftConflict);
    });

    // 5. Reject appointment for non-existent doctor
    hms::AppointmentDraft draftInvalidDoctor{
        patient.id, "D-99999", "2026-11-15", "11:00", "Checkup"
    };
    checkThrows<std::invalid_argument>([&] {
        system.scheduleAppointment(receptionist, draftInvalidDoctor);
    });

    // 6. Reschedule appt1 to next available slot
    hms::AppointmentUpdate update;
    update.time = "11:00";
    const auto rescheduled = system.rescheduleAppointment(receptionist, appt1.id, update);
    CHECK(rescheduled.time == "11:00");
    CHECK(rescheduled.status == hms::AppointmentStatus::Scheduled);

    // 7. Cancel appointment and verify status
    const auto cancelled = system.cancelAppointment(receptionist, appt1.id);
    CHECK(cancelled.status == hms::AppointmentStatus::Cancelled);

    if (failures != 0) {
        std::cerr << failures << " appointment conflict assertion(s) failed.\n";
        return 1;
    }
    std::cout << "FR-06 and FR-07 appointment conflict tests passed successfully.\n";
    return 0;
}
