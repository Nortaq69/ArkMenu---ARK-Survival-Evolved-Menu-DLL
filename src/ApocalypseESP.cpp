#include "ApocalypseESP.h"
#include <algorithm>
#include <sstream>
#include <iomanip>

// Global instances
ApocalypseSettings ApocalypseESP;
std::vector<ESPTag> activeTags;
std::vector<AudioVisualization> activeAudioEvents;

// Static members
std::unordered_map<std::string, PhantomActor> EntityMemory::phantomActors;
std::deque<MovementData> EntityMemory::globalMovementHistory;
std::unordered_map<std::string, DinoDNA> DinoAnalyzer::dnaCache;
std::unordered_map<std::string, PredictionData> PredictionSystem::predictions;
std::unordered_map<std::string, int> PlayerRelations::playerKarma;
std::vector<std::pair<std::string, std::string>> PlayerRelations::karmaEvents;
std::vector<AudioVisualization> SoundVisualizer::audioEvents;
std::vector<ImColor> StyleManager::heatmapColors;

// ============================================================================
// VISUAL OVERLAY SYSTEM
// ============================================================================

void VisualOverlaySystem::RenderESPGraffiti() {
    if (!ApocalypseESP.Graffiti.enabled) return;
    
    auto currentTime = ImGui::GetTime();
    auto drawList = ImGui::GetBackgroundDrawList();
    
    // Clean up old tags
    activeTags.erase(
        std::remove_if(activeTags.begin(), activeTags.end(),
            [&](const ESPTag& tag) {
                return !tag.persistent && (currentTime - tag.timestamp) > ApocalypseESP.Graffiti.tagLifetime;
            }),
        activeTags.end()
    );
    
    // Render all active tags
    for (const auto& tag : activeTags) {
        RenderTag(tag);
    }
}

void VisualOverlaySystem::RenderTag(const ESPTag& tag) {
    auto drawList = ImGui::GetBackgroundDrawList();
    auto currentTime = ImGui::GetTime();
    
    // Calculate alpha based on age
    float alpha = 1.0f;
    if (!tag.persistent) {
        alpha = 1.0f - ((currentTime - tag.timestamp) / ApocalypseESP.Graffiti.tagLifetime);
        alpha = std::max(0.0f, alpha);
    }
    
    ImColor finalColor = tag.color;
    finalColor.Value.w = alpha;
    
    // Render tag background
    ImVec2 textSize = ImGui::CalcTextSize(tag.label.c_str());
    ImVec2 bgMin = ImVec2(tag.position.x - 2, tag.position.y - 2);
    ImVec2 bgMax = ImVec2(tag.position.x + textSize.x + 2, tag.position.y + textSize.y + 2);
    
    drawList->AddRectFilled(bgMin, bgMax, ImColor(0, 0, 0, alpha * 0.7f), 3.0f);
    drawList->AddRect(bgMin, bgMax, finalColor, 3.0f, 0, 2.0f);
    
    // Render tag text
    drawList->AddText(tag.position, finalColor, tag.label.c_str());
    
    // Render timestamp if enabled
    if (ApocalypseESP.Graffiti.showTimestamps) {
        std::string timeStr = ApocalypseUtils::FormatTimeAgo(tag.timestamp);
        ImVec2 timePos = ImVec2(tag.position.x, tag.position.y + textSize.y + 2);
        drawList->AddText(timePos, ImColor(255, 255, 255, alpha * 0.8f), timeStr.c_str());
    }
}

