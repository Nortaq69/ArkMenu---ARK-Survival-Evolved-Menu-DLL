#include "Menu.h"
#include "Helpers/random/byte.h"
#include "cheat.h"

bool NearbyNoglin = false;
std::string CompareName;
std::string CurrentContainer = "Storage Box";
int PossibleDinos = 0;
int AimBone = 8; // Head bone
ImFont* zzzz = nullptr;
ImFont* icons = nullptr;

// Icon font data (placeholder - you'll need to add actual icon font data)
unsigned char iconfont[] = { 0x00 };

void Renderer::RemoveInput()
{
    if (D3D.WndProcOriginal)
    {
        SetWindowLongPtrA(D3D.GameWindow, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(D3D.WndProcOriginal));
        D3D.WndProcOriginal = nullptr;
    }
}

void Renderer::HookInput()
{
    Renderer::RemoveInput();
    D3D.WndProcOriginal = reinterpret_cast<WNDPROC>(SetWindowLongPtrA(D3D.GameWindow, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(WndProc)));
}

bool Renderer::Remove()
{
    Renderer::RemoveInput();
    if (!RemoveHook(D3D.OriginalPresent) || !RemoveHook(D3D.SetCursorPosOriginal) || !RemoveHook(D3D.SetCursorOriginal))
    {
        return false;
    }
    if (D3D.RenderTargetView)
    {
        ImGui_ImplDX11_Shutdown();
        ImGui::DestroyContext();
        D3D.RenderTargetView->Release();
        D3D.RenderTargetView = nullptr;
    }
    if (D3D.Ctx)
    {
        D3D.Ctx->Release();
        D3D.Ctx = nullptr;
    }
    if (D3D.Device)
    {
        D3D.Device->Release();
        D3D.Device = nullptr;
    }
    return true;
}

BOOL WINAPI Renderer::SetCursorPosHook(int X, int Y)
{
    if (Settings.IsMenuOpen) return FALSE;
    return reinterpret_cast<decltype(&SetCursorPos)>(D3D.SetCursorPosOriginal)(X, Y);
}

HCURSOR WINAPI Renderer::SetCursorHook(HCURSOR hCursor)
{
    if (Settings.IsMenuOpen) return 0;
    return reinterpret_cast<decltype(&SetCursor)>(D3D.SetCursorOriginal)(hCursor);
}

bool Renderer::CreateView()
{
    ID3D11Texture2D* Buffer = nullptr;
    if (FAILED(D3D.SwapChain->GetBuffer(0, __uuidof(Buffer), reinterpret_cast<PVOID*>(&Buffer)))) return false;
    if (FAILED(D3D.Device->CreateRenderTargetView(Buffer, nullptr, &D3D.RenderTargetView))) return false;
    Buffer->Release();
    return true;
}

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT WINAPI Renderer::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
    {
        return true;
    }
    switch (msg)
    {
    case WM_SIZE:
        if (D3D.Device != nullptr && wParam != SIZE_MINIMIZED)
        {
            if (D3D.RenderTargetView)
            {
                D3D.RenderTargetView->Release();
                D3D.RenderTargetView = nullptr;
            }
            D3D.SwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING);
            CreateView();
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) return 0;
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hWnd, msg, wParam, lParam);
}

inline bool Renderer::Init()
{
    D3D.PresentFunc = GetD3D11PresentFunction();
    SetHook(D3D.PresentFunc, D3D_HOOK, reinterpret_cast<void**>(&D3D.OriginalPresent));
    SetHook(SetCursorPos, SetCursorPosHook, reinterpret_cast<void**>(&D3D.SetCursorPosOriginal));
    SetHook(SetCursor, SetCursorHook, reinterpret_cast<void**>(&D3D.SetCursorOriginal));
    SetHook(reinterpret_cast<void*>(Cache.GameBase + 0x173CA60), Utils::PE_HOOK, reinterpret_cast<PVOID*>(&Settings.OriginalPE));
    SetHook(reinterpret_cast<void*>(Cache.GameBase + 0x866D50), Utils::GetAdjustedAim, reinterpret_cast<PVOID*>(&Cache.OriginalGetAdjustedAim));
    return true;
}

