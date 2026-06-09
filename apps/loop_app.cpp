#include <iostream>
#include <windows.h>

int main() {
    int count = 0;
    std::cout << "--- Loop App Started (PID: " << GetCurrentProcessId() << ") ---\n";
    while (true) {
        std::cout << "[Loop App] Working hard... Tick: " << count++ << std::endl;
        Sleep(1000); // Ngủ 1 giây
    }
    return 0;
}