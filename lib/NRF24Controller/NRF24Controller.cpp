/**
 * NRF24Controller Library Implementation
 * 
 * Author: GitHub Copilot
 * Date: 2025
 */

#include "NRF24Controller.h"

// Constructor
NRF24Controller::NRF24Controller(uint8_t cePin, uint8_t csnPin) {
    _cePin = cePin;
    _csnPin = csnPin;
    _radio = new RF24(cePin, csnPin);
    
    // Default configuration
    _channel = 76;
    _powerLevel = POWER_HIGH;
    _dataRate = RATE_1MBPS;
    _txAddress = 0xE8E8F0F0E1LL;
    _rxAddress = 0xE8E8F0F0E2LL;
    
    // Initialize control arrays
    _joystickCount = 0;
    _leverCount = 0;
    for (int i = 0; i < MAX_JOYSTICKS; i++) {
        _joysticks[i] = nullptr;
        _joystickEnabled[i] = true;
    }
    for (int i = 0; i < MAX_LEVERS; i++) {
        _levers[i] = nullptr;
        _leverEnabled[i] = true;
    }
    
    // Packet configuration
    _packetCounter = 0;
    _autoSend = false;
    _sendInterval = 50;
    _lastSendTime = 0;
    
    // Transmission settings
    _enableAck = true;
    _retryCount = 15;
    _retryDelay = 5;
    
    // Data filtering
    _joystickThreshold = 5;
    _leverThreshold = 5;
    _sendOnlyChanges = true;
    
    // Initialize stats
    memset(&_stats, 0, sizeof(_stats));
    
    // Initialize last data arrays
    memset(_lastJoystickData, 0, sizeof(_lastJoystickData));
    memset(_lastLeverData, 0, sizeof(_lastLeverData));
    
    // Initialize profile system
    _activeProfile = 0;
    _profileCount = 0;
    _lastProfileExecution = 0;
    memset(_channelValues, 0, sizeof(_channelValues));
    memset(_channelUpdated, false, sizeof(_channelUpdated));
    _initializeProfiles();
}

// Initialize the NRF24L01
bool NRF24Controller::begin() {
    if (!_radio->begin()) {
        Serial.println("NRF24Controller: Failed to initialize radio");
        return false;
    }
    
    _initializeRadio();
    
    Serial.println("NRF24Controller: Initialized successfully");
    return true;
}

void NRF24Controller::_initializeRadio() {
    _radio->setPALevel(_powerLevel);
    
    // Convertir DataRate personalizado a rf24_datarate_e
    rf24_datarate_e rf24_rate;
    switch (_dataRate) {
        case RATE_250KBPS: rf24_rate = RF24_250KBPS; break;
        case RATE_1MBPS: rf24_rate = RF24_1MBPS; break;
        case RATE_2MBPS: rf24_rate = RF24_2MBPS; break;
        default: rf24_rate = RF24_1MBPS; break;
    }
    _radio->setDataRate(rf24_rate);
    
    _radio->setChannel(_channel);
    _radio->enableAckPayload();
    _radio->setRetries(_retryDelay, _retryCount);
    _radio->setPayloadSize(sizeof(DataPacket));
    
    _radio->openWritingPipe(_txAddress);
    _radio->openReadingPipe(1, _rxAddress);
    
    _radio->powerUp();
    _radio->stopListening(); // Start in transmit mode by default
}

// Configuration methods
void NRF24Controller::setChannel(uint8_t channel) {
    _channel = constrain(channel, 0, 125);
    _radio->setChannel(_channel);
}

void NRF24Controller::setPowerLevel(PowerLevel level) {
    _powerLevel = level;
    _radio->setPALevel(_powerLevel);
}

void NRF24Controller::setDataRate(DataRate rate) {
    _dataRate = rate;
    
    // Convertir DataRate personalizado a rf24_datarate_e
    rf24_datarate_e rf24_rate;
    switch (_dataRate) {
        case RATE_250KBPS: rf24_rate = RF24_250KBPS; break;
        case RATE_1MBPS: rf24_rate = RF24_1MBPS; break;
        case RATE_2MBPS: rf24_rate = RF24_2MBPS; break;
        default: rf24_rate = RF24_1MBPS; break;
    }
    _radio->setDataRate(rf24_rate);
}

void NRF24Controller::setAddresses(uint64_t txAddr, uint64_t rxAddr) {
    _txAddress = txAddr;
    _rxAddress = rxAddr;
    _radio->openWritingPipe(_txAddress);
    _radio->openReadingPipe(1, _rxAddress);
}

void NRF24Controller::enableAckPayload(bool enable) {
    _enableAck = enable;
    if (enable) {
        _radio->enableAckPayload();
    } else {
        _radio->disableAckPayload();
    }
}

void NRF24Controller::setRetrySettings(uint8_t count, uint8_t delay) {
    _retryCount = constrain(count, 0, 15);
    _retryDelay = constrain(delay, 0, 15);
    _radio->setRetries(_retryDelay, _retryCount);
}

// Control management
bool NRF24Controller::addJoystick(Joystick* joystick, uint8_t id) {
    if (id >= MAX_JOYSTICKS || _joysticks[id] != nullptr) {
        return false;
    }
    
    _joysticks[id] = joystick;
    _joystickEnabled[id] = true;
    _joystickCount++;
    
    // Initialize last data
    _lastJoystickData[id].id = id;
    _lastJoystickData[id].type = CONTROL_JOYSTICK;
    
    return true;
}

bool NRF24Controller::addLever(Lever* lever, uint8_t id) {
    if (id >= MAX_LEVERS || _levers[id] != nullptr) {
        return false;
    }
    
    _levers[id] = lever;
    _leverEnabled[id] = true;
    _leverCount++;
    
    // Initialize last data
    _lastLeverData[id].id = id;
    switch (lever->getLeverType()) {
        case ANALOG_LEVER:
            _lastLeverData[id].type = CONTROL_LEVER_ANALOG;
            break;
        case ROTARY_ENCODER:
            _lastLeverData[id].type = CONTROL_LEVER_ENCODER;
            break;
        case DIGITAL_LEVER:
            _lastLeverData[id].type = CONTROL_LEVER_DIGITAL;
            break;
    }
    
    return true;
}

void NRF24Controller::removeJoystick(uint8_t id) {
    if (id < MAX_JOYSTICKS && _joysticks[id] != nullptr) {
        _joysticks[id] = nullptr;
        _joystickEnabled[id] = false;
        _joystickCount--;
    }
}

void NRF24Controller::removeLever(uint8_t id) {
    if (id < MAX_LEVERS && _levers[id] != nullptr) {
        _levers[id] = nullptr;
        _leverEnabled[id] = false;
        _leverCount--;
    }
}

