#pragma once

#include <boost/asio.hpp>

#include <cinttypes>
#include <string_view>
#include <thread>
#include <condition_variable>
#include <fstream>
#include <iostream>

namespace port_listener {

    enum LogLevel {
        kLogDebug,
        kLogInfo,
        kLogWarning,
        kLogError,
        kLogCritical,
    };

    class Logger {
    public:
        // Constructor: Opens the log file in append mode
        explicit Logger (const std::string_view& filename) {
            logFile.open(filename.data(), std::ios::app);

            if (!logFile.is_open()) {
                std::cerr << "Error opening log file." << std::endl;
                exit(EXIT_FAILURE);
            }
        }

        // Destructor: Closes the log file
        ~Logger() {
            logFile.close();
        }

        // Logs a message with a given log level
        void log(const LogLevel level, const std::string_view& message)
        {
            // Get current timestamp
            const auto now = std::time(nullptr);
            const auto time_info = *std::localtime(&now);

            char timestamp[20];
            strftime(timestamp, sizeof(timestamp),
                     "%Y-%m-%d %H:%M:%S", &time_info);

            // Create log entry
            std::ostringstream logEntry;

            logEntry << "[" << timestamp << "] "
                     << levelToString(level) << ": " << message
                     << std::endl;

            // Output to console
            std::cout << logEntry.str();

            // Output to log file
            if (logFile.is_open()) {
                logFile << logEntry.str();
                logFile.flush(); // Ensure immediate write to file
            }
        }

    private:
        std::ofstream logFile; // File stream for the log file

        // Converts log level to a string for output
        static std::string_view levelToString(const LogLevel level)
        {
            switch (level) {
                case LogLevel::kLogDebug:
                    return "DEBUG";
                case LogLevel::kLogInfo:
                    return "INFO";
                case LogLevel::kLogWarning:
                    return "WARNING";
                case LogLevel::kLogError:
                    return "ERROR";
                case LogLevel::kLogCritical:
                    return "CRITICAL";
                default:
                    return "UNKNOWN";
            }
        }
    };


    // Base listener class
    class Listener {
    protected:
        explicit Listener(uint64_t, const std::string_view&);

        std::thread listening_thread_;
        uint64_t buffer_size_{1024};
        std::string_view odata_filename_{};

        std::mutex mtx;
        std::condition_variable cv;
        //bool is_listening_done{false};

        Logger logger_{"logs.txt"};
    public:
        explicit Listener() = delete;
        virtual ~Listener() = default;

        void virtual startListening() = 0;
        void virtual stopListening() = 0;

        void virtual sendCommand(const std::string_view&) = 0;

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

        void sendCommand(const std::string_view&) override;

        ~TCPListener() override {stopListening();};
    private:
        void startListeningThread() override;

        std::string_view endpoint_ip_{"127.0.0.1"};
        std::string_view endpoint_port_{};

        boost::asio::io_context io_context_;
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

        void sendCommand(const std::string_view&) override;

        ~COMListener() override {stopListening();};
    private:
        void startListeningThread() override;

        std::string_view endpoint_com_{};

        boost::asio::io_service io_service_;
        boost::asio::serial_port serial_;
    };

} // port_listener