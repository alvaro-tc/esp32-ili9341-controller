/**
 * Ejemplo Completo: Auto RC con Pantalla TFT - Proyecto Beta01
 * 
 * Este ejemplo demuestra el sistema completo de auto RC:
 * - Control con joysticks (velocidad y giro)
 * - Valores simples: -255 a +255 para velocidad y giro
 * - Transmisión NRF24L01 con tus pines específicos
 * - Visualización en pantalla TFT con LVGL
 * 
 * CONTROLES:
 * Joystick Izquierdo:
 *   - Eje Y: Velocidad (-255 reversa máxima, +255 adelante máxima)
 *   - Botón: Turbo (aumenta potencia máxima)
 * 
 * Joystick Derecho:
 *   - Eje X: Giro (-255 izquierda máxima, +255 derecha máxima)
 *   - Botón: Freno de emergencia (para todo inmediatamente)
 * 
 * Palancas:
 *   - Palanca 1: Modo de conducción (Normal/Sport/Eco)
 *   - Palanca 2: Luces (ON/OFF)
 */

#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include "RCCarController.h"

// Configuración de hardware
TFT_eSPI tft = TFT_eSPI();
RCCarController carController;

// Variables de control
unsigned long lastUpdate = 0;
unsigned long lastDisplay = 0;
unsigned long lastSerial = 0;

void setup() {
    Serial.begin(115200);
    while (!Serial) delay(10);
    
    Serial.println("=====================================");
    Serial.println("    AUTO RC BETA01 - SISTEMA COMPLETO");
    Serial.println("=====================================");
    Serial.println();
    
    // Inicializar pantalla
    Serial.println("🖥️  Inicializando pantalla TFT...");
    pinMode(38, OUTPUT);  // TFT_LED
    digitalWrite(38, HIGH);
    
    tft.init();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);
    
    // Mostrar pantalla de bienvenida
    showWelcomeMessage();
    
    // Inicializar sistema de auto RC
    Serial.println("🚗 Inicializando sistema Auto RC...");
    if (carController.begin()) {
        Serial.println("✅ Sistema Auto RC inicializado correctamente");
        showReadyMessage();
    } else {
        Serial.println("❌ Error al inicializar sistema Auto RC");
        showErrorMessage();
        while (1) delay(1000); // Parar aquí si hay error
    }
    
    Serial.println();
    Serial.println("🎮 CONTROLES:");
    Serial.println("   Joystick Izq (pines 2,5,4):");
    Serial.println("     - Eje Y: Velocidad (-255 a +255)");
    Serial.println("     - Botón: Turbo");
    Serial.println("   Joystick Der (pines 9,8,10):");
    Serial.println("     - Eje X: Giro (-255 a +255)");
    Serial.println("     - Botón: Freno emergencia");
    Serial.println();
    Serial.println("📡 NRF24L01: Canal 85, Pines CE=6, CSN=7");
    Serial.println("🔄 Iniciando transmisión...");
    Serial.println();
    
    delay(2000);
    tft.fillScreen(TFT_BLACK);
}

void loop() {
    // === ACTUALIZAR CONTROLADOR (cada 30ms) ===
    if (millis() - lastUpdate >= 30) {
        carController.update();
        lastUpdate = millis();
    }
    
    // === ACTUALIZAR PANTALLA (cada 100ms) ===
    if (millis() - lastDisplay >= 100) {
        updateDisplay();
        lastDisplay = millis();
    }
    
    // === DEBUG POR SERIAL (cada 500ms) ===
    if (millis() - lastSerial >= 500) {
        printDebugInfo();
        lastSerial = millis();
    }
}

// ========== FUNCIONES DE PANTALLA ==========

void showWelcomeMessage() {
    tft.setTextSize(2);
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.setCursor(60, 40);
    tft.println("AUTO RC");
    
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setCursor(80, 80);
    tft.println("BETA01");
    
    tft.setCursor(20, 120);
    tft.println("Inicializando sistema...");
    
    // Barra de progreso
    for (int i = 0; i < 200; i += 10) {
        tft.fillRect(60 + i, 160, 8, 10, TFT_CYAN);
        delay(100);
    }
}

