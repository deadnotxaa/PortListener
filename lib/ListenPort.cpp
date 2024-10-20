#include "ListenPort.hpp"

#include <boost/asio.hpp>

#include <iostream>
#include <iomanip>
#include <ctime>
#include <sstream>
#include <filesystem>
#include <fstream>

using boost::asio::ip::tcp;
using namespace port_listener;


Listener::Listener(const uint64_t buffer_size, const std::string_view& odata_filename)
            : buffer_size_(buffer_size)
{
    // Set inputted filename or unique if not stated
    setOutputDataFilename(odata_filename);
}

void Listener::setOutputDataFilename(const std::string_view& filename) {
    if (!filename.empty() && !std::filesystem::exists(filename)) {
        odata_filename_ = filename;
        return;
    }

    // TODO: Add overwrite support with copying old versions to specific folder
    std::cerr << "File name not specified or file with such name already exists.\n"
                 "File with unique name will be created instead" << std::endl;

    // Creating file with unique name
    const auto t = std::time(nullptr);
    const auto tm = *std::localtime(&t);

    std::ostringstream oss;
    oss << std::put_time(&tm, "data_%d%m%Y_%H%M%S"); // TODO: Add custom file name formatting
    const auto str = oss.str();

    odata_filename_ = str;
}


TCPListener::TCPListener(
    const uint64_t buffer_size,
    const std::string_view& odata_filename,
    const std::string_view& endpoint_ip,
    const std::string_view& endpoint_port
    )
    : Listener(buffer_size, odata_filename), endpoint_ip_(endpoint_ip), endpoint_port_(endpoint_port)
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

void TCPListener::startListeningThread() {
    std::ofstream odata_file(odata_filename_, std::ios::binary | std::ios::out);

    if (!odata_file.is_open()) {
        std::cerr << "Failed to open output file." << std::endl;
        return;
    }

    boost::asio::io_context io_context;

    // Define a TCP endpoint with the local host and the port
    tcp::resolver resolver(io_context);
    const tcp::resolver::results_type endpoints = resolver.resolve(endpoint_ip_, endpoint_port_); // Change port number accordingly

    // Create a socket
    tcp::socket socket(io_context);

    // Connect to the endpoint
    [[maybe_unused]] auto tcp = boost::asio::connect(socket, endpoints);

    // Buffer to hold incoming data
    char buffer[buffer_size_];

    // Keep reading data from the socket
    while (true) {
        boost::system::error_code error;
        size_t len = socket.read_some(boost::asio::buffer(buffer, sizeof(buffer)), error);

        if (error == boost::asio::error::eof) {
            // Connection closed cleanly by peer
            std::cerr << "Connection closed by peer" << std::endl;
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
    : Listener(buffer_size, odata_filename), endpoint_com_(endpoint_com)
{}

void COMListener::startListening() {
    listening_thread_ = std::thread(&COMListener::startListeningThread, this);
}

void COMListener::stopListening() {
    {
        std::lock_guard lock(mtx);
        is_listening_done = true;
    }
    cv.notify_one();

    if (listening_thread_.joinable()) {
        listening_thread_.join();
    }
}

void COMListener::startListeningThread() {
    std::ofstream odata_file(odata_filename_, std::ios::binary | std::ios::out);

    if (!odata_file.is_open()) {
        std::cerr << "Failed to open output file." << std::endl;
        return;
    }

    boost::asio::io_context io_context;

    // Create a serial port object
    boost::asio::serial_port serial(io_context, endpoint_com_.data());

    // Set serial port parameters (baud rate, character size, parity, stop bits, etc.)
    serial.set_option(boost::asio::serial_port_base::baud_rate(9600));  // You can adjust baud rate here
    serial.set_option(boost::asio::serial_port_base::character_size(8));
    serial.set_option(boost::asio::serial_port_base::parity(boost::asio::serial_port_base::parity::none));
    serial.set_option(boost::asio::serial_port_base::stop_bits(boost::asio::serial_port_base::stop_bits::one));
    serial.set_option(boost::asio::serial_port_base::flow_control(boost::asio::serial_port_base::flow_control::none));

    // Buffer to hold incoming data
    char buffer[buffer_size_];

    // Keep reading data from the serial port
    while (true) {
        boost::system::error_code error;
        const size_t len = serial.read_some(boost::asio::buffer(buffer, sizeof(buffer)), error);

        if (error == boost::asio::error::eof) {
            // Serial connection closed cleanly by peer
            std::cerr << "Connection closed by peer" << std::endl;
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
