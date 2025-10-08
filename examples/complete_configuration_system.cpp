/**
 * Ejemplo Completo: Sistema de Configuración NRF24Controller
 * 
 * Este ejemplo demuestra:
 * 1. Cargar configuraciones predefinidas
 * 2. Crear configuración personalizada
 * 3. Guardar/cargar desde EEPROM
 * 4. Usar archivos de configuración
 */

#include <Arduino.h>
#include <SPI.h>
#include <RF24.h>
#include "NRF24Controller.h"
#include "NRF24Config.h"

// Crear controlador
NRF24Controller controller(9, 10);

// Variables globales
SystemConfig currentConfig;
bool configurationMode = false;

void setup() {
    Serial.begin(115200);
    while (!Serial) delay(10);
    
    Serial.println("=== NRF24Controller Configuration System Demo ===\n");
    
    // Mostrar menú de opciones
    showConfigurationMenu();
    
    // Configuración inicial por defecto
    loadDefaultConfiguration();
}

void loop() {
    // Manejar comandos por Serial
    if (Serial.available()) {
        handleSerialCommand();
    }
    
    // Si no estamos en modo configuración, transmitir datos
    if (!configurationMode) {
        controller.update();
        delay(50);
    }
}

void showConfigurationMenu() {
    Serial.println("Comandos disponibles:");
    Serial.println("1 - Cargar configuración básica");
    Serial.println("2 - Cargar configuración para drone");
    Serial.println("3 - Cargar configuración para RC car");
    Serial.println("4 - Cargar configuración para avión RC");
    Serial.println("5 - Crear configuración personalizada");
    Serial.println("6 - Guardar configuración en EEPROM");
    Serial.println("7 - Cargar configuración desde EEPROM");
    Serial.println("8 - Mostrar configuración actual");
    Serial.println("9 - Calibrar controles");
    Serial.println("0 - Alternar modo configuración/transmisión");
    Serial.println("h - Mostrar este menú");
    Serial.println();
}

void handleSerialCommand() {
    char command = Serial.read();
    
    switch (command) {
        case '1':
            loadBasicConfiguration();
            break;
        case '2':
            loadDroneConfiguration();
            break;
        case '3':
            loadCarConfiguration();
            break;
        case '4':
            loadPlaneConfiguration();
            break;
        case '5':
            createCustomConfiguration();
            break;
        case '6':
            saveConfigurationToEEPROM();
            break;
        case '7':
            loadConfigurationFromEEPROM();
            break;
        case '8':
            showCurrentConfiguration();
            break;
        case '9':
            calibrateControls();
            break;
        case '0':
            toggleTransmissionMode();
            break;
        case 'h':
        case 'H':
            showConfigurationMenu();
            break;
    }
}

void loadDefaultConfiguration() {
    Serial.println(">>> Cargando configuración básica por defecto...");
    
    currentConfig = NRF24Config::loadDefault();
    
    if (NRF24Config::validateConfig(currentConfig)) {
        controller.applySystemConfig(currentConfig);
        Serial.println("✅ Configuración básica cargada correctamente");
    } else {
        Serial.println("❌ Error en la configuración básica");
    }
    Serial.println();
}

void loadBasicConfiguration() {
    Serial.println(">>> Cargando configuración básica...");
    
    currentConfig = NRF24Config::createBasicSetup(9, 10);
    controller.applySystemConfig(currentConfig);
    
    Serial.println("✅ Configuración básica aplicada:");
    Serial.println("   - 1 Joystick (A0, A1, pin 2)");
    Serial.println("   - 1 Palanca analógica (A2)");
    Serial.println("   - NRF24L01 en pines 9/10");
    Serial.println();
}

void loadDroneConfiguration() {
    Serial.println(">>> Cargando configuración para drone...");
    
    currentConfig = NRF24Config::createDroneSetup(9, 10);
    controller.applySystemConfig(currentConfig);
    
    Serial.println("✅ Configuración de drone aplicada:");
    Serial.println("   - Stick derecho: Roll/Pitch (A0, A1)");
    Serial.println("   - Stick izquierdo: Throttle/Yaw (A2, A3)");
    Serial.println("   - Switch de modo de vuelo");
    Serial.println("   - Transmisión rápida (20ms)");
    Serial.println();
}