void VisualOverlaySystem::RenderSatelliteView() {
    if (!ApocalypseESP.SatelliteView.enabled) return;
    
    auto drawList = ImGui::GetBackgroundDrawList();
    ImVec2 screenSize = ImGui::GetIO().DisplaySize;
    ImVec2 radarCenter = ImVec2(screenSize.x - ApocalypseESP.SatelliteView.radarSize - 20, 
                                ApocalypseESP.SatelliteView.radarSize + 20);
    
    // Render radar background
    ImVec2 radarMin = ImVec2(radarCenter.x - ApocalypseESP.SatelliteView.radarSize, 
                             radarCenter.y - ApocalypseESP.SatelliteView.radarSize);
    ImVec2 radarMax = ImVec2(radarCenter.x + ApocalypseESP.SatelliteView.radarSize, 
                             radarCenter.y + ApocalypseESP.SatelliteView.radarSize);
    
    drawList->AddCircleFilled(radarCenter, ApocalypseESP.SatelliteView.radarSize, 
                             ImColor(0, 0, 0, 0.8f));
    drawList->AddCircle(radarCenter, ApocalypseESP.SatelliteView.radarSize, 
                       ImColor(255, 255, 255, 0.5f), 0, 2.0f);
    
    // Render radar grid
    for (int i = 1; i <= 4; i++) {
        float radius = (ApocalypseESP.SatelliteView.radarSize / 4.0f) * i;
        drawList->AddCircle(radarCenter, radius, ImColor(255, 255, 255, 0.2f), 0, 1.0f);
    }
    
    // Render player position (center dot)
    drawList->AddCircleFilled(radarCenter, 3.0f, ImColor(0, 255, 0, 255));
    
    // Render entities on radar
    if (Cache.LocalActor) {
        FVector playerPos = Cache.LocalActor->RootComponent->GetWorldLocation();
        
        // Render dinos
        if (ApocalypseESP.SatelliteView.showDinos) {
            auto actors = UWorld::GWorld->PersistentLevel->Actors;
            for (int i = 0; i < actors.Count; i++) {
                auto actor = actors[i];
                if (actor && actor->IsDino()) {
                    ImVec2 radarPos = ApocalypseUtils::WorldToRadarPosition(
                        actor->RootComponent->GetWorldLocation(), 
                        radarCenter, 
                        ApocalypseESP.SatelliteView.radarSize, 
                        ApocalypseESP.SatelliteView.zoomLevel
                    );
                    
                    if (actor->IsTamed()) {
                        RenderRadarIcon(actor->RootComponent->GetWorldLocation(), "dino", ImColor(0, 255, 0, 255));
                    } else {
                        RenderRadarIcon(actor->RootComponent->GetWorldLocation(), "dino", ImColor(255, 255, 0, 255));
                    }
                }
            }
        }
        
        // Render players
        if (ApocalypseESP.SatelliteView.showPlayers) {
            auto actors = UWorld::GWorld->PersistentLevel->Actors;
            for (int i = 0; i < actors.Count; i++) {
                auto actor = actors[i];
                if (actor && actor->IsPlayer() && actor != Cache.LocalActor) {
                    RenderRadarIcon(actor->RootComponent->GetWorldLocation(), "player", ImColor(255, 0, 0, 255));
                }
            }
        }
    }
}

void VisualOverlaySystem::RenderPsychoVision() {
    if (!ApocalypseESP.PsychoVision.enabled) return;
    
    auto drawList = ImGui::GetBackgroundDrawList();
    auto currentTime = ImGui::GetTime();
    
    // Render movement trails for all tracked entities
    for (const auto& phantom : EntityMemory::GetPhantomActors()) {
        if (phantom.movementHistory.size() > 1) {
            RenderMovementTrail(phantom.movementHistory);
        }
    }
}

void VisualOverlaySystem::RenderMovementTrail(const std::vector<MovementData>& trail) {
    auto drawList = ImGui::GetBackgroundDrawList();
    auto currentTime = ImGui::GetTime();
    
    for (size_t i = 1; i < trail.size(); i++) {
        FVector2D screenPos1, screenPos2;
        if (W2S(trail[i-1].position, screenPos1) && W2S(trail[i].position, screenPos2)) {
            float age = currentTime - trail[i].timestamp;
            float alpha = 1.0f - (age / ApocalypseESP.PsychoVision.fadeTime);
            alpha = std::max(0.0f, alpha);
            
            ImColor trailColor = ApocalypseESP.PsychoVision.trailColor;
            trailColor.Value.w = alpha;
            
            drawList->AddLine(
                ImVec2(screenPos1.X, screenPos1.Y),
                ImVec2(screenPos2.X, screenPos2.Y),
                trailColor,
                ApocalypseESP.PsychoVision.heatmapMode ? 3.0f : 2.0f
            );
            
            // Render velocity indicator
            if (ApocalypseESP.PsychoVision.showVelocity) {
                float velocity = trail[i].velocity;
                ImColor velocityColor = StyleManager::GetHeatmapColor(velocity / 1000.0f);
                velocityColor.Value.w = alpha;
                
                ImVec2 center = ImVec2((screenPos1.X + screenPos2.X) * 0.5f, 
                                      (screenPos1.Y + screenPos2.Y) * 0.5f);
                drawList->AddCircleFilled(center, 2.0f, velocityColor);
            }
        }
    }
}