inline void Renderer::Drawing::RenderText(ImVec2 ScreenPosition, ImColor Color, const char* Text, int WidthText)
{
    bool Center = true;
    std::stringstream Stream(Text);
    std::string Line;
    float Y = 0.0f;
    int Index = 0;
    auto FontSize = ImGui::GetFontSize();
    auto Font = ImGui::GetFont();
    while (std::getline(Stream, Line))
    {
        ImVec2 TextSize = ImVec2(0, 0);
        TextSize = ImGui::GetFont()->CalcTextSizeA(FontSize, FLT_MAX, 0.0f, Line.c_str());
        if (Font) TextSize = Font->CalcTextSizeA(FontSize, FLT_MAX, 0.0f, Line.c_str());
        if (Center)
        {
            ImGui::GetBackgroundDrawList()->AddText(Font, FontSize, ImVec2((ScreenPosition.x - TextSize.x / 2.0f) + 1, (ScreenPosition.y + TextSize.y * Index) + 1), ImGui::GetColorU32(ImVec4(0, 0, 0, 255)), Line.c_str());
            ImGui::GetBackgroundDrawList()->AddText(Font, FontSize, ImVec2((ScreenPosition.x - TextSize.x / 2.0f) - 1, (ScreenPosition.y + TextSize.y * Index) - 1), ImGui::GetColorU32(ImVec4(0, 0, 0, 255)), Line.c_str());
            ImGui::GetBackgroundDrawList()->AddText(Font, FontSize, ImVec2(ScreenPosition.x - TextSize.x / 2.0f, ScreenPosition.y + TextSize.y * Index), Color, Line.c_str());
        }
        else
        {
            ImGui::GetBackgroundDrawList()->AddText(Font, FontSize, ImVec2((ScreenPosition.x) + 1, (ScreenPosition.y + TextSize.y * Index) + 1), ImGui::GetColorU32(ImVec4(0, 0, 0, 255)), Line.c_str());
            ImGui::GetBackgroundDrawList()->AddText(Font, FontSize, ImVec2((ScreenPosition.x) - 1, (ScreenPosition.y + TextSize.y * Index) - 1), ImGui::GetColorU32(ImVec4(0, 0, 0, 255)), Line.c_str());
            ImGui::GetBackgroundDrawList()->AddText(Font, FontSize, ImVec2(ScreenPosition.x, ScreenPosition.y + TextSize.y * Index), Color, Line.c_str());
        }
        Y = ScreenPosition.y + TextSize.y * (Index + 1);
        Index++;
    }
}

inline void Renderer::Drawing::RenderText2(ImVec2 ScreenPosition, ImColor Color, const char* Text, int WidthText)
{
    if (!Text) return;
    auto ImScreen = *reinterpret_cast<const ImVec2*>(&ScreenPosition);
    if (ScreenPosition.x > 0 && ScreenPosition.y > 0 && Cache.WindowSizeX > ScreenPosition.x && Cache.WindowSizeY > ScreenPosition.y)
    {
        auto Size = ImGui::CalcTextSize(Text);
        ImScreen.x -= Size.x * 0.5f;
        ImScreen.y -= Size.y;
        ImGui::GetBackgroundDrawList()->AddText(ImGui::GetFont(), 20, ImScreen, Color, Text, Text + strlen(Text), WidthText);
    }
}

inline void Renderer::Drawing::RenderCollapseFriendlyDisplayList(ImVec2 ScreenStartPosition, ImColor Color, std::vector<std::string> DisplayArray, float HeightFactor)
{
    auto StartPosition = *reinterpret_cast<const ImVec2*>(&ScreenStartPosition);
    for (size_t s = 0; s < DisplayArray.size(); s++)
    {
        auto& DisplayString = DisplayArray[s];
        if (DisplayString.length() < 1) continue;
        auto CurrentPosition = ImVec2(StartPosition.x, StartPosition.y);
        auto StringSize = ImGui::CalcTextSize(DisplayString.c_str());
        CurrentPosition.y += StringSize.y;
        ImGui::GetBackgroundDrawList()->AddText(ImGui::GetFont(), ImGui::GetFontSize(), ImVec2(CurrentPosition.x, StartPosition.y), Color, DisplayString.c_str(), DisplayString.c_str() + strlen(DisplayString.c_str()), 500);
        StartPosition.y += HeightFactor;
    }
}

