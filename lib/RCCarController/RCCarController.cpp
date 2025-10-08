/**
 * Implementación del Controlador de Auto RC - Proyecto Beta01
 * 
 * Sistema simple y fácil de entender para controlar un auto RC.
 * Usa valores de -255 a +255 para velocidad y giro:
 * 
 * VELOCIDAD:
 * -255 = Máxima velocidad hacia atrás (reversa)
 *    0 = Parado
 * +255 = Máxima velocidad hacia adelante
 * 
 * GIRO:
 * -255 = Giro máximo a la izquierda
 *    0 = Derecho
 * +255 = Giro máximo a la derecha
 */

#include "RCCarController.h"

// Definir pines específicos del hardware (deben coincidir con el header)
#ifndef JOYSTICK_LEFT_X
#define JOYSTICK_LEFT_X 2
#endif
#ifndef JOYSTICK_LEFT_Y
#define JOYSTICK_LEFT_Y 5
#endif
#ifndef JOYSTICK_LEFT_BTN
#define JOYSTICK_LEFT_BTN 4
#endif
#ifndef JOYSTICK_RIGHT_X
#define JOYSTICK_RIGHT_X 9
#endif
#ifndef JOYSTICK_RIGHT_Y
#define JOYSTICK_RIGHT_Y 8
#endif
#ifndef JOYSTICK_RIGHT_BTN
#define JOYSTICK_RIGHT_BTN 10
#endif

// ========== CONFIGURACIÓN PREDEFINIDA PARA AUTO RC ==========
const char* RC_CAR_CONFIG_STRING = R"(
# Configuración Auto RC - Proyecto Beta01
SYSTEM_NAME=RC Car Beta01
DEBUG_MODE=false
TRANSMISSION_INTERVAL=30

# NRF24L01 configuración optimizada para auto RC
NRF_CE_PIN=6
NRF_CSN_PIN=7
NRF_CHANNEL=85
NRF_POWER=HIGH
NRF_DATA_RATE=1MBPS
NRF_TX_ADDRESS=0xE8E8F0F0E1
NRF_RX_ADDRESS=0xE8E8F0F0E2

# Joystick Izquierdo (Velocidad y Dirección Fina)
JOY0_ENABLED=true
JOY0_NAME=VelocidadControl
JOY0_PIN_X=2
JOY0_PIN_Y=5
JOY0_PIN_BUTTON=4
JOY0_MIN_X=100
JOY0_MAX_X=3995
JOY0_CENTER_X=2048
JOY0_MIN_Y=100
JOY0_MAX_Y=3995
JOY0_CENTER_Y=2048
JOY0_DEAD_ZONE=80
JOY0_INVERT_X=false
JOY0_INVERT_Y=true
JOY0_SMOOTHING=0.2

# Joystick Derecho (Giro Principal)
JOY1_ENABLED=true
JOY1_NAME=GiroControl
JOY1_PIN_X=9
JOY1_PIN_Y=8
JOY1_PIN_BUTTON=10
JOY1_MIN_X=100
JOY1_MAX_X=3995
JOY1_CENTER_X=2048
JOY1_MIN_Y=100
JOY1_MAX_Y=3995
JOY1_CENTER_Y=2048
JOY1_DEAD_ZONE=60
JOY1_INVERT_X=false
JOY1_INVERT_Y=false
JOY1_SMOOTHING=0.15

# Palanca de Modo 1 (Normal/Sport/Eco)
LEV0_ENABLED=true
LEV0_NAME=ModoConduccion
LEV0_TYPE=DIGITAL
LEV0_PIN_A=16
LEV0_PIN_B=17
LEV0_PIN_BUTTON=255
LEV0_DIGITAL_POSITIONS=3

# Palanca de Modo 2 (Funciones especiales)
LEV1_ENABLED=true
LEV1_NAME=FuncionesExtra
LEV1_TYPE=DIGITAL
LEV1_PIN_A=39
LEV1_PIN_B=1
LEV1_PIN_BUTTON=255
LEV1_DIGITAL_POSITIONS=2
)";

