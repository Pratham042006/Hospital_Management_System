#include "hms/patient_repository.hpp"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
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

std::string hexEncode(const std::string& value) {
    static constexpr char digits[] = "0123456789ABCDEF";
    std::string encoded;
    encoded.reserve(value.size() * 2);
    for (const unsigned char ch : value) {
        encoded.push_back(digits[ch >> 4]);
        encoded.push_back(digits[ch & 0x0F]);
    }
    return encoded;
}

int hexValue(char value) {
    if (value >= '0' && value <= '9') return value - '0';
    if (value >= 'A' && value <= 'F') return value - 'A' + 10;
    if (value >= 'a' && value <= 'f') return value - 'a' + 10;
    return -1;
}

std::string hexDecode(const std::string& value) {
    if (value.size() % 2 != 0) throw std::runtime_error("Malformed patient journal field.");
    std::string decoded;
    decoded.reserve(value.size() / 2);
    for (std::size_t index = 0; index < value.size(); index += 2) {
        const int high = hexValue(value[index]);
        const int low = hexValue(value[index + 1]);
        if (high < 0 || low < 0) throw std::runtime_error("Malformed patient journal field.");
        decoded.push_back(static_cast<char>((high << 4) | low));
    }
    return decoded;
}

std::vector<std::string> splitTabs(const std::string& line) {
    std::vector<std::string> fields;
    std::istringstream input(line);
    std::string field;
    while (std::getline(input, field, '\t')) fields.push_back(field);
    return fields;
}

}  // namespace

PatientRepository::PatientRepository(const std::optional<std::string>& journalPath)
    : journalPath_(journalPath) {
    if (journalPath_) loadJournal();
}

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
    appendToJournal(patient);
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
    appendToJournal(candidate);

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

void PatientRepository::appendToJournal(const Patient& patient) const {
    if (!journalPath_) return;
    const std::filesystem::path path(*journalPath_);
    const auto parent = path.parent_path();
    std::error_code error;
    if (!parent.empty()) std::filesystem::create_directories(parent, error);
    if (error) {
        throw std::runtime_error("Unable to create patient storage directory: " + error.message());
    }

    std::ofstream output(path, std::ios::app);
    if (!output) throw std::runtime_error("Unable to open patient journal for writing.");
    output << "UPSERT\t" << hexEncode(patient.id) << '\t' << hexEncode(patient.fullName) << '\t'
           << hexEncode(patient.dateOfBirth) << '\t' << hexEncode(patient.gender) << '\t'
           << hexEncode(patient.phone) << '\t' << hexEncode(patient.email) << '\t'
           << hexEncode(patient.address) << '\t' << hexEncode(patient.emergencyContactName) << '\t'
           << hexEncode(patient.emergencyContactPhone) << '\n';
    output.flush();
    if (!output) throw std::runtime_error("Unable to save the patient journal record.");
}

void PatientRepository::loadJournal() {
    const std::filesystem::path path(*journalPath_);
    if (!std::filesystem::exists(path)) return;
    std::ifstream input(path);
    if (!input) throw std::runtime_error("Unable to open patient journal for reading.");

    std::string line;
    std::size_t lineNumber = 0;
    while (std::getline(input, line)) {
        ++lineNumber;
        const auto fields = splitTabs(line);
        if (fields.size() != 10 || fields.front() != "UPSERT") {
            throw std::runtime_error("Malformed patient journal record at line " +
                                     std::to_string(lineNumber) + '.');
        }
        Patient patient{hexDecode(fields[1]), hexDecode(fields[2]), hexDecode(fields[3]),
                        hexDecode(fields[4]), hexDecode(fields[5]), hexDecode(fields[6]),
                        hexDecode(fields[7]), hexDecode(fields[8]), hexDecode(fields[9])};
        validate(patient);
        const auto existing = idIndex_.find(patient.id);
        if (existing == idIndex_.end()) {
            ensureUniqueContacts(patient);
            const std::size_t index = patients_.size();
            patients_.push_back(patient);
            addToIndexes(patients_.back(), index);
        } else {
            ensureUniqueContacts(patient, patient.id);
            Patient& stored = patients_.at(existing->second);
            removeContactIndexes(stored);
            stored = patient;
            addToIndexes(stored, existing->second);
        }
        if (patient.id.size() == 7 && patient.id.front() == 'P') {
            try {
                nextPatientNumber_ = std::max(nextPatientNumber_,
                                              static_cast<std::size_t>(std::stoul(patient.id.substr(1))) + 1);
            } catch (const std::exception&) {
                throw std::runtime_error("Malformed Patient ID in journal at line " +
                                         std::to_string(lineNumber) + '.');
            }
        }
    }
    if (!input.eof()) throw std::runtime_error("Unable to finish reading the patient journal.");
}

}  // namespace hms
