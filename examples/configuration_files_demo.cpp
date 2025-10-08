/**
 * Ejemplo: Configuración Personalizada con Archivos
 * 
 * Este ejemplo muestra cómo crear configuraciones personalizadas
 * usando strings de configuración y el sistema de parsing inteligente.
 */

#include <Arduino.h>
#include "NRF24Controller.h"
#include "NRF24Config.h"

// Configuración personalizada para simulador de vuelo
const char* FLIGHT_SIM_CONFIG = R"(
# Configuración para Simulador de Vuelo
# Compatible con Microsoft Flight Simulator, X-Plane, etc.

SYSTEM_NAME=Flight Simulator Controller
DEBUG_MODE=true
TRANSMISSION_INTERVAL=20

# NRF24L01 configuración de alta velocidad
NRF_CE_PIN=9
NRF_CSN_PIN=10
NRF_CHANNEL=100
NRF_POWER=MAX
NRF_DATA_RATE=2MBPS
NRF_TX_ADDRESS=0xE8E8F0F0E1
NRF_RX_ADDRESS=0xE8E8F0F0E2

# Joystick Principal (Yoke/Stick)
JOY0_ENABLED=true
JOY0_NAME=PrimaryYoke
JOY0_PIN_X=A0
JOY0_PIN_Y=A1
JOY0_PIN_BUTTON=2
JOY0_MIN_X=50
JOY0_MAX_X=4045
JOY0_CENTER_X=2048
JOY0_MIN_Y=50
JOY0_MAX_Y=4045
JOY0_CENTER_Y=2048
JOY0_DEAD_ZONE=30
JOY0_INVERT_X=false
JOY0_INVERT_Y=true
JOY0_SMOOTHING=0.1

# Joystick Secundario (Rudder/View)
JOY1_ENABLED=true
JOY1_NAME=RudderView
JOY1_PIN_X=A2
JOY1_PIN_Y=A3
JOY1_PIN_BUTTON=3
JOY1_MIN_X=100
JOY1_MAX_X=3995
JOY1_CENTER_X=2048
JOY1_MIN_Y=100
JOY1_MAX_Y=3995
JOY1_CENTER_Y=2048
JOY1_DEAD_ZONE=40
JOY1_INVERT_X=false
JOY1_INVERT_Y=false
JOY1_SMOOTHING=0.15

# Palanca de Acelerador
LEV0_ENABLED=true
LEV0_NAME=Throttle
LEV0_TYPE=ANALOG
LEV0_PIN_A=A4
LEV0_PIN_B=255
LEV0_PIN_BUTTON=255
LEV0_MIN_POS=100
LEV0_MAX_POS=3995
LEV0_CENTER_POS=100
LEV0_DEAD_ZONE=20
LEV0_INVERT=false
LEV0_SMOOTHING=0.05

# Control de Flaps (Encoder)
LEV1_ENABLED=true
LEV1_NAME=Flaps
LEV1_TYPE=ENCODER
LEV1_PIN_A=4
LEV1_PIN_B=5
LEV1_PIN_BUTTON=6
LEV1_STEPS_PER_DETENT=4
LEV1_MIN_STEPS=-20
LEV1_MAX_STEPS=20

# Selector de Modo
LEV2_ENABLED=true
LEV2_NAME=ModeSelector
LEV2_TYPE=DIGITAL
LEV2_PIN_A=7
LEV2_PIN_B=8
LEV2_PIN_BUTTON=255
LEV2_DIGITAL_POSITIONS=5
)";

// Configuración para robot explorador
const char* ROBOT_CONFIG = R"(
# Configuración para Robot Explorador

SYSTEM_NAME=Explorer Robot
DEBUG_MODE=false
TRANSMISSION_INTERVAL=100

# NRF24L01 para largo alcance
NRF_CE_PIN=9
NRF_CSN_PIN=10
NRF_CHANNEL=50
NRF_POWER=MAX
NRF_DATA_RATE=250KBPS
NRF_TX_ADDRESS=0xE8E8F0F0E1
NRF_RX_ADDRESS=0xE8E8F0F0E2