inline void Renderer::Drawing::RenderCrosshair(ImColor Color, int Thickness)
{
    auto DrawList = ImGui::GetBackgroundDrawList();
    auto WinSizeX = ImGui::GetWindowSize().x;
    auto WinSizeY = ImGui::GetWindowSize().y;
    DrawList->AddLine(ImVec2(WinSizeX / 2, WinSizeY / 2), ImVec2(WinSizeX / 2, WinSizeY / 2 - Settings.Visuals.CrosshairSize), Color, Thickness);
    DrawList->AddLine(ImVec2(WinSizeX / 2, WinSizeY / 2), ImVec2(WinSizeX / 2, WinSizeY / 2 + Settings.Visuals.CrosshairSize), Color, Thickness);
    DrawList->AddLine(ImVec2(WinSizeX / 2, WinSizeY / 2), ImVec2(WinSizeX / 2 - Settings.Visuals.CrosshairSize, WinSizeY / 2), Color, Thickness);
    DrawList->AddLine(ImVec2(WinSizeX / 2, WinSizeY / 2), ImVec2(WinSizeX / 2 + Settings.Visuals.CrosshairSize, WinSizeY / 2), Color, Thickness);
}

void Utils::PE_HOOK(void* obj, void* fn, void* params)
{
    if (GetAsyncKeyState(VK_DELETE) & 0x0001) Settings.IsMenuOpen = !Settings.IsMenuOpen, Renderer::RemoveInput();
    return Settings.OriginalPE(obj, fn, params);
}

FVector* Utils::GetAdjustedAim(AShooterWeapon* Weapon, FVector* Result) {
    if (Cache.AimbotTarget && Settings.Aimbot.SilentAim)
    {
        FVector BoneLocation;
        Settings.GetBoneLocation(Cache.AimbotTarget->MeshComponent, &BoneLocation, Cache.AimbotTarget->MeshComponent->GetBoneName(AimBone), 0);
        FVector AimDirection = Cache.LocalActor->GetDirectionVector(Cache.LPC->PlayerCameraManager->GetCameraLocation(), BoneLocation);
        *Result = AimDirection;
        if (Result->X == 0 || Result->Y == 0 || Result->Z == 0) return Cache.OriginalGetAdjustedAim(Weapon, Result);
        return Result;
    }
    return Cache.OriginalGetAdjustedAim(Weapon, Result);
}

inline void Renderer::Drawing::RenderAimFOV(ImColor Color)
{
    ImGui::GetBackgroundDrawList()->AddCircle(ImVec2(Cache.WindowSizeX / 2, Cache.WindowSizeY / 2), Settings.Visuals.FOVSize, Color, 100, 1.0f);
}

inline int Renderer::Drawing::ReturnDistance(int X1, int Y1, int X2, int Y2) { return sqrt(pow(X2 - X1, 2) + pow(Y2 - Y1, 2) * 1); }

inline bool Renderer::Drawing::WithinAimFOV(int CircleX, int CircleY, int R, int X, int Y)
{
    int Dist = (X - CircleX) * (X - CircleX) + (Y - CircleY) * (Y - CircleY);
    if (Dist <= R * R) return true;
    else return false;
}

