#pragma once

#include "hms/hospital_system.hpp"

#include <atomic>
#include <string>
#include <thread>

namespace hms {

class HttpServer {
public:
    HttpServer(HospitalSystem& system, int port = 8080, std::string webRoot = "web");
    ~HttpServer();

    void start();
    void stop();
    bool isRunning() const noexcept;
    int getPort() const noexcept;

    std::string handleRequest(const std::string& rawRequest);

private:
    HospitalSystem& system_;
    int port_;
    std::string webRoot_;
    std::atomic<bool> running_{false};
    std::thread serverThread_;

    void runServer();
    std::string routeApi(const std::string& method, const std::string& path, const std::string& body);
    std::string serveStatic(const std::string& path);
};

}  // namespace hms