# Control de Movimiento
JOY0_ENABLED=true
JOY0_NAME=Movement
JOY0_PIN_X=A0
JOY0_PIN_Y=A1
JOY0_PIN_BUTTON=2
JOY0_MIN_X=0
JOY0_MAX_X=4095
JOY0_CENTER_X=2048
JOY0_MIN_Y=0
JOY0_MAX_Y=4095
JOY0_CENTER_Y=2048
JOY0_DEAD_ZONE=100
JOY0_INVERT_X=false
JOY0_INVERT_Y=true
JOY0_SMOOTHING=0.3

# Control de Cámara
JOY1_ENABLED=true
JOY1_NAME=Camera
JOY1_PIN_X=A2
JOY1_PIN_Y=A3
JOY1_PIN_BUTTON=3
JOY1_MIN_X=200
JOY1_MAX_X=3895
JOY1_CENTER_X=2048
JOY1_MIN_Y=200
JOY1_MAX_Y=3895
JOY1_CENTER_Y=2048
JOY1_DEAD_ZONE=80
JOY1_INVERT_X=false
JOY1_INVERT_Y=false
JOY1_SMOOTHING=0.25

# Control de Velocidad
LEV0_ENABLED=true
LEV0_NAME=Speed
LEV0_TYPE=ANALOG
LEV0_PIN_A=A4
LEV0_PIN_B=255
LEV0_PIN_BUTTON=255
LEV0_MIN_POS=0
LEV0_MAX_POS=4095
LEV0_CENTER_POS=0
LEV0_DEAD_ZONE=50
LEV0_INVERT=false
LEV0_SMOOTHING=0.2

# Selector de Modo
LEV1_ENABLED=true
LEV1_NAME=RobotMode
LEV1_TYPE=DIGITAL
LEV1_PIN_A=4
LEV1_PIN_B=5
LEV1_PIN_BUTTON=255
LEV1_DIGITAL_POSITIONS=4
)";

NRF24Controller controller(9, 10);
SystemConfig currentConfig;

void setup() {
    Serial.begin(115200);
    while (!Serial) delay(10);
    
    Serial.println("=== Sistema de Archivos de Configuración ===\n");
    
    demonstrateConfigurationParsing();
}

void loop() {
    // Manejar comandos
    if (Serial.available()) {
        char cmd = Serial.read();
        
        switch (cmd) {
            case '1':
                loadFlightSimConfig();
                break;
            case '2':
                loadRobotConfig();
                break;
            case '3':
                createCustomConfigFromScratch();
                break;
            case '4':
                demonstrateConfigurationEditing();
                break;
            case '5':
                saveAndLoadConfigurationExample();
                break;
            case 's':
                showCurrentStatus();
                break;
            case 'h':
                showHelp();
                break;
        }
    }
    
    // Actualizar controlador
    controller.update();
    delay(50);
}

void demonstrateConfigurationParsing() {
    Serial.println(">>> Demostrando parsing de configuraciones...\n");
    
    Serial.println("Configuraciones disponibles:");
    Serial.println("1 - Simulador de Vuelo (5 controles)");
    Serial.println("2 - Robot Explorador (4 controles)");
    Serial.println("3 - Crear configuración desde cero");
    Serial.println("4 - Editar configuración existente");
    Serial.println("5 - Guardar/Cargar ejemplo completo");
    Serial.println("s - Mostrar estado actual");
    Serial.println("h - Ayuda");
    Serial.println();
    
    // Cargar configuración por defecto
    loadFlightSimConfig();
}

void loadFlightSimConfig() {
    Serial.println(">>> Cargando configuración de simulador de vuelo...");
    
    // Parsear configuración desde string
    currentConfig = NRF24Config::loadFromString(FLIGHT_SIM_CONFIG);
    
    // Validar configuración
    if (NRF24Config::validateConfig(currentConfig)) {
        // Aplicar al controlador
        controller.applySystemConfig(currentConfig);
        
        Serial.println("✅ Configuración de simulador cargada:");
        Serial.println("   - Yoke principal (alerones/elevador)");
        Serial.println("   - Control rudder/vista");
        Serial.println("   - Acelerador analógico");
        Serial.println("   - Flaps con encoder");
        Serial.println("   - Selector de modo (5 posiciones)");
        Serial.println("   - Optimizado para alta velocidad");
        
        // Mostrar detalles técnicos
        Serial.print("   Canal NRF: "); Serial.println(currentConfig.nrfChannel);
        Serial.print("   Intervalo: "); Serial.print(currentConfig.transmissionInterval); Serial.println("ms");
        
    } else {
        Serial.println("❌ Error en la configuración del simulador");
    }
    Serial.println();
}

