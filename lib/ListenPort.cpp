#include "ListenPort.hpp"

#include <iomanip>
#include <ctime>
#include <sstream>

using boost::asio::ip::tcp;
using namespace port_listener;


std::mutex file_mutex;
std::mutex console_mutex;  // Separate mutex for console output
std::atomic<bool> is_listening_done(false);


Listener::Listener(const uint64_t buffer_size, const std::string_view& odata_filename)
            : buffer_size_(buffer_size)
{
    // Set inputted filename or unique if not stated
    setOutputDataFilename(odata_filename);
}

void Listener::setOutputDataFilename(const std::string_view& filename) {
    if (!filename.empty() /*&& !std::filesystem::exists(filename.data())*/) {
        odata_filename_ = filename;
        return;
    }

    // TODO: Add overwrite support with copying old versions to specific folder
    logger_.log(LogLevel::kLogWarning, "File name not specified\n"
                 "File with unique name will be created instead");

    // Creating file with unique name
    const auto t = std::time(nullptr);
    const auto tm = *std::localtime(&t);

    std::ostringstream oss;
    oss << std::put_time(&tm, "data_%d%m%Y_%H%M%S"); // TODO: Add custom file name formatting
    const auto str = oss.str() + ".txt";

    odata_filename_ = str;
}


TCPListener::TCPListener(
    const uint64_t buffer_size,
    const std::string_view& odata_filename,
    const std::string_view& endpoint_ip,
    const std::string_view& endpoint_port
    )
    : Listener(buffer_size, odata_filename),
    endpoint_ip_(endpoint_ip),
    endpoint_port_(endpoint_port),
    resolver_(tcp::resolver(io_context_)),
    socket_(tcp::socket(io_context_))
{}

void TCPListener::startListening() {
    listening_thread_ = std::thread(&TCPListener::startListeningThread, this);
}

void TCPListener::stopListening() {
    {
        std::lock_guard lock(mtx);
        is_listening_done = true;
    }
    cv.notify_one();

    if (listening_thread_.joinable()) {
        listening_thread_.join();
    }
}

void TCPListener::sendCommand(const std::string_view& command) {
    boost::system::error_code error;

    const tcp::resolver::results_type endpoints = resolver_.resolve(endpoint_ip_, endpoint_port_);
    [[maybe_unused]] auto connection = boost::asio::connect(socket_, endpoints);

    // Write the command to the TCP port
    boost::asio::write(socket_, boost::asio::buffer(command), error);
}


void TCPListener::startListeningThread() {
    std::ofstream odata_file(odata_filename_.data(), std::ios::binary | std::ios::out);

    if (!odata_file.is_open()) {
        logger_.log(LogLevel::kLogError, "Failed to open output file.");
        return;
    }

    // Define a TCP endpoint with the local host and the port
    const tcp::resolver::results_type endpoints = resolver_.resolve(endpoint_ip_, endpoint_port_);

    // Connect to the endpoint
    [[maybe_unused]] auto tcp = boost::asio::connect(socket_, endpoints);

    // Buffer to hold incoming data
    char buffer[buffer_size_];

    // Keep reading data from the socket
    while (true) {
        boost::system::error_code error;
        const std::size_t len = socket_.read_some(boost::asio::buffer(buffer, sizeof(buffer)), error);

        if (error == boost::asio::error::eof) {
            // Connection closed cleanly by peer
            logger_.log(LogLevel::kLogError, "Connection closed by peer");
            break;
        }

        if (error) {
            // Some other error occurred
            throw boost::system::system_error(error);
        }

        // Write data to file
        odata_file.write(buffer, static_cast<std::streamsize>(len));

        // Check if join requested
        {
            std::lock_guard lock(mtx);
            if (is_listening_done) {
                break;  // Stop the loop when signaled
            }
        }
    }
}


COMListener::COMListener(
    const uint64_t buffer_size,
    const std::string_view& odata_filename,
    const std::string_view& endpoint_com
    )
    : Listener(buffer_size, odata_filename), endpoint_com_(endpoint_com), serial_(boost::asio::serial_port(io_service_, endpoint_com_.data()))
{
    // Set serial port options (adjust based on your device's requirements)
    serial_.set_option(boost::asio::serial_port_base::baud_rate(115200));
    serial_.set_option(boost::asio::serial_port_base::character_size(8));
    serial_.set_option(boost::asio::serial_port_base::parity(boost::asio::serial_port_base::parity::none));
    serial_.set_option(boost::asio::serial_port_base::stop_bits(boost::asio::serial_port_base::stop_bits::one));
    serial_.set_option(boost::asio::serial_port_base::flow_control(boost::asio::serial_port_base::flow_control::none));

    logger_.log(LogLevel::kLogInfo, "[COM] port listener class instance configured");
    logger_.log(LogLevel::kLogInfo, "[PARAMS] baud_rate = 115200;\ncharacter_size=8\nparity::none;\nstop_bits::one;\nflow_control::none");
}

void COMListener::startListening() {
    logger_.log(LogLevel::kLogInfo, "[COM] port Listening thread started");
    listening_thread_ = std::thread(&COMListener::startListeningThread, this);
}

void COMListener::stopListening() {
    logger_.log(LogLevel::kLogInfo, "[COM] port stop listening initiated");

	is_listening_done = true;
    if (listening_thread_.joinable()) {
        listening_thread_.join();
      logger_.log(LogLevel::kLogInfo, "[COM] port listening thread joined");
    }
}

void COMListener::sendCommand(const std::string_view& command) {
    boost::system::error_code error;

    // Write the command to the serial port
    boost::asio::write(serial_, boost::asio::buffer(command), error);
    logger_.log(LogLevel::kLogInfo, "[COM] command send successfully");
}

void COMListener::startListeningThread() {
    std::ofstream odata_file(odata_filename_.data(),  std::ios::out);

    if (!odata_file.is_open()) {
        logger_.log(LogLevel::kLogError, "Failed to open output file.");
        return;
    }

    // Buffer to hold incoming data
    char buffer[buffer_size_];

    logger_.log(LogLevel::kLogInfo, "[COM] reading data from the serial port");
    // Keep reading data from the serial port
    while (true) {
        boost::system::error_code error;
        const std::size_t len = serial_.read_some(boost::asio::buffer(buffer, sizeof(buffer)), error);

        if (error == boost::asio::error::eof) {
            // Serial connection closed cleanly by peer
            logger_.log(LogLevel::kLogError, "Connection closed by peer");
            break;
        }

        if (error) {
            // Some other error occurred
            throw boost::system::system_error(error);
        }

      	{
            std::lock_guard lock(file_mutex);
            odata_file.write(buffer, static_cast<std::streamsize>(len));
            odata_file.flush();
        logger_.log(LogLevel::kLogInfo, "[COM] data wrote to output file successfully");
        }

        {
            std::lock_guard lock(console_mutex);
            std::cout.write(buffer, static_cast<std::streamsize>(len));
            std::cout.flush();
        }

        // Check if join requested
        {
            std::lock_guard lock(mtx);
            if (is_listening_done) {
                break;  // Stop the loop when signaled
            }
        }
    }
}