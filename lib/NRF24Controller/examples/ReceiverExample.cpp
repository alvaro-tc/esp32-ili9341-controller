/**
 * NRF24Controller Receiver Example
 * 
 * This example shows how to set up a receiver to decode
 * control data from the NRF24Controller transmitter.
 */

#include <SPI.h>
#include <RF24.h>
#include <Servo.h>
#include <NRF24Controller.h>

// NRF24L01 pins (adjust for your board)
#define CE_PIN 9
#define CSN_PIN 10

// Output pins for received control data
#define SERVO_AILERON_PIN 3
#define SERVO_ELEVATOR_PIN 5
#define SERVO_RUDDER_PIN 6
#define MOTOR_THROTTLE_PIN 11
#define LED_STATUS_PIN 13

// Create controller instance
NRF24Controller nrfController(CE_PIN, CSN_PIN);

// Create servo instances for output
Servo aileronServo;
Servo elevatorServo;
Servo rudderServo;
Servo throttleMotor;

// Control state variables
struct ReceivedControls {
    // Joystick data
    int mainJoystickX;      // Aileron control
    int mainJoystickY;      // Elevator control
    int auxJoystickX;       // Rudder control
    int auxJoystickY;       // Camera tilt
    bool mainJoystickBtn;   // Emergency/special function
    
    // Lever data
    int throttle;           // 0-100%
    int trim;               // Trim adjustment
    int gearPosition;       // Gear selection
    
    // System data
    int batteryVoltage;     // Battery level
    bool emergencyMode;     // Emergency flag
    unsigned long lastUpdate;
};

ReceivedControls controls;

// Failsafe values
const ReceivedControls FAILSAFE_VALUES = {
    0, 0, 0, 0, false,     // Joysticks centered, button not pressed
    0, 0, 1,               // Throttle 0%, trim center, gear neutral
    0, false, 0            // Battery unknown, no emergency
};

// Connection monitoring
unsigned long lastPacketTime = 0;
const unsigned long CONNECTION_TIMEOUT = 1000; // 1 second timeout
bool connectionLost = false;

void setup() {
    Serial.begin(9600);
    Serial.println("NRF24Controller Receiver Example");
    
    // Initialize output devices
    aileronServo.attach(SERVO_AILERON_PIN);
    elevatorServo.attach(SERVO_ELEVATOR_PIN);
    rudderServo.attach(SERVO_RUDDER_PIN);
    throttleMotor.attach(MOTOR_THROTTLE_PIN);
    
    pinMode(LED_STATUS_PIN, OUTPUT);
    
    // Initialize controls to failsafe values
    controls = FAILSAFE_VALUES;
    applyFailsafe();
    
    // Initialize NRF24 controller
    if (!nrfController.begin()) {
        Serial.println("Failed to initialize NRF24Controller!");
        while (1) {
            // Blink LED to indicate error
            digitalWrite(LED_STATUS_PIN, HIGH);
            delay(200);
            digitalWrite(LED_STATUS_PIN, LOW);
            delay(200);
        }
    }
    
    // Configure NRF24 settings (must match transmitter)
    nrfController.setChannel(76);
    nrfController.setPowerLevel(POWER_HIGH);
    nrfController.setDataRate(RATE_1MBPS);
    nrfController.setAddresses(0xE8E8F0F0E2LL, 0xE8E8F0F0E1LL); // Swapped for receiver
    
    // Start listening mode
    nrfController.startListening();
    
    Serial.println("Receiver ready! Waiting for control data...");
    nrfController.printStatus();
}

void loop() {
    // Check for incoming data
    if (nrfController.available()) {
        DataPacket receivedPacket;
        
        if (nrfController.readData(receivedPacket)) {
            processReceivedPacket(receivedPacket);
            lastPacketTime = millis();
            connectionLost = false;
            
            // Blink LED to show activity
            digitalWrite(LED_STATUS_PIN, HIGH);
        }
    } else {
        digitalWrite(LED_STATUS_PIN, LOW);
    }
    
    // Check for connection timeout
    if (millis() - lastPacketTime > CONNECTION_TIMEOUT) {
        if (!connectionLost) {
            Serial.println("Connection lost - applying failsafe!");
            connectionLost = true;
            applyFailsafe();
        }
    }
    
    // Apply received controls to outputs
    updateOutputs();
    
    // Display status periodically
    static unsigned long lastStatusDisplay = 0;
    if (millis() - lastStatusDisplay > 2000) { // Every 2 seconds
        displayStatus();
        lastStatusDisplay = millis();
    }
    
    delay(10); // Small delay for stability
}

