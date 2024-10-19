#pragma once

#include <cinttypes>
#include <string_view>
#include <thread>
#include <condition_variable>

namespace port_listener {

    // Base listener class
    class Listener {
    protected:
        explicit Listener(uint64_t, const std::string_view&);

        std::thread listening_thread_;
        uint64_t buffer_size_{1024};
        std::string_view odata_filename_{};

        std::mutex mtx;
        std::condition_variable cv;
        bool is_listening_done{false};

    public:
        explicit Listener() = delete;
        virtual ~Listener() = default;

        void virtual startListening() = 0;
        void virtual stopListening() = 0;

    private:
        void setOutputDataFilename(const std::string_view&);
        void virtual startListeningThread() = 0;
    };

    // TCP ports listener
    class TCPListener final : public Listener {
    public:
        TCPListener() = delete;
        TCPListener(uint64_t, const std::string_view&, const std::string_view&, const std::string_view&);

        void startListening() override;
        void stopListening() override;


        ~TCPListener() override {stopListening();};
    private:
        void startListeningThread() override;

        std::string_view endpoint_ip_{"127.0.0.1"};
        std::string_view endpoint_port_{};
    };

    // COM ports listener
    class COMListener final : public Listener {
    public:
        COMListener() = delete;
        COMListener(uint64_t, const std::string_view&, const std::string_view&);

        void startListening() override;
        void stopListening() override;

        ~COMListener() override {stopListening();};
    private:
        void startListeningThread() override;

        std::string_view endpoint_com_{};
    };

} // port_listener