inline bool Renderer::Drawing::RenderPlayerSkeleton(USkeletalMeshComponent* Mesh, int Gender, ImColor Color)
{
    FVector BoneWorldLocation;
    // Spine
    FVector2D Head;
    FVector2D Neck;
    FVector2D Spine;
    FVector2D Pelvis;
    // Left Arm
    FVector2D LeftShoulder;
    FVector2D LeftElbow;
    FVector2D LeftWrist;
    // Right Arm
    FVector2D RightShoulder;
    FVector2D RightElbow;
    FVector2D RightWrist;
    // Left Leg
    FVector2D LeftKnee;
    FVector2D LeftAnkle;
    // Right Leg
    FVector2D RightKnee;
    FVector2D RightAnkle;

    // Spine
    Settings.GetBoneLocation(Mesh, &BoneWorldLocation, Mesh->GetBoneName(8), 0);
    if (!W2S(BoneWorldLocation, Head)) return false;
    Settings.GetBoneLocation(Mesh, &BoneWorldLocation, Mesh->GetBoneName(6), 0);
    if (!W2S(BoneWorldLocation, Neck)) return false;
    Settings.GetBoneLocation(Mesh, &BoneWorldLocation, Mesh->GetBoneName(4), 0);
    if (!W2S(BoneWorldLocation, Spine)) return false;
    Settings.GetBoneLocation(Mesh, &BoneWorldLocation, Mesh->GetBoneName(1), 0);
    if (!W2S(BoneWorldLocation, Pelvis)) return false;
    // Left Arm
    Settings.GetBoneLocation(Mesh, &BoneWorldLocation, Mesh->GetBoneName(33), 0);
    if (!W2S(BoneWorldLocation, LeftShoulder)) return false;
    Settings.GetBoneLocation(Mesh, &BoneWorldLocation, Mesh->GetBoneName(36), 0);
    if (!W2S(BoneWorldLocation, LeftElbow)) return false;
    Settings.GetBoneLocation(Mesh, &BoneWorldLocation, Mesh->GetBoneName(38), 0);
    if (!W2S(BoneWorldLocation, LeftWrist)) return false;
    // Right Arm
    if (Gender == 1) { Settings.GetBoneLocation(Mesh, &BoneWorldLocation, Mesh->GetBoneName(57), 0); } // Male
    else if (Gender == 2) { Settings.GetBoneLocation(Mesh, &BoneWorldLocation, Mesh->GetBoneName(59), 0); } // Female
    if (!W2S(BoneWorldLocation, RightShoulder)) return false;
    if (Gender == 1) { Settings.GetBoneLocation(Mesh, &BoneWorldLocation, Mesh->GetBoneName(60), 0); }
    else if (Gender == 2) { Settings.GetBoneLocation(Mesh, &BoneWorldLocation, Mesh->GetBoneName(62), 0); }
    if (!W2S(BoneWorldLocation, RightElbow)) return false;
    if (Gender == 1) { Settings.GetBoneLocation(Mesh, &BoneWorldLocation, Mesh->GetBoneName(62), 0); }
    else if (Gender == 2) { Settings.GetBoneLocation(Mesh, &BoneWorldLocation, Mesh->GetBoneName(64), 0); }
    if (!W2S(BoneWorldLocation, RightWrist)) return false;
    // Left Leg
    if (Gender == 1) { Settings.GetBoneLocation(Mesh, &BoneWorldLocation, Mesh->GetBoneName(82), 0); } // Male
    else if (Gender == 2) { Settings.GetBoneLocation(Mesh, &BoneWorldLocation, Mesh->GetBoneName(84), 0); } // Female
    if (!W2S(BoneWorldLocation, LeftKnee)) return false;
    if (Gender == 1) { Settings.GetBoneLocation(Mesh, &BoneWorldLocation, Mesh->GetBoneName(84), 0); } // Male
    else if (Gender == 2) { Settings.GetBoneLocation(Mesh, &BoneWorldLocation, Mesh->GetBoneName(86), 0); } // Female
    if (!W2S(BoneWorldLocation, LeftAnkle)) return false;
    // Right Leg
    if (Gender == 1) { Settings.GetBoneLocation(Mesh, &BoneWorldLocation, Mesh->GetBoneName(88), 0); } // Male
    else if (Gender == 2) { Settings.GetBoneLocation(Mesh, &BoneWorldLocation, Mesh->GetBoneName(90), 0); } // Female
    if (!W2S(BoneWorldLocation, RightKnee)) return false;
    if (Gender == 1) { Settings.GetBoneLocation(Mesh, &BoneWorldLocation, Mesh->GetBoneName(90), 0); } // Male
    else if (Gender == 2) { Settings.GetBoneLocation(Mesh, &BoneWorldLocation, Mesh->GetBoneName(92), 0); } // Female
    if (!W2S(BoneWorldLocation, RightAnkle)) return false;

    auto DrawList = ImGui::GetBackgroundDrawList();
    DrawList->AddLine(ImVec2(LeftAnkle.X, LeftAnkle.Y), ImVec2(LeftKnee.X, LeftKnee.Y), Color);
    DrawList->AddLine(ImVec2(RightAnkle.X, RightAnkle.Y), ImVec2(RightKnee.X, RightKnee.Y), Color);
    DrawList->AddLine(ImVec2(LeftKnee.X, LeftKnee.Y), ImVec2(Pelvis.X, Pelvis.Y), Color);
    DrawList->AddLine(ImVec2(RightKnee.X, RightKnee.Y), ImVec2(Pelvis.X, Pelvis.Y), Color);
    DrawList->AddLine(ImVec2(Pelvis.X, Pelvis.Y), ImVec2(Spine.X, Spine.Y), Color);
    DrawList->AddLine(ImVec2(Spine.X, Spine.Y), ImVec2(Neck.X, Neck.Y), Color);
    DrawList->AddLine(ImVec2(Neck.X, Neck.Y), ImVec2(Head.X, Head.Y), Color);
    DrawList->AddLine(ImVec2(LeftWrist.X, LeftWrist.Y), ImVec2(LeftElbow.X, LeftElbow.Y), Color);
    DrawList->AddLine(ImVec2(LeftElbow.X, LeftElbow.Y), ImVec2(LeftShoulder.X, LeftShoulder.Y), Color);
    DrawList->AddLine(ImVec2(LeftShoulder.X, LeftShoulder.Y), ImVec2(Neck.X, Neck.Y), Color);
    DrawList->AddLine(ImVec2(RightWrist.X, RightWrist.Y), ImVec2(RightElbow.X, RightElbow.Y), Color);
    DrawList->AddLine(ImVec2(RightElbow.X, RightElbow.Y), ImVec2(RightShoulder.X, RightShoulder.Y), Color);
    DrawList->AddLine(ImVec2(RightShoulder.X, RightShoulder.Y), ImVec2(Neck.X, Neck.Y), Color);
    return true;
}

