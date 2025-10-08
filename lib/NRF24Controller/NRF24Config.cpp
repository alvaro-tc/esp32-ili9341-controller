/**
 * NRF24Controller Configuration Library Implementation
 * 
 * Author: GitHub Copilot
 * Date: 2025
 */

#include "NRF24Config.h"

SystemConfig NRF24Config::_currentConfig;

// ========== PREDEFINED CONFIGURATION STRINGS ==========

namespace NRF24Configs {
    
    const char* BASIC_CONFIG = R"(
# NRF24Controller Basic Configuration
# Lines starting with # are comments

# System Settings
SYSTEM_NAME=Basic Controller
DEBUG_MODE=false
TRANSMISSION_INTERVAL=50

# NRF24L01 Settings
NRF_CE_PIN=9
NRF_CSN_PIN=10
NRF_CHANNEL=76
NRF_POWER=HIGH
NRF_DATA_RATE=1MBPS
NRF_TX_ADDRESS=0xE8E8F0F0E1
NRF_RX_ADDRESS=0xE8E8F0F0E2

# Joystick 0 Configuration (Main Stick)
JOY0_ENABLED=true
JOY0_NAME=MainStick
JOY0_PIN_X=A0
JOY0_PIN_Y=A1
JOY0_PIN_BUTTON=2
JOY0_MIN_X=0
JOY0_MAX_X=4095
JOY0_CENTER_X=2048
JOY0_MIN_Y=0
JOY0_MAX_Y=4095
JOY0_CENTER_Y=2048
JOY0_DEAD_ZONE=60
JOY0_INVERT_X=false
JOY0_INVERT_Y=false
JOY0_SMOOTHING=0.2

# Lever 0 Configuration (Throttle)
LEV0_ENABLED=true
LEV0_NAME=Throttle
LEV0_TYPE=ANALOG
LEV0_PIN_A=A2
LEV0_PIN_B=255
LEV0_PIN_BUTTON=255
LEV0_MIN_POS=0
LEV0_MAX_POS=4095
LEV0_CENTER_POS=0
LEV0_DEAD_ZONE=30
LEV0_INVERT=false
LEV0_SMOOTHING=0.1
)";

    const char* DRONE_CONFIG = R"(
# NRF24Controller Drone Configuration
# Optimized for quadcopter/drone control

SYSTEM_NAME=Drone Controller
DEBUG_MODE=false
TRANSMISSION_INTERVAL=20

# NRF24L01 Settings (High frequency for drones)
NRF_CE_PIN=9
NRF_CSN_PIN=10
NRF_CHANNEL=76
NRF_POWER=HIGH
NRF_DATA_RATE=2MBPS
NRF_TX_ADDRESS=0xE8E8F0F0E1
NRF_RX_ADDRESS=0xE8E8F0F0E2

# Right Stick (Roll/Pitch)
JOY0_ENABLED=true
JOY0_NAME=RightStick
JOY0_PIN_X=A0
JOY0_PIN_Y=A1
JOY0_PIN_BUTTON=2
JOY0_MIN_X=100
JOY0_MAX_X=3995
JOY0_CENTER_X=2048
JOY0_MIN_Y=100
JOY0_MAX_Y=3995
JOY0_CENTER_Y=2048
JOY0_DEAD_ZONE=40
JOY0_INVERT_X=false
JOY0_INVERT_Y=true
JOY0_SMOOTHING=0.15

# Left Stick (Throttle/Yaw)
JOY1_ENABLED=true
JOY1_NAME=LeftStick
JOY1_PIN_X=A2
JOY1_PIN_Y=A3
JOY1_PIN_BUTTON=3
JOY1_MIN_X=100
JOY1_MAX_X=3995
JOY1_CENTER_X=2048
JOY1_MIN_Y=100
JOY1_MAX_Y=3995
JOY1_CENTER_Y=100
JOY1_DEAD_ZONE=30
JOY1_INVERT_X=false
JOY1_INVERT_Y=false
JOY1_SMOOTHING=0.1

# Flight Mode Switch
LEV0_ENABLED=true
LEV0_NAME=FlightMode
LEV0_TYPE=DIGITAL
LEV0_PIN_A=4
LEV0_PIN_B=5
LEV0_PIN_BUTTON=255
LEV0_DIGITAL_POSITIONS=3
)";

    const char* CAR_CONFIG = R"(
