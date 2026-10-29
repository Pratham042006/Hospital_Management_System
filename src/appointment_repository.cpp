#include "hms/appointment_repository.hpp"

#include <algorithm>
#include <cctype>
#include <ctime>
#include <iomanip>
#include <regex>
#include <sstream>
#include <stdexcept>

namespace hms {

namespace {

bool isLeapYear(int year) {
    return (year % 400 == 0) || (year % 4 == 0 && year % 100 != 0);
}

std::string todayIso() {
    const std::time_t current = std::time(nullptr);
    std::tm local{};
#if defined(_WIN32)
    localtime_s(&local, &current);
#else
    localtime_r(&current, &local);
#endif
    std::ostringstream date;
    date << std::put_time(&local, "%Y-%m-%d");
    return date.str();
}

}  // namespace

std::string toString(AppointmentStatus status) {
    switch (status) {
        case AppointmentStatus::Scheduled:
            return "Scheduled";
        case AppointmentStatus::Cancelled:
            return "Cancelled";
        case AppointmentStatus::Completed:
            return "Completed";
    }
    return "Unknown";
}

Appointment AppointmentRepository::schedule(const AppointmentDraft& draft) {
    Appointment appointment{generateAppointmentId(), trim(draft.patientId), trim(draft.doctorId),
                            trim(draft.date), trim(draft.time), trim(draft.purpose),
                            AppointmentStatus::Scheduled};
    if (appointment.patientId.empty() || appointment.doctorId.empty()) {
        throw std::invalid_argument("Patient ID and Doctor ID are required.");
    }
    if (!isValidDate(appointment.date)) {
        throw std::invalid_argument("Appointment date must be a real YYYY-MM-DD date.");
    }
    if (isPastDate(appointment.date)) {
        throw std::invalid_argument("Appointment date cannot be in the past.");
    }
    if (!isValidTime(appointment.time)) {
        throw std::invalid_argument("Appointment time must be a valid HH:MM time.");
    }
    if (appointment.purpose.empty()) {
        throw std::invalid_argument("Appointment purpose is required.");
    }
    const auto conflict = std::find_if(
        appointments_.begin(), appointments_.end(), [&](const Appointment& existing) {
            return existing.status == AppointmentStatus::Scheduled &&
                   existing.date == appointment.date && existing.time == appointment.time &&
                   (existing.patientId == appointment.patientId ||
                    existing.doctorId == appointment.doctorId);
        });
    if (conflict != appointments_.end()) {
        throw std::invalid_argument("The patient or doctor already has an appointment at this time.");
    }
    const std::size_t index = appointments_.size();
    appointments_.push_back(appointment);
    idIndex_[appointment.id] = index;
    return appointment;
}

std::optional<Appointment> AppointmentRepository::findById(
    const std::string& appointmentId) const {
    const auto located = idIndex_.find(appointmentId);
    if (located == idIndex_.end()) return std::nullopt;
    return appointments_.at(located->second);
}

std::vector<Appointment> AppointmentRepository::list() const {
    return appointments_;
}

bool AppointmentRepository::isValidDate(const std::string& date) {
    static const std::regex format(R"(^\d{4}-\d{2}-\d{2}$)");
    if (!std::regex_match(date, format)) return false;
    const int year = std::stoi(date.substr(0, 4));
    const int month = std::stoi(date.substr(5, 2));
    const int day = std::stoi(date.substr(8, 2));
    if (year < 1900 || month < 1 || month > 12 || day < 1) return false;
    const int days[] = {31, isLeapYear(year) ? 29 : 28, 31, 30, 31, 30,
                        31, 31, 30, 31, 30, 31};
    return day <= days[month - 1];
}

bool AppointmentRepository::isValidTime(const std::string& time) {
    static const std::regex format(R"(^\d{2}:\d{2}$)");
    if (!std::regex_match(time, format)) return false;
    const int hour = std::stoi(time.substr(0, 2));
    const int minute = std::stoi(time.substr(3, 2));
    return hour >= 0 && hour <= 23 && minute >= 0 && minute <= 59;
}

bool AppointmentRepository::isPastDate(const std::string& date) {
    return isValidDate(date) && date < todayIso();
}

std::string AppointmentRepository::trim(const std::string& value) {
    const auto first = std::find_if_not(value.begin(), value.end(), [](unsigned char ch) {
        return std::isspace(ch) != 0;
    });
    if (first == value.end()) return {};
    const auto last = std::find_if_not(value.rbegin(), value.rend(), [](unsigned char ch) {
        return std::isspace(ch) != 0;
    }).base();
    return std::string(first, last);
}

std::string AppointmentRepository::generateAppointmentId() {
    std::ostringstream id;
    id << 'A' << std::setw(6) << std::setfill('0') << nextAppointmentNumber_++;
    return id.str();
}

}  // namespace hms
