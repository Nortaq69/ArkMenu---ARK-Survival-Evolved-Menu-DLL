#pragma once
#include <Windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <vector>
#include <string>
#include <memory>
#include <sstream>
#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include <algorithm>
#include <cmath>
#include <unordered_map>
#include <functional>

// ImGui includes
#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"

// MinHook includes
#include "MinHook.h"

// Forward declarations
struct FVector;
struct FVector2D;
struct FRotator;
struct FString;
class UObject;
class UWorld;
class UPlayer;
class APrimalCharacter;
class APrimalDinoCharacter;
class APrimalTargetableActor;
class USkeletalMeshComponent;
class UPrimalItem;
class UPrimalInventoryComponent;
class AShooterWeapon;
class AShooterWeapon_Rapid;

// Game structures
struct FVector {
    float X, Y, Z;
    
    FVector() : X(0), Y(0), Z(0) {}
    FVector(float x, float y, float z) : X(x), Y(y), Z(z) {}
    
    float DistTo(const FVector& Other) const {
        return sqrt(pow(X - Other.X, 2) + pow(Y - Other.Y, 2) + pow(Z - Other.Z, 2));
    }
    
    FVector operator+(const FVector& Other) const {
        return FVector(X + Other.X, Y + Other.Y, Z + Other.Z);
    }
    
    FVector operator-(const FVector& Other) const {
        return FVector(X - Other.X, Y - Other.Y, Z - Other.Z);
    }
    
    FVector operator*(float Scalar) const {
        return FVector(X * Scalar, Y * Scalar, Z * Scalar);
    }
    
    FVector operator/(float Scalar) const {
        return FVector(X / Scalar, Y / Scalar, Z / Scalar);
    }
};

struct FVector2D {
    float X, Y;
    
    FVector2D() : X(0), Y(0) {}
    FVector2D(float x, float y) : X(x), Y(y) {}
};

struct FRotator {
    float Pitch, Yaw, Roll;
    
    FRotator() : Pitch(0), Yaw(0), Roll(0) {}
    FRotator(float pitch, float yaw, float roll) : Pitch(pitch), Yaw(yaw), Roll(roll) {}
    
    bool hasValue() const { return Pitch != 0 || Yaw != 0 || Roll != 0; }
};

struct FString {
    wchar_t* Data;
    int Count;
    int Max;
    
    std::string ToString() const {
        if (!Data || Count <= 0) return "";
        return std::string(Data, Data + Count);
    }
};

// Game classes (simplified)
class UObject {
public:
    void* VTable;
    int32_t ObjectFlags;
    int32_t InternalIndex;
    class UClass* Class;
    FString Name;
    class UObject* Outer;
};

class UClass : public UObject {
public:
    // Class-specific members
};

class UWorld {
public:
    static UWorld* GWorld;
    class ULevel* PersistentLevel;
    class UGameInstance* OwningGameInstance;
    class UGameState* GameState;
};

class ULevel {
public:
    struct TArray<AActor*> Actors;
};

template<typename T>
struct TArray {
    T* Data;
    int Count;
    int Max;
    
    T* operator[](int Index) const {
        if (Index >= 0 && Index < Count) return &Data[Index];
        return nullptr;
    }
};

class AActor : public UObject {
public:
    class USceneComponent* RootComponent;
    class USkeletalMeshComponent* MeshComponent;
    class UCharacterMovementComponent* CharacterMovement;
    class UPrimalInventoryComponent* MyInventoryComponent;
    class AShooterWeapon* CurrentWeapon;
    class UPrimalCharacterStatusComponent* MyCharacterStatusComponent;
    
    FString PlayerName;
    FString PlatformProfileName;
    FString DinoNameTag;
    FString DescriptiveName;
    FString TribeName;
    
    float ReplicatedCurrentHealth;
    float ReplicatedMaxHealth;
    float ReplicatedCurrentTorpor;
    float ReplicatedMaxTorpor;
    int NumBullets;
    int CurrentItemCount;
    int MaxItemCount;
    
    bool IsPlayer() const;
    bool IsDino() const;
    bool IsLocalPlayer() const;
    bool IsDead() const;
    bool IsConscious() const;
    bool IsTamed() const;
    bool IsFish(const std::string& name) const;
    bool IsManta(const std::string& name) const;
    bool IsAlpha(const std::string& name) const;
    bool IsTurret(const std::string& name, std::string& shortName) const;
    bool IsItemContainer() const;
    bool IsExcludedContainer(const std::string& name) const;
    int RetrievePlayerGender(const std::string& name) const;
    bool IsPrimalCharFriendly(APrimalCharacter* Other) const;
    class APrimalDinoCharacter* GetBasedOrSeatingOnDino() const;
    FVector GetDirectionVector(const FVector& From, const FVector& To) const;
    FRotator FindLookAtRotation(const FVector& From, const FVector& To) const;
    class FPrimalPlayerDataStruct* GetPlayerData() const;
};

