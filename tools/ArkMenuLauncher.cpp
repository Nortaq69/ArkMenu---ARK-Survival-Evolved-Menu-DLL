#include <Windows.h>
#include <TlHelp32.h>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>

// Helper: Get process ID by name
DWORD GetProcessIdByName(const std::wstring& processName) {
    DWORD processId = 0;
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32W processEntry;
        processEntry.dwSize = sizeof(processEntry);
        if (Process32FirstW(snapshot, &processEntry)) {
            do {
                if (_wcsicmp(processEntry.szExeFile, processName.c_str()) == 0) {
                    processId = processEntry.th32ProcessID;
                    break;
                }
            } while (Process32NextW(snapshot, &processEntry));
        }
        CloseHandle(snapshot);
    }
    return processId;
}

// Helper: Inject DLL into process
bool InjectDLL(DWORD processId, const std::wstring& dllPath) {
    HANDLE processHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);
    if (!processHandle) return false;
    SIZE_T pathSize = (dllPath.length() + 1) * sizeof(wchar_t);
    LPVOID remotePath = VirtualAllocEx(processHandle, NULL, pathSize, MEM_COMMIT, PAGE_READWRITE);
    if (!remotePath) { CloseHandle(processHandle); return false; }
    if (!WriteProcessMemory(processHandle, remotePath, dllPath.c_str(), pathSize, NULL)) {
        VirtualFreeEx(processHandle, remotePath, 0, MEM_RELEASE);
        CloseHandle(processHandle);
        return false;
    }
    HMODULE kernel32 = GetModuleHandleW(L"kernel32.dll");
    LPVOID loadLibraryAddr = GetProcAddress(kernel32, "LoadLibraryW");
    HANDLE threadHandle = CreateRemoteThread(processHandle, NULL, 0, (LPTHREAD_START_ROUTINE)loadLibraryAddr, remotePath, 0, NULL);
    if (!threadHandle) {
        VirtualFreeEx(processHandle, remotePath, 0, MEM_RELEASE);
        CloseHandle(processHandle);
        return false;
    }
    WaitForSingleObject(threadHandle, INFINITE);
    CloseHandle(threadHandle);
    VirtualFreeEx(processHandle, remotePath, 0, MEM_RELEASE);
    CloseHandle(processHandle);
    return true;
}

// Helper: Launch ARK via Microsoft Store URI
bool LaunchArk() {
    // Microsoft Store ARK URI (may need adjustment for your install)
    // This is the generic way to launch a UWP app:
    std::wstring arkUri = L"shell:AppsFolder\StudioWildcard.4558480580BB9_ark\ARK: Survival Evolved";
    // Try to launch via explorer
    HINSTANCE result = ShellExecuteW(NULL, L"open", L"explorer.exe", arkUri.c_str(), NULL, SW_SHOWNORMAL);
    return ((INT_PTR)result > 32);
}

int main() {
    std::wcout << L"ArkMenu Launcher v1.0" << std::endl;
    std::wcout << L"=====================" << std::endl;

    // Step 1: Launch ARK
    std::wcout << L"Launching ARK: Survival Evolved (Microsoft Store)..." << std::endl;
    if (!LaunchArk()) {
        std::wcout << L"Failed to launch ARK via Microsoft Store URI." << std::endl;
        std::wcout << L"You may need to adjust the URI in the launcher source." << std::endl;
        std::wcout << L"Press any key to exit..." << std::endl;
        std::cin.get();
        return 1;
    }

    // Step 2: Wait for ShooterGame.exe
    std::wcout << L"Waiting for ShooterGame.exe to start..." << std::endl;
    DWORD pid = 0;
    for (int i = 0; i < 120; ++i) { // Wait up to 2 minutes
        pid = GetProcessIdByName(L"ShooterGame.exe");
        if (pid != 0) break;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    if (pid == 0) {
        std::wcout << L"ShooterGame.exe not found after 2 minutes. Exiting." << std::endl;
        std::wcout << L"Press any key to exit..." << std::endl;
        std::cin.get();
        return 1;
    }
    std::wcout << L"Found ShooterGame.exe (PID: " << pid << L")" << std::endl;

    // Step 3: Inject ArkMenu.dll
    wchar_t currentDir[MAX_PATH];
    GetCurrentDirectoryW(MAX_PATH, currentDir);
    std::wstring dllPath = std::wstring(currentDir) + L"\\..\\build\\bin\\Release\\ArkMenu.dll";
    if (GetFileAttributesW(dllPath.c_str()) == INVALID_FILE_ATTRIBUTES) {
        std::wcout << L"DLL not found at: " << dllPath << std::endl;
        std::wcout << L"Please build the project first using build.bat" << std::endl;
        std::wcout << L"Press any key to exit..." << std::endl;
        std::cin.get();
        return 1;
    }
    std::wcout << L"Injecting ArkMenu.dll..." << std::endl;
    if (InjectDLL(pid, dllPath)) {
        std::wcout << L"Injection successful! Press Delete in-game to open the menu." << std::endl;
    } else {
        std::wcout << L"Injection failed!" << std::endl;
    }
    std::wcout << L"Press any key to exit..." << std::endl;
    std::cin.get();
    return 0;
} 