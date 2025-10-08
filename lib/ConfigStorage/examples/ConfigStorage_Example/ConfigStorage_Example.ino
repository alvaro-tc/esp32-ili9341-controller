/**
 * Ejemplo de uso de ConfigStorage Library
 * 
 * Este ejemplo muestra cómo usar la librería ConfigStorage
 * para guardar y cargar configuraciones de forma súper simple.
 */

#include <Arduino.h>
#include "ConfigStorage.h"

ConfigStorage config;

void setup() {
    Serial.begin(115200);
    delay(2000);
    
    Serial.println("=== ConfigStorage Example ===");
    
    // Inicializar la librería
    if (!config.begin()) {
        Serial.println("❌ Error al inicializar ConfigStorage");
        return;
    }
    
    // Mostrar configuración actual
    config.printCurrentConfig();
    
    // ========== EJEMPLO 1: CONFIGURAR VALORES ==========
    Serial.println("\n--- Configurando valores ---");
    
    // Configurar algunos valores para joysticks y palancas
    config.setValue(0, 100);  // Joystick X min
    config.setValue(1, 150);  // Joystick X max
    config.setValue(2, 80);   // Joystick Y min
    config.setValue(3, 170);  // Joystick Y max
    
    config.setValue(4, 50);   // Palanca 1 pos1
    config.setValue(5, 100);  // Palanca 1 pos2
    config.setValue(6, 150);  // Palanca 1 pos3
    
    config.setValue(7, 200);  // Brillo TFT
    config.setValue(8, 75);   // Volumen
    config.setValue(9, 1);    // Modo invertido
    
    // Configurar dirección NRF24L01
    config.setAddress(0x1234567890LL);
    
    // Mostrar configuración modificada
    config.printCurrentConfig();
    
    // ========== EJEMPLO 2: GUARDAR CONFIGURACIÓN ==========
    Serial.println("\n--- Guardando configuración ---");
    
    // Guardar en el perfil actual (perfil 0)
    config.saveCurrentConfig();
    
    // Guardar también en perfil 1
    config.saveConfigToProfile(1);
    
    // ========== EJEMPLO 3: CAMBIAR DE PERFIL ==========
    Serial.println("\n--- Cambiando de perfil ---");
    
    // Cambiar al perfil 2
    config.setActiveProfile(2);
    
    // Configurar valores diferentes para el perfil 2
    config.setValue(0, 200);  // Joystick X min diferente
    config.setValue(7, 255);  // Brillo máximo
    config.setAddress(0xABCDEF1234LL);
    
    config.printCurrentConfig();
    config.saveCurrentConfig();
    
    // ========== EJEMPLO 4: COMPARAR PERFILES ==========
    Serial.println("\n--- Comparando perfiles ---");
    
    config.printProfile(0);
    config.printProfile(1);
    config.printProfile(2);
    config.printProfile(3);
    
    // ========== EJEMPLO 5: FUNCIONES RÁPIDAS ==========
    Serial.println("\n--- Funciones rápidas ---");
    
    // Crear array de valores para guardar rápido
    uint8_t quickValues[14] = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 110, 120, 130, 140};
    uint64_t quickAddress = 0x9876543210LL;
    
    // Guardar rápido en perfil 3
    config.quickSave(3, quickValues, quickAddress);
    
    // Verificar que se guardó
    config.printProfile(3);
    
    // Cargar rápido desde perfil 3
    uint8_t loadedValues[14];
    uint64_t loadedAddress;
    
    if (config.quickLoad(3, loadedValues, &loadedAddress)) {
        Serial.println("✅ Carga rápida exitosa:");
        Serial.print("Valores cargados: [");
        for (int i = 0; i < 14; i++) {
            Serial.print(loadedValues[i]);
            if (i < 13) Serial.print(", ");
        }
        Serial.println("]");
        Serial.print("Dirección cargada: 0x");
        Serial.println((unsigned long)loadedAddress, HEX);
    }
    
    Serial.println("\n=== Ejemplo completo ===");
}

void loop() {
    // En el loop podríamos hacer actualizaciones periódicas
    static unsigned long lastSave = 0;
    static int counter = 0;
    
    // Cada 10 segundos, actualizar un valor y guardar
    if (millis() - lastSave > 10000) {
        lastSave = millis();
        counter++;
        
        Serial.println("\n--- Actualización automática ---");
        
        // Actualizar algún valor (por ejemplo, un contador)
        config.setValue(13, counter % 256);
        
        // Guardar la configuración
        config.saveCurrentConfig();
        
        Serial.print("Contador actualizado a: ");
        Serial.println(counter % 256);
    }
    
    delay(100);
}