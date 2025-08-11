#include "Menu.h"
#include <Psapi.h>
#include <TlHelp32.h>

// Global variables
UObject** UObject::GObjects = nullptr;
FName* FName::GNames = nullptr;
UWorld* UWorld::GWorld = nullptr;

// Logger implementation
std::ofstream Logger::LogFile;

bool Logger::Init(const wchar_t* FilePath) {
    LogFile.open(FilePath, std::ios::app);
    return LogFile.is_open();
}

void Logger::Log(const char* Format, ...) {
    if (!LogFile.is_open()) return;
    
    char Buffer[4096];
    va_list Args;
    va_start(Args, Format);
    vsprintf_s(Buffer, Format, Args);
    va_end(Args);
    
    LogFile << Buffer << std::flush;
}

void Logger::Close() {
    if (LogFile.is_open()) {
        LogFile.close();
    }
}

// Pattern scanning function
void* PatternScan(uintptr_t Base, const char* Pattern) {
    MODULEINFO ModuleInfo;
    if (!GetModuleInformation(GetCurrentProcess(), (HMODULE)Base, &ModuleInfo, sizeof(ModuleInfo))) {
        return nullptr;
    }
    
    uintptr_t Start = Base;
    uintptr_t End = Base + ModuleInfo.SizeOfImage;
    
    std::vector<int> Bytes;
    std::string PatternStr(Pattern);
    std::stringstream SS(PatternStr);
    std::string Byte;
    
    while (std::getline(SS, Byte, ' ')) {
        if (Byte == "?") {
            Bytes.push_back(-1);
        } else {
            Bytes.push_back(std::stoi(Byte, nullptr, 16));
        }
    }
    
    for (uintptr_t i = Start; i < End - Bytes.size(); i++) {
        bool Found = true;
        for (size_t j = 0; j < Bytes.size(); j++) {
            if (Bytes[j] != -1 && *(uint8_t*)(i + j) != Bytes[j]) {
                Found = false;
                break;
            }
        }
        if (Found) {
            return (void*)i;
        }
    }
    
    return nullptr;
}

// D3D11 Present function finder
void* GetD3D11PresentFunction() {
    HMODULE D3D11Module = GetModuleHandleA("d3d11.dll");
    if (!D3D11Module) return nullptr;
    
    return GetProcAddress(D3D11Module, "Present");
}

// Hook functions
bool SetHook(void* Target, void* Detour, void** Original) {
    if (MH_Initialize() != MH_OK) return false;
    if (MH_CreateHook(Target, Detour, Original) != MH_OK) return false;
    if (MH_EnableHook(Target) != MH_OK) return false;
    return true;
}

bool RemoveHook(void* Original) {
    if (MH_DisableHook(Original) != MH_OK) return false;
    if (MH_RemoveHook(Original) != MH_OK) return false;
    return true;
}

// World to Screen function
bool W2S(const FVector& WorldLocation, FVector2D& ScreenLocation) {
    if (!Cache.LPC || !Cache.LPC->PlayerCameraManager) return false;
    
    FVector CameraLocation = Cache.LPC->PlayerCameraManager->GetCameraLocation();
    FRotator CameraRotation = Cache.LPC->PlayerCameraManager->GetCameraRotation();
    
    // Convert to radians
    float Pitch = CameraRotation.Pitch * 3.14159f / 180.0f;
    float Yaw = CameraRotation.Yaw * 3.14159f / 180.0f;
    
    // Calculate direction vectors
    FVector Forward(
        cos(Pitch) * cos(Yaw),
        cos(Pitch) * sin(Yaw),
        sin(Pitch)
    );
    
    FVector Right(
        cos(Yaw - 3.14159f / 2.0f),
        sin(Yaw - 3.14159f / 2.0f),
        0
    );
    
    FVector Up = FVector(0, 0, 1);
    
    // Calculate relative position
    FVector Delta = WorldLocation - CameraLocation;
    
    // Project onto screen
    float DotForward = Delta.X * Forward.X + Delta.Y * Forward.Y + Delta.Z * Forward.Z;
    float DotRight = Delta.X * Right.X + Delta.Y * Right.Y + Delta.Z * Right.Z;
    float DotUp = Delta.X * Up.X + Delta.Y * Up.Y + Delta.Z * Up.Z;
    
    // Check if behind camera
    if (DotForward < 0.1f) return false;
    
    // Convert to screen coordinates
    float FOV = Cache.LPFOV ? Cache.LPFOV->DefaultFOV : 90.0f;
    float FOVRadians = FOV * 3.14159f / 180.0f;
    
    float ScreenX = (DotRight / DotForward) / tan(FOVRadians / 2.0f) * Cache.WindowSizeX / 2.0f + Cache.WindowSizeX / 2.0f;
    float ScreenY = -(DotUp / DotForward) / tan(FOVRadians / 2.0f) * Cache.WindowSizeY / 2.0f + Cache.WindowSizeY / 2.0f;
    
    ScreenLocation = FVector2D(ScreenX, ScreenY);
    return true;
}

// SDK initialization
bool InitSDK() {
    uintptr_t GameBase = reinterpret_cast<uintptr_t>(GetModuleHandle(nullptr));
    
    // Find GObjects
    UObject::GObjects = *reinterpret_cast<UObject***>(GameBase + 0x173CA60);
    if (!UObject::GObjects) {
        Logger::Log("[SDK]: Failed to find GObjects\n");
        return false;
    }
    
    // Find GNames
    FName::GNames = *reinterpret_cast<FName**>(GameBase + 0x173CA60);
    if (!FName::GNames) {
        Logger::Log("[SDK]: Failed to find GNames\n");
        return false;
    }
    
    Logger::Log("[SDK]: Initialized successfully\n");
    return true;
}

// Game class implementations (simplified)
bool AActor::IsPlayer() const {
    // Implementation would check if actor is a player
    return Class && Class->Name.ToString().find("Player") != std::string::npos;
}

bool AActor::IsDino() const {
    // Implementation would check if actor is a dinosaur
    return Class && Class->Name.ToString().find("Dino") != std::string::npos;
}

bool AActor::IsLocalPlayer() const {
    return this == Cache.LocalActor;
}

bool AActor::IsDead() const {
    return ReplicatedCurrentHealth <= 0;
}

bool AActor::IsConscious() const {
    return ReplicatedCurrentTorpor < ReplicatedMaxTorpor * 0.9f;
}

bool AActor::IsTamed() const {
    // Implementation would check if dino is tamed
    return false; // Placeholder
}

bool AActor::IsFish(const std::string& name) const {
    return name.find("Fish") != std::string::npos;
}

bool AActor::IsManta(const std::string& name) const {
    return name.find("Manta") != std::string::npos;
}

bool AActor::IsAlpha(const std::string& name) const {
    return name.find("Elite") != std::string::npos;
}

bool AActor::IsTurret(const std::string& name, std::string& shortName) const {
    if (name.find("Turret") != std::string::npos) {
        shortName = "Turret";
        return true;
    }
    return false;
}

bool AActor::IsItemContainer() const {
    return Class && Class->Name.ToString().find("Container") != std::string::npos;
}

bool AActor::IsExcludedContainer(const std::string& name) const {
    return name.find("Excluded") != std::string::npos;
}

int AActor::RetrievePlayerGender(const std::string& name) const {
    // Implementation would determine player gender
    return 1; // Default to male
}

bool AActor::IsPrimalCharFriendly(APrimalCharacter* Other) const {
    // Implementation would check if characters are friendly
    return false; // Placeholder
}

APrimalDinoCharacter* AActor::GetBasedOrSeatingOnDino() const {
    // Implementation would return mounted dino
    return nullptr; // Placeholder
}

FVector AActor::GetDirectionVector(const FVector& From, const FVector& To) const {
    FVector Direction = To - From;
    float Length = sqrt(Direction.X * Direction.X + Direction.Y * Direction.Y + Direction.Z * Direction.Z);
    if (Length > 0) {
        Direction = Direction / Length;
    }
    return Direction;
}