void VisualOverlaySystem::RenderPhantomTrackers() {
    if (!ApocalypseESP.PhantomTracker.enabled) return;
    
    auto drawList = ImGui::GetBackgroundDrawList();
    auto currentTime = ImGui::GetTime();
    
    for (const auto& phantom : EntityMemory::GetPhantomActors()) {
        float timeSinceLastSeen = currentTime - phantom.lastSeen;
        if (timeSinceLastSeen > ApocalypseESP.PhantomTracker.phantomLifetime) continue;
        
        float confidence = ApocalypseUtils::CalculateConfidence(timeSinceLastSeen, phantom.confidence);
        if (confidence < ApocalypseESP.PhantomTracker.confidenceThreshold) continue;
        
        FVector2D screenPos;
        if (W2S(phantom.lastPosition, screenPos)) {
            ImColor phantomColor = ImColor(255, 255, 255, confidence * 255);
            
            // Render phantom indicator
            drawList->AddCircle(ImVec2(screenPos.X, screenPos.Y), 15.0f, phantomColor, 0, 2.0f);
            drawList->AddText(ImVec2(screenPos.X - 20, screenPos.Y - 25), phantomColor, 
                            phantom.name.c_str());
            
            // Render confidence indicator
            if (ApocalypseESP.PhantomTracker.showConfidence) {
                std::string confStr = std::to_string((int)(confidence * 100)) + "%";
                drawList->AddText(ImVec2(screenPos.X - 15, screenPos.Y + 20), phantomColor, 
                                confStr.c_str());
            }
            
            // Render time since last seen
            std::string timeStr = ApocalypseUtils::FormatTimeAgo(phantom.lastSeen);
            drawList->AddText(ImVec2(screenPos.X - 25, screenPos.Y + 35), 
                            ImColor(255, 255, 255, confidence * 200), timeStr.c_str());
        }
    }
}

void VisualOverlaySystem::RenderForesightMode() {
    if (!ApocalypseESP.Foresight.enabled) return;
    
    auto drawList = ImGui::GetBackgroundDrawList();
    
    // Render predictions for all tracked entities
    for (const auto& prediction : PredictionSystem::predictions) {
        if (prediction.second.confidence > 0.3f) {
            FVector2D screenPos;
            if (W2S(prediction.second.predictedPosition, screenPos)) {
                ImColor futureColor = ApocalypseESP.Foresight.futureColor;
                futureColor.Value.w = prediction.second.confidence;
                
                // Render predicted position
                drawList->AddCircle(ImVec2(screenPos.X, screenPos.Y), 20.0f, futureColor, 0, 3.0f);
                drawList->AddText(ImVec2(screenPos.X - 30, screenPos.Y - 30), futureColor, 
                                "FUTURE");
                
                // Render trajectory
                if (ApocalypseESP.Foresight.showTrajectory) {
                    RenderPredictionTrajectory(prediction.second.trajectory);
                }
                
                // Render confidence
                if (ApocalypseESP.Foresight.showConfidence) {
                    std::string confStr = std::to_string((int)(prediction.second.confidence * 100)) + "%";
                    drawList->AddText(ImVec2(screenPos.X - 15, screenPos.Y + 25), futureColor, 
                                    confStr.c_str());
                }
            }
        }
    }
}

void VisualOverlaySystem::RenderPredictionTrajectory(const std::vector<FVector>& trajectory) {
    auto drawList = ImGui::GetBackgroundDrawList();
    
    for (size_t i = 1; i < trajectory.size(); i++) {
        FVector2D screenPos1, screenPos2;
        if (W2S(trajectory[i-1], screenPos1) && W2S(trajectory[i], screenPos2)) {
            float alpha = 1.0f - ((float)i / trajectory.size());
            ImColor trajectoryColor = ApocalypseESP.Foresight.futureColor;
            trajectoryColor.Value.w = alpha;
            
            drawList->AddLine(
                ImVec2(screenPos1.X, screenPos1.Y),
                ImVec2(screenPos2.X, screenPos2.Y),
                trajectoryColor,
                2.0f
            );
        }
    }
}

