#pragma once

#include "hms/hospital_system.hpp"

#include <vector>

inline hms::HospitalSystem makeTestSystem() {
    return hms::HospitalSystem({
        {"admin", "test-admin-password", hms::Role::Administrator},
        {"receptionist", "test-reception-password", hms::Role::Receptionist},
        {"doctor", "test-doctor-password", hms::Role::Doctor},
        {"billing", "test-billing-password", hms::Role::BillingStaff},
    });
}
