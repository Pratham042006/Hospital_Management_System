#include "hms/doctor_repository.hpp"

#include <algorithm>
#include <cctype>
#include <iomanip>
#include <regex>
#include <sstream>
#include <stdexcept>

namespace hms {

namespace {

bool isLeapYear(int year) {
    return (year % 400 == 0) || (year % 4 == 0 && year % 100 != 0);
}

bool validDate(const std::string& date) {
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

bool validTime(const std::string& time) {
    static const std::regex format(R"(^\d{2}:\d{2}$)");
    if (!std::regex_match(time, format)) return false;
    const int hour = std::stoi(time.substr(0, 2));
    const int minute = std::stoi(time.substr(3, 2));
    return hour >= 0 && hour <= 23 && minute >= 0 && minute <= 59;
}

bool validSlot(const std::string& slot) {
    return slot.size() == 16 && slot[10] == ' ' && validDate(slot.substr(0, 10)) &&
           validTime(slot.substr(11, 5));
}

bool validEmail(const std::string& email) {
    if (email.empty()) return true;
    const auto at = email.find('@');
    const auto dot = email.rfind('.');
    return at != std::string::npos && at > 0 && dot != std::string::npos && dot > at + 1 &&
           dot + 1 < email.size();
}

}  // namespace

Doctor DoctorRepository::addDoctor(const DoctorDraft& draft) {
    Doctor doctor{generateDoctorId(), trim(draft.fullName), trim(draft.specialization),
                  trim(draft.department), trim(draft.phone), trim(draft.email),
                  draft.availability};
    for (auto& slot : doctor.availability) slot = trim(slot);
    validate(doctor);
    ensureUniqueContacts(doctor);
    const std::size_t index = doctors_.size();
    doctors_.push_back(doctor);
    addToIndexes(doctors_.back(), index);
    return doctor;
}

Doctor DoctorRepository::updateDoctor(const std::string& doctorId, const DoctorUpdate& update) {
    const auto located = idIndex_.find(doctorId);
    if (located == idIndex_.end()) {
        throw std::invalid_argument("Doctor ID was not found: " + doctorId);
    }
    Doctor candidate = doctors_.at(located->second);
    if (update.fullName) candidate.fullName = trim(*update.fullName);
    if (update.specialization) candidate.specialization = trim(*update.specialization);
    if (update.department) candidate.department = trim(*update.department);
    if (update.phone) candidate.phone = trim(*update.phone);
    if (update.email) candidate.email = trim(*update.email);
    if (update.availability) {
        candidate.availability = *update.availability;
        for (auto& slot : candidate.availability) slot = trim(slot);
    }
    validate(candidate);
    ensureUniqueContacts(candidate, doctorId);
    Doctor& stored = doctors_.at(located->second);
    removeContactIndexes(stored);
    stored = candidate;
    addToIndexes(stored, located->second);
    return stored;
}

std::optional<Doctor> DoctorRepository::findById(const std::string& doctorId) const {
    const auto located = idIndex_.find(doctorId);
    if (located == idIndex_.end()) return std::nullopt;
    return doctors_.at(located->second);
}

std::vector<Doctor> DoctorRepository::search(const std::string& query) const {
    const std::string normalized = normalizeText(query);
    const std::string phoneQuery = normalizePhone(query);
    std::vector<Doctor> matches;
    if (normalized.empty()) return matches;
    for (const auto& doctor : doctors_) {
        if (normalizeText(doctor.id).find(normalized) != std::string::npos ||
            normalizeText(doctor.fullName).find(normalized) != std::string::npos ||
            normalizeText(doctor.specialization).find(normalized) != std::string::npos ||
            normalizeText(doctor.department).find(normalized) != std::string::npos ||
            (!phoneQuery.empty() && normalizePhone(doctor.phone).find(phoneQuery) != std::string::npos) ||
            normalizeText(doctor.email).find(normalized) != std::string::npos) {
            matches.push_back(doctor);
        }
    }
    return matches;
}

std::vector<Doctor> DoctorRepository::list() const {
    return doctors_;
}

bool DoctorRepository::isAvailable(const std::string& doctorId,
                                   const std::string& date,
                                   const std::string& time) const {
    const auto doctor = findById(doctorId);
    if (!doctor) return false;
    const std::string requested = date + ' ' + time;
    return std::find(doctor->availability.begin(), doctor->availability.end(), requested) !=
           doctor->availability.end();
}

std::string DoctorRepository::trim(const std::string& value) {
    const auto first = std::find_if_not(value.begin(), value.end(), [](unsigned char ch) {
        return std::isspace(ch) != 0;
    });
    if (first == value.end()) return {};
    const auto last = std::find_if_not(value.rbegin(), value.rend(), [](unsigned char ch) {
        return std::isspace(ch) != 0;
    }).base();
    return std::string(first, last);
}

std::string DoctorRepository::normalizeText(const std::string& value) {
    std::string normalized = trim(value);
    std::transform(normalized.begin(), normalized.end(), normalized.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return normalized;
}

std::string DoctorRepository::normalizePhone(const std::string& value) {
    std::string normalized;
    for (const unsigned char ch : value) {
        if (std::isdigit(ch) != 0) normalized.push_back(static_cast<char>(ch));
    }
    return normalized;
}

void DoctorRepository::validate(const Doctor& doctor) {
    if (doctor.fullName.empty() || doctor.specialization.empty() || doctor.department.empty()) {
        throw std::invalid_argument("Doctor name, specialization, and department are required.");
    }
    const auto phone = normalizePhone(doctor.phone);
    if (phone.size() < 7 || phone.size() > 15) {
        throw std::invalid_argument("Doctor phone must contain 7 to 15 digits.");
    }
    if (!validEmail(doctor.email)) throw std::invalid_argument("Doctor email is invalid.");
    if (doctor.availability.empty()) {
        throw std::invalid_argument("At least one doctor availability slot is required.");
    }
    for (const auto& slot : doctor.availability) {
        if (!validSlot(slot)) {
            throw std::invalid_argument("Availability slots must use YYYY-MM-DD HH:MM.");
        }
    }
}

void DoctorRepository::ensureUniqueContacts(
    const Doctor& doctor,
    const std::optional<std::string>& ignoredDoctorId) const {
    const auto phoneOwner = phoneIndex_.find(normalizePhone(doctor.phone));
    if (phoneOwner != phoneIndex_.end() &&
        (!ignoredDoctorId || phoneOwner->second != *ignoredDoctorId)) {
        throw std::invalid_argument("A doctor with this phone number already exists.");
    }
    const std::string email = normalizeText(doctor.email);
    if (!email.empty()) {
        const auto emailOwner = emailIndex_.find(email);
        if (emailOwner != emailIndex_.end() &&
            (!ignoredDoctorId || emailOwner->second != *ignoredDoctorId)) {
            throw std::invalid_argument("A doctor with this email already exists.");
        }
    }
}

std::string DoctorRepository::generateDoctorId() {
    std::ostringstream id;
    id << 'D' << std::setw(6) << std::setfill('0') << nextDoctorNumber_++;
    return id.str();
}

void DoctorRepository::addToIndexes(const Doctor& doctor, std::size_t index) {
    idIndex_[doctor.id] = index;
    phoneIndex_[normalizePhone(doctor.phone)] = doctor.id;
    const std::string email = normalizeText(doctor.email);
    if (!email.empty()) emailIndex_[email] = doctor.id;
}

void DoctorRepository::removeContactIndexes(const Doctor& doctor) {
    phoneIndex_.erase(normalizePhone(doctor.phone));
    const std::string email = normalizeText(doctor.email);
    if (!email.empty()) emailIndex_.erase(email);
}

}  // namespace hms