void loadCarConfiguration() {
    Serial.println(">>> Cargando configuración para RC car...");
    
    currentConfig = NRF24Config::createCarSetup(9, 10);
    controller.applySystemConfig(currentConfig);
    
    Serial.println("✅ Configuración de RC car aplicada:");
    Serial.println("   - Volante (solo eje X, A0)");
    Serial.println("   - Acelerador/Freno (A2)");
    Serial.println("   - Selector de marchas");
    Serial.println();
}

void loadPlaneConfiguration() {
    Serial.println(">>> Cargando configuración para avión...");
    
    currentConfig = NRF24Config::loadPlaneConfig();
    controller.applySystemConfig(currentConfig);
    
    Serial.println("✅ Configuración de avión aplicada:");
    Serial.println("   - Stick primario: Alerones/Elevador");
    Serial.println("   - Stick secundario: Timón/Acelerador");
    Serial.println("   - Trim encoder");
    Serial.println("   - Modo de vuelo");
    Serial.println();
}

void createCustomConfiguration() {
    Serial.println(">>> Creando configuración personalizada...");
    
    // Empezar con configuración vacía
    currentConfig = controller.getDefaultConfig();
    
    // Configurar NRF24L01
    NRF24Config::configureNRF24(currentConfig, 9, 10, 76, POWER_HIGH);
    
    // Configurar joystick personalizado
    NRF24Config::configureJoystick(currentConfig, 0, A0, A1, 2, "CustomStick");
    NRF24Config::setJoystickCalibration(currentConfig, 0, 
                                        100, 3995, 2048,    // X: min, max, center
                                        100, 3995, 2048,    // Y: min, max, center
                                        50);                // dead zone
    
    // Configurar palanca personalizada
    NRF24Config::configureLever(currentConfig, 0, ANALOG_LEVER, A2, 255, 255, "CustomLever");
    NRF24Config::setLeverCalibration(currentConfig, 0, 0, 4095, 0, 30);
    
    // Aplicar configuración
    controller.applySystemConfig(currentConfig);
    
    Serial.println("✅ Configuración personalizada creada:");
    Serial.println("   - Joystick calibrado (A0, A1)");
    Serial.println("   - Palanca analógica (A2)");
    Serial.println("   - Configuración optimizada");
    Serial.println();
}

void saveConfigurationToEEPROM() {
    Serial.println(">>> Guardando configuración en EEPROM...");
    
    if (controller.saveSystemConfig()) {
        Serial.println("✅ Configuración guardada en slot 0");
        
        // También guardar en otros slots como ejemplo
        if (controller.saveSystemConfig(1)) {
            Serial.println("✅ Configuración guardada en slot 1 (backup)");
        }
    } else {
        Serial.println("❌ Error al guardar configuración");
    }
    Serial.println();
}

void loadConfigurationFromEEPROM() {
    Serial.println(">>> Cargando configuración desde EEPROM...");
    
    if (controller.loadSystemConfig()) {
        currentConfig = controller.getCurrentSystemConfig();
        Serial.println("✅ Configuración cargada desde EEPROM");
        
        // Mostrar información básica de la configuración cargada
        Serial.print("   Sistema: ");
        Serial.println(currentConfig.systemName);
        Serial.print("   Canal NRF: ");
        Serial.println(currentConfig.nrfChannel);
    } else {
        Serial.println("❌ Error al cargar configuración desde EEPROM");
        Serial.println("   Usando configuración por defecto");
        loadDefaultConfiguration();
    }
    Serial.println();
}

