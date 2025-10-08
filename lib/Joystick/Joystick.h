/**
 * Joystick Library - Flexible joystick control library
 * 
 * Features:
 * - Dead zone configuration
 * - Min/Max limits configuration
 * - Axis inversion
 * - Value smoothing/filtering
 * - Calibration support
 * - Multiple output formats (raw, percentage, mapped)
 * 
 * Author: GitHub Copilot
 * Date: 2025
 */

#ifndef JOYSTICK_H
#define JOYSTICK_H

#include <Arduino.h>

class Joystick {
private:
    // Pin assignments
    uint8_t _pinX;
    uint8_t _pinY;
    uint8_t _pinButton;
    
    // Calibration values
    int _minX, _maxX, _centerX;
    int _minY, _maxY, _centerY;
    
    // Dead zone configuration
    int _deadZoneRadius;
    bool _useCircularDeadZone;
    
    // Axis inversion flags
    bool _invertX;
    bool _invertY;
    
    // Smoothing filter
    bool _useSmoothing;
    float _smoothingFactor;
    float _lastX, _lastY;
    
    // Button state
    bool _lastButtonState;
    unsigned long _lastDebounceTime;
    unsigned long _debounceDelay;
    
    // Internal helper methods
    int _readRawX();
    int _readRawY();
    float _applySmoothing(float newValue, float lastValue);
    bool _isInDeadZone(int x, int y);
    int _applyDeadZone(int value, int center, int deadZone);
    
public:
    // Constructor
    Joystick(uint8_t pinX, uint8_t pinY, uint8_t pinButton = 255);
    
    // Initialization
    void begin();
    void calibrate();
    void autoCalibrate(unsigned long duration = 5000);
    
    // Configuration methods
    void setDeadZone(int radius, bool circular = true);
    void setLimits(int minX, int maxX, int minY, int maxY);
    void setCenter(int centerX, int centerY);
    void invertAxis(bool invertX, bool invertY);
    void setSmoothing(bool enable, float factor = 0.1);
    void setDebounceDelay(unsigned long delay);
    
    // Reading methods - Raw values
    int readRawX();
    int readRawY();
    
    // Reading methods - Processed values (-255 to 255)
    int readX();
    int readY();
    
    // Reading methods - Float values (-1.0 to 1.0)
    float readXFloat();
    float readYFloat();
    
    // Reading methods - Mapped values
    int readXMapped(int minVal, int maxVal);
    int readYMapped(int minVal, int maxVal);
    
    // Reading methods - Magnitude and angle
    float readMagnitude();
    float readAngle(); // Returns angle in radians
    float readAngleDegrees(); // Returns angle in degrees
    
    // Button methods
    bool isPressed();
    bool wasPressed();
    bool wasReleased();
    
    // Utility methods
    bool isNeutral();
    bool isAtEdge();
    void resetPosition();
    
    // Calibration getters
    int getMinX() { return _minX; }
    int getMaxX() { return _maxX; }
    int getMinY() { return _minY; }
    int getMaxY() { return _maxY; }
    int getCenterX() { return _centerX; }
    int getCenterY() { return _centerY; }
    int getDeadZone() { return _deadZoneRadius; }
};

#endif // JOYSTICK_H