void NRF24Controller::enableJoystick(uint8_t id, bool enable) {
    if (id < MAX_JOYSTICKS) {
        _joystickEnabled[id] = enable;
    }
}

void NRF24Controller::enableLever(uint8_t id, bool enable) {
    if (id < MAX_LEVERS) {
        _leverEnabled[id] = enable;
    }
}

// Transmission settings
void NRF24Controller::setAutoSend(bool enable, unsigned long interval) {
    _autoSend = enable;
    _sendInterval = interval;
}

void NRF24Controller::setSendThresholds(int joystickThreshold, int leverThreshold) {
    _joystickThreshold = joystickThreshold;
    _leverThreshold = leverThreshold;
}

void NRF24Controller::setSendOnlyChanges(bool enable) {
    _sendOnlyChanges = enable;
}

// Update control data
void NRF24Controller::_updateControlData() {
    clearPacket();
    
    // Add joystick data
    for (uint8_t i = 0; i < MAX_JOYSTICKS; i++) {
        if (_joysticks[i] != nullptr && _joystickEnabled[i]) {
            int16_t x = _joysticks[i]->readX();
            int16_t y = _joysticks[i]->readY();
            uint8_t flags = 0;
            
            if (_joysticks[i]->isPressed()) flags |= 0x01;
            if (_joysticks[i]->isNeutral()) flags |= 0x02;
            if (_joysticks[i]->isAtEdge()) flags |= 0x04;
            
            // Check if data changed significantly
            bool changed = !_sendOnlyChanges ||
                          abs(x - _lastJoystickData[i].valueX) >= _joystickThreshold ||
                          abs(y - _lastJoystickData[i].valueY) >= _joystickThreshold ||
                          flags != _lastJoystickData[i].flags;
            
            if (changed) {
                addToPacket(i, CONTROL_JOYSTICK, x, y, flags);
                _lastJoystickData[i].valueX = x;
                _lastJoystickData[i].valueY = y;
                _lastJoystickData[i].flags = flags;
            }
        }
    }
    
    // Add lever data
    for (uint8_t i = 0; i < MAX_LEVERS; i++) {
        if (_levers[i] != nullptr && _leverEnabled[i]) {
            int16_t position = _levers[i]->readPosition();
            int16_t velocity = (int16_t)_levers[i]->readVelocity();
            uint8_t flags = 0;
            
            if (_levers[i]->isPressed()) flags |= 0x01;
            if (_levers[i]->isAtCenter()) flags |= 0x02;
            if (_levers[i]->isAtMinimum()) flags |= 0x04;
            if (_levers[i]->isAtMaximum()) flags |= 0x08;
            if (_levers[i]->isMoving()) flags |= 0x10;
            
            // Check if data changed significantly
            bool changed = !_sendOnlyChanges ||
                          abs(position - _lastLeverData[i].valueX) >= _leverThreshold ||
                          flags != _lastLeverData[i].flags;
            
            if (changed) {
                addToPacket(i + 100, _lastLeverData[i].type, position, velocity, flags);
                _lastLeverData[i].valueX = position;
                _lastLeverData[i].valueY = velocity;
                _lastLeverData[i].flags = flags;
            }
        }
    }
}

// Check if any data has changed
bool NRF24Controller::_hasDataChanged() {
    for (uint8_t i = 0; i < MAX_JOYSTICKS; i++) {
        if (_joysticks[i] != nullptr && _joystickEnabled[i]) {
            int16_t x = _joysticks[i]->readX();
            int16_t y = _joysticks[i]->readY();
            
            if (abs(x - _lastJoystickData[i].valueX) >= _joystickThreshold ||
                abs(y - _lastJoystickData[i].valueY) >= _joystickThreshold) {
                return true;
            }
        }
    }
    
    for (uint8_t i = 0; i < MAX_LEVERS; i++) {
        if (_levers[i] != nullptr && _leverEnabled[i]) {
            int16_t position = _levers[i]->readPosition();
            
            if (abs(position - _lastLeverData[i].valueX) >= _leverThreshold) {
                return true;
            }
        }
    }
    
    return false;
}

// Calculate checksum
uint16_t NRF24Controller::_calculateChecksum(const DataPacket& packet) {
    uint16_t checksum = 0;
    const uint8_t* data = (const uint8_t*)&packet;
    
    // Calculate checksum for all data except the checksum field itself
    for (size_t i = 0; i < sizeof(DataPacket) - sizeof(uint16_t); i++) {
        checksum ^= data[i];
        checksum = (checksum << 1) | (checksum >> 15); // Rotate left
    }
    
    return checksum;
}

// Update transmission statistics
void NRF24Controller::_updateStats(bool success) {
    if (success) {
        _stats.packetsSent++;
    } else {
        _stats.packetsLost++;
    }
    
    _stats.lastTransmissionTime = millis();
    
    uint32_t totalAttempts = _stats.packetsSent + _stats.packetsLost;
    if (totalAttempts > 0) {
        _stats.successRate = (float)_stats.packetsSent / totalAttempts * 100.0;
    }
}

// Main update function - call this in loop()
void NRF24Controller::update() {
    // Update levers (important for encoders)
    for (uint8_t i = 0; i < MAX_LEVERS; i++) {
        if (_levers[i] != nullptr) {
            _levers[i]->update();
        }
    }
    
    // Auto-send if enabled
    if (_autoSend && (millis() - _lastSendTime >= _sendInterval)) {
        if (!_sendOnlyChanges || _hasDataChanged()) {
            sendData();
        }
        _lastSendTime = millis();
    }
}

// Send current control data
bool NRF24Controller::sendData() {
    _updateControlData();
    
    if (_currentPacket.controlCount == 0) {
        return true; // Nothing to send
    }
    
    _currentPacket.packetId = _packetCounter++;
    _currentPacket.timestamp = millis();
    _currentPacket.checksum = _calculateChecksum(_currentPacket);
    
    _radio->stopListening();
    bool result = _radio->write(&_currentPacket, sizeof(DataPacket));
    
    _updateStats(result);
    
    if (result) {
        Serial.print("Sent packet #");
        Serial.print(_currentPacket.packetId);
        Serial.print(" with ");
        Serial.print(_currentPacket.controlCount);
        Serial.println(" controls");
    }
    
    return result;
}

