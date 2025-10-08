/**
 * Joystick Library Implementation
 * 
 * Author: GitHub Copilot
 * Date: 2025
 */

#include "Joystick.h"

// Constructor
Joystick::Joystick(uint8_t pinX, uint8_t pinY, uint8_t pinButton) {
    _pinX = pinX;
    _pinY = pinY;
    _pinButton = pinButton;
    
    // Default calibration values (will be updated during calibration)
    _minX = 0; _maxX = 4095; _centerX = 2048;
    _minY = 0; _maxY = 4095; _centerY = 2048;
    
    // Default configuration
    _deadZoneRadius = 50; // Adjust based on your ADC resolution
    _useCircularDeadZone = true;
    _invertX = false;
    _invertY = false;
    _useSmoothing = false;
    _smoothingFactor = 0.1;
    _lastX = 0;
    _lastY = 0;
    
    // Button configuration
    _lastButtonState = false;
    _lastDebounceTime = 0;
    _debounceDelay = 50;
}

// Initialize the joystick
void Joystick::begin() {
    if (_pinButton != 255) {
        pinMode(_pinButton, INPUT_PULLUP);
    }
    
    // Take initial readings for center position
    _centerX = analogRead(_pinX);
    _centerY = analogRead(_pinY);
    
    // Initialize smoothing values
    _lastX = _centerX;
    _lastY = _centerY;
}

// Manual calibration - call this and move joystick through full range
void Joystick::calibrate() {
    Serial.println("Joystick Calibration Started");
    Serial.println("Move joystick through full range for 5 seconds...");
    
    _minX = 4095; _maxX = 0;
    _minY = 4095; _maxY = 0;
    
    unsigned long startTime = millis();
    while (millis() - startTime < 5000) {
        int x = analogRead(_pinX);
        int y = analogRead(_pinY);
        
        if (x < _minX) _minX = x;
        if (x > _maxX) _maxX = x;
        if (y < _minY) _minY = y;
        if (y > _maxY) _maxY = y;
        
        delay(10);
    }
    
    Serial.println("Center the joystick and hold for 2 seconds...");
    delay(2000);
    
    // Read center position
    _centerX = analogRead(_pinX);
    _centerY = analogRead(_pinY);
    
    Serial.println("Calibration complete!");
    Serial.print("X: Min="); Serial.print(_minX); 
    Serial.print(", Max="); Serial.print(_maxX);
    Serial.print(", Center="); Serial.println(_centerX);
    Serial.print("Y: Min="); Serial.print(_minY); 
    Serial.print(", Max="); Serial.print(_maxY);
    Serial.print(", Center="); Serial.println(_centerY);
}

// Auto calibration
void Joystick::autoCalibrate(unsigned long duration) {
    _minX = 4095; _maxX = 0;
    _minY = 4095; _maxY = 0;
    
    unsigned long startTime = millis();
    while (millis() - startTime < duration) {
        int x = analogRead(_pinX);
        int y = analogRead(_pinY);
        
        if (x < _minX) _minX = x;
        if (x > _maxX) _maxX = x;
        if (y < _minY) _minY = y;
        if (y > _maxY) _maxY = y;
        
        delay(10);
    }
    
    _centerX = (_minX + _maxX) / 2;
    _centerY = (_minY + _maxY) / 2;
}

// Configuration methods
void Joystick::setDeadZone(int radius, bool circular) {
    _deadZoneRadius = radius;
    _useCircularDeadZone = circular;
}

void Joystick::setLimits(int minX, int maxX, int minY, int maxY) {
    _minX = minX;
    _maxX = maxX;
    _minY = minY;
    _maxY = maxY;
}

void Joystick::setCenter(int centerX, int centerY) {
    _centerX = centerX;
    _centerY = centerY;
}

void Joystick::invertAxis(bool invertX, bool invertY) {
    _invertX = invertX;
    _invertY = invertY;
}

void Joystick::setSmoothing(bool enable, float factor) {
    _useSmoothing = enable;
    _smoothingFactor = constrain(factor, 0.0, 1.0);
}

void Joystick::setDebounceDelay(unsigned long delay) {
    _debounceDelay = delay;
}

// Internal helper methods
int Joystick::_readRawX() {
    const int samples = 10;  // Número de lecturas
    int buffer[samples];
    int sum = 0;

    // Leer muestras
    for (int i = 0; i < samples; i++) {
        buffer[i] = analogRead(_pinX);
        sum += buffer[i];
    }

    // Promedio inicial
    float avg = (float)sum / samples;

    // Segunda pasada: quitar outliers
    const int tolerance = 150;  // Máxima desviación permitida (ajústalo según tu ruido)
    int validSum = 0;
    int validCount = 0;

    for (int i = 0; i < samples; i++) {
        if (abs(buffer[i] - avg) < tolerance) {
            validSum += buffer[i];
            validCount++;
        }
    }

    // Si todas fueron descartadas, devuelve el promedio bruto
    if (validCount == 0) return (int)avg;

    return validSum / validCount;
}