void loadRobotConfig() {
    Serial.println(">>> Cargando configuración de robot explorador...");
    
    currentConfig = NRF24Config::loadFromString(ROBOT_CONFIG);
    
    if (NRF24Config::validateConfig(currentConfig)) {
        controller.applySystemConfig(currentConfig);
        
        Serial.println("✅ Configuración de robot cargada:");
        Serial.println("   - Control de movimiento (X/Y)");
        Serial.println("   - Control de cámara (pan/tilt)");
        Serial.println("   - Control de velocidad");
        Serial.println("   - Selector de modo robot");
        Serial.println("   - Optimizado para largo alcance");
        
        Serial.print("   Canal NRF: "); Serial.println(currentConfig.nrfChannel);
        Serial.print("   Suavizado: Activado para estabilidad");
        
    } else {
        Serial.println("❌ Error en la configuración del robot");
    }
    Serial.println();
}

void createCustomConfigFromScratch() {
    Serial.println(">>> Creando configuración personalizada desde cero...");
    
    // Crear configuración personalizada usando las funciones helper
    currentConfig = controller.getDefaultConfig();
    
    // Configurar sistema
    strcpy(currentConfig.systemName, "Custom Controller");
    currentConfig.debugMode = true;
    currentConfig.transmissionInterval = 30;
    
    // Configurar NRF24L01
    NRF24Config::configureNRF24(currentConfig, 9, 10, 65, POWER_HIGH);
    
    // Configurar controles específicos
    NRF24Config::configureJoystick(currentConfig, 0, A0, A1, 2, "MainControl");
    NRF24Config::setJoystickCalibration(currentConfig, 0, 
                                        150, 3945, 2048,   // X calibrado
                                        150, 3945, 2048,   // Y calibrado  
                                        35);               // Dead zone pequeña
    
    // Configurar palanca con configuración específica
    NRF24Config::configureLever(currentConfig, 0, ANALOG_LEVER, A2, 255, 255, "Precision");
    NRF24Config::setLeverCalibration(currentConfig, 0, 200, 3895, 200, 25);
    
    // Aplicar configuración
    controller.applySystemConfig(currentConfig);
    
    Serial.println("✅ Configuración personalizada creada:");
    Serial.println("   - Joystick de alta precisión");
    Serial.println("   - Palanca calibrada manualmente");
    Serial.println("   - Configuración optimizada para precisión");
    Serial.println();
}

void demonstrateConfigurationEditing() {
    Serial.println(">>> Demostrando edición de configuración...");
    
    // Empezar con configuración básica
    SystemConfig baseConfig = NRF24Config::loadDefault();
    
    Serial.println("Configuración base cargada");
    Serial.print("Joysticks activos: ");
    
    uint8_t activeJoys = 0;
    for (uint8_t i = 0; i < MAX_JOYSTICKS; i++) {
        if (baseConfig.joysticks[i].enabled) activeJoys++;
    }
    Serial.println(activeJoys);
    
    // Modificar configuración programáticamente
    Serial.println("Modificando configuración...");
    
    // Agregar segundo joystick
    NRF24Config::configureJoystick(baseConfig, 1, A2, A3, 3, "SecondStick");
    NRF24Config::setJoystickCalibration(baseConfig, 1, 
                                        100, 3995, 2048,
                                        100, 3995, 2048, 
                                        60);
    
    // Agregar palanca encoder
    NRF24Config::configureLever(baseConfig, 1, ROTARY_ENCODER, 4, 5, 6, "RotaryControl");
    
    // Cambiar configuración NRF
    NRF24Config::configureNRF24(baseConfig, 9, 10, 90, POWER_MAX);
    
    // Aplicar configuración modificada
    controller.applySystemConfig(baseConfig);
    currentConfig = baseConfig;
    
    Serial.println("✅ Configuración modificada:");
    Serial.println("   - Segundo joystick agregado");
    Serial.println("   - Control rotary encoder agregado");
    Serial.println("   - Potencia NRF aumentada");
    Serial.println("   - Canal cambiado a 90");
    Serial.println();
}

