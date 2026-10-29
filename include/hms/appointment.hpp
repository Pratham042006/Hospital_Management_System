#pragma once

#include <optional>
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

struct AppointmentUpdate {
    std::optional<std::string> doctorId;
    std::optional<std::string> date;
    std::optional<std::string> time;
    std::optional<std::string> purpose;
};

}  // namespace hms
