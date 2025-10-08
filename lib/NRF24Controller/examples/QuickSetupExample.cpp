/**
 * Quick Setup Example - ULTRA SIMPLE!
 * 
 * This example shows the quickest way to get started.
 * Just 3 lines of configuration!
 */

#include <SPI.h>
#include <RF24.h>
#include <Joystick.h>
#include <Lever.h>
#include <NRF24Controller.h>

// Hardware setup
NRF24Controller nrf(9, 10);        // CE, CSN pins
Joystick joy(A0, A1, 2);          // X, Y, Button
Lever throttle(ANALOG_LEVER, A2);  // Throttle

void setup() {
    Serial.begin(9600);
    Serial.println("Quick Setup Example - 3 Lines and Done!");
    
    // Initialize hardware
    joy.begin();
    throttle.begin();
    nrf.begin();
    
    // Add controls
    nrf.addJoystick(&joy, 0);
    nrf.addLever(&throttle, 0);
    
    // ========== SUPER QUICK SETUP - CHOOSE ONE ==========
    
    // Option 1: Basic setup (1 joystick + 1 lever)
    nrf.quickSetupBasic();
    
    // Option 2: Drone setup (optimized for drones)
    // nrf.quickSetupDrone();
    
    // Option 3: Car setup (steering + throttle)
    // nrf.quickSetupCar(); 
    
    // Option 4: Plane setup (aileron, elevator, rudder, throttle)
    // nrf.quickSetupPlane();
    
    Serial.println("Setup complete! System running automatically...");
}

void loop() {
    // Just one line in loop - that's it!
    nrf.executeProfiles();
    
    delay(10);
}

/*
 * That's literally it! 
 * 
 * The quickSetup functions automatically:
 * - Create an appropriate profile
 * - Map controls to standard channels
 * - Enable auto-execution
 * - Configure optimal transmission rates
 * 
 * For most use cases, you don't need anything else!
 */