struct COMDeleter {
    template <typename T>
    void operator()(T* ptr) const {
        if (ptr) ptr->Release();
    }
};

template <typename T>
using COMPtr = std::unique_ptr<T, COMDeleter>;

void Renderer::setupImGui() {
    IMGUI_CHECKVERSION();
    auto Window = FindWindowA("Windows.UI.Core.CoreWindow", "ARK: Survival Evolved");
    if (!Window) {
        HWND ParentWindow = FindWindowA("ApplicationFrameWindow", "ARK: Survival Evolved");
        HWND ChildWindow = FindWindowExA(ParentWindow, NULL, "Windows.UI.Core.CoreWindow", "ARK: Survival Evolved");
        D3D.GameWindow = ChildWindow;
    }
    else {
        D3D.GameWindow = Window;
    }
    ImGui::CreateContext();
    ImGuiIO& IO = ImGui::GetIO();
    ImFontConfig Config;
    ImGuiStyle* Style = &ImGui::GetStyle();
    bool show_demo_window = true, loader_window = false;
    bool show_another_window = false;
    ImGui::StyleColorsDark();
    setupImGuiStyle(Style, IO, Config);
}

void Renderer::setupImGuiStyle(ImGuiStyle* Style, ImGuiIO& IO, ImFontConfig& Config) {
    Style->Alpha = 1.f;
    Style->WindowRounding = 12.f;
    Style->FramePadding = ImVec2(4, 3);
    Style->WindowPadding = ImVec2(8, 8);
    Style->ItemInnerSpacing = ImVec2(4, 4);
    Style->ItemSpacing = ImVec2(8, 5);
    Style->FrameRounding = 4.f;
    Style->ScrollbarSize = 2.f;
    Style->ScrollbarRounding = 12.f;
    Style->PopupRounding = 5.f;
    ImVec4* colors = ImGui::GetStyle().Colors;
    setColorSettings(colors);
    Config.OversampleH = 1;
    Config.OversampleV = 1;
    Config.PixelSnapH = 1;
    static const ImWchar ranges[] =
    {
        0x0020, 0x00FF,
        0x0400, 0x044F,
        0,
    };
    zzzz = IO.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\arialbd.ttf", 13.0f, &Config);
    icons = IO.Fonts->AddFontFromMemoryTTF((void*)iconfont, sizeof(iconfont), 50.f, &Config);
    Config.GlyphRanges = IO.Fonts->GetGlyphRangesCyrillic();
    Config.RasterizerMultiply = 1.125f;
    IO.IniFilename = nullptr;
}

