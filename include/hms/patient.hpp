#pragma once

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

}  // namespace hms