class APrimalCharacter : public AActor {
public:
    float ExtraMaxSpeedModifier;
    float RunningSpeedModifier;
    float AdditionalMaxUseDistance;
    float OrbitCamMaxZoomLevel;
};

class APrimalDinoCharacter : public APrimalCharacter {
public:
    float RiderMovementSpeedScalingRotationRatePowerMultiplier;
    float WalkingRotationRateModifier;
    float RiderFlyingRotationRateModifier;
};

class AShooterWeapon : public AActor {
public:
    float AimDriftPitchFrequency;
    float AimDriftYawFrequency;
    float bUseFireCameraShakeScale;
    float GlobalFireCameraShakeScale;
    float ReloadCameraShakeSpeedScale;
    float GlobalFireCameraShakeScaleTargeting;
    float LastFireTime;
    float LastNotifyShotTime;
    
    struct WeaponConfig {
        float TimeBetweenShots;
        float WeaponSpread;
        float TargetingSpreadMod;
    } WeaponConfig, InstantConfig;
    
    FVector* GetAdjustedAim(FVector* Result);
};

class AShooterWeapon_Rapid : public AShooterWeapon {
public:
    // Rapid fire specific members
};

class UPlayer {
public:
    class APlayerController* PlayerController;
};

class APlayerController : public AActor {
public:
    class APlayerCameraManager* PlayerCameraManager;
    FRotator ControlRotation;
    
    FVector GetCameraLocation() const;
    FRotator GetCameraRotation() const;
    void SetControlRotation(const FRotator& Rotation);
    bool IsInputKeyDown(const std::string& KeyName) const;
    void IgnoreLookInput(bool bIgnore);
    bool LineOfSightTo(AActor* Actor, const FVector& ViewLocation, bool bAlternateChecks);
    void EquipPawnItem(int ItemId);
};

class APlayerCameraManager : public AActor {
public:
    float DefaultFOV;
    
    FVector GetCameraLocation() const;
    FRotator GetCameraRotation() const;
};

class UCharacterMovementComponent : public UObject {
public:
    FVector Velocity;
    float Acceleration;
    float MaxFlySpeed;
};

class USkeletalMeshComponent : public UObject {
public:
    std::string GetBoneName(int BoneIndex) const;
};

class UPrimalItem : public UObject {
public:
    FString DescriptiveNameBase;
    int ItemId;
    float Durability;
    float MaxDurability;
    
    enum EquipmentType {
        Hat,
        Shirt,
        Gloves,
        Pants,
        Boots
    } MyEquipmentType;
    
    enum EPrimalItemStats {
        MaxDurability
    };
    
    bool IsBroken() const;
    bool CanDrop() const;
    bool CanUse(bool bIgnoreDurability) const;
    float GetDurabilityPercentage() const;
    float GetItemStatModifier(EPrimalItemStats Stat) const;
    FString GetDurabilityStringShort() const;
};

class UPrimalInventoryComponent : public UObject {
public:
    TArray<UPrimalItem*> InventoryItems;
    TArray<UPrimalItem*> EquippedItems;
    
    UPrimalItem* GetEquippedItemOfType(UPrimalItem::EquipmentType Type) const;
};

class UPrimalCharacterStatusComponent : public UObject {
public:
    int BaseCharacterLevel;
    int ExtraCharacterLevel;
};

class FPrimalPlayerDataStruct {
public:
    // Player data structure
};

// Settings structure
struct Settings {
    struct {
        bool DrawPlayers = true;
        bool DrawPlayerDistance = true;
        bool DrawPlayerHP = true;
        bool DrawPlayerBones = false;
        bool DrawPlayerTorp = false;
        bool DrawDeadPlayers = false;
        bool DrawSleepingPlayers = false;
        bool HideSelf = false;
        bool HideTeamPlayers = false;
        bool RenderPlayerName = true;
        bool ShowWeapons = false;
        
