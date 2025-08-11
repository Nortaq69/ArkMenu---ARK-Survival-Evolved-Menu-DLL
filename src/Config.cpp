#include "Menu.h"
#include <fstream>
#include <json/json.h>

class Config {
private:
    std::string configPath;
    
public:
    Config() {
        char appData[MAX_PATH];
        SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, 0, appData);
        configPath = std::string(appData) + "\\ArkMenu\\config.json";
        
        // Create directory if it doesn't exist
        std::string dir = std::string(appData) + "\\ArkMenu";
        CreateDirectoryA(dir.c_str(), NULL);
    }
    
    void Save() {
        try {
            Json::Value root;
            
            // Visuals
            Json::Value visuals;
            visuals["DrawPlayers"] = Settings.Visuals.DrawPlayers;
            visuals["DrawPlayerDistance"] = Settings.Visuals.DrawPlayerDistance;
            visuals["DrawPlayerHP"] = Settings.Visuals.DrawPlayerHP;
            visuals["DrawPlayerBones"] = Settings.Visuals.DrawPlayerBones;
            visuals["DrawWildCreatures"] = Settings.Visuals.DrawWildCreatures;
            visuals["DrawTamedCreatures"] = Settings.Visuals.DrawTamedCreatures;
            visuals["DrawCrosshair"] = Settings.Visuals.DrawCrosshair;
            visuals["DrawAimFOV"] = Settings.Visuals.DrawAimFOV;
            visuals["CrosshairSize"] = Settings.Visuals.CrosshairSize;
            visuals["FOVSize"] = Settings.Visuals.FOVSize;
            root["Visuals"] = visuals;
            
            // Aimbot
            Json::Value aimbot;
            aimbot["EnableAimbot"] = Settings.Aimbot.EnableAimbot;
            aimbot["SilentAim"] = Settings.Aimbot.SilentAim;
            aimbot["TargetTribe"] = Settings.Aimbot.TargetTribe;
            aimbot["TargetSleepers"] = Settings.Aimbot.TargetSleepers;
            aimbot["VisibleOnly"] = Settings.Aimbot.VisibleOnly;
            root["Aimbot"] = aimbot;
            
            // Misc
            Json::Value misc;
            misc["ExtraInfo"] = Settings.Misc.ExtraInfo;
            misc["ShowPlayers"] = Settings.Misc.ShowPlayers;
            misc["ShowFPS"] = Settings.Misc.ShowFPS;
            misc["ShowXYZ"] = Settings.Misc.ShowXYZ;
            misc["NoSway"] = Settings.Misc.NoSway;
            misc["NoSpread"] = Settings.Misc.NoSpread;
            misc["NoShake"] = Settings.Misc.NoShake;
            misc["SpeedHacks"] = Settings.Misc.SpeedHacks;
            misc["RapidFire"] = Settings.Misc.RapidFire;
            misc["InfiniteOrbit"] = Settings.Misc.InfiniteOrbit;
            misc["LongArms"] = Settings.Misc.LongArms;
            misc["AutoArmor"] = Settings.Misc.AutoArmor;
            misc["NewSpeed"] = Settings.Misc.NewSpeed;
            misc["AutoArmorPercent"] = Settings.Misc.AutoArmorPercent;
            root["Misc"] = misc;
            
            // Colors
            Json::Value colors;
            for (int i = 0; i < 3; i++) {
                colors["PlayerColor"][i] = Settings.PlayerColor1[i];
                colors["TamedDinoColor"][i] = Settings.TamedDinoColor1[i];
                colors["WildDinoColor"][i] = Settings.WildDinoColor1[i];
                colors["TurretColor"][i] = Settings.TurretColor1[i];
                colors["ContainerColor"][i] = Settings.ContainerColor1[i];
                colors["CrosshairColor"][i] = Settings.CrosshairColor1[i];
            }
            root["Colors"] = colors;
            
            // Menu
            Json::Value menu;
            menu["MenuSizeX"] = Settings.MenuSizeX;
            menu["MenuSizeY"] = Settings.MenuSizeY;
            root["Menu"] = menu;
            
            // Write to file
            std::ofstream file(configPath);
            Json::StyledWriter writer;
            file << writer.write(root);
            file.close();
            
            Logger::Log("[CONFIG]: Settings saved successfully\n");
        }
        catch (const std::exception& e) {
            Logger::Log("[CONFIG]: Failed to save settings: %s\n", e.what());
        }
    }
    
    void Load() {
        try {
            std::ifstream file(configPath);
            if (!file.is_open()) {
                Logger::Log("[CONFIG]: No config file found, using defaults\n");
                return;
            }
            
            Json::Value root;
            Json::Reader reader;
            if (!reader.parse(file, root)) {
                Logger::Log("[CONFIG]: Failed to parse config file\n");
                return;
            }
            
            // Visuals
            if (root.isMember("Visuals")) {
                Json::Value visuals = root["Visuals"];
                Settings.Visuals.DrawPlayers = visuals.get("DrawPlayers", true).asBool();
                Settings.Visuals.DrawPlayerDistance = visuals.get("DrawPlayerDistance", true).asBool();
                Settings.Visuals.DrawPlayerHP = visuals.get("DrawPlayerHP", true).asBool();
                Settings.Visuals.DrawPlayerBones = visuals.get("DrawPlayerBones", false).asBool();
                Settings.Visuals.DrawWildCreatures = visuals.get("DrawWildCreatures", true).asBool();
                Settings.Visuals.DrawTamedCreatures = visuals.get("DrawTamedCreatures", true).asBool();
                Settings.Visuals.DrawCrosshair = visuals.get("DrawCrosshair", true).asBool();
                Settings.Visuals.DrawAimFOV = visuals.get("DrawAimFOV", true).asBool();
                Settings.Visuals.CrosshairSize = visuals.get("CrosshairSize", 10.0f).asFloat();
                Settings.Visuals.FOVSize = visuals.get("FOVSize", 100.0f).asFloat();
            }
            
            // Aimbot
            if (root.isMember("Aimbot")) {
                Json::Value aimbot = root["Aimbot"];
                Settings.Aimbot.EnableAimbot = aimbot.get("EnableAimbot", false).asBool();
                Settings.Aimbot.SilentAim = aimbot.get("SilentAim", false).asBool();
                Settings.Aimbot.TargetTribe = aimbot.get("TargetTribe", false).asBool();
                Settings.Aimbot.TargetSleepers = aimbot.get("TargetSleepers", false).asBool();
                Settings.Aimbot.VisibleOnly = aimbot.get("VisibleOnly", false).asBool();
            }
            
            // Misc
            if (root.isMember("Misc")) {
                Json::Value misc = root["Misc"];
                Settings.Misc.ExtraInfo = misc.get("ExtraInfo", true).asBool();
                Settings.Misc.ShowPlayers = misc.get("ShowPlayers", true).asBool();
                Settings.Misc.ShowFPS = misc.get("ShowFPS", true).asBool();
                Settings.Misc.ShowXYZ = misc.get("ShowXYZ", true).asBool();
                Settings.Misc.NoSway = misc.get("NoSway", false).asBool();
                Settings.Misc.NoSpread = misc.get("NoSpread", false).asBool();
                Settings.Misc.NoShake = misc.get("NoShake", false).asBool();
                Settings.Misc.SpeedHacks = misc.get("SpeedHacks", false).asBool();
                Settings.Misc.RapidFire = misc.get("RapidFire", false).asBool();
                Settings.Misc.InfiniteOrbit = misc.get("InfiniteOrbit", false).asBool();
                Settings.Misc.LongArms = misc.get("LongArms", false).asBool();
                Settings.Misc.AutoArmor = misc.get("AutoArmor", false).asBool();
                Settings.Misc.NewSpeed = misc.get("NewSpeed", 1000.0f).asFloat();
                Settings.Misc.AutoArmorPercent = misc.get("AutoArmorPercent", 0.3f).asFloat();
            }
            
            // Colors
            if (root.isMember("Colors")) {
                Json::Value colors = root["Colors"];
                for (int i = 0; i < 3; i++) {
                    Settings.PlayerColor1[i] = colors["PlayerColor"].get(i, 1.0f).asFloat();
                    Settings.TamedDinoColor1[i] = colors["TamedDinoColor"].get(i, 0.0f).asFloat();
                    Settings.WildDinoColor1[i] = colors["WildDinoColor"].get(i, 1.0f).asFloat();
                    Settings.TurretColor1[i] = colors["TurretColor"].get(i, 1.0f).asFloat();
                    Settings.ContainerColor1[i] = colors["ContainerColor"].get(i, 0.5f).asFloat();
                    Settings.CrosshairColor1[i] = colors["CrosshairColor"].get(i, 1.0f).asFloat();
                }
            }
            
            // Menu
            if (root.isMember("Menu")) {
                Json::Value menu = root["Menu"];
                Settings.MenuSizeX = menu.get("MenuSizeX", 800.0f).asFloat();
                Settings.MenuSizeY = menu.get("MenuSizeY", 600.0f).asFloat();
            }
            
            file.close();
            Logger::Log("[CONFIG]: Settings loaded successfully\n");
        }
        catch (const std::exception& e) {
            Logger::Log("[CONFIG]: Failed to load settings: %s\n", e.what());
        }
    }
    
    void Reset() {
        // Reset to default values
        Settings.Visuals.DrawPlayers = true;
        Settings.Visuals.DrawPlayerDistance = true;
        Settings.Visuals.DrawPlayerHP = true;
        Settings.Visuals.DrawPlayerBones = false;
        Settings.Visuals.DrawWildCreatures = true;
        Settings.Visuals.DrawTamedCreatures = true;
        Settings.Visuals.DrawCrosshair = true;
        Settings.Visuals.DrawAimFOV = true;
        Settings.Visuals.CrosshairSize = 10.0f;
        Settings.Visuals.FOVSize = 100.0f;
        
        Settings.Aimbot.EnableAimbot = false;
        Settings.Aimbot.SilentAim = false;
        Settings.Aimbot.TargetTribe = false;
        Settings.Aimbot.TargetSleepers = false;
        Settings.Aimbot.VisibleOnly = false;
        
        Settings.Misc.ExtraInfo = true;
        Settings.Misc.ShowPlayers = true;
        Settings.Misc.ShowFPS = true;
        Settings.Misc.ShowXYZ = true;
        Settings.Misc.NoSway = false;
        Settings.Misc.NoSpread = false;
        Settings.Misc.NoShake = false;
        Settings.Misc.SpeedHacks = false;
        Settings.Misc.RapidFire = false;
        Settings.Misc.InfiniteOrbit = false;
        Settings.Misc.LongArms = false;
        Settings.Misc.AutoArmor = false;
        Settings.Misc.NewSpeed = 1000.0f;
        Settings.Misc.AutoArmorPercent = 0.3f;
        
        // Default colors
        Settings.PlayerColor1[0] = 1.0f; Settings.PlayerColor1[1] = 0.0f; Settings.PlayerColor1[2] = 0.0f;
        Settings.TamedDinoColor1[0] = 0.0f; Settings.TamedDinoColor1[1] = 1.0f; Settings.TamedDinoColor1[2] = 0.0f;
        Settings.WildDinoColor1[0] = 1.0f; Settings.WildDinoColor1[1] = 1.0f; Settings.WildDinoColor1[2] = 0.0f;
        Settings.TurretColor1[0] = 1.0f; Settings.TurretColor1[1] = 0.5f; Settings.TurretColor1[2] = 0.0f;
        Settings.ContainerColor1[0] = 0.5f; Settings.ContainerColor1[1] = 0.5f; Settings.ContainerColor1[2] = 1.0f;
        Settings.CrosshairColor1[0] = 1.0f; Settings.CrosshairColor1[1] = 1.0f; Settings.CrosshairColor1[2] = 1.0f;
        
        Settings.MenuSizeX = 800.0f;
        Settings.MenuSizeY = 600.0f;
        
        Logger::Log("[CONFIG]: Settings reset to defaults\n");
    }
};

// Global config instance
static Config g_Config;

// Export functions for use in other files
extern "C" {
    __declspec(dllexport) void SaveConfig() {
        g_Config.Save();
    }
    
    __declspec(dllexport) void LoadConfig() {
        g_Config.Load();
    }
    
    __declspec(dllexport) void ResetConfig() {
        g_Config.Reset();
    }
} 