// Send custom control data
bool NRF24Controller::sendCustomData(const ControlData& data) {
    clearPacket();
    _currentPacket.controls[0] = data;
    _currentPacket.controlCount = 1;
    _currentPacket.packetId = _packetCounter++;
    _currentPacket.timestamp = millis();
    _currentPacket.checksum = _calculateChecksum(_currentPacket);
    
    _radio->stopListening();
    bool result = _radio->write(&_currentPacket, sizeof(DataPacket));
    _updateStats(result);
    
    return result;
}

// Send custom packet
bool NRF24Controller::sendCustomPacket(const DataPacket& packet) {
    _radio->stopListening();
    bool result = _radio->write(&packet, sizeof(DataPacket));
    _updateStats(result);
    
    return result;
}

// Check if data is available
bool NRF24Controller::available() {
    _radio->startListening();
    return _radio->available();
}

// Read received data
bool NRF24Controller::readData(DataPacket& packet) {
    if (!available()) {
        return false;
    }
    
    _radio->read(&packet, sizeof(DataPacket));
    bool result = true;
    
    if (result) {
        // Verify checksum
        uint16_t calculatedChecksum = _calculateChecksum(packet);
        if (calculatedChecksum == packet.checksum) {
            _stats.packetsReceived++;
            return true;
        } else {
            Serial.println("Checksum mismatch - packet corrupted");
            return false;
        }
    }
    
    return false;
}

// Read specific control data from last received packet
bool NRF24Controller::readControlData(uint8_t controlId, ControlData& data) {
    // This would require storing the last received packet
    // Implementation depends on your specific needs
    return false; // Placeholder
}

// Packet management
void NRF24Controller::clearPacket() {
    memset(&_currentPacket, 0, sizeof(DataPacket));
    _currentPacket.controlCount = 0;
}

void NRF24Controller::addToPacket(uint8_t controlId, ControlType type, int16_t valueX, int16_t valueY, uint8_t flags) {
    if (_currentPacket.controlCount >= 8) {
        return; // Packet full
    }
    
    ControlData& control = _currentPacket.controls[_currentPacket.controlCount];
    control.id = controlId;
    control.type = type;
    control.valueX = valueX;
    control.valueY = valueY;
    control.flags = flags;
    control.timestamp = millis();
    
    _currentPacket.controlCount++;
}

uint8_t NRF24Controller::getPacketSize() {
    return _currentPacket.controlCount;
}

// Status and diagnostics
bool NRF24Controller::isConnected() {
    return _radio->isChipConnected();
}

TransmissionStats NRF24Controller::getStats() {
    return _stats;
}

void NRF24Controller::resetStats() {
    memset(&_stats, 0, sizeof(_stats));
}

float NRF24Controller::getSignalQuality() {
    return _stats.successRate;
}

void NRF24Controller::printStatus() {
    Serial.println("=== NRF24Controller Status ===");
    Serial.print("Channel: "); Serial.println(_channel);
    Serial.print("Power Level: "); Serial.println(_powerLevel);
    Serial.print("Data Rate: "); Serial.println(_dataRate);
    Serial.print("Connected: "); Serial.println(isConnected() ? "Yes" : "No");
    Serial.print("Joysticks: "); Serial.println(_joystickCount);
    Serial.print("Levers: "); Serial.println(_leverCount);
    Serial.print("Auto Send: "); Serial.println(_autoSend ? "Enabled" : "Disabled");
    Serial.print("Send Interval: "); Serial.print(_sendInterval); Serial.println("ms");
    Serial.print("Packets Sent: "); Serial.println(_stats.packetsSent);
    Serial.print("Packets Lost: "); Serial.println(_stats.packetsLost);
    Serial.print("Success Rate: "); Serial.print(_stats.successRate, 1); Serial.println("%");
}

void NRF24Controller::printPacket(const DataPacket& packet) {
    Serial.print("Packet ID: "); Serial.println(packet.packetId);
    Serial.print("Controls: "); Serial.println(packet.controlCount);
    Serial.print("Timestamp: "); Serial.println(packet.timestamp);
    
    for (uint8_t i = 0; i < packet.controlCount; i++) {
        const ControlData& control = packet.controls[i];
        Serial.print("  Control "); Serial.print(control.id);
        Serial.print(" Type: "); Serial.print(control.type);
        Serial.print(" X: "); Serial.print(control.valueX);
        Serial.print(" Y: "); Serial.print(control.valueY);
        Serial.print(" Flags: 0x"); Serial.println(control.flags, HEX);
    }
}

// Advanced configuration methods
void NRF24Controller::setPayloadSize(uint8_t size) {
    _radio->setPayloadSize(constrain(size, 1, 32));
}

void NRF24Controller::enableDynamicPayloads(bool enable) {
    if (enable) {
        _radio->enableDynamicPayloads();
    }
}

void NRF24Controller::enableAutoAck(bool enable) {
    _radio->setAutoAck(enable);
}

void NRF24Controller::openWritingPipe(uint64_t address) {
    _txAddress = address;
    _radio->openWritingPipe(address);
}

void NRF24Controller::openReadingPipe(uint8_t pipe, uint64_t address) {
    _radio->openReadingPipe(pipe, address);
}

void NRF24Controller::startListening() {
    _radio->startListening();
}

void NRF24Controller::stopListening() {
    _radio->stopListening();
}

// Utility methods
void NRF24Controller::powerUp() {
    _radio->powerUp();
}

void NRF24Controller::powerDown() {
    _radio->powerDown();
}

bool NRF24Controller::testConnection() {
    return _radio->isChipConnected();
}

void NRF24Controller::scanChannels() {
    Serial.println("Scanning channels for interference...");
    
    _radio->startListening();
    _radio->stopListening();
    
    for (uint8_t channel = 0; channel <= 125; channel++) {
        _radio->setChannel(channel);
        _radio->startListening();
        delayMicroseconds(128);
        bool result = _radio->testCarrier();
        _radio->stopListening();
        
        if (channel % 20 == 0) Serial.println();
        Serial.print(result ? "1" : "0");
    }
    
    Serial.println();
    _radio->setChannel(_channel); // Restore original channel
}

uint8_t NRF24Controller::getOptimalChannel() {
    uint8_t bestChannel = _channel;
    uint8_t minInterference = 255;
    
    for (uint8_t channel = 0; channel <= 125; channel += 5) {
        _radio->setChannel(channel);
        _radio->startListening();
        
        uint8_t interference = 0;
        for (int i = 0; i < 100; i++) {
            if (_radio->testCarrier()) {
                interference++;
            }
            delayMicroseconds(10);
        }
        
        _radio->stopListening();
        
        if (interference < minInterference) {
            minInterference = interference;
            bestChannel = channel;
        }
    }
    
    _radio->setChannel(_channel); // Restore original channel
    Serial.print("Optimal channel: ");
    Serial.print(bestChannel);
    Serial.print(" (interference level: ");
    Serial.print(minInterference);
    Serial.println(")");
    
    return bestChannel;
}