# NRF24Controller RC Car Configuration

SYSTEM_NAME=RC Car Controller
DEBUG_MODE=false
TRANSMISSION_INTERVAL=50

# NRF24L01 Settings
NRF_CE_PIN=9
NRF_CSN_PIN=10
NRF_CHANNEL=82
NRF_POWER=HIGH
NRF_DATA_RATE=1MBPS
NRF_TX_ADDRESS=0xE8E8F0F0E1
NRF_RX_ADDRESS=0xE8E8F0F0E2

# Steering Wheel (X-axis only)
JOY0_ENABLED=true
JOY0_NAME=Steering
JOY0_PIN_X=A0
JOY0_PIN_Y=A1
JOY0_PIN_BUTTON=2
JOY0_MIN_X=0
JOY0_MAX_X=4095
JOY0_CENTER_X=2048
JOY0_MIN_Y=2048
JOY0_MAX_Y=2048
JOY0_CENTER_Y=2048
JOY0_DEAD_ZONE=80
JOY0_INVERT_X=false
JOY0_INVERT_Y=false
JOY0_SMOOTHING=0.3

# Throttle/Brake Lever
LEV0_ENABLED=true
LEV0_NAME=ThrottleBrake
LEV0_TYPE=ANALOG
LEV0_PIN_A=A2
LEV0_PIN_B=255
LEV0_PIN_BUTTON=255
LEV0_MIN_POS=0
LEV0_MAX_POS=4095
LEV0_CENTER_POS=2048
LEV0_DEAD_ZONE=50
LEV0_INVERT=false
LEV0_SMOOTHING=0.2

# Gear Selector
LEV1_ENABLED=true
LEV1_NAME=Gear
LEV1_TYPE=DIGITAL
LEV1_PIN_A=6
LEV1_PIN_B=7
LEV1_PIN_BUTTON=255
LEV1_DIGITAL_POSITIONS=3
)";

    const char* PLANE_CONFIG = R"(
# NRF24Controller RC Plane Configuration

SYSTEM_NAME=RC Plane Controller
DEBUG_MODE=false
TRANSMISSION_INTERVAL=50

# NRF24L01 Settings
NRF_CE_PIN=9
NRF_CSN_PIN=10
NRF_CHANNEL=88
NRF_POWER=MAX
NRF_DATA_RATE=1MBPS
NRF_TX_ADDRESS=0xE8E8F0F0E1
NRF_RX_ADDRESS=0xE8E8F0F0E2

# Primary Control Stick (Aileron/Elevator)
JOY0_ENABLED=true
JOY0_NAME=PrimaryStick
JOY0_PIN_X=A0
JOY0_PIN_Y=A1
JOY0_PIN_BUTTON=2
JOY0_MIN_X=50
JOY0_MAX_X=4045
JOY0_CENTER_X=2048
JOY0_MIN_Y=50
JOY0_MAX_Y=4045
JOY0_CENTER_Y=2048
JOY0_DEAD_ZONE=50
JOY0_INVERT_X=false
JOY0_INVERT_Y=true
JOY0_SMOOTHING=0.2

# Secondary Stick (Rudder/Throttle)
JOY1_ENABLED=true
JOY1_NAME=SecondaryStick
JOY1_PIN_X=A2
JOY1_PIN_Y=A3
JOY1_PIN_BUTTON=3
JOY1_MIN_X=50
JOY1_MAX_X=4045
JOY1_CENTER_X=2048
JOY1_MIN_Y=50
JOY1_MAX_Y=4045
JOY1_CENTER_Y=50
JOY1_DEAD_ZONE=40
JOY1_INVERT_X=false
JOY1_INVERT_Y=false
JOY1_SMOOTHING=0.15

# Trim Encoder
LEV0_ENABLED=true
LEV0_NAME=Trim
LEV0_TYPE=ENCODER
LEV0_PIN_A=4
LEV0_PIN_B=5
LEV0_PIN_BUTTON=6
LEV0_STEPS_PER_DETENT=2
LEV0_MIN_STEPS=-50
LEV0_MAX_STEPS=50

# Flight Mode
LEV1_ENABLED=true
LEV1_NAME=FlightMode
LEV1_TYPE=DIGITAL
LEV1_PIN_A=7
LEV1_PIN_B=8
LEV1_PIN_BUTTON=255
LEV1_DIGITAL_POSITIONS=3
)";

} // namespace NRF24Configs

