#pragma once

#include <boost/asio.hpp>

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
        boost::asio::io_context io_context_;

        std::mutex mtx;
        std::condition_variable cv;
        bool is_listening_done{false};

    public:
        explicit Listener() = delete;
        virtual ~Listener() = default;

        void virtual startListening() = 0;
        void virtual stopListening() = 0;

        bool virtual sendCommand(const std::string_view&) = 0;

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

        bool sendCommand(const std::string_view&) override;

        ~TCPListener() override {stopListening();};
    private:
        void startListeningThread() override;

        std::string_view endpoint_ip_{"127.0.0.1"};
        std::string_view endpoint_port_{};

        boost::asio::ip::tcp::resolver resolver_;
        boost::asio::ip::tcp::socket socket_;
    };

    // COM ports listener
    class COMListener final : public Listener {
    public:
        COMListener() = delete;
        COMListener(uint64_t, const std::string_view&, const std::string_view&);

        void startListening() override;
        void stopListening() override;

        bool sendCommand(const std::string_view&) override;

        ~COMListener() override {stopListening();};
    private:
        void startListeningThread() override;

        std::string_view endpoint_com_{};
        boost::asio::serial_port serial_;
    };

} // port_listener
