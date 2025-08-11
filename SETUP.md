# ArkMenu Setup Guide

This guide will help you set up and build the ArkMenu project for ARK Survival Evolved.

## Quick Start

### 1. Prerequisites Installation

**Visual Studio 2019/2022:**
- Download from: https://visualstudio.microsoft.com/
- Install with "Desktop development with C++" workload
- Make sure to include MSVC v142/v143 compiler

**CMake:**
- Download from: https://cmake.org/download/
- Add to PATH during installation

**Git:**
- Download from: https://git-scm.com/
- Add to PATH during installation

### 2. Dependencies Setup

**ImGui:**
```bash
cd external
git clone https://github.com/ocornut/imgui.git
```

**MinHook:**
```bash
cd external
git clone https://github.com/TsudaKageyu/minhook.git
```

**JsonCpp (for config system):**
```bash
cd external
git clone https://github.com/open-source-parsers/jsoncpp.git
```

### 3. Build Process

**Option 1: Using build.bat (Recommended)**
```bash
# Run the automated build script
build.bat
```

**Option 2: Manual CMake build**
```bash
mkdir build
cd build
cmake .. -G "Visual Studio 16 2019" -A x64
cmake --build . --config Release
```

### 4. Build Injector (Optional)

```bash
cd tools
build_injector.bat
```

## Project Structure

```
ArkMenu/
├── include/                 # Header files
│   └── Menu.h              # Main header
├── src/                    # Source files
│   ├── Menu.cpp            # Main implementation
│   ├── cheat.cpp           # Game-specific functions
│   └── Config.cpp          # Configuration system
├── external/               # External libraries
│   ├── imgui/              # ImGui library
│   ├── minhook/            # MinHook library
│   └── jsoncpp/            # JsonCpp library
├── tools/                  # Build tools
│   ├── Injector.cpp        # DLL injector
│   └── build_injector.bat  # Injector build script
├── CMakeLists.txt          # CMake configuration
├── build.bat               # Automated build script
├── README.md               # Project documentation
└── SETUP.md                # This file
```

## Configuration

### Game Offsets

The project uses pattern scanning to find game functions, but you may need to update offsets for different game versions:

```cpp
// In Menu.h, update these offsets if needed:
Settings.uworld = 0x173CA60;  // UWorld offset
```

### Build Configuration

**CMakeLists.txt settings:**
- Target: x64 architecture
- C++17 standard
- Static linking for runtime libraries
- DirectX 11 dependencies

## Troubleshooting

### Common Build Issues

**"CMake not found":**
- Install CMake and add to PATH
- Restart command prompt after installation

**"Visual Studio not found":**
- Install Visual Studio with C++ tools
- Run from Developer Command Prompt

**"Dependencies missing":**
- Ensure all external libraries are cloned
- Check paths in CMakeLists.txt

**"Linker errors":**
- Verify DirectX SDK is installed
- Check library paths and names

### Runtime Issues

**DLL won't inject:**
- Run ARK as Administrator
- Disable antivirus temporarily
- Check DLL architecture (x64)

**Menu doesn't appear:**
- Press Delete key to toggle
- Check DirectX 11 is being used
- Verify ImGui initialization

**ESP not working:**
- Enable ESP features in menu
- Check game offsets are correct
- Ensure you're in-game (not in menu)

## Development

### Adding New Features

1. **Add settings to Menu.h:**
```cpp
struct {
    bool NewFeature = false;
    float NewValue = 1.0f;
} NewCategory;
```

2. **Implement in Menu.cpp:**
```cpp
if (Settings.NewCategory.NewFeature) {
    // Implementation here
}
```

3. **Add to menu UI:**
```cpp
ImGui::Checkbox("New Feature", &Settings.NewCategory.NewFeature);
ImGui::SliderFloat("New Value", &Settings.NewCategory.NewValue, 0.0f, 10.0f);
```

### Code Style

- Use C++17 features
- Follow RAII principles
- Use smart pointers where possible
- Add error handling and logging
- Comment complex game-specific code

## Security Notes

⚠️ **Important Security Considerations:**

1. **Anti-Cheat Detection:**
   - This software may be detected by anti-cheat systems
   - Use only on private servers with permission
   - Test thoroughly before use

2. **Code Obfuscation:**
   - Consider obfuscating strings and function names
   - Use encryption for sensitive data
   - Implement anti-debugging measures

3. **Update Strategy:**
   - Game updates may break functionality
   - Keep offsets and patterns updated
   - Test after each game patch

## Support

For issues and questions:

1. Check this setup guide
2. Review the README.md
3. Check troubleshooting section
4. Create an issue with detailed information

## Legal Disclaimer

This software is for educational purposes only. Users are responsible for compliance with local laws and game terms of service. The authors are not responsible for any consequences resulting from use of this software. 