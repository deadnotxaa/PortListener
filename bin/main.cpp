#include "lib/ListenPort.hpp"

#include <chrono>
#include <thread>

int main(int argc, char** argv) {
    if (argc > 1 && strcmp(argv[1], "no-reset") != 0) {
        auto restart_listener = port_listener::COMListener(1024, "test1.txt", "COM3");
        restart_listener.sendCommand("FRESET STANDARD\r\n");

        std::this_thread::sleep_for(std::chrono::milliseconds(10'000));
    }

    std::string_view input_filename = (argc > 2) ? argv[2] : "test.txt";
	auto listener = port_listener::COMListener(1024, input_filename, "COM3");

    if (argc > 3) {
        std::ifstream command_file(argv[3]);
        std::string command;

        while (std::getline(command_file, command)) {
            if (command.empty()) {
                continue;
            }

            listener.sendCommand(command + "\r\n");
        }
    } else {
        listener.sendCommand("LOG TIMEA ONTIME 1\r\n");
    }

    listener.startListening();

    if (argc > 4) {
        std::this_thread::sleep_for(std::chrono::milliseconds(std::stoi(argv[4]) * 1000));
    } else {
        std::string input;

        while (input.empty()) {
            std::cin >> input;
        }
    }

    listener.stopListening();

    return 0;
}
