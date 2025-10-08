/**
 * ConfigStorage Library - Simple Configuration Storage
 * 
 * Librería simple para guardar configuraciones con Preferences.h
 * 
 * Características:
 * - 4 perfiles de configuración (0-3)
 * - Cada perfil tiene: 14 valores uint8_t + 1 valor uint64_t
 * - Selector de perfil activo
 * - Funciones súper simples
 * 
 * Autor: GitHub Copilot
 * Fecha: 2025
 */

#ifndef CONFIG_STORAGE_H
#define CONFIG_STORAGE_H

#include <Arduino.h>
#include <Preferences.h>

// Configuración de la librería
#define MAX_PROFILES 4          // Número de perfiles (0-3)
#define CONFIG_VALUES_COUNT 15  // Número de valores uint8_t por perfil (ahora 15 para incluir intensidad)

// Estructura para un perfil de configuración
struct ConfigProfile {
    uint8_t values[CONFIG_VALUES_COUNT];  // 14 valores de 0-255
    uint64_t address;                     // 1 valor tipo dirección NRF24L01
};

// Clase principal de almacenamiento
class ConfigStorage {
private:
    Preferences preferences;
    uint8_t activeProfile;
    ConfigProfile currentConfig;
    
    // Claves para Preferences (nombres cortos para ahorrar espacio)
    String getProfileKey(uint8_t profile);
    String getAddressKey(uint8_t profile);
    
public:
    // Constructor
    ConfigStorage();
    
    // ========== FUNCIONES BÁSICAS ==========
    bool begin();                           // Inicializar librería
    void end();                            // Cerrar librería
    
    // ========== GESTIÓN DE PERFILES ==========
    bool setActiveProfile(uint8_t profile); // Cambiar perfil activo (0-3)
    uint8_t getActiveProfile();             // Obtener perfil activo
    
    // ========== GUARDAR/CARGAR CONFIGURACIÓN ==========
    bool saveCurrentConfig();               // Guardar config actual al perfil activo
    bool loadCurrentConfig();               // Cargar config desde perfil activo
    bool saveConfigToProfile(uint8_t profile); // Guardar config actual a perfil específico
    bool loadConfigFromProfile(uint8_t profile); // Cargar config desde perfil específico
    
    // ========== ACCESO A DATOS ==========
    // Valores uint8_t individuales
    void setValue(uint8_t index, uint8_t value);     // Cambiar valor (0-13)
    uint8_t getValue(uint8_t index);                 // Leer valor (0-13)
    
    // Dirección uint64_t
    void setAddress(uint64_t address);               // Cambiar dirección
    uint64_t getAddress();                           // Leer dirección
    
    // Configuración completa
    void setConfig(const ConfigProfile& config);     // Establecer config completa
    ConfigProfile getConfig();                       // Obtener config completa
    
    // ========== FUNCIONES DE UTILIDAD ==========
    void resetProfile(uint8_t profile);             // Resetear perfil a valores por defecto
    void resetCurrentConfig();                       // Resetear configuración actual                     // Resetear todos los perfiles
    
    bool isProfileEmpty(uint8_t profile);           // Verificar si perfil está vacío
    void printCurrentConfig();                       // Debug: imprimir config actual
    void printProfile(uint8_t profile);              // Debug: imprimir perfil específico
    
    // ========== FUNCIONES RÁPIDAS ==========
    // Para uso súper rápido sin cambiar perfil activo
    bool quickSave(uint8_t profile, uint8_t values[CONFIG_VALUES_COUNT], uint64_t address);
    bool quickLoad(uint8_t profile, uint8_t values[CONFIG_VALUES_COUNT], uint64_t* address);
    
    // ========== FUNCIONES ESPECÍFICAS PARA DATOS ORGANIZADOS ==========
     
    // LÍMITES DE VELOCIDAD (índices 0, 1, 2)
    void setSpeedLimits(uint8_t limit1, uint8_t limit2, uint8_t limit3);
    void setSpeedLimit(uint8_t index, uint8_t value);  // index: 0, 1, 2
    uint8_t getSpeedLimit(uint8_t index);              // index: 0, 1, 2
    void getSpeedLimits(uint8_t* limit1, uint8_t* limit2, uint8_t* limit3);
    
    // LÍMITES DE GIRO (índices 3, 4, 5)
    void setTurnLimits(uint8_t limit1, uint8_t limit2, uint8_t limit3);
    void setTurnLimit(uint8_t index, uint8_t value);   // index: 0, 1, 2 (se mapean a 3, 4, 5)
    uint8_t getTurnLimit(uint8_t index);               // index: 0, 1, 2 (se mapean a 3, 4, 5)
    void getTurnLimits(uint8_t* limit1, uint8_t* limit2, uint8_t* limit3);
    
    // LÍMITES DE BOOST (índices 6, 7, 8)
    void setBoostLimits(uint8_t limit1, uint8_t limit2, uint8_t limit3);
    void setBoostLimit(uint8_t index, uint8_t value);  // index: 0, 1, 2 (se mapean a 6, 7, 8)
    uint8_t getBoostLimit(uint8_t index);              // index: 0, 1, 2 (se mapean a 6, 7, 8)
    void getBoostLimits(uint8_t* limit1, uint8_t* limit2, uint8_t* limit3);
    
    // LÍMITES ADICIONALES (índices 9, 10, 11)
    void setExtraLimits(uint8_t limit1, uint8_t limit2, uint8_t limit3);
    void setExtraLimit(uint8_t index, uint8_t value);  // index: 0, 1, 2 (se mapean a 9, 10, 11)
    uint8_t getExtraLimit(uint8_t index);              // index: 0, 1, 2 (se mapean a 9, 10, 11)
    void getExtraLimits(uint8_t* limit1, uint8_t* limit2, uint8_t* limit3);
    
    // LÍMITE DE BRILLO (índice 12)
    void setBrightnessLimit(uint8_t brightness);
    uint8_t getBrightnessLimit();
    
    // CONFIGURACIÓN ADICIONAL (índice 13) 
    void setExtraConfig(uint8_t config);
    uint8_t getExtraConfig();
    
    // CONFIGURACIÓN DE INTENSIDAD (índice 14) - Nuevo
    void setIntensity(uint8_t intensity);     // Valores de 1-4
    uint8_t getIntensityLimit();              // Obtener intensidad actual
    
    // DIRECCIÓN NRF24L01
    void setNRFAddress(uint64_t address);
    uint64_t getNRFAddress();
    
    // INFORMACIÓN DE CONFIGURACIÓN ACTIVA
    uint8_t getActiveProfileNumber();
    String getActiveProfileName();
    void printActiveConfig();
    
    // FUNCIONES DE CARGA Y VERIFICACIÓN
    bool isConfigurationLoaded();
    void reloadActiveConfig();
    
    // FUNCIONES DE MANTENIMIENTO
    void clearAllProfiles();                     // Limpiar todos los perfiles (usar con cuidado)
    bool repairProfile(uint8_t profile);        // Reparar un perfil corrupto
};

#endif // CONFIG_STORAGE_H