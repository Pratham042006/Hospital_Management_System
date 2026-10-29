#pragma once

#include "hms/doctor.hpp"

#include <cstddef>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace hms {

class DoctorRepository {
public:
    Doctor addDoctor(const DoctorDraft& draft);
    Doctor updateDoctor(const std::string& doctorId, const DoctorUpdate& update);
    std::optional<Doctor> findById(const std::string& doctorId) const;
    std::vector<Doctor> search(const std::string& query) const;
    std::vector<Doctor> list() const;
    bool isAvailable(const std::string& doctorId,
                     const std::string& date,
                     const std::string& time) const;

private:
    std::vector<Doctor> doctors_;
    std::unordered_map<std::string, std::size_t> idIndex_;
    std::unordered_map<std::string, std::string> phoneIndex_;
    std::unordered_map<std::string, std::string> emailIndex_;
    std::size_t nextDoctorNumber_{1};

    static std::string trim(const std::string& value);
    static std::string normalizeText(const std::string& value);
    static std::string normalizePhone(const std::string& value);
    static void validate(const Doctor& doctor);
    void ensureUniqueContacts(
        const Doctor& doctor,
        const std::optional<std::string>& ignoredDoctorId = std::nullopt) const;
    std::string generateDoctorId();
    void addToIndexes(const Doctor& doctor, std::size_t index);
    void removeContactIndexes(const Doctor& doctor);
};

}  // namespace hms
