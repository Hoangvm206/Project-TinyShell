#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <algorithm>
#include <ctime>
#include <csignal>
#include <windows.h>
#include<stdio.h>
#ifndef WINVER
#define WINVER 0x0600
#endif

extern "C" {
    BOOL WINAPI K32GetProcessMemoryInfo(HANDLE, PVOID, DWORD);
    BOOL WINAPI QueryFullProcessImageNameA(HANDLE, DWORD, LPSTR, PDWORD);
}
using namespace std;

class TinyShell {
private:
    struct ProcessInfo {
        int id;
        string name;
        bool isSuspended;
        PROCESS_INFORMATION processInfo;
        FILETIME createTime;

        ProcessInfo(int _id, const string& _name, PROCESS_INFORMATION& _pi)
            : id(_id), name(_name), isSuspended(false), processInfo(_pi) {

            HANDLE hProcess = _pi.hProcess;
            FILETIME exit, kernel, user;
            GetProcessTimes(hProcess, &createTime, &exit, &kernel, &user);
        }
    };

    map<int, ProcessInfo> processes;
    set<int> availableIds;
    map<string, int> commandMap;
    int nextId;
    volatile sig_atomic_t stopFlag;

    // lệnh
    void showHelp(const vector<string>& args);
    void exitShell(const vector<string>& args);
    void listDirectory(const vector<string>& args);
    void startProcess(const vector<string>& args);
    void stopProcess(const vector<string>& args);
    void resumeProcess(const vector<string>& args);
    void killProcess(const vector<string>& args);
    void listProcesses(const vector<string>& args);
    void clearScreen(const vector<string>& args);
    void showPath(const vector<string>& args);
    void addToPath(const vector<string>& args);
    void echoCommand(const vector<string>& args);
    void infoProcess(const vector<string>& args);
    void foregroundProcess(const vector<string>& args);

    // check
    bool isValidNumber(const string& str);
    string extractFileName(const string& path);
    vector<string> splitCommand(const string& line);
    void executeBatchFile(const string& filename);
    void cleanupProcesses();
    int getNewId();

public:
    TinyShell() : nextId(1), stopFlag(0) {
        initializeCommands();
    }

    void run();

private:
    void initializeCommands() {
        commandMap["help"] = 0;
        commandMap["exit"] = 1;
        commandMap["dir"] = 2;
        commandMap["start"] = 3;
        commandMap["stop"] = 4;
        commandMap["resume"] = 5;
        commandMap["kill"] = 6;
        commandMap["list"] = 7;
        commandMap["clear"] = 8;
        commandMap["path"] = 9;
        commandMap["addpath"] = 10;
        commandMap["echo"] = 11;
        commandMap["info"] = 12;
        commandMap["fg"] = 13;
    }

    void executeCommand(const vector<string>& args) {
        if (args.empty()) return;

        auto it = commandMap.find(args[0]);
        if (it == commandMap.end()) {
            cout << "Command not found: " << args[0]
                      << ". Type 'help' for available commands.\n";
            return;
        }

        switch (it->second) {
            case 0: showHelp(args); break;
            case 1: exitShell(args); break;
            case 2: listDirectory(args); break;
            case 3: startProcess(args); break;
            case 4: stopProcess(args); break;
            case 5: resumeProcess(args); break;
            case 6: killProcess(args); break;
            case 7: listProcesses(args); break;
            case 8: clearScreen(args); break;
            case 9: showPath(args); break;
            case 10: addToPath(args); break;
            case 11: echoCommand(args); break;
            case 12: infoProcess(args); break;
            case 13: foregroundProcess(args); break;
        }
    }
};

// --------------------------------------------------------------
// Lệnh help
// --------------------------------------------------------------