void Renderer::setColorSettings(ImVec4* colors) {
    colors[ImGuiCol_ChildBg] = ImColor(24, 29, 59, 0);
    colors[ImGuiCol_Border] = ImVec4(255, 255, 255, 0);
    colors[ImGuiCol_FrameBg] = ImColor(25, 25, 33, 255);
    colors[ImGuiCol_FrameBgActive] = ImColor(25, 25, 33, 255);
    colors[ImGuiCol_FrameBgHovered] = ImColor(25, 25, 33, 255);
    colors[ImGuiCol_Header] = ImColor(25, 25, 33, 255);
    colors[ImGuiCol_HeaderActive] = ImColor(25, 25, 33, 255);
    colors[ImGuiCol_HeaderHovered] = ImColor(25, 25, 33, 255);
    colors[ImGuiCol_PopupBg] = ImColor(162, 112, 191, 255);
    colors[ImGuiCol_Button] = ImColor(162, 112, 191, 255);
    colors[ImGuiCol_ButtonHovered] = ImColor(155, 90, 191, 255);
    colors[ImGuiCol_ButtonActive] = ImColor(152, 75, 191, 255);
    colors[ImGuiCol_TitleBgActive] = ImColor(24, 29, 59, 0);
    colors[ImGuiCol_WindowBg] = ImColor(24, 29, 59, 0);
    colors[ImGuiCol_Border] = ImColor(24, 29, 59, 0);
}

void Renderer::renderFrame() {
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
    ImGui::Begin("##ESP", nullptr, ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));
    auto& io = ImGui::GetIO();
    Cache.WindowSizeX = io.DisplaySize.x;
    Cache.WindowSizeY = io.DisplaySize.y;
    ImGui::SetWindowSize(ImVec2(io.DisplaySize.x, io.DisplaySize.y));

    if (Settings.IsMenuOpen)
    {
        Renderer::HookInput();

        ImGui::Begin("[ArkMenu]", &Settings.IsMenuOpen, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration);
        ImGui::SetWindowSize(ImVec2(Settings.MenuSizeX, Settings.MenuSizeY));

        decorations();
        tabss();

        ImGui::End();
    }

    ImGui::PopStyleColor();
    ImGui::PopStyleVar(2);
    ImGui::End();
}

HRESULT Renderer::D3D_HOOK(IDXGISwapChain* SwapChain, UINT SyncInterval, UINT Flags) {
    COMPtr<ID3D11Texture2D> Surface;

    if (!D3D.Device) {
        if (FAILED(SwapChain->GetBuffer(0, _uuidof(ID3D11Texture2D), reinterpret_cast<PVOID*>(&Surface)))) {
            Logger::Log("Failed to get buffer from SwapChain");
            return D3D.PresentFunc(SwapChain, SyncInterval, Flags);
        }

        Logger::Log("[ID3D11Texture2D]: %p\n", Surface.get());

        if (FAILED(SwapChain->GetDevice(__uuidof(ID3D11Device), reinterpret_cast<PVOID*>(&D3D.Device)))) {
            Logger::Log("Failed to get device from SwapChain");
            return D3D.PresentFunc(SwapChain, SyncInterval, Flags);
        }

        Logger::Log("[ID3D11Device]: %p\n", D3D.Device);

        if (FAILED(D3D.Device->CreateRenderTargetView(Surface.get(), nullptr, &D3D.RenderTargetView))) {
            Logger::Log("Failed to create render target view");
            return D3D.PresentFunc(SwapChain, SyncInterval, Flags);
        }

        Logger::Log("[ID3D11RenderTargetView]: %p\n", D3D.RenderTargetView);
        Surface.reset();
        D3D.Device->GetImmediateContext(&D3D.Ctx);
        Logger::Log("[ID3D11DeviceContext]: %p\n", D3D.Ctx);

        setupImGui();

        if (!ImGui_ImplDX11_Init(D3D.Device, D3D.Ctx)) {
            Logger::Log("Failed to initialize ImGui_DX11");
            return D3D.PresentFunc(SwapChain, SyncInterval, Flags);
        }

        if (!ImGui_ImplDX11_CreateDeviceObjects()) {
            Logger::Log("Failed to create ImGui DX11 device objects");
            return D3D.PresentFunc(SwapChain, SyncInterval, Flags);
        }

        Logger::Log("[IMGUI]: Initialized Successfully!\n");
    }

    renderFrame();

    D3D.Ctx->OMSetRenderTargets(1, &D3D.RenderTargetView, nullptr);
    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    return D3D.OriginalPresent(SwapChain, SyncInterval, Flags);
}

