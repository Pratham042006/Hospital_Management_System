#include "hms/hospital_system.hpp"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <string>
#include <sstream>
#include <vector>

#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

namespace {

std::string prompt(const std::string& label) {
    std::cout << label;
    std::string value;
    std::getline(std::cin, value);
    return value;
}

std::string promptPassword(const std::string& label) {
    std::cout << label << std::flush;
    std::string password;

#if defined(_WIN32)
    HANDLE input = GetStdHandle(STD_INPUT_HANDLE);
    DWORD originalMode = 0;
    if (input != INVALID_HANDLE_VALUE && GetConsoleMode(input, &originalMode) != 0) {
        if (SetConsoleMode(input, originalMode & ~ENABLE_ECHO_INPUT) == 0) return {};
        std::getline(std::cin, password);
        SetConsoleMode(input, originalMode);
        std::cout << '\n';
        return password;
    }
#else
    if (isatty(STDIN_FILENO)) {
        termios originalMode{};
        if (tcgetattr(STDIN_FILENO, &originalMode) != 0) return {};
        termios hiddenMode = originalMode;
        hiddenMode.c_lflag &= static_cast<tcflag_t>(~ECHO);
        if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &hiddenMode) != 0) return {};
        std::getline(std::cin, password);
        const bool restored = tcsetattr(STDIN_FILENO, TCSAFLUSH, &originalMode) == 0;
        std::cout << '\n';
        return restored ? password : std::string{};
    }
#endif

    std::getline(std::cin, password);
    return password;
}

hms::PatientDraft readPatientDraft() {
    return {
        prompt("Full name: "),
        prompt("Date of birth (YYYY-MM-DD): "),
        prompt("Gender: "),
        prompt("Phone: "),
        prompt("Email (optional): "),
        prompt("Address: "),
        prompt("Emergency contact name: "),
        prompt("Emergency contact phone: "),
    };
}

std::optional<std::string> optionalPrompt(const std::string& label) {
    const std::string value = prompt(label + " (leave blank to keep current): ");
    return value.empty() ? std::nullopt : std::optional<std::string>{value};
}

void updatePatient(hms::HospitalSystem& system, const hms::Session& session) {
    const std::string patientId = prompt("Patient ID: ");
    hms::PatientUpdate update{
        optionalPrompt("Full name"),
        optionalPrompt("Date of birth"),
        optionalPrompt("Gender"),
        optionalPrompt("Phone"),
        optionalPrompt("Email"),
        optionalPrompt("Address"),
        optionalPrompt("Emergency contact name"),
        optionalPrompt("Emergency contact phone"),
    };
    const auto patient = system.updatePatient(session, patientId, update);
    std::cout << "Patient updated successfully. ID remains " << patient.id << ".\n";
}

void printPatient(const hms::PatientSearchResult& patient) {
    std::cout << patient.id << " | " << patient.fullName;
    if (patient.dateOfBirth) std::cout << " | DOB: " << *patient.dateOfBirth;
    if (patient.gender) std::cout << " | Gender: " << *patient.gender;
    if (patient.phone) std::cout << " | Phone: " << *patient.phone;
    if (patient.email) std::cout << " | Email: " << *patient.email;
    if (patient.address) std::cout << " | Address: " << *patient.address;
    if (patient.emergencyContactName && patient.emergencyContactPhone) {
        std::cout << " | Emergency: " << *patient.emergencyContactName << " ("
                  << *patient.emergencyContactPhone << ')';
    }
    std::cout << '\n';
}

void searchPatients(const hms::HospitalSystem& system, const hms::Session& session) {
    std::cout << "Search by: 1) Patient ID  2) Name  3) Contact\n";
    const std::string option = prompt("Choice: ");
    const std::string query = prompt("Search value: ");
    if (option == "1") {
        const auto patient = system.searchPatientById(session, query);
        if (patient) printPatient(*patient);
        else std::cout << "No patient matched that ID.\n";
        return;
    }

    if (option != "2" && option != "3") {
        std::cout << "Invalid search choice. Enter 1, 2, or 3.\n";
        return;
    }
    const auto matches = option == "2" ? system.searchPatientsByName(session, query)
                                        : system.searchPatientsByContact(session, query);
    if (matches.empty()) {
        std::cout << "No patients matched the search.\n";
        return;
    }
    for (const auto& patient : matches) printPatient(patient);
}

bool hasAction(const std::vector<std::string>& actions, const std::string& action) {
    return std::find(actions.begin(), actions.end(), action) != actions.end();
}

