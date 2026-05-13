#include "telemetry_server.h"

#include <arpa/inet.h>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <netinet/in.h>
#include <sstream>
#include <utility>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

TelemetryServer::TelemetryServer(SnapshotFn snapshot_fn,
                                 SnapshotFn comparison_fn,
                                 std::string static_root)
    : snapshot_fn_(snapshot_fn),
      comparison_fn_(comparison_fn),
      static_root_(std::move(static_root)) {}

TelemetryServer::~TelemetryServer() { stop(); }

bool TelemetryServer::start(int port) {
    if (port <= 0) return true;
    fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (fd_ < 0) return false;
    int yes = 1;
    setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = htons(static_cast<uint16_t>(port));
    if (bind(fd_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) != 0 || listen(fd_, 8) != 0) {
        close(fd_);
        fd_ = -1;
        return false;
    }
    running_.store(true, std::memory_order_relaxed);
    thread_ = std::thread([this] { run(); });
    return true;
}

void TelemetryServer::stop() {
    running_.store(false, std::memory_order_relaxed);
    if (fd_ >= 0) shutdown(fd_, SHUT_RDWR);
    if (thread_.joinable()) thread_.join();
    if (fd_ >= 0) close(fd_);
    fd_ = -1;
}

void TelemetryServer::run() {
    while (running_.load(std::memory_order_relaxed)) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(fd_, &readfds);
        timeval tv{0, 200000};
        int ready = select(fd_ + 1, &readfds, nullptr, nullptr, &tv);
        if (ready <= 0) continue;
        int client = accept(fd_, nullptr, nullptr);
        if (client < 0) continue;
        handle_client(client);
        close(client);
    }
}

void TelemetryServer::handle_client(int client) {
    std::string req;
    char buf[1024];
    while (req.find("\r\n\r\n") == std::string::npos && req.size() < 8192) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(client, &readfds);
        timeval tv{0, 200000};
        int ready = select(client + 1, &readfds, nullptr, nullptr, &tv);
        if (ready <= 0) break;
        ssize_t n = recv(client, buf, sizeof(buf), 0);
        if (n <= 0) break;
        req.append(buf, static_cast<size_t>(n));
    }
    if (req.empty()) return;
    bool snapshot = req.compare(0, 13, "GET /snapshot") == 0;
    bool comparison = req.compare(0, 15, "GET /comparison") == 0;
    bool favicon = req.compare(0, 16, "GET /favicon.ico") == 0;
    if (favicon) {
        const char* response =
            "HTTP/1.1 204 No Content\r\nConnection: close\r\nContent-Length: 0\r\n\r\n";
        send_all(client, response, std::strlen(response));
        return;
    }
    std::string body;
    const char* type = "text/html; charset=utf-8";
    int status = 200;
    const char* status_text = "OK";
    if (snapshot) {
        body = snapshot_fn_();
        type = "application/json";
    } else if (comparison) {
        body = comparison_fn_ ? comparison_fn_() : "{\"enabled\":false}";
        type = "application/json";
    } else {
        const char* path = "index.html";
        if (req.compare(0, 12, "GET /app.css") == 0) {
            path = "app.css";
            type = "text/css; charset=utf-8";
        } else if (req.compare(0, 11, "GET /app.js") == 0) {
            path = "app.js";
            type = "application/javascript; charset=utf-8";
        }
        bool ok = false;
        body = read_static_file(path, ok);
        if (!ok) {
            status = 404;
            status_text = "Not Found";
            type = "text/plain; charset=utf-8";
            body = "telemetry viewer asset not found\n";
        }
    }
    char header[256];
    std::snprintf(header, sizeof(header),
                  "HTTP/1.1 %d %s\r\nContent-Type: %s\r\nCache-Control: no-store\r\nContent-Length: %zu\r\nConnection: close\r\n\r\n",
                  status, status_text, type, body.size());
    send_all(client, header, std::strlen(header));
    send_all(client, body.data(), body.size());
}

std::string TelemetryServer::read_static_file(const char* path, bool& ok) const {
    std::string full = static_root_;
    if (!full.empty() && full.back() != '/') full.push_back('/');
    full += path;
    std::ifstream in(full, std::ios::binary);
    if (!in) {
        ok = false;
        return {};
    }
    std::ostringstream os;
    os << in.rdbuf();
    ok = true;
    return os.str();
}

void TelemetryServer::send_all(int fd, const char* data, size_t size) {
    size_t sent = 0;
    while (sent < size) {
        ssize_t n = send(fd, data + sent, size - sent, MSG_NOSIGNAL);
        if (n <= 0) return;
        sent += static_cast<size_t>(n);
    }
}
