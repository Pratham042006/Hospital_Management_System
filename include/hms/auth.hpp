#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
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
};

class HospitalSystem;

struct Session {
    Session(const Session&) = default;
    Session(Session&&) = default;

    const std::string username;
    const Role role;

private:
    Session(std::string authenticatedUsername, Role authenticatedRole)
        : username(std::move(authenticatedUsername)), role(authenticatedRole) {}

    friend class AuthService;
};

std::string toString(Role role);

class AuthService {
public:
    AuthService();
    std::optional<Session> authenticate(const std::string& username,
                                        const std::string& plainTextPassword) const;
    bool isAllowed(const Session& session, Permission permission) const;
    std::vector<std::string> availableActions(const Session& session) const;

private:
    struct Account {
        std::uint64_t passwordHash;
        Role role;
    };

    friend class HospitalSystem;

    bool addUser(const std::string& username, const std::string& plainTextPassword, Role role);

    std::unordered_map<std::string, Account> accounts_;

    static std::uint64_t hashPassword(const std::string& username,
                                      const std::string& plainTextPassword);
};

}  // namespace hms
