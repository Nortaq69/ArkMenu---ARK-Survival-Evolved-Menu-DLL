#include <Windows.h>
#include <TlHelp32.h>
#include <iostream>
#include <string>

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

bool InjectDLL(DWORD processId, const std::wstring& dllPath) {
    HANDLE processHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);
    if (!processHandle) {
        std::wcout << L"Failed to open process. Error: " << GetLastError() << std::endl;
        return false;
    }
    
    // Allocate memory for DLL path
    SIZE_T pathSize = (dllPath.length() + 1) * sizeof(wchar_t);
    LPVOID remotePath = VirtualAllocEx(processHandle, NULL, pathSize, MEM_COMMIT, PAGE_READWRITE);
    if (!remotePath) {
        std::wcout << L"Failed to allocate memory in target process." << std::endl;
        CloseHandle(processHandle);
        return false;
    }
    
    // Write DLL path to target process
    if (!WriteProcessMemory(processHandle, remotePath, dllPath.c_str(), pathSize, NULL)) {
        std::wcout << L"Failed to write DLL path to target process." << std::endl;
        VirtualFreeEx(processHandle, remotePath, 0, MEM_RELEASE);
        CloseHandle(processHandle);
        return false;
    }
    
    // Get LoadLibraryW address
    HMODULE kernel32 = GetModuleHandleW(L"kernel32.dll");
    LPVOID loadLibraryAddr = GetProcAddress(kernel32, "LoadLibraryW");
    
    // Create remote thread to load DLL
    HANDLE threadHandle = CreateRemoteThread(processHandle, NULL, 0, 
        (LPTHREAD_START_ROUTINE)loadLibraryAddr, remotePath, 0, NULL);
    
    if (!threadHandle) {
        std::wcout << L"Failed to create remote thread. Error: " << GetLastError() << std::endl;
        VirtualFreeEx(processHandle, remotePath, 0, MEM_RELEASE);
        CloseHandle(processHandle);
        return false;
    }
    
    // Wait for thread to complete
    WaitForSingleObject(threadHandle, INFINITE);
    
    // Get thread exit code (DLL handle)
    DWORD exitCode;
    GetExitCodeThread(threadHandle, &exitCode);
    
    // Cleanup
    CloseHandle(threadHandle);
    VirtualFreeEx(processHandle, remotePath, 0, MEM_RELEASE);
    CloseHandle(processHandle);
    
    if (exitCode == 0) {
        std::wcout << L"DLL injection failed." << std::endl;
        return false;
    }
    
    std::wcout << L"DLL injected successfully!" << std::endl;
    return true;
}

int main() {
    std::wcout << L"ArkMenu Injector v1.0" << std::endl;
    std::wcout << L"=====================" << std::endl;
    
    // Find ARK process
    std::wcout << L"Looking for ARK: Survival Evolved process..." << std::endl;
    DWORD processId = GetProcessIdByName(L"ShooterGame.exe");
    
    if (processId == 0) {
        std::wcout << L"ARK process not found. Make sure ARK is running." << std::endl;
        std::wcout << L"Press any key to exit..." << std::endl;
        std::cin.get();
        return 1;
    }
    
    std::wcout << L"Found ARK process (PID: " << processId << L")" << std::endl;
    
    // Get DLL path
    wchar_t currentDir[MAX_PATH];
    GetCurrentDirectoryW(MAX_PATH, currentDir);
    std::wstring dllPath = std::wstring(currentDir) + L"\\..\\build\\bin\\Release\\ArkMenu.dll";
    
    // Check if DLL exists
    if (GetFileAttributesW(dllPath.c_str()) == INVALID_FILE_ATTRIBUTES) {
        std::wcout << L"DLL not found at: " << dllPath << std::endl;
        std::wcout << L"Please build the project first using build.bat" << std::endl;
        std::wcout << L"Press any key to exit..." << std::endl;
        std::cin.get();
        return 1;
    }
    
    std::wcout << L"Found DLL at: " << dllPath << std::endl;
    
    // Inject DLL
    std::wcout << L"Injecting DLL..." << std::endl;
    if (InjectDLL(processId, dllPath)) {
        std::wcout << L"Injection successful!" << std::endl;
        std::wcout << L"Press Delete key in ARK to open/close the menu." << std::endl;
    } else {
        std::wcout << L"Injection failed!" << std::endl;
    }
    
    std::wcout << L"Press any key to exit..." << std::endl;
    std::cin.get();
    return 0;
} 