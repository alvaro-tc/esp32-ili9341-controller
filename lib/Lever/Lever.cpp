/**
 * Lever Library Implementation
 * 
 * Author: GitHub Copilot
 * Date: 2025
 */

#include "Lever.h"

// Constructor
Lever::Lever(LeverType type, uint8_t pinA, uint8_t pinB, uint8_t pinButton) {
    _leverType = type;
    _pinA = pinA;
    _pinB = pinB;
    _pinButton = pinButton;
    
    // Default analog lever configuration
    _minPosition = 0;
    _maxPosition = 4095;
    _centerPosition = 2048;
    _deadZone = 50;
    _invertDirection = false;
    
    // Default encoder configuration
    _encoderPosition = 0;
    _lastEncoderA = 0;
    _stepsPerDetent = 4;
    _minSteps = -100;
    _maxSteps = 100;
    
    // Default digital lever configuration
    _digitalPositions = 5;
    _currentDigitalPos = 0;
    
    // Movement detection
    _lastPosition = 0;
    _lastUpdateTime = 0;
    _velocity = 0;
    _isMoving = false;
    
    // Smoothing
    _useSmoothing = false;
    _smoothingFactor = 0.1;
    _smoothedPosition = 0;
    
    // Button configuration
    _lastButtonState = false;
    _lastDebounceTime = 0;
    _debounceDelay = 50;
}

// Initialize the lever
void Lever::begin() {
    switch (_leverType) {
        case ANALOG_LEVER:
            // Analog input, no special setup needed
            _smoothedPosition = analogRead(_pinA);
            break;
            
        case ROTARY_ENCODER:
            pinMode(_pinA, INPUT_PULLUP);
            pinMode(_pinB, INPUT_PULLUP);
            _lastEncoderA = digitalRead(_pinA);
            break;
            
        case DIGITAL_LEVER:
            pinMode(_pinA, INPUT_PULLUP);
            if (_pinB != 255) {
                pinMode(_pinB, INPUT_PULLUP);
            }
            break;
    }
    
    if (_pinButton != 255) {
        pinMode(_pinButton, INPUT_PULLUP);
    }
    
    _lastUpdateTime = millis();
}

// Attach interrupt for encoder (call this after begin())
void Lever::attachInterrupt() {
    if (_leverType == ROTARY_ENCODER) {
        // Note: You'll need to create a wrapper function for each encoder instance
        // Example: attachInterrupt(digitalPinToInterrupt(_pinA), encoderISR, CHANGE);
        Serial.println("Warning: attachInterrupt() must be implemented with external ISR function");
    }
}

// Configuration methods for analog levers
void Lever::setAnalogLimits(int minPos, int maxPos, int centerPos) {
    _minPosition = minPos;
    _maxPosition = maxPos;
    if (centerPos == -1) {
        _centerPosition = (minPos + maxPos) / 2;
    } else {
        _centerPosition = centerPos;
    }
}

void Lever::setDeadZone(int deadZone) {
    _deadZone = deadZone;
}

void Lever::invertDirection(bool invert) {
    _invertDirection = invert;
}

// Configuration methods for encoders
void Lever::setEncoderLimits(int minSteps, int maxSteps) {
    _minSteps = minSteps;
    _maxSteps = maxSteps;
}

void Lever::setStepsPerDetent(int steps) {
    _stepsPerDetent = steps;
}

void Lever::setEncoderPosition(long position) {
    _encoderPosition = position;
}

// Configuration methods for digital levers
void Lever::setDigitalPositions(int positions) {
    _digitalPositions = positions;
}

// General configuration
void Lever::setSmoothing(bool enable, float factor) {
    _useSmoothing = enable;
    _smoothingFactor = constrain(factor, 0.0, 1.0);
}

void Lever::setDebounceDelay(unsigned long delay) {
    _debounceDelay = delay;
}

// Internal helper methods
int Lever::_readAnalogPosition() {
    return analogRead(_pinA);
}

void Lever::_updateEncoder() {
    int currentA = digitalRead(_pinA);
    int currentB = digitalRead(_pinB);
    
    if (currentA != _lastEncoderA) {
        if (currentB != currentA) {
            _encoderPosition++;
        } else {
            _encoderPosition--;
        }
        
        // Apply limits
        _encoderPosition = constrain(_encoderPosition, _minSteps, _maxSteps);
        
        _lastEncoderA = currentA;
    }
}

