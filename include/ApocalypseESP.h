#pragma once
#include "Menu.h"
#include <unordered_map>
#include <deque>
#include <chrono>
#include <regex>
#include <memory>

// Forward declarations
struct ESPTag;
struct MovementData;
struct PhantomActor;
struct DinoDNA;
struct PredictionData;
struct AudioVisualization;

// ESP Tag System
struct ESPTag {
    std::string label;
    ImColor color;
    ImVec2 position;
    float timestamp;
    bool persistent;
    std::string category; // "player", "dino", "loot", "custom"
    
    ESPTag(const std::string& lbl, const ImColor& col, const ImVec2& pos, bool persist = false)
        : label(lbl), color(col), position(pos), timestamp(ImGui::GetTime()), persistent(persist) {}
};

// Movement Tracking System
struct MovementData {
    FVector position;
    float timestamp;
    float velocity;
    FRotator direction;
    
    MovementData(const FVector& pos, float vel, const FRotator& dir)
        : position(pos), timestamp(ImGui::GetTime()), velocity(vel), direction(dir) {}
};

// Phantom Tracker System
struct PhantomActor {
    std::string name;
    FVector lastPosition;
    float lastSeen;
    float confidence; // 0.0 to 1.0
    std::string type; // "player", "dino", "loot"
    std::vector<MovementData> movementHistory;
    
    PhantomActor(const std::string& n, const FVector& pos, const std::string& t)
        : name(n), lastPosition(pos), lastSeen(ImGui::GetTime()), confidence(1.0f), type(t) {}
};

// DNA Scanner System
struct DinoDNA {
    std::string species;
    int level;
    float health;
    float stamina;
    float weight;
    float melee;
    float speed;
    std::vector<std::string> mutations;
    std::string ancestry; // "Pure", "Mutated", "Bred"
    float imprintQuality;
    
    DinoDNA() : level(0), health(0), stamina(0), weight(0), melee(0), speed(0), imprintQuality(0) {}
};

// Prediction System
struct PredictionData {
    FVector predictedPosition;
    float confidence;
    float timeAhead;
    std::vector<FVector> trajectory;
    
    PredictionData() : confidence(0.0f), timeAhead(0.0f) {}
};

// Audio Visualization
struct AudioVisualization {
    FVector sourcePosition;
    float volume;
    float frequency;
    std::string soundType;
    float timestamp;
    
    AudioVisualization(const FVector& pos, float vol, const std::string& type)
        : sourcePosition(pos), volume(vol), soundType(type), timestamp(ImGui::GetTime()) {}
};

// Apocalypse ESP Settings
struct ApocalypseSettings {
    // ESP Graffiti System
    struct {
        bool enabled = false;
        bool persistentTags = true;
        bool showTimestamps = true;
        float tagLifetime = 30.0f;
        bool graffitiMode = false;
        std::vector<std::string> customTags;
    } Graffiti;
    
    // PsychoVision (Movement Heatmap)
    struct {
        bool enabled = false;
        float trailLength = 10.0f;
        bool showVelocity = true;
        bool heatmapMode = false;
        float fadeTime = 5.0f;
        ImColor trailColor = ImColor(255, 0, 255, 255);
    } PsychoVision;
    
    // Satellite View Radar
    struct {
        bool enabled = false;
        float radarSize = 200.0f;
        bool showDinos = true;
        bool showPlayers = true;
        bool showLoot = false;
        bool parallaxEffect = true;
        float zoomLevel = 1.0f;
    } SatelliteView;
    
    // Phantom Tracker
    struct {
        bool enabled = false;
        float phantomLifetime = 60.0f;
        bool showConfidence = true;
        bool predictMovement = true;
        float confidenceThreshold = 0.3f;
    } PhantomTracker;
    
    // Wish List ESP
    struct {
        bool enabled = false;
        std::vector<std::string> wishList;
        bool highlightOnly = false;
        ImColor highlightColor = ImColor(255, 255, 0, 255);
        bool showDistance = true;
    } WishList;
    
    // Foresight Mode
    struct {
        bool enabled = false;
        float predictionTime = 3.0f;
        bool showTrajectory = true;
        bool showConfidence = true;
        ImColor futureColor = ImColor(0, 255, 255, 128);
    } Foresight;
    
    // DNA Scanner
    struct {
        bool enabled = false;
        bool showStats = true;
        bool showMutations = true;
        bool showAncestry = true;
        bool hoverInfo = true;
        ImColor mutationColor = ImColor(255, 0, 255, 255);
    } DNAScanner;
    