        // Advanced Player ESP
        bool DrawLocalPlayer = false;
        bool DrawConsciousTribePlayers = true;
        bool DrawSleepingTribePlayers = false;
        bool DrawDeadTribePlayers = false;
        bool DrawConsciousEnemyPlayers = true;
        bool DrawSleepingEnemyPlayers = false;
        bool DrawDeadEnemyPlayers = false;
        bool DrawLine = false;
        bool DrawLineTopScreen = false;
        bool DrawMuzzleLine = false;
        bool DrawLineConsciousTribePlayer = false;
        bool DrawLineSleepingTribePlayer = false;
        bool DrawLineDeadTribePlayer = false;
        bool DrawLineConsciousEnemyPlayer = false;
        bool DrawLineSleepingEnemyPlayer = false;
        bool DrawLineDeadEnemyPlayer = false;
        bool DrawGamertag = true;
        bool DrawName = true;
        bool DrawTribe = true;
        bool DrawLevel = true;
        bool DrawHealth = true;
        bool DrawTorpor = true;
        bool DrawWeight = false;
        bool DrawDistance = true;
        bool DrawSkeleton = false;
        bool DrawBox = false;
        bool DrawArmor = false;
        bool DrawWeapon = false;
        bool DrawRadarLocalPlayer = false;
        bool DrawRadarConsciousTribePlayer = false;
        bool DrawRadarSleepingTribePlayer = false;
        bool DrawRadarDeadTribePlayer = false;
        bool DrawRadarConsciousEnemyPlayer = false;
        bool DrawRadarSleepingEnemyPlayer = false;
        bool DrawRadarDeadEnemyPlayer = false;
        
        bool DrawWildCreatures = true;
        bool DrawTamedCreatures = true;
        bool DrawDinoDistance = true;
        bool DrawDinoHP = true;
        bool DrawSleepingDinos = false;
        bool DinoDead = false;
        bool HideTeamDinos = false;
        bool WildDinoTorp = false;
        bool TamedDinoTorp = false;
        bool WildDinoFilter = false;
        bool AlphaFilter = false;
        bool HideFish = false;
        bool isBee = false;
        bool hideManta = false;
        
        // Advanced Dino ESP
        bool DrawConsciousTribeDinos = true;
        bool DrawSleepingTribeDinos = false;
        bool DrawDeadTribeDinos = false;
        bool DrawConsciousEnemyDinos = true;
        bool DrawSleepingEnemyDinos = false;
        bool DrawDeadEnemyDinos = false;
        bool DrawWildDinos = true;
        bool DrawTamedName = true;
        bool DrawTamedTribe = true;
        bool DrawTamedLevel = true;
        bool DrawTamedHealth = true;
        bool DrawTamedTorpor = true;
        bool DrawTamedDistance = true;
        bool DrawTamedGender = false;
        bool DrawTamedAggression = false;
        bool HideFish = false;
        bool WildAlphaFilter = false;
        bool WildLevelFilter = false;
        bool DinoChams = false;
        bool DrawWildName = true;
        bool DrawWildLevel = true;
        bool DrawWildHealth = true;
        bool DrawWildTorpor = true;
        bool DrawWildDistance = true;
        bool DrawWildGender = false;
        int DinoFilter = 1;
        
        bool DrawTurrets = true;
        bool HideTeamTurrets = false;
        bool ShowEmptyTurrets = false;
        bool ShowBulletCount = false;
        
        // Advanced Turret ESP
        bool DrawTribeTurrets = true;
        bool DrawEnemyTurrets = true;
        bool TurretChams = false;
        bool DrawTurretName = true;
        bool DrawTurretDistance = true;
        bool DrawBulletCount = false;
        bool DrawShotCount = false;
        bool DrawTargetSetting = false;
        bool DrawRangeSetting = false;
        bool DrawWarningSetting = false;
        bool DrawEmptyTurrets = false;
        
        bool DrawContainers = true;
        bool HideTeamContainers = false;
        bool ShowEmptyContainers = false;
        bool ContainerFilter = false;
        
        // Advanced Container ESP
        bool DrawTribeContainers = true;
        bool DrawEnemyContainers = true;
        bool DrawTribeStructures = true;
        bool DrawEnemyStructures = true;
        bool ContainerChams = false;
        bool DrawContainerName = true;
        bool DrawContainerDistance = true;
        bool DrawContainerItemCount = true;
        bool DrawContainerHealth = true;
        bool IgnoreManequins = true;
        bool BeaverDamOnly = false;
        bool DrawSupplyLootCrates = true;
        bool ShowEmptyContainers = false;
        bool StructureChams = false;
        bool GeneratorChams = false;
        bool DrawStructureName = true;
        bool DrawStructureHealth = true;
        bool DrawStructureDistance = true;
        int ContainerFilter = 1;
        
