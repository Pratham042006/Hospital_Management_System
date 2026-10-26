#include "hms/hospital_system.hpp"

#include <iostream>
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
        const std::string choice = prompt("\n1) Register patient  0) Exit\nChoice: ");
        if (choice == "0") break;
        if (choice != "1") {
            std::cout << "Unknown menu option.\n";
            continue;
        }
        try {
            const auto patient = system.registerPatient(*session, readPatientDraft());
            std::cout << "Patient registered successfully with ID " << patient.id << ".\n";
        } catch (const std::exception& error) {
            std::cout << "Operation failed: " << error.what() << '\n';
        }
    }
    return 0;
}