void saveAndLoadConfigurationExample() {
    Serial.println(">>> Ejemplo completo de guardar/cargar...");
    
    // 1. Crear configuración especial
    SystemConfig specialConfig = controller.getDefaultConfig();
    strcpy(specialConfig.systemName, "SaveLoad Test");
    specialConfig.nrfChannel = 123;
    specialConfig.transmissionInterval = 75;
    
    // Configurar controles únicos
    NRF24Config::configureJoystick(specialConfig, 0, A0, A1, 2, "TestStick");
    specialConfig.joysticks[0].deadZone = 25;
    specialConfig.joysticks[0].smoothingFactor = 0.35;
    
    controller.applySystemConfig(specialConfig);
    Serial.println("Configuración especial aplicada");
    
    // 2. Guardar en EEPROM
    Serial.println("Guardando en EEPROM slot 2...");
    if (controller.saveSystemConfig(2)) {
        Serial.println("✅ Guardado exitoso");
    } else {
        Serial.println("❌ Error al guardar");
        return;
    }
    
    // 3. Cambiar a configuración diferente
    Serial.println("Cambiando a configuración diferente...");
    currentConfig = NRF24Config::loadDroneConfig();
    controller.applySystemConfig(currentConfig);
    Serial.print("Configuración actual: ");
    Serial.println(currentConfig.systemName);
    
    // 4. Cargar configuración guardada
    Serial.println("Cargando configuración guardada desde slot 2...");
    if (controller.loadSystemConfig(2)) {
        currentConfig = controller.getCurrentSystemConfig();
        Serial.println("✅ Cargado exitoso");
        Serial.print("Configuración restaurada: ");
        Serial.println(currentConfig.systemName);
        Serial.print("Canal: ");
        Serial.println(currentConfig.nrfChannel);
        Serial.print("Intervalo: ");
        Serial.print(currentConfig.transmissionInterval);
        Serial.println("ms");
    } else {
        Serial.println("❌ Error al cargar");
    }
    Serial.println();
}

void showCurrentStatus() {
    Serial.println(">>> Estado actual del sistema:");
    
    // Mostrar configuración completa
    NRF24Config::printConfig(currentConfig);
    
    // Mostrar estadísticas de transmisión
    Serial.println("Estadísticas de transmisión:");
    Serial.print("Último envío: "); 
    Serial.print(millis() % 10000);
    Serial.println("ms");
    
    // Mostrar valores actuales de controles
    Serial.println("Valores actuales:");
    for (uint8_t i = 0; i < MAX_JOYSTICKS; i++) {
        if (currentConfig.joysticks[i].enabled) {
            int x = analogRead(currentConfig.joysticks[i].pinX);
            int y = analogRead(currentConfig.joysticks[i].pinY);
            Serial.print("  ");
            Serial.print(currentConfig.joysticks[i].name);
            Serial.print(": X=");
            Serial.print(x);
            Serial.print(", Y=");
            Serial.println(y);
        }
    }
    Serial.println();
}

void showHelp() {
    Serial.println(">>> Ayuda - Comandos disponibles:");
    Serial.println("1 - Cargar configuración de simulador de vuelo");
    Serial.println("2 - Cargar configuración de robot explorador");
    Serial.println("3 - Crear configuración personalizada desde cero");
    Serial.println("4 - Demostrar edición de configuración");
    Serial.println("5 - Ejemplo completo de guardar/cargar EEPROM");
    Serial.println("s - Mostrar estado actual del sistema");
    Serial.println("h - Mostrar esta ayuda");
    Serial.println();
    Serial.println("Características del sistema:");
    Serial.println("✓ Parsing inteligente de configuraciones");
    Serial.println("✓ Validación automática");
    Serial.println("✓ Configuraciones predefinidas");
    Serial.println("✓ Edición programática");
    Serial.println("✓ Almacenamiento EEPROM");
    Serial.println("✓ Calibración automática");
    Serial.println();
}