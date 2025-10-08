/**
 * Combined Joystick and Lever Example
 * 
 * This example shows how to use both Joystick and Lever libraries
 * together in a single project, useful for complex control systems.
 */

#include <Joystick.h>
#include <Lever.h>

// Create instances
Joystick mainJoystick(A0, A1, 8);        // Main control joystick
Lever throttleLever(ANALOG_LEVER, A2);   // Throttle control
Lever trimEncoder(ROTARY_ENCODER, 2, 3, 4); // Trim adjustment encoder

// System state variables
struct ControlState {
    int joystickX, joystickY;
    int throttle;
    float trim;
    bool armed;
};

ControlState currentState = {0, 0, 0, 0.0, false};

void setup() {
    Serial.begin(9600);
    Serial.println("Combined Control System Example");
    
    // Initialize all controls
    mainJoystick.begin();
    throttleLever.begin();
    trimEncoder.begin();
    
    // Configure joystick
    mainJoystick.setDeadZone(60, true);
    mainJoystick.setSmoothing(true, 0.2);
    mainJoystick.invertAxis(false, true); // Invert Y for aircraft-style control
    
    // Configure throttle (0-100% range)
    throttleLever.setAnalogLimits(0, 4095, 0); // No center, starts at 0
    throttleLever.setSmoothing(true, 0.1);
    
    // Configure trim encoder
    trimEncoder.setEncoderLimits(-100, 100);
    trimEncoder.setStepsPerDetent(2);
    
    Serial.println("Control system ready. ARM with joystick button...");
}

void loop() {
    // Update encoders
    trimEncoder.update();
    
    // Read all controls
    updateControls();
    
    // Handle arming/disarming
    handleArming();
    
    // Process control logic
    processControls();
    
    // Send output or display status
    displayStatus();
    
    delay(50); // 20Hz update rate
}

void updateControls() {
    // Read joystick
    currentState.joystickX = mainJoystick.readX();
    currentState.joystickY = mainJoystick.readY();
    
    // Read throttle (0-100%)
    currentState.throttle = throttleLever.readMapped(0, 100);
    
    // Update trim with encoder
    int trimDirection = trimEncoder.getEncoderDirection();
    if (trimDirection != 0) {
        currentState.trim += trimDirection * 0.5; // 0.5% per step
        currentState.trim = constrain(currentState.trim, -15.0, 15.0);
    }
}

void handleArming() {
    static bool lastArmed = false;
    
    if (mainJoystick.wasPressed()) {
        if (!currentState.armed) {
            // Arm only if throttle is low and joystick is centered
            if (currentState.throttle < 10 && mainJoystick.isNeutral()) {
                currentState.armed = true;
                Serial.println("SYSTEM ARMED");
            } else {
                Serial.println("Cannot arm: Throttle must be low and joystick centered");
            }
        } else {
            // Disarm
            currentState.armed = false;
            Serial.println("SYSTEM DISARMED");
        }
    }
    
    // Auto-disarm if throttle goes to zero
    if (currentState.armed && currentState.throttle == 0 && lastArmed) {
        currentState.armed = false;
        Serial.println("AUTO-DISARM: Throttle at zero");
    }
    
    lastArmed = currentState.armed;
}

void processControls() {
    if (!currentState.armed) {
        return; // Don't process controls when disarmed
    }
    
    // Apply trim to joystick Y axis
    float adjustedY = currentState.joystickY + currentState.trim;
    adjustedY = constrain(adjustedY, -100, 100);
    
    // Here you would send the control values to your actuators
    // Examples:
    // - Servo control for aircraft
    // - Motor control for drone
    // - Steering control for vehicle
    
    // Simulate control output
    int servoX = map(currentState.joystickX, -100, 100, 1000, 2000);
    int servoY = map(adjustedY, -100, 100, 1000, 2000);
    int motorSpeed = map(currentState.throttle, 0, 100, 1000, 2000);
    
    // You would use these values with Servo library or similar
    // servo1.writeMicroseconds(servoX);
    // servo2.writeMicroseconds(servoY);
    // motor.writeMicroseconds(motorSpeed);
}

void displayStatus() {
    static unsigned long lastDisplay = 0;
    
    if (millis() - lastDisplay > 500) { // Update display every 500ms
        Serial.print("Status: ");
        Serial.print(currentState.armed ? "ARMED" : "DISARMED");
        Serial.print(" | Joy: X="); Serial.print(currentState.joystickX);
        Serial.print(", Y="); Serial.print(currentState.joystickY);
        Serial.print(" | Throttle: "); Serial.print(currentState.throttle); Serial.print("%");
        Serial.print(" | Trim: "); Serial.print(currentState.trim, 1); Serial.print("%");
        
        // Status indicators
        if (mainJoystick.isAtEdge()) {
            Serial.print(" | JOY_EDGE");
        }
        if (throttleLever.isAtMaximum()) {
            Serial.print(" | FULL_THROTTLE");
        }
        if (trimEncoder.isPressed()) {
            Serial.print(" | TRIM_PRESSED");
        }
        
        Serial.println();
        lastDisplay = millis();
    }
}

/*
 * Additional features you could add:
 * 
 * 1. Exponential curves for smoother control:
 *    float applyExpo(float input, float expo) {
 *        return expo * input * input * input + (1.0 - expo) * input;
 *    }
 * 
 * 2. Dual rates (high/low sensitivity):
 *    bool highRate = digitalRead(RATE_SWITCH);
 *    int rate = highRate ? 100 : 60; // 100% or 60% rates
 *    int adjustedX = (currentState.joystickX * rate) / 100;
 * 
 * 3. Failsafe functionality:
 *    if (millis() - lastControlUpdate > 1000) {
 *        // No control input for 1 second - activate failsafe
 *        currentState.armed = false;
 *        currentState.throttle = 0;
 *    }
 * 
 * 4. EEPROM storage for trim and calibration:
 *    EEPROM.put(0, currentState.trim);
 *    // Read on startup: EEPROM.get(0, currentState.trim);
 */