        // Other ESP
        bool DrawBossFights = false;
        bool DrawDroppedItems = true;
        bool DrawElementVein = true;
        bool CreateMarker = false;
        bool ClearMarker = false;
        bool ExplorerNoteEsp = false;
        bool HideUnlockedExplorerNotes = true;
        bool DrawExplorerName = true;
        bool DrawExplorerDistance = true;
        bool SelfArmorESP = false;
        bool SelfWeaponESP = false;
        bool SelfHealthESP = false;
        float SelfHealthESPHeight = 100.0f;
        bool SelfReloadCooldown = false;
        bool ChamWireFrame = false;
        float ChamOpacity = 1.0f;
        bool DrawBossName = true;
        bool DrawDroppedItemName = true;
        bool DrawDroppedItemDistance = true;
        bool DrawDroppedItemOwnerName = true;
        bool DrawOSDName = true;
        bool DrawOSDHealth = true;
        bool DrawOSDItemCount = true;
        bool DrawOSDDistance = true;
        bool DrawElementVeinName = true;
        bool DrawElementVeinHealth = true;
        bool DrawElementVeinItemCount = true;
        bool DrawElementVeinDistance = true;
        bool EnableMarkers = false;
        bool ClearAllMarkers = false;
        bool ShowMarkerName = true;
        bool DrawMarkerDistance = true;
        
        bool DrawCrosshair = true;
        bool DrawAimFOV = true;
        float CrosshairSize = 10.0f;
        float CrosshairWidth = 2.0f;
        float FOVSize = 100.0f;
    } Visuals;
    
    struct {
        bool EnableAimbot = false;
        bool SilentAim = false;
        bool TargetTribe = false;
        bool TargetSleepers = false;
        bool VisibleOnly = false;
        bool AimLocked = false;
        std::string AimbotKey = "RButton";
        
        // Advanced Aimbot Features
        bool WhipBot = false;
        bool GhostPeak = false;
        bool ManagarmarSilent = false;
        bool DesertTitanSilent = false;
        bool TriggerBot = false;
        bool TargetEnemy = true;
        bool TargetDino = false;
        bool TargetFjordHawk = false;
        
        // Tek Saddle Aimbot
        bool TribeTekSaddleAimbot = false;
        bool EnemyTekSaddleAimbot = false;
        bool TribeTekSaddleSilent = false;
        bool EnemyTekSaddleSilent = false;
        bool TargetTekGenerator = false;
        bool TargetGasGenerator = false;
        bool TargetTekTurret = false;
        bool TargetTekFoundation = false;
        bool TargetMetalFoundation = false;
        bool TargetTekTriangleFoundation = false;
        bool TargetMetalTriangleFoundation = false;
        bool TargetMetalCliffPlatform = false;
        bool TargetStoneCliffPlatform = false;
        bool TargetTekDoubleDoorFrame = false;
        bool TargetMetalDoubleDoorFrame = false;
        bool TargetTekHatchFrame = false;
        bool TargetMetalHatchFrame = false;
        
        // Target Bone
        bool TargetHead = true;
        bool TargetChest = false;
        bool TargetPelvis = false;
        bool TargetLegs = false;
        bool TargetHands = false;
        bool TargetFeet = false;
        bool TargetMix = false;
        bool CycleBone = false;
        
        float AimFOV = 100.0f;
    } Aimbot;
    
    struct {
        bool ExtraInfo = true;
        bool ShowPlayers = true;
        bool ShowFPS = true;
        bool ShowXYZ = true;
        bool NoglinAlert = true;
        bool NoSway = false;
        bool NoSpread = false;
        bool NoShake = false;
        bool SpeedHacks = false;
        bool RapidFire = false;
        bool InfiniteOrbit = false;
        bool LongArms = false;
        bool InfiniteArms = false;
        bool AutoArmor = false;
        bool InstantDinoTurn = false;
        float NewSpeed = 1000.0f;
        float AutoArmorPercent = 0.3f;
        
