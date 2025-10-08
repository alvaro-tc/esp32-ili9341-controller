/**
 * NRF24Controller Transmitter Example
 * 
 * This example shows how to set up a transmitter with multiple
 * joysticks and levers using the NRF24Controller library.
 */

#include <SPI.h>
#include <RF24.h>
#include <Joystick.h>
#include <Lever.h>
#include <NRF24Controller.h>

// NRF24L01 pins (adjust for your board)
#define CE_PIN 9
#define CSN_PIN 10

// Create controller instance
NRF24Controller nrfController(CE_PIN, CSN_PIN);

// Create control instances
Joystick mainJoystick(A0, A1, 2);        // Primary flight stick
Joystick auxJoystick(A2, A3, 3);         // Secondary control stick
Lever throttleLever(ANALOG_LEVER, A4);   // Throttle control
Lever trimEncoder(ROTARY_ENCODER, 4, 5, 6); // Trim adjustment
Lever gearLever(DIGITAL_LEVER, 7, 8);    // Gear selection

void setup() {
    Serial.begin(9600);
    Serial.println("NRF24Controller Transmitter Example");
    
    // Initialize controls
    mainJoystick.begin();
    auxJoystick.begin();
    throttleLever.begin();
    trimEncoder.begin();
    gearLever.begin();
    
    // Configure joysticks
    mainJoystick.setDeadZone(60, true);
    mainJoystick.setSmoothing(true, 0.2);
    mainJoystick.invertAxis(false, true); // Invert Y for aircraft control
    
    auxJoystick.setDeadZone(40, true);
    auxJoystick.setSmoothing(true, 0.15);
    
    // Configure levers
    throttleLever.setAnalogLimits(0, 4095, 0); // 0-100% throttle range
    throttleLever.setSmoothing(true, 0.1);
    
    trimEncoder.setEncoderLimits(-50, 50);
    trimEncoder.setStepsPerDetent(2);
    
    gearLever.setDigitalPositions(3); // Up, Neutral, Down
    
    // Initialize NRF24 controller
    if (!nrfController.begin()) {
        Serial.println("Failed to initialize NRF24Controller!");
        while (1); // Halt on failure
    }
    
    // Configure NRF24 settings
    nrfController.setChannel(76);                    // Set channel (0-125)
    nrfController.setPowerLevel(POWER_HIGH);         // Set power level
    nrfController.setDataRate(RATE_1MBPS);          // Set data rate
    nrfController.setAddresses(0xE8E8F0F0E1LL, 0xE8E8F0F0E2LL); // TX, RX addresses
    
    // Add controls to the controller
    nrfController.addJoystick(&mainJoystick, 0);    // ID 0
    nrfController.addJoystick(&auxJoystick, 1);     // ID 1
    nrfController.addLever(&throttleLever, 0);      // ID 0
    nrfController.addLever(&trimEncoder, 1);        // ID 1
    nrfController.addLever(&gearLever, 2);          // ID 2
    
    // Configure transmission settings
    nrfController.setAutoSend(true, 50);            // Auto-send every 50ms
    nrfController.setSendThresholds(5, 3);          // Joystick: 5, Lever: 3
    nrfController.setSendOnlyChanges(true);         // Send only when values change
    
    // Optional: Enable only specific controls
    // nrfController.enableJoystick(0, true);       // Enable main joystick
    // nrfController.enableJoystick(1, false);      // Disable aux joystick
    // nrfController.enableLever(0, true);          // Enable throttle
    
    Serial.println("Transmitter ready! Move controls to send data...");
    nrfController.printStatus();
}

void loop() {
    // Update the controller (handles auto-send and lever updates)
    nrfController.update();
    
    // Manual send example (if auto-send is disabled)
    // if (someCondition) {
    //     nrfController.sendData();
    // }
    
    // Send custom data example
    static unsigned long lastCustomSend = 0;
    if (millis() - lastCustomSend > 1000) { // Every 1 second
        // Send battery voltage as custom data
        ControlData batteryData;
        batteryData.id = 255;                    // Special ID for battery
        batteryData.type = CONTROL_CUSTOM;
        batteryData.valueX = analogRead(7);     // Battery voltage reading
        batteryData.valueY = 0;
        batteryData.flags = 0;
        batteryData.timestamp = millis();
        
        nrfController.sendCustomData(batteryData);
        lastCustomSend = millis();
    }
    
    // Display status periodically
    static unsigned long lastStatus = 0;
    if (millis() - lastStatus > 5000) { // Every 5 seconds
        Serial.println("--- Status Update ---");
        Serial.print("Main Joystick: X="); Serial.print(mainJoystick.readX());
        Serial.print(", Y="); Serial.println(mainJoystick.readY());
        
        Serial.print("Throttle: "); Serial.print(throttleLever.readMapped(0, 100)); Serial.println("%");
        
        Serial.print("Trim: "); Serial.println(trimEncoder.readPosition());
        
        Serial.print("Gear: Position "); Serial.println(gearLever.getDigitalPosition());
        
        TransmissionStats stats = nrfController.getStats();
        Serial.print("Transmission success rate: "); Serial.print(stats.successRate, 1); Serial.println("%");
        Serial.print("Packets sent: "); Serial.println(stats.packetsSent);
        
        lastStatus = millis();
    }
    
    // Handle button presses for special functions
    if (mainJoystick.wasPressed()) {
        Serial.println("Main joystick button pressed - sending emergency packet");
        
        // Send emergency/special packet
        ControlData emergencyData;
        emergencyData.id = 254;                  // Emergency ID
        emergencyData.type = CONTROL_BUTTON;
        emergencyData.valueX = 1;                // Emergency flag
        emergencyData.valueY = mainJoystick.readMagnitude() * 100; // Current magnitude
        emergencyData.flags = 0x80;              // Emergency flag in MSB
        emergencyData.timestamp = millis();
        
        nrfController.sendCustomData(emergencyData);
    }
    
    if (trimEncoder.wasPressed()) {
        Serial.println("Trim encoder pressed - resetting trim to center");
        trimEncoder.resetEncoder();
    }
    
    delay(10); // Small delay for stability
}

/*
 * Advanced Configuration Examples:
 * 
 * 1. Different power levels for different ranges:
 *    nrfController.setPowerLevel(POWER_MIN);   // Short range, low power
 *    nrfController.setPowerLevel(POWER_MAX);   // Long range, high power
 * 
 * 2. Channel scanning and optimization:
 *    nrfController.scanChannels();             // Scan for interference
 *    uint8_t bestChannel = nrfController.getOptimalChannel();
 *    nrfController.setChannel(bestChannel);
 * 
 * 3. Dynamic control enabling/disabling:
 *    if (digitalRead(MODE_SWITCH) == HIGH) {
 *        nrfController.enableJoystick(1, false); // Disable aux joystick
 *        nrfController.enableLever(2, false);    // Disable gear lever
 *    }
 * 
 * 4. Conditional sending:
 *    nrfController.setAutoSend(false);         // Disable auto-send
 *    if (mainJoystick.isMoving() || throttleLever.isMoving()) {
 *        nrfController.sendData();             // Send only when moving
 *    }
 * 
 * 5. Power management:
 *    if (batteryLow) {
 *        nrfController.setPowerLevel(POWER_MIN); // Reduce power
 *        nrfController.setAutoSend(true, 100);   // Reduce send rate
 *    }
 */