    // Audio ESP
    struct {
        bool enabled = false;
        bool visualizeSounds = true;
        bool showVolume = true;
        float audioRange = 100.0f;
        ImColor audioColor = ImColor(0, 255, 0, 255);
    } AudioESP;
    
    // Karma System
    struct {
        bool enabled = false;
        std::unordered_map<std::string, int> playerKarma;
        bool showKarmaLines = true;
        ImColor goodKarmaColor = ImColor(0, 255, 0, 255);
        ImColor badKarmaColor = ImColor(255, 0, 0, 255);
    } Karma;
};

// Core Systems
class VisualOverlaySystem {
public:
    static void RenderESPGraffiti();
    static void RenderSatelliteView();
    static void RenderPsychoVision();
    static void RenderPhantomTrackers();
    static void RenderForesightMode();
    static void RenderAudioVisualization();
    static void RenderKarmaSystem();
    
private:
    static void RenderTag(const ESPTag& tag);
    static void RenderMovementTrail(const std::vector<MovementData>& trail);
    static void RenderRadarIcon(const FVector& worldPos, const std::string& type, const ImColor& color);
    static void RenderPredictionTrajectory(const std::vector<FVector>& trajectory);
    static void RenderAudioWave(const AudioVisualization& audio);
    static void RenderKarmaLine(const FVector& from, const FVector& to, int karma);
};

class EntityMemory {
public:
    static void UpdatePhantomActors();
    static void AddPhantomActor(const std::string& name, const FVector& position, const std::string& type);
    static void UpdateMovementHistory(const std::string& name, const FVector& position, float velocity, const FRotator& direction);
    static std::vector<PhantomActor> GetPhantomActors();
    static void CleanupOldPhantoms();
    
private:
    static std::unordered_map<std::string, PhantomActor> phantomActors;
    static std::deque<MovementData> globalMovementHistory;
};

class DinoAnalyzer {
public:
    static DinoDNA AnalyzeDino(AActor* dino);
    static void RenderDNAInfo(const DinoDNA& dna, const ImVec2& position);
    static bool IsWishListMatch(const DinoDNA& dna, const std::vector<std::string>& wishList);
    static std::string GetDinoAncestry(const DinoDNA& dna);
    
private:
    static std::unordered_map<std::string, DinoDNA> dnaCache;
    static bool ParseWishListFilter(const std::string& filter, const DinoDNA& dna);
};

class PredictionSystem {
public:
    static PredictionData PredictMovement(const std::vector<MovementData>& history, float timeAhead);
    static void UpdatePredictions();
    static std::vector<FVector> CalculateTrajectory(const FVector& start, const FVector& velocity, float time);
    
private:
    static std::unordered_map<std::string, PredictionData> predictions;
    static FVector ExtrapolatePosition(const MovementData& current, float timeAhead);
};

class PlayerRelations {
public:
    static void UpdateKarma(const std::string& playerName, int karmaChange);
    static int GetPlayerKarma(const std::string& playerName);
    static void RenderKarmaLines();
    static void AddKarmaEvent(const std::string& player, const std::string& action, int karma);
    
private:
    static std::unordered_map<std::string, int> playerKarma;
    static std::vector<std::pair<std::string, std::string>> karmaEvents; // player, action
};

class StyleManager {
public:
    static void ApplyGraffitiStyle();
    static void ApplySciFiTheme();
    static void RenderCustomTag(const std::string& text, const ImVec2& pos, const ImColor& color);
    static ImColor GetHeatmapColor(float intensity);
    
private:
    static std::vector<ImColor> heatmapColors;
    static void InitializeHeatmapColors();
};

class SoundVisualizer {
public:
    static void AddAudioEvent(const FVector& position, float volume, const std::string& type);
    static void UpdateAudioVisualizations();
    static void RenderAudioESP();
    
private:
    static std::vector<AudioVisualization> audioEvents;
    static void CleanupOldAudioEvents();
};

// Global instances
extern ApocalypseSettings ApocalypseESP;
extern std::vector<ESPTag> activeTags;
extern std::vector<AudioVisualization> activeAudioEvents;

// Utility functions
namespace ApocalypseUtils {
    bool IsInWishList(const std::string& name, const std::vector<std::string>& wishList);
    ImColor GetKarmaColor(int karma);
    float CalculateConfidence(float timeSinceLastSeen, float baseConfidence);
    std::string FormatTimeAgo(float timestamp);
    ImVec2 WorldToRadarPosition(const FVector& worldPos, const ImVec2& radarCenter, float radarSize, float zoom);
} 