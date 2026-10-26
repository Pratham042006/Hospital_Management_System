#include "hms/patient_repository.hpp"

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

Patient PatientRepository::registerPatient(const PatientDraft& draft) {
    Patient patient{
        generatePatientId(),
        trim(draft.fullName),
        trim(draft.dateOfBirth),
        trim(draft.gender),
        trim(draft.phone),
        trim(draft.email),
        trim(draft.address),
        trim(draft.emergencyContactName),
        trim(draft.emergencyContactPhone),
    };

    validate(patient);
    ensureUniqueContacts(patient);
    const std::size_t index = patients_.size();
    patients_.push_back(patient);
    addToIndexes(patients_.back(), index);
    return patient;
}

Patient PatientRepository::updatePatient(const std::string& patientId,
                                         const PatientUpdate& update) {
    const std::size_t index = indexForId(patientId);
    Patient candidate = patients_.at(index);
    if (update.fullName) candidate.fullName = trim(*update.fullName);
    if (update.dateOfBirth) candidate.dateOfBirth = trim(*update.dateOfBirth);
    if (update.gender) candidate.gender = trim(*update.gender);
    if (update.phone) candidate.phone = trim(*update.phone);
    if (update.email) candidate.email = trim(*update.email);
    if (update.address) candidate.address = trim(*update.address);
    if (update.emergencyContactName) {
        candidate.emergencyContactName = trim(*update.emergencyContactName);
    }
    if (update.emergencyContactPhone) {
        candidate.emergencyContactPhone = trim(*update.emergencyContactPhone);
    }

    validate(candidate);
    ensureUniqueContacts(candidate, patientId);

    Patient& stored = patients_.at(index);
    removeContactIndexes(stored);
    stored = candidate;
    addToIndexes(stored, index);
    return stored;
}

std::optional<Patient> PatientRepository::findById(const std::string& patientId) const {
    const auto found = idIndex_.find(patientId);
    if (found == idIndex_.end()) return std::nullopt;
    return patients_.at(found->second);
}

std::vector<Patient> PatientRepository::findByName(const std::string& name) const {
    const std::string query = normalizeText(name);
    std::vector<Patient> matches;
    if (query.empty()) return matches;
    for (const auto& patient : patients_) {
        if (normalizeText(patient.fullName).find(query) != std::string::npos) {
            matches.push_back(patient);
        }
    }
    return matches;
}

std::vector<Patient> PatientRepository::findByContact(const std::string& contact) const {
    const std::string phone = normalizePhone(contact);
    const std::string email = normalizeText(contact);
    std::vector<Patient> matches;

    if (!phone.empty()) {
        const auto phoneMatch = phoneIndex_.find(phone);
        if (phoneMatch != phoneIndex_.end()) {
            const auto patient = findById(phoneMatch->second);
            if (patient) matches.push_back(*patient);
        }
    }
    const auto emailMatch = emailIndex_.find(email);
    if (emailMatch != emailIndex_.end()) {
        const auto patient = findById(emailMatch->second);
        if (patient && (matches.empty() || matches.front().id != patient->id)) {
            matches.push_back(*patient);
        }
    }
    return matches;
}

std::size_t PatientRepository::size() const noexcept {
    return patients_.size();
}

std::string PatientRepository::trim(const std::string& value) {
    const auto first = std::find_if_not(value.begin(), value.end(), [](unsigned char ch) {
        return std::isspace(ch) != 0;
    });
    if (first == value.end()) return {};
    const auto last = std::find_if_not(value.rbegin(), value.rend(), [](unsigned char ch) {
        return std::isspace(ch) != 0;
    }).base();
    return std::string(first, last);
}

std::string PatientRepository::normalizeText(const std::string& value) {
    std::string normalized = trim(value);
    std::transform(normalized.begin(), normalized.end(), normalized.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return normalized;
}

std::string PatientRepository::normalizePhone(const std::string& value) {
    std::string normalized;
    for (const unsigned char ch : value) {
        if (std::isdigit(ch) != 0) normalized.push_back(static_cast<char>(ch));
    }
    return normalized;
}

void PatientRepository::validate(const Patient& patient) {
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

std::string PatientRepository::generatePatientId() {
    std::ostringstream id;
    id << 'P' << std::setw(6) << std::setfill('0') << nextPatientNumber_++;
    return id.str();
}

std::size_t PatientRepository::indexForId(const std::string& patientId) const {
    const auto found = idIndex_.find(patientId);
    if (found == idIndex_.end()) {
        throw std::invalid_argument("Patient ID was not found: " + patientId);
    }
    return found->second;
}

void PatientRepository::ensureUniqueContacts(
    const Patient& patient,
    const std::optional<std::string>& ignoredPatientId) const {
    const std::string phone = normalizePhone(patient.phone);
    const std::string email = normalizeText(patient.email);
    const auto phoneOwner = phoneIndex_.find(phone);
    if (phoneOwner != phoneIndex_.end() &&
        (!ignoredPatientId || phoneOwner->second != *ignoredPatientId)) {
        throw std::invalid_argument("A patient with this phone number already exists.");
    }
    if (!email.empty()) {
        const auto emailOwner = emailIndex_.find(email);
        if (emailOwner != emailIndex_.end() &&
            (!ignoredPatientId || emailOwner->second != *ignoredPatientId)) {
            throw std::invalid_argument("A patient with this email address already exists.");
        }
    }
}

void PatientRepository::addToIndexes(const Patient& patient, std::size_t index) {
    idIndex_[patient.id] = index;
    phoneIndex_[normalizePhone(patient.phone)] = patient.id;
    const std::string email = normalizeText(patient.email);
    if (!email.empty()) emailIndex_[email] = patient.id;
}

void PatientRepository::removeContactIndexes(const Patient& patient) {
    phoneIndex_.erase(normalizePhone(patient.phone));
    const std::string email = normalizeText(patient.email);
    if (!email.empty()) emailIndex_.erase(email);
}

}  // namespace hms
