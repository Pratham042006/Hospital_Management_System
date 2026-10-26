#pragma once

#include <optional>
#include <string>

namespace hms {

struct PatientDraft {
    std::string fullName;
    std::string dateOfBirth;
    std::string gender;
    std::string phone;
    std::string email;
    std::string address;
    std::string emergencyContactName;
    std::string emergencyContactPhone;
};

struct Patient {
    std::string id;
    std::string fullName;
    std::string dateOfBirth;
    std::string gender;
    std::string phone;
    std::string email;
    std::string address;
    std::string emergencyContactName;
    std::string emergencyContactPhone;
};

struct PatientUpdate {
    std::optional<std::string> fullName;
    std::optional<std::string> dateOfBirth;
    std::optional<std::string> gender;
    std::optional<std::string> phone;
    std::optional<std::string> email;
    std::optional<std::string> address;
    std::optional<std::string> emergencyContactName;
    std::optional<std::string> emergencyContactPhone;
};

}  // namespace hms