void processReceivedPacket(const DataPacket& packet) {
    Serial.print("Received packet #");
    Serial.print(packet.packetId);
    Serial.print(" with ");
    Serial.print(packet.controlCount);
    Serial.println(" controls");
    
    // Process each control in the packet
    for (uint8_t i = 0; i < packet.controlCount; i++) {
        const ControlData& control = packet.controls[i];
        
        switch (control.type) {
            case CONTROL_JOYSTICK:
                processJoystickData(control);
                break;
                
            case CONTROL_LEVER_ANALOG:
            case CONTROL_LEVER_ENCODER:
            case CONTROL_LEVER_DIGITAL:
                processLeverData(control);
                break;
                
            case CONTROL_CUSTOM:
                processCustomData(control);
                break;
                
            case CONTROL_BUTTON:
                processButtonData(control);
                break;
                
            default:
                Serial.print("Unknown control type: ");
                Serial.println(control.type);
                break;
        }
    }
    
    controls.lastUpdate = millis();
}

void processJoystickData(const ControlData& control) {
    switch (control.id) {
        case 0: // Main joystick
            controls.mainJoystickX = control.valueX;
            controls.mainJoystickY = control.valueY;
            controls.mainJoystickBtn = (control.flags & 0x01) != 0;
            
            Serial.print("Main Joystick: X=");
            Serial.print(control.valueX);
            Serial.print(", Y=");
            Serial.print(control.valueY);
            if (controls.mainJoystickBtn) Serial.print(" [PRESSED]");
            Serial.println();
            break;
            
        case 1: // Auxiliary joystick
            controls.auxJoystickX = control.valueX;
            controls.auxJoystickY = control.valueY;
            
            Serial.print("Aux Joystick: X=");
            Serial.print(control.valueX);
            Serial.print(", Y=");
            Serial.println(control.valueY);
            break;
    }
}

void processLeverData(const ControlData& control) {
    // Lever IDs are offset by 100 from joystick IDs
    uint8_t leverId = control.id - 100;
    
    switch (leverId) {
        case 0: // Throttle lever
            controls.throttle = map(control.valueX, -100, 100, 0, 100);
            controls.throttle = constrain(controls.throttle, 0, 100);
            
            Serial.print("Throttle: ");
            Serial.print(controls.throttle);
            Serial.println("%");
            break;
            
        case 1: // Trim encoder
            controls.trim = control.valueX;
            
            Serial.print("Trim: ");
            Serial.println(controls.trim);
            break;
            
        case 2: // Gear lever
            controls.gearPosition = control.valueX;
            
            Serial.print("Gear Position: ");
            Serial.println(controls.gearPosition);
            break;
    }
}

void processCustomData(const ControlData& control) {
    switch (control.id) {
        case 255: // Battery voltage
            controls.batteryVoltage = control.valueX;
            
            Serial.print("Battery: ");
            Serial.print(map(controls.batteryVoltage, 0, 1023, 0, 500)); // Convert to voltage * 100
            Serial.println(" (raw ADC)");
            break;
            
        default:
            Serial.print("Custom data ID ");
            Serial.print(control.id);
            Serial.print(": ");
            Serial.println(control.valueX);
            break;
    }
}

void processButtonData(const ControlData& control) {
    switch (control.id) {
        case 254: // Emergency signal
            controls.emergencyMode = (control.flags & 0x80) != 0;
            
            if (controls.emergencyMode) {
                Serial.println("*** EMERGENCY MODE ACTIVATED ***");
                // Handle emergency procedures
                applyEmergencyProcedure();
            }
            break;
    }
}

