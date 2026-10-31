#include "test_support.hpp"

#include <iostream>
#include <stdexcept>

// FR-08 Admission & FR-11 Discharge Management Tests
// Author: Prathik Nambiar (PES2UG24CS371)

namespace {

int failures = 0;

#define CHECK(condition)                                                                    \
    do {                                                                                    \
        if (!(condition)) {                                                                 \
            std::cerr << "CHECK failed at line " << __LINE__ << ": " #condition << '\n'; \
            ++failures;                                                                     \
        }                                                                                   \
    } while (false)

template <typename Exception, typename Callable>
void checkThrows(Callable&& callable) {
    try {
        callable();
        ++failures;
    } catch (const Exception&) {
    } catch (...) {
        ++failures;
    }
}

}  // namespace

int main() {
    auto system = makeTestSystem();
    const auto receptionist = *system.login("receptionist", "test-reception-password");
    const auto doctorSession = *system.login("doctor", "test-doctor-password");

    // 1. Seed doctor, patient, and room
    hms::DoctorDraft docDraft{
        "Dr. Arvind Swamy", "Internal Medicine", "General Medicine",
        "9845012345", "aswamy@hms.org", {"2026-11-20 09:00"}
    };
    const auto doc = system.addDoctor(receptionist, docDraft);

    hms::PatientDraft patDraft{
        "Sunil Kumar", "1982-11-14", "Male", "9845098765",
        "sunil@example.com", "Jayanagar, Bangalore", "Anita Kumar", "9845098766"
    };
    const auto pat = system.registerPatient(receptionist, patDraft);

    hms::RoomDraft roomDraft{
        "R-ICU-01", "Intensive Care Unit", hms::RoomType::ICU, 1, 3000.0
    };
    system.addRoom(receptionist, roomDraft);

    // 2. Admit patient successfully (FR-08)
    hms::AdmissionDraft admDraft{
        pat.id, doc.id, "R-ICU-01", "2026-11-01", "Acute respiratory observation"
    };
    const auto admission = system.admitPatient(receptionist, admDraft);
    CHECK(admission.id == "ADM-00001");
    CHECK(admission.status == hms::AdmissionStatus::Admitted);
    CHECK(admission.patientId == pat.id);
    CHECK(admission.roomId == "R-ICU-01");

    // Verify room bed is now occupied
    const auto roomAfter = system.findRoomById(receptionist, "R-ICU-01");
    CHECK(roomAfter && roomAfter->occupiedBeds == 1);
    CHECK(!roomAfter->hasAvailableBed());

    // 3. Reject admission to a fully occupied room
    hms::PatientDraft patDraft2{
        "Deepa Nair", "1991-05-22", "Female", "9845123456",
        "deepa@example.com", "Indiranagar, Bangalore", "Ramesh Nair", "9845123457"
    };
    const auto pat2 = system.registerPatient(receptionist, patDraft2);

    hms::AdmissionDraft admDraftFullRoom{
        pat2.id, doc.id, "R-ICU-01", "2026-11-01", "Observation"
    };
    checkThrows<std::invalid_argument>([&] {
        system.admitPatient(receptionist, admDraftFullRoom);
    });

    // 4. Reject admitting the same patient twice while currently admitted
    hms::RoomDraft roomDraft2{
        "R-GEN-02", "General Ward B", hms::RoomType::GeneralWard, 2, 400.0
    };
    system.addRoom(receptionist, roomDraft2);

    hms::AdmissionDraft admDraftDuplicate{
        pat.id, doc.id, "R-GEN-02", "2026-11-01", "Second admission attempt"
    };
    checkThrows<std::invalid_argument>([&] {
        system.admitPatient(receptionist, admDraftDuplicate);
    });

    // 5. Reject admission with non-existent doctor or patient
    hms::AdmissionDraft invalidDoctorAdm{
        pat2.id, "D-MISSING", "R-GEN-02", "2026-11-01", "Test"
    };
    checkThrows<std::invalid_argument>([&] {
        system.admitPatient(receptionist, invalidDoctorAdm);
    });

    // 6. Discharge patient and verify bed release (FR-11)
    const auto discharged = system.dischargePatient(
        receptionist, admission.id, "2026-11-05", "Recovered completely, prescribed inhaler."
    );
    CHECK(discharged.status == hms::AdmissionStatus::Discharged);
    CHECK(discharged.dischargeDate == "2026-11-05");
    CHECK(discharged.dischargeSummary.has_value());

    // Bed in R-ICU-01 should now be released
    const auto roomAfterDischarge = system.findRoomById(receptionist, "R-ICU-01");
    CHECK(roomAfterDischarge && roomAfterDischarge->occupiedBeds == 0);
    CHECK(roomAfterDischarge->hasAvailableBed());

    // 7. Verify duplicate discharge is rejected
    checkThrows<std::invalid_argument>([&] {
        system.dischargePatient(receptionist, admission.id, "2026-11-06", "Second discharge");
    });

    // 8. Verify admissions listings
    const auto activeAdmissions = system.listActiveAdmissions(doctorSession);
    CHECK(activeAdmissions.empty());

    const auto allAdmissions = system.listAllAdmissions(doctorSession);
    CHECK(allAdmissions.size() == 1);

    if (failures != 0) {
        std::cerr << failures << " admission/discharge assertion(s) failed.\n";
        return 1;
    }
    std::cout << "FR-08 & FR-11 admission and discharge tests passed successfully.\n";
    return 0;
}
