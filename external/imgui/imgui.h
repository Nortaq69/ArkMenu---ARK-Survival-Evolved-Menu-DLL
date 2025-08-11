#pragma once

// This is a placeholder for ImGui
// In a real project, you would download ImGui from: https://github.com/ocornut/imgui

#include <string>

struct ImVec2 {
    float x, y;
    ImVec2() { x = y = 0.0f; }
    ImVec2(float _x, float _y) { x = _x; y = _y; }
};

struct ImVec4 {
    float x, y, z, w;
    ImVec4() { x = y = z = w = 0.0f; }
    ImVec4(float _x, float _y, float _z, float _w) { x = _x; y = _y; z = _z; w = _w; }
};

struct ImColor {
    ImVec4 Value;
    ImColor() { Value.x = Value.y = Value.z = Value.w = 0.0f; }
    ImColor(float r, float g, float b, float a = 1.0f) { Value.x = r; Value.y = g; Value.z = b; Value.w = a; }
    ImColor(const ImVec4& col) { Value = col; }
    operator ImVec4() const { return Value; }
};

namespace ImGui {
    struct ImDrawList;
    struct ImFont;
    struct ImGuiIO;
    struct ImGuiStyle;
    struct ImFontConfig;
    
    // Core functions
    IMGUI_API void CreateContext();
    IMGUI_API void DestroyContext();
    IMGUI_API ImGuiIO& GetIO();
    IMGUI_API ImGuiStyle& GetStyle();
    IMGUI_API ImFont* GetFont();
    IMGUI_API float GetFontSize();
    IMGUI_API ImVec2 CalcTextSize(const char* text);
    IMGUI_API ImDrawList* GetBackgroundDrawList();
    IMGUI_API ImDrawList* GetCurrentWindow();
    IMGUI_API void NewFrame();
    IMGUI_API void Render();
    IMGUI_API void EndFrame();
    IMGUI_API void StyleColorsDark();
    IMGUI_API void CheckVersion();
    
    // Window functions
    IMGUI_API bool Begin(const char* name, bool* p_open = nullptr, ImGuiWindowFlags flags = 0);
    IMGUI_API void End();
    IMGUI_API void SetWindowSize(const ImVec2& size);
    IMGUI_API void SetWindowPos(const ImVec2& pos);
    IMGUI_API ImVec2 GetWindowSize();
    
    // Widget functions
    IMGUI_API bool Checkbox(const char* label, bool* v);
    IMGUI_API bool Button(const char* label, const ImVec2& size = ImVec2(0, 0));
    IMGUI_API bool SliderFloat(const char* label, float* v, float v_min, float v_max, const char* format = "%.3f");
    IMGUI_API bool ColorEdit3(const char* label, float col[3]);
    IMGUI_API void Text(const char* fmt, ...);
    IMGUI_API void SetCursorPos(const ImVec2& pos);
    
    // Style functions
    IMGUI_API void PushStyleVar(ImGuiStyleVar idx, float val);
    IMGUI_API void PushStyleVar(ImGuiStyleVar idx, const ImVec2& val);
    IMGUI_API void PopStyleVar(int count = 1);
    IMGUI_API void PushStyleColor(ImGuiCol idx, const ImVec4& col);
    IMGUI_API void PopStyleColor(int count = 1);
    
    // Tab functions
    IMGUI_API bool BeginTabBar(const char* str_id, ImGuiTabBarFlags flags = 0);
    IMGUI_API void EndTabBar();
    IMGUI_API bool BeginTabItem(const char* label, bool* p_open = nullptr, ImGuiTabBarFlags flags = 0);
    IMGUI_API void EndTabItem();
    
    // Font functions
    IMGUI_API ImFont* AddFontFromFileTTF(const char* filename, float size_pixels, const ImFontConfig* font_cfg = nullptr, const ImWchar* glyph_ranges = nullptr);
    IMGUI_API ImFont* AddFontFromMemoryTTF(void* font_data, int font_data_size, float size_pixels, const ImFontConfig* font_cfg = nullptr, const ImWchar* glyph_ranges = nullptr);
    IMGUI_API const ImWchar* GetGlyphRangesCyrillic();
    
    // Enums
    enum ImGuiWindowFlags_ {
        ImGuiWindowFlags_NoInputs = 1 << 0,
        ImGuiWindowFlags_NoTitleBar = 1 << 1,
        ImGuiWindowFlags_NoBackground = 1 << 2,
        ImGuiWindowFlags_NoScrollbar = 1 << 3,
        ImGuiWindowFlags_NoScrollWithMouse = 1 << 4,
        ImGuiWindowFlags_NoDecoration = 1 << 5
    };
    
    enum ImGuiCol_ {
        ImGuiCol_ChildBg,
        ImGuiCol_Border,
        ImGuiCol_FrameBg,
        ImGuiCol_FrameBgActive,
        ImGuiCol_FrameBgHovered,
        ImGuiCol_Header,
        ImGuiCol_HeaderActive,
        ImGuiCol_HeaderHovered,
        ImGuiCol_PopupBg,
        ImGuiCol_Button,
        ImGuiCol_ButtonHovered,
        ImGuiCol_ButtonActive,
        ImGuiCol_TitleBgActive,
        ImGuiCol_WindowBg,
        ImGuiCol_COUNT
    };
    
    enum ImGuiStyleVar_ {
        ImGuiStyleVar_WindowBorderSize,
        ImGuiStyleVar_WindowPadding,
        ImGuiStyleVar_Alpha,
        ImGuiStyleVar_WindowRounding,
        ImGuiStyleVar_FramePadding,
        ImGuiStyleVar_ItemInnerSpacing,
        ImGuiStyleVar_ItemSpacing,
        ImGuiStyleVar_FrameRounding,
        ImGuiStyleVar_ScrollbarSize,
        ImGuiStyleVar_ScrollbarRounding,
        ImGuiStyleVar_PopupRounding
    };
    
    enum ImGuiTabBarFlags_ {
        ImGuiTabBarFlags_None = 0
    };
}

// ImGui implementation functions
IMGUI_API bool ImGui_ImplDX11_Init(void* device, void* device_context);
IMGUI_API void ImGui_ImplDX11_Shutdown();
IMGUI_API void ImGui_ImplDX11_NewFrame();
IMGUI_API void ImGui_ImplDX11_RenderDrawData(ImDrawData* draw_data);
IMGUI_API bool ImGui_ImplDX11_CreateDeviceObjects();
IMGUI_API void ImGui_ImplDX11_InvalidateDeviceObjects();

IMGUI_API bool ImGui_ImplWin32_Init(void* hwnd);
IMGUI_API void ImGui_ImplWin32_Shutdown();
IMGUI_API void ImGui_ImplWin32_NewFrame();
IMGUI_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Forward declarations
struct ImDrawData;
struct ImDrawList;
struct ImFont;
struct ImGuiIO;
struct ImGuiStyle;
struct ImFontConfig;

// Macros
#define IMGUI_CHECKVERSION() ImGui::CheckVersion()
#define IMGUI_API __declspec(dllexport) 