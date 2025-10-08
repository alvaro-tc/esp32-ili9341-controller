/**
 * Ejemplo Avanzado de ConfigStorage - Funciones Específicas
 * 
 * Este ejemplo muestra cómo usar las funciones organizadas
 * para manejar límites de velocidad, giro, boost, etc.
 */

#include <Arduino.h>
#include "ConfigStorage.h"

ConfigStorage config;

void setup() {
    Serial.begin(115200);
    delay(2000);
    
    Serial.println("=== ConfigStorage - Funciones Específicas ===");
    
    // Inicializar la librería
    if (!config.begin()) {
        Serial.println("❌ Error al inicializar ConfigStorage");
        return;
    }
    
    // ========== CONFIGURAR LÍMITES DE VELOCIDAD ==========
    Serial.println("\n--- Configurando Límites de Velocidad ---");
    
    // Opción 1: Configurar todos los límites a la vez
    config.setSpeedLimits(50, 150, 255);  // Lento, Medio, Rápido
    
    // Opción 2: Configurar límites individuales
    // config.setSpeedLimit(0, 50);   // Velocidad baja
    // config.setSpeedLimit(1, 150);  // Velocidad media
    // config.setSpeedLimit(2, 255);  // Velocidad alta
    
    // Leer límites de velocidad
    uint8_t vel1, vel2, vel3;
    config.getSpeedLimits(&vel1, &vel2, &vel3);
    Serial.print("Límites leídos: "); 
    Serial.print(vel1); Serial.print(", ");
    Serial.print(vel2); Serial.print(", ");
    Serial.println(vel3);
    
    // ========== CONFIGURAR LÍMITES DE GIRO ==========
    Serial.println("\n--- Configurando Límites de Giro ---");
    
    config.setTurnLimits(30, 128, 200);  // Giro suave, normal, agresivo
    
    // Leer límite individual
    Serial.print("Giro suave: "); Serial.println(config.getTurnLimit(0));
    Serial.print("Giro normal: "); Serial.println(config.getTurnLimit(1));
    Serial.print("Giro agresivo: "); Serial.println(config.getTurnLimit(2));
    
    // ========== CONFIGURAR LÍMITES DE BOOST ==========
    Serial.println("\n--- Configurando Límites de Boost ---");
    
    config.setBoostLimits(80, 160, 255);  // Boost bajo, medio, máximo
    
    // ========== CONFIGURAR LÍMITES ADICIONALES ==========
    Serial.println("\n--- Configurando Límites Adicionales ---");
    
    config.setExtraLimits(100, 120, 140);  // Configuraciones custom
    
    // ========== CONFIGURAR BRILLO Y EXTRAS ==========
    Serial.println("\n--- Configurando Brillo y Extras ---");
    
    config.setBrightnessLimit(200);  // Brillo de pantalla
    config.setExtraConfig(42);       // Configuración adicional
    
    // ========== CONFIGURAR DIRECCIÓN NRF24L01 ==========
    Serial.println("\n--- Configurando Dirección NRF24L01 ---");
    
    config.setNRFAddress(0x1234567890LL);  // Dirección del control
    
    // ========== MOSTRAR CONFIGURACIÓN COMPLETA ==========
    Serial.println("\n--- Configuración Actual Completa ---");
    config.printActiveConfig();
    
    // ========== GUARDAR CONFIGURACIÓN ==========
    Serial.println("\n--- Guardando Configuración ---");
    config.saveCurrentConfig();
    
    // ========== PROBAR CAMBIO DE PERFILES ==========
    Serial.println("\n--- Probando Cambio de Perfiles ---");
    
    // Crear configuración diferente para perfil 1
    config.setActiveProfile(1);
    config.setSpeedLimits(100, 180, 255);  // Más velocidad
    config.setTurnLimits(50, 128, 180);    // Menos giro
    config.setBoostLimits(120, 200, 255);  // Más boost
    config.setBrightnessLimit(150);
    config.setNRFAddress(0xABCDEF1234LL);
    
    config.printActiveConfig();
    config.saveCurrentConfig();
    
    // ========== COMPARAR PERFILES ==========
    Serial.println("\n--- Comparando Perfiles ---");
    
    // Volver al perfil 0
    config.setActiveProfile(0);
    Serial.println("Perfil 0:");
    config.printActiveConfig();
    
    // Cambiar al perfil 1
    config.setActiveProfile(1);
    Serial.println("Perfil 1:");
    config.printActiveConfig();
    
    Serial.println("\n=== Configuración Completa ===");
}

