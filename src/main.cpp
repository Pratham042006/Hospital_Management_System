#include "hms/hospital_system.hpp"

#include <algorithm>
#include <iostream>
#include <optional>
#include <string>
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
        if (SetConsoleMode(input, originalMode & ~ENABLE_ECHO_INPUT) == 0) {
            std::cerr << "\nCould not safely read the password.\n";
            return {};
        }
        std::getline(std::cin, password);
        SetConsoleMode(input, originalMode);
        std::cout << '\n';
        return password;
    }
#else
    if (isatty(STDIN_FILENO)) {
        termios originalMode{};
        if (tcgetattr(STDIN_FILENO, &originalMode) != 0) {
            std::cerr << "\nCould not safely read the password.\n";
            return {};
        }
        termios hiddenMode = originalMode;
        hiddenMode.c_lflag &= static_cast<tcflag_t>(~ECHO);
        if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &hiddenMode) != 0) {
            std::cerr << "\nCould not safely read the password.\n";
            return {};
        }
        std::getline(std::cin, password);
        const bool restored = tcsetattr(STDIN_FILENO, TCSAFLUSH, &originalMode) == 0;
        std::cout << '\n';
        if (!restored) {
            std::cerr << "Could not restore terminal echo; password login aborted.\n";
            return {};
        }
        return password;
    }
#endif

    // Redirected input is not echoed by the terminal, so line input is safe here.
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
    // The service has already removed fields this role cannot view.
    std::cout << patient.id << " | " << patient.fullName;
    if (patient.dateOfBirth) std::cout << " | DOB: " << *patient.dateOfBirth;
    if (patient.gender) std::cout << " | Gender: " << *patient.gender;
    if (patient.phone) std::cout << " | Phone: " << *patient.phone;
    if (patient.email) std::cout << " | Email: " << *patient.email;
    if (patient.address) std::cout << " | Address: " << *patient.address;
    if (patient.emergencyContactName && patient.emergencyContactPhone) {
        std::cout << " | Emergency contact: " << *patient.emergencyContactName << " ("
                  << *patient.emergencyContactPhone << ')';
    }
    std::cout << '\n';
}

bool hasAction(const std::vector<std::string>& actions, const std::string& action) {
    return std::find(actions.begin(), actions.end(), action) != actions.end();
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

    const auto matches = option == "2" ? system.searchPatientsByName(session, query)
                                         : system.searchPatientsByContact(session, query);
    if (matches.empty()) {
        std::cout << "No patients matched the search.\n";
        return;
    }
    for (const auto& patient : matches) printPatient(patient);
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
    if (!role) {
        std::cout << "User not created: choose a valid role.\n";
        return;
    }
    if (!system.addUser(session, username, password, *role)) {
        std::cout << "User not created: username may already exist or password must be at least 8 characters.\n";
        return;
    }
    std::cout << "User created.\n";
}

void printMenu(const std::vector<std::string>& actions) {
    std::cout << '\n';
    if (hasAction(actions, "Register patient")) std::cout << "1) Register patient  ";
    if (hasAction(actions, "Update patient")) std::cout << "2) Update patient  ";
    if (hasAction(actions, "Search patients")) std::cout << "3) Search patients  ";
    if (hasAction(actions, "Manage users")) std::cout << "4) Add user  ";
    std::cout << "0) Exit\n";
}

}  // namespace

int main() {
    hms::HospitalSystem system;
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
        printMenu(actions);
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
            } else {
                std::cout << "This action is not available for your role.\n";
            }
        } catch (const std::exception& error) {
            std::cout << "Operation failed: " << error.what() << '\n';
        }
    }
    return 0;
}
