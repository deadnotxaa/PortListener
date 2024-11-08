#include "lib/ListenPort.hpp"

#include <chrono>
#include <thread>

int main(int argc, char** argv) {
    auto restart_listener = port_listener::COMListener(1024, "test1.txt", "COM3");

    restart_listener.sendCommand("FRESET STANDARD\r\n");
    std::this_thread::sleep_for(std::chrono::milliseconds(10'000));
    
                                                          
	auto listener = port_listener::COMListener(1024, "test1.txt", "COM3");
    listener.sendCommand("LOG TIMEA ONTIME 0.2\r\n");

    listener.startListening();
    std::this_thread::sleep_for(std::chrono::milliseconds(1'000));
    listener.stopListening();

    return 0;
}

/*#include <iostream>
#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>
#include <string>
#include <thread>
#include <fstream>

using namespace boost::asio;

void sendCommandAndReadResponse(const std::string& port_name) {
    try {
      	std::ofstream file("test_log.txt", std::ios::out);
        // Set up IO context and serial port
        io_service io;
        serial_port serial(io, port_name);

        // Configure serial port settings
        serial.set_option(serial_port_base::baud_rate(115200));
        serial.set_option(serial_port_base::character_size(8));
        serial.set_option(serial_port_base::parity(serial_port_base::parity::none));
        serial.set_option(serial_port_base::stop_bits(serial_port_base::stop_bits::one));
        serial.set_option(serial_port_base::flow_control(serial_port_base::flow_control::none));

        // Command to request the TIME log (NovAtel syntax for OEM6) log bestxyza ontime 1\r\n FRESET STANDARD
        std::string command = "LOG TIMEA ONCE\r\n";

        // Write the command to the port
        boost::system::error_code error;
        write(serial, buffer(command), error);
        if (error) {
            std::cerr << "Failed to send command: " << error.message() << std::endl;
            return;
        }
        std::cout << "Command sent: " << command;

        // Read the response from the device
        char read_buf[256];
        while (true) {
            size_t len = serial.read_some(buffer(read_buf), error);

            if (error == boost::asio::error::eof) {
                break; // End of file, port was closed
            } else if (error) {
                std::cerr << "Error reading: " << error.message() << std::endl;
                break;
            }
          
          	file.write(read_buf, len);
          	file.flush();

            // Print the response
            std::cout.write(read_buf, len);
            std::cout.flush();
        }

    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
}

int main() {
    // Replace with the name of the serial port, e.g., "COM3" on Windows or "/dev/ttyUSB0" on Linux
    std::string port_name = "COM3"; // Adjust as needed

    // Start the command function
    sendCommandAndReadResponse(port_name);

    return 0;
}*/