#include "hms/report_service.hpp"

#include <cmath>

namespace hms {

PatientCensusReport ReportService::generatePatientCensus(
    std::size_t patientCount,
    const std::vector<Admission>& allAdmissions) {
    PatientCensusReport report;
    report.totalPatients = patientCount;

    for (const auto& adm : allAdmissions) {
        if (adm.status == AdmissionStatus::Admitted) {
            report.activeInpatients++;
        } else if (adm.status == AdmissionStatus::Discharged) {
            report.dischargedPatients++;
        }
    }
    return report;
}

AppointmentSummaryReport ReportService::generateAppointmentSummary(
    const std::vector<Appointment>& allAppointments) {
    AppointmentSummaryReport report;
    report.totalAppointments = allAppointments.size();

    for (const auto& appt : allAppointments) {
        if (appt.status == AppointmentStatus::Scheduled) {
            report.scheduledCount++;
        } else if (appt.status == AppointmentStatus::Completed) {
            report.completedCount++;
        } else if (appt.status == AppointmentStatus::Cancelled) {
            report.cancelledCount++;
        }
        report.countByDoctor[appt.doctorId]++;
    }
    return report;
}

RoomOccupancyReport ReportService::generateRoomOccupancy(
    const std::vector<Room>& allRooms) {
    RoomOccupancyReport report;
    report.totalRooms = allRooms.size();

    for (const auto& room : allRooms) {
        report.totalBeds += room.totalBeds;
        report.occupiedBeds += room.occupiedBeds;
    }

    if (report.totalBeds >= report.occupiedBeds) {
        report.availableBeds = report.totalBeds - report.occupiedBeds;
    }

    if (report.totalBeds > 0) {
        report.occupancyRate = std::round((static_cast<double>(report.occupiedBeds) /
                                           static_cast<double>(report.totalBeds)) * 10000.0) / 100.0;
    } else {
        report.occupancyRate = 0.0;
    }

    return report;
}

BillingRevenueReport ReportService::generateBillingRevenue(
    const std::vector<Bill>& allBills) {
    BillingRevenueReport report;
    report.totalBills = allBills.size();

    for (const auto& bill : allBills) {
        report.totalBilled += bill.totalAmount;
        report.totalCollected += bill.paidAmount;
        report.totalOutstanding += bill.balance;

        if (bill.status == BillStatus::Unpaid) {
            report.unpaidBills++;
        } else if (bill.status == BillStatus::PartiallyPaid) {
            report.partiallyPaidBills++;
        } else if (bill.status == BillStatus::Paid) {
            report.paidBills++;
        }
    }

    report.totalBilled = std::round(report.totalBilled * 100.0) / 100.0;
    report.totalCollected = std::round(report.totalCollected * 100.0) / 100.0;
    report.totalOutstanding = std::round(report.totalOutstanding * 100.0) / 100.0;

    return report;
}

DoctorUtilizationReport ReportService::generateDoctorUtilization(
    const std::vector<Doctor>& allDoctors,
    const std::vector<Appointment>& allAppointments,
    const std::vector<Admission>& allAdmissions) {
    DoctorUtilizationReport report;

    for (const auto& doc : allDoctors) {
        DoctorUtilizationItem item;
        item.doctorId = doc.id;
        item.doctorName = doc.fullName;
        item.specialization = doc.specialization;
        item.department = doc.department;

        for (const auto& appt : allAppointments) {
            if (appt.doctorId == doc.id) {
                item.appointmentCount++;
            }
        }

        for (const auto& adm : allAdmissions) {
            if (adm.doctorId == doc.id && adm.status == AdmissionStatus::Admitted) {
                item.activeAdmissionCount++;
            }
        }

        report.doctors.push_back(item);
    }

    return report;
}

}  // namespace hms