FRotator AActor::FindLookAtRotation(const FVector& From, const FVector& To) const {
    FVector Direction = To - From;
    
    float Pitch = atan2(-Direction.Z, sqrt(Direction.X * Direction.X + Direction.Y * Direction.Y)) * 180.0f / 3.14159f;
    float Yaw = atan2(Direction.Y, Direction.X) * 180.0f / 3.14159f;
    
    return FRotator(Pitch, Yaw, 0);
}

FPrimalPlayerDataStruct* AActor::GetPlayerData() const {
    // Implementation would return player data
    return nullptr; // Placeholder
}

// PlayerController implementations
FVector APlayerController::GetCameraLocation() const {
    if (PlayerCameraManager) {
        return PlayerCameraManager->GetCameraLocation();
    }
    return FVector();
}

FRotator APlayerController::GetCameraRotation() const {
    if (PlayerCameraManager) {
        return PlayerCameraManager->GetCameraRotation();
    }
    return FRotator();
}

void APlayerController::SetControlRotation(const FRotator& Rotation) {
    ControlRotation = Rotation;
}

bool APlayerController::IsInputKeyDown(const std::string& KeyName) const {
    // Implementation would check if key is pressed
    if (KeyName == "RButton") {
        return (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0;
    }
    return false;
}

void APlayerController::IgnoreLookInput(bool bIgnore) {
    // Implementation would ignore look input
}

bool APlayerController::LineOfSightTo(AActor* Actor, const FVector& ViewLocation, bool bAlternateChecks) {
    // Implementation would check line of sight
    return true; // Placeholder
}

void APlayerController::EquipPawnItem(int ItemId) {
    // Implementation would equip item
}

// PlayerCameraManager implementations
FVector APlayerCameraManager::GetCameraLocation() const {
    // Implementation would return camera location
    return FVector(); // Placeholder
}

FRotator APlayerCameraManager::GetCameraRotation() const {
    // Implementation would return camera rotation
    return FRotator(); // Placeholder
}

// SkeletalMeshComponent implementations
std::string USkeletalMeshComponent::GetBoneName(int BoneIndex) const {
    // Implementation would return bone name
    return "Bone_" + std::to_string(BoneIndex); // Placeholder
}

// PrimalItem implementations
bool UPrimalItem::IsBroken() const {
    return Durability <= 0;
}

bool UPrimalItem::CanDrop() const {
    return true; // Placeholder
}

bool UPrimalItem::CanUse(bool bIgnoreDurability) const {
    return !IsBroken() || bIgnoreDurability;
}

float UPrimalItem::GetDurabilityPercentage() const {
    if (MaxDurability <= 0) return 0;
    return Durability / MaxDurability;
}

float UPrimalItem::GetItemStatModifier(EPrimalItemStats Stat) const {
    // Implementation would return stat modifier
    return 1.0f; // Placeholder
}

FString UPrimalItem::GetDurabilityStringShort() const {
    // Implementation would return durability string
    return FString(); // Placeholder
}

// Inventory component implementations
UPrimalItem* UPrimalInventoryComponent::GetEquippedItemOfType(UPrimalItem::EquipmentType Type) const {
    for (int i = 0; i < EquippedItems.Count; i++) {
        if (EquippedItems[i] && EquippedItems[i]->MyEquipmentType == Type) {
            return EquippedItems[i];
        }
    }
    return nullptr;
}

// Weapon implementations
FVector* AShooterWeapon::GetAdjustedAim(FVector* Result) {
    // Implementation would return adjusted aim
    return Result; // Placeholder
}

// Menu decoration function
void decorations() {
    ImGui::SetCursorPos(ImVec2(10, 10));
    ImGui::Text("ArkMenu v1.0");
    ImGui::SetCursorPos(ImVec2(Settings.MenuSizeX - 100, 10));
    if (ImGui::Button("X")) {
        Settings.IsMenuOpen = false;
    }
}

// Menu tabs function
void tabss() {
    ImGui::SetCursorPos(ImVec2(10, 40));
    
    if (ImGui::BeginTabBar("##Tabs")) {
        if (ImGui::BeginTabItem("Visuals")) {
            ImGui::Checkbox("Draw Players", &Settings.Visuals.DrawPlayers);
            ImGui::Checkbox("Draw Player Distance", &Settings.Visuals.DrawPlayerDistance);
            ImGui::Checkbox("Draw Player HP", &Settings.Visuals.DrawPlayerHP);
            ImGui::Checkbox("Draw Player Bones", &Settings.Visuals.DrawPlayerBones);
            ImGui::Checkbox("Draw Wild Creatures", &Settings.Visuals.DrawWildCreatures);
            ImGui::Checkbox("Draw Tamed Creatures", &Settings.Visuals.DrawTamedCreatures);
            ImGui::Checkbox("Draw Crosshair", &Settings.Visuals.DrawCrosshair);
            ImGui::Checkbox("Draw Aim FOV", &Settings.Visuals.DrawAimFOV);
            ImGui::SliderFloat("Crosshair Size", &Settings.Visuals.CrosshairSize, 1.0f, 50.0f);
            ImGui::SliderFloat("FOV Size", &Settings.Visuals.FOVSize, 10.0f, 500.0f);
            // --- New Visuals/ESP Features ---
            ImGui::Separator();
            ImGui::Text("Advanced Player ESP");
            ImGui::Checkbox("Draw Local Player", &Settings.Visuals.DrawLocalPlayer);
            ImGui::Checkbox("Draw Conscious Tribe Players", &Settings.Visuals.DrawConsciousTribePlayers);
            ImGui::Checkbox("Draw Sleeping Tribe Players", &Settings.Visuals.DrawSleepingTribePlayers);
            ImGui::Checkbox("Draw Dead Tribe Players", &Settings.Visuals.DrawDeadTribePlayers);
            ImGui::Checkbox("Draw Conscious Enemy Players", &Settings.Visuals.DrawConsciousEnemyPlayers);
            ImGui::Checkbox("Draw Sleeping Enemy Players", &Settings.Visuals.DrawSleepingEnemyPlayers);
            ImGui::Checkbox("Draw Dead Enemy Players", &Settings.Visuals.DrawDeadEnemyPlayers);
            ImGui::Checkbox("Draw Line", &Settings.Visuals.DrawLine);
            ImGui::Checkbox("Draw Line Top Screen", &Settings.Visuals.DrawLineTopScreen);
            ImGui::Checkbox("Draw Muzzle Line", &Settings.Visuals.DrawMuzzleLine);
            ImGui::Checkbox("Draw Gamertag", &Settings.Visuals.DrawGamertag);
            ImGui::Checkbox("Draw Name", &Settings.Visuals.DrawName);
            ImGui::Checkbox("Draw Tribe", &Settings.Visuals.DrawTribe);
            ImGui::Checkbox("Draw Level", &Settings.Visuals.DrawLevel);
            ImGui::Checkbox("Draw Health", &Settings.Visuals.DrawHealth);
            ImGui::Checkbox("Draw Torpor", &Settings.Visuals.DrawTorpor);
            ImGui::Checkbox("Draw Weight", &Settings.Visuals.DrawWeight);
            ImGui::Checkbox("Draw Distance", &Settings.Visuals.DrawDistance);
            ImGui::Checkbox("Draw Skeleton", &Settings.Visuals.DrawSkeleton);
            ImGui::Checkbox("Draw Box", &Settings.Visuals.DrawBox);
            ImGui::Checkbox("Draw Armor", &Settings.Visuals.DrawArmor);
            ImGui::Checkbox("Draw Weapon", &Settings.Visuals.DrawWeapon);
            ImGui::Checkbox("Draw Radar Local Player", &Settings.Visuals.DrawRadarLocalPlayer);
            ImGui::Checkbox("Draw Radar Conscious Tribe Player", &Settings.Visuals.DrawRadarConsciousTribePlayer);
            ImGui::Checkbox("Draw Radar Sleeping Tribe Player", &Settings.Visuals.DrawRadarSleepingTribePlayer);
            ImGui::Checkbox("Draw Radar Dead Tribe Player", &Settings.Visuals.DrawRadarDeadTribePlayer);
            ImGui::Checkbox("Draw Radar Conscious Enemy Player", &Settings.Visuals.DrawRadarConsciousEnemyPlayer);
            ImGui::Checkbox("Draw Radar Sleeping Enemy Player", &Settings.Visuals.DrawRadarSleepingEnemyPlayer);
            ImGui::Checkbox("Draw Radar Dead Enemy Player", &Settings.Visuals.DrawRadarDeadEnemyPlayer);
            // --- Advanced Dino ESP ---
            ImGui::Separator();
            ImGui::Text("Advanced Dino ESP");
            ImGui::Checkbox("Draw Conscious Tribe Dinos", &Settings.Visuals.DrawConsciousTribeDinos);
            ImGui::Checkbox("Draw Sleeping Tribe Dinos", &Settings.Visuals.DrawSleepingTribeDinos);
            ImGui::Checkbox("Draw Dead Tribe Dinos", &Settings.Visuals.DrawDeadTribeDinos);
            ImGui::Checkbox("Draw Conscious Enemy Dinos", &Settings.Visuals.DrawConsciousEnemyDinos);
            ImGui::Checkbox("Draw Sleeping Enemy Dinos", &Settings.Visuals.DrawSleepingEnemyDinos);
            ImGui::Checkbox("Draw Dead Enemy Dinos", &Settings.Visuals.DrawDeadEnemyDinos);
            ImGui::Checkbox("Draw Wild Dinos", &Settings.Visuals.DrawWildDinos);
            ImGui::Checkbox("Draw Tamed Name", &Settings.Visuals.DrawTamedName);
            ImGui::Checkbox("Draw Tamed Tribe", &Settings.Visuals.DrawTamedTribe);
            ImGui::Checkbox("Draw Tamed Level", &Settings.Visuals.DrawTamedLevel);
            ImGui::Checkbox("Draw Tamed Health", &Settings.Visuals.DrawTamedHealth);
            ImGui::Checkbox("Draw Tamed Torpor", &Settings.Visuals.DrawTamedTorpor);
            ImGui::Checkbox("Draw Tamed Distance", &Settings.Visuals.DrawTamedDistance);
            ImGui::Checkbox("Draw Tamed Gender", &Settings.Visuals.DrawTamedGender);
            ImGui::Checkbox("Draw Tamed Aggression", &Settings.Visuals.DrawTamedAggression);
            ImGui::Checkbox("Wild Alpha Filter", &Settings.Visuals.WildAlphaFilter);
            ImGui::Checkbox("Wild Level Filter", &Settings.Visuals.WildLevelFilter);
            ImGui::Checkbox("Dino Chams", &Settings.Visuals.DinoChams);
            ImGui::Checkbox("Draw Wild Name", &Settings.Visuals.DrawWildName);
            ImGui::Checkbox("Draw Wild Level", &Settings.Visuals.DrawWildLevel);
            ImGui::Checkbox("Draw Wild Health", &Settings.Visuals.DrawWildHealth);
            ImGui::Checkbox("Draw Wild Torpor", &Settings.Visuals.DrawWildTorpor);
            ImGui::Checkbox("Draw Wild Distance", &Settings.Visuals.DrawWildDistance);
            ImGui::Checkbox("Draw Wild Gender", &Settings.Visuals.DrawWildGender);
            ImGui::SliderInt("Dino Filter", &Settings.Visuals.DinoFilter, 1, 3);
            // --- Advanced Container ESP ---
            ImGui::Separator();
            ImGui::Text("Container/Structure ESP");
            ImGui::Checkbox("Draw Tribe Containers", &Settings.Visuals.DrawTribeContainers);
            ImGui::Checkbox("Draw Enemy Containers", &Settings.Visuals.DrawEnemyContainers);
            ImGui::Checkbox("Draw Tribe Structures", &Settings.Visuals.DrawTribeStructures);
            ImGui::Checkbox("Draw Enemy Structures", &Settings.Visuals.DrawEnemyStructures);
            ImGui::Checkbox("Container Chams", &Settings.Visuals.ContainerChams);
            ImGui::Checkbox("Draw Container Name", &Settings.Visuals.DrawContainerName);
            ImGui::Checkbox("Draw Container Distance", &Settings.Visuals.DrawContainerDistance);
            ImGui::Checkbox("Draw Container Item Count", &Settings.Visuals.DrawContainerItemCount);
            ImGui::Checkbox("Draw Container Health", &Settings.Visuals.DrawContainerHealth);
            ImGui::Checkbox("Ignore Manequins", &Settings.Visuals.IgnoreManequins);
            ImGui::Checkbox("Beaver Dam Only", &Settings.Visuals.BeaverDamOnly);
            ImGui::Checkbox("Draw Supply/Loot Crates", &Settings.Visuals.DrawSupplyLootCrates);
            ImGui::Checkbox("Show Empty Containers", &Settings.Visuals.ShowEmptyContainers);
            ImGui::Checkbox("Structure Chams", &Settings.Visuals.StructureChams);
            ImGui::Checkbox("Generator Chams", &Settings.Visuals.GeneratorChams);
            ImGui::Checkbox("Draw Structure Name", &Settings.Visuals.DrawStructureName);
            ImGui::Checkbox("Draw Structure Health", &Settings.Visuals.DrawStructureHealth);
            ImGui::Checkbox("Draw Structure Distance", &Settings.Visuals.DrawStructureDistance);
            ImGui::SliderInt("Container Filter", &Settings.Visuals.ContainerFilter, 1, 3);
            // --- Advanced Turret ESP ---
            ImGui::Separator();
            ImGui::Text("Turret ESP");
            ImGui::Checkbox("Draw Tribe Turrets", &Settings.Visuals.DrawTribeTurrets);
            ImGui::Checkbox("Draw Enemy Turrets", &Settings.Visuals.DrawEnemyTurrets);
            ImGui::Checkbox("Turret Chams", &Settings.Visuals.TurretChams);
            ImGui::Checkbox("Draw Turret Name", &Settings.Visuals.DrawTurretName);
            ImGui::Checkbox("Draw Turret Distance", &Settings.Visuals.DrawTurretDistance);
            ImGui::Checkbox("Draw Bullet Count", &Settings.Visuals.DrawBulletCount);
            ImGui::Checkbox("Draw Shot Count", &Settings.Visuals.DrawShotCount);
            ImGui::Checkbox("Draw Target Setting", &Settings.Visuals.DrawTargetSetting);
            ImGui::Checkbox("Draw Range Setting", &Settings.Visuals.DrawRangeSetting);
            ImGui::Checkbox("Draw Warning Setting", &Settings.Visuals.DrawWarningSetting);
            ImGui::Checkbox("Draw Empty Turrets", &Settings.Visuals.DrawEmptyTurrets);
            // --- Other ESP ---
            ImGui::Separator();
            ImGui::Text("Other ESP");
            ImGui::Checkbox("Draw Boss Fights", &Settings.Visuals.DrawBossFights);
            ImGui::Checkbox("Draw Dropped Items", &Settings.Visuals.DrawDroppedItems);
            ImGui::Checkbox("Draw Element Vein", &Settings.Visuals.DrawElementVein);
            ImGui::Checkbox("Create Marker", &Settings.Visuals.CreateMarker);
            ImGui::Checkbox("Clear Marker", &Settings.Visuals.ClearMarker);
            ImGui::Checkbox("Explorer Note ESP", &Settings.Visuals.ExplorerNoteEsp);
            ImGui::Checkbox("Hide Unlocked Explorer Notes", &Settings.Visuals.HideUnlockedExplorerNotes);
            ImGui::Checkbox("Draw Explorer Name", &Settings.Visuals.DrawExplorerName);
            ImGui::Checkbox("Draw Explorer Distance", &Settings.Visuals.DrawExplorerDistance);
            ImGui::Checkbox("Self Armor ESP", &Settings.Visuals.SelfArmorESP);
            ImGui::Checkbox("Self Weapon ESP", &Settings.Visuals.SelfWeaponESP);
            ImGui::Checkbox("Self Health ESP", &Settings.Visuals.SelfHealthESP);
            ImGui::SliderFloat("Self Health ESP Height", &Settings.Visuals.SelfHealthESPHeight, 10.0f, 200.0f);
            ImGui::Checkbox("Self Reload Cooldown", &Settings.Visuals.SelfReloadCooldown);
            ImGui::Checkbox("Cham Wire Frame", &Settings.Visuals.ChamWireFrame);
            ImGui::SliderFloat("Cham Opacity", &Settings.Visuals.ChamOpacity, 0.0f, 1.0f);
            ImGui::Checkbox("Draw Boss Name", &Settings.Visuals.DrawBossName);
            ImGui::Checkbox("Draw Dropped Item Name", &Settings.Visuals.DrawDroppedItemName);
            ImGui::Checkbox("Draw Dropped Item Distance", &Settings.Visuals.DrawDroppedItemDistance);
            ImGui::Checkbox("Draw Dropped Item Owner Name", &Settings.Visuals.DrawDroppedItemOwnerName);
            ImGui::Checkbox("Draw OSD Name", &Settings.Visuals.DrawOSDName);
            ImGui::Checkbox("Draw OSD Health", &Settings.Visuals.DrawOSDHealth);
            ImGui::Checkbox("Draw OSD Item Count", &Settings.Visuals.DrawOSDItemCount);
            ImGui::Checkbox("Draw OSD Distance", &Settings.Visuals.DrawOSDDistance);
            ImGui::Checkbox("Draw Element Vein Name", &Settings.Visuals.DrawElementVeinName);
            ImGui::Checkbox("Draw Element Vein Health", &Settings.Visuals.DrawElementVeinHealth);
            ImGui::Checkbox("Draw Element Vein Item Count", &Settings.Visuals.DrawElementVeinItemCount);
            ImGui::Checkbox("Draw Element Vein Distance", &Settings.Visuals.DrawElementVeinDistance);
            ImGui::Checkbox("Enable Markers", &Settings.Visuals.EnableMarkers);
            ImGui::Checkbox("Clear All Markers", &Settings.Visuals.ClearAllMarkers);
            ImGui::Checkbox("Show Marker Name", &Settings.Visuals.ShowMarkerName);
            ImGui::Checkbox("Draw Marker Distance", &Settings.Visuals.DrawMarkerDistance);
            ImGui::EndTabItem();
        }
        
        if (ImGui::BeginTabItem("Aimbot")) {
            ImGui::Checkbox("Enable Aimbot", &Settings.Aimbot.EnableAimbot);
            ImGui::Checkbox("Silent Aim", &Settings.Aimbot.SilentAim);
            ImGui::Checkbox("Target Tribe", &Settings.Aimbot.TargetTribe);
            ImGui::Checkbox("Target Sleepers", &Settings.Aimbot.TargetSleepers);
            ImGui::Checkbox("Visible Only", &Settings.Aimbot.VisibleOnly);
            
            // --- Advanced Aimbot Features ---
            ImGui::Separator();
            ImGui::Text("Advanced Aimbot");
            ImGui::Checkbox("Whip Bot", &Settings.Aimbot.WhipBot);
            ImGui::Checkbox("Ghost Peak", &Settings.Aimbot.GhostPeak);
            ImGui::Checkbox("Managarmar Silent", &Settings.Aimbot.ManagarmarSilent);
            ImGui::Checkbox("Desert Titan Silent", &Settings.Aimbot.DesertTitanSilent);
            ImGui::Checkbox("Trigger Bot", &Settings.Aimbot.TriggerBot);
            ImGui::Checkbox("Target Enemy", &Settings.Aimbot.TargetEnemy);
            ImGui::Checkbox("Target Dino", &Settings.Aimbot.TargetDino);
            ImGui::Checkbox("Target FjordHawk", &Settings.Aimbot.TargetFjordHawk);
            
            // --- Tek Saddle Aimbot ---
            ImGui::Separator();
            ImGui::Text("Tek Saddle Aimbot");
            ImGui::Checkbox("Tribe Tek Saddle Aimbot", &Settings.Aimbot.TribeTekSaddleAimbot);
            ImGui::Checkbox("Enemy Tek Saddle Aimbot", &Settings.Aimbot.EnemyTekSaddleAimbot);
            ImGui::Checkbox("Tribe Tek Saddle Silent", &Settings.Aimbot.TribeTekSaddleSilent);
            ImGui::Checkbox("Enemy Tek Saddle Silent", &Settings.Aimbot.EnemyTekSaddleSilent);
            ImGui::Checkbox("Target Tek Generator", &Settings.Aimbot.TargetTekGenerator);
            ImGui::Checkbox("Target Gas Generator", &Settings.Aimbot.TargetGasGenerator);
            ImGui::Checkbox("Target Tek Turret", &Settings.Aimbot.TargetTekTurret);
            ImGui::Checkbox("Target Tek Foundation", &Settings.Aimbot.TargetTekFoundation);
            ImGui::Checkbox("Target Metal Foundation", &Settings.Aimbot.TargetMetalFoundation);
            ImGui::Checkbox("Target Tek Triangle Foundation", &Settings.Aimbot.TargetTekTriangleFoundation);
            ImGui::Checkbox("Target Metal Triangle Foundation", &Settings.Aimbot.TargetMetalTriangleFoundation);
            ImGui::Checkbox("Target Metal Cliff Platform", &Settings.Aimbot.TargetMetalCliffPlatform);
            ImGui::Checkbox("Target Stone Cliff Platform", &Settings.Aimbot.TargetStoneCliffPlatform);
            ImGui::Checkbox("Target Tek Double Door Frame", &Settings.Aimbot.TargetTekDoubleDoorFrame);
            ImGui::Checkbox("Target Metal Double Door Frame", &Settings.Aimbot.TargetMetalDoubleDoorFrame);
            ImGui::Checkbox("Target Tek Hatch Frame", &Settings.Aimbot.TargetTekHatchFrame);
            ImGui::Checkbox("Target Metal Hatch Frame", &Settings.Aimbot.TargetMetalHatchFrame);
            
            // --- Target Bone ---
            ImGui::Separator();
            ImGui::Text("Target Bone");
            ImGui::Checkbox("Target Head", &Settings.Aimbot.TargetHead);
            ImGui::Checkbox("Target Chest", &Settings.Aimbot.TargetChest);
            ImGui::Checkbox("Target Pelvis", &Settings.Aimbot.TargetPelvis);
            ImGui::Checkbox("Target Legs", &Settings.Aimbot.TargetLegs);
            ImGui::Checkbox("Target Hands", &Settings.Aimbot.TargetHands);
            ImGui::Checkbox("Target Feet", &Settings.Aimbot.TargetFeet);
            ImGui::Checkbox("Target Mix", &Settings.Aimbot.TargetMix);
            ImGui::Checkbox("Cycle Bone", &Settings.Aimbot.CycleBone);
            
            ImGui::SliderFloat("Aim FOV", &Settings.Aimbot.AimFOV, 10.0f, 500.0f);
            
            ImGui::EndTabItem();
        }
        
        if (ImGui::BeginTabItem("Weapon")) {
            ImGui::Text("Weapon Modifications");
            ImGui::Separator();
            
            ImGui::Checkbox("Self C4", &Settings.Misc.SelfC4);
            ImGui::Checkbox("Infinite C4", &Settings.Misc.InfiniteC4);
            ImGui::Checkbox("Unlock Weapon Dino Rotation", &Settings.Misc.UnlockWeaponDinoRotation);
            ImGui::Checkbox("No OverHeat", &Settings.Misc.NoOverHeat);
            ImGui::Checkbox("Instant Shotgun Reload", &Settings.Misc.InstantShotgunReload);
            ImGui::Checkbox("Auto Swap Fabi", &Settings.Misc.AutoSwapFabi);
            ImGui::Checkbox("Auto Swap AR", &Settings.Misc.AutoSwapAR);
            ImGui::Checkbox("No Scope Overlay", &Settings.Misc.NoScopeOverlay);
            ImGui::Checkbox("No Tek Rifle Overlay", &Settings.Misc.NoTekRifleOverlay);
            ImGui::Checkbox("Shield Bypass", &Settings.Misc.ShieldBypass);
            ImGui::Checkbox("No Spread", &Settings.Misc.NoSpread);
            ImGui::Checkbox("No Sway", &Settings.Misc.NoSway);
            ImGui::Checkbox("No Recoil", &Settings.Misc.NoRecoil);
            ImGui::Checkbox("Aim While Reloading", &Settings.Misc.AimWhileReloading);
            ImGui::Checkbox("Allow Mounted Weapon", &Settings.Misc.AllowMountedWeapon);
            ImGui::Checkbox("Infinite Weapon Range", &Settings.Misc.InfiniteWeaponRange);
            ImGui::Checkbox("Tek Rifle Hit Indicator", &Settings.Misc.TekRifleHitIndicator);
            ImGui::Checkbox("Conserve Shots", &Settings.Misc.ConserveShots);
            ImGui::Checkbox("Weapon Spyglass", &Settings.Misc.WeaponSpyglass);
            ImGui::Checkbox("Running Weapon", &Settings.Misc.RunningWeapon);
            ImGui::Checkbox("No Weapon Un-Equip", &Settings.Misc.NoWeaponUnEquip);
            ImGui::Checkbox("Quick Reload Fabricated Sniper", &Settings.Misc.QuickReloadFabricatedSniper);
            ImGui::Checkbox("No Reload Shake", &Settings.Misc.NoReloadShake);
            ImGui::Checkbox("Tracers", &Settings.Misc.Tracers);
            ImGui::Checkbox("Rapid Fire", &Settings.Misc.RapidFire);
            ImGui::Checkbox("Adaptive Rapid Fire", &Settings.Misc.AdaptiveRapidFire);
            ImGui::Checkbox("Rocket Spam", &Settings.Misc.RocketSpam);
            ImGui::Checkbox("Bow Spam", &Settings.Misc.BowSpam);
            ImGui::SliderFloat("Weapon Size", &Settings.Misc.WeaponSize, 0.1f, 5.0f);
            ImGui::SliderFloat("Extra Scope Zoom", &Settings.Misc.ExtraScopeZoom, 1.0f, 10.0f);
            ImGui::Checkbox("Better Chainsaw", &Settings.Misc.BetterChainsaw);
            ImGui::Checkbox("Net Spam", &Settings.Misc.NetSpam);
            ImGui::Checkbox("Cryopod Spam", &Settings.Misc.CryopodSpam);
            ImGui::Checkbox("Tek Rifle Spam", &Settings.Misc.TekRifleSpam);
            ImGui::Checkbox("Tek Launcher Spam", &Settings.Misc.TekLauncherSpam);
            ImGui::Checkbox("Railgun Spam", &Settings.Misc.RailgunSpam);
            
            ImGui::EndTabItem();
        }
        
        if (ImGui::BeginTabItem("Character")) {
            ImGui::Text("Character Modifications");
            ImGui::Separator();
            
            ImGui::Checkbox("Tek Punch", &Settings.Misc.TekPunch);
            ImGui::Checkbox("Tek Chest", &Settings.Misc.TekChest);
            ImGui::Checkbox("Tek Sword Fly", &Settings.Misc.TekSwordFly);
            ImGui::SliderFloat("Player Charge Speed", &Settings.Misc.PlayerChargeSpeed, 1.0f, 10.0f);
            ImGui::Checkbox("Unlock Explorer Notes", &Settings.Misc.UnlockExplorerNotes);
            ImGui::Checkbox("Unlock Engrams", &Settings.Misc.UnlockEngrams);
            ImGui::Checkbox("Air Struck", &Settings.Misc.AirStruck);
            ImGui::Checkbox("Upload Inventory To Ark", &Settings.Misc.UploadInventoryToArk);
            ImGui::Checkbox("Download Inventory From Ark", &Settings.Misc.DownloadInventoryFromArk);
            ImGui::Checkbox("Auto Mount", &Settings.Misc.AutoMount);
            ImGui::Checkbox("Auto Hatch Egg", &Settings.Misc.AutoHatchEgg);
            ImGui::Checkbox("Auto Destroy Egg", &Settings.Misc.AutoDestroyEgg);
            ImGui::Checkbox("Remove Bolad Tribemates", &Settings.Misc.RemoveBoladTribemates);
            ImGui::Checkbox("Tek Gaunlet Air Stuck", &Settings.Misc.TekGaunletAirStuck);
            ImGui::Checkbox("Long Arms", &Settings.Misc.LongArms);
            ImGui::Checkbox("Infinite Element", &Settings.Misc.InfiniteElement);
            ImGui::Checkbox("Insta Turn", &Settings.Misc.InstaTurn);
            ImGui::Checkbox("Infinite Orbit", &Settings.Misc.InfiniteOrbit);
            ImGui::Checkbox("Gamertag Spoof", &Settings.Misc.GamertagSpoof);
            ImGui::Checkbox("Gamertag Spam", &Settings.Misc.GamertagSpam);
            ImGui::Checkbox("Ghost Mode", &Settings.Misc.GhostMode);
            ImGui::Checkbox("Auto Brew", &Settings.Misc.AutoBrew);
            ImGui::SliderFloat("Character Size", &Settings.Misc.CharacterSize, 0.1f, 5.0f);
            ImGui::Checkbox("Anti Hit", &Settings.Misc.AntiHit);
            ImGui::Checkbox("Spinbot", &Settings.Misc.Spinbot);
            ImGui::SliderFloat("Player Speed", &Settings.Misc.PlayerSpeed, 1.0f, 10.0f);
            ImGui::SliderFloat("FOV Changer", &Settings.Misc.FOVChanger, 30.0f, 180.0f);
            ImGui::Checkbox("Desync", &Settings.Misc.Desync);
            ImGui::Checkbox("Gamerscore Unlocker", &Settings.Misc.GamerscoreUnlocker);
            ImGui::Checkbox("Auto Armor", &Settings.Misc.AutoArmor);
            
            if (Settings.Misc.AutoArmor) {
                ImGui::Indent();
                ImGui::Checkbox("Prioritize Tek Helmet", &Settings.Misc.PrioritizeTekHelmet);
                ImGui::Checkbox("Prioritize Tek Chest", &Settings.Misc.PrioritizeTekChest);
                ImGui::Checkbox("Prioritize Tek Leggings", &Settings.Misc.PrioritizeTekLeggings);
                ImGui::Checkbox("Prioritize Tek Gauntlets", &Settings.Misc.PrioritizeTekGauntlets);
                ImGui::Checkbox("Prioritize Tek Boots", &Settings.Misc.PrioritizeTekBoots);
                ImGui::Checkbox("Prioritize Tek Shield", &Settings.Misc.PrioritizeTekShield);
                ImGui::Unindent();
            }
            
            ImGui::EndTabItem();
        }
        
        if (ImGui::BeginTabItem("Dino")) {
            ImGui::Text("Dino Modifications");
            ImGui::Separator();
            
            ImGui::Checkbox("Backwards Dino Movement", &Settings.Misc.BackwardsDinoMovement);
            ImGui::Checkbox("Insta Turn (Dino)", &Settings.Misc.InstaTurnDino);
            ImGui::SliderFloat("Dino Charge Speed", &Settings.Misc.DinoChargeSpeed, 1.0f, 10.0f);
            ImGui::Checkbox("Auto Claim", &Settings.Misc.AutoClaim);
            ImGui::Checkbox("Auto Imprint", &Settings.Misc.AutoImprint);
            ImGui::Checkbox("Instant Magmasaur Fire", &Settings.Misc.InstantMagmasaurFire);
            ImGui::Checkbox("Desert Titan Lightning", &Settings.Misc.DesertTitanLightning);
            ImGui::Checkbox("Desert Titan Infinite Charge", &Settings.Misc.DesertTitanInfiniteCharge);
            ImGui::Checkbox("Auto Feed Babys", &Settings.Misc.AutoFeedBabys);
            ImGui::Checkbox("Auto Feed Daedons", &Settings.Misc.AutoFeedDaedons);
            ImGui::Checkbox("Auto Feed Mounted Dino", &Settings.Misc.AutoFeedMountedDino);
            ImGui::Checkbox("Auto Name Dinos", &Settings.Misc.AutoNameDinos);
            ImGui::Checkbox("Auto Name Babys", &Settings.Misc.AutoNameBabys);
            ImGui::SliderFloat("Auto Name Radius", &Settings.Misc.AutoNameRadius, 10.0f, 500.0f);
            ImGui::SliderFloat("Dino Speed", &Settings.Misc.DinoSpeed, 1.0f, 10.0f);
            ImGui::SliderFloat("Dino Speed Latency", &Settings.Misc.DinoSpeedLatency, 1.0f, 10.0f);
            ImGui::Checkbox("Deino Jump", &Settings.Misc.DeinoJump);
            ImGui::Checkbox("Deino Fly", &Settings.Misc.DeinoFly);
            ImGui::SliderFloat("Maewing Speed", &Settings.Misc.MaewingSpeed, 1.0f, 10.0f);
            ImGui::Checkbox("Infinite Dino Dive", &Settings.Misc.InfiniteDinoDive);
            ImGui::Checkbox("Managarmr Dash", &Settings.Misc.ManagarmrDash);
            ImGui::Checkbox("Super Andrew Jump", &Settings.Misc.SuperAndrewJump);
            ImGui::Checkbox("Auto Level Health", &Settings.Misc.AutoLevelHealth);
            ImGui::Checkbox("Auto Level Stamina", &Settings.Misc.AutoLevelStamina);
            ImGui::Checkbox("Auto Level Oxygen", &Settings.Misc.AutoLevelOxygen);
            ImGui::Checkbox("Auto Level Food", &Settings.Misc.AutoLevelFood);
            ImGui::Checkbox("Auto Level Weight", &Settings.Misc.AutoLevelWeight);
            ImGui::Checkbox("Auto Level Melee", &Settings.Misc.AutoLevelMelee);
            ImGui::Checkbox("Auto Level Speed", &Settings.Misc.AutoLevelSpeed);
            
            ImGui::EndTabItem();
        }
        
        if (ImGui::BeginTabItem("Extra 1")) {
            ImGui::Text("Extra Features 1");
            ImGui::Separator();
            
            ImGui::Checkbox("Plus Crosshair", &Settings.Misc.PlusCrosshair);
            ImGui::Checkbox("Circle Crosshair", &Settings.Misc.CircleCrosshair);
            ImGui::SliderFloat("Crosshair Size", &Settings.Misc.CrosshairSize, 1.0f, 50.0f);
            ImGui::SliderFloat("Crosshair Width", &Settings.Misc.CrosshairWidth, 1.0f, 10.0f);
            ImGui::Checkbox("Pickup Turrets", &Settings.Misc.PickupTurrets);
            ImGui::Checkbox("Pickup Bear Traps", &Settings.Misc.PickupBearTraps);
            ImGui::Checkbox("Pickup C4", &Settings.Misc.PickupC4);
            ImGui::Checkbox("PopCorn Generators", &Settings.Misc.PopCornGenerators);
            ImGui::Checkbox("PopCorn Turrets", &Settings.Misc.PopCornTurrets);
            ImGui::Checkbox("Demolish Grinders", &Settings.Misc.DemolishGrinders);
            ImGui::Checkbox("Auto Craft ARB", &Settings.Misc.AutoCraftARB);
            ImGui::Checkbox("Auto Craft GB", &Settings.Misc.AutoCraftGB);
            ImGui::Checkbox("Auto Craft SP", &Settings.Misc.AutoCraftSP);
            ImGui::Checkbox("Auto Foundation To Block", &Settings.Misc.AutoFoundationToBlock);
            ImGui::Checkbox("Auto Foundation To Default", &Settings.Misc.AutoFoundationToDefault);
            ImGui::Checkbox("WildCard Admin Chat", &Settings.Misc.WildCardAdminChat);
            ImGui::Checkbox("Server Admin Chat", &Settings.Misc.ServerAdminChat);
            ImGui::Checkbox("Ark Developer Chat", &Settings.Misc.ArkDeveloperChat);
            ImGui::Checkbox("Transmitter Bag", &Settings.Misc.TransmitterBag);
            ImGui::Checkbox("Dupe", &Settings.Misc.Dupe);
            ImGui::Checkbox("Click Dupe", &Settings.Misc.ClickDupe);
            ImGui::Checkbox("Force Upload", &Settings.Misc.ForceUpload);
            ImGui::Checkbox("Remove Upload/Download Screen", &Settings.Misc.RemoveUploadDownloadScreen);
            ImGui::Checkbox("Auto Pick Up Dropped Items", &Settings.Misc.AutoPickUpDroppedItems);
            ImGui::Checkbox("Damage Numbers", &Settings.Misc.DamageNumbers);
            ImGui::Checkbox("Mesh Warning", &Settings.Misc.MeshWarning);
            ImGui::Checkbox("Line Of Sight Warning", &Settings.Misc.LineOfSightWarning);
            ImGui::Checkbox("Gamma Gen 2 Boss", &Settings.Misc.GammaGen2Boss);
            ImGui::Checkbox("Beta Gen 2 Boss", &Settings.Misc.BetaGen2Boss);
            ImGui::Checkbox("Alpha Gen 2 Boss", &Settings.Misc.AlphaGen2Boss);
            ImGui::Checkbox("Force Open Hexagon Shop", &Settings.Misc.ForceOpenHexagonShop);
            ImGui::Checkbox("Better Upload + Download", &Settings.Misc.BetterUploadDownload);
            ImGui::Checkbox("Ghost Peak Target Name", &Settings.Misc.GhostPeakTargetName);
            ImGui::Checkbox("Aimbot Target Name", &Settings.Misc.AimbotTargetName);
            ImGui::Checkbox("Auto Loot Containers", &Settings.Misc.AutoLootContainers);
            ImGui::Checkbox("Auto Loot Players", &Settings.Misc.AutoLootPlayers);
            
            ImGui::EndTabItem();
        }
        
        if (ImGui::BeginTabItem("Extra 2")) {
            ImGui::Text("Extra Features 2");
            ImGui::Separator();
            
            ImGui::Checkbox("Panic Mode", &Settings.Misc.PanicMode);
            ImGui::Checkbox("Chat Log", &Settings.Misc.ChatLog);
            ImGui::Checkbox("Potatoe INI", &Settings.Misc.PotatoeINI);
            ImGui::Checkbox("Hard INI", &Settings.Misc.HardINI);
            ImGui::Checkbox("No Landscape INI", &Settings.Misc.NoLandscapeINI);
            ImGui::Checkbox("No Rocks and Trees", &Settings.Misc.NoRocksAndTrees);
            ImGui::Checkbox("No Water INI", &Settings.Misc.NoWaterINI);
            ImGui::Checkbox("No Particle INI", &Settings.Misc.NoParticleINI);
            ImGui::Checkbox("Full Bright INI", &Settings.Misc.FullBrightINI);
            ImGui::Checkbox("No XYZ", &Settings.Misc.NoXYZ);
            ImGui::Checkbox("No Respawn Animation", &Settings.Misc.NoRespawnAnimation);
            ImGui::Checkbox("Create Character", &Settings.Misc.CreateCharacter);
            
            if (Settings.Misc.CreateCharacter) {
                ImGui::Indent();
                ImGui::Checkbox("Small Character", &Settings.Misc.SmallCharacter);
                ImGui::Checkbox("Female Character", &Settings.Misc.FemaleCharacter);
                ImGui::Checkbox("Custom Hair Color", &Settings.Misc.CustomHairColor);
                ImGui::Checkbox("Custom Body Color", &Settings.Misc.CustomBodyColor);
                ImGui::InputText("Character Name", &Settings.Misc.CharacterName);
                ImGui::Unindent();
            }
            
            ImGui::Checkbox("Resource Pull", &Settings.Misc.ResourcePull);
            ImGui::Checkbox("Auto Craft Selected", &Settings.Misc.AutoCraftSelected);
            ImGui::Checkbox("Force Open Inventory", &Settings.Misc.ForceOpenInventory);
            ImGui::Checkbox("Unlock Gen 1 Glitches", &Settings.Misc.UnlockGen1Glitches);
            ImGui::Checkbox("Turn On Tek Lights", &Settings.Misc.TurnOnTekLights);
            ImGui::SliderFloat("Aim FOV", &Settings.Misc.AimFov, 10.0f, 500.0f);
            ImGui::Checkbox("Admin Cracker", &Settings.Misc.AdminCracker);
            ImGui::Checkbox("Pin Cracker", &Settings.Misc.PinCracker);
            ImGui::Checkbox("Free Camera", &Settings.Misc.FreeCamera);
            ImGui::Checkbox("Custom Kill", &Settings.Misc.CustomKill);
            ImGui::Checkbox("Discord Tribe Log", &Settings.Misc.DiscordTribeLog);
            ImGui::Checkbox("Chat Spammer", &Settings.Misc.ChatSpammer);
            ImGui::Checkbox("Dino Death", &Settings.Misc.DinoDeath);
            ImGui::InputText("Dino Name", &Settings.Misc.DinoName);
            ImGui::Checkbox("Enemy Warning", &Settings.Misc.EnemyWarning);
            ImGui::Checkbox("Admin Warning", &Settings.Misc.AdminWarning);
            ImGui::SliderFloat("Warning Size", &Settings.Misc.WarningSize, 0.1f, 5.0f);
            ImGui::SliderFloat("ESP Size", &Settings.Misc.ESPSize, 0.1f, 5.0f);
            ImGui::SliderFloat("ESP Icon Size", &Settings.Misc.ESPIconSize, 0.1f, 5.0f);
            
            ImGui::EndTabItem();
        }
        
        if (ImGui::BeginTabItem("HUD-Display")) {
            ImGui::Text("HUD Display Settings");
            ImGui::Separator();
            
            ImGui::Checkbox("HUD Information", &Settings.HudDisplay.HudInformation);
            ImGui::Checkbox("Show FPS", &Settings.HudDisplay.ShowFPS);
            ImGui::Checkbox("Show AimKey", &Settings.HudDisplay.ShowAimKey);
            ImGui::Checkbox("Show AimBone", &Settings.HudDisplay.ShowAimBone);
            ImGui::Checkbox("Show Cords", &Settings.HudDisplay.ShowCords);
            ImGui::Checkbox("Show Ping", &Settings.HudDisplay.ShowPing);
            ImGui::Checkbox("Show Server FPS", &Settings.HudDisplay.ShowServerFPS);
            ImGui::Checkbox("Show Server Name", &Settings.HudDisplay.ShowServerName);
            ImGui::Checkbox("Show Map Name", &Settings.HudDisplay.ShowMapName);
            ImGui::Checkbox("Show Connected Players", &Settings.HudDisplay.ShowConnectedPlayers);
            ImGui::Checkbox("Show Tamed Dinos", &Settings.HudDisplay.ShowTamedDinos);
            ImGui::Checkbox("Show Max Tamed Dinos", &Settings.HudDisplay.ShowMaxTamedDinos);
            ImGui::Checkbox("Show Turrets Limit", &Settings.HudDisplay.ShowTurretsLimit);
            ImGui::Checkbox("Show Turrets Limit Range", &Settings.HudDisplay.ShowTurretsLimitRange);
            ImGui::Checkbox("Show Structure Limit", &Settings.HudDisplay.ShowStructureLimit);
            ImGui::Checkbox("Show Day Number", &Settings.HudDisplay.ShowDayNumber);
            ImGui::Checkbox("Show Day Time", &Settings.HudDisplay.ShowDayTime);
            ImGui::InputText("Admin Cracker Password", &Settings.HudDisplay.AdminCrackerPassword);
            ImGui::InputText("Pin Cracker Password", &Settings.HudDisplay.PinCrackerPassword);
            ImGui::Checkbox("Show Toggle Keys", &Settings.HudDisplay.ShowToggleKeys);
            ImGui::InputText("Discord Invite", &Settings.HudDisplay.DiscordInvite);
            ImGui::Checkbox("RainBow Info", &Settings.HudDisplay.RainBowInfo);
            
            ImGui::EndTabItem();
        }
        
        if (ImGui::BeginTabItem("Keybinds 1")) {
            ImGui::Text("Keybinds 1");
            ImGui::Separator();
            
            ImGui::InputText("Aimbot", &Settings.Keybinds.AimbotKey);
            ImGui::InputText("Air Struck", &Settings.Keybinds.AirStruckKey);
            ImGui::InputText("Auto Loot", &Settings.Keybinds.AutoLootKey);
            ImGui::InputText("Tek Punch", &Settings.Keybinds.TekPunchKey);
            ImGui::InputText("Tek Chest", &Settings.Keybinds.TekChestKey);
            ImGui::InputText("Tek Sword Fly", &Settings.Keybinds.TekSwordFlyKey);
            ImGui::InputText("Ghost Mode", &Settings.Keybinds.GhostModeKey);
            ImGui::InputText("Infinite Dino Dive", &Settings.Keybinds.InfiniteDinoDiveKey);
            ImGui::InputText("Player Speed", &Settings.Keybinds.PlayerSpeedKey);
            ImGui::InputText("Dino Speed", &Settings.Keybinds.DinoSpeedKey);
            ImGui::InputText("Rocket Spam", &Settings.Keybinds.RocketSpamKey);
            ImGui::InputText("Bow Spam", &Settings.Keybinds.BowSpamKey);
            ImGui::InputText("Upload", &Settings.Keybinds.UploadKey);
            ImGui::InputText("Desync", &Settings.Keybinds.DesyncKey);
            ImGui::InputText("Upload Inventory", &Settings.Keybinds.UploadInventoryKey);
            ImGui::InputText("Download Inventory", &Settings.Keybinds.DownloadInventoryKey);
            ImGui::InputText("Player Charge Speed", &Settings.Keybinds.PlayerChargeSpeedKey);
            ImGui::InputText("Dino Charge Speed", &Settings.Keybinds.DinoChargeSpeedKey);
            ImGui::InputText("Ghost Peak", &Settings.Keybinds.GhostPeakKey);
            ImGui::InputText("Cryopod Spam", &Settings.Keybinds.CryopodSpamKey);
            ImGui::InputText("Net Spam", &Settings.Keybinds.NetSpamKey);
            ImGui::InputText("Deino Fly", &Settings.Keybinds.DeinoFlyKey);
            ImGui::InputText("Force Open", &Settings.Keybinds.ForceOpenKey);
            ImGui::InputText("Railgun Spam", &Settings.Keybinds.RailgunSpamKey);
            ImGui::InputText("Tek Rifle", &Settings.Keybinds.TekRifleKey);
            ImGui::InputText("Tek Gauntlet Air Stuck", &Settings.Keybinds.TekGauntletAirStuckKey);
            ImGui::InputText("Auto Mount", &Settings.Keybinds.AutoMountKey);
            ImGui::InputText("Whip Bot Key", &Settings.Keybinds.WhipBotKey);
            
            ImGui::EndTabItem();
        }
        
        if (ImGui::BeginTabItem("Keybinds 2")) {
            ImGui::Text("Keybinds 2");
            ImGui::Separator();
            
            ImGui::InputText("Create Marker Key", &Settings.Keybinds.CreateMarkerKey);
            ImGui::InputText("Auto Level Key", &Settings.Keybinds.AutoLevelKey);
            ImGui::InputText("Bone Cycle Key", &Settings.Keybinds.BoneCycleKey);
            ImGui::InputText("Dupe Key", &Settings.Keybinds.DupeKey);
            ImGui::InputText("Click Dupe Key", &Settings.Keybinds.ClickDupeKey);
            ImGui::InputText("Insta Turn Key", &Settings.Keybinds.InstaTurnKey);
            ImGui::InputText("Silent Aim Key", &Settings.Keybinds.SilentAimKey);
            ImGui::InputText("Anti Hit Key", &Settings.Keybinds.AntiHitKey);
            ImGui::InputText("Panic Key", &Settings.Keybinds.PanicKey);
            ImGui::InputText("Menu", &Settings.Keybinds.MenuKey);
            
            ImGui::EndTabItem();
        }
        
        if (ImGui::BeginTabItem("Colors")) {
            ImGui::Text("Color Settings");
            ImGui::Separator();
            
            ImGui::ColorEdit3("Player Color", Settings.PlayerColor1);
            ImGui::ColorEdit3("Tamed Dino Color", Settings.TamedDinoColor1);
            ImGui::ColorEdit3("Wild Dino Color", Settings.WildDinoColor1);
            ImGui::ColorEdit3("Turret Color", Settings.TurretColor1);
            ImGui::ColorEdit3("Container Color", Settings.ContainerColor1);
            ImGui::ColorEdit3("Crosshair Color", Settings.CrosshairColor1);
            
            ImGui::Separator();
            ImGui::Text("Advanced Colors");
            
            ImGui::ColorEdit3("Conscious Tribe Players", Settings.ConsciousTribePlayers);
            ImGui::ColorEdit3("Sleeping Tribe Players", Settings.SleepingTribePlayers);
            ImGui::ColorEdit3("Dead Tribe Players", Settings.DeadTribePlayers);
            ImGui::ColorEdit3("Conscious Enemy Players", Settings.ConsciousEnemyPlayers);
            ImGui::ColorEdit3("Sleeping Enemy Players", Settings.SleepingEnemyPlayers);
            ImGui::ColorEdit3("Dead Enemy Players", Settings.DeadEnemyPlayers);
            ImGui::ColorEdit3("Conscious Tribe Dinos", Settings.ConsciousTribeDinos);
            ImGui::ColorEdit3("Sleeping Tribe Dinos", Settings.SleepingTribeDinos);
            ImGui::ColorEdit3("Dead Tribe Dinos", Settings.DeadTribeDinos);
            ImGui::ColorEdit3("Conscious Enemy Dinos", Settings.ConsciousEnemyDinos);
            ImGui::ColorEdit3("Sleeping Enemy Dinos", Settings.SleepingEnemyDinos);
            ImGui::ColorEdit3("Dead Enemy Dinos", Settings.DeadEnemyDinos);
            ImGui::ColorEdit3("Wild Dinos", Settings.WildDinos);
            ImGui::ColorEdit3("Container Tribe", Settings.ContainerTribe);
            ImGui::ColorEdit3("Container Enemy", Settings.ContainerEnemy);
            ImGui::ColorEdit3("Character Body", Settings.CharacterBody);
            ImGui::ColorEdit3("Enemy Hair", Settings.EnemyHair);
            ImGui::ColorEdit3("HUD Information", Settings.HudInformation);
            ImGui::ColorEdit3("CrossHair", Settings.CrossHair);
            ImGui::ColorEdit3("Tracer", Settings.Tracer);
            ImGui::ColorEdit3("Aimbot FOV", Settings.AimbotFOV);
            ImGui::ColorEdit3("Admin Warning", Settings.AdminWarning);
            ImGui::ColorEdit3("Enemy Warning", Settings.EnemyWarning);
            ImGui::ColorEdit3("Line Of Sight Warning", Settings.LineOfSightWarning);
            ImGui::ColorEdit3("Mesh Warning", Settings.MeshWarning);
            ImGui::ColorEdit3("Aimbot Target Name", Settings.AimbotTargetName);
            ImGui::ColorEdit3("Ghost Peak Target Name", Settings.GhostPeakTargetName);
            ImGui::ColorEdit3("Tribe Containers", Settings.TribeContainers);
            ImGui::ColorEdit3("Enemy Containers", Settings.EnemyContainers);
            ImGui::ColorEdit3("Tribe Structures", Settings.TribeStructures);
            ImGui::ColorEdit3("Enemy Structures", Settings.EnemyStructures);
            ImGui::ColorEdit3("Tribe Turrets", Settings.TribeTurrets);
            ImGui::ColorEdit3("Enemy Turrets", Settings.EnemyTurrets);
            ImGui::ColorEdit3("Boss Fight", Settings.BossFight);
            ImGui::ColorEdit3("Marker", Settings.Marker);
            
            ImGui::EndTabItem();
        }
        
        if (ImGui::BeginTabItem("Configuration")) {
            ImGui::Text("Configuration");
            ImGui::Separator();
            
            ImGui::InputText("Configuration Name", &Settings.Configuration.ConfigurationName);
            
            if (ImGui::Button("Save")) {
                Settings.Configuration.Save = true;
            }
            if (ImGui::Button("Load")) {
                Settings.Configuration.Load = true;
            }
            if (ImGui::Button("Delete")) {
                Settings.Configuration.Delete = true;
            }
            if (ImGui::Button("Refresh")) {
                Settings.Configuration.Refresh = true;
            }
            
            ImGui::Separator();
            ImGui::Text("Saved Configurations:");
            for (const auto& config : Settings.Configuration.ListOfConfigurations) {
                if (ImGui::Selectable(config.c_str())) {
                    Settings.Configuration.ConfigurationName = config;
                }
            }
            
            ImGui::EndTabItem();
        }
        
        if (ImGui::BeginTabItem("Favourites")) {
            ImGui::Text("Favourited Features");
            ImGui::Separator();
            
            for (const auto& feature : Settings.Favourites.FavouritedFeatures) {
                ImGui::Text(feature.c_str());
            }
            
            ImGui::EndTabItem();
        }
        
        if (ImGui::BeginTabItem("Search")) {
            ImGui::Text("Search Features");
            ImGui::Separator();
            
            ImGui::InputText("Search Feature", &Settings.Search.SearchFeature);
            
            ImGui::EndTabItem();
        }
        
        if (ImGui::BeginTabItem("Misc")) {
            ImGui::Checkbox("Extra Info", &Settings.Misc.ExtraInfo);
            ImGui::Checkbox("Show Players", &Settings.Misc.ShowPlayers);
            ImGui::Checkbox("Show FPS", &Settings.Misc.ShowFPS);
            ImGui::Checkbox("Show XYZ", &Settings.Misc.ShowXYZ);
            ImGui::Checkbox("NoglinAlert", &Settings.Misc.NoglinAlert);
            ImGui::Checkbox("No Sway", &Settings.Misc.NoSway);
            ImGui::Checkbox("No Spread", &Settings.Misc.NoSpread);
            ImGui::Checkbox("No Shake", &Settings.Misc.NoShake);
            ImGui::Checkbox("Speed Hacks", &Settings.Misc.SpeedHacks);
            ImGui::Checkbox("Rapid Fire", &Settings.Misc.RapidFire);
            ImGui::Checkbox("Infinite Orbit", &Settings.Misc.InfiniteOrbit);
            ImGui::Checkbox("Long Arms", &Settings.Misc.LongArms);
            ImGui::Checkbox("Infinite Arms", &Settings.Misc.InfiniteArms);
            ImGui::Checkbox("Auto Armor", &Settings.Misc.AutoArmor);
            ImGui::Checkbox("Instant Dino Turn", &Settings.Misc.InstantDinoTurn);
            
            if (Settings.Misc.SpeedHacks) {
                ImGui::SliderFloat("Speed Multiplier", &Settings.Misc.NewSpeed, 1.0f, 5000.0f);
            }
            
            if (Settings.Misc.AutoArmor) {
                ImGui::SliderFloat("Auto Armor %", &Settings.Misc.AutoArmorPercent, 0.0f, 1.0f);
            }
            
            ImGui::EndTabItem();
        }
        
        ImGui::EndTabBar();
    }
}

// DLL Entry Point
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hModule);
        CreateThread(nullptr, 0, [](LPVOID) -> DWORD {
            InitCheat();
            return 0;
        }, nullptr, 0, nullptr);
        break;
    case DLL_PROCESS_DETACH:
        Logger::Close();
        break;
    }
    return TRUE;
} 