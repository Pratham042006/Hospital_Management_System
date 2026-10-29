#pragma once

#include <string>

namespace hms {

enum class AppointmentStatus { Scheduled, Cancelled, Completed };

std::string toString(AppointmentStatus status);

struct AppointmentDraft {
    std::string patientId;
    std::string doctorId;
    std::string date;
    std::string time;
    std::string purpose;
};

struct Appointment {
    std::string id;
    std::string patientId;
    std::string doctorId;
    std::string date;
    std::string time;
    std::string purpose;
    AppointmentStatus status{AppointmentStatus::Scheduled};
};

}  // namespace hms