        // Weapon Modifications
        bool SelfC4 = false;
        bool InfiniteC4 = false;
        bool UnlockWeaponDinoRotation = false;
        bool NoOverHeat = false;
        bool InstantShotgunReload = false;
        bool AutoSwapFabi = false;
        bool AutoSwapAR = false;
        bool NoScopeOverlay = false;
        bool NoTekRifleOverlay = false;
        bool ShieldBypass = false;
        bool NoSpread = false;
        bool NoSway = false;
        bool NoRecoil = false;
        bool AimWhileReloading = false;
        bool AllowMountedWeapon = false;
        bool InfiniteWeaponRange = false;
        bool TekRifleHitIndicator = false;
        bool ConserveShots = false;
        bool WeaponSpyglass = false;
        bool RunningWeapon = false;
        bool NoWeaponUnEquip = false;
        bool QuickReloadFabricatedSniper = false;
        bool NoReloadShake = false;
        bool Tracers = false;
        bool RapidFire = false;
        bool AdaptiveRapidFire = false;
        bool RocketSpam = false;
        bool BowSpam = false;
        float WeaponSize = 1.0f;
        float ExtraScopeZoom = 1.0f;
        bool BetterChainsaw = false;
        bool NetSpam = false;
        bool CryopodSpam = false;
        bool TekRifleSpam = false;
        bool TekLauncherSpam = false;
        bool RailgunSpam = false;
        
        // Character Modifications
        bool TekPunch = false;
        bool TekChest = false;
        bool TekSwordFly = false;
        float PlayerChargeSpeed = 1.0f;
        bool UnlockExplorerNotes = false;
        bool UnlockEngrams = false;
        bool AirStruck = false;
        bool UploadInventoryToArk = false;
        bool DownloadInventoryFromArk = false;
        bool AutoMount = false;
        bool AutoHatchEgg = false;
        bool AutoDestroyEgg = false;
        bool RemoveBoladTribemates = false;
        bool TekGaunletAirStuck = false;
        bool LongArms = false;
        bool InfiniteElement = false;
        bool InstaTurn = false;
        bool InfiniteOrbit = false;
        bool GamertagSpoof = false;
        bool GamertagSpam = false;
        bool GhostMode = false;
        bool AutoBrew = false;
        float CharacterSize = 1.0f;
        bool AntiHit = false;
        bool Spinbot = false;
        float PlayerSpeed = 1.0f;
        float FOVChanger = 90.0f;
        bool Desync = false;
        bool GamerscoreUnlocker = false;
        bool AutoArmor = false;
        bool PrioritizeTekHelmet = false;
        bool PrioritizeTekChest = false;
        bool PrioritizeTekLeggings = false;
        bool PrioritizeTekGauntlets = false;
        bool PrioritizeTekBoots = false;
        bool PrioritizeTekShield = false;
        
        // Dino Modifications
        bool BackwardsDinoMovement = false;
        bool InstaTurnDino = false;
        float DinoChargeSpeed = 1.0f;
        bool AutoClaim = false;
        bool AutoImprint = false;
        bool InstantMagmasaurFire = false;
        bool DesertTitanLightning = false;
        bool DesertTitanInfiniteCharge = false;
        bool AutoFeedBabys = false;
        bool AutoFeedDaedons = false;
        bool AutoFeedMountedDino = false;
        bool AutoNameDinos = false;
        bool AutoNameBabys = false;
        float AutoNameRadius = 100.0f;
        float DinoSpeed = 1.0f;
        float DinoSpeedLatency = 1.0f;
        bool DeinoJump = false;
        bool DeinoFly = false;
        float MaewingSpeed = 1.0f;
        bool InfiniteDinoDive = false;
        bool ManagarmrDash = false;
        bool SuperAndrewJump = false;
        bool AutoLevelHealth = false;
        bool AutoLevelStamina = false;
        bool AutoLevelOxygen = false;
        bool AutoLevelFood = false;
        bool AutoLevelWeight = false;
        bool AutoLevelMelee = false;
        bool AutoLevelSpeed = false;
        
        // Extra Features
        bool PlusCrosshair = false;
        bool CircleCrosshair = false;
        float CrosshairSize = 10.0f;
        float CrosshairWidth = 2.0f;
        bool PickupTurrets = false;
        bool PickupBearTraps = false;
        bool PickupC4 = false;
        bool PopCornGenerators = false;
        bool PopCornTurrets = false;
        bool DemolishGrinders = false;
        bool AutoCraftARB = false;
        bool AutoCraftGB = false;
        bool AutoCraftSP = false;
        bool AutoFoundationToBlock = false;
        bool AutoFoundationToDefault = false;
        bool WildCardAdminChat = false;
        bool ServerAdminChat = false;
        bool ArkDeveloperChat = false;
        bool TransmitterBag = false;
        bool Dupe = false;
        bool ClickDupe = false;
        bool ForceUpload = false;
        bool RemoveUploadDownloadScreen = false;
        bool AutoPickUpDroppedItems = false;
        bool DamageNumbers = false;
        bool MeshWarning = false;
        bool LineOfSightWarning = false;
        bool GammaGen2Boss = false;
        bool BetaGen2Boss = false;
        bool AlphaGen2Boss = false;
        bool ForceOpenHexagonShop = false;
        bool BetterUploadDownload = false;
        bool GhostPeakTargetName = false;
        bool AimbotTargetName = false;
        bool AutoLootContainers = false;
        bool AutoLootPlayers = false;
        
