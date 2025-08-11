# ArkMenu - ARK Survival Evolved Menu DLL

A comprehensive cheat/hack menu for ARK Survival Evolved Microsoft Store version with ESP, Aimbot, and various game modifications.

## Features

### Visuals (ESP)
- **Player ESP**: Display player information, health, distance, and names
- **Dinosaur ESP**: Show wild and tamed creatures with health and level info
- **Turret ESP**: Display turret locations and bullet counts
- **Container ESP**: Show storage containers and their contents
- **Skeleton ESP**: Render player and dino skeletons
- **Crosshair**: Customizable crosshair with FOV circle
- **Color Customization**: Fully customizable colors for all ESP elements

### Aimbot
- **Silent Aim**: Aim without visible movement
- **Target Filtering**: Target specific players or tribes
- **FOV Limiting**: Only target players within FOV circle
- **Visibility Checks**: Only target visible players
- **Sleeper Targeting**: Option to target sleeping players

### Miscellaneous
- **Speed Hacks**: Increase movement speed
- **No Recoil**: Remove weapon recoil and spread
- **No Camera Shake**: Remove weapon firing camera shake
- **Rapid Fire**: Increase weapon fire rate
- **Infinite Orbit**: Extended camera zoom
- **Long Arms**: Extended interaction range
- **Auto Armor**: Automatically equip best armor
- **Instant Dino Turn**: Faster dino rotation
- **Extra Information**: Display FPS, player count, coordinates

## Building

### Prerequisites
- Visual Studio 2019 or later
- CMake 3.16 or later
- Windows 10/11
- ARK Survival Evolved (Microsoft Store version)

### Dependencies
This project requires the following external libraries:
- **ImGui**: Download from https://github.com/ocornut/imgui
- **MinHook**: Download from https://github.com/TsudaKageyu/minhook

### Build Steps

1. **Clone the repository**:
   ```bash
   git clone https://github.com/yourusername/ArkMenu.git
   cd ArkMenu
   ```

2. **Download dependencies**:
   - Download ImGui and place files in `external/imgui/`
   - Download MinHook and place files in `external/minhook/`

3. **Build the project**:
   ```bash
   mkdir build
   cd build
   cmake ..
   cmake --build . --config Release
   ```

4. **Install the DLL**:
   - Copy `ArkMenu.dll` from `build/bin/Release/` to your ARK installation directory
   - For Microsoft Store version: `C:\Program Files\WindowsApps\Microsoft.FlightSimulator_*\ARK\ShooterGame\Binaries\WinGDK\`

## Usage

### Injection
1. Start ARK Survival Evolved
2. Inject the DLL using your preferred injector
3. Press `Delete` to open/close the menu
4. Configure settings in the menu

### Controls
- **Delete**: Toggle menu visibility
- **Right Mouse Button**: Aimbot key (configurable)
- **Mouse**: Navigate menu and adjust settings

### Menu Navigation
- **Visuals Tab**: Configure ESP settings and colors
- **Aimbot Tab**: Configure aimbot settings
- **Misc Tab**: Configure miscellaneous features
- **Colors Tab**: Customize ESP colors

## Configuration

### Visual Settings
- Enable/disable different ESP features
- Adjust crosshair size and FOV circle
- Customize colors for different entity types
- Set distance limits and filters

### Aimbot Settings
- Enable/disable aimbot
- Configure target filters
- Set FOV limits
- Adjust aim settings

### Misc Settings
- Configure speed multipliers
- Set auto armor thresholds
- Enable/disable various hacks
- Display options

## Safety and Anti-Cheat

⚠️ **Warning**: This software is for educational purposes only. Using cheats in online games may result in:
- Account bans
- Server bans
- Loss of progress
- Legal consequences

### Recommendations
- Only use on private servers with permission
- Test on single-player first
- Keep the software updated
- Use at your own risk

## Technical Details

### Architecture
- **Language**: C++17
- **Graphics API**: DirectX 11
- **UI Framework**: ImGui
- **Hooking Library**: MinHook
- **Target**: ARK Survival Evolved (Microsoft Store)

### Key Components
- **Renderer**: DirectX 11 overlay rendering
- **Hooks**: Game function hooking for modifications
- **ESP**: World-to-screen coordinate conversion
- **Aimbot**: Target acquisition and aim assistance
- **Menu**: ImGui-based configuration interface

### Memory Management
- Safe pattern scanning for game offsets
- Proper COM object management
- RAII resource management
- Exception-safe code

## Troubleshooting

### Common Issues

**DLL won't inject**:
- Ensure ARK is running as administrator
- Check if antivirus is blocking the DLL
- Verify DLL architecture matches game (x64)

**Menu doesn't appear**:
- Press Delete key to toggle menu
- Check if DirectX 11 is being used
- Verify ImGui initialization

**ESP not working**:
- Check if ESP features are enabled in menu
- Verify game offsets are correct for your version
- Ensure you're not in a menu or loading screen

**Aimbot not working**:
- Check if aimbot is enabled
- Verify aim key is set correctly
- Ensure targets are within FOV

### Debug Information
- Check the log file in `%LOCALAPPDATA%\logger.gc`
- Enable debug output in Visual Studio
- Monitor for error messages in console

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test thoroughly
5. Submit a pull request

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Disclaimer

This software is provided "as is" without warranty. The authors are not responsible for any damage or consequences resulting from its use. Use at your own risk.

## Support

For support, please:
1. Check the troubleshooting section
2. Search existing issues
3. Create a new issue with detailed information
4. Include log files and error messages

## Changelog

### Version 1.0
- Initial release
- Basic ESP functionality
- Aimbot implementation
- Menu system
- Various game modifications 