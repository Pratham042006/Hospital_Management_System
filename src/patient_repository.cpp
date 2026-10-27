#include "hms/patient_repository.hpp"
#include "hms/patient_validation.hpp"

#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace hms {
namespace {

constexpr char kPatientIdPrefix = 'P';
constexpr int kPatientIdWidth = 6;

}  // namespace

Patient PatientRepository::registerPatient(const PatientDraft& draft) {
    Patient patient{
        generatePatientId(),
        validation::trim(draft.fullName),
        validation::trim(draft.dateOfBirth),
        validation::trim(draft.gender),
        validation::trim(draft.phone),
        validation::trim(draft.email),
        validation::trim(draft.address),
        validation::trim(draft.emergencyContactName),
        validation::trim(draft.emergencyContactPhone),
    };

    validation::validatePatient(patient);
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
    if (update.fullName) candidate.fullName = validation::trim(*update.fullName);
    if (update.dateOfBirth) candidate.dateOfBirth = validation::trim(*update.dateOfBirth);
    if (update.gender) candidate.gender = validation::trim(*update.gender);
    if (update.phone) candidate.phone = validation::trim(*update.phone);
    if (update.email) candidate.email = validation::trim(*update.email);
    if (update.emergencyContactName) {
        candidate.emergencyContactName = validation::trim(*update.emergencyContactName);
    }
    if (update.emergencyContactPhone) {
        candidate.emergencyContactPhone = validation::trim(*update.emergencyContactPhone);
    }

    validation::validatePatient(candidate);
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
    const std::string query = validation::normalizeText(name);
    std::vector<Patient> matches;
    if (query.empty()) return matches;
    for (const auto& patient : patients_) {
        if (validation::normalizeText(patient.fullName).find(query) != std::string::npos) {
            matches.push_back(patient);
        }
    }
    return matches;
}

std::vector<Patient> PatientRepository::findByContact(const std::string& contact) const {
    const std::string phone = validation::normalizePhone(contact);
    const std::string email = validation::normalizeText(contact);
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

std::string PatientRepository::generatePatientId() {
    std::ostringstream id;
    id << kPatientIdPrefix << std::setw(kPatientIdWidth) << std::setfill('0')
       << nextPatientNumber_++;
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
    const std::string phone = validation::normalizePhone(patient.phone);
    const std::string email = validation::normalizeText(patient.email);
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
    phoneIndex_[validation::normalizePhone(patient.phone)] = patient.id;
    const std::string email = validation::normalizeText(patient.email);
    if (!email.empty()) emailIndex_[email] = patient.id;
}

void PatientRepository::removeContactIndexes(const Patient& patient) {
    phoneIndex_.erase(validation::normalizePhone(patient.phone));
    const std::string email = validation::normalizeText(patient.email);
    if (!email.empty()) emailIndex_.erase(email);
}

}  // namespace hms