// ========== CONSTRUCTOR Y DESTRUCTOR ==========
RCCarController::RCCarController() {
    controller = nullptr;
    
    // Inicializar datos del auto
    currentData.velocidad = 0;
    currentData.giro = 0;
    currentData.turbo_activo = 0;
    currentData.freno_emergencia = 0;
    currentData.modo_conduccion = 0;
    currentData.luces_activas = 0;
    currentData.bateria_nivel = 100;
    currentData.timestamp = 0;
    
    velocidad_filtrada = 0.0;
    giro_filtrado = 0.0;
}

RCCarController::~RCCarController() {
    if (controller) {
        delete controller;
    }
}

// ========== CONFIGURACIÓN INICIAL ==========
bool RCCarController::begin() {
    Serial.println("=== Inicializando Controlador de Auto RC ===");
    
    // Crear controlador NRF24 con tus pines específicos
    controller = new NRF24Controller(NRF24_CE, NRF24_CSN);
    
    if (!controller) {
        Serial.println("❌ Error: No se pudo crear el controlador NRF24");
        return false;
    }
    
    // Cargar configuración específica para auto RC
    if (!loadConfiguration()) {
        Serial.println("❌ Error: No se pudo cargar la configuración");
        return false;
    }
    
    // Inicializar pines de palancas digitales
    pinMode(LEVER_MODE_1_R, INPUT_PULLUP);
    pinMode(LEVER_MODE_1_L, INPUT_PULLUP);
    pinMode(LEVER_MODE_2_R, INPUT_PULLUP);
    pinMode(LEVER_MODE_2_L, INPUT_PULLUP);
    
    Serial.println("✅ Controlador RC inicializado correctamente");
    Serial.println("📡 Transmitiendo en canal 85");
    Serial.println("🎮 Joystick Izq: Velocidad | Joystick Der: Giro");
    Serial.println();
    
    return true;
}

bool RCCarController::loadConfiguration() {
    // Cargar configuración desde string predefinido
    config = NRF24Config::loadFromString(RC_CAR_CONFIG_STRING);
    
    // Validar configuración
    if (!NRF24Config::validateConfig(config)) {
        Serial.println("❌ Configuración inválida para auto RC");
        return false;
    }
    
    // Aplicar configuración al controlador
    controller->applySystemConfig(config);
    
    Serial.println("✅ Configuración de auto RC cargada");
    return true;
}

void RCCarController::printConfiguration() {
    Serial.println(">>> Configuración actual del Auto RC:");
    NRF24Config::printConfig(config);
}

// ========== CONTROL PRINCIPAL ==========
void RCCarController::update() {
    // 1. Leer todos los controles
    readControls();
    
    // 2. Procesar lógica del auto
    processCarLogic();
    
    // 3. Actualizar timestamp
    currentData.timestamp = millis();
    
    // 4. Transmitir datos
    transmitData();
}

void RCCarController::readControls() {
    // JOYSTICK IZQUIERDO - VELOCIDAD (Eje Y)
    if (config.joysticks[0].enabled) {
        // Leer directamente del pin analógico
        int leftY = analogRead(JOYSTICK_LEFT_Y);
        int16_t velocidad_raw = mapVelocidad(leftY, true); // Invertido para que arriba = adelante
        
        // Aplicar filtro suave
        velocidad_filtrada = velocidad_filtrada * 0.8 + velocidad_raw * 0.2;
        currentData.velocidad = (int16_t)velocidad_filtrada;
        
        // Leer botón de turbo
        currentData.turbo_activo = !digitalRead(JOYSTICK_LEFT_BTN) ? 1 : 0; // Activo en bajo
    }
    
    // JOYSTICK DERECHO - GIRO (Eje X)  
    if (config.joysticks[1].enabled) {
        // Leer directamente del pin analógico
        int rightX = analogRead(JOYSTICK_RIGHT_X);
        int16_t giro_raw = mapGiro(rightX, false); // No invertido
        
        // Aplicar filtro suave
        giro_filtrado = giro_filtrado * 0.7 + giro_raw * 0.3;
        currentData.giro = (int16_t)giro_filtrado;
        
        // Leer botón de freno de emergencia
        currentData.freno_emergencia = !digitalRead(JOYSTICK_RIGHT_BTN) ? 1 : 0; // Activo en bajo
    }
    
    // PALANCAS DIGITALES - MODOS
    currentData.modo_conduccion = readModoConduccion();
    
    // Leer estado de luces (palanca 2)
    bool luces_switch = !digitalRead(LEVER_MODE_2_R); // Activo en bajo
    currentData.luces_activas = luces_switch ? 1 : 0;
}

