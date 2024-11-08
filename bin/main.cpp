#include "lib/ListenPort.hpp"

#include <chrono>
#include <thread>

int main(int argc, char** argv) {
    if (argc > 1 && strcmp(argv[1], "reset") == 0) {
        auto restart_listener = port_listener::COMListener(1024, "test1.txt", "COM3");
        restart_listener.sendCommand("FRESET STANDARD\r\n");

        std::this_thread::sleep_for(std::chrono::milliseconds(10'000));
    }

    std::string_view filename = (argc > 2) ? argv[2] : "test.txt";
                                                          
	auto listener = port_listener::COMListener(1024, filename, "COM3");
    listener.sendCommand("LOG TIMEA ONTIME 0.2\r\n");
    listener.startListening();
    std::cout << "Enter 'q' to stop listening" << std::endl;
    std::string input;

    while (input != "q") {
        std::cin >> input;
    }

    listener.stopListening();

    return 0;
}
