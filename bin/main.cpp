#include "lib/ListenPort.hpp"

#include <iostream>

int main(int argc, char** argv) {
    auto listener = port_listener::COMListener(1024, "test1.txt", "COM3");
    listener.startListening();
    listener.sendCommand("LOG USB1 TIMEA ONTIME 1");
    listener.stopListening();
    return 0;
}