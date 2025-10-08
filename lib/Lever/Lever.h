/**
 * Lever Library - Flexible lever/encoder control library
 * 
 * Features:
 * - Support for analog levers and rotary encoders
 * - Position limits configuration
 * - Direction detection
 * - Speed/velocity calculation
 * - Step-based movement for encoders
 * - Smooth analog reading for potentiometer levers
 * - Center detection and auto-return functionality
 * 
 * Author: GitHub Copilot
 * Date: 2025
 */

#ifndef LEVER_H
#define LEVER_H

#include <Arduino.h>

// Lever types
enum LeverType {
    ANALOG_LEVER,    // Potentiometer-based lever
    ROTARY_ENCODER,  // Quadrature encoder
    DIGITAL_LEVER    // Switch-based discrete positions
};

class Lever {
private:
    // Configuration
    LeverType _leverType;
    uint8_t _pinA;
    uint8_t _pinB;
    uint8_t _pinButton;
    
    // Analog lever properties
    int _minPosition;
    int _maxPosition;
    int _centerPosition;
    int _deadZone;
    bool _invertDirection;
    
    // Encoder properties
    volatile long _encoderPosition;
    int _lastEncoderA;
    int _stepsPerDetent;
    int _minSteps;
    int _maxSteps;
    
    // Digital lever properties
    int _digitalPositions;
    int _currentDigitalPos;
    
    // Movement detection
    float _lastPosition;
    unsigned long _lastUpdateTime;
    float _velocity;
    bool _isMoving;
    
    // Smoothing
    bool _useSmoothing;
    float _smoothingFactor;
    float _smoothedPosition;
    
    // Button state
    bool _lastButtonState;
    unsigned long _lastDebounceTime;
    unsigned long _debounceDelay;
    
    // Internal helper methods
    int _readAnalogPosition();
    void _updateEncoder();
    void _updateDigitalPosition();
    float _calculateVelocity();
    bool _isInDeadZone(int position);
    
public:
    // Constructors for different lever types
    Lever(LeverType type, uint8_t pinA, uint8_t pinB = 255, uint8_t pinButton = 255);
    
    // Initialization
    void begin();
    void attachInterrupt();
    
    // Configuration methods for analog levers
    void setAnalogLimits(int minPos, int maxPos, int centerPos = -1);
    void setDeadZone(int deadZone);
    void invertDirection(bool invert);
    
    // Configuration methods for encoders
    void setEncoderLimits(int minSteps, int maxSteps);
    void setStepsPerDetent(int steps);
    void setEncoderPosition(long position);
    
    // Configuration methods for digital levers
    void setDigitalPositions(int positions);
    
    // General configuration
    void setSmoothing(bool enable, float factor = 0.1);
    void setDebounceDelay(unsigned long delay);
    
    // Reading methods - Raw values
    int readRaw();
    long readEncoderSteps();
    
    // Reading methods - Processed values (0 to 100 or -100 to 100)
    int readPosition(); // Returns 0-100 for single direction, -100 to 100 for bidirectional
    float readPositionFloat(); // Returns 0.0-1.0 or -1.0 to 1.0
    
    // Reading methods - Mapped values
    int readMapped(int minVal, int maxVal);
    float readMappedFloat(float minVal, float maxVal);
    
    // Movement detection
    float readVelocity(); // Returns velocity in units per second
    bool isMoving();
    bool isMovingLeft();
    bool isMovingRight();
    
    // Position queries
    bool isAtCenter();
    bool isAtMinimum();
    bool isAtMaximum();
    bool isInDeadZone();
    
    // Button methods
    bool isPressed();
    bool wasPressed();
    bool wasReleased();
    
    // Encoder specific methods
    void resetEncoder();
    long getEncoderPosition();
    int getEncoderDirection(); // -1, 0, or 1
    
    // Digital lever specific methods
    int getDigitalPosition();
    bool moveToNextPosition();
    bool moveToPreviousPosition();
    
    // Utility methods
    void update(); // Call this regularly in loop() for proper operation
    void reset();
    
    // Calibration methods
    void calibrateAnalog();
    void calibrateCenter();
    
    // Getters
    LeverType getLeverType() { return _leverType; }
    int getMinPosition() { return _minPosition; }
    int getMaxPosition() { return _maxPosition; }
    int getCenterPosition() { return _centerPosition; }
    int getDeadZone() { return _deadZone; }
};

#endif // LEVER_H