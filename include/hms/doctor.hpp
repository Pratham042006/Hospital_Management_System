#pragma once

#include <optional>
#include <string>
#include <vector>

namespace hms {

struct DoctorDraft {
    std::string fullName;
    std::string specialization;
    std::string department;
    std::string phone;
    std::string email;
    std::vector<std::string> availability;
};

struct Doctor {
    std::string id;
    std::string fullName;
    std::string specialization;
    std::string department;
    std::string phone;
    std::string email;
    std::vector<std::string> availability;
};

struct DoctorUpdate {
    std::optional<std::string> fullName;
    std::optional<std::string> specialization;
    std::optional<std::string> department;
    std::optional<std::string> phone;
    std::optional<std::string> email;
    std::optional<std::vector<std::string>> availability;
};

}  // namespace hms