void loop() {
    static unsigned long lastUpdate = 0;
    static uint8_t counter = 0;
    
    // Cada 5 segundos, mostrar info del perfil activo
    if (millis() - lastUpdate > 5000) {
        lastUpdate = millis();
        
        Serial.println("\n--- Estado Actual ---");
        Serial.print("Perfil activo: "); Serial.println(config.getActiveProfileName());
        Serial.print("¿Configuración cargada? "); 
        Serial.println(config.isConfigurationLoaded() ? "SÍ" : "NO");
        
        // Mostrar algunos valores importantes
        Serial.print("Velocidad actual: [");
        Serial.print(config.getSpeedLimit(0)); Serial.print(", ");
        Serial.print(config.getSpeedLimit(1)); Serial.print(", ");
        Serial.print(config.getSpeedLimit(2)); Serial.println("]");
        
        Serial.print("Brillo: "); Serial.println(config.getBrightnessLimit());
        Serial.print("NRF Addr: 0x"); 
        Serial.println((unsigned long)config.getNRFAddress(), HEX);
        
        // Cambiar algún valor para demostrar persistencia
        counter++;
        config.setExtraConfig(counter % 256);
        
        if (counter % 10 == 0) {
            // Cada 50 segundos, cambiar de perfil
            uint8_t newProfile = (config.getActiveProfileNumber() + 1) % 2; // Alternar 0-1
            Serial.print("🔄 Cambiando a perfil "); Serial.println(newProfile);
            config.setActiveProfile(newProfile);
        }
    }
    
    delay(100);
}

// ========== FUNCIONES HELPER PARA TU PROYECTO ==========

// Ejemplo de función para configurar perfiles preestablecidos
void setupRCProfiles() {
    // Perfil 0: Principiante
    config.setActiveProfile(0);
    config.setSpeedLimits(50, 100, 150);    // Velocidades bajas
    config.setTurnLimits(30, 80, 120);      // Giros suaves
    config.setBoostLimits(60, 100, 140);    // Boost limitado
    config.setAddress(0x1A2B3C4D5ELL); // Dirección NRF24
    config.setBrightnessLimit(180);
    config.saveCurrentConfig();
    
    // Perfil 1: Intermedio
    config.setActiveProfile(1);
    config.setSpeedLimits(80, 160, 220);    // Más velocidad
    config.setTurnLimits(50, 128, 180);     // Giros normales
    config.setBoostLimits(100, 180, 240);   // Más boost
    config.setBrightnessLimit(200);
    config.saveCurrentConfig();
    
    // Perfil 2: Experto
    config.setActiveProfile(2);
    config.setSpeedLimits(120, 200, 255);   // Máxima velocidad
    config.setTurnLimits(80, 160, 255);     // Giros agresivos
    config.setBoostLimits(150, 220, 255);   // Boost máximo
    config.setBrightnessLimit(255);
    config.saveCurrentConfig();
    
    // Perfil 3: Custom/Test
    config.setActiveProfile(3);
    config.setSpeedLimits(100, 150, 200);   
    config.setTurnLimits(60, 120, 180);     
    config.setBoostLimits(80, 140, 200);    
    config.setBrightnessLimit(220);
    config.saveCurrentConfig();
    
    config.getActiveProfile(); // Volver al perfil activo original
    Serial.println("✅ Todos los perfiles RC configurados");
}

// Función para leer la configuración actual del RC
void getCurrentRCConfig(uint8_t* speeds, uint8_t* turns, uint8_t* boosts, uint8_t* brightness, uint64_t* nrfAddr) {
    // Leer todos los límites
    config.getSpeedLimits(&speeds[0], &speeds[1], &speeds[2]);
    config.getTurnLimits(&turns[0], &turns[1], &turns[2]);
    config.getBoostLimits(&boosts[0], &boosts[1], &boosts[2]);
    
    *brightness = config.getBrightnessLimit();
    *nrfAddr = config.getNRFAddress();
    
    Serial.println("📖 Configuración RC leída desde perfil activo");
}