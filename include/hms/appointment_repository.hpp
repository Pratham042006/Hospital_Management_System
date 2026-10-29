#pragma once

#include "hms/appointment.hpp"

#include <cstddef>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace hms {

class AppointmentRepository {
public:
    Appointment schedule(const AppointmentDraft& draft);
    std::optional<Appointment> findById(const std::string& appointmentId) const;
    std::vector<Appointment> list() const;

    static bool isValidDate(const std::string& date);
    static bool isValidTime(const std::string& time);
    static bool isPastDate(const std::string& date);

private:
    std::vector<Appointment> appointments_;
    std::unordered_map<std::string, std::size_t> idIndex_;
    std::size_t nextAppointmentNumber_{1};

    static std::string trim(const std::string& value);
    std::string generateAppointmentId();
};

}  // namespace hms