// ========== PROFILE MANAGEMENT SYSTEM IMPLEMENTATION ==========

void NRF24Controller::_initializeProfiles() {
    // Initialize all profiles
    for (uint8_t i = 0; i < 4; i++) {
        memset(&_profiles[i], 0, sizeof(ControlProfile));
        _profiles[i].enabled = false;
        _profiles[i].autoExecute = false;
        _profiles[i].executeInterval = 50;
        
        // Initialize mappings with default values
        for (uint8_t j = 0; j < MAX_JOYSTICKS; j++) {
            _profiles[i].joystickMappings[j][0] = {0, -100, 100, 0, false, 1.0, false}; // X axis
            _profiles[i].joystickMappings[j][1] = {1, -100, 100, 0, false, 1.0, false}; // Y axis
        }
        
        for (uint8_t j = 0; j < MAX_LEVERS; j++) {
            _profiles[i].leverMappings[j] = {(uint8_t)(j + 10), 0, 100, 50, false, 1.0, false};
        }
    }
}

uint8_t NRF24Controller::createProfile(const char* name) {
    if (_profileCount >= 4) {
        Serial.println("Maximum profiles reached (4)");
        return 255; // Error
    }
    
    uint8_t index = _profileCount++;
    strncpy(_profiles[index].name, name, 15);
    _profiles[index].name[15] = '\0';
    _profiles[index].enabled = true;
    _profiles[index].autoExecute = true;
    _profiles[index].executeInterval = 50;
    _profiles[index].conditionalCount = 0;
    
    Serial.print("Created profile '");
    Serial.print(name);
    Serial.print("' at index ");
    Serial.println(index);
    
    return index;
}

bool NRF24Controller::selectProfile(uint8_t profileIndex) {
    if (profileIndex >= _profileCount) {
        Serial.println("Profile index out of range");
        return false;
    }
    
    _activeProfile = profileIndex;
    Serial.print("Active profile: ");
    Serial.println(_profiles[profileIndex].name);
    return true;
}

bool NRF24Controller::selectProfile(const char* name) {
    for (uint8_t i = 0; i < _profileCount; i++) {
        if (strcmp(_profiles[i].name, name) == 0) {
            return selectProfile(i);
        }
    }
    
    Serial.print("Profile '");
    Serial.print(name);
    Serial.println("' not found");
    return false;
}

void NRF24Controller::enableProfile(uint8_t profileIndex, bool enable) {
    if (profileIndex < _profileCount) {
        _profiles[profileIndex].enabled = enable;
    }
}

void NRF24Controller::deleteProfile(uint8_t profileIndex) {
    if (profileIndex >= _profileCount) return;
    
    // Shift profiles down
    for (uint8_t i = profileIndex; i < _profileCount - 1; i++) {
        _profiles[i] = _profiles[i + 1];
    }
    _profileCount--;
    
    // Adjust active profile if necessary
    if (_activeProfile >= _profileCount && _profileCount > 0) {
        _activeProfile = _profileCount - 1;
    }
}

// SIMPLE MAPPING FUNCTIONS (SUPER EASY TO USE!)
void NRF24Controller::mapJoystickToChannel(uint8_t joystickId, bool xAxis, uint8_t channel, 
                                          int16_t minVal, int16_t maxVal) {
    if (joystickId >= MAX_JOYSTICKS || _activeProfile >= _profileCount) return;
    
    ControlMapping* mapping = &_profiles[_activeProfile].joystickMappings[joystickId][xAxis ? 0 : 1];
    mapping->outputChannel = channel;
    mapping->minValue = minVal;
    mapping->maxValue = maxVal;
    mapping->centerValue = (minVal + maxVal) / 2;
    mapping->invertOutput = false;
    mapping->scaleFactor = 1.0;
    mapping->enabled = true;
    
    Serial.print("Mapped Joystick ");
    Serial.print(joystickId);
    Serial.print(xAxis ? " X-axis" : " Y-axis");
    Serial.print(" to channel ");
    Serial.print(channel);
    Serial.print(" (");
    Serial.print(minVal);
    Serial.print(" to ");
    Serial.print(maxVal);
    Serial.println(")");
}

void NRF24Controller::mapLeverToChannel(uint8_t leverId, uint8_t channel, 
                                       int16_t minVal, int16_t maxVal) {
    if (leverId >= MAX_LEVERS || _activeProfile >= _profileCount) return;
    
    ControlMapping* mapping = &_profiles[_activeProfile].leverMappings[leverId];
    mapping->outputChannel = channel;
    mapping->minValue = minVal;
    mapping->maxValue = maxVal;
    mapping->centerValue = (minVal + maxVal) / 2;
    mapping->invertOutput = false;
    mapping->scaleFactor = 1.0;
    mapping->enabled = true;
    
    Serial.print("Mapped Lever ");
    Serial.print(leverId);
    Serial.print(" to channel ");
    Serial.print(channel);
    Serial.print(" (");
    Serial.print(minVal);
    Serial.print(" to ");
    Serial.print(maxVal);
    Serial.println(")");
}

// CONDITIONAL MAPPING (ADVANCED FEATURE!)
void NRF24Controller::mapJoystickConditional(uint8_t joystickId, bool xAxis, uint8_t channel,
                                            uint8_t conditionLever, uint8_t leverPosition,
                                            int16_t minVal, int16_t maxVal) {
    if (_activeProfile >= _profileCount || _profiles[_activeProfile].conditionalCount >= 8) {
        return;
    }
    
    ConditionalMapping* conditional = &_profiles[_activeProfile].conditionalMappings[_profiles[_activeProfile].conditionalCount++];
    conditional->conditionLeverId = conditionLever;
    conditional->conditionPosition = leverPosition;
    conditional->mapping.outputChannel = channel;
    conditional->mapping.minValue = minVal;
    conditional->mapping.maxValue = maxVal;
    conditional->mapping.centerValue = (minVal + maxVal) / 2;
    conditional->mapping.invertOutput = false;
    conditional->mapping.scaleFactor = 1.0;
    conditional->mapping.enabled = true;
    
    Serial.print("Conditional: Joystick ");
    Serial.print(joystickId);
    Serial.print(xAxis ? " X" : " Y");
    Serial.print(" -> Channel ");
    Serial.print(channel);
    Serial.print(" when Lever ");
    Serial.print(conditionLever);
    Serial.print(" = ");
    Serial.println(leverPosition);
}