void RCCarController::processCarLogic() {
    // LÓGICA DE FRENO DE EMERGENCIA
    if (currentData.freno_emergencia) {
        currentData.velocidad = 0; // Parar inmediatamente
        currentData.giro = 0;      // Centrar dirección
    }
    
    // LÓGICA DE TURBO (aumenta velocidad máxima)
    if (currentData.turbo_activo && abs(currentData.velocidad) > 0) {
        // En modo turbo, permitir velocidad hasta ±255
        // Sin turbo, limitar a ±200
        if (!currentData.turbo_activo) {
            if (currentData.velocidad > 200) currentData.velocidad = 200;
            if (currentData.velocidad < -200) currentData.velocidad = -200;
        }
    }
    
    // LÓGICA POR MODO DE CONDUCCIÓN
    switch (currentData.modo_conduccion) {
        case 0: // Modo Normal
            // Sin modificaciones
            break;
            
        case 1: // Modo Sport (más sensible)
            currentData.velocidad = (int16_t)(currentData.velocidad * 1.2);
            currentData.giro = (int16_t)(currentData.giro * 1.1);
            break;
            
        case 2: // Modo Eco (más suave)
            currentData.velocidad = (int16_t)(currentData.velocidad * 0.7);
            currentData.giro = (int16_t)(currentData.giro * 0.8);
            break;
    }
    
    // LIMITAR VALORES FINALES
    if (currentData.velocidad > 255) currentData.velocidad = 255;
    if (currentData.velocidad < -255) currentData.velocidad = -255;
    if (currentData.giro > 255) currentData.giro = 255;
    if (currentData.giro < -255) currentData.giro = -255;
    
    // SIMULAR NIVEL DE BATERÍA (decrece lentamente)
    static unsigned long lastBatteryUpdate = 0;
    if (millis() - lastBatteryUpdate > 10000) { // Cada 10 segundos
        if (currentData.bateria_nivel > 0) {
            currentData.bateria_nivel--;
        }
        lastBatteryUpdate = millis();
    }
}

bool RCCarController::transmitData() {
    // Crear paquete DataPacket para transmitir
    DataPacket packet;
    packet.packetId = 1;
    packet.controlCount = 2; // Velocidad y Giro
    packet.timestamp = currentData.timestamp;
    
    // Control 0: Velocidad
    packet.controls[0].id = 0;
    packet.controls[0].type = CONTROL_JOYSTICK;
    packet.controls[0].valueX = 0; // No usado para velocidad
    packet.controls[0].valueY = currentData.velocidad;
    packet.controls[0].flags = (currentData.turbo_activo << 0) | 
                               (currentData.freno_emergencia << 1) |
                               (currentData.modo_conduccion << 2) |
                               (currentData.luces_activas << 5);
    packet.controls[0].timestamp = currentData.timestamp;
    
    // Control 1: Giro
    packet.controls[1].id = 1;
    packet.controls[1].type = CONTROL_JOYSTICK;
    packet.controls[1].valueX = currentData.giro;
    packet.controls[1].valueY = 0; // No usado para giro
    packet.controls[1].flags = currentData.bateria_nivel;
    packet.controls[1].timestamp = currentData.timestamp;
    
    // Limpiar controles no usados
    for (int i = 2; i < 8; i++) {
        packet.controls[i].id = 255;
        packet.controls[i].type = CONTROL_CUSTOM;
        packet.controls[i].valueX = 0;
        packet.controls[i].valueY = 0;
        packet.controls[i].flags = 0;
        packet.controls[i].timestamp = 0;
    }
    
    // Calcular checksum simple
    packet.checksum = packet.packetId + packet.controlCount + 
                      packet.controls[0].valueY + packet.controls[1].valueX;
    
    // Transmitir usando el controlador NRF24
    return controller->sendCustomPacket(packet);
}