void Lever::_updateDigitalPosition() {
    bool pinAState = !digitalRead(_pinA);
    bool pinBState = (_pinB != 255) ? !digitalRead(_pinB) : false;
    
    static bool lastPinAState = false;
    static bool lastPinBState = false;
    
    if (pinAState && !lastPinAState) {
        _currentDigitalPos = min(_currentDigitalPos + 1, _digitalPositions - 1);
    }
    
    if (pinBState && !lastPinBState) {
        _currentDigitalPos = max(_currentDigitalPos - 1, 0);
    }
    
    lastPinAState = pinAState;
    lastPinBState = pinBState;
}

float Lever::_calculateVelocity() {
    unsigned long currentTime = millis();
    float deltaTime = (currentTime - _lastUpdateTime) / 1000.0; // Convert to seconds
    
    if (deltaTime <= 0) return _velocity;
    
    float currentPosition = readRaw();
    float deltaPosition = currentPosition - _lastPosition;
    
    _velocity = deltaPosition / deltaTime;
    _lastPosition = currentPosition;
    _lastUpdateTime = currentTime;
    
    _isMoving = (abs(_velocity) > 1.0); // Threshold for movement detection
    
    return _velocity;
}

bool Lever::_isInDeadZone(int position) {
    return (abs(position - _centerPosition) <= _deadZone);
}

// Reading methods - Raw values
int Lever::readRaw() {
    switch (_leverType) {
        case ANALOG_LEVER:
            return _readAnalogPosition();
            
        case ROTARY_ENCODER:
            return _encoderPosition;
            
        case DIGITAL_LEVER:
            return _currentDigitalPos;
            
        default:
            return 0;
    }
}

long Lever::readEncoderSteps() {
    if (_leverType == ROTARY_ENCODER) {
        return _encoderPosition;
    }
    return 0;
}

// Reading methods - Processed values
int Lever::readPosition() {
    int rawValue = readRaw();
    
    switch (_leverType) {
        case ANALOG_LEVER: {
            if (_isInDeadZone(rawValue)) {
                rawValue = _centerPosition;
            }
            
            // Apply smoothing
            if (_useSmoothing) {
                _smoothedPosition = _smoothedPosition * (1.0 - _smoothingFactor) + rawValue * _smoothingFactor;
                rawValue = (int)_smoothedPosition;
            }
            
            // Map to -100 to 100 range
            int processedValue;
            if (rawValue >= _centerPosition) {
                processedValue = map(rawValue, _centerPosition, _maxPosition, 0, 100);
            } else {
                processedValue = map(rawValue, _minPosition, _centerPosition, -100, 0);
            }
            
            // Apply inversion if needed
            if (_invertDirection) {
                processedValue = -processedValue;
            }
            
            return constrain(processedValue, -100, 100);
        }
        
        case ROTARY_ENCODER: {
            int steps = _encoderPosition / _stepsPerDetent;
            return map(steps, _minSteps / _stepsPerDetent, _maxSteps / _stepsPerDetent, -100, 100);
        }
        
        case DIGITAL_LEVER: {
            return map(_currentDigitalPos, 0, _digitalPositions - 1, 0, 100);
        }
        
        default:
            return 0;
    }
}

float Lever::readPositionFloat() {
    return readPosition() / 100.0;
}

// Reading methods - Mapped values
int Lever::readMapped(int minVal, int maxVal) {
    int position = readPosition();
    
    if (_leverType == DIGITAL_LEVER || (_leverType == ANALOG_LEVER && _minPosition >= 0)) {
        // Map from 0-100 to minVal-maxVal
        return map(position, 0, 100, minVal, maxVal);
    } else {
        // Map from -100 to 100 to minVal-maxVal
        return map(position, -100, 100, minVal, maxVal);
    }
}

float Lever::readMappedFloat(float minVal, float maxVal) {
    float position = readPositionFloat();
    
    if (_leverType == DIGITAL_LEVER || (_leverType == ANALOG_LEVER && _minPosition >= 0)) {
        // Map from 0.0-1.0 to minVal-maxVal
        return minVal + position * (maxVal - minVal);
    } else {
        // Map from -1.0 to 1.0 to minVal-maxVal
        return minVal + (position + 1.0) * 0.5 * (maxVal - minVal);
    }
}

// Movement detection
float Lever::readVelocity() {
    return _calculateVelocity();
}

bool Lever::isMoving() {
    _calculateVelocity(); // Update movement status
    return _isMoving;
}

bool Lever::isMovingLeft() {
    return isMoving() && (_velocity < 0);
}

bool Lever::isMovingRight() {
    return isMoving() && (_velocity > 0);
}

// Position queries
bool Lever::isAtCenter() {
    if (_leverType == ANALOG_LEVER) {
        return _isInDeadZone(readRaw());
    } else if (_leverType == ROTARY_ENCODER) {
        return (abs(_encoderPosition) <= _stepsPerDetent);
    } else {
        return (_currentDigitalPos == _digitalPositions / 2);
    }
}

