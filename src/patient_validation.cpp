#include "hms/patient_validation.hpp"

#include <algorithm>
#include <cctype>
#include <regex>
#include <stdexcept>

namespace hms::validation {
namespace {

bool isLeapYear(int year) {
    return (year % 400 == 0) || (year % 4 == 0 && year % 100 != 0);
}

bool isValidDate(const std::string& date) {
    static const std::regex format(R"(^\d{4}-\d{2}-\d{2}$)");
    if (!std::regex_match(date, format)) return false;

    const int year = std::stoi(date.substr(0, 4));
    const int month = std::stoi(date.substr(5, 2));
    const int day = std::stoi(date.substr(8, 2));
    if (year < 1900 || month < 1 || month > 12 || day < 1) return false;

    const int daysInMonth[] = {
        31, isLeapYear(year) ? 29 : 28, 31, 30, 31, 30,
        31, 31, 30, 31, 30, 31,
    };
    return day <= daysInMonth[month - 1];
}

bool isValidEmail(const std::string& email) {
    if (email.empty()) return true;
    const auto at = email.find('@');
    const auto dot = email.rfind('.');
    return at != std::string::npos && at > 0 && dot != std::string::npos && dot > at + 1 &&
           dot + 1 < email.size();
}

}  // namespace

std::string trim(const std::string& value) {
    const auto first = std::find_if_not(value.begin(), value.end(), [](unsigned char ch) {
        return std::isspace(ch) != 0;
    });
    if (first == value.end()) return {};
    const auto last = std::find_if_not(value.rbegin(), value.rend(), [](unsigned char ch) {
        return std::isspace(ch) != 0;
    }).base();
    return std::string(first, last);
}

std::string normalizeText(const std::string& value) {
    std::string normalized = trim(value);
    std::transform(normalized.begin(), normalized.end(), normalized.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return normalized;
}

std::string normalizePhone(const std::string& value) {
    std::string normalized;
    for (const unsigned char ch : value) {
        if (std::isdigit(ch) != 0) normalized.push_back(static_cast<char>(ch));
    }
    return normalized;
}

void validatePatient(const Patient& patient) {
    if (patient.fullName.empty()) throw std::invalid_argument("Patient name is required.");
    if (!isValidDate(patient.dateOfBirth)) {
        throw std::invalid_argument("Date of birth must be a valid YYYY-MM-DD date.");
    }
    if (patient.gender.empty()) throw std::invalid_argument("Gender is required.");
    const std::string phone = normalizePhone(patient.phone);
    if (phone.size() < 7 || phone.size() > 15) {
        throw std::invalid_argument("Phone number must contain 7 to 15 digits.");
    }
    if (!isValidEmail(patient.email)) throw std::invalid_argument("Email address is invalid.");
    if (patient.address.empty()) throw std::invalid_argument("Address is required.");
    if (patient.emergencyContactName.empty()) {
        throw std::invalid_argument("Emergency contact name is required.");
    }
    const std::string emergencyPhone = normalizePhone(patient.emergencyContactPhone);
    if (emergencyPhone.size() < 7 || emergencyPhone.size() > 15) {
        throw std::invalid_argument("Emergency contact phone must contain 7 to 15 digits.");
    }
}

}  // namespace hms::validation
