/**
 * Simple NRF24Controller Example
 * 
 * This is a basic example showing minimal setup for quick testing.
 * Choose TRANSMITTER or RECEIVER mode by uncommenting the appropriate line.
 */

#include <SPI.h>
#include <RF24.h>
#include <Joystick.h>
#include <Lever.h>
#include <NRF24Controller.h>

// Uncomment ONE of these lines to choose mode
// #define TRANSMITTER_MODE
// #define RECEIVER_MODE

// NRF24L01 pins
#define CE_PIN 9
#define CSN_PIN 10

NRF24Controller nrf(CE_PIN, CSN_PIN);

#ifdef TRANSMITTER_MODE
    // Transmitter setup
    Joystick joy(A0, A1, 2);
    
    void setup() {
        Serial.begin(9600);
        Serial.println("Simple Transmitter");
        
        // Initialize controls
        joy.begin();
        joy.setDeadZone(50, true);
        
        // Initialize NRF24
        nrf.begin();
        nrf.setChannel(76);
        nrf.setPowerLevel(POWER_HIGH);
        
        // Add joystick
        nrf.addJoystick(&joy, 0);
        
        // Enable auto-send every 100ms
        nrf.setAutoSend(true, 100);
        
        Serial.println("Ready! Move joystick to transmit...");
    }
    
    void loop() {
        nrf.update(); // Handles automatic transmission
        
        // Show current values
        static unsigned long lastPrint = 0;
        if (millis() - lastPrint > 1000) {
            Serial.print("Joy X: "); Serial.print(joy.readX());
            Serial.print(", Y: "); Serial.print(joy.readY());
            Serial.print(", Packets sent: "); Serial.println(nrf.getStats().packetsSent);
            lastPrint = millis();
        }
        
        delay(10);
    }

#endif

#ifdef RECEIVER_MODE
    // Receiver setup
    void setup() {
        Serial.begin(9600);
        Serial.println("Simple Receiver");
        
        // Initialize NRF24
        nrf.begin();
        nrf.setChannel(76);
        nrf.setPowerLevel(POWER_HIGH);
        nrf.setAddresses(0xE8E8F0F0E2LL, 0xE8E8F0F0E1LL); // Swapped addresses
        
        nrf.startListening();
        Serial.println("Listening for data...");
    }
    
    void loop() {
        if (nrf.available()) {
            DataPacket packet;
            if (nrf.readData(packet)) {
                Serial.println("--- Received Data ---");
                
                for (uint8_t i = 0; i < packet.controlCount; i++) {
                    const ControlData& control = packet.controls[i];
                    
                    if (control.type == CONTROL_JOYSTICK) {
                        Serial.print("Joystick "); Serial.print(control.id);
                        Serial.print(": X="); Serial.print(control.valueX);
                        Serial.print(", Y="); Serial.println(control.valueY);
                    }
                }
                
                Serial.print("Packets received: ");
                Serial.println(nrf.getStats().packetsReceived);
            }
        }
        
        delay(10);
    }

#endif

// If neither mode is selected, show error
#if !defined(TRANSMITTER_MODE) && !defined(RECEIVER_MODE)
    void setup() {
        Serial.begin(9600);
        Serial.println("ERROR: Please uncomment either TRANSMITTER_MODE or RECEIVER_MODE");
    }
    
    void loop() {
        delay(1000);
    }
#endif