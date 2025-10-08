/**
 * NRF24Controller Library - Flexible NRF24L01 communication controller
 * 
 * Features:
 * - Easy integration with Joystick and Lever libraries
 * - Flexible data packet system
 * - Configurable transmission parameters
 * - Multiple joystick and lever support
 * - Automatic packet management
 * - Power level configuration
 * - Channel and address management
 * - Built-in acknowledgment system
 * - Data rate configuration
 * 
 * Author: GitHub Copilot
 * Date: 2025
 */

#ifndef NRF24CONTROLLER_H
#define NRF24CONTROLLER_H

#include <Arduino.h>
#include <RF24.h>
#include <Joystick.h>
#include <Lever.h>
#include <EEPROM.h>

// Maximum number of controls supported
#define MAX_JOYSTICKS 4
#define MAX_LEVERS 6
#define MAX_PACKET_SIZE 32

// Power levels
enum PowerLevel {
    POWER_MIN = RF24_PA_MIN,     // -18dBm
    POWER_LOW = RF24_PA_LOW,     // -12dBm  
    POWER_HIGH = RF24_PA_HIGH,   // -6dBm
    POWER_MAX = RF24_PA_MAX      // 0dBm
};

// Data rates
enum DataRate {
    RATE_250KBPS = RF24_250KBPS,
    RATE_1MBPS = RF24_1MBPS,
    RATE_2MBPS = RF24_2MBPS
};

// Control types for packet identification
enum ControlType {
    CONTROL_JOYSTICK,
    CONTROL_LEVER_ANALOG,
    CONTROL_LEVER_ENCODER,
    CONTROL_LEVER_DIGITAL,
    CONTROL_BUTTON,
    CONTROL_CUSTOM
};

// Individual control data structure
struct ControlData {
    uint8_t id;           // Control identifier (0-255)
    ControlType type;     // Type of control
    int16_t valueX;       // Primary value (or X axis for joystick)
    int16_t valueY;       // Secondary value (or Y axis for joystick)
    uint8_t flags;        // Status flags (button states, etc.)
    uint32_t timestamp;   // Timestamp for this data
};

// Main data packet structure
struct DataPacket {
    uint8_t packetId;           // Packet identifier
    uint8_t controlCount;       // Number of controls in this packet
    ControlData controls[8];    // Control data array (max 8 per packet)
    uint16_t checksum;          // Simple checksum for data integrity
    uint32_t timestamp;         // Packet timestamp
};

// Transmission statistics
struct TransmissionStats {
    uint32_t packetsSent;
    uint32_t packetsReceived;
    uint32_t packetsLost;
    uint32_t lastTransmissionTime;
    float successRate;
};

// Control mapping configuration
struct ControlMapping {
    uint8_t outputChannel;      // Output channel (0-31)
    int16_t minValue;          // Minimum output value
    int16_t maxValue;          // Maximum output value
    int16_t centerValue;       // Center/neutral value
    bool invertOutput;         // Invert the output
    float scaleFactor;         // Scale factor (1.0 = normal)
    bool enabled;              // Enable/disable this mapping
};

// Conditional mapping based on lever position
struct ConditionalMapping {
    uint8_t conditionLeverId;   // Lever ID to check condition
    uint8_t conditionPosition;  // Position that triggers this mapping (0,1,2,etc)
    ControlMapping mapping;     // Mapping to apply when condition is met
};

// Control profile for flexible configuration
struct ControlProfile {
    char name[16];                              // Profile name
    ControlMapping joystickMappings[MAX_JOYSTICKS][2]; // X and Y mappings for each joystick
    ControlMapping leverMappings[MAX_LEVERS];   // Mappings for each lever
    ConditionalMapping conditionalMappings[8];  // Up to 8 conditional mappings
    uint8_t conditionalCount;                   // Number of active conditionals
    bool autoExecute;                          // Auto-execute this profile
    unsigned long executeInterval;              // Execution interval (ms)
    bool enabled;                              // Profile enabled/disabled
};

// EEPROM storage structure for profiles
struct EEPROMProfileData {
    uint32_t magicNumber;      // 0x12345678 - validation
    uint8_t version;           // Version for compatibility
    ControlProfile profiles[4]; // All profiles
    uint8_t profileCount;      // Number of profiles
    uint16_t checksum;         // Data integrity check
};

// Configuration file structure
struct SystemConfig {
    // NRF24 settings
    uint8_t nrfCEPin;
    uint8_t nrfCSNPin;
    uint8_t nrfChannel;
    PowerLevel nrfPowerLevel;
    DataRate nrfDataRate;
    uint64_t nrfTxAddress;
    uint64_t nrfRxAddress;
    
    // Joystick configurations
    struct JoystickConfig {
        bool enabled;
        uint8_t pinX, pinY, pinButton;
        int minX, maxX, centerX;
        int minY, maxY, centerY;
        int deadZone;
        bool invertX, invertY;
        float smoothingFactor;
        char name[16];
    } joysticks[MAX_JOYSTICKS];
    
    // Lever configurations
    struct LeverConfig {
        bool enabled;
        LeverType type;
        uint8_t pinA, pinB, pinButton;
        int minPosition, maxPosition, centerPosition;
        int deadZone;
        bool invertDirection;
        float smoothingFactor;
        int stepsPerDetent;    // For encoders
        int digitalPositions;  // For digital levers
        char name[16];
    } levers[MAX_LEVERS];
    
    // System settings
    char systemName[32];
    bool debugMode;
    unsigned long transmissionInterval;
};

class NRF24Controller {
private:
    RF24* _radio;
    
    // NRF24 configuration
    uint8_t _cePin;
    uint8_t _csnPin;
    uint8_t _channel;
    PowerLevel _powerLevel;
    DataRate _dataRate;
    uint64_t _txAddress;
    uint64_t _rxAddress;
    
    // Control management
    Joystick* _joysticks[MAX_JOYSTICKS];
    Lever* _levers[MAX_LEVERS];
    uint8_t _joystickCount;
    uint8_t _leverCount;
    
    // Packet management
    DataPacket _currentPacket;
    uint8_t _packetCounter;
    bool _autoSend;
    unsigned long _sendInterval;
    unsigned long _lastSendTime;
    
    // Transmission control
    bool _enableAck;
    uint8_t _retryCount;
    uint8_t _retryDelay;
    
    // Statistics
    TransmissionStats _stats;
    
    // Control selection (which controls to include in packets)
    bool _joystickEnabled[MAX_JOYSTICKS];
    bool _leverEnabled[MAX_LEVERS];
    
    // Data filtering
    int _joystickThreshold;     // Minimum change to trigger send
    int _leverThreshold;        // Minimum change to trigger send
    bool _sendOnlyChanges;      // Send only when values change
    
    // Last sent values for change detection
    ControlData _lastJoystickData[MAX_JOYSTICKS];
    ControlData _lastLeverData[MAX_LEVERS];
    
    // Profile management
    ControlProfile _profiles[4];        // Up to 4 different profiles
    uint8_t _activeProfile;            // Currently active profile index
    uint8_t _profileCount;             // Number of configured profiles
    unsigned long _lastProfileExecution;
    
    // Channel output values (32 channels available)
    int16_t _channelValues[32];
    bool _channelUpdated[32];
    
    // Internal helper methods
    void _initializeRadio();
    void _updateControlData();
    bool _hasDataChanged();
    uint16_t _calculateChecksum(const DataPacket& packet);
    void _updateStats(bool success);
    
    // Profile helper methods
    void _initializeProfiles();
    void _executeActiveProfile();
    int16_t _applyMapping(int16_t inputValue, const ControlMapping& mapping);
    bool _checkConditionalMapping(const ConditionalMapping& conditional);
    void _updateChannelValues();
    
public:
    // Constructor
    NRF24Controller(uint8_t cePin, uint8_t csnPin);
    
    // Initialization and configuration
    bool begin();
    void setChannel(uint8_t channel);
    void setPowerLevel(PowerLevel level);
    void setDataRate(DataRate rate);
    void setAddresses(uint64_t txAddr, uint64_t rxAddr);
    void enableAckPayload(bool enable);
    void setRetrySettings(uint8_t count, uint8_t delay);
    
    // Control management
    bool addJoystick(Joystick* joystick, uint8_t id = 0);
    bool addLever(Lever* lever, uint8_t id = 0);
    void removeJoystick(uint8_t id);
    void removeLever(uint8_t id);
    void enableJoystick(uint8_t id, bool enable = true);
    void enableLever(uint8_t id, bool enable = true);
    
    // Transmission settings
    void setAutoSend(bool enable, unsigned long interval = 50);
    void setSendThresholds(int joystickThreshold = 5, int leverThreshold = 5);
    void setSendOnlyChanges(bool enable = true);
    
    // Data transmission methods
    bool sendData();
    bool sendCustomData(const ControlData& data);
    bool sendCustomPacket(const DataPacket& packet);
    void update(); // Call this regularly in loop()
    
    // Data reception methods
    bool available();
    bool readData(DataPacket& packet);
    bool readControlData(uint8_t controlId, ControlData& data);
    
    // Packet management
    void clearPacket();
    void addToPacket(uint8_t controlId, ControlType type, int16_t valueX, int16_t valueY = 0, uint8_t flags = 0);
    uint8_t getPacketSize();
    
    // Status and diagnostics
    bool isConnected();
    TransmissionStats getStats();
    void resetStats();
    float getSignalQuality(); // Based on success rate
    void printStatus();
    void printPacket(const DataPacket& packet);
    
    // Advanced configuration
    void setPayloadSize(uint8_t size);
    void enableDynamicPayloads(bool enable = true);
    void enableAutoAck(bool enable = true);
    void openWritingPipe(uint64_t address);
    void openReadingPipe(uint8_t pipe, uint64_t address);
    void startListening();
    void stopListening();
    
    // Utility methods
    void powerUp();
    void powerDown();
    bool testConnection();
    void scanChannels(); // Scan for interference
    uint8_t getOptimalChannel(); // Find best channel
    
    // ========== PROFILE MANAGEMENT SYSTEM ==========
    
    // Profile creation and management
    uint8_t createProfile(const char* name);
    bool selectProfile(uint8_t profileIndex);
    bool selectProfile(const char* name);
    void enableProfile(uint8_t profileIndex, bool enable = true);
    void deleteProfile(uint8_t profileIndex);
    
    // Simple channel mapping (EASY TO USE!)
    void mapJoystickToChannel(uint8_t joystickId, bool xAxis, uint8_t channel, 
                             int16_t minVal = -100, int16_t maxVal = 100);
    void mapLeverToChannel(uint8_t leverId, uint8_t channel, 
                          int16_t minVal = 0, int16_t maxVal = 100);
    
    // Advanced mapping with conditions
    void mapJoystickConditional(uint8_t joystickId, bool xAxis, uint8_t channel,
                               uint8_t conditionLever, uint8_t leverPosition,
                               int16_t minVal, int16_t maxVal);
    
    // Channel value access
    int16_t getChannelValue(uint8_t channel);
    void setChannelValue(uint8_t channel, int16_t value);
    bool isChannelUpdated(uint8_t channel);
    void clearChannelUpdated(uint8_t channel);
    
    // Auto-execution system (SET AND FORGET!)
    void enableAutoExecution(bool enable = true, unsigned long interval = 50);
    void executeProfiles(); // Call this once in loop() and forget!
    
    // Profile status and debugging
    void printProfile(uint8_t profileIndex);
    void printAllProfiles();
    void printChannelValues();
    const char* getProfileName(uint8_t profileIndex);
    uint8_t getActiveProfile() { return _activeProfile; }
    
    // Quick setup helpers (SUPER EASY!)
    void quickSetupBasic();           // Basic 1 joystick, 1 lever setup
    void quickSetupDrone();           // Drone control setup
    void quickSetupCar();             // RC car setup
    void quickSetupPlane();           // RC plane setup
    
    // Getters
    uint8_t getChannel() { return _channel; }
    PowerLevel getPowerLevel() { return _powerLevel; }
    DataRate getDataRate() { return _dataRate; }
    uint8_t getJoystickCount() { return _joystickCount; }
    uint8_t getLeverCount() { return _leverCount; }
    bool getAutoSend() { return _autoSend; }
    unsigned long getSendInterval() { return _sendInterval; }
    uint8_t getProfileCount() { return _profileCount; }
    
    // ========== CONFIGURATION AND EEPROM MANAGEMENT ==========
    
    // EEPROM profile management
    bool saveProfilesToEEPROM(uint16_t startAddress = 0);
    bool loadProfilesFromEEPROM(uint16_t startAddress = 0);
    void clearEEPROMProfiles(uint16_t startAddress = 0);
    bool validateEEPROMData(uint16_t startAddress = 0);
    
    // Configuration management
    bool loadSystemConfig(const char* configData);
    void applySystemConfig(const SystemConfig& config);
    void printSystemConfig(const SystemConfig& config);
    SystemConfig getDefaultConfig();
    
    // Auto-configuration from config data
    bool autoConfigureFromString(const char* configData);
    void initializeFromConfig(const SystemConfig& config);
    
    // Calibration helpers
    void saveJoystickCalibration(uint8_t joystickId);
    void saveLeverCalibration(uint8_t leverId);
    void loadCalibrations();
    
    // Backup and restore
    void backupCurrentSettings();
    bool restoreSettings();
    void factoryReset();
};

#endif // NRF24CONTROLLER_H