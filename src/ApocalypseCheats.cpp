#include "ApocalypseCheats.h"
#include <cmath>
#include <cfloat>
#include <algorithm>

ApocalypseCheatToggles CheatToggles;

// 1. Flash Step
void ApocalypseCheats::FlashStep() {
    if (!Cache.LocalActor || !Cache.LPC) return;
    FRotator rot = Cache.LPC->PlayerCameraManager->GetCameraRotation();
    float radYaw = rot.Yaw * 3.14159f / 180.0f;
    float dx = std::cos(radYaw) * 300.0f;
    float dy = std::sin(radYaw) * 300.0f;
    FVector pos = Cache.LocalActor->RootComponent->GetWorldLocation();
    pos.X += dx;
    pos.Y += dy;
    Cache.LocalActor->RootComponent->SetWorldLocation(pos);
}

// 2. Bunny Hop Multiplier
static int bunnyHopCount = 0;
static float lastJumpTime = 0.0f;
void ApocalypseCheats::BunnyHopTick() {
    if (!Cache.LocalActor || !Cache.LocalActor->CharacterMovement) return;
    static bool wasJumping = false;
    bool isJumping = Cache.LocalActor->CharacterMovement->IsFalling();
    float now = ImGui::GetTime();
    if (isJumping && !wasJumping) {
        if (now - lastJumpTime < 0.4f) {
            bunnyHopCount = std::min(bunnyHopCount + 1, 5);
        } else {
            bunnyHopCount = 1;
        }
        lastJumpTime = now;
    }
    if (!isJumping) {
        Cache.LocalActor->CharacterMovement->MaxWalkSpeed = 600.0f + 100.0f * bunnyHopCount;
    }
    wasJumping = isJumping;
}

// 3. Water Walker
void ApocalypseCheats::WaterWalkerTick() {
    if (!Cache.LocalActor || !Cache.LocalActor->CharacterMovement) return;
    FVector pos = Cache.LocalActor->RootComponent->GetWorldLocation();
    // Simplified: if Z is near water level, force walking
    float waterLevel = 0.0f; // TODO: Get actual water level
    if (std::abs(pos.Z - waterLevel) < 50.0f) {
        Cache.LocalActor->CharacterMovement->SetMovementMode(MOVE_Walking);
    }
}

// 4. Wall Run Mode
static float wallRunTimer = 0.0f;
void ApocalypseCheats::WallRunTick() {
    if (!Cache.LocalActor || !Cache.LocalActor->CharacterMovement) return;
    // Simplified: if near a wall, apply lateral velocity and suppress gravity
    FVector forward = Cache.LocalActor->GetDirectionVector(
        Cache.LocalActor->RootComponent->GetWorldLocation(),
        Cache.LocalActor->RootComponent->GetWorldLocation() + FVector(100,0,0));
    // TODO: Raycast to detect wall
    bool nearWall = false; // Placeholder
    if (nearWall) {
        wallRunTimer = ImGui::GetTime();
        Cache.LocalActor->CharacterMovement->Velocity = forward * 800.0f;
        Cache.LocalActor->CharacterMovement->GravityScale = 0.1f;
    } else if (ImGui::GetTime() - wallRunTimer < 2.0f) {
        Cache.LocalActor->CharacterMovement->GravityScale = 0.1f;
    } else {
        Cache.LocalActor->CharacterMovement->GravityScale = 1.0f;
    }
}

// 5. Ghost Vaulting
static bool ghostVaultActive = false;
void ApocalypseCheats::GhostVault() {
    if (!Cache.LocalActor) return;
    Cache.LocalActor->SetActorEnableCollision(false);
    ghostVaultActive = true;
    static float vaultStart = ImGui::GetTime();
    vaultStart = ImGui::GetTime();
}
void GhostVaultTick() {
    static float vaultStart = 0.0f;
    if (ghostVaultActive && ImGui::GetTime() - vaultStart > 2.0f) {
        if (Cache.LocalActor) Cache.LocalActor->SetActorEnableCollision(true);
        ghostVaultActive = false;
    }
}

// 6. Spoil Time Freezer
void ApocalypseCheats::SpoilTimeFreezerTick() {
    if (!Cache.LocalActor || !Cache.LocalActor->MyInventoryComponent) return;
    for (int i = 0; i < Cache.LocalActor->MyInventoryComponent->InventoryItems.Count; i++) {
        auto item = Cache.LocalActor->MyInventoryComponent->InventoryItems[i];
        if (item) {
            item->SpoilTime = FLT_MAX;
        }
    }
}

// 7. Auto-Harvest Bot
void ApocalypseCheats::AutoHarvestTick() {
    if (!Cache.LocalActor) return;
    FVector myPos = Cache.LocalActor->RootComponent->GetWorldLocation();
    auto actors = UWorld::GWorld->PersistentLevel->Actors;
    for (int i = 0; i < actors.Count; i++) {
        auto actor = actors[i];
        if (!actor) continue;
        if (actor->IsHarvestable() && myPos.DistTo(actor->RootComponent->GetWorldLocation()) < 200.0f) {
            actor->Use(Cache.LocalActor);
        }
    }
}

// 8. Build Through Walls
static bool buildBypassActive = false;
void ApocalypseCheats::EnableBuildThroughWalls() {
    buildBypassActive = true;
    // TODO: Patch placement validation logic
}
void ApocalypseCheats::DisableBuildThroughWalls() {
    buildBypassActive = false;
    // TODO: Restore placement validation logic
}

// 9. Tether Breaker
void ApocalypseCheats::TetherBreakerTick() {
    if (!Cache.LocalActor) return;
    auto dino = Cache.LocalActor->GetBasedOrSeatingOnDino();
    if (dino) {
        dino->RiderLeashDistance = FLT_MAX;
    }
}

// 10. Stack Multiplier Toggle
static bool stackMultActive = false;
void ApocalypseCheats::EnableStackMultiplier() {
    stackMultActive = true;
    if (!Cache.LocalActor || !Cache.LocalActor->MyInventoryComponent) return;
    for (int i = 0; i < Cache.LocalActor->MyInventoryComponent->InventoryItems.Count; i++) {
        auto item = Cache.LocalActor->MyInventoryComponent->InventoryItems[i];
        if (item) {
            item->MaxStackSize = item->MaxStackSize * 10;
        }
    }
}
void ApocalypseCheats::DisableStackMultiplier() {
    stackMultActive = false;
    if (!Cache.LocalActor || !Cache.LocalActor->MyInventoryComponent) return;
    for (int i = 0; i < Cache.LocalActor->MyInventoryComponent->InventoryItems.Count; i++) {
        auto item = Cache.LocalActor->MyInventoryComponent->InventoryItems[i];
        if (item) {
            item->MaxStackSize = item->MaxStackSize / 10;
        }
    }
} 