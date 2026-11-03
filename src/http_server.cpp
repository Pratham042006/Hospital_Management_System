#include "hms/http_server.hpp"

#include <chrono>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
typedef int socklen_t;
#define CLOSE_SOCKET closesocket
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#define CLOSE_SOCKET close
#define INVALID_SOCKET (-1)
#define SOCKET_ERROR (-1)
typedef int SOCKET;
#endif

namespace hms {

namespace fs = std::filesystem;

namespace {

std::string getContentType(const std::string& path) {
    if (path.rfind(".html") != std::string::npos) return "text/html; charset=UTF-8";
    if (path.rfind(".css") != std::string::npos)  return "text/css; charset=UTF-8";
    if (path.rfind(".js") != std::string::npos)   return "application/javascript; charset=UTF-8";
    if (path.rfind(".json") != std::string::npos) return "application/json; charset=UTF-8";
    if (path.rfind(".png") != std::string::npos)  return "image/png";
    return "text/plain; charset=UTF-8";
}

std::string makeHttpResponse(int statusCode, const std::string& statusText,
                             const std::string& contentType, const std::string& body) {
    std::ostringstream response;
    response << "HTTP/1.1 " << statusCode << " " << statusText << "\r\n";
    response << "Content-Type: " << contentType << "\r\n";
    response << "Content-Length: " << body.size() << "\r\n";
    response << "Access-Control-Allow-Origin: *\r\n";
    response << "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n";
    response << "Access-Control-Allow-Headers: Content-Type, Authorization\r\n";
    response << "Connection: close\r\n\r\n";
    response << body;
    return response.str();
}

}  // namespace

HttpServer::HttpServer(HospitalSystem& system, int port, std::string webRoot)
    : system_(system), port_(port), webRoot_(std::move(webRoot)) {}

HttpServer::~HttpServer() {
    stop();
}

bool HttpServer::isRunning() const noexcept {
    return running_.load();
}

int HttpServer::getPort() const noexcept {
    return port_;
}

void HttpServer::start() {
    if (running_.load()) return;
    running_.store(true);
    serverThread_ = std::thread(&HttpServer::runServer, this);
}

void HttpServer::stop() {
    if (!running_.load()) return;
    running_.store(false);

    // Ping the local socket to unblock accept() if needed
    SOCKET client = socket(AF_INET, SOCK_STREAM, 0);
    if (client != INVALID_SOCKET) {
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(static_cast<uint16_t>(port_));
        addr.sin_addr.s_addr = inet_addr("127.0.0.1");
        connect(client, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
        CLOSE_SOCKET(client);
    }

    if (serverThread_.joinable()) {
        serverThread_.join();
    }
}

void HttpServer::runServer() {
#if defined(_WIN32)
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif

    SOCKET serverSock = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSock == INVALID_SOCKET) {
        running_.store(false);
#if defined(_WIN32)
        WSACleanup();
#endif
        return;
    }

    int opt = 1;
#if defined(_WIN32)
    setsockopt(serverSock, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&opt), sizeof(opt));
#else
    setsockopt(serverSock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#endif

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(static_cast<uint16_t>(port_));
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == SOCKET_ERROR) {
        CLOSE_SOCKET(serverSock);
        running_.store(false);
#if defined(_WIN32)
        WSACleanup();
#endif
        return;
    }

    if (listen(serverSock, 10) == SOCKET_ERROR) {
        CLOSE_SOCKET(serverSock);
        running_.store(false);
#if defined(_WIN32)
        WSACleanup();
#endif
        return;
    }

    while (running_.load()) {
        sockaddr_in clientAddr{};
        socklen_t clientLen = sizeof(clientAddr);
        SOCKET clientSock = accept(serverSock, reinterpret_cast<sockaddr*>(&clientAddr), &clientLen);
        if (clientSock == INVALID_SOCKET) {
            break;
        }

        if (!running_.load()) {
            CLOSE_SOCKET(clientSock);
            break;
        }

        char buffer[4096];
        int bytesRead = recv(clientSock, buffer, sizeof(buffer) - 1, 0);
        if (bytesRead > 0) {
            buffer[bytesRead] = '\0';
            std::string response = handleRequest(std::string(buffer, static_cast<size_t>(bytesRead)));
            send(clientSock, response.c_str(), static_cast<int>(response.size()), 0);
        }
        CLOSE_SOCKET(clientSock);
    }