        // Extra 2
        bool PanicMode = false;
        bool ChatLog = false;
        bool PotatoeINI = false;
        bool HardINI = false;
        bool NoLandscapeINI = false;
        bool NoRocksAndTrees = false;
        bool NoWaterINI = false;
        bool NoParticleINI = false;
        bool FullBrightINI = false;
        bool NoXYZ = false;
        bool NoRespawnAnimation = false;
        bool CreateCharacter = false;
        bool SmallCharacter = false;
        bool FemaleCharacter = false;
        bool CustomHairColor = false;
        bool CustomBodyColor = false;
        std::string CharacterName = "Player";
        bool ResourcePull = false;
        bool AutoCraftSelected = false;
        bool ForceOpenInventory = false;
        bool UnlockGen1Glitches = false;
        bool TurnOnTekLights = false;
        float AimFov = 100.0f;
        bool AdminCracker = false;
        bool PinCracker = false;
        bool FreeCamera = false;
        bool CustomKill = false;
        bool DiscordTribeLog = false;
        bool ChatSpammer = false;
        bool DinoDeath = false;
        std::string DinoName = "Dino";
        bool EnemyWarning = false;
        bool AdminWarning = false;
        float WarningSize = 1.0f;
        float ESPSize = 1.0f;
        float ESPIconSize = 1.0f;
    } Misc;
    
    struct {
        std::string AimbotKey = "RButton";
        std::string AirStruckKey = "F1";
        std::string AutoLootKey = "F2";
        std::string TekPunchKey = "F3";
        std::string TekChestKey = "F4";
        std::string TekSwordFlyKey = "F5";
        std::string GhostModeKey = "F6";
        std::string InfiniteDinoDiveKey = "F7";
        std::string PlayerSpeedKey = "F8";
        std::string DinoSpeedKey = "F9";
        std::string RocketSpamKey = "F10";
        std::string BowSpamKey = "F11";
        std::string UploadKey = "F12";
        std::string DesyncKey = "Insert";
        std::string UploadInventoryKey = "Home";
        std::string DownloadInventoryKey = "PageUp";
        std::string PlayerChargeSpeedKey = "PageDown";
        std::string DinoChargeSpeedKey = "End";
        std::string GhostPeakKey = "Delete";
        std::string CryopodSpamKey = "NumPad0";
        std::string NetSpamKey = "NumPad1";
        std::string DeinoFlyKey = "NumPad2";
        std::string ForceOpenKey = "NumPad3";
        std::string RailgunSpamKey = "NumPad4";
        std::string TekRifleKey = "NumPad5";
        std::string TekGauntletAirStuckKey = "NumPad6";
        std::string AutoMountKey = "NumPad7";
        std::string WhipBotKey = "NumPad8";
        
        // Keybinds 2
        std::string CreateMarkerKey = "NumPad9";
        std::string AutoLevelKey = "Add";
        std::string BoneCycleKey = "Subtract";
        std::string DupeKey = "Multiply";
        std::string ClickDupeKey = "Divide";
        std::string InstaTurnKey = "Decimal";
        std::string SilentAimKey = "Separator";
        std::string AntiHitKey = "NumPadEnter";
        std::string PanicKey = "Escape";
        std::string MenuKey = "Insert";
    } Keybinds;
    
    struct {
        bool HudInformation = true;
        bool ShowFPS = true;
        bool ShowAimKey = true;
        bool ShowAimBone = true;
        bool ShowCords = true;
        bool ShowPing = true;
        bool ShowServerFPS = true;
        bool ShowServerName = true;
        bool ShowMapName = true;
        bool ShowConnectedPlayers = true;
        bool ShowTamedDinos = true;
        bool ShowMaxTamedDinos = true;
        bool ShowTurretsLimit = true;
        bool ShowTurretsLimitRange = true;
        bool ShowStructureLimit = true;
        bool ShowDayNumber = true;
        bool ShowDayTime = true;
        std::string AdminCrackerPassword = "";
        std::string PinCrackerPassword = "";
        bool ShowToggleKeys = true;
        std::string DiscordInvite = "";
        bool RainBowInfo = false;
    } HudDisplay;
    