// ========== FUNCIONES DE CONVERSIÓN ==========
int16_t RCCarController::mapVelocidad(int joystickValue, bool invertido) {
    // Convertir valor del joystick (0-4095) a velocidad (-255 a +255)
    int16_t centro = 2048;
    int16_t rango = 2048;
    
    // Calcular valor relativo al centro
    int16_t relativo = joystickValue - centro;
    
    // Mapear a rango -255 a +255
    int16_t velocidad = (relativo * 255) / rango;
    
    // Aplicar inversión si es necesario
    if (invertido) {
        velocidad = -velocidad;
    }
    
    // Limitar valores
    if (velocidad > 255) velocidad = 255;
    if (velocidad < -255) velocidad = -255;
    
    return velocidad;
}

int16_t RCCarController::mapGiro(int joystickValue, bool invertido) {
    // Similar a velocidad pero para giro
    int16_t centro = 2048;
    int16_t rango = 2048;
    
    int16_t relativo = joystickValue - centro;
    int16_t giro = (relativo * 255) / rango;
    
    if (invertido) {
        giro = -giro;
    }
    
    if (giro > 255) giro = 255;
    if (giro < -255) giro = -255;
    
    return giro;
}

uint8_t RCCarController::readModoConduccion() {
    // Leer estado de las palancas de modo
    bool mode1_r = !digitalRead(LEVER_MODE_1_R); // Activo en bajo
    bool mode1_l = !digitalRead(LEVER_MODE_1_L);
    
    // Determinar modo basado en posición de la palanca
    if (!mode1_r && !mode1_l) {
        return 0; // Modo Normal (centro)
    } else if (mode1_r && !mode1_l) {
        return 1; // Modo Sport (derecha)
    } else if (!mode1_r && mode1_l) {
        return 2; // Modo Eco (izquierda)
    } else {
        return 0; // Por defecto, modo normal
    }
}

// ========== UTILIDADES PARA DEBUGGING ==========
void RCCarController::printControlValues() {
    Serial.print("🎮 Controles - Vel: ");
    Serial.print(currentData.velocidad);
    Serial.print(" | Giro: ");
    Serial.print(currentData.giro);
    Serial.print(" | Modo: ");
    
    switch (currentData.modo_conduccion) {
        case 0: Serial.print("Normal"); break;
        case 1: Serial.print("Sport"); break;
        case 2: Serial.print("Eco"); break;
    }
    
    Serial.print(" | Turbo: ");
    Serial.print(currentData.turbo_activo ? "ON" : "OFF");
    Serial.print(" | Freno: ");
    Serial.print(currentData.freno_emergencia ? "ON" : "OFF");
    Serial.print(" | Luces: ");
    Serial.println(currentData.luces_activas ? "ON" : "OFF");
}

void RCCarController::printCarData() {
    Serial.println(">>> Datos completos del Auto RC:");
    Serial.print("Velocidad: "); Serial.print(currentData.velocidad); Serial.println(" (-255 a +255)");
    Serial.print("Giro: "); Serial.print(currentData.giro); Serial.println(" (-255 a +255)");
    Serial.print("Turbo: "); Serial.println(currentData.turbo_activo ? "Activado" : "Desactivado");
    Serial.print("Freno Emergencia: "); Serial.println(currentData.freno_emergencia ? "Activado" : "Desactivado");
    Serial.print("Modo Conducción: ");
    switch (currentData.modo_conduccion) {
        case 0: Serial.println("Normal"); break;
        case 1: Serial.println("Sport"); break;
        case 2: Serial.println("Eco"); break;
    }
    Serial.print("Luces: "); Serial.println(currentData.luces_activas ? "Encendidas" : "Apagadas");
    Serial.print("Batería: "); Serial.print(currentData.bateria_nivel); Serial.println("%");
    Serial.println();
}

String RCCarController::getStatusString() {
    String status = "Vel:";
    status += String(currentData.velocidad);
    status += " Giro:";
    status += String(currentData.giro);
    status += " Bat:";
    status += String(currentData.bateria_nivel);
    status += "%";
    
    if (currentData.turbo_activo) status += " [TURBO]";
    if (currentData.freno_emergencia) status += " [FRENO]";
    if (currentData.luces_activas) status += " [LUCES]";
    
    return status;
}