bool Lever::isAtMinimum() {
    int position = readPosition();
    return (position <= -95); // Near minimum
}

bool Lever::isAtMaximum() {
    int position = readPosition();
    return (position >= 95); // Near maximum
}

bool Lever::isInDeadZone() {
    if (_leverType == ANALOG_LEVER) {
        return _isInDeadZone(readRaw());
    }
    return false;
}

// Button methods
bool Lever::isPressed() {
    if (_pinButton == 255) return false;
    return !digitalRead(_pinButton);
}

bool Lever::wasPressed() {
    if (_pinButton == 255) return false;
    
    bool currentState = !digitalRead(_pinButton);
    bool wasPressed = false;
    
    if (currentState != _lastButtonState) {
        _lastDebounceTime = millis();
    }
    
    if ((millis() - _lastDebounceTime) > _debounceDelay) {
        if (currentState && !_lastButtonState) {
            wasPressed = true;
        }
        _lastButtonState = currentState;
    }
    
    return wasPressed;
}

bool Lever::wasReleased() {
    if (_pinButton == 255) return false;
    
    bool currentState = !digitalRead(_pinButton);
    bool wasReleased = false;
    
    if (currentState != _lastButtonState) {
        _lastDebounceTime = millis();
    }
    
    if ((millis() - _lastDebounceTime) > _debounceDelay) {
        if (!currentState && _lastButtonState) {
            wasReleased = true;
        }
        _lastButtonState = currentState;
    }
    
    return wasReleased;
}

// Encoder specific methods
void Lever::resetEncoder() {
    if (_leverType == ROTARY_ENCODER) {
        _encoderPosition = 0;
    }
}

long Lever::getEncoderPosition() {
    return _encoderPosition;
}

int Lever::getEncoderDirection() {
    if (_leverType != ROTARY_ENCODER) return 0;
    
    static long lastPosition = 0;
    long currentPosition = _encoderPosition;
    
    int direction = 0;
    if (currentPosition > lastPosition) {
        direction = 1;
    } else if (currentPosition < lastPosition) {
        direction = -1;
    }
    
    lastPosition = currentPosition;
    return direction;
}

// Digital lever specific methods
int Lever::getDigitalPosition() {
    return _currentDigitalPos;
}

bool Lever::moveToNextPosition() {
    if (_leverType == DIGITAL_LEVER && _currentDigitalPos < _digitalPositions - 1) {
        _currentDigitalPos++;
        return true;
    }
    return false;
}

bool Lever::moveToPreviousPosition() {
    if (_leverType == DIGITAL_LEVER && _currentDigitalPos > 0) {
        _currentDigitalPos--;
        return true;
    }
    return false;
}

// Utility methods
void Lever::update() {
    switch (_leverType) {
        case ROTARY_ENCODER:
            _updateEncoder();
            break;
            
        case DIGITAL_LEVER:
            _updateDigitalPosition();
            break;
            
        default:
            break;
    }
    
    _calculateVelocity();
}

void Lever::reset() {
    switch (_leverType) {
        case ANALOG_LEVER:
            _smoothedPosition = _centerPosition;
            break;
            
        case ROTARY_ENCODER:
            _encoderPosition = 0;
            break;
            
        case DIGITAL_LEVER:
            _currentDigitalPos = 0;
            break;
    }
    
    _lastPosition = 0;
    _velocity = 0;
    _isMoving = false;
}

// Calibration methods
void Lever::calibrateAnalog() {
    if (_leverType != ANALOG_LEVER) return;
    
    Serial.println("Lever Calibration Started");
    Serial.println("Move lever through full range for 5 seconds...");
    
    _minPosition = 4095;
    _maxPosition = 0;
    
    unsigned long startTime = millis();
    while (millis() - startTime < 5000) {
        int value = analogRead(_pinA);
        
        if (value < _minPosition) _minPosition = value;
        if (value > _maxPosition) _maxPosition = value;
        
        delay(10);
    }
    
    Serial.println("Center the lever and hold for 2 seconds...");
    delay(2000);
    
    _centerPosition = analogRead(_pinA);
    
    Serial.println("Calibration complete!");
    Serial.print("Min="); Serial.print(_minPosition);
    Serial.print(", Max="); Serial.print(_maxPosition);
    Serial.print(", Center="); Serial.println(_centerPosition);
}

void Lever::calibrateCenter() {
    if (_leverType == ANALOG_LEVER) {
        _centerPosition = analogRead(_pinA);
        Serial.print("Center position set to: ");
        Serial.println(_centerPosition);
    } else if (_leverType == ROTARY_ENCODER) {
        _encoderPosition = 0;
        Serial.println("Encoder position reset to 0");
    }
}