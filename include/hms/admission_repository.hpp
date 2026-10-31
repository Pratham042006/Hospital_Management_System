#pragma once

#include "hms/admission.hpp"

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace hms {

class AdmissionRepository {
public:
    Admission admitPatient(const AdmissionDraft& draft);
    Admission dischargePatient(const std::string& admissionId,
                               const std::string& dischargeDate,
                               const std::string& dischargeSummary);

    std::optional<Admission> findAdmissionById(const std::string& admissionId) const;
    std::optional<Admission> findActiveAdmissionByPatient(const std::string& patientId) const;
    std::vector<Admission> listActiveAdmissions() const;
    std::vector<Admission> listAllAdmissions() const;
    std::vector<Admission> listAdmissionsForDoctor(const std::string& doctorId) const;

    std::size_t totalAdmissions() const;

private:
    std::unordered_map<std::string, Admission> admissions_;
    int nextId_{1};
};

}  // namespace hms