// ========== CONFIGURATION LOADING FUNCTIONS ==========

SystemConfig NRF24Config::loadFromString(const char* configData) {
    SystemConfig config;
    
    // Start with default config
    NRF24Controller controller(9, 10);
    config = controller.getDefaultConfig();
    
    // Parse the configuration string
    char* workingData = strdup(configData);
    if (!workingData) {
        Serial.println("Memory allocation failed");
        return config;
    }
    
    char* line = strtok(workingData, "\n");
    while (line != nullptr) {
        parseConfigLine(config, line);
        line = strtok(nullptr, "\n");
    }
    
    free(workingData);
    return config;
}

SystemConfig NRF24Config::loadDefault() {
    return loadFromString(NRF24Configs::BASIC_CONFIG);
}

SystemConfig NRF24Config::loadDroneConfig() {
    return loadFromString(NRF24Configs::DRONE_CONFIG);
}

SystemConfig NRF24Config::loadCarConfig() {
    return loadFromString(NRF24Configs::CAR_CONFIG);
}

SystemConfig NRF24Config::loadPlaneConfig() {
    return loadFromString(NRF24Configs::PLANE_CONFIG);
}

// ========== CONFIGURATION HELPER FUNCTIONS ==========

void NRF24Config::configureJoystick(SystemConfig& config, uint8_t id, 
                                   uint8_t pinX, uint8_t pinY, uint8_t pinButton,
                                   const char* name) {
    if (id >= MAX_JOYSTICKS) return;
    
    auto& joy = config.joysticks[id];
    joy.enabled = true;
    joy.pinX = pinX;
    joy.pinY = pinY;
    joy.pinButton = pinButton;
    
    if (name) {
        strncpy(joy.name, name, 15);
        joy.name[15] = '\0';
    } else {
        snprintf(joy.name, 16, "Joystick%d", id + 1);
    }
    
    Serial.print("Configured joystick ");
    Serial.print(id);
    Serial.print(" (");
    Serial.print(joy.name);
    Serial.println(")");
}

void NRF24Config::configureLever(SystemConfig& config, uint8_t id, LeverType type,
                                uint8_t pinA, uint8_t pinB, uint8_t pinButton,
                                const char* name) {
    if (id >= MAX_LEVERS) return;
    
    auto& lever = config.levers[id];
    lever.enabled = true;
    lever.type = type;
    lever.pinA = pinA;
    lever.pinB = pinB;
    lever.pinButton = pinButton;
    
    if (name) {
        strncpy(lever.name, name, 15);
        lever.name[15] = '\0';
    } else {
        snprintf(lever.name, 16, "Lever%d", id + 1);
    }
    
    Serial.print("Configured lever ");
    Serial.print(id);
    Serial.print(" (");
    Serial.print(lever.name);
    Serial.println(")");
}

void NRF24Config::setJoystickCalibration(SystemConfig& config, uint8_t id,
                                        int minX, int maxX, int centerX,
                                        int minY, int maxY, int centerY,
                                        int deadZone) {
    if (id >= MAX_JOYSTICKS) return;
    
    auto& joy = config.joysticks[id];
    joy.minX = minX;
    joy.maxX = maxX;
    joy.centerX = centerX;
    joy.minY = minY;
    joy.maxY = maxY;
    joy.centerY = centerY;
    joy.deadZone = deadZone;
}

void NRF24Config::setLeverCalibration(SystemConfig& config, uint8_t id,
                                     int minPos, int maxPos, int centerPos,
                                     int deadZone) {
    if (id >= MAX_LEVERS) return;
    
    auto& lever = config.levers[id];
    lever.minPosition = minPos;
    lever.maxPosition = maxPos;
    lever.centerPosition = (centerPos == -1) ? (minPos + maxPos) / 2 : centerPos;
    lever.deadZone = deadZone;
}

void NRF24Config::configureNRF24(SystemConfig& config, uint8_t cePin, uint8_t csnPin,
                                 uint8_t channel, PowerLevel power) {
    config.nrfCEPin = cePin;
    config.nrfCSNPin = csnPin;
    config.nrfChannel = channel;
    config.nrfPowerLevel = power;
}

// ========== EASY SETUP FUNCTIONS ==========