int16_t NRF24Controller::_applyMapping(int16_t inputValue, const ControlMapping& mapping) {
    if (!mapping.enabled) return 0;
    
    // Apply scale factor
    float scaledValue = inputValue * mapping.scaleFactor;
    
    // Map from input range to output range
    int16_t mappedValue = map(scaledValue, -100, 100, mapping.minValue, mapping.maxValue);
    
    // Apply inversion if needed
    if (mapping.invertOutput) {
        mappedValue = mapping.maxValue - mappedValue + mapping.minValue;
    }
    
    return mappedValue;
}

bool NRF24Controller::_checkConditionalMapping(const ConditionalMapping& conditional) {
    if (conditional.conditionLeverId >= MAX_LEVERS || _levers[conditional.conditionLeverId] == nullptr) {
        return false;
    }
    
    // Check if lever is in the required position
    int leverPosition = _levers[conditional.conditionLeverId]->getDigitalPosition();
    return (leverPosition == conditional.conditionPosition);
}

void NRF24Controller::_updateChannelValues() {
    ControlProfile& profile = _profiles[_activeProfile];
    
    // Clear updated flags
    memset(_channelUpdated, false, sizeof(_channelUpdated));
    
    // Process joystick mappings
    for (uint8_t i = 0; i < MAX_JOYSTICKS; i++) {
        if (_joysticks[i] != nullptr && _joystickEnabled[i]) {
            // X axis
            if (profile.joystickMappings[i][0].enabled) {
                int16_t value = _joysticks[i]->readX();
                int16_t mappedValue = _applyMapping(value, profile.joystickMappings[i][0]);
                uint8_t channel = profile.joystickMappings[i][0].outputChannel;
                _channelValues[channel] = mappedValue;
                _channelUpdated[channel] = true;
            }
            
            // Y axis
            if (profile.joystickMappings[i][1].enabled) {
                int16_t value = _joysticks[i]->readY();
                int16_t mappedValue = _applyMapping(value, profile.joystickMappings[i][1]);
                uint8_t channel = profile.joystickMappings[i][1].outputChannel;
                _channelValues[channel] = mappedValue;
                _channelUpdated[channel] = true;
            }
        }
    }
    
    // Process lever mappings
    for (uint8_t i = 0; i < MAX_LEVERS; i++) {
        if (_levers[i] != nullptr && _leverEnabled[i] && profile.leverMappings[i].enabled) {
            int16_t value = _levers[i]->readPosition();
            int16_t mappedValue = _applyMapping(value, profile.leverMappings[i]);
            uint8_t channel = profile.leverMappings[i].outputChannel;
            _channelValues[channel] = mappedValue;
            _channelUpdated[channel] = true;
        }
    }
    
    // Process conditional mappings
    for (uint8_t i = 0; i < profile.conditionalCount; i++) {
        if (_checkConditionalMapping(profile.conditionalMappings[i])) {
            // Apply conditional mapping
            // This is a simplified version - you might want to extend this
            uint8_t channel = profile.conditionalMappings[i].mapping.outputChannel;
            // Apply the conditional limits to existing channel value
            int16_t currentValue = _channelValues[channel];
            int16_t newMin = profile.conditionalMappings[i].mapping.minValue;
            int16_t newMax = profile.conditionalMappings[i].mapping.maxValue;
            _channelValues[channel] = map(currentValue, -100, 100, newMin, newMax);
            _channelUpdated[channel] = true;
        }
    }
}

void NRF24Controller::_executeActiveProfile() {
    if (_activeProfile >= _profileCount || !_profiles[_activeProfile].enabled) {
        return;
    }
    
    ControlProfile& profile = _profiles[_activeProfile];
    
    if (!profile.autoExecute) return;
    
    unsigned long currentTime = millis();
    if (currentTime - _lastProfileExecution < profile.executeInterval) {
        return;
    }
    
    // Update channel values based on current profile
    _updateChannelValues();
    
    // Create and send packet with channel data
    clearPacket();
    
    // Add updated channels to packet
    for (uint8_t i = 0; i < 32; i++) {
        if (_channelUpdated[i]) {
            addToPacket(i, CONTROL_CUSTOM, _channelValues[i], 0, 0);
        }
    }
    
    // Send the data
    if (_currentPacket.controlCount > 0) {
        sendData();
    }
    
    _lastProfileExecution = currentTime;
}

// CHANNEL ACCESS METHODS
int16_t NRF24Controller::getChannelValue(uint8_t channel) {
    if (channel < 32) {
        return _channelValues[channel];
    }
    return 0;
}

void NRF24Controller::setChannelValue(uint8_t channel, int16_t value) {
    if (channel < 32) {
        _channelValues[channel] = value;
        _channelUpdated[channel] = true;
    }
}

bool NRF24Controller::isChannelUpdated(uint8_t channel) {
    if (channel < 32) {
        return _channelUpdated[channel];
    }
    return false;
}

void NRF24Controller::clearChannelUpdated(uint8_t channel) {
    if (channel < 32) {
        _channelUpdated[channel] = false;
    }
}

// AUTO-EXECUTION SYSTEM (SET AND FORGET!)
void NRF24Controller::enableAutoExecution(bool enable, unsigned long interval) {
    if (_activeProfile < _profileCount) {
        _profiles[_activeProfile].autoExecute = enable;
        _profiles[_activeProfile].executeInterval = interval;
    }
}

void NRF24Controller::executeProfiles() {
    // Update all levers first
    for (uint8_t i = 0; i < MAX_LEVERS; i++) {
        if (_levers[i] != nullptr) {
            _levers[i]->update();
        }
    }
    
    // Execute active profile
    _executeActiveProfile();
}

