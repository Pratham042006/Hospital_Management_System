#pragma once

#include <string>
#include <vector>

namespace hms {

class ValidationService {
public:
    static bool isNonEmpty(const std::string& value);
    static bool isValidId(const std::string& prefix, const std::string& id);
    static bool isValidDate(const std::string& date);
    static bool isFutureOrTodayDate(const std::string& date);
    static bool isValidTime(const std::string& time);
    static bool isValidPhone(const std::string& phone);
    static bool isValidEmail(const std::string& email);
    static bool isPositiveAmount(double amount);
    static bool isNonNegativeAmount(double amount);

    static std::string sanitizeInput(const std::string& input);
};

}  // namespace hms