void VisualOverlaySystem::RenderAudioVisualization() {
    if (!ApocalypseESP.AudioESP.enabled) return;
    
    auto drawList = ImGui::GetBackgroundDrawList();
    auto currentTime = ImGui::GetTime();
    
    for (const auto& audio : activeAudioEvents) {
        float age = currentTime - audio.timestamp;
        if (age > 3.0f) continue; // Audio events fade after 3 seconds
        
        FVector2D screenPos;
        if (W2S(audio.sourcePosition, screenPos)) {
            float alpha = 1.0f - (age / 3.0f);
            ImColor audioColor = ApocalypseESP.AudioESP.audioColor;
            audioColor.Value.w = alpha;
            
            // Render audio wave
            RenderAudioWave(audio);
            
            // Render sound type
            drawList->AddText(ImVec2(screenPos.X - 20, screenPos.Y - 15), audioColor, 
                            audio.soundType.c_str());
            
            // Render volume indicator
            if (ApocalypseESP.AudioESP.showVolume) {
                float radius = (audio.volume / 100.0f) * 30.0f;
                drawList->AddCircle(ImVec2(screenPos.X, screenPos.Y), radius, audioColor, 0, 2.0f);
            }
        }
    }
}

void VisualOverlaySystem::RenderAudioWave(const AudioVisualization& audio) {
    auto drawList = ImGui::GetBackgroundDrawList();
    FVector2D screenPos;
    if (!W2S(audio.sourcePosition, screenPos)) return;
    
    auto currentTime = ImGui::GetTime();
    float timeOffset = (currentTime - audio.timestamp) * 10.0f; // Wave speed
    
    ImColor waveColor = ApocalypseESP.AudioESP.audioColor;
    waveColor.Value.w = 0.7f;
    
    // Render expanding wave circles
    for (int i = 0; i < 3; i++) {
        float radius = 10.0f + (i * 15.0f) + timeOffset;
        float alpha = 1.0f - (i * 0.3f) - (timeOffset / 100.0f);
        alpha = std::max(0.0f, alpha);
        
        ImColor circleColor = waveColor;
        circleColor.Value.w = alpha;
        
        drawList->AddCircle(ImVec2(screenPos.X, screenPos.Y), radius, circleColor, 0, 2.0f);
    }
}

void VisualOverlaySystem::RenderKarmaSystem() {
    if (!ApocalypseESP.Karma.enabled) return;
    
    auto drawList = ImGui::GetBackgroundDrawList();
    
    // Render karma lines between players
    if (ApocalypseESP.Karma.showKarmaLines) {
        auto actors = UWorld::GWorld->PersistentLevel->Actors;
        for (int i = 0; i < actors.Count; i++) {
            auto actor1 = actors[i];
            if (!actor1 || !actor1->IsPlayer() || actor1 == Cache.LocalActor) continue;
            
            for (int j = i + 1; j < actors.Count; j++) {
                auto actor2 = actors[j];
                if (!actor2 || !actor2->IsPlayer()) continue;
                
                std::string name1 = actor1->PlayerName.ToString();
                std::string name2 = actor2->PlayerName.ToString();
                
                int karma1 = PlayerRelations::GetPlayerKarma(name1);
                int karma2 = PlayerRelations::GetPlayerKarma(name2);
                
                // Only show lines between players with significant karma differences
                if (abs(karma1 - karma2) > 10) {
                    FVector2D screenPos1, screenPos2;
                    if (W2S(actor1->RootComponent->GetWorldLocation(), screenPos1) &&
                        W2S(actor2->RootComponent->GetWorldLocation(), screenPos2)) {
                        
                        ImColor karmaColor = ApocalypseUtils::GetKarmaColor(karma1 - karma2);
                        drawList->AddLine(
                            ImVec2(screenPos1.X, screenPos1.Y),
                            ImVec2(screenPos2.X, screenPos2.Y),
                            karmaColor,
                            2.0f
                        );
                    }
                }
            }
        }
    }
}

// ============================================================================
// ENTITY MEMORY SYSTEM
// ============================================================================

