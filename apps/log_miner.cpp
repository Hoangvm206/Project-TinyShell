#include <iostream>
#include <windows.h>

int main() {
    std::cout << "[LOG MINER] Service initialized. Monitoring events... (PID: " << GetCurrentProcessId() << ")\n";
    int eventId = 100;
    while (true) {
        std::cout << "[LOG UNKNOWN] Analyzing packet ID: " << eventId++ << " -> Status: OK\n";
        Sleep(1500);
    }
    return 0;
}