std::optional<hms::Role> roleFromChoice(const std::string& choice) {
    if (choice == "1") return hms::Role::Administrator;
    if (choice == "2") return hms::Role::Receptionist;
    if (choice == "3") return hms::Role::Doctor;
    if (choice == "4") return hms::Role::BillingStaff;
    return std::nullopt;
}

void addUser(hms::HospitalSystem& system, const hms::Session& session) {
    const std::string username = prompt("New username: ");
    const std::string password = promptPassword("New password: ");
    std::cout << "Role: 1) Administrator  2) Receptionist  3) Doctor  4) Billing staff\n";
    const auto role = roleFromChoice(prompt("Choose role: "));
    if (!role || !system.addUser(session, username, password, *role)) {
        std::cout << "User not created. Check the role, username, and password length.\n";
        return;
    }
    std::cout << "User created successfully.\n";
}

std::vector<std::string> splitSlots(const std::string& value) {
    std::vector<std::string> slots;
    std::istringstream input(value);
    std::string slot;
    while (std::getline(input, slot, ',')) {
        if (!slot.empty()) slots.push_back(slot);
    }
    return slots;
}

void printDoctor(const hms::Doctor& doctor) {
    std::cout << doctor.id << " | " << doctor.fullName << " | " << doctor.specialization
              << " | " << doctor.department << " | " << doctor.phone;
    for (const auto& slot : doctor.availability) std::cout << " | " << slot;
    std::cout << '\n';
}

void doctorMenu(hms::HospitalSystem& system,
                const hms::Session& session,
                const std::vector<std::string>& actions) {
    const bool canManage = hasAction(actions, "Manage doctors");
    std::cout << (canManage ? "1) Add  2) Update  " : "")
              << "3) Search  4) List  0) Back\n";
    const std::string choice = prompt("Doctor choice: ");
    if (choice == "0") return;
    if (choice == "1" && canManage) {
        hms::DoctorDraft draft{
            prompt("Full name: "), prompt("Specialization: "), prompt("Department: "),
            prompt("Phone: "), prompt("Email: "),
            splitSlots(prompt("Availability slots (YYYY-MM-DD HH:MM, comma-separated): "))};
        const auto doctor = system.addDoctor(session, draft);
        std::cout << "Doctor created successfully with ID " << doctor.id << ".\n";
        return;
    }
    if (choice == "2" && canManage) {
        const std::string id = prompt("Doctor ID: ");
        hms::DoctorUpdate update;
        update.fullName = optionalPrompt("Full name");
        update.specialization = optionalPrompt("Specialization");
        update.department = optionalPrompt("Department");
        update.phone = optionalPrompt("Phone");
        update.email = optionalPrompt("Email");
        const std::string slots = prompt("Availability slots (blank keeps current): ");
        if (!slots.empty()) update.availability = splitSlots(slots);
        printDoctor(system.updateDoctor(session, id, update));
        std::cout << "Doctor updated successfully.\n";
        return;
    }
    if (choice != "3" && choice != "4") {
        std::cout << "Invalid doctor choice. Choose one of the displayed options.\n";
        return;
    }
    const auto doctors = choice == "3" ? system.searchDoctors(session, prompt("Search: "))
                                         : system.listDoctors(session);
    if (doctors.empty()) std::cout << "No doctors matched.\n";
    for (const auto& doctor : doctors) printDoctor(doctor);
}

void printAppointment(const hms::Appointment& appointment) {
    std::cout << appointment.id << " | Patient " << appointment.patientId << " | Doctor "
              << appointment.doctorId << " | " << appointment.date << ' ' << appointment.time
              << " | " << appointment.purpose << " | " << hms::toString(appointment.status)
              << '\n';
}