void updateOutputs() {
    if (connectionLost) {
        return; // Don't update outputs during connection loss
    }
    
    // Convert joystick values to servo positions (1000-2000 microseconds)
    int aileronPos = map(controls.mainJoystickX + controls.trim, -100, 100, 1000, 2000);
    int elevatorPos = map(controls.mainJoystickY, -100, 100, 1000, 2000);
    int rudderPos = map(controls.auxJoystickX, -100, 100, 1000, 2000);
    int throttlePos = map(controls.throttle, 0, 100, 1000, 2000);
    
    // Apply servo positions
    aileronServo.writeMicroseconds(aileronPos);
    elevatorServo.writeMicroseconds(elevatorPos);
    rudderServo.writeMicroseconds(rudderPos);
    throttleMotor.writeMicroseconds(throttlePos);
    
    // Handle gear position (example: control additional servo or relay)
    // if (controls.gearPosition == 0) {
    //     // Gear up
    // } else if (controls.gearPosition == 2) {
    //     // Gear down
    // }
}

void applyFailsafe() {
    // Set all servos to safe positions
    aileronServo.writeMicroseconds(1500);   // Center
    elevatorServo.writeMicroseconds(1500);  // Center
    rudderServo.writeMicroseconds(1500);    // Center
    throttleMotor.writeMicroseconds(1000);  // Minimum throttle
    
    // Reset control values
    controls = FAILSAFE_VALUES;
    
    Serial.println("Failsafe applied - all controls in safe position");
}

void applyEmergencyProcedure() {
    // Example emergency procedure
    Serial.println("Executing emergency procedure...");
    
    // Cut throttle immediately
    throttleMotor.writeMicroseconds(1000);
    controls.throttle = 0;
    
    // Level the aircraft (center controls)
    aileronServo.writeMicroseconds(1500);
    elevatorServo.writeMicroseconds(1500);
    rudderServo.writeMicroseconds(1500);
    
    // Flash LED rapidly
    for (int i = 0; i < 10; i++) {
        digitalWrite(LED_STATUS_PIN, HIGH);
        delay(100);
        digitalWrite(LED_STATUS_PIN, LOW);
        delay(100);
    }
}

void displayStatus() {
    Serial.println("=== Receiver Status ===");
    Serial.print("Connection: ");
    Serial.println(connectionLost ? "LOST" : "OK");
    
    if (!connectionLost) {
        Serial.print("Last update: ");
        Serial.print(millis() - controls.lastUpdate);
        Serial.println("ms ago");
        
        Serial.print("Main Stick: X=");
        Serial.print(controls.mainJoystickX);
        Serial.print(", Y=");
        Serial.println(controls.mainJoystickY);
        
        Serial.print("Throttle: ");
        Serial.print(controls.throttle);
        Serial.println("%");
        
        if (controls.emergencyMode) {
            Serial.println("*** EMERGENCY MODE ACTIVE ***");
        }
    }
    
    TransmissionStats stats = nrfController.getStats();
    Serial.print("Packets received: ");
    Serial.println(stats.packetsReceived);
    
    Serial.println();
}

/*
 * Advanced Receiver Features:
 * 
 * 1. Data validation and filtering:
 *    if (abs(newValue - lastValue) > MAX_CHANGE) {
 *        // Reject suspicious data
 *        return;
 *    }
 * 
 * 2. Servo output curves:
 *    int applyCurve(int input) {
 *        // Apply exponential curve for smoother control
 *        float normalized = input / 100.0;
 *        float curved = normalized * normalized * normalized;
 *        return curved * 100;
 *    }
 * 
 * 3. Multiple receiver setup:
 *    // Use different channels for multiple receivers
 *    if (receiverID == RECEIVER_A) {
 *        nrfController.setChannel(76);
 *    } else {
 *        nrfController.setChannel(82);
 *    }
 * 
 * 4. Data logging:
 *    void logControlData() {
 *        // Log to SD card or EEPROM
 *        dataFile.print(millis());
 *        dataFile.print(",");
 *        dataFile.print(controls.throttle);
 *        dataFile.println();
 *    }
 * 
 * 5. Acknowledgment with status data:
 *    DataPacket ackPacket;
 *    ackPacket.controls[0].valueX = batteryLevel;
 *    ackPacket.controls[0].valueY = signalStrength;
 *    nrfController.sendCustomPacket(ackPacket);
 */