    struct {
        std::string ConfigurationName = "Default";
        std::vector<std::string> ListOfConfigurations;
        bool Load = false;
        bool Save = false;
        bool Delete = false;
        bool Refresh = false;
    } Configuration;
    
    struct {
        std::vector<std::string> FavouritedFeatures;
    } Favourites;
    
    struct {
        std::string SearchFeature = "";
    } Search;
    
    // Colors
    float PlayerColor1[3] = {1.0f, 0.0f, 0.0f};
    float TamedDinoColor1[3] = {0.0f, 1.0f, 0.0f};
    float WildDinoColor1[3] = {1.0f, 1.0f, 0.0f};
    float FilteredDinoColor1[3] = {0.7f, 0.0f, 0.5f};
    float TurretColor1[3] = {1.0f, 0.5f, 0.0f};
    float ContainerColor1[3] = {0.5f, 0.5f, 1.0f};
    float CrosshairColor1[3] = {1.0f, 1.0f, 1.0f};
    
    // Advanced Colors
    float ConsciousTribePlayers[3] = {0.0f, 1.0f, 0.0f};
    float SleepingTribePlayers[3] = {0.0f, 0.5f, 0.0f};
    float DeadTribePlayers[3] = {0.5f, 0.5f, 0.5f};
    float ConsciousEnemyPlayers[3] = {1.0f, 0.0f, 0.0f};
    float SleepingEnemyPlayers[3] = {0.5f, 0.0f, 0.0f};
    float DeadEnemyPlayers[3] = {0.3f, 0.3f, 0.3f};
    float ConsciousTribeDinos[3] = {0.0f, 1.0f, 0.0f};
    float SleepingTribeDinos[3] = {0.0f, 0.5f, 0.0f};
    float DeadTribeDinos[3] = {0.5f, 0.5f, 0.5f};
    float ConsciousEnemyDinos[3] = {1.0f, 0.0f, 0.0f};
    float SleepingEnemyDinos[3] = {0.5f, 0.0f, 0.0f};
    float DeadEnemyDinos[3] = {0.3f, 0.3f, 0.3f};
    float WildDinos[3] = {1.0f, 1.0f, 0.0f};
    float ContainerTribe[3] = {0.0f, 1.0f, 0.0f};
    float ContainerEnemy[3] = {1.0f, 0.0f, 0.0f};
    float CharacterBody[3] = {1.0f, 1.0f, 1.0f};
    float EnemyHair[3] = {1.0f, 0.8f, 0.0f};
    float HudInformation[3] = {1.0f, 1.0f, 1.0f};
    float CrossHair[3] = {1.0f, 1.0f, 1.0f};
    float Tracer[3] = {1.0f, 1.0f, 1.0f};
    float AimbotFOV[3] = {1.0f, 0.0f, 0.0f};
    float AdminWarning[3] = {1.0f, 0.0f, 0.0f};
    float EnemyWarning[3] = {1.0f, 0.5f, 0.0f};
    float LineOfSightWarning[3] = {1.0f, 1.0f, 0.0f};
    float MeshWarning[3] = {0.5f, 0.0f, 1.0f};
    float AimbotTargetName[3] = {1.0f, 0.0f, 0.0f};
    float GhostPeakTargetName[3] = {0.0f, 1.0f, 1.0f};
    float TribeContainers[3] = {0.0f, 1.0f, 0.0f};
    float EnemyContainers[3] = {1.0f, 0.0f, 0.0f};
    float TribeStructures[3] = {0.0f, 1.0f, 0.0f};
    float EnemyStructures[3] = {1.0f, 0.0f, 0.0f};
    float TribeTurrets[3] = {0.0f, 1.0f, 0.0f};
    float EnemyTurrets[3] = {1.0f, 0.0f, 0.0f};
    float BossFight[3] = {1.0f, 0.0f, 1.0f};
    float Marker[3] = {0.0f, 1.0f, 1.0f};
    
    // Menu settings
    bool IsMenuOpen = false;
    float MenuSizeX = 800.0f;
    float MenuSizeY = 600.0f;
    
    // Offsets
    uintptr_t uworld = 0x173CA60;
    
