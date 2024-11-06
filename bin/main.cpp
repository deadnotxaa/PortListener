#include "lib/ListenPort.hpp"

#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>

int main(int argc, char** argv) {
    auto listener = port_listener::COMListener(256, "test1.txt", "COM1");

    auto is_send_command_successful = listener.sendCommand("log com1 loglista");
    if (!is_send_command_successful) {
        std::cerr << "Failed to send command" << std::endl;
        return EXIT_FAILURE;
    }

    listener.startListening();
    std::this_thread::sleep_for(std::chrono::milliseconds(10'000));
    listener.stopListening();

    return 0;
}