void TinyShell::showHelp(const vector<string>& args) {
    if (args.size() > 1) {
        cout << "Help command doesn't accept arguments.\n";
        return;
    }

    vector<string> helpLines = {
        "help               Show this help message",
        "exit               Exit the shell",
        "dir [path]        List directory contents",
        "start <file> [mode] Start a process (mode: background/foreground)",
        "stop <id>          Suspend a background process",
        "resume <id>          Resume a suspended process",
        "kill <id|-1>       Terminate a process or all (-1)",
        "list               List all background processes",
        "info <id>          Show detailed info about a process",
        "fg <id>          Bring a background process to foreground",
        "clear               Clear the screen",
        "path               Show PATH environment variable",
        "addpath <path>        Add directory to PATH",
        "echo <text>        Display a line of text"
    };

    cout << "\n========== TINY SHELL COMMANDS ==========\n\n";
    cout << left << setw(20) << "Command" << "Description\n";
    cout << string(60, '-') << "\n";

    for (const string& line : helpLines) {
        size_t pos = line.find(' ');
        if (pos != string::npos) {
            string cmd = line.substr(0, pos);
            string desc = line.substr(pos + 1);
            cout << left << setw(20) << cmd << desc << "\n";
        } else {
            cout << line << "\n";
        }
    }
    cout << "\n=========================================\n";
}

void TinyShell::exitShell(const vector<string>& args) {
    cleanupProcesses();
    cout << "Exiting TinyShell...\n";
    exit(0);
}

void TinyShell::listDirectory(const vector<string>& args) {
    string searchPath = "*.*";
    if (args.size() > 1) {
        searchPath = args[1];
        if (searchPath.back() != '\\' && searchPath.find('.') == string::npos) {
            searchPath += "\\*";
        } else if (searchPath.find('*') == string::npos) {
            searchPath += "\\*.*";
        }
    }

    WIN32_FIND_DATAA findData;
    HANDLE findHandle = FindFirstFileA(searchPath.c_str(), &findData);

    if (findHandle == INVALID_HANDLE_VALUE) {
        cout << "Cannot access path: " << searchPath << "\n";
        return;
    }

    cout << "\n" << left << setw(10) << "Mode"
              << right << setw(20) << "Modified Date"
              << right << setw(12) << "Size"
              << "  Name\n";
    cout << string(60, '-') << "\n";

    do {
        string mode = "------";
        if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) mode[0] = 'd';
        if (findData.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE) mode[1] = 'a';
        if (findData.dwFileAttributes & FILE_ATTRIBUTE_READONLY) mode[2] = 'r';
        if (findData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) mode[3] = 'h';
        if (findData.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) mode[4] = 's';

        SYSTEMTIME sysTime;
        FileTimeToSystemTime(&findData.ftLastWriteTime, &sysTime);

        long long fileSize = (static_cast<long long>(findData.nFileSizeHigh) << 32) + findData.nFileSizeLow;

        cout << left << setw(10) << mode
                  << right << setw(8) << sysTime.wDay << "/" << sysTime.wMonth << "/" << sysTime.wYear
                  << setw(16) << fileSize
                  << "  " << findData.cFileName << "\n";

    } while (FindNextFileA(findHandle, &findData));

    FindClose(findHandle);
}

void TinyShell::startProcess(const vector<string>& args) {
    if (args.size() < 2) {
        cout << "Usage: start <executable> [background|foreground]\n";
        return;
    }

    string executable = args[1];
    bool foreground = (args.size() > 2 && args[2] == "foreground");

    if (executable.length() >= 4 && executable.substr(executable.length() - 4) == ".bat") {
        executeBatchFile(executable);
        return;
    }

    if (executable.find('.') == string::npos) {
        executable += ".exe";
    }

    PROCESS_INFORMATION pi;
    STARTUPINFOA si;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);

    if (!CreateProcessA(executable.c_str(), NULL, NULL, NULL, FALSE,
                       CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi)) {
        cout << "Failed to start process: " << executable << "\n";
        return;
    }

    if (foreground) {
        cout << "Process running in foreground. Press Ctrl+C to terminate...\n";
        WaitForSingleObject(pi.hProcess, INFINITE);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    } else {
        int id = getNewId();
        processes.emplace(id, ProcessInfo(id, extractFileName(executable), pi));
        cout << "Process started with ID: " << id << "\n";
    }
}