// DEBUG AND STATUS METHODS
void NRF24Controller::printProfile(uint8_t profileIndex) {
    if (profileIndex >= _profileCount) return;
    
    ControlProfile& profile = _profiles[profileIndex];
    
    Serial.println("===============================");
    Serial.print("Profile: ");
    Serial.println(profile.name);
    Serial.print("Enabled: ");
    Serial.println(profile.enabled ? "Yes" : "No");
    Serial.print("Auto Execute: ");
    Serial.println(profile.autoExecute ? "Yes" : "No");
    Serial.print("Interval: ");
    Serial.print(profile.executeInterval);
    Serial.println("ms");
    
    Serial.println("Joystick Mappings:");
    for (uint8_t i = 0; i < MAX_JOYSTICKS; i++) {
        if (profile.joystickMappings[i][0].enabled || profile.joystickMappings[i][1].enabled) {
            Serial.print("  Joystick ");
            Serial.print(i);
            Serial.print(":");
            if (profile.joystickMappings[i][0].enabled) {
                Serial.print(" X->Ch");
                Serial.print(profile.joystickMappings[i][0].outputChannel);
            }
            if (profile.joystickMappings[i][1].enabled) {
                Serial.print(" Y->Ch");
                Serial.print(profile.joystickMappings[i][1].outputChannel);
            }
            Serial.println();
        }
    }
    
    Serial.println("Lever Mappings:");
    for (uint8_t i = 0; i < MAX_LEVERS; i++) {
        if (profile.leverMappings[i].enabled) {
            Serial.print("  Lever ");
            Serial.print(i);
            Serial.print(" -> Channel ");
            Serial.println(profile.leverMappings[i].outputChannel);
        }
    }
    
    if (profile.conditionalCount > 0) {
        Serial.println("Conditional Mappings:");
        for (uint8_t i = 0; i < profile.conditionalCount; i++) {
            Serial.print("  When Lever ");
            Serial.print(profile.conditionalMappings[i].conditionLeverId);
            Serial.print(" = ");
            Serial.print(profile.conditionalMappings[i].conditionPosition);
            Serial.print(" -> Channel ");
            Serial.println(profile.conditionalMappings[i].mapping.outputChannel);
        }
    }
    
    Serial.println("===============================");
}

void NRF24Controller::printAllProfiles() {
    Serial.print("Total Profiles: ");
    Serial.println(_profileCount);
    Serial.print("Active Profile: ");
    Serial.println(_activeProfile);
    
    for (uint8_t i = 0; i < _profileCount; i++) {
        printProfile(i);
    }
}

void NRF24Controller::printChannelValues() {
    Serial.println("=== Channel Values ===");
    for (uint8_t i = 0; i < 32; i++) {
        if (_channelUpdated[i]) {
            Serial.print("Ch");
            Serial.print(i);
            Serial.print(": ");
            Serial.print(_channelValues[i]);
            Serial.println();
        }
    }
}

const char* NRF24Controller::getProfileName(uint8_t profileIndex) {
    if (profileIndex < _profileCount) {
        return _profiles[profileIndex].name;
    }
    return "Invalid";
}

// QUICK SETUP HELPERS (SUPER EASY!)
void NRF24Controller::quickSetupBasic() {
    uint8_t profile = createProfile("Basic");
    selectProfile(profile);
    
    if (_joystickCount > 0) {
        mapJoystickToChannel(0, true, 0, -100, 100);   // X -> Channel 0
        mapJoystickToChannel(0, false, 1, -100, 100);  // Y -> Channel 1
    }
    
    if (_leverCount > 0) {
        mapLeverToChannel(0, 2, 0, 100);               // Lever -> Channel 2
    }
    
    enableAutoExecution(true, 50);
    Serial.println("Quick Setup: Basic configuration complete!");
}

void NRF24Controller::quickSetupDrone() {
    uint8_t profile = createProfile("Drone");
    selectProfile(profile);
    
    // Typical drone configuration
    mapJoystickToChannel(0, true, 0, -500, 500);      // Roll (aileron)
    mapJoystickToChannel(0, false, 1, -500, 500);     // Pitch (elevator)
    
    if (_joystickCount > 1) {
        mapJoystickToChannel(1, true, 3, -500, 500);   // Yaw (rudder)
    }
    
    if (_leverCount > 0) {
        mapLeverToChannel(0, 2, 1000, 2000);           // Throttle (1000-2000us)
    }
    
    enableAutoExecution(true, 20); // High frequency for drones
    Serial.println("Quick Setup: Drone configuration complete!");
}

void NRF24Controller::quickSetupCar() {
    uint8_t profile = createProfile("Car");
    selectProfile(profile);
    
    mapJoystickToChannel(0, true, 0, -100, 100);      // Steering
    
    if (_leverCount > 0) {
        mapLeverToChannel(0, 1, -100, 100);            // Throttle/Brake
    }
    
    enableAutoExecution(true, 50);
    Serial.println("Quick Setup: Car configuration complete!");
}

void NRF24Controller::quickSetupPlane() {
    uint8_t profile = createProfile("Plane");
    selectProfile(profile);
    
    mapJoystickToChannel(0, true, 0, -100, 100);      // Aileron
    mapJoystickToChannel(0, false, 1, -100, 100);     // Elevator
    
    if (_joystickCount > 1) {
        mapJoystickToChannel(1, true, 2, -100, 100);   // Rudder
    }
    
    if (_leverCount > 0) {
        mapLeverToChannel(0, 3, 0, 100);               // Throttle
    }
    
    enableAutoExecution(true, 50);
    Serial.println("Quick Setup: Plane configuration complete!");
}

// ========== CONFIGURATION AND EEPROM MANAGEMENT IMPLEMENTATION ==========

bool NRF24Controller::saveProfilesToEEPROM(uint16_t startAddress) {
    EEPROMProfileData eepromData;
    
    // Set magic number and version
    eepromData.magicNumber = 0x12345678;
    eepromData.version = 1;
    eepromData.profileCount = _profileCount;
    
    // Copy profiles
    for (uint8_t i = 0; i < 4; i++) {
        eepromData.profiles[i] = _profiles[i];
    }
    
    // Calculate checksum
    eepromData.checksum = 0;
    uint8_t* data = (uint8_t*)&eepromData;
    for (size_t i = 0; i < sizeof(EEPROMProfileData) - sizeof(uint16_t); i++) {
        eepromData.checksum ^= data[i];
    }
    
    // Write to EEPROM
    for (size_t i = 0; i < sizeof(EEPROMProfileData); i++) {
        EEPROM.write(startAddress + i, data[i]);
    }
    
    Serial.print("Profiles saved to EEPROM at address ");
    Serial.println(startAddress);
    return true;
}

bool NRF24Controller::loadProfilesFromEEPROM(uint16_t startAddress) {
    EEPROMProfileData eepromData;
    
    // Read from EEPROM
    uint8_t* data = (uint8_t*)&eepromData;
    for (size_t i = 0; i < sizeof(EEPROMProfileData); i++) {
        data[i] = EEPROM.read(startAddress + i);
    }
    
    // Validate magic number
    if (eepromData.magicNumber != 0x12345678) {
        Serial.println("Invalid EEPROM data - no valid profiles found");
        return false;
    }
    
    // Validate version
    if (eepromData.version != 1) {
        Serial.println("EEPROM version mismatch");
        return false;
    }
    
    // Validate checksum
    uint16_t calculatedChecksum = 0;
    for (size_t i = 0; i < sizeof(EEPROMProfileData) - sizeof(uint16_t); i++) {
        calculatedChecksum ^= data[i];
    }
    
    if (calculatedChecksum != eepromData.checksum) {
        Serial.println("EEPROM checksum mismatch - data corrupted");
        return false;
    }
    
    // Load profiles
    _profileCount = eepromData.profileCount;
    for (uint8_t i = 0; i < 4; i++) {
        _profiles[i] = eepromData.profiles[i];
    }
    
    Serial.print("Loaded ");
    Serial.print(_profileCount);
    Serial.println(" profiles from EEPROM");
    
    return true;
}

