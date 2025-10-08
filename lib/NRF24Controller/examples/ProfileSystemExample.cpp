/**
 * Profile System Example - SUPER SIMPLE TO USE!
 * 
 * This example shows how to use the new profile system for flexible control mapping.
 * NO NEED TO WRITE COMPLEX CODE IN LOOP - JUST CONFIGURE AND FORGET!
 */

#include <SPI.h>
#include <RF24.h>
#include <Joystick.h>
#include <Lever.h>
#include <NRF24Controller.h>

// NRF24L01 pins
#define CE_PIN 9
#define CSN_PIN 10

// Create controller and controls
NRF24Controller nrf(CE_PIN, CSN_PIN);
Joystick joy(A0, A1, 2);                    // Main joystick
Lever throttle(ANALOG_LEVER, A2);           // Throttle lever
Lever modeSelect(DIGITAL_LEVER, 3, 4);      // Mode selection (3 positions)

void setup() {
    Serial.begin(9600);
    Serial.println("Profile System Example - EASY MODE!");
    
    // Initialize controls
    joy.begin();
    joy.setDeadZone(50, true);
    
    throttle.begin();
    throttle.setAnalogLimits(0, 4095, 0);
    
    modeSelect.begin();
    modeSelect.setDigitalPositions(3); // 3 positions: 0, 1, 2
    
    // Initialize NRF24
    nrf.begin();
    nrf.setChannel(76);
    nrf.setPowerLevel(POWER_HIGH);
    
    // Add controls to NRF controller
    nrf.addJoystick(&joy, 0);
    nrf.addLever(&throttle, 0);
    nrf.addLever(&modeSelect, 1);
    
    // =================  SETUP PROFILES - SUPER EASY!  =================
    
    // Profile 1: Basic Mode (Position 0 of mode selector)
    uint8_t basicProfile = nrf.createProfile("Basic");
    nrf.selectProfile(basicProfile);
    
    // Map joystick to channels with normal range
    nrf.mapJoystickToChannel(0, true, 1, -100, 100);   // X-axis -> Channel 1 (normal range)
    nrf.mapJoystickToChannel(0, false, 2, -100, 100);  // Y-axis -> Channel 2 (normal range)
    nrf.mapLeverToChannel(0, 3, 0, 100);               // Throttle -> Channel 3 (0-100%)
    
    // Profile 2: Precision Mode (Position 1 of mode selector)  
    uint8_t precisionProfile = nrf.createProfile("Precision");
    nrf.selectProfile(precisionProfile);
    
    // Map joystick with REDUCED range for precision
    nrf.mapJoystickToChannel(0, true, 1, -50, 50);     // X-axis -> Channel 1 (REDUCED: ±50)
    nrf.mapJoystickToChannel(0, false, 2, -50, 50);    // Y-axis -> Channel 2 (REDUCED: ±50)
    nrf.mapLeverToChannel(0, 3, 0, 60);                // Throttle -> Channel 3 (REDUCED: 0-60%)
    
    // Profile 3: Sport Mode (Position 2 of mode selector)
    uint8_t sportProfile = nrf.createProfile("Sport");
    nrf.selectProfile(sportProfile);
    
    // Map joystick with EXTENDED range for sport mode
    nrf.mapJoystickToChannel(0, true, 1, -150, 150);   // X-axis -> Channel 1 (EXTENDED: ±150)
    nrf.mapJoystickToChannel(0, false, 2, -150, 150);  // Y-axis -> Channel 2 (EXTENDED: ±150)  
    nrf.mapLeverToChannel(0, 3, 0, 120);               // Throttle -> Channel 3 (EXTENDED: 0-120%)
    
    // ================= CONDITIONAL MAPPING EXAMPLE =================
    
    // Advanced: Make joystick limits change based on mode selector position
    nrf.selectProfile("Basic");
    nrf.mapJoystickConditional(0, true, 1, 1, 1, -50, 50);    // If mode lever = pos 1, X = ±50
    nrf.mapJoystickConditional(0, true, 1, 1, 2, -150, 150);  // If mode lever = pos 2, X = ±150
    
    // Start with basic profile
    nrf.selectProfile("Basic");
    
    // Enable auto-execution - THIS IS THE MAGIC LINE!
    nrf.enableAutoExecution(true, 50); // Execute every 50ms automatically
    
    Serial.println("Setup complete! The system will now:");
    Serial.println("- Automatically switch profiles based on mode lever");
    Serial.println("- Send data every 50ms");
    Serial.println("- Apply different limits per mode");
    Serial.println();
    Serial.println("Move mode lever to switch between:");
    Serial.println("  Position 0: Basic Mode (normal limits)");
    Serial.println("  Position 1: Precision Mode (reduced limits)");
    Serial.println("  Position 2: Sport Mode (extended limits)");
    Serial.println();
    
    nrf.printAllProfiles(); // Show all configured profiles
}

