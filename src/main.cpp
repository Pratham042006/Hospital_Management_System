#include "hms/auth.hpp"

#include <iostream>
#include <string>

namespace {

std::string prompt(const std::string& label) {
    std::cout << label;
    std::string value;
    std::getline(std::cin, value);
    return value;
}

}  // namespace

int main() {
    hms::AuthService auth;
    std::cout << "Hospital Management System\n";
    const std::string username = prompt("Username: ");
    const std::string password = prompt("Password: ");
    const auto session = auth.authenticate(username, password);
    if (!session) {
        std::cerr << "Login failed: invalid username or password.\n";
        return 1;
    }

    std::cout << "Welcome, " << session->username << " (" << hms::toString(session->role)
              << ").\nAvailable actions:\n";
    for (const auto& action : auth.availableActions(*session)) {
        std::cout << " - " << action << '\n';
    }
    return 0;
}
