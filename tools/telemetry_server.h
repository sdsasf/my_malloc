#pragma once

#include <atomic>
#include <string>
#include <thread>

class TelemetryServer {
public:
    using SnapshotFn = std::string (*)();

    TelemetryServer(SnapshotFn snapshot_fn, std::string static_root);
    ~TelemetryServer();

    bool start(int port);
    void stop();

private:
    void run();
    void handle_client(int client);
    std::string read_static_file(const char* path, bool& ok) const;
    static void send_all(int fd, const char* data, size_t size);

    SnapshotFn snapshot_fn_;
    std::string static_root_;
    int fd_ = -1;
    std::atomic<bool> running_{false};
    std::thread thread_;
};