    CLOSE_SOCKET(serverSock);
#if defined(_WIN32)
    WSACleanup();
#endif
}

std::string HttpServer::handleRequest(const std::string& rawRequest) {
    std::istringstream stream(rawRequest);
    std::string method, path, version;
    stream >> method >> path >> version;

    if (method.empty() || path.empty()) {
        return makeHttpResponse(400, "Bad Request", "text/plain", "Bad Request");
    }

    if (method == "OPTIONS") {
        return makeHttpResponse(200, "OK", "text/plain", "");
    }

    // Extract body if any
    std::string body;
    size_t headerEnd = rawRequest.find("\r\n\r\n");
    if (headerEnd != std::string::npos) {
        body = rawRequest.substr(headerEnd + 4);
    }

    if (path.rfind("/api/", 0) == 0) {
        return routeApi(method, path, body);
    }

    return serveStatic(path);
}

std::string HttpServer::routeApi(const std::string& method, const std::string& path, const std::string& body) {
    (void)body;

    if (path == "/api/health") {
        return makeHttpResponse(200, "OK", "application/json",
            "{\"status\":\"UP\",\"system\":\"Hospital Management System\",\"version\":\"2.0.0\"}");
    }

    if (path == "/api/login" && method == "POST") {
        return makeHttpResponse(200, "OK", "application/json",
            "{\"success\":true,\"username\":\"admin\",\"role\":\"Administrator\",\"token\":\"hms-session-token-valid\"}");
    }

    if (path == "/api/patients") {
        return makeHttpResponse(200, "OK", "application/json",
            "{\"success\":true,\"total\":" + std::to_string(system_.patientCount()) + "}");
    }

    if (path == "/api/reports/census") {
        return makeHttpResponse(200, "OK", "application/json",
            "{\"totalPatients\":" + std::to_string(system_.patientCount()) + ",\"activeInpatients\":1,\"discharged\":0}");
    }

    if (path == "/api/reports/occupancy") {
        return makeHttpResponse(200, "OK", "application/json",
            "{\"totalRooms\":4,\"totalBeds\":10,\"occupiedBeds\":2,\"occupancyRate\":20.0}");
    }

    if (path == "/api/reports/revenue") {
        return makeHttpResponse(200, "OK", "application/json",
            "{\"totalBilled\":4500.00,\"totalCollected\":3200.00,\"outstanding\":1300.00}");
    }

    return makeHttpResponse(404, "Not Found", "application/json", "{\"error\":\"Endpoint not found\"}");
}

std::string HttpServer::serveStatic(const std::string& requestedPath) {
    std::string normalizedPath = requestedPath;
    if (normalizedPath == "/" || normalizedPath.empty()) {
        normalizedPath = "/index.html";
    }

    // Security: prevent directory traversal
    if (normalizedPath.find("..") != std::string::npos) {
        return makeHttpResponse(403, "Forbidden", "text/plain", "Forbidden");
    }

    fs::path fullPath = fs::path(webRoot_) / (normalizedPath[0] == '/' ? normalizedPath.substr(1) : normalizedPath);

    if (!fs::exists(fullPath) || fs::is_directory(fullPath)) {
        return makeHttpResponse(404, "Not Found", "text/html",
            "<html><body><h1>404 Not Found</h1><p>HMS Web Asset not found: " + normalizedPath + "</p></body></html>");
    }

    std::ifstream file(fullPath, std::ios::binary);
    if (!file.is_open()) {
        return makeHttpResponse(500, "Internal Server Error", "text/plain", "Failed to read file.");
    }

    std::ostringstream ss;
    ss << file.rdbuf();
    std::string content = ss.str();

    return makeHttpResponse(200, "OK", getContentType(fullPath.string()), content);
}

}  // namespace hms
