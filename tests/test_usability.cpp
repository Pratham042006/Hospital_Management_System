#include "test_support.hpp"

#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {
int failures = 0;
#define CHECK(condition)                                                                    \
    do {                                                                                    \
        if (!(condition)) {                                                                 \
            std::cerr << "CHECK failed at line " << __LINE__ << ": " #condition << '\n'; \
            ++failures;                                                                     \
        }                                                                                   \
    } while (false)

bool contains(const std::vector<std::string>& values, const std::string& value) {
    return std::find(values.begin(), values.end(), value) != values.end();
}
}  // namespace

int main() {
    auto system = makeTestSystem();
    const auto administrator = *system.login("admin", "test-admin-password");
    const auto receptionist = *system.login("receptionist", "test-reception-password");
    const auto doctor = *system.login("doctor", "test-doctor-password");
    const auto billing = *system.login("billing", "test-billing-password");

    const auto adminMenu = system.menuFor(administrator);
    const auto receptionMenu = system.menuFor(receptionist);
    const auto doctorMenu = system.menuFor(doctor);
    const auto billingMenu = system.menuFor(billing);
    CHECK(contains(adminMenu, "Manage users"));
    CHECK(contains(receptionMenu, "Register patient"));
    CHECK(contains(receptionMenu, "Manage appointments"));
    CHECK(contains(doctorMenu, "View appointments"));
    CHECK(!contains(doctorMenu, "Manage appointments"));
    CHECK(!contains(billingMenu, "Manage doctors"));

    try {
        system.registerPatient(receptionist, {});
        CHECK(false);
    } catch (const std::invalid_argument& error) {
        CHECK(std::string(error.what()).find("name") != std::string::npos);
    }

    const auto patient = system.registerPatient(
        receptionist, {"Usability Patient", "1995-04-12", "Other", "9000033333",
                       "usability@example.com", "4 Test Street", "Emergency", "8000033333"});
    CHECK(!patient.id.empty());
    hms::PatientUpdate update;
    update.address = "5 Updated Street";
    CHECK(system.updatePatient(receptionist, patient.id, update).address == "5 Updated Street");

    const auto clinician = system.addDoctor(
        receptionist, {"Dr Usability", "General Medicine", "Outpatients", "9111133333",
                       "usability.doctor@example.com", {"2099-12-05 09:30"}});
    const auto appointment = system.scheduleAppointment(
        receptionist, {patient.id, clinician.id, "2099-12-05", "09:30", "Usability check"});
    CHECK(system.cancelAppointment(receptionist, appointment.id).status ==
          hms::AppointmentStatus::Cancelled);

    if (failures != 0) return 1;
    std::cout << "NFR-02 representative workflow and menu tests passed.\n";
    return 0;
}