void EntityMemory::UpdatePhantomActors() {
    auto currentTime = ImGui::GetTime();
    
    // Update existing phantoms
    for (auto& pair : phantomActors) {
        auto& phantom = pair.second;
        float timeSinceLastSeen = currentTime - phantom.lastSeen;
        phantom.confidence = ApocalypseUtils::CalculateConfidence(timeSinceLastSeen, phantom.confidence);
    }
    
    // Clean up old phantoms
    CleanupOldPhantoms();
}

void EntityMemory::AddPhantomActor(const std::string& name, const FVector& position, const std::string& type) {
    phantomActors[name] = PhantomActor(name, position, type);
}

void EntityMemory::UpdateMovementHistory(const std::string& name, const FVector& position, float velocity, const FRotator& direction) {
    auto it = phantomActors.find(name);
    if (it != phantomActors.end()) {
        it->second.movementHistory.push_back(MovementData(position, velocity, direction));
        
        // Keep only recent movement data
        while (it->second.movementHistory.size() > 50) {
            it->second.movementHistory.erase(it->second.movementHistory.begin());
        }
    }
}

std::vector<PhantomActor> EntityMemory::GetPhantomActors() {
    std::vector<PhantomActor> result;
    for (const auto& pair : phantomActors) {
        result.push_back(pair.second);
    }
    return result;
}

void EntityMemory::CleanupOldPhantoms() {
    auto currentTime = ImGui::GetTime();
    
    for (auto it = phantomActors.begin(); it != phantomActors.end();) {
        float timeSinceLastSeen = currentTime - it->second.lastSeen;
        if (timeSinceLastSeen > ApocalypseESP.PhantomTracker.phantomLifetime) {
            it = phantomActors.erase(it);
        } else {
            ++it;
        }
    }
}

// ============================================================================
// DINO ANALYZER SYSTEM
// ============================================================================

DinoDNA DinoAnalyzer::AnalyzeDino(AActor* dino) {
    if (!dino || !dino->IsDino()) return DinoDNA();
    
    std::string dinoName = dino->DinoNameTag.ToString();
    
    // Check cache first
    auto it = dnaCache.find(dinoName);
    if (it != dnaCache.end()) {
        return it->second;
    }
    
    DinoDNA dna;
    dna.species = dinoName;
    
    if (dino->MyCharacterStatusComponent) {
        dna.level = dino->MyCharacterStatusComponent->BaseCharacterLevel + 
                   dino->MyCharacterStatusComponent->ExtraCharacterLevel;
    }
    
    dna.health = dino->ReplicatedCurrentHealth;
    
    // Analyze dino stats (simplified - in real implementation you'd read from game memory)
    dna.stamina = 100.0f; // Placeholder
    dna.weight = 100.0f;  // Placeholder
    dna.melee = 100.0f;   // Placeholder
    dna.speed = 100.0f;   // Placeholder
    
    // Determine ancestry
    dna.ancestry = "Pure"; // Placeholder - would analyze breeding history
    
    // Check for mutations (simplified)
    if (dna.level > 150) {
        dna.mutations.push_back("High Level");
    }
    
    // Cache the result
    dnaCache[dinoName] = dna;
    
    return dna;
}

void DinoAnalyzer::RenderDNAInfo(const DinoDNA& dna, const ImVec2& position) {
    if (!ApocalypseESP.DNAScanner.enabled) return;
    
    auto drawList = ImGui::GetBackgroundDrawList();
    
    std::string info = dna.species + " Lv." + std::to_string(dna.level);
    drawList->AddText(position, ImColor(255, 255, 255, 255), info.c_str());
    
    if (ApocalypseESP.DNAScanner.showStats) {
        std::string stats = "HP: " + std::to_string((int)dna.health) + 
                           " | Stam: " + std::to_string((int)dna.stamina);
        drawList->AddText(ImVec2(position.x, position.y + 15), 
                         ImColor(200, 200, 200, 255), stats.c_str());
    }
    
    if (ApocalypseESP.DNAScanner.showMutations && !dna.mutations.empty()) {
        std::string mutations = "Mutations: " + dna.mutations[0];
        drawList->AddText(ImVec2(position.x, position.y + 30), 
                         ApocalypseESP.DNAScanner.mutationColor, mutations.c_str());
    }
    
    if (ApocalypseESP.DNAScanner.showAncestry) {
        drawList->AddText(ImVec2(position.x, position.y + 45), 
                         ImColor(150, 150, 255, 255), dna.ancestry.c_str());
    }
}

