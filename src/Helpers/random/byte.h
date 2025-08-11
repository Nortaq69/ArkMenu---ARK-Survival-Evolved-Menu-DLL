#pragma once
#include <random>
#include <cstdint>

namespace Helpers {
    namespace Random {
        // Random number generator
        inline std::random_device rd;
        inline std::mt19937 gen(rd());
        
        // Generate random bytes
        inline void GenerateRandomBytes(uint8_t* buffer, size_t size) {
            std::uniform_int_distribution<> dis(0, 255);
            for (size_t i = 0; i < size; ++i) {
                buffer[i] = static_cast<uint8_t>(dis(gen));
            }
        }
        
        // Generate random integer
        template<typename T>
        inline T RandomInt(T min, T max) {
            std::uniform_int_distribution<T> dis(min, max);
            return dis(gen);
        }
        
        // Generate random float
        inline float RandomFloat(float min, float max) {
            std::uniform_real_distribution<float> dis(min, max);
            return dis(gen);
        }
        
        // XOR encryption/decryption
        inline void XORBuffer(uint8_t* buffer, size_t size, const uint8_t* key, size_t keySize) {
            for (size_t i = 0; i < size; ++i) {
                buffer[i] ^= key[i % keySize];
            }
        }
        
        // Simple string obfuscation
        inline std::string ObfuscateString(const std::string& input, const std::string& key) {
            std::string result = input;
            for (size_t i = 0; i < result.length(); ++i) {
                result[i] ^= key[i % key.length()];
            }
            return result;
        }
        
        // Deobfuscate string
        inline std::string DeobfuscateString(const std::string& input, const std::string& key) {
            return ObfuscateString(input, key); // XOR is symmetric
        }
    }
} 