SystemConfig NRF24Config::createBasicSetup(uint8_t cePin, uint8_t csnPin) {
    SystemConfig config = loadDefault();
    configureNRF24(config, cePin, csnPin);
    return config;
}

SystemConfig NRF24Config::createDroneSetup(uint8_t cePin, uint8_t csnPin) {
    SystemConfig config = loadDroneConfig();
    configureNRF24(config, cePin, csnPin);
    return config;
}

SystemConfig NRF24Config::createCarSetup(uint8_t cePin, uint8_t csnPin) {
    SystemConfig config = loadCarConfig();
    configureNRF24(config, cePin, csnPin);
    return config;
}

// ========== CONFIGURATION PARSING ==========

bool NRF24Config::parseConfigLine(SystemConfig& config, const char* line) {
    // Skip comments and empty lines
    if (!line || line[0] == '#' || line[0] == '\0' || line[0] == '\r' || line[0] == '\n') {
        return true;
    }
    
    // Create working copy
    char workLine[256];
    strncpy(workLine, line, 255);
    workLine[255] = '\0';
    
    // Remove whitespace
    char* trimmed = workLine;
    while (*trimmed == ' ' || *trimmed == '\t') trimmed++;
    
    // Find equals sign
    char* equals = strchr(trimmed, '=');
    if (!equals) return true;
    
    *equals = '\0';
    char* key = trimmed;
    char* value = equals + 1;
    
    // Remove trailing whitespace from key
    char* keyEnd = key + strlen(key) - 1;
    while (keyEnd > key && (*keyEnd == ' ' || *keyEnd == '\t')) {
        *keyEnd = '\0';
        keyEnd--;
    }
    
    // Remove leading whitespace from value
    while (*value == ' ' || *value == '\t') value++;
    
    // Parse key-value pairs
    if (strcmp(key, "SYSTEM_NAME") == 0) {
        strncpy(config.systemName, value, 31);
        config.systemName[31] = '\0';
    } else if (strcmp(key, "DEBUG_MODE") == 0) {
        config.debugMode = (strcmp(value, "true") == 0);
    } else if (strcmp(key, "TRANSMISSION_INTERVAL") == 0) {
        config.transmissionInterval = atol(value);
    }
    // NRF24 settings
    else if (strcmp(key, "NRF_CE_PIN") == 0) {
        config.nrfCEPin = atoi(value);
    } else if (strcmp(key, "NRF_CSN_PIN") == 0) {
        config.nrfCSNPin = atoi(value);
    } else if (strcmp(key, "NRF_CHANNEL") == 0) {
        config.nrfChannel = atoi(value);
    } else if (strcmp(key, "NRF_POWER") == 0) {
        if (strcmp(value, "MIN") == 0) config.nrfPowerLevel = POWER_MIN;
        else if (strcmp(value, "LOW") == 0) config.nrfPowerLevel = POWER_LOW;
        else if (strcmp(value, "HIGH") == 0) config.nrfPowerLevel = POWER_HIGH;
        else if (strcmp(value, "MAX") == 0) config.nrfPowerLevel = POWER_MAX;
    }
    // Add joystick parsing (JOY0_*, JOY1_*, etc.)
    else if (strncmp(key, "JOY", 3) == 0 && key[4] == '_') {
        uint8_t joyId = key[3] - '0';
        if (joyId < MAX_JOYSTICKS) {
            char* param = key + 5;
            auto& joy = config.joysticks[joyId];
            
            if (strcmp(param, "ENABLED") == 0) {
                joy.enabled = (strcmp(value, "true") == 0);
            } else if (strcmp(param, "NAME") == 0) {
                strncpy(joy.name, value, 15);
                joy.name[15] = '\0';
            } else if (strcmp(param, "PIN_X") == 0) {
                if (value[0] == 'A') joy.pinX = A0 + atoi(value + 1);
                else joy.pinX = atoi(value);
            } else if (strcmp(param, "PIN_Y") == 0) {
                if (value[0] == 'A') joy.pinY = A0 + atoi(value + 1);
                else joy.pinY = atoi(value);
            } else if (strcmp(param, "PIN_BUTTON") == 0) {
                joy.pinButton = atoi(value);
            } else if (strcmp(param, "DEAD_ZONE") == 0) {
                joy.deadZone = atoi(value);
            } else if (strcmp(param, "CENTER_X") == 0) {
                joy.centerX = atoi(value);
            } else if (strcmp(param, "CENTER_Y") == 0) {
                joy.centerY = atoi(value);
            } else if (strcmp(param, "MIN_X") == 0) {
                joy.minX = atoi(value);
            } else if (strcmp(param, "MAX_X") == 0) {
                joy.maxX = atoi(value);
            } else if (strcmp(param, "MIN_Y") == 0) {
                joy.minY = atoi(value);
            } else if (strcmp(param, "MAX_Y") == 0) {
                joy.maxY = atoi(value);
            } else if (strcmp(param, "INVERT_X") == 0) {
                joy.invertX = (strcmp(value, "true") == 0);
            } else if (strcmp(param, "INVERT_Y") == 0) {
                joy.invertY = (strcmp(value, "true") == 0);
            } else if (strcmp(param, "SMOOTHING") == 0) {
                joy.smoothingFactor = atof(value);
            }
        }
    }
    // Add lever parsing (LEV0_*, LEV1_*, etc.)
    else if (strncmp(key, "LEV", 3) == 0 && key[4] == '_') {
        uint8_t levId = key[3] - '0';
        if (levId < MAX_LEVERS) {
            char* param = key + 5;
            auto& lever = config.levers[levId];
            
            if (strcmp(param, "ENABLED") == 0) {
                lever.enabled = (strcmp(value, "true") == 0);
            } else if (strcmp(param, "NAME") == 0) {
                strncpy(lever.name, value, 15);
                lever.name[15] = '\0';
            } else if (strcmp(param, "TYPE") == 0) {
                if (strcmp(value, "ANALOG") == 0) lever.type = ANALOG_LEVER;
                else if (strcmp(value, "ENCODER") == 0) lever.type = ROTARY_ENCODER;
                else if (strcmp(value, "DIGITAL") == 0) lever.type = DIGITAL_LEVER;
            } else if (strcmp(param, "PIN_A") == 0) {
                if (value[0] == 'A') lever.pinA = A0 + atoi(value + 1);
                else lever.pinA = atoi(value);
            } else if (strcmp(param, "PIN_B") == 0) {
                lever.pinB = atoi(value);
            } else if (strcmp(param, "PIN_BUTTON") == 0) {
                lever.pinButton = atoi(value);
            } else if (strcmp(param, "DIGITAL_POSITIONS") == 0) {
                lever.digitalPositions = atoi(value);
            } else if (strcmp(param, "STEPS_PER_DETENT") == 0) {
                lever.stepsPerDetent = atoi(value);
            }
        }
    }
    
    return true;
}