void NRF24Controller::clearEEPROMProfiles(uint16_t startAddress) {
    // Write zeros to clear EEPROM
    for (size_t i = 0; i < sizeof(EEPROMProfileData); i++) {
        EEPROM.write(startAddress + i, 0);
    }
    
    Serial.println("EEPROM profiles cleared");
}

bool NRF24Controller::validateEEPROMData(uint16_t startAddress) {
    uint32_t magicNumber;
    uint8_t* data = (uint8_t*)&magicNumber;
    
    for (uint8_t i = 0; i < 4; i++) {
        data[i] = EEPROM.read(startAddress + i);
    }
    
    return (magicNumber == 0x12345678);
}

SystemConfig NRF24Controller::getDefaultConfig() {
    SystemConfig config;
    
    // NRF24 default settings
    config.nrfCEPin = 9;
    config.nrfCSNPin = 10;
    config.nrfChannel = 76;
    config.nrfPowerLevel = POWER_HIGH;
    config.nrfDataRate = RATE_1MBPS;
    config.nrfTxAddress = 0xE8E8F0F0E1LL;
    config.nrfRxAddress = 0xE8E8F0F0E2LL;
    
    // Default joystick configurations
    for (uint8_t i = 0; i < MAX_JOYSTICKS; i++) {
        config.joysticks[i].enabled = false;
        config.joysticks[i].pinX = A0 + i * 2;
        config.joysticks[i].pinY = A1 + i * 2;
        config.joysticks[i].pinButton = 2 + i;
        config.joysticks[i].minX = 0;
        config.joysticks[i].maxX = 4095;
        config.joysticks[i].centerX = 2048;
        config.joysticks[i].minY = 0;
        config.joysticks[i].maxY = 4095;
        config.joysticks[i].centerY = 2048;
        config.joysticks[i].deadZone = 60;
        config.joysticks[i].invertX = false;
        config.joysticks[i].invertY = false;
        config.joysticks[i].smoothingFactor = 0.2;
        snprintf(config.joysticks[i].name, 16, "Joystick%d", i + 1);
    }
    
    // Default lever configurations
    for (uint8_t i = 0; i < MAX_LEVERS; i++) {
        config.levers[i].enabled = false;
        config.levers[i].type = ANALOG_LEVER;
        config.levers[i].pinA = A4 + i;
        config.levers[i].pinB = 255;
        config.levers[i].pinButton = 255;
        config.levers[i].minPosition = 0;
        config.levers[i].maxPosition = 4095;
        config.levers[i].centerPosition = 2048;
        config.levers[i].deadZone = 50;
        config.levers[i].invertDirection = false;
        config.levers[i].smoothingFactor = 0.1;
        config.levers[i].stepsPerDetent = 4;
        config.levers[i].digitalPositions = 3;
        snprintf(config.levers[i].name, 16, "Lever%d", i + 1);
    }
    
    // System settings
    strcpy(config.systemName, "NRF24Controller");
    config.debugMode = false;
    config.transmissionInterval = 50;
    
    return config;
}

