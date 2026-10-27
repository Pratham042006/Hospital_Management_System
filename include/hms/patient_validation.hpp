#pragma once

#include "hms/patient.hpp"

#include <string>

namespace hms::validation {

std::string trim(const std::string& value);
std::string normalizeText(const std::string& value);
std::string normalizePhone(const std::string& value);
void validatePatient(const Patient& patient);

}  // namespace hms::validation
