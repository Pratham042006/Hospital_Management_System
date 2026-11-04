#include "hms/console_app.hpp"
#include "hms/hospital_system.hpp"
#include "hms/http_server.hpp"

#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--server" || arg == "-s" || arg == "--web") {
            int port = 8080;
            if (i + 1 < argc && argv[i + 1][0] != '-') {
                try {
                    port = std::stoi(argv[i + 1]);
                } catch (...) {}
            }
            std::cout << "[HMS] Launching Embedded Web Dashboard Server on port " << port << "...\n";
            std::cout << "[HMS] Access URL: http://localhost:" << port << "\n";
            std::cout << "[HMS] Press Enter or Ctrl+C to terminate server.\n";

            auto system = hms::HospitalSystem({
                {"admin", "test-admin-password", hms::Role::Administrator},
                {"receptionist", "test-reception-password", hms::Role::Receptionist},
                {"doctor", "test-doctor-password", hms::Role::Doctor},
                {"billing", "test-billing-password", hms::Role::BillingStaff},
            }, "data/patients.journal", "data/medical_records.journal", "data/billing.journal");
            hms::HttpServer server(system, port);
            server.start();
            std::string line;
            std::getline(std::cin, line);
            server.stop();
            std::cout << "[HMS] Server stopped cleanly.\n";
            return 0;
        }
    }
    return hms::runConsoleApp();
}