void loop() {
    // ============= THIS IS ALL YOU NEED IN LOOP! =============
    
    // Automatically switch profiles based on mode selector
    int currentMode = modeSelect.getDigitalPosition();
    static int lastMode = -1;
    
    if (currentMode != lastMode) {
        switch (currentMode) {
            case 0:
                nrf.selectProfile("Basic");
                Serial.println("Switched to: Basic Mode");
                break;
            case 1:
                nrf.selectProfile("Precision");
                Serial.println("Switched to: Precision Mode (reduced limits)");
                break;
            case 2:
                nrf.selectProfile("Sport");
                Serial.println("Switched to: Sport Mode (extended limits)");
                break;
        }
        lastMode = currentMode;
    }
    
    // Execute profiles automatically - ONE LINE DOES EVERYTHING!
    nrf.executeProfiles();
    
    // Optional: Display current values every 2 seconds
    static unsigned long lastDisplay = 0;
    if (millis() - lastDisplay > 2000) {
        Serial.println("--- Current Status ---");
        Serial.print("Mode: ");
        Serial.println(nrf.getProfileName(nrf.getActiveProfile()));
        
        Serial.print("Joystick: X=");
        Serial.print(joy.readX());
        Serial.print(", Y=");
        Serial.println(joy.readY());
        
        Serial.print("Throttle: ");
        Serial.print(throttle.readMapped(0, 100));
        Serial.println("%");
        
        Serial.println("Channel Values:");
        Serial.print("  Ch1 (X): ");
        Serial.println(nrf.getChannelValue(1));
        Serial.print("  Ch2 (Y): ");
        Serial.println(nrf.getChannelValue(2));
        Serial.print("  Ch3 (Throttle): ");
        Serial.println(nrf.getChannelValue(3));
        
        TransmissionStats stats = nrf.getStats();
        Serial.print("Packets sent: ");
        Serial.print(stats.packetsSent);
        Serial.print(" (");
        Serial.print(stats.successRate, 1);
        Serial.println("% success)");
        
        Serial.println();
        lastDisplay = millis();
    }
    
    // That's it! No complex code needed in loop!
    delay(10);
}

/*
 * What this example demonstrates:
 * 
 * 1. SUPER SIMPLE SETUP:
 *    - Create profiles with one function call
 *    - Map controls to channels with one line
 *    - Enable auto-execution and forget!
 * 
 * 2. FLEXIBLE MAPPING:
 *    - Different limits per profile
 *    - Conditional mappings based on lever positions
 *    - Automatic profile switching
 * 
 * 3. NO COMPLEX LOOP CODE:
 *    - Just call nrf.executeProfiles() once
 *    - Everything else is automatic!
 * 
 * 4. REAL-WORLD USE CASES:
 *    - RC planes with different flight modes
 *    - Drones with beginner/expert modes  
 *    - Cars with eco/normal/sport modes
 *    - Industrial controls with safety/normal modes
 * 
 * The beauty is that you can create complex control systems
 * with just a few configuration lines, and the loop stays clean!
 */