/**
 * NRF24Controller Configuration Library
 * 
 * This library provides easy configuration management for NRF24Controller systems.
 * It allows loading configurations from strings, files, or hardcoded setups.
 * 
 * Author: GitHub Copilot
 * Date: 2025
 */

#ifndef NRF24CONFIG_H
#define NRF24CONFIG_H

#include <Arduino.h>
#include <NRF24Controller.h>

// Configuration helper class
class NRF24Config {
private:
    static SystemConfig _currentConfig;
    
public:
    // Configuration loading
    static SystemConfig loadFromString(const char* configData);
    static SystemConfig loadDefault();
    static SystemConfig loadDroneConfig();
    static SystemConfig loadCarConfig();
    static SystemConfig loadPlaneConfig();
    
    // Hardware configuration helpers
    static void configureJoystick(SystemConfig& config, uint8_t id, 
                                 uint8_t pinX, uint8_t pinY, uint8_t pinButton = 255,
                                 const char* name = nullptr);
    
    static void configureLever(SystemConfig& config, uint8_t id, LeverType type,
                              uint8_t pinA, uint8_t pinB = 255, uint8_t pinButton = 255,
                              const char* name = nullptr);
    
    // Calibration helpers
    static void setJoystickCalibration(SystemConfig& config, uint8_t id,
                                      int minX, int maxX, int centerX,
                                      int minY, int maxY, int centerY,
                                      int deadZone = 60);
    
    static void setLeverCalibration(SystemConfig& config, uint8_t id,
                                   int minPos, int maxPos, int centerPos = -1,
                                   int deadZone = 50);
    
    // NRF24 configuration
    static void configureNRF24(SystemConfig& config, uint8_t cePin, uint8_t csnPin,
                               uint8_t channel = 76, PowerLevel power = POWER_HIGH);
    
    // Configuration validation and printing
    static bool validateConfig(const SystemConfig& config);
    static void printConfig(const SystemConfig& config);
    
    // Easy setup functions
    static SystemConfig createBasicSetup(uint8_t cePin, uint8_t csnPin);
    static SystemConfig createDroneSetup(uint8_t cePin, uint8_t csnPin);
    static SystemConfig createCarSetup(uint8_t cePin, uint8_t csnPin);
    
    // Parse configuration from string format
    static bool parseConfigLine(SystemConfig& config, const char* line);
};

// Predefined configurations
namespace NRF24Configs {
    // Basic configuration string
    extern const char* BASIC_CONFIG;
    
    // Drone configuration string
    extern const char* DRONE_CONFIG;
    
    // Car configuration string
    extern const char* CAR_CONFIG;
    
    // Plane configuration string
    extern const char* PLANE_CONFIG;
}

#endif // NRF24CONFIG_H