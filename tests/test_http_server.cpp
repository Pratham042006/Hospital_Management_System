#include "hms/http_server.hpp"
#include "test_support.hpp"

#include <cassert>
#include <filesystem>
#include <iostream>

void testHttpServerRequestHandling() {
    auto system = makeTestSystem();
    std::string webDir = std::filesystem::exists("web") ? "web" : "../web";
    hms::HttpServer server(system, 8089, webDir);

    // Test health endpoint
    std::string healthReq = "GET /api/health HTTP/1.1\r\nHost: localhost\r\n\r\n";
    std::string healthRes = server.handleRequest(healthReq);
    assert(healthRes.find("HTTP/1.1 200 OK") != std::string::npos);
    assert(healthRes.find("\"UP\"") != std::string::npos);

    // Test login endpoint
    std::string loginReq = "POST /api/login HTTP/1.1\r\nHost: localhost\r\nContent-Length: 0\r\n\r\n";
    std::string loginRes = server.handleRequest(loginReq);
    assert(loginRes.find("HTTP/1.1 200 OK") != std::string::npos);
    assert(loginRes.find("\"success\":true") != std::string::npos);

    // Test patients endpoint
    std::string patReq = "GET /api/patients HTTP/1.1\r\nHost: localhost\r\n\r\n";
    std::string patRes = server.handleRequest(patReq);
    assert(patRes.find("HTTP/1.1 200 OK") != std::string::npos);

    // Test reports endpoint
    std::string repReq = "GET /api/reports/occupancy HTTP/1.1\r\nHost: localhost\r\n\r\n";
    std::string repRes = server.handleRequest(repReq);
    assert(repRes.find("HTTP/1.1 200 OK") != std::string::npos);

    // Test 404 for unknown endpoint
    std::string badReq = "GET /api/unknown_endpoint HTTP/1.1\r\nHost: localhost\r\n\r\n";
    std::string badRes = server.handleRequest(badReq);
    assert(badRes.find("HTTP/1.1 404 Not Found") != std::string::npos);

    // Test static asset serving (index.html)
    std::string staticReq = "GET /index.html HTTP/1.1\r\nHost: localhost\r\n\r\n";
    std::string staticRes = server.handleRequest(staticReq);
    assert(staticRes.find("HTTP/1.1 200 OK") != std::string::npos);
    assert(staticRes.find("Hospital HMS") != std::string::npos);

    // Test path traversal blocking
    std::string hackReq = "GET /../CMakeLists.txt HTTP/1.1\r\nHost: localhost\r\n\r\n";
    std::string hackRes = server.handleRequest(hackReq);
    assert(hackRes.find("HTTP/1.1 403 Forbidden") != std::string::npos);

    std::cout << "testHttpServerRequestHandling passed\n";
}

int main() {
    testHttpServerRequestHandling();
    std::cout << "All HTTP server tests passed successfully.\n";
    return 0;
}
