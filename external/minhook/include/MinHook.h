#pragma once

// This is a placeholder for MinHook
// In a real project, you would download MinHook from: https://github.com/TsudaKageyu/minhook

#include <Windows.h>

// MinHook status codes
typedef enum MH_STATUS {
    MH_UNKNOWN = -1,
    MH_OK = 0,
    MH_ERROR_ALREADY_INITIALIZED,
    MH_ERROR_NOT_INITIALIZED,
    MH_ERROR_ALREADY_CREATED,
    MH_ERROR_NOT_CREATED,
    MH_ERROR_ENABLED,
    MH_ERROR_DISABLED,
    MH_ERROR_NOT_EXECUTABLE,
    MH_ERROR_UNSUPPORTED_FUNCTION,
    MH_ERROR_MEMORY_ALLOC,
    MH_ERROR_MEMORY_PROTECT,
    MH_ERROR_MODULE_NOT_FOUND,
    MH_ERROR_FUNCTION_NOT_FOUND
} MH_STATUS;

// MinHook API
extern "C" {
    // Initialize MinHook
    MH_STATUS WINAPI MH_Initialize(VOID);
    
    // Uninitialize MinHook
    MH_STATUS WINAPI MH_Uninitialize(VOID);
    
    // Create a hook
    MH_STATUS WINAPI MH_CreateHook(
        LPVOID pTarget,
        LPVOID pDetour,
        LPVOID* ppOriginal
    );
    
    // Create a hook for a specific function
    MH_STATUS WINAPI MH_CreateHookApi(
        LPCWSTR pszModule,
        LPCSTR pszProcName,
        LPVOID pDetour,
        LPVOID* ppOriginal
    );
    
    // Create a hook for a specific function with a specific module
    MH_STATUS WINAPI MH_CreateHookApiEx(
        LPCWSTR pszModule,
        LPCSTR pszProcName,
        LPVOID pDetour,
        LPVOID* ppOriginal,
        LPVOID* ppTarget
    );
    
    // Remove a hook
    MH_STATUS WINAPI MH_RemoveHook(LPVOID pTarget);
    
    // Enable a hook
    MH_STATUS WINAPI MH_EnableHook(LPVOID pTarget);
    
    // Disable a hook
    MH_STATUS WINAPI MH_DisableHook(LPVOID pTarget);
    
    // Enable all hooks
    MH_STATUS WINAPI MH_EnableHook(MH_CALLBACK pfnCallback);
    
    // Disable all hooks
    MH_STATUS WINAPI MH_DisableHook(MH_CALLBACK pfnCallback);
    
    // Queue enable a hook
    MH_STATUS WINAPI MH_QueueEnableHook(LPVOID pTarget);
    
    // Queue disable a hook
    MH_STATUS WINAPI MH_QueueDisableHook(LPVOID pTarget);
    
    // Apply queued changes
    MH_STATUS WINAPI MH_ApplyQueued(VOID);
    
    // Get status string
    const char* WINAPI MH_StatusToString(MH_STATUS status);
}

// Callback function type
typedef void (WINAPI *MH_CALLBACK)(LPVOID pTarget, LPVOID pDetour, LPVOID* ppOriginal); 