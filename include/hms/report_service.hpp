#pragma once

#include "hms/admission.hpp"
#include "hms/appointment.hpp"
#include "hms/bill.hpp"
#include "hms/doctor.hpp"
#include "hms/patient.hpp"
#include "hms/room.hpp"

#include <string>
#include <unordered_map>
#include <vector>

namespace hms {

struct PatientCensusReport {
    std::size_t totalPatients{0};
    std::size_t activeInpatients{0};
    std::size_t dischargedPatients{0};
};

struct AppointmentSummaryReport {
    std::size_t totalAppointments{0};
    std::size_t scheduledCount{0};
    std::size_t completedCount{0};
    std::size_t cancelledCount{0};
    std::unordered_map<std::string, std::size_t> countByDoctor;
};

struct RoomOccupancyReport {
    std::size_t totalRooms{0};
    std::size_t totalBeds{0};
    std::size_t occupiedBeds{0};
    std::size_t availableBeds{0};
    double occupancyRate{0.0};  // percentage 0.0 - 100.0
};

struct BillingRevenueReport {
    std::size_t totalBills{0};
    double totalBilled{0.0};
    double totalCollected{0.0};
    double totalOutstanding{0.0};
    std::size_t unpaidBills{0};
    std::size_t partiallyPaidBills{0};
    std::size_t paidBills{0};
};

struct DoctorUtilizationItem {
    std::string doctorId;
    std::string doctorName;
    std::string specialization;
    std::string department;
    std::size_t appointmentCount{0};
    std::size_t activeAdmissionCount{0};
};

struct DoctorUtilizationReport {
    std::vector<DoctorUtilizationItem> doctors;
};

class ReportService {
public:
    static PatientCensusReport generatePatientCensus(
        std::size_t patientCount,
        const std::vector<Admission>& allAdmissions);

    static AppointmentSummaryReport generateAppointmentSummary(
        const std::vector<Appointment>& allAppointments);

    static RoomOccupancyReport generateRoomOccupancy(
        const std::vector<Room>& allRooms);

    static BillingRevenueReport generateBillingRevenue(
        const std::vector<Bill>& allBills);

    static DoctorUtilizationReport generateDoctorUtilization(
        const std::vector<Doctor>& allDoctors,
        const std::vector<Appointment>& allAppointments,
        const std::vector<Admission>& allAdmissions);
};

}  // namespace hms
