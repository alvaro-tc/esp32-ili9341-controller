/**
 * Lever Library Example
 * 
 * This example demonstrates how to use the Lever library
 * with different lever types and configurations.
 */

#include <Lever.h>

// Create different lever instances
Lever analogLever(ANALOG_LEVER, A2);              // Potentiometer lever
Lever rotaryEncoder(ROTARY_ENCODER, 3, 4, 5);     // Rotary encoder with button
Lever digitalLever(DIGITAL_LEVER, 6, 7);          // Switch-based lever

// For encoder interrupt handling
void encoderISR() {
    // This would need to call rotaryEncoder.update() or handle encoder logic
    // Note: You might need to make the encoder instance global or use a different approach
}

void setup() {
    Serial.begin(9600);
    Serial.println("Lever Library Examples");
    
    // Initialize all levers
    analogLever.begin();
    rotaryEncoder.begin();
    digitalLever.begin();
    
    // Configure analog lever
    analogLever.setAnalogLimits(0, 4095, 2048);  // Min, Max, Center
    analogLever.setDeadZone(100);                 // Dead zone around center
    analogLever.setSmoothing(true, 0.15);         // Enable smoothing
    analogLever.invertDirection(false);           // Don't invert
    
    // Configure rotary encoder
    rotaryEncoder.setEncoderLimits(-50, 50);      // -50 to +50 steps
    rotaryEncoder.setStepsPerDetent(4);           // 4 steps per click
    rotaryEncoder.setEncoderPosition(0);          // Start at center
    
    // Configure digital lever
    digitalLever.setDigitalPositions(5);          // 5 discrete positions (0-4)
    
    Serial.println("Levers initialized. Test each lever type...");
    Serial.println();
}

void loop() {
    // Update all levers (important for encoders and digital levers)
    rotaryEncoder.update();
    digitalLever.update();
    
    Serial.println("=== Lever Readings ===");
    
    // Analog Lever Example
    Serial.println("Analog Lever:");
    int analogPos = analogLever.readPosition();
    float analogFloat = analogLever.readPositionFloat();
    int analogMapped = analogLever.readMapped(0, 255);
    float analogVelocity = analogLever.readVelocity();
    
    Serial.print("  Position: "); Serial.print(analogPos);
    Serial.print(" | Float: "); Serial.print(analogFloat, 2);
    Serial.print(" | Mapped(0-255): "); Serial.print(analogMapped);
    Serial.print(" | Velocity: "); Serial.print(analogVelocity, 1);
    
    if (analogLever.isAtCenter()) Serial.print(" | CENTER");
    if (analogLever.isInDeadZone()) Serial.print(" | DEAD_ZONE");
    if (analogLever.isMovingLeft()) Serial.print(" | MOVING_LEFT");
    if (analogLever.isMovingRight()) Serial.print(" | MOVING_RIGHT");
    Serial.println();
    
    // Rotary Encoder Example
    Serial.println("Rotary Encoder:");
    int encoderPos = rotaryEncoder.readPosition();
    long encoderSteps = rotaryEncoder.readEncoderSteps();
    int encoderDirection = rotaryEncoder.getEncoderDirection();
    bool encoderButton = rotaryEncoder.wasPressed();
    
    Serial.print("  Position: "); Serial.print(encoderPos);
    Serial.print(" | Steps: "); Serial.print(encoderSteps);
    Serial.print(" | Direction: ");
    if (encoderDirection == 1) Serial.print("CW");
    else if (encoderDirection == -1) Serial.print("CCW");
    else Serial.print("STOP");
    
    if (encoderButton) Serial.print(" | BUTTON_CLICKED");
    if (rotaryEncoder.isPressed()) Serial.print(" | BUTTON_HELD");
    Serial.println();
    
    // Digital Lever Example  
    Serial.println("Digital Lever:");
    int digitalPos = digitalLever.getDigitalPosition();
    int digitalMapped = digitalLever.readMapped(0, 100);
    
    Serial.print("  Position: "); Serial.print(digitalPos);
    Serial.print(" | Mapped(0-100): "); Serial.print(digitalMapped);
    
    if (digitalLever.isAtMinimum()) Serial.print(" | AT_MIN");
    if (digitalLever.isAtMaximum()) Serial.print(" | AT_MAX");
    Serial.println();
    
    Serial.println();
    delay(200); // Update every 200ms
}

/*
 * Advanced usage examples:
 * 
 * 1. Volume Control with Encoder:
 *    int volume = rotaryEncoder.readMapped(0, 100);
 *    if (rotaryEncoder.wasPressed()) {
 *        muteToggle = !muteToggle;
 *    }
 * 
 * 2. Throttle Control with Analog Lever:
 *    int throttle = analogLever.readMapped(0, 255);
 *    // Use throttle value for motor control
 * 
 * 3. Menu Navigation with Digital Lever:
 *    static int lastPos = 0;
 *    int currentPos = digitalLever.getDigitalPosition();
 *    if (currentPos != lastPos) {
 *        menuIndex = currentPos;
 *        updateDisplay();
 *        lastPos = currentPos;
 *    }
 * 
 * 4. Flight Simulator Trim with Encoder:
 *    static float trim = 0.0;
 *    int encoderDelta = rotaryEncoder.getEncoderDirection();
 *    if (encoderDelta != 0) {
 *        trim += encoderDelta * 0.1; // 0.1 degree per step
 *        trim = constrain(trim, -30.0, 30.0);
 *    }
 * 
 * 5. Calibration Example:
 *    if (digitalRead(CALIBRATE_BUTTON) == LOW) {
 *        analogLever.calibrateAnalog(); // Auto calibration
 *        // or
 *        analogLever.calibrateCenter(); // Just center position
 *    }
 */