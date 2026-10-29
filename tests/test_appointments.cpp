#include "hms/hospital_system.hpp"
#include <iostream>
#include <stdexcept>

int main() {
    int failures = 0;
    auto check = [&](bool condition, const char* expression, int line) {
        if (!condition) { std::cerr << "CHECK failed at line " << line << ": " << expression << '\n'; ++failures; }
    };
#define CHECK(x) check((x), #x, __LINE__)
    hms::HospitalSystem system;
    const auto receptionist = *system.login("receptionist", "reception123");
    const auto doctorRole = *system.login("doctor", "doctor123");
    hms::PatientDraft pd{"Alex Patient", "1995-04-12", "Other", "9000011111", "alex@example.com", "1 Main St", "Contact", "8000011111"};
    const auto patient = system.registerPatient(receptionist, pd);
    const auto doctor = system.addDoctor(receptionist, {"Dr. Asha Rao", "Cardiology", "9111122222", "asha@example.com"});
    CHECK(doctor.id == "DOC1");
    CHECK(system.listDoctors(receptionist).size() == 1);
    try { system.addDoctor(receptionist, {"", "", "", ""}); CHECK(false); } catch (const std::invalid_argument&) {}
    try { system.listDoctors(doctorRole); CHECK(false); } catch (const hms::AuthorizationError&) {}
    hms::AppointmentDraft draft{patient.id, doctor.id, "2026-11-02", "09:30", "Consultation"};
    const auto appointment = system.scheduleAppointment(receptionist, draft);
    CHECK(appointment.id == "APT1");
    try { system.scheduleAppointment(receptionist, draft); CHECK(false); } catch (const std::invalid_argument&) {}
    auto changed = draft; changed.date = "2026-11-03"; changed.time = "10:15";
    const auto updated = system.modifyAppointment(receptionist, appointment.id, changed);
    CHECK(updated.date == "2026-11-03");
    CHECK(system.listAppointments(receptionist).size() == 1);
    CHECK(system.cancelAppointment(receptionist, appointment.id));
    CHECK(!system.cancelAppointment(receptionist, "missing"));
    CHECK(failures == 0);
    if (failures == 0) std::cout << "FR-05/06/07 doctor and appointment tests passed.\n";
    return failures == 0 ? 0 : 1;
}