void appointmentMenu(hms::HospitalSystem& system,
                     const hms::Session& session,
                     const std::vector<std::string>& actions) {
    const bool canManage = hasAction(actions, "Manage appointments");
    std::cout << (canManage ? "1) Schedule  4) Reschedule  5) Cancel  " : "")
              << "2) Find  3) List  0) Back\n";
    const std::string choice = prompt("Appointment choice: ");
    if (choice == "0") return;
    if (choice == "1" && canManage) {
        hms::AppointmentDraft draft{prompt("Patient ID: "), prompt("Doctor ID: "),
                                    prompt("Date (YYYY-MM-DD): "), prompt("Time (HH:MM): "),
                                    prompt("Purpose: ")};
        const auto appointment = system.scheduleAppointment(session, draft);
        std::cout << "Appointment scheduled successfully with ID " << appointment.id << ".\n";
        return;
    }
    if (choice == "2") {
        const auto appointment = system.findAppointmentById(session, prompt("Appointment ID: "));
        if (appointment) printAppointment(*appointment);
        else std::cout << "No appointment matched that ID.\n";
        return;
    }
    if (choice == "4" && canManage) {
        const std::string id = prompt("Appointment ID: ");
        hms::AppointmentUpdate update;
        update.doctorId = optionalPrompt("Doctor ID");
        update.date = optionalPrompt("Date (YYYY-MM-DD)");
        update.time = optionalPrompt("Time (HH:MM)");
        update.purpose = optionalPrompt("Purpose");
        printAppointment(system.rescheduleAppointment(session, id, update));
        std::cout << "Appointment rescheduled successfully.\n";
        return;
    }
    if (choice == "5" && canManage) {
        printAppointment(system.cancelAppointment(session, prompt("Appointment ID: ")));
        std::cout << "Appointment cancelled successfully and retained in history.\n";
        return;
    }
    if (choice != "3") {
        std::cout << "Invalid appointment choice. Choose one of the displayed options.\n";
        return;
    }
    const auto appointments = system.listAppointments(session);
    if (appointments.empty()) std::cout << "No appointments found.\n";
    for (const auto& appointment : appointments) printAppointment(appointment);
}

}  // namespace

int main() {
    const char* configuredPassword = std::getenv("HMS_ADMIN_PASSWORD");
    if (configuredPassword == nullptr || std::string(configuredPassword).size() < 8) {
        std::cerr << "Set HMS_ADMIN_PASSWORD to at least 8 characters before starting HMS.\n";
        return 1;
    }
    hms::HospitalSystem system({{"admin", configuredPassword, hms::Role::Administrator}},
                               "data/patients.journal");
    std::cout << "Hospital Management System\n";
    const std::string username = prompt("Username: ");
    const std::string password = promptPassword("Password: ");
    const auto session = system.login(username, password);
    if (!session) {
        std::cerr << "Login failed: invalid username or password.\n";
        return 1;
    }

    const auto actions = system.menuFor(*session);
    std::cout << "Welcome, " << session->username << " (" << hms::toString(session->role)
              << ").\nAvailable actions:\n";
    for (const auto& action : actions) {
        std::cout << " - " << action << '\n';
    }

    while (true) {
        std::cout << '\n';
        if (hasAction(actions, "Register patient")) std::cout << "1) Register patient  ";
        if (hasAction(actions, "Update patient")) std::cout << "2) Update patient  ";
        if (hasAction(actions, "Search patients")) std::cout << "3) Search patients  ";
        if (hasAction(actions, "Manage users")) std::cout << "4) Add user  ";
        if (hasAction(actions, "Manage doctors") || hasAction(actions, "View doctors")) {
            std::cout << "5) Doctors  ";
        }
        if (hasAction(actions, "Manage appointments") ||
            hasAction(actions, "View appointments")) {
            std::cout << "6) Appointments  ";
        }
        std::cout << "0) Exit\n";
        const std::string choice = prompt("Choice: ");
        if (choice == "0") break;
        try {
            if (choice == "1" && hasAction(actions, "Register patient")) {
                const auto patient = system.registerPatient(*session, readPatientDraft());
                std::cout << "Patient registered successfully with ID " << patient.id << ".\n";
            } else if (choice == "2" && hasAction(actions, "Update patient")) {
                updatePatient(system, *session);
            } else if (choice == "3" && hasAction(actions, "Search patients")) {
                searchPatients(system, *session);
            } else if (choice == "4" && hasAction(actions, "Manage users")) {
                addUser(system, *session);
            } else if (choice == "5" &&
                       (hasAction(actions, "Manage doctors") || hasAction(actions, "View doctors"))) {
                doctorMenu(system, *session, actions);
            } else if (choice == "6" &&
                       (hasAction(actions, "Manage appointments") ||
                        hasAction(actions, "View appointments"))) {
                appointmentMenu(system, *session, actions);
            } else {
                std::cout << "This action is not available for your role.\n";
            }
        } catch (const std::exception& error) {
            std::cout << "Operation failed: " << error.what() << '\n';
        }
    }
    std::cout << "Goodbye. All completed in-memory operations were closed safely.\n";
    return 0;
}