void showCurrentConfiguration() {
    Serial.println(">>> Configuración actual:");
    NRF24Config::printConfig(currentConfig);
    
    // Mostrar estadísticas adicionales
    Serial.println("\nEstadísticas de sistema:");
    Serial.print("Controles activos: ");
    
    uint8_t activeJoysticks = 0, activeLevers = 0;
    for (uint8_t i = 0; i < MAX_JOYSTICKS; i++) {
        if (currentConfig.joysticks[i].enabled) activeJoysticks++;
    }
    for (uint8_t i = 0; i < MAX_LEVERS; i++) {
        if (currentConfig.levers[i].enabled) activeLevers++;
    }
    
    Serial.print(activeJoysticks);
    Serial.print(" joysticks, ");
    Serial.print(activeLevers);
    Serial.println(" palancas");
    Serial.println();
}

void calibrateControls() {
    Serial.println(">>> Iniciando calibración de controles...");
    configurationMode = true;
    
    // Calibrar joysticks activos
    for (uint8_t i = 0; i < MAX_JOYSTICKS; i++) {
        if (currentConfig.joysticks[i].enabled) {
            calibrateJoystick(i);
        }
    }
    
    // Calibrar palancas activas
    for (uint8_t i = 0; i < MAX_LEVERS; i++) {
        if (currentConfig.levers[i].enabled && 
            currentConfig.levers[i].type == ANALOG_LEVER) {
            calibrateLever(i);
        }
    }
    
    // Aplicar nueva calibración
    controller.applySystemConfig(currentConfig);
    
    Serial.println("✅ Calibración completada");
    configurationMode = false;
    Serial.println();
}

void calibrateJoystick(uint8_t id) {
    auto& joy = currentConfig.joysticks[id];
    
    Serial.print("Calibrando ");
    Serial.print(joy.name);
    Serial.println("...");
    
    Serial.println("Mueve el joystick a todas las posiciones y presiona Enter...");
    
    int minX = 4095, maxX = 0, minY = 4095, maxY = 0;
    int centerX = 0, centerY = 0, samples = 0;
    
    // Período de calibración
    unsigned long startTime = millis();
    while (millis() - startTime < 5000) { // 5 segundos
        int x = analogRead(joy.pinX);
        int y = analogRead(joy.pinY);
        
        if (x < minX) minX = x;
        if (x > maxX) maxX = x;
        if (y < minY) minY = y;
        if (y > maxY) maxY = y;
        
        centerX += x;
        centerY += y;
        samples++;
        
        delay(10);
    }
    
    // Actualizar calibración
    joy.minX = minX;
    joy.maxX = maxX;
    joy.centerX = centerX / samples;
    joy.minY = minY;
    joy.maxY = maxY;
    joy.centerY = centerY / samples;
    
    Serial.print("✅ ");
    Serial.print(joy.name);
    Serial.println(" calibrado");
}

void calibrateLever(uint8_t id) {
    auto& lever = currentConfig.levers[id];
    
    Serial.print("Calibrando ");
    Serial.print(lever.name);
    Serial.println("...");
    
    Serial.println("Mueve la palanca por todo su rango y presiona Enter...");
    
    int minPos = 4095, maxPos = 0;
    
    // Período de calibración
    unsigned long startTime = millis();
    while (millis() - startTime < 3000) { // 3 segundos
        int pos = analogRead(lever.pinA);
        
        if (pos < minPos) minPos = pos;
        if (pos > maxPos) maxPos = pos;
        
        delay(10);
    }
    
    // Actualizar calibración
    lever.minPosition = minPos;
    lever.maxPosition = maxPos;
    lever.centerPosition = (minPos + maxPos) / 2;
    
    Serial.print("✅ ");
    Serial.print(lever.name);
    Serial.println(" calibrada");
}

void toggleTransmissionMode() {
    configurationMode = !configurationMode;
    
    if (configurationMode) {
        Serial.println(">>> Modo CONFIGURACIÓN activado");
        Serial.println("   Transmisión pausada");
    } else {
        Serial.println(">>> Modo TRANSMISIÓN activado");
        Serial.println("   Enviando datos...");
    }
    Serial.println();
}