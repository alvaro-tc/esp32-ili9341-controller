/**
 * Advanced Conditional Mapping Example
 * 
 * This example demonstrates the powerful conditional mapping system.
 * Shows how lever positions can change the behavior of other controls.
 */

#include <SPI.h>
#include <RF24.h>
#include <Joystick.h>
#include <Lever.h>
#include <NRF24Controller.h>

NRF24Controller nrf(9, 10);
Joystick mainStick(A0, A1, 2);              // Main control stick
Joystick auxStick(A2, A3, 3);               // Auxiliary stick
Lever throttle(ANALOG_LEVER, A4);           // Throttle
Lever flightMode(DIGITAL_LEVER, 4, 5);      // Flight mode selector (3 positions)
Lever sensitivity(DIGITAL_LEVER, 6, 7);     // Sensitivity selector (3 positions)

void setup() {
    Serial.begin(9600);
    Serial.println("Advanced Conditional Mapping Example");
    
    // Initialize all controls
    mainStick.begin();
    mainStick.setDeadZone(40, true);
    
    auxStick.begin();
    auxStick.setDeadZone(30, true);
    
    throttle.begin();
    throttle.setAnalogLimits(0, 4095, 0);
    
    flightMode.begin();
    flightMode.setDigitalPositions(3);  // 0=Stabilize, 1=Sport, 2=Manual
    
    sensitivity.begin();  
    sensitivity.setDigitalPositions(3); // 0=Low, 1=Normal, 2=High
    
    nrf.begin();
    nrf.setChannel(76);
    
    // Add all controls
    nrf.addJoystick(&mainStick, 0);
    nrf.addJoystick(&auxStick, 1);  
    nrf.addLever(&throttle, 0);
    nrf.addLever(&flightMode, 1);
    nrf.addLever(&sensitivity, 2);
    
    // ================ ADVANCED CONDITIONAL SETUP ================
    
    uint8_t profile = nrf.createProfile("Advanced");
    nrf.selectProfile(profile);
    
    // Base mapping (always active)
    nrf.mapJoystickToChannel(0, true, 1, -100, 100);   // Main X -> Channel 1
    nrf.mapJoystickToChannel(0, false, 2, -100, 100);  // Main Y -> Channel 2
    nrf.mapJoystickToChannel(1, true, 3, -100, 100);   // Aux X -> Channel 3  
    nrf.mapJoystickToChannel(1, false, 4, -100, 100);  // Aux Y -> Channel 4
    nrf.mapLeverToChannel(0, 5, 0, 100);               // Throttle -> Channel 5
    
    // CONDITIONAL MAPPINGS BASED ON FLIGHT MODE (Lever 1):
    
    // When Flight Mode = 0 (Stabilize): Reduced limits for safety
    nrf.mapJoystickConditional(0, true, 1, 1, 0, -60, 60);    // Main X limited to ±60
    nrf.mapJoystickConditional(0, false, 2, 1, 0, -60, 60);   // Main Y limited to ±60
    nrf.mapJoystickConditional(1, true, 3, 1, 0, -40, 40);    // Aux X limited to ±40
    
    // When Flight Mode = 1 (Sport): Normal limits
    nrf.mapJoystickConditional(0, true, 1, 1, 1, -100, 100);  // Main X normal
    nrf.mapJoystickConditional(0, false, 2, 1, 1, -100, 100); // Main Y normal
    
    // When Flight Mode = 2 (Manual): Extended limits for experts
    nrf.mapJoystickConditional(0, true, 1, 1, 2, -150, 150);  // Main X extended
    nrf.mapJoystickConditional(0, false, 2, 1, 2, -150, 150); // Main Y extended
    nrf.mapJoystickConditional(1, true, 3, 1, 2, -120, 120);  // Aux X extended
    
    // CONDITIONAL MAPPINGS BASED ON SENSITIVITY (Lever 2):
    
    // When Sensitivity = 0 (Low): Further reduce all limits
    nrf.mapJoystickConditional(0, true, 1, 2, 0, -30, 30);    // Very limited
    nrf.mapJoystickConditional(0, false, 2, 2, 0, -30, 30);   
    
    // When Sensitivity = 2 (High): Boost all limits  
    nrf.mapJoystickConditional(0, true, 1, 2, 2, -200, 200);  // Maximum sensitivity
    nrf.mapJoystickConditional(0, false, 2, 2, 2, -200, 200);
    
    nrf.enableAutoExecution(true, 30); // High frequency for smooth control
    
    Serial.println("Advanced conditional mapping configured!");
    Serial.println();
    Serial.println("Flight Mode Lever (positions):");
    Serial.println("  0 = Stabilize Mode (±60 limits)");  
    Serial.println("  1 = Sport Mode (±100 limits)");
    Serial.println("  2 = Manual Mode (±150 limits)");
    Serial.println();
    Serial.println("Sensitivity Lever (positions):");
    Serial.println("  0 = Low Sensitivity (±30 limits)");
    Serial.println("  1 = Normal Sensitivity (no change)");
    Serial.println("  2 = High Sensitivity (±200 limits)");
    Serial.println();
    Serial.println("The system combines both conditions!");
    Serial.println("Example: Flight=Manual + Sensitivity=High = ±200 limits");
    Serial.println("Example: Flight=Stabilize + Sensitivity=Low = ±30 limits");
    
    nrf.printProfile(0);
}

void loop() {
    // Execute the conditional mapping system
    nrf.executeProfiles();
    
    // Display current configuration
    static unsigned long lastDisplay = 0;
    if (millis() - lastDisplay > 3000) {
        int flightModePos = flightMode.getDigitalPosition();
        int sensitivityPos = sensitivity.getDigitalPosition();
        
        Serial.println("=== Current Configuration ===");
        
        Serial.print("Flight Mode: ");
        switch (flightModePos) {
            case 0: Serial.println("Stabilize (Safe)"); break;
            case 1: Serial.println("Sport (Normal)"); break;
            case 2: Serial.println("Manual (Expert)"); break;
        }
        
        Serial.print("Sensitivity: ");
        switch (sensitivityPos) {
            case 0: Serial.println("Low (Beginner)"); break;
            case 1: Serial.println("Normal"); break;
            case 2: Serial.println("High (Expert)"); break;
        }
        
        Serial.print("Main Stick: X=");
        Serial.print(mainStick.readX());
        Serial.print(", Y=");
        Serial.println(mainStick.readY());
        
        Serial.println("Mapped Channel Values:");
        for (int i = 1; i <= 5; i++) {
            if (nrf.isChannelUpdated(i)) {
                Serial.print("  Channel ");
                Serial.print(i);
                Serial.print(": ");
                Serial.println(nrf.getChannelValue(i));
            }
        }
        
        Serial.print("Packets sent: ");
        Serial.println(nrf.getStats().packetsSent);
        Serial.println();
        
        lastDisplay = millis();
    }
    
    delay(10);
}

/*
 * This example shows how powerful conditional mapping can be:
 * 
 * 1. Multiple conditions can affect the same control
 * 2. Conditions can override base mappings
 * 3. Perfect for creating user-friendly systems with:
 *    - Beginner/Expert modes
 *    - Safety/Performance modes  
 *    - Context-dependent behaviors
 * 
 * Real-world applications:
 * - Flight controllers with multiple safety levels
 * - Industrial equipment with operator skill levels
 * - Gaming controllers with difficulty settings
 * - Camera gimbals with stabilization modes
 * 
 * The system automatically calculates the final limits
 * based on all active conditions!
 */