/**
 * Joystick Library Example
 * 
 * This example demonstrates how to use the Joystick library
 * with different configurations and features.
 */

#include <Joystick.h>

// Create joystick instance
// Parameters: pinX, pinY, pinButton (optional)
Joystick joystick(A0, A1, 2);

void setup() {
    Serial.begin(9600);
    Serial.println("Joystick Library Example");
    
    // Initialize the joystick
    joystick.begin();
    
    // Configure dead zone (radius of 80 units, circular)
    joystick.setDeadZone(80, true);
    
    // Enable smoothing for smoother readings
    joystick.setSmoothing(true, 0.2); // 20% smoothing factor
    
    // Invert Y axis if needed (common for some joysticks)
    joystick.invertAxis(false, true);
    
    // Optional: Manual calibration
    // Uncomment the next line to calibrate the joystick
    // joystick.calibrate();
    
    // Or set manual limits if you know them
    joystick.setLimits(0, 4095, 0, 4095);
    joystick.setCenter(2048, 2048);
    
    Serial.println("Joystick initialized. Move joystick to see readings...");
    Serial.println();
}

void loop() {
    // Basic position reading (-100 to 100)
    int x = joystick.readX();
    int y = joystick.readY();
    
    // Float values (-1.0 to 1.0)
    float xFloat = joystick.readXFloat();
    float yFloat = joystick.readYFloat();
    
    // Mapped values (custom range)
    int xMapped = joystick.readXMapped(-255, 255);
    int yMapped = joystick.readYMapped(-255, 255);
    
    // Magnitude and angle
    float magnitude = joystick.readMagnitude();
    float angle = joystick.readAngleDegrees();
    
    // Button state
    bool buttonPressed = joystick.isPressed();
    bool buttonWasPressed = joystick.wasPressed();
    
    // Display readings
    Serial.print("Position: X="); Serial.print(x);
    Serial.print(", Y="); Serial.print(y);
    Serial.print(" | Float: X="); Serial.print(xFloat, 2);
    Serial.print(", Y="); Serial.print(yFloat, 2);
    Serial.print(" | Mapped: X="); Serial.print(xMapped);
    Serial.print(", Y="); Serial.print(yMapped);
    Serial.print(" | Magnitude="); Serial.print(magnitude, 2);
    Serial.print(", Angle="); Serial.print(angle, 1); Serial.print("°");
    
    if (buttonPressed) {
        Serial.print(" | BUTTON PRESSED");
    }
    if (buttonWasPressed) {
        Serial.print(" | BUTTON CLICKED");
    }
    
    // Status indicators
    if (joystick.isNeutral()) {
        Serial.print(" | NEUTRAL");
    }
    if (joystick.isAtEdge()) {
        Serial.print(" | AT EDGE");
    }
    
    Serial.println();
    
    delay(100); // Update every 100ms
}

/*
 * Advanced usage example with different configurations:
 * 
 * // For flight simulator stick (different dead zones per axis)
 * joystick.setDeadZone(50, false); // Rectangular dead zone
 * 
 * // For racing game (only positive Y values)
 * int throttle = joystick.readYMapped(0, 255);
 * 
 * // For menu navigation (detect discrete movements)
 * static int lastX = 0, lastY = 0;
 * int currentX = joystick.readX();
 * int currentY = joystick.readY();
 * 
 * if (currentX > 80 && lastX <= 80) {
 *     Serial.println("Move RIGHT");
 * } else if (currentX < -80 && lastX >= -80) {
 *     Serial.println("Move LEFT");
 * }
 * 
 * lastX = currentX;
 * lastY = currentY;
 */