bool NRF24Controller::loadSystemConfig(const char* configData) {
    // Parse configuration from string format
    // This is a simplified parser - you might want to use JSON or similar
    
    Serial.println("Parsing configuration data...");
    
    // Create a working copy
    char* workingData = strdup(configData);
    if (!workingData) {
        Serial.println("Memory allocation failed");
        return false;
    }
    
    SystemConfig config = getDefaultConfig();
    
    // Simple line-by-line parser
    char* line = strtok(workingData, "\n");
    while (line != nullptr) {
        // Skip comments and empty lines
        if (line[0] == '#' || line[0] == '\0' || line[0] == ' ') {
            line = strtok(nullptr, "\n");
            continue;
        }
        
        // Parse key-value pairs
        char* key = strtok(line, "=");
        char* value = strtok(nullptr, "=");
        
        if (key && value) {
            // NRF24 settings
            if (strcmp(key, "NRF_CE_PIN") == 0) {
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
            // Add more parsing logic as needed...
        }
        
        line = strtok(nullptr, "\n");
    }
    
    free(workingData);
    
    // Apply the configuration
    applySystemConfig(config);
    
    return true;
}

void NRF24Controller::applySystemConfig(const SystemConfig& config) {
    Serial.println("Applying system configuration...");
    
    // Apply NRF24 settings
    _cePin = config.nrfCEPin;
    _csnPin = config.nrfCSNPin;
    _channel = config.nrfChannel;
    _powerLevel = config.nrfPowerLevel;
    _dataRate = config.nrfDataRate;
    _txAddress = config.nrfTxAddress;
    _rxAddress = config.nrfRxAddress;
    
    // Note: Joystick and Lever configurations would need to be applied
    // during object creation, so this function mainly sets internal parameters
    
    Serial.print("Applied configuration for system: ");
    Serial.println(config.systemName);
}

void NRF24Controller::printSystemConfig(const SystemConfig& config) {
    Serial.println("========== SYSTEM CONFIGURATION ==========");
    Serial.print("System Name: "); Serial.println(config.systemName);
    Serial.print("Debug Mode: "); Serial.println(config.debugMode ? "ON" : "OFF");
    
    Serial.println("\n--- NRF24 Settings ---");
    Serial.print("CE Pin: "); Serial.println(config.nrfCEPin);
    Serial.print("CSN Pin: "); Serial.println(config.nrfCSNPin);
    Serial.print("Channel: "); Serial.println(config.nrfChannel);
    Serial.print("Power Level: "); Serial.println(config.nrfPowerLevel);
    Serial.print("Data Rate: "); Serial.println(config.nrfDataRate);
    
    Serial.println("\n--- Joystick Configurations ---");
    for (uint8_t i = 0; i < MAX_JOYSTICKS; i++) {
        if (config.joysticks[i].enabled) {
            Serial.print("Joystick "); Serial.print(i); Serial.print(" (");
            Serial.print(config.joysticks[i].name); Serial.println("):");
            Serial.print("  Pins: X="); Serial.print(config.joysticks[i].pinX);
            Serial.print(", Y="); Serial.print(config.joysticks[i].pinY);
            Serial.print(", Button="); Serial.println(config.joysticks[i].pinButton);
            Serial.print("  Limits X: "); Serial.print(config.joysticks[i].minX);
            Serial.print("-"); Serial.print(config.joysticks[i].maxX);
            Serial.print(", Center: "); Serial.println(config.joysticks[i].centerX);
            Serial.print("  Dead Zone: "); Serial.println(config.joysticks[i].deadZone);
        }
    }
    
    Serial.println("\n--- Lever Configurations ---");
    for (uint8_t i = 0; i < MAX_LEVERS; i++) {
        if (config.levers[i].enabled) {
            Serial.print("Lever "); Serial.print(i); Serial.print(" (");
            Serial.print(config.levers[i].name); Serial.println("):");
            Serial.print("  Type: "); Serial.println(config.levers[i].type);
            Serial.print("  Pins: A="); Serial.print(config.levers[i].pinA);
            if (config.levers[i].pinB != 255) {
                Serial.print(", B="); Serial.print(config.levers[i].pinB);
            }
            if (config.levers[i].pinButton != 255) {
                Serial.print(", Button="); Serial.print(config.levers[i].pinButton);
            }
            Serial.println();
        }
    }
    
    Serial.println("==========================================");
}

bool NRF24Controller::autoConfigureFromString(const char* configData) {
    SystemConfig config;
    if (loadSystemConfig(configData)) {
        initializeFromConfig(config);
        return true;
    }
    return false;
}

void NRF24Controller::initializeFromConfig(const SystemConfig& config) {
    // This would create and configure joysticks and levers based on config
    // Implementation depends on how you want to handle dynamic object creation
    Serial.println("Initializing system from configuration...");
    
    applySystemConfig(config);
    
    Serial.println("System initialization complete");
}

void NRF24Controller::saveJoystickCalibration(uint8_t joystickId) {
    if (joystickId >= MAX_JOYSTICKS || _joysticks[joystickId] == nullptr) {
        return;
    }
    
    // Save calibration data to EEPROM
    uint16_t address = 1000 + (joystickId * 20); // Offset for calibration data
    
    Joystick* joy = _joysticks[joystickId];
    EEPROM.write(address, joy->getMinX() & 0xFF);
    EEPROM.write(address + 1, (joy->getMinX() >> 8) & 0xFF);
    EEPROM.write(address + 2, joy->getMaxX() & 0xFF);
    EEPROM.write(address + 3, (joy->getMaxX() >> 8) & 0xFF);
    EEPROM.write(address + 4, joy->getCenterX() & 0xFF);
    EEPROM.write(address + 5, (joy->getCenterX() >> 8) & 0xFF);
    
    EEPROM.write(address + 10, joy->getMinY() & 0xFF);
    EEPROM.write(address + 11, (joy->getMinY() >> 8) & 0xFF);
    EEPROM.write(address + 12, joy->getMaxY() & 0xFF);
    EEPROM.write(address + 13, (joy->getMaxY() >> 8) & 0xFF);
    EEPROM.write(address + 14, joy->getCenterY() & 0xFF);
    EEPROM.write(address + 15, (joy->getCenterY() >> 8) & 0xFF);
    
    Serial.print("Saved calibration for joystick ");
    Serial.println(joystickId);
}

void NRF24Controller::saveLeverCalibration(uint8_t leverId) {
    if (leverId >= MAX_LEVERS || _levers[leverId] == nullptr) {
        return;
    }
    
    // Save lever calibration data
    uint16_t address = 1200 + (leverId * 20);
    
    Lever* lever = _levers[leverId];
    EEPROM.write(address, lever->getMinPosition() & 0xFF);
    EEPROM.write(address + 1, (lever->getMinPosition() >> 8) & 0xFF);
    EEPROM.write(address + 2, lever->getMaxPosition() & 0xFF);
    EEPROM.write(address + 3, (lever->getMaxPosition() >> 8) & 0xFF);
    EEPROM.write(address + 4, lever->getCenterPosition() & 0xFF);
    EEPROM.write(address + 5, (lever->getCenterPosition() >> 8) & 0xFF);
    
    Serial.print("Saved calibration for lever ");
    Serial.println(leverId);
}

void NRF24Controller::loadCalibrations() {
    // Load joystick calibrations
    for (uint8_t i = 0; i < MAX_JOYSTICKS; i++) {
        if (_joysticks[i] != nullptr) {
            uint16_t address = 1000 + (i * 20);
            
            int minX = EEPROM.read(address) | (EEPROM.read(address + 1) << 8);
            int maxX = EEPROM.read(address + 2) | (EEPROM.read(address + 3) << 8);
            int centerX = EEPROM.read(address + 4) | (EEPROM.read(address + 5) << 8);
            
            int minY = EEPROM.read(address + 10) | (EEPROM.read(address + 11) << 8);
            int maxY = EEPROM.read(address + 12) | (EEPROM.read(address + 13) << 8);
            int centerY = EEPROM.read(address + 14) | (EEPROM.read(address + 15) << 8);
            
            // Apply calibration if valid
            if (minX != 0 && maxX != 0) {
                _joysticks[i]->setLimits(minX, maxX, minY, maxY);
                _joysticks[i]->setCenter(centerX, centerY);
            }
        }
    }
    
    // Load lever calibrations
    for (uint8_t i = 0; i < MAX_LEVERS; i++) {
        if (_levers[i] != nullptr) {
            uint16_t address = 1200 + (i * 20);
            
            int minPos = EEPROM.read(address) | (EEPROM.read(address + 1) << 8);
            int maxPos = EEPROM.read(address + 2) | (EEPROM.read(address + 3) << 8);
            int centerPos = EEPROM.read(address + 4) | (EEPROM.read(address + 5) << 8);
            
            if (minPos != 0 && maxPos != 0) {
                _levers[i]->setAnalogLimits(minPos, maxPos, centerPos);
            }
        }
    }
    
    Serial.println("Calibrations loaded from EEPROM");
}

void NRF24Controller::backupCurrentSettings() {
    // Backup current profiles and settings
    saveProfilesToEEPROM(0);
    
    Serial.println("Current settings backed up to EEPROM");
}

bool NRF24Controller::restoreSettings() {
    if (validateEEPROMData(0)) {
        loadProfilesFromEEPROM(0);
        loadCalibrations();
        Serial.println("Settings restored from EEPROM");
        return true;
    }
    
    Serial.println("No valid backup found in EEPROM");
    return false;
}

void NRF24Controller::factoryReset() {
    // Clear all EEPROM data
    clearEEPROMProfiles(0);
    
    // Reset to default profiles
    _initializeProfiles();
    _activeProfile = 0;
    _profileCount = 0;
    
    Serial.println("Factory reset complete");
}