#include "hms/hospital_system.hpp"

#include <iostream>
#include <optional>
#include <string>

namespace {

std::string prompt(const std::string& label) {
    std::cout << label;
    std::string value;
    std::getline(std::cin, value);
    return value;
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

}  // namespace

int main() {
    hms::HospitalSystem system;
    std::cout << "Hospital Management System\n";
    const std::string username = prompt("Username: ");
    const std::string password = prompt("Password: ");
    const auto session = system.login(username, password);
    if (!session) {
        std::cerr << "Login failed: invalid username or password.\n";
        return 1;
    }

    std::cout << "Welcome, " << session->username << " (" << hms::toString(session->role)
              << ").\nAvailable actions:\n";
    for (const auto& action : system.menuFor(*session)) {
        std::cout << " - " << action << '\n';
    }

    while (true) {
        const std::string choice = prompt(
            "\n1) Register patient  2) Update patient  0) Exit\nChoice: ");
        if (choice == "0") break;
        if (choice != "1" && choice != "2") {
            std::cout << "Unknown menu option.\n";
            continue;
        }
        try {
            if (choice == "1") {
                const auto patient = system.registerPatient(*session, readPatientDraft());
                std::cout << "Patient registered successfully with ID " << patient.id << ".\n";
            } else {
                updatePatient(system, *session);
            }
        } catch (const std::exception& error) {
            std::cout << "Operation failed: " << error.what() << '\n';
        }
    }
    return 0;
}