std::wstring RetrieveUWPFolder(const wchar_t* fileName)
{
    wchar_t szPath[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, szPath)))
    {
        std::wstringstream data;
        data << szPath << L"\\" << fileName;
        return data.str();
    }
    else
    {
        throw std::runtime_error("Could not retrieve folder");
    }
}

std::wstring InsertDoubleSlashes(std::wstring CurrentPath)
{
    std::wstringstream Stream;
    for (const auto& ch : CurrentPath)
    {
        if (ch == '\\') Stream << L"\\\\";
        else Stream << ch;
    }
    return Stream.str();
}

void InitCheat()
{
    try
    {
        std::wstring loggerFile = InsertDoubleSlashes(RetrieveUWPFolder(L"logger.gc"));

        uintptr_t GameBase = reinterpret_cast<uintptr_t>(GetModuleHandle(nullptr));
        UWorld::GWorld = *reinterpret_cast<decltype(UWorld::GWorld)*>(GameBase + Settings.uworld);
        Cache.GameBase = GameBase;

        Settings.OriginalGetBoneLocation = reinterpret_cast<decltype(Settings.OriginalGetBoneLocation)>(PatternScan(GameBase, "40 57 48 83 EC 70 48 C7 44 24 ? ? ? ? ? 48 89 9C 24 ? ? ? ? 48 8B DA 48 8B F9 49 8B D0 E8 ? ? ? ? 83 F8 FF"));
        Settings.GetBoneLocation = reinterpret_cast<decltype(Settings.GetBoneLocation)>(Settings.OriginalGetBoneLocation);

        Settings.OriginalInputKey = reinterpret_cast<decltype(Settings.OriginalInputKey)>(PatternScan(GameBase, "48 8B C4 48 89 50 10 56 57 41 56 48 81 EC ? ? ? ? 48 C7 44 24 ? ? ? ? ? 48 89 58 18 48 89 68 20 0F 29 70 D8"));
        Settings.InputKey = reinterpret_cast<decltype(Settings.InputKey)>(Settings.OriginalInputKey);

        if (!Logger::Init(loggerFile.c_str()))
            throw std::runtime_error("[LOGGER]: Failed to initialize logger!");

        if (!Renderer::Init())
            throw std::runtime_error("[RENDERER]: Renderer Failed To Initialize!");

        if (!InitSDK())
            throw std::runtime_error("[SDK]: SDK Failed To Initialize!");

        Logger::Log("[GObjects]: %p\n", UObject::GObjects);
        Logger::Log("[GNames]: %p\n", FName::GNames);
        Logger::Log("[UWorld]: %p\n", UWorld::GWorld);
        Logger::Log("[PersistentLevel]: %p\n", UWorld::GWorld->PersistentLevel);
    }
    catch (const std::exception& ex)
    {
        Logger::Log(ex.what());
    }
}