void showReadyMessage() {
    tft.fillRect(0, 180, 320, 60, TFT_GREEN);
    tft.setTextColor(TFT_BLACK, TFT_GREEN);
    tft.setTextSize(2);
    tft.setCursor(80, 200);
    tft.println("LISTO!");
    delay(1000);
}

void showErrorMessage() {
    tft.fillScreen(TFT_RED);
    tft.setTextColor(TFT_WHITE, TFT_RED);
    tft.setTextSize(2);
    tft.setCursor(100, 100);
    tft.println("ERROR");
    
    tft.setTextSize(1);
    tft.setCursor(60, 130);
    tft.println("Revisar conexiones NRF24L01");
}

void updateDisplay() {
    RCCarData data = carController.getCurrentData();
    
    // Limpiar pantalla
    tft.fillScreen(TFT_BLACK);
    
    // === TÍTULO ===
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.setTextSize(2);
    tft.setCursor(80, 5);
    tft.println("AUTO RC");
    
    // === VELOCIDAD (Principal) ===
    tft.setTextSize(3);
    tft.setCursor(10, 35);
    
    if (data.velocidad > 10) {
        tft.setTextColor(TFT_GREEN, TFT_BLACK);
        tft.print("VEL: +");
        tft.print(data.velocidad);
        tft.setTextSize(1);
        tft.setCursor(200, 50);
        tft.print("ADELANTE");
    } else if (data.velocidad < -10) {
        tft.setTextColor(TFT_RED, TFT_BLACK);
        tft.print("VEL: ");
        tft.print(data.velocidad);
        tft.setTextSize(1);
        tft.setCursor(200, 50);
        tft.print("REVERSA");
    } else {
        tft.setTextColor(TFT_YELLOW, TFT_BLACK);
        tft.print("VEL: 0");
        tft.setTextSize(1);
        tft.setCursor(200, 50);
        tft.print("PARADO");
    }
    
    // === GIRO (Principal) ===
    tft.setTextSize(3);
    tft.setCursor(10, 75);
    
    if (data.giro > 10) {
        tft.setTextColor(TFT_BLUE, TFT_BLACK);
        tft.print("GIR: +");
        tft.print(data.giro);
        tft.setTextSize(1);
        tft.setCursor(200, 90);
        tft.print("DERECHA");
    } else if (data.giro < -10) {
        tft.setTextColor(TFT_MAGENTA, TFT_BLACK);
        tft.print("GIR: ");
        tft.print(data.giro);
        tft.setTextSize(1);
        tft.setCursor(200, 90);
        tft.print("IZQUIERDA");
    } else {
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.print("GIR: 0");
        tft.setTextSize(1);
        tft.setCursor(200, 90);
        tft.print("CENTRO");
    }
    
    // === INFORMACIÓN DE ESTADO ===
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    
    // Línea 1: Modo y estados
    tft.setCursor(10, 120);
    tft.print("Modo: ");
    switch (data.modo_conduccion) {
        case 0: 
            tft.setTextColor(TFT_WHITE, TFT_BLACK);
            tft.print("Normal");
            break;
        case 1: 
            tft.setTextColor(TFT_ORANGE, TFT_BLACK);
            tft.print("Sport");
            break;
        case 2: 
            tft.setTextColor(TFT_GREEN, TFT_BLACK);
            tft.print("Eco");
            break;
    }
    
    // Estados especiales
    tft.setCursor(10, 140);
    if (data.turbo_activo) {
        tft.setTextColor(TFT_YELLOW, TFT_BLACK);
        tft.print("[TURBO] ");
    }
    if (data.freno_emergencia) {
        tft.setTextColor(TFT_RED, TFT_BLACK);
        tft.print("[FRENO] ");
    }
    if (data.luces_activas) {
        tft.setTextColor(TFT_CYAN, TFT_BLACK);
        tft.print("[LUCES] ");
    }
    
    // Batería y conexión
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setCursor(10, 160);
    tft.print("Bateria: ");
    if (data.bateria_nivel > 50) {
        tft.setTextColor(TFT_GREEN, TFT_BLACK);
    } else if (data.bateria_nivel > 20) {
        tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    } else {
        tft.setTextColor(TFT_RED, TFT_BLACK);
    }
    tft.print(data.bateria_nivel);
    tft.print("%");
    
    // Indicador de transmisión
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.setCursor(150, 160);
    tft.print("TX: ");
    tft.print((millis() / 1000) % 60);
    tft.print("s");
    
    // === INDICADORES VISUALES ===
    drawCarIndicator(data.velocidad, data.giro);
}