bool DinoAnalyzer::IsWishListMatch(const DinoDNA& dna, const std::vector<std::string>& wishList) {
    for (const auto& filter : wishList) {
        if (ParseWishListFilter(filter, dna)) {
            return true;
        }
    }
    return false;
}

bool DinoAnalyzer::ParseWishListFilter(const std::string& filter, const DinoDNA& dna) {
    // Simple filter parsing - can be expanded
    if (filter.find(dna.species) != std::string::npos) return true;
    if (filter.find("lvl") != std::string::npos) {
        // Parse level requirement
        std::regex levelRegex(R"(\d+)");
        std::smatch match;
        if (std::regex_search(filter, match, levelRegex)) {
            int requiredLevel = std::stoi(match.str());
            if (dna.level >= requiredLevel) return true;
        }
    }
    return false;
}

// ============================================================================
// PREDICTION SYSTEM
// ============================================================================

PredictionData PredictionSystem::PredictMovement(const std::vector<MovementData>& history, float timeAhead) {
    if (history.size() < 2) return PredictionData();
    
    PredictionData prediction;
    prediction.timeAhead = timeAhead;
    
    // Simple linear prediction based on last movement
    const auto& current = history.back();
    const auto& previous = history[history.size() - 2];
    
    FVector velocity = (current.position - previous.position) / (current.timestamp - previous.timestamp);
    prediction.predictedPosition = current.position + (velocity * timeAhead);
    
    // Calculate confidence based on movement consistency
    float consistency = 1.0f;
    if (history.size() > 3) {
        // Check if movement direction is consistent
        FVector avgDirection = FVector(0, 0, 0);
        for (size_t i = 1; i < history.size(); i++) {
            avgDirection = avgDirection + (history[i].position - history[i-1].position);
        }
        avgDirection = avgDirection / (history.size() - 1);
        
        FVector lastDirection = current.position - previous.position;
        float dotProduct = avgDirection.X * lastDirection.X + avgDirection.Y * lastDirection.Y + avgDirection.Z * lastDirection.Z;
        consistency = std::max(0.0f, dotProduct / (avgDirection.DistTo(FVector(0,0,0)) * lastDirection.DistTo(FVector(0,0,0))));
    }
    
    prediction.confidence = consistency;
    
    // Calculate trajectory
    prediction.trajectory = CalculateTrajectory(current.position, velocity, timeAhead);
    
    return prediction;
}

void PredictionSystem::UpdatePredictions() {
    for (const auto& phantom : EntityMemory::GetPhantomActors()) {
        if (phantom.movementHistory.size() >= 2) {
            predictions[phantom.name] = PredictMovement(phantom.movementHistory, 
                                                      ApocalypseESP.Foresight.predictionTime);
        }
    }
}

std::vector<FVector> PredictionSystem::CalculateTrajectory(const FVector& start, const FVector& velocity, float time) {
    std::vector<FVector> trajectory;
    int steps = 10;
    
    for (int i = 0; i <= steps; i++) {
        float t = (float)i / steps * time;
        trajectory.push_back(start + (velocity * t));
    }
    
    return trajectory;
}

// ============================================================================
// PLAYER RELATIONS SYSTEM
// ============================================================================

void PlayerRelations::UpdateKarma(const std::string& playerName, int karmaChange) {
    playerKarma[playerName] += karmaChange;
}

int PlayerRelations::GetPlayerKarma(const std::string& playerName) {
    auto it = playerKarma.find(playerName);
    return (it != playerKarma.end()) ? it->second : 0;
}

void PlayerRelations::AddKarmaEvent(const std::string& player, const std::string& action, int karma) {
    karmaEvents.push_back(std::make_pair(player, action));
    UpdateKarma(player, karma);
}

// ============================================================================
// STYLE MANAGER
// ============================================================================

void StyleManager::ApplyGraffitiStyle() {
    // Apply graffiti-style colors and effects
    ImGuiStyle& style = ImGui::GetStyle();
    style.Colors[ImGuiCol_Button] = ImColor(255, 0, 255, 255);
    style.Colors[ImGuiCol_ButtonHovered] = ImColor(255, 100, 255, 255);
    style.Colors[ImGuiCol_ButtonActive] = ImColor(200, 0, 200, 255);
}

