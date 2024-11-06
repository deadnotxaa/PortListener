#include "lib/ListenPort.hpp"

#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>

int main(int argc, char** argv) {
    auto listener = port_listener::COMListener(256, "test1.txt", "COM3");

    listener.startListening();
    listener.sendCommand("log bestxyz ontime 1\r\n");

    std::this_thread::sleep_for(std::chrono::milliseconds(10'000));
    listener.stopListening();

    return 0;
}