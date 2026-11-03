#include "hms/validation_service.hpp"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <regex>
#include <sstream>

namespace hms {

namespace {

bool isLeapYear(int year) {
    return (year % 400 == 0) || (year % 4 == 0 && year % 100 != 0);
}

}  // namespace

bool ValidationService::isNonEmpty(const std::string& value) {
    return std::any_of(value.begin(), value.end(), [](unsigned char ch) {
        return std::isspace(ch) == 0;
    });
}

bool ValidationService::isValidId(const std::string& prefix, const std::string& id) {
    if (id.rfind(prefix, 0) != 0) {
        return false;
    }
    const std::string remainder = id.substr(prefix.size());
    if (remainder.empty()) {
        return false;
    }
    return std::all_of(remainder.begin(), remainder.end(), [](unsigned char ch) {
        return std::isdigit(ch) != 0;
    });
}

bool ValidationService::isValidDate(const std::string& date) {
    if (date.size() != 10 || date[4] != '-' || date[7] != '-') {
        return false;
    }
    try {
        int year = std::stoi(date.substr(0, 4));
        int month = std::stoi(date.substr(5, 2));
        int day = std::stoi(date.substr(8, 2));
        if (year < 1900 || year > 2100 || month < 1 || month > 12 || day < 1) {
            return false;
        }
        const int daysInMonth[] = {
            31, isLeapYear(year) ? 29 : 28, 31, 30, 31, 30,
            31, 31, 30, 31, 30, 31,
        };
        return day <= daysInMonth[month - 1];
    } catch (...) {
        return false;
    }
}

bool ValidationService::isFutureOrTodayDate(const std::string& date) {
    if (!isValidDate(date)) {
        return false;
    }
    const auto now = std::chrono::system_clock::now();
    const std::time_t tt = std::chrono::system_clock::to_time_t(now);
    std::tm currentTm{};
#if defined(_WIN32)
    localtime_s(&currentTm, &tt);
#else
    localtime_r(&tt, &currentTm);
#endif
    std::ostringstream todayStream;
    todayStream << std::setfill('0')
                << std::setw(4) << (currentTm.tm_year + 1900) << '-'
                << std::setw(2) << (currentTm.tm_mon + 1) << '-'
                << std::setw(2) << currentTm.tm_mday;
    return date >= todayStream.str();
}

bool ValidationService::isValidTime(const std::string& time) {
    if (time.size() != 5 || time[2] != ':') {
        return false;
    }
    try {
        int hour = std::stoi(time.substr(0, 2));
        int minute = std::stoi(time.substr(3, 2));
        return hour >= 0 && hour <= 23 && minute >= 0 && minute <= 59;
    } catch (...) {
        return false;
    }
}

bool ValidationService::isValidPhone(const std::string& phone) {
    std::string digits;
    for (const unsigned char ch : phone) {
        if (std::isdigit(ch) != 0) {
            digits.push_back(static_cast<char>(ch));
        }
    }
    return digits.size() >= 7 && digits.size() <= 15;
}

bool ValidationService::isValidEmail(const std::string& email) {
    if (email.empty()) {
        return true;
    }
    const auto at = email.find('@');
    const auto dot = email.rfind('.');
    return at != std::string::npos && at > 0 && dot != std::string::npos &&
           dot > at + 1 && dot + 1 < email.size();
}

bool ValidationService::isPositiveAmount(double amount) {
    return amount > 0.0;
}

bool ValidationService::isNonNegativeAmount(double amount) {
    return amount >= 0.0;
}

std::string ValidationService::sanitizeInput(const std::string& input) {
    const auto first = std::find_if_not(input.begin(), input.end(), [](unsigned char ch) {
        return std::isspace(ch) != 0;
    });
    if (first == input.end()) {
        return {};
    }
    const auto last = std::find_if_not(input.rbegin(), input.rend(), [](unsigned char ch) {
        return std::isspace(ch) != 0;
    }).base();
    return std::string(first, last);
}

}  // namespace hms