void AimbotThread() 
{
    do
    {
        try
        {
            UWorld* GWorld = UWorld::GWorld;
            void* GameState = GWorld->GameState;
            UPlayer* LocalPlayer = GWorld->OwningGameInstance->LocalPlayers[0];
            auto PC = LocalPlayer->PlayerController;
            FVector2D ActorScreenLocation;
            auto WinSizeX = ImGui::GetWindowSize().x;
            auto WinSizeY = ImGui::GetWindowSize().y;
            FVector BoneWorldLocation;
            FVector2D BoneScreenLocation;
            int LastDistance = Cache.WindowSizeX;
            bool AimKeyDown = Cache.LPC->IsInputKeyDown(Settings.Keybinds.AimbotKey);
            if (!AimKeyDown) { Cache.LPC->IgnoreLookInput(false), Settings.Aimbot.AimLocked = false, Cache.AimbotTarget = nullptr; }


            if (!Settings.Aimbot.AimLocked)
            {
                auto Actors = UWorld::GWorld->PersistentLevel->Actors;
                for (int i = 0; i < Actors.Count; i++)
                {
                    auto Actor = Actors[i];
                    if (Actor && Actor->IsPlayer())
                    {
                        Settings.GetBoneLocation(Actor->MeshComponent, &BoneWorldLocation, Actor->MeshComponent->GetBoneName(AimBone), 0);
                        if (!W2S(BoneWorldLocation, BoneScreenLocation) || !Renderer::Drawing::WithinAimFOV(Cache.WindowSizeX / 2, Cache.WindowSizeY / 2, Settings.Visuals.FOVSize, BoneScreenLocation.X, BoneScreenLocation.Y)) continue;
                        if (Actor->IsDead() || Actor->IsLocalPlayer()) continue;
                        if (Settings.Aimbot.TargetTribe)
                        {
                            if (Actor->TribeName.IsValid() && Actor->TribeName.ToString() == Cache.LocalActor->TribeName.ToString()) continue;
                        }
                        if (!Settings.Aimbot.TargetSleepers && !Actor->IsConscious()) continue;
                        if (Cache.AimbotTarget) {
                            FVector TheirVelocity = Cache.AimbotTarget->CharacterMovement->Velocity;
                            FVector MyVelocity = Cache.LocalActor->CharacterMovement->Velocity;
                            if (TheirVelocity != MyVelocity)
                            {
                                FVector TheSpeed = MyVelocity - TheirVelocity;
                                BoneWorldLocation += BoneWorldLocation + (TheSpeed / TheirVelocity);
                            }
                        }
                        if (Settings.Aimbot.VisibleOnly && !Cache.LPC->LineOfSightTo(Actor, Cache.LPC->PlayerCameraManager->GetCameraLocation(), false)) continue;
                        int Distance = Renderer::Drawing::ReturnDistance(Cache.WindowSizeX / 2, Cache.WindowSizeY / 2, BoneScreenLocation.X, BoneScreenLocation.Y);
                        if (Distance < LastDistance) { LastDistance = Distance, Cache.AimbotTarget = Actor; }
                    }
                }
                if (AimKeyDown && Cache.AimbotTarget)
                {
                    Cache.LPC->IgnoreLookInput(true), Settings.Aimbot.AimLocked = true;
                }
            }

            if (Settings.Aimbot.AimLocked && Cache.AimbotTarget)
            {
                auto Target = Cache.AimbotTarget;
                FRotator ControlRotation = Cache.LPC->PlayerCameraManager->GetCameraRotation();
                FVector CameraLoc = Cache.LPC->PlayerCameraManager->GetCameraLocation();
                if (Target)
                {
                    if (Target->IsDead()) { Cache.LPC->IgnoreLookInput(false), Settings.Aimbot.AimLocked = false, Cache.AimbotTarget = nullptr; }
                    if (Target->MeshComponent)
                    {
                        Settings.GetBoneLocation(Target->MeshComponent, &BoneWorldLocation, Target->MeshComponent->GetBoneName(AimBone), 0);
                        auto Rotator = Cache.LocalPlayer->FindLookAtRotation(Cache.LPC->PlayerCameraManager->GetCameraLocation(), BoneWorldLocation);
                        if (Rotator.hasValue() && Rotator.Yaw != 0 && Rotator.Pitch != 0)
                        {
                            Rotator.Roll = 0;
                            Cache.LPC->ControlRotation = Rotator;
                        }
                    }
                }
            }
        }
        catch (const std::exception&)
        {
                
        }
    } while (false);
} 