void drawCarIndicator(int16_t velocidad, int16_t giro) {
    // Dibujar indicador visual del auto en la esquina
    int centerX = 280;
    int centerY = 180;
    
    // Auto (rectángulo)
    tft.fillRect(centerX-15, centerY-8, 30, 16, TFT_WHITE);
    
    // Dirección de velocidad
    if (abs(velocidad) > 20) {
        if (velocidad > 0) {
            // Flecha adelante
            tft.fillTriangle(centerX, centerY-20, centerX-8, centerY-12, centerX+8, centerY-12, TFT_GREEN);
        } else {
            // Flecha atrás
            tft.fillTriangle(centerX, centerY+20, centerX-8, centerY+12, centerX+8, centerY+12, TFT_RED);
        }
    }
    
    // Dirección de giro
    if (abs(giro) > 20) {
        if (giro > 0) {
            // Flecha derecha
            tft.fillTriangle(centerX+25, centerY, centerX+17, centerY-8, centerX+17, centerY+8, TFT_BLUE);
        } else {
            // Flecha izquierda
            tft.fillTriangle(centerX-25, centerY, centerX-17, centerY-8, centerX-17, centerY+8, TFT_MAGENTA);
        }
    }
}

// ========== FUNCIONES DE DEBUG ==========

void printDebugInfo() {
    RCCarData data = carController.getCurrentData();
    
    // Header cada 10 impresiones
    static int contador = 0;
    if (contador % 10 == 0) {
        Serial.println();
        Serial.println("=== ESTADO AUTO RC ===");
        Serial.println("Tiempo | Vel  | Gir  | Modo | Turbo | Freno | Luces | Bat%");
        Serial.println("-------|------|------|------|-------|-------|-------|-----");
    }
    contador++;
    
    // Línea de datos
    Serial.printf("%6lu | %4d | %4d |", millis()/1000, data.velocidad, data.giro);
    
    // Modo
    switch (data.modo_conduccion) {
        case 0: Serial.print("  N  |"); break;
        case 1: Serial.print("  S  |"); break;
        case 2: Serial.print("  E  |"); break;
    }
    
    // Estados
    Serial.printf("   %d   |   %d   |   %d   | %3d%%", 
                  data.turbo_activo, 
                  data.freno_emergencia, 
                  data.luces_activas,
                  data.bateria_nivel);
    
    Serial.println();
}

// ========== FUNCIÓN DE AYUDA PARA DEBUGGING ==========
void printFullStatus() {
    Serial.println();
    Serial.println("==========================================");
    Serial.println("         ESTADO COMPLETO DEL SISTEMA");
    Serial.println("==========================================");
    
    carController.printCarData();
    
    Serial.println("📊 Estadísticas:");
    Serial.printf("   Tiempo ejecutando: %lu segundos\n", millis()/1000);
    Serial.printf("   Memoria libre: %d bytes\n", ESP.getFreeHeap());
    
    Serial.println("🔧 Configuración de pines:");
    Serial.println("   NRF24L01: CE=6, CSN=7");
    Serial.println("   Joystick Izq: X=2, Y=5, BTN=4");
    Serial.println("   Joystick Der: X=9, Y=8, BTN=10");
    Serial.println("   Palancas: 16,17,39,1");
    
    Serial.println("==========================================");
    Serial.println();
}