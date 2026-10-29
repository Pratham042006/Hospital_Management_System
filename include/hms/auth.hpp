#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace hms {

enum class Role {
    Administrator,
    Receptionist,
    Doctor,
    BillingStaff,
};

enum class Permission {
    ManageUsers,
    RegisterPatient,
    UpdatePatient,
    SearchPatient,
    ViewClinicalData,
    ManageBilling,
    ManageDoctors,
    ManageAppointments,
};

struct Session {
    std::string username;
    Role role;
};

std::string toString(Role role);

class AuthService {
public:
    AuthService();

    bool addUser(const std::string& username, const std::string& plainTextPassword, Role role);
    std::optional<Session> authenticate(const std::string& username,
                                        const std::string& plainTextPassword) const;
    bool isAllowed(const Session& session, Permission permission) const;
    std::vector<std::string> availableActions(const Session& session) const;

private:
    struct Account {
        std::uint64_t passwordHash;
        Role role;
    };

    std::unordered_map<std::string, Account> accounts_;

    static std::uint64_t hashPassword(const std::string& username,
                                      const std::string& plainTextPassword);
};

}  // namespace hms