int Joystick::_readRawY() {
    const int samples = 10;
    int buffer[samples];
    int sum = 0;

    for (int i = 0; i < samples; i++) {
        buffer[i] = analogRead(_pinY);
        sum += buffer[i];
    }

    float avg = (float)sum / samples;

    const int tolerance = 150;  
    int validSum = 0;
    int validCount = 0;

    for (int i = 0; i < samples; i++) {
        if (abs(buffer[i] - avg) < tolerance) {
            validSum += buffer[i];
            validCount++;
        }
    }

    if (validCount == 0) return (int)avg;

    return validSum / validCount;
}


float Joystick::_applySmoothing(float newValue, float lastValue) {
    return _useSmoothing ? (lastValue * (1.0 - _smoothingFactor) + newValue * _smoothingFactor) : newValue;
}

bool Joystick::_isInDeadZone(int x, int y) {
    if (_useCircularDeadZone) {
        int dx = x - _centerX;
        int dy = y - _centerY;
        return (sqrt(dx*dx + dy*dy) <= _deadZoneRadius);
    } else {
        return (abs(x - _centerX) <= _deadZoneRadius && abs(y - _centerY) <= _deadZoneRadius);
    }
}

int Joystick::_applyDeadZone(int value, int center, int deadZone) {
    int diff = value - center;
    if (abs(diff) <= deadZone) {
        return center;
    }
    return value;
}

// Reading methods - Raw values
int Joystick::readRawX() {
    return _readRawX();
}

int Joystick::readRawY() {
    return _readRawY();
}

// Reading methods - Processed values (-255 to 255)
int Joystick::readX() {
    int rawX = _readRawX();
    int rawY = _readRawY(); // necesario para deadzone circular

    // Si está dentro de la zona muerta -> devolver 0
    if (_isInDeadZone(rawX, rawY)) {
        return 0;
    }
    
    // Apply smoothing
    float smoothedX = _applySmoothing(rawX, _lastX);
    _lastX = smoothedX;
    
    // Map to -255 to 255 range
    int processedX;
    if (smoothedX >= _centerX) {
        processedX = map(smoothedX, _centerX, _maxX, 0, 255);
    } else {
        processedX = map(smoothedX, _minX, _centerX, -255, 0);
    }
    
    // Apply inversion if needed
    if (_invertX) {
        processedX = -processedX;
    }
    
    return constrain(processedX, -255, 255);
}

int Joystick::readY() {
    int rawX = _readRawX();
    int rawY = _readRawY(); // necesario para deadzone circular

    // Si está dentro de la zona muerta -> devolver 0
    if (_isInDeadZone(rawX, rawY)) {
        return 0;
    }
    
    // Apply smoothing
    float smoothedY = _applySmoothing(rawY, _lastY);
    _lastY = smoothedY;
    
    // Map to -255 to 255 range
    int processedY;
    if (smoothedY >= _centerY) {
        processedY = map(smoothedY, _centerY, _maxY, 0, 255);
    } else {
        processedY = map(smoothedY, _minY, _centerY, -255, 0);
    }
    
    // Apply inversion if needed
    if (_invertY) {
        processedY = -processedY;
    }
    
    return constrain(processedY, -255, 255);
}

// Reading methods - Float values (-1.0 to 1.0)
float Joystick::readXFloat() {
    return readX() / 255.0;
}

float Joystick::readYFloat() {
    return readY() / 255.0;
}

// Reading methods - Mapped values
int Joystick::readXMapped(int minVal, int maxVal) {
    int x = readX();
    return map(x, -255, 255, minVal, maxVal);
}

int Joystick::readYMapped(int minVal, int maxVal) {
    int y = readY();
    return map(y, -255, 255, minVal, maxVal);
}

// Reading methods - Magnitude and angle
float Joystick::readMagnitude() {
    float x = readXFloat();
    float y = readYFloat();
    return sqrt(x*x + y*y);
}

float Joystick::readAngle() {
    float x = readXFloat();
    float y = readYFloat();
    return atan2(y, x);
}

float Joystick::readAngleDegrees() {
    return readAngle() * 180.0 / PI;
}

// Button methods
bool Joystick::isPressed() {
    if (_pinButton == 255) return false;
    return !digitalRead(_pinButton); // Assuming pullup configuration
}

bool Joystick::wasPressed() {
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

bool Joystick::wasReleased() {
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

// Utility methods
bool Joystick::isNeutral() {
    return (readX() == 0 && readY() == 0);
}

bool Joystick::isAtEdge() {
    int x = readX();
    int y = readY();
    return (abs(x) >= 95 || abs(y) >= 95); // Near max range
}

void Joystick::resetPosition() {
    _lastX = _centerX;
    _lastY = _centerY;
}