void TinyShell::stopProcess(const vector<string>& args) {
    if (args.size() < 2 || !isValidNumber(args[1])) {
        cout << "Please provide a valid process ID\n";
        return;
    }

    int id = stoi(args[1]);
    auto it = processes.find(id);

    if (it == processes.end()) {
        cout << "Process ID " << id << " not found\n";
        return;
    }

    if (it->second.isSuspended) {
        cout << "Process already suspended\n";
        return;
    }

    SuspendThread(it->second.processInfo.hThread);
    it->second.isSuspended = true;
    cout << "Process '" << it->second.name << "' suspended\n";
}

void TinyShell::resumeProcess(const vector<string>& args) {
    if (args.size() < 2 || !isValidNumber(args[1])) {
        cout << "Please provide a valid process ID\n";
        return;
    }

    int id = stoi(args[1]);
    auto it = processes.find(id);

    if (it == processes.end()) {
        cout << "Process ID " << id << " not found\n";
        return;
    }

    if (!it->second.isSuspended) {
        cout << "Process is already running\n";
        return;
    }

    ResumeThread(it->second.processInfo.hThread);
    it->second.isSuspended = false;
    cout << "Process '" << it->second.name << "' resumed\n";
}

void TinyShell::killProcess(const vector<string>& args) {
    if (args.size() < 2) {
        cout << "Usage: kill <process_id|-1>\n";
        return;
    }

    if (args[1] == "-1") {
        cleanupProcesses();
        cout << "All processes terminated\n";
        return;
    }

    if (!isValidNumber(args[1])) {
        cout << "Please provide a valid process ID\n";
        return;
    }

    int id = stoi(args[1]);
    auto it = processes.find(id);

    if (it == processes.end()) {
        cout << "Process ID " << id << " not found\n";
        return;
    }

    TerminateProcess(it->second.processInfo.hProcess, 0);
    CloseHandle(it->second.processInfo.hProcess);
    CloseHandle(it->second.processInfo.hThread);
    processes.erase(it);
    availableIds.insert(id);
    cout << "Process terminated\n";
}

void TinyShell::listProcesses(const vector<string>& args) {
    if (processes.empty()) {
        cout << "No background processes running\n";
        return;
    }

    cout << "\n" << left << setw(6) << "ID"
              << setw(10) << "PID"
              << setw(12) << "Status"
              << "Name\n";
    cout << string(50, '-') << "\n";

    for (auto it = processes.begin(); it != processes.end();) {
        DWORD exitCode;
        GetExitCodeProcess(it->second.processInfo.hProcess, &exitCode);

        if (exitCode != STILL_ACTIVE) {
            cout << left << setw(6) << it->first
                      << setw(10) << it->second.processInfo.dwProcessId
                      << setw(12) << "FINISHED"
                      << it->second.name << "  [cleaned up]\n";
            CloseHandle(it->second.processInfo.hProcess);
            CloseHandle(it->second.processInfo.hThread);
            availableIds.insert(it->first);
            it = processes.erase(it);
            continue;
        }

        cout << left << setw(6) << it->first
                  << setw(10) << it->second.processInfo.dwProcessId
                  << setw(12) << (it->second.isSuspended ? "SUSPENDED" : "RUNNING")
                  << it->second.name << "\n";
        ++it;
    }
    cout << "\n";
}

void TinyShell::clearScreen(const vector<string>& args) {
    system("cls");
}

void TinyShell::showPath(const vector<string>& args) {
    char* path = getenv("PATH");
    if (path) {
        cout << "Current PATH:\n";
        string pathStr(path);
        size_t pos = 0;
        while ((pos = pathStr.find(';')) != string::npos) {
            cout << "  " << pathStr.substr(0, pos) << "\n";
            pathStr.erase(0, pos + 1);
        }
        cout << "  " << pathStr << "\n";
    } else {
        cout << "PATH environment variable not found\n";
    }
}

