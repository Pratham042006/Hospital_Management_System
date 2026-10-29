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
        receptionist, {"Modify Patient", "1995-04-12", "Other", "9000098765",
                       "modify@example.com", "2 Test Street", "Emergency", "8000098765"});
    const auto doctor = system.addDoctor(
        administrator, {"Dr Modify", "General Medicine", "Outpatients", "9111198765",
                        "modify.doctor@example.com",
                        {"2099-12-02 09:30", "2099-12-03 10:30", "2099-12-04 11:30"}});
    const auto appointment = system.scheduleAppointment(
        receptionist, {patient.id, doctor.id, "2099-12-02", "09:30", "Initial visit"});
    const auto secondPatient = system.registerPatient(
        receptionist, {"Conflict Patient", "1990-01-10", "Other", "9000076543",
                       "conflict@example.com", "3 Test Street", "Emergency", "8000076543"});
    system.scheduleAppointment(
        receptionist, {secondPatient.id, doctor.id, "2099-12-04", "11:30", "Other visit"});

    hms::AppointmentUpdate update;
    update.date = "2099-12-03";
    update.time = "10:30";
    update.purpose = "Rescheduled visit";
    const auto modified = system.rescheduleAppointment(receptionist, appointment.id, update);
    CHECK(modified.id == appointment.id);
    CHECK(modified.date == "2099-12-03");
    CHECK(modified.purpose == "Rescheduled visit");

    hms::AppointmentUpdate impossible;
    impossible.date = "2099-02-31";
    checkThrows<std::invalid_argument>([&] {
        system.rescheduleAppointment(receptionist, appointment.id, impossible);
    });
    const auto unchanged = *system.findAppointmentById(doctorRole, appointment.id);
    CHECK(unchanged.date == "2099-12-03");

    hms::AppointmentUpdate past;
    past.date = "2020-01-01";
    checkThrows<std::invalid_argument>([&] {
        system.rescheduleAppointment(receptionist, appointment.id, past);
    });
    checkThrows<std::invalid_argument>([&] {
        system.rescheduleAppointment(receptionist, "A999999", update);
    });
    hms::AppointmentUpdate conflict;
    conflict.date = "2099-12-04";
    conflict.time = "11:30";
    checkThrows<std::invalid_argument>([&] {
        system.rescheduleAppointment(receptionist, appointment.id, conflict);
    });

    const auto cancelled = system.cancelAppointment(receptionist, appointment.id);
    CHECK(cancelled.status == hms::AppointmentStatus::Cancelled);
    const auto history = system.listAppointments(receptionist);
    CHECK(history.size() == 2);
    CHECK(history.front().status == hms::AppointmentStatus::Cancelled);
    checkThrows<std::invalid_argument>([&] { system.cancelAppointment(receptionist, appointment.id); });
    checkThrows<std::invalid_argument>([&] {
        system.rescheduleAppointment(receptionist, appointment.id, update);
    });
    checkThrows<hms::AuthorizationError>([&] { system.cancelAppointment(doctorRole, appointment.id); });

    if (failures != 0) return 1;
    std::cout << "FR-07 appointment modification tests passed.\n";
    return 0;
}
