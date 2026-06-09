#include <iostream>
#include <windows.h>

int main() {
    std::cout << "--- Quick App Started (PID: " << GetCurrentProcessId() << ") ---\n";
    std::cout << "I will self-destruct in 4 seconds...\n";
    Sleep(4000);
    std::cout << "Goodbye!\n";
    return 0;
}