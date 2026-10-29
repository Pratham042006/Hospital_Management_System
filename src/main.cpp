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

void printPatient(const hms::Patient& patient) {
    std::cout << patient.id << " | " << patient.fullName << " | " << patient.dateOfBirth
              << " | " << patient.gender << " | " << patient.phone << " | "
              << patient.email << " | " << patient.address << '\n';
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


void manageDoctors(hms::HospitalSystem& system, const hms::Session& session) {
    const std::string name = prompt("Doctor full name: ");
    const std::string specialty = prompt("Specialty: ");
    const std::string phone = prompt("Phone: ");
    const std::string email = prompt("Email (optional): ");
    const auto doctor = system.addDoctor(session, {name, specialty, phone, email});
    std::cout << "Doctor added: " << doctor.id << " | " << doctor.fullName << " | " << doctor.specialty << '\n';
}

void listDoctors(hms::HospitalSystem& system, const hms::Session& session) {
    const auto doctors = system.listDoctors(session);
    if (doctors.empty()) { std::cout << "No doctors registered yet.\n"; return; }
    for (const auto& doctor : doctors)
        std::cout << doctor.id << " | " << doctor.fullName << " | " << doctor.specialty << " | " << doctor.phone << '\n';
}

void scheduleAppointment(hms::HospitalSystem& system, const hms::Session& session) {
    hms::AppointmentDraft draft;
    draft.patientId = prompt("Patient ID: ");
    draft.doctorId = prompt("Doctor ID: ");
    draft.date = prompt("Date (YYYY-MM-DD): ");
    draft.time = prompt("Time (HH:MM, 24-hour): ");
    draft.reason = prompt("Reason: ");
    const auto appointment = system.scheduleAppointment(session, draft);
    std::cout << "Appointment scheduled: " << appointment.id << '\n';
}

void modifyAppointment(hms::HospitalSystem& system, const hms::Session& session) {
    const std::string id = prompt("Appointment ID: ");
    hms::AppointmentDraft changes;
    changes.patientId = prompt("New patient ID (blank keeps current): ");
    changes.doctorId = prompt("New doctor ID (blank keeps current): ");
    changes.date = prompt("New date YYYY-MM-DD (blank keeps current): ");
    changes.time = prompt("New time HH:MM (blank keeps current): ");
    changes.reason = prompt("New reason (blank keeps current): ");
    const auto appointment = system.modifyAppointment(session, id, changes);
    std::cout << "Appointment " << appointment.id << " updated successfully.\n";
}

void listAppointments(hms::HospitalSystem& system, const hms::Session& session) {
    const auto appointments = system.listAppointments(session);
    if (appointments.empty()) { std::cout << "No appointments found.\n"; return; }
    for (const auto& item : appointments)
        std::cout << item.id << " | Patient " << item.patientId << " | Doctor " << item.doctorId
                  << " | " << item.date << ' ' << item.time << " | " << item.reason
                  << " | " << hms::toString(item.status) << '\n';
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
            "\n1) Register patient  2) Update patient  3) Search patients  4) Add doctor  5) List doctors"
            "  6) Schedule appointment  7) Modify appointment  8) List appointments  0) Exit\nChoice: ");
        if (choice == "0") break;
        if (choice != "1" && choice != "2" && choice != "3" && choice != "4" && choice != "5" && choice != "6" && choice != "7" && choice != "8") {
            std::cout << "Unknown menu option.\n";
            continue;
        }
        try {
            if (choice == "1") {
                const auto patient = system.registerPatient(*session, readPatientDraft());
                std::cout << "Patient registered successfully with ID " << patient.id << ".\n";
            } else if (choice == "2") {
                updatePatient(system, *session);
            } else if (choice == "3") {
                searchPatients(system, *session);
            } else if (choice == "4") {
                manageDoctors(system, *session);
            } else if (choice == "5") {
                listDoctors(system, *session);
            } else if (choice == "6") {
                scheduleAppointment(system, *session);
            } else if (choice == "7") {
                modifyAppointment(system, *session);
            } else {
                listAppointments(system, *session);
            }
        } catch (const std::exception& error) {
            std::cout << "Operation failed: " << error.what() << '\n';
        }
    }
    return 0;
}
