#include "hms/auth.hpp"

#include <algorithm>
#include <array>
#include <utility>

namespace hms {

namespace {

using PermissionList = std::vector<Permission>;

const PermissionList& permissionsForRole(Role role) {
    static const PermissionList administrator{
        Permission::ManageUsers,
        Permission::RegisterPatient,
        Permission::UpdatePatient,
        Permission::SearchPatient,
        Permission::ViewClinicalData,
        Permission::ManageBilling,
        Permission::ManageDoctors,
        Permission::ManageAppointments,
    };
    static const PermissionList receptionist{
        Permission::RegisterPatient,
        Permission::UpdatePatient,
        Permission::SearchPatient,
        Permission::ManageDoctors,
        Permission::ManageAppointments,
    };
    static const PermissionList doctor{
        Permission::SearchPatient,
        Permission::ViewClinicalData,
    };
    static const PermissionList billing{
        Permission::SearchPatient,
        Permission::ManageBilling,
    };

    switch (role) {
        case Role::Administrator:
            return administrator;
        case Role::Receptionist:
            return receptionist;
        case Role::Doctor:
            return doctor;
        case Role::BillingStaff:
            return billing;
    }
    return receptionist;
}

}  // namespace

std::string toString(Role role) {
    switch (role) {
        case Role::Administrator:
            return "Administrator";
        case Role::Receptionist:
            return "Receptionist";
        case Role::Doctor:
            return "Doctor";
        case Role::BillingStaff:
            return "Billing Staff";
    }
    return "Unknown";
}

AuthService::AuthService() {
    addUser("admin", "admin123", Role::Administrator);
    addUser("receptionist", "reception123", Role::Receptionist);
    addUser("doctor", "doctor123", Role::Doctor);
    addUser("billing", "billing123", Role::BillingStaff);
}

bool AuthService::addUser(const std::string& username,
                          const std::string& plainTextPassword,
                          Role role) {
    if (username.empty() || plainTextPassword.size() < 8 || accounts_.count(username) != 0U) {
        return false;
    }
    accounts_.emplace(username, Account{hashPassword(username, plainTextPassword), role});
    return true;
}

std::optional<Session> AuthService::authenticate(const std::string& username,
                                                 const std::string& plainTextPassword) const {
    const auto found = accounts_.find(username);
    if (found == accounts_.end()) {
        return std::nullopt;
    }
    if (found->second.passwordHash != hashPassword(username, plainTextPassword)) {
        return std::nullopt;
    }
    return Session{username, found->second.role};
}

bool AuthService::isAllowed(const Session& session, Permission permission) const {
    const auto& allowed = permissionsForRole(session.role);
    return std::find(allowed.begin(), allowed.end(), permission) != allowed.end();
}

std::vector<std::string> AuthService::availableActions(const Session& session) const {
    std::vector<std::string> actions;
    if (isAllowed(session, Permission::ManageUsers)) {
        actions.emplace_back("Manage users");
    }
    if (isAllowed(session, Permission::RegisterPatient)) {
        actions.emplace_back("Register patient");
    }
    if (isAllowed(session, Permission::UpdatePatient)) {
        actions.emplace_back("Update patient");
    }
    if (isAllowed(session, Permission::SearchPatient)) {
        actions.emplace_back("Search patients");
    }
    if (isAllowed(session, Permission::ViewClinicalData)) {
        actions.emplace_back("View clinical data");
    }
    if (isAllowed(session, Permission::ManageBilling)) {
        actions.emplace_back("Manage billing");
    }
    if (isAllowed(session, Permission::ManageDoctors)) {
        actions.emplace_back("Manage doctors");
    }
    if (isAllowed(session, Permission::ManageAppointments)) {
        actions.emplace_back("Manage appointments");
    }
    return actions;
}

std::uint64_t AuthService::hashPassword(const std::string& username,
                                        const std::string& plainTextPassword) {
    // A salted FNV-1a digest avoids retaining plain-text credentials in memory.
    // Replace with a dedicated password KDF when an external crypto dependency is approved.
    constexpr std::uint64_t offsetBasis = 14695981039346656037ULL;
    constexpr std::uint64_t prime = 1099511628211ULL;
    std::uint64_t hash = offsetBasis;
    const std::string input = "hms:" + username + ':' + plainTextPassword;
    for (const unsigned char character : input) {
        hash ^= character;
        hash *= prime;
    }
    return hash;
}

}  // namespace hms
