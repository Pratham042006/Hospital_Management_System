#pragma once

#include <string>

namespace hms {

struct DoctorDraft {
    std::string fullName;
    std::string specialty;
    std::string phone;
    std::string email;
};

struct Doctor {
    std::string id;
    std::string fullName;
    std::string specialty;
    std::string phone;
    std::string email;
};

enum class AppointmentStatus { Scheduled, Cancelled, Completed };
std::string toString(AppointmentStatus status);

struct AppointmentDraft {
    std::string patientId;
    std::string doctorId;
    std::string date; // YYYY-MM-DD
    std::string time; // HH:MM (24-hour)
    std::string reason;
};

struct Appointment {
    std::string id;
    std::string patientId;
    std::string doctorId;
    std::string date;
    std::string time;
    std::string reason;
    AppointmentStatus status{AppointmentStatus::Scheduled};
};

} // namespace hms
