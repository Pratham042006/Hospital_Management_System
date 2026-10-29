#include "test_support.hpp"

#include <iostream>
#include <stdexcept>

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
    try { callable(); ++failures; } catch (const Exception&) {} catch (...) { ++failures; }
}
}  // namespace

int main() {
    auto system = makeTestSystem();
    const auto administrator = *system.login("admin", "test-admin-password");
    const auto receptionist = *system.login("receptionist", "test-reception-password");
    const auto doctorRole = *system.login("doctor", "test-doctor-password");

    const auto patient = system.registerPatient(
        receptionist, {"Appointment Patient", "1995-04-12", "Other", "9000012345",
                       "appointment@example.com", "1 Test Street", "Emergency", "8000012345"});
    const auto doctor = system.addDoctor(
        administrator, {"Dr Future", "General Medicine", "Outpatients", "9111123456",
                        "future@example.com", {"2099-12-01 09:30", "2099-12-01 10:30"}});

    hms::AppointmentDraft draft{patient.id, doctor.id, "2099-12-01", "09:30", "Consultation"};
    const auto appointment = system.scheduleAppointment(receptionist, draft);
    CHECK(appointment.id == "A000001");
    CHECK(appointment.status == hms::AppointmentStatus::Scheduled);
    CHECK(system.findAppointmentById(doctorRole, appointment.id).has_value());
    CHECK(system.listAppointments(receptionist).size() == 1);

    checkThrows<std::invalid_argument>([&] { system.scheduleAppointment(receptionist, draft); });
    auto missingPatient = draft;
    missingPatient.patientId = "P999999";
    missingPatient.time = "10:30";
    checkThrows<std::invalid_argument>([&] { system.scheduleAppointment(receptionist, missingPatient); });
    auto unavailable = draft;
    unavailable.time = "11:30";
    checkThrows<std::invalid_argument>([&] { system.scheduleAppointment(receptionist, unavailable); });
    auto impossibleDate = draft;
    impossibleDate.date = "2099-02-31";
    checkThrows<std::invalid_argument>([&] { system.scheduleAppointment(receptionist, impossibleDate); });
    auto past = draft;
    past.date = "2020-01-01";
    checkThrows<std::invalid_argument>([&] { system.scheduleAppointment(receptionist, past); });
    checkThrows<hms::AuthorizationError>([&] { system.scheduleAppointment(doctorRole, draft); });

    if (failures != 0) return 1;
    std::cout << "FR-06 appointment scheduling tests passed.\n";
    return 0;
}