void StyleManager::ApplySciFiTheme() {
    // Apply sci-fi theme colors
    ImGuiStyle& style = ImGui::GetStyle();
    style.Colors[ImGuiCol_WindowBg] = ImColor(10, 10, 30, 200);
    style.Colors[ImGuiCol_Button] = ImColor(0, 150, 255, 255);
    style.Colors[ImGuiCol_ButtonHovered] = ImColor(0, 200, 255, 255);
}

void StyleManager::RenderCustomTag(const std::string& text, const ImVec2& pos, const ImColor& color) {
    auto drawList = ImGui::GetBackgroundDrawList();
    
    // Add glow effect
    for (int i = 0; i < 3; i++) {
        ImVec2 offset = ImVec2(i, i);
        drawList->AddText(ImVec2(pos.x + offset.x, pos.y + offset.y), 
                         ImColor(0, 0, 0, 100), text.c_str());
    }
    
    drawList->AddText(pos, color, text.c_str());
}

ImColor StyleManager::GetHeatmapColor(float intensity) {
    if (heatmapColors.empty()) {
        InitializeHeatmapColors();
    }
    
    intensity = std::max(0.0f, std::min(1.0f, intensity));
    int index = (int)(intensity * (heatmapColors.size() - 1));
    
    return heatmapColors[index];
}

void StyleManager::InitializeHeatmapColors() {
    heatmapColors = {
        ImColor(0, 0, 255, 255),    // Blue (cold)
        ImColor(0, 255, 255, 255),  // Cyan
        ImColor(0, 255, 0, 255),    // Green
        ImColor(255, 255, 0, 255),  // Yellow
        ImColor(255, 128, 0, 255),  // Orange
        ImColor(255, 0, 0, 255)     // Red (hot)
    };
}

// ============================================================================
// SOUND VISUALIZER
// ============================================================================

void SoundVisualizer::AddAudioEvent(const FVector& position, float volume, const std::string& type) {
    audioEvents.push_back(AudioVisualization(position, volume, type));
}

void SoundVisualizer::UpdateAudioVisualizations() {
    CleanupOldAudioEvents();
}

void SoundVisualizer::CleanupOldAudioEvents() {
    auto currentTime = ImGui::GetTime();
    audioEvents.erase(
        std::remove_if(audioEvents.begin(), audioEvents.end(),
            [&](const AudioVisualization& audio) {
                return (currentTime - audio.timestamp) > 3.0f;
            }),
        audioEvents.end()
    );
}

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

namespace ApocalypseUtils {
    bool IsInWishList(const std::string& name, const std::vector<std::string>& wishList) {
        for (const auto& filter : wishList) {
            if (name.find(filter) != std::string::npos) {
                return true;
            }
        }
        return false;
    }
    
    ImColor GetKarmaColor(int karma) {
        if (karma > 50) return ApocalypseESP.Karma.goodKarmaColor;
        if (karma < -50) return ApocalypseESP.Karma.badKarmaColor;
        return ImColor(255, 255, 255, 255); // Neutral
    }
    
    float CalculateConfidence(float timeSinceLastSeen, float baseConfidence) {
        float decayRate = 0.1f; // Confidence decays by 10% per second
        return baseConfidence * std::exp(-decayRate * timeSinceLastSeen);
    }
    
    std::string FormatTimeAgo(float timestamp) {
        float timeAgo = ImGui::GetTime() - timestamp;
        
        if (timeAgo < 60.0f) {
            return std::to_string((int)timeAgo) + "s ago";
        } else if (timeAgo < 3600.0f) {
            return std::to_string((int)(timeAgo / 60.0f)) + "m ago";
        } else {
            return std::to_string((int)(timeAgo / 3600.0f)) + "h ago";
        }
    }
    
    ImVec2 WorldToRadarPosition(const FVector& worldPos, const ImVec2& radarCenter, float radarSize, float zoom) {
        if (!Cache.LocalActor) return radarCenter;
        
        FVector localPos = Cache.LocalActor->RootComponent->GetWorldLocation();
        FVector relativePos = worldPos - localPos;
        
        // Convert to radar coordinates
        float radarX = (relativePos.X / (1000.0f * zoom)) * radarSize;
        float radarY = (relativePos.Y / (1000.0f * zoom)) * radarSize;
        
        return ImVec2(radarCenter.x + radarX, radarCenter.y + radarY);
    }
} 