void TinyShell::addToPath(const vector<string>& args) {
    if (args.size() < 2) {
        cout << "Please provide a directory path\n";
        return;
    }

    string newPath = args[1];

    if (GetFileAttributesA(newPath.c_str()) == INVALID_FILE_ATTRIBUTES) {
        cout << "Invalid directory path\n";
        return;
    }

    HKEY hKey;
    if (RegOpenKeyExA(HKEY_CURRENT_USER, "Environment", 0, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS) {
        string currentPath = getenv("PATH") ?: "";
        if (!currentPath.empty() && currentPath.back() != ';') {
            currentPath += ";";
        }
        currentPath += newPath;

        RegSetValueExA(hKey, "PATH", 0, REG_EXPAND_SZ,
                      (const BYTE*)currentPath.c_str(), currentPath.length() + 1);
        RegCloseKey(hKey);

        SendMessageTimeoutA(HWND_BROADCAST, WM_SETTINGCHANGE, 0,
                           (LPARAM)"Environment", SMTO_ABORTIFHUNG, 5000, nullptr);

        cout << "Directory added to PATH successfully\n";
    } else {
        cout << "Failed to modify PATH\n";
    }
}

void TinyShell::echoCommand(const vector<string>& args) {
    if (args.size() < 2) {
        cout << "\n";
        return;
    }

    for (size_t i = 1; i < args.size(); ++i) {
        if (i > 1) cout << " ";
        cout << args[i];
    }
    cout << "\n";
}

// Hàm infoProcess
void TinyShell::infoProcess(const vector<string>& args) {
    if (args.size() < 2 || !isValidNumber(args[1])) {
        cout << "Usage: info <process_id>\n";
        return;
    }

    int id = stoi(args[1]);
    auto it = processes.find(id);
    if (it == processes.end()) {
        cout << "Process ID " << id << " not found\n";
        return;
    }

    ProcessInfo& p = it->second;
    HANDLE hProcess = p.processInfo.hProcess;

    // FIX: kiểm tra process còn sống không trước khi lấy thông tin
    DWORD exitCode;
    GetExitCodeProcess(hProcess, &exitCode);
    if (exitCode != STILL_ACTIVE) {
        cout << "Process ID " << id << " has already terminated. Cleaning up.\n";
        CloseHandle(hProcess);
        CloseHandle(p.processInfo.hThread);
        availableIds.insert(id);
        processes.erase(it);
        return;
    }

    // 1. Tính Uptime
    FILETIME nowFt;
    GetSystemTimeAsFileTime(&nowFt);
    ULARGE_INTEGER nowLi, createLi;
    nowLi.LowPart = nowFt.dwLowDateTime;
    nowLi.HighPart = nowFt.dwHighDateTime;
    createLi.LowPart = p.createTime.dwLowDateTime;
    createLi.HighPart = p.createTime.dwHighDateTime;
    unsigned long long elapsed = (nowLi.QuadPart - createLi.QuadPart) / 10000000;
    int hours = (int)(elapsed / 3600), minutes = (int)((elapsed % 3600) / 60), seconds = (int)(elapsed % 60);

   // 2. Lấy RAM bằng cách tự định nghĩa struct và ép kiểu PVOID
    struct {
        DWORD cb; DWORD PageFaultCount; SIZE_T PeakWorkingSetSize; SIZE_T WorkingSetSize;
        SIZE_T QuotaPeakPagedPoolUsage; SIZE_T QuotaPagedPoolUsage;
        SIZE_T QuotaPeakNonPagedPoolUsage; SIZE_T QuotaNonPagedPoolUsage;
        SIZE_T PagefileUsage; SIZE_T PeakPagefileUsage;
    } myPmc;

    string memUsage = "Unknown";
    if (K32GetProcessMemoryInfo(hProcess, (PVOID)&myPmc, sizeof(myPmc))) {
        memUsage = to_string(myPmc.WorkingSetSize / 1024) + " KB";
    }

    // 3. Lấy đường dẫn file bằng QueryFullProcessImageNameA
    char exePath[MAX_PATH];
    DWORD sz = MAX_PATH;
    string pathStr = "unknown";
    if (QueryFullProcessImageNameA(hProcess, 0, exePath, (PDWORD)&sz)) {
        pathStr = exePath;
    }

    cout << "\n========== PROCESS INFO (ID " << id << ") ==========\n";
    cout << "Name:        " << p.name << "\n";
    cout << "PID:         " << p.processInfo.dwProcessId << "\n";
    cout << "Status:      " << (p.isSuspended ? "SUSPENDED" : "RUNNING") << "\n";
    cout << "Uptime:      " << hours << "h " << minutes << "m " << seconds << "s\n";
    cout << "Memory:      " << memUsage << "\n";
    cout << "Executable:  " << pathStr << "\n";
    cout << "==========================================\n";
}
// Lệnh fg
void TinyShell::foregroundProcess(const vector<string>& args) {
    if (args.size() < 2 || !isValidNumber(args[1])) {
        cout << "Usage: fg <process_id>\n";
        return;
    }

    int id = stoi(args[1]);
    auto it = processes.find(id);
    if (it == processes.end()) {
        cout << "Process ID " << id << " not found\n";
        return;
    }

    ProcessInfo& p = it->second;  // FIX: dùng reference để cập nhật map
    HANDLE hProcess = p.processInfo.hProcess;

    DWORD exitCode;
    GetExitCodeProcess(hProcess, &exitCode);
    if (exitCode != STILL_ACTIVE) {
        cout << "Process has already terminated\n";
        CloseHandle(hProcess);
        CloseHandle(p.processInfo.hThread);
        availableIds.insert(id);
        processes.erase(it);
        return;
    }

    // Tự động Resume nếu tiến trình đang bị Stop
    if (p.isSuspended) {
        ResumeThread(p.processInfo.hThread);
        p.isSuspended = false;  // giờ cập nhật đúng vào map
    }

    cout << "Bringing process '" << p.name << "' to foreground. Waiting...\n";
    WaitForSingleObject(hProcess, INFINITE);

    CloseHandle(hProcess);
    CloseHandle(p.processInfo.hThread);
    availableIds.insert(id);
    processes.erase(it);
    cout << "Process finished. Returning to shell.\n";
}

// --------------------------------------------------------------
// Utility functions
// --------------------------------------------------------------

bool TinyShell::isValidNumber(const string& str) {
    return !str.empty() && all_of(str.begin(), str.end(), ::isdigit);
}

string TinyShell::extractFileName(const string& path) {
    size_t pos = path.find_last_of("\\/");
    return (pos != string::npos) ? path.substr(pos + 1) : path;
}

vector<string> TinyShell::splitCommand(const string& line) {
    vector<string> args;
    string arg;
    bool inQuotes = false;

    for (size_t i = 0; i < line.size(); ++i) {
        char c = line[i];
        if (c == '"') {
            inQuotes = !inQuotes;
        } else if (c == ' ' && !inQuotes) {
            if (!arg.empty()) {
                if (args.empty()) {
                    transform(arg.begin(), arg.end(), arg.begin(), ::tolower);
                }
                args.push_back(arg);
                arg.clear();
            }
        } else {
            arg += c;
        }
    }
    if (!arg.empty()) {
        if (args.empty()) {
            transform(arg.begin(), arg.end(), arg.begin(), ::tolower);
        }
        args.push_back(arg);
    }
    return args;
}

void TinyShell::executeBatchFile(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cout << "Cannot open batch file: " << filename << "\n";
        return;
    }

    string line;
    while (getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;

        vector<string> args = splitCommand(line);
        if (!args.empty()) {
            cout << "> " << line << "\n";
            executeCommand(args);
        }
    }
    file.close();
}

void TinyShell::cleanupProcesses() {
    for (auto& pair : processes) {
        TerminateProcess(pair.second.processInfo.hProcess, 0);
        CloseHandle(pair.second.processInfo.hProcess);
        CloseHandle(pair.second.processInfo.hThread);
    }
    processes.clear();
    availableIds.clear();
    nextId = 1;  // FIX: reset ID counter sau khi dọn sạch
}

int TinyShell::getNewId() {
    if (!availableIds.empty()) {
        int id = *availableIds.begin();
        availableIds.erase(availableIds.begin());
        return id;
    }
    // FIX: nếu processes rỗng, reset nextId về 1 để tránh ID tăng mãi
    if (processes.empty()) {
        nextId = 1;
    }
    return nextId++;
}

void TinyShell::run() {
    SetConsoleTitle("TinyShell");
    cout << "Welcome to TinyShell! Type 'help' for commands.\n\n";

    string input;
    while (true) {
        cout << "TinyShell> ";
        getline(cin, input);

        if (input.empty()) continue;

        vector<string> args = splitCommand(input);
        executeCommand(args);
    }
}

int main() {
    TinyShell shell;
    shell.run();
    return 0;
}