    // Function pointers
    typedef bool(*GetBoneLocationFunc)(USkeletalMeshComponent*, FVector*, const std::string&, int);
    typedef bool(*InputKeyFunc)(APlayerController*, const std::string&);
    
    GetBoneLocationFunc GetBoneLocation = nullptr;
    GetBoneLocationFunc OriginalGetBoneLocation = nullptr;
    InputKeyFunc InputKey = nullptr;
    InputKeyFunc OriginalInputKey = nullptr;
    
    // PE Hook
    typedef void(*PEFunc)(void*, void*, void*);
    PEFunc OriginalPE = nullptr;
} Settings;

// Cache structure
struct Cache {
    uintptr_t GameBase = 0;
    UWorld* GWorld = nullptr;
    UPlayer* LocalPlayer = nullptr;
    APlayerController* LPC = nullptr;
    AActor* LocalActor = nullptr;
    APrimalCharacter* LPFOV = nullptr;
    FVector LocalLocation;
    int NearbyEnemies = 0;
    float WindowSizeX = 0;
    float WindowSizeY = 0;
    
    // Aimbot
    AActor* AimbotTarget = nullptr;
    
    // Auto armor
    struct {
        std::vector<uintptr_t> FlakHelmets, FlakChestplates, FlakGloves, FlakPants, FlakBoots;
        std::vector<uintptr_t> TekHelmets, TekChestplates, TekGloves, TekPants, TekBoots;
        std::vector<uintptr_t> RiotHelmets, RiotChestplates, RiotGloves, RiotPants, RiotBoots;
    } AutoArmor;
} Cache;

// Global variables
extern bool NearbyNoglin;
extern std::string CompareName;
extern std::string CurrentContainer;
extern int PossibleDinos;
extern int AimBone;

// Function declarations
bool W2S(const FVector& WorldLocation, FVector2D& ScreenLocation);
void* PatternScan(uintptr_t Base, const char* Pattern);
void* GetD3D11PresentFunction();
bool SetHook(void* Target, void* Detour, void** Original);
bool RemoveHook(void* Original);

// Logger
class Logger {
public:
    static bool Init(const wchar_t* FilePath);
    static void Log(const char* Format, ...);
    static void Close();
private:
    static std::ofstream LogFile;
};

// Renderer class
class Renderer {
public:
    struct {
        ID3D11Device* Device = nullptr;
        ID3D11DeviceContext* Ctx = nullptr;
        IDXGISwapChain* SwapChain = nullptr;
        ID3D11RenderTargetView* RenderTargetView = nullptr;
        HWND GameWindow = nullptr;
        
        void* PresentFunc = nullptr;
        void* OriginalPresent = nullptr;
        void* SetCursorPosOriginal = nullptr;
        void* SetCursorOriginal = nullptr;
        WNDPROC WndProcOriginal = nullptr;
    } D3D;
    
    static bool Init();
    static bool Remove();
    static void RemoveInput();
    static void HookInput();
    static bool CreateView();
    static void setupImGui();
    static void setupImGuiStyle(ImGuiStyle* Style, ImGuiIO& IO, ImFontConfig& Config);
    static void setColorSettings(ImVec4* colors);
    static void renderFrame();
    static HRESULT D3D_HOOK(IDXGISwapChain* SwapChain, UINT SyncInterval, UINT Flags);
    
    class Drawing {
    public:
        static void RenderText(ImVec2 ScreenPosition, ImColor Color, const char* Text, int WidthText);
        static void RenderText2(ImVec2 ScreenPosition, ImColor Color, const char* Text, int WidthText);
        static void RenderCollapseFriendlyDisplayList(ImVec2 ScreenStartPosition, ImColor Color, std::vector<std::string> DisplayArray, float HeightFactor);
        static void RenderCrosshair(ImColor Color, int Thickness);
        static void RenderAimFOV(ImColor Color);
        static int ReturnDistance(int X1, int Y1, int X2, int Y2);
        static bool WithinAimFOV(int CircleX, int CircleY, int R, int X, int Y);
        static bool RenderPlayerSkeleton(USkeletalMeshComponent* Mesh, int Gender, ImColor Color);
    };
};

// Utils class
class Utils {
public:
    static void PE_HOOK(void* obj, void* fn, void* params);
    static FVector* GetAdjustedAim(AShooterWeapon* Weapon, FVector* Result);
};

// Menu functions
void decorations();
void tabss();
void AimbotThread();
void InitCheat();

// External variables
extern ImFont* zzzz;
extern ImFont* icons;
extern unsigned char iconfont[]; 