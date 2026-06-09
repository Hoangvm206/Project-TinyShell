#include <iostream>
#include <windows.h>

int main() {
    std::cout << "[SYSTEM] Monitor Countdown Started (PID: " << GetCurrentProcessId() << ")\n";
    for (int i = 60; i >= 0; --i) {
        std::cout << "[Countdown] Time remaining: " << i << "s\n";
        Sleep(1000);
    }
    std::cout << "[SYSTEM] Countdown finished.\n";
    return 0;
}