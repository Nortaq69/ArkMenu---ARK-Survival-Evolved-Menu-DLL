#pragma once
#include "Menu.h"
#include <vector>

struct ApocalypseCheatToggles {
    bool FlashStep = false;
    bool BunnyHop = false;
    bool WaterWalker = false;
    bool WallRun = false;
    bool GhostVault = false;
    bool SpoilFreeze = false;
    bool AutoHarvest = false;
    bool BuildThroughWalls = false;
    bool TetherBreaker = false;
    bool StackMultiplier = false;
};

extern ApocalypseCheatToggles CheatToggles;

namespace ApocalypseCheats {
    // 1. Flash Step
    void FlashStep();
    // 2. Bunny Hop Multiplier
    void BunnyHopTick();
    // 3. Water Walker
    void WaterWalkerTick();
    // 4. Wall Run Mode
    void WallRunTick();
    // 5. Ghost Vaulting
    void GhostVault();
    // 6. Spoil Time Freezer
    void SpoilTimeFreezerTick();
    // 7. Auto-Harvest Bot
    void AutoHarvestTick();
    // 8. Build Through Walls
    void EnableBuildThroughWalls();
    void DisableBuildThroughWalls();
    // 9. Tether Breaker
    void TetherBreakerTick();
    // 10. Stack Multiplier Toggle
    void EnableStackMultiplier();
    void DisableStackMultiplier();
} 