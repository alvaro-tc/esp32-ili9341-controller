/**
 * Configuración Específica para Auto RC - Proyecto Beta01
 * 
 * Este archivo contiene la configuración personalizada para tu controlador
 * de auto RC usando los pines específicos de tu hardware.
 * 
 * Controles:
 * - Joystick Izquierdo: Velocidad (Y) y Dirección fina (X)
 * - Joystick Derecho: Giro principal (X) y funciones extra (Y)
 * - Palancas: Modos de conducción y funciones especiales
 */

#ifndef RC_CAR_CONFIG_H
#define RC_CAR_CONFIG_H

#include "NRF24Controller.h"
#include "NRF24Config.h"

// ========== DEFINICIÓN DE PINES (TU HARDWARE) ==========
// NRF24L01
#define NRF24_CE 6
#define NRF24_CSN 7  // Nota: era NRF24_CNS, cambié a CSN

// Joysticks
#define JOYSTICK_LEFT_X 2    // Dirección fina
#define JOYSTICK_LEFT_Y 5    // Velocidad principal
#define JOYSTICK_LEFT_BTN 4  // Botón modo turbo

#define JOYSTICK_RIGHT_X 9   // Giro principal
#define JOYSTICK_RIGHT_Y 8   // Funciones extra
#define JOYSTICK_RIGHT_BTN 10 // Botón freno de emergencia

// Palancas digitales (para modos)
#define LEVER_MODE_1_R 16    // Modo Normal/Sport
#define LEVER_MODE_1_L 17
#define LEVER_MODE_2_R 39    // Modo Reversa/Parking
#define LEVER_MODE_2_L 1

// ========== ESTRUCTURA DE DATOS PARA AUTO RC ==========
struct RCCarData {
    // Valores principales (-255 a +255)
    int16_t velocidad;     // -255 (máxima reversa) a +255 (máxima adelante)
    int16_t giro;          // -255 (máximo izquierda) a +255 (máximo derecha)
    
    // Estados adicionales (0 o 1)
    uint8_t turbo_activo;      // 0 = normal, 1 = turbo
    uint8_t freno_emergencia;  // 0 = normal, 1 = freno
    uint8_t modo_conduccion;   // 0 = normal, 1 = sport, 2 = eco
    uint8_t luces_activas;     // 0 = apagadas, 1 = encendidas
    
    // Información de estado
    uint8_t bateria_nivel;     // 0-100%
    uint32_t timestamp;        // Para verificar conexión
};

// ========== CONFIGURACIÓN PREDEFINIDA PARA AUTO RC ==========
extern const char* RC_CAR_CONFIG_STRING;

// ========== CLASE PARA MANEJO DE AUTO RC ==========
class RCCarController {
private:
    NRF24Controller* controller;
    RCCarData currentData;
    SystemConfig config;
    
    // Filtros para suavizado
    float velocidad_filtrada = 0.0;
    float giro_filtrado = 0.0;
    
public:
    RCCarController();
    ~RCCarController();
    
    // Configuración inicial
    bool begin();
    bool loadConfiguration();
    void printConfiguration();
    
    // Control principal
    void update();
    void readControls();
    void processCarLogic();
    bool transmitData();
    
    // Funciones de conversión
    int16_t mapVelocidad(int joystickValue, bool invertido = true);
    int16_t mapGiro(int joystickValue, bool invertido = false);
    uint8_t readModoConduccion();
    
    // Getters para datos actuales
    RCCarData getCurrentData() { return currentData; }
    int16_t getVelocidad() { return currentData.velocidad; }
    int16_t getGiro() { return currentData.giro; }
    
    // Utilidades para debugging
    void printControlValues();
    void printCarData();
    String getStatusString();
};

#endif // RC_CAR_CONFIG_H