bool NRF24Config::validateConfig(const SystemConfig& config) {
    // Basic validation
    if (config.nrfCEPin == config.nrfCSNPin) {
        Serial.println("Error: CE and CSN pins cannot be the same");
        return false;
    }
    
    if (config.nrfChannel > 125) {
        Serial.println("Error: NRF channel must be 0-125");
        return false;
    }
    
    // Check for pin conflicts
    // ... add more validation as needed
    
    return true;
}

void NRF24Config::printConfig(const SystemConfig& config) {
    Serial.println("========== PARSED CONFIGURATION ==========");
    Serial.print("System Name: "); Serial.println(config.systemName);
    Serial.print("NRF CE Pin: "); Serial.println(config.nrfCEPin);
    Serial.print("NRF CSN Pin: "); Serial.println(config.nrfCSNPin);
    Serial.print("NRF Channel: "); Serial.println(config.nrfChannel);
    
    Serial.println("\nEnabled Joysticks:");
    for (uint8_t i = 0; i < MAX_JOYSTICKS; i++) {
        if (config.joysticks[i].enabled) {
            Serial.print("  ");
            Serial.print(config.joysticks[i].name);
            Serial.print(" (X=");
            Serial.print(config.joysticks[i].pinX);
            Serial.print(", Y=");
            Serial.print(config.joysticks[i].pinY);
            Serial.println(")");
        }
    }
    
    Serial.println("\nEnabled Levers:");
    for (uint8_t i = 0; i < MAX_LEVERS; i++) {
        if (config.levers[i].enabled) {
            Serial.print("  ");
            Serial.print(config.levers[i].name);
            Serial.print(" (Type=");
            Serial.print(config.levers[i].type);
            Serial.print(", Pin=");
            Serial.print(config.levers[i].pinA);
            Serial.println(")");
        }
    }
    
    Serial.println("==========================================");
}