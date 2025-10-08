# Librerías de Control - Joystick, Lever y NRF24Controller

Este proyecto incluye tres librerías flexibles y completas para el control de joysticks, palancas (levers) y comunicación inalámbrica NRF24L01 en Arduino/ESP32, especialmente diseñadas para sistemas de control avanzados.

## 📋 Contenido

- [Librería Joystick](#librería-joystick)
- [Librería Lever](#librería-lever)
- [Librería NRF24Controller](#librería-nrf24controller)
- [Instalación](#instalación)
- [Ejemplos](#ejemplos)
- [API Reference](#api-reference)

## 🕹️ Librería Joystick

### Características

- ✅ **Zona muerta configurable** (circular o rectangular)
- ✅ **Límites mínimos y máximos ajustables**
- ✅ **Calibración automática o manual**
- ✅ **Inversión de ejes independiente**
- ✅ **Filtrado y suavizado de valores**
- ✅ **Múltiples formatos de salida** (raw, porcentaje, mapeado, float)
- ✅ **Soporte para botón integrado**
- ✅ **Detección de posición neutral y bordes**
- ✅ **Cálculo de magnitud y ángulo**

### Uso Básico

```cpp
#include <Joystick.h>

// Crear instancia (pinX, pinY, pinButton)
Joystick joystick(A0, A1, 2);

void setup() {
    joystick.begin();
    
    // Configurar zona muerta circular de 80 unidades
    joystick.setDeadZone(80, true);
    
    // Habilitar suavizado
    joystick.setSmoothing(true, 0.2);
    
    // Invertir eje Y si es necesario
    joystick.invertAxis(false, true);
}

void loop() {
    int x = joystick.readX();        // -100 a 100
    int y = joystick.readY();        // -100 a 100
    float mag = joystick.readMagnitude();
    bool pressed = joystick.wasPressed();
}
```

## 🎛️ Librería Lever

### Características

- ✅ **Soporte para múltiples tipos**: Analógico, Encoder Rotativo, Digital
- ✅ **Configuración de límites personalizables**
- ✅ **Detección de dirección y velocidad**
- ✅ **Suavizado configurable para palancas analógicas**
- ✅ **Soporte para encoder con interrupciones**
- ✅ **Posiciones discretas para palancas digitales**
- ✅ **Detección de centro y extremos**
- ✅ **Múltiples formatos de salida**

### Tipos de Palancas Soportadas

#### 1. Palanca Analógica (Potenciómetro)
```cpp
Lever analogLever(ANALOG_LEVER, A2);

void setup() {
    analogLever.begin();
    analogLever.setAnalogLimits(0, 4095, 2048);  // min, max, center
    analogLever.setDeadZone(100);
    analogLever.setSmoothing(true, 0.15);
}
```

#### 2. Encoder Rotativo
```cpp
Lever rotaryEncoder(ROTARY_ENCODER, 2, 3, 4);  // pinA, pinB, button

void setup() {
    rotaryEncoder.begin();
    rotaryEncoder.setEncoderLimits(-50, 50);
    rotaryEncoder.setStepsPerDetent(4);
}

void loop() {
    rotaryEncoder.update();  // Llamar regularmente
    int pos = rotaryEncoder.readPosition();
    int direction = rotaryEncoder.getEncoderDirection();
}
```

#### 3. Palanca Digital (Switches)
```cpp
Lever digitalLever(DIGITAL_LEVER, 6, 7);  // pin up, pin down

void setup() {
    digitalLever.begin();
    digitalLever.setDigitalPositions(5);  // 5 posiciones (0-4)
}
```

## � **Librería NRF24Controller**

### Características

- ✅ **Integración perfecta** con librerías Joystick y Lever
- ✅ **Sistema de paquetes flexible** - agregar/quitar controles dinámicamente
- ✅ **Configuración avanzada de NRF24L01** (potencia, canal, velocidad)
- ✅ **Transmisión automática o manual**
- ✅ **Detección de cambios** para envío eficiente
- ✅ **Sistema de estadísticas** y monitoreo de conexión
- ✅ **Manejo de failsafe** automático
- ✅ **Soporte para datos personalizados**
- ✅ **Escaneo de canales** para evitar interferencias

### Uso Básico

```cpp
#include <NRF24Controller.h>

// Crear controlador NRF24
NRF24Controller nrf(9, 10);  // CE, CSN pins

// Crear controles
Joystick joy(A0, A1, 2);
Lever throttle(ANALOG_LEVER, A2);

void setup() {
    nrf.begin();
    nrf.setChannel(76);
    nrf.setPowerLevel(POWER_HIGH);
    
    // Agregar controles
    nrf.addJoystick(&joy, 0);
    nrf.addLever(&throttle, 0);
    
    // Configurar envío automático
    nrf.setAutoSend(true, 50);  // Cada 50ms
}

void loop() {
    nrf.update();  // Maneja todo automáticamente
}
```

### Configuración de Potencia

```cpp
nrf.setPowerLevel(POWER_MIN);    // -18dBm (corto alcance)
nrf.setPowerLevel(POWER_LOW);    // -12dBm 
nrf.setPowerLevel(POWER_HIGH);   // -6dBm
nrf.setPowerLevel(POWER_MAX);    // 0dBm (máximo alcance)
```

### Configuración Flexible de Datos

```cpp
// Habilitar/deshabilitar controles específicos
nrf.enableJoystick(0, true);   // Habilitar joystick 0
nrf.enableLever(1, false);     // Deshabilitar lever 1

// Configurar umbrales de cambio
nrf.setSendThresholds(5, 3);   // Joystick: 5, Lever: 3

// Enviar solo cuando hay cambios
nrf.setSendOnlyChanges(true);
```

## �📦 Instalación

1. Copia las carpetas `Joystick`, `Lever` y `NRF24Controller` a tu directorio `lib/` del proyecto
2. Instala la librería RF24 desde el Library Manager de Arduino
3. Incluye las librerías en tu código:
```cpp
#include <Joystick.h>
#include <Lever.h>
#include <NRF24Controller.h>
```

## 📚 Ejemplos

### Ejemplos Incluidos

1. **JoystickExample.cpp** - Uso básico del joystick
2. **LeverExample.cpp** - Ejemplos de los tres tipos de palancas
3. **CombinedExample.cpp** - Sistema de control completo combinando ambas librerías
4. **TransmitterExample.cpp** - Transmisor completo con NRF24L01
5. **ReceiverExample.cpp** - Receptor con manejo de failsafe y servos
6. **SimpleExample.cpp** - Ejemplo básico para pruebas rápidas

### Ejemplo Completo con NRF24L01

#### Transmisor
```cpp
#include <Joystick.h>
#include <Lever.h>
#include <NRF24Controller.h>

NRF24Controller nrf(9, 10);          // CE, CSN
Joystick joy(A0, A1, 2);            // Joystick principal
Lever throttle(ANALOG_LEVER, A2);    // Palanca de aceleración

void setup() {
    // Inicializar controles
    joy.begin();
    throttle.begin();
    
    // Configurar NRF24
    nrf.begin();
    nrf.setChannel(76);
    nrf.setPowerLevel(POWER_HIGH);
    
    // Agregar controles
    nrf.addJoystick(&joy, 0);
    nrf.addLever(&throttle, 0);
    
    // Envío automático cada 50ms
    nrf.setAutoSend(true, 50);
}

void loop() {
    nrf.update();  // Maneja todo automáticamente
}
```

#### Receptor
```cpp
#include <NRF24Controller.h>
#include <Servo.h>

NRF24Controller nrf(9, 10);
Servo servo1, servo2;

void setup() {
    servo1.attach(3);
    servo2.attach(5);
    
    nrf.begin();
    nrf.setChannel(76);
    nrf.setAddresses(0xE8E8F0F0E2LL, 0xE8E8F0F0E1LL); // Direcciones invertidas
    nrf.startListening();
}

void loop() {
    if (nrf.available()) {
        DataPacket packet;
        if (nrf.readData(packet)) {
            // Procesar datos y mover servos
            processControlData(packet);
        }
    }
}
```

## 📖 API Reference

### Clase Joystick

#### Métodos de Configuración
- `begin()` - Inicializar joystick
- `setDeadZone(radius, circular)` - Configurar zona muerta
- `setLimits(minX, maxX, minY, maxY)` - Establecer límites
- `setCenter(centerX, centerY)` - Configurar posición central
- `invertAxis(invertX, invertY)` - Invertir ejes
- `setSmoothing(enable, factor)` - Configurar suavizado
- `calibrate()` - Calibración manual interactiva
- `autoCalibrate(duration)` - Calibración automática

#### Métodos de Lectura
- `readX()` / `readY()` - Valores -100 a 100
- `readXFloat()` / `readYFloat()` - Valores -1.0 a 1.0
- `readXMapped(min, max)` / `readYMapped(min, max)` - Valores mapeados
- `readMagnitude()` - Magnitud del vector (0.0 a 1.0)
- `readAngle()` / `readAngleDegrees()` - Ángulo en radianes/grados

#### Métodos de Estado
- `isPressed()` - Botón presionado actualmente
- `wasPressed()` / `wasReleased()` - Detección de eventos
- `isNeutral()` - En posición neutral
- `isAtEdge()` - En posición extrema

### Clase Lever

#### Métodos de Configuración
- `begin()` - Inicializar palanca
- `setAnalogLimits(min, max, center)` - Límites para palanca analógica
- `setEncoderLimits(minSteps, maxSteps)` - Límites para encoder
- `setDigitalPositions(positions)` - Posiciones para palanca digital
- `setSmoothing(enable, factor)` - Configurar suavizado
- `invertDirection(invert)` - Invertir dirección

#### Métodos de Lectura
- `readPosition()` - Posición procesada (-100 a 100 o 0 a 100)
- `readPositionFloat()` - Posición como float
- `readMapped(min, max)` - Valor mapeado
- `readVelocity()` - Velocidad de cambio
- `readEncoderSteps()` - Pasos del encoder (solo encoders)

#### Métodos de Estado
- `update()` - Actualizar estado (llamar en loop)
- `isMoving()` / `isMovingLeft()` / `isMovingRight()` - Detección de movimiento
- `isAtCenter()` / `isAtMinimum()` / `isAtMaximum()` - Posición
- `getEncoderDirection()` - Dirección del encoder (-1, 0, 1)

### Clase NRF24Controller

#### Configuración Básica
- `begin()` - Inicializar NRF24L01
- `setChannel(channel)` - Configurar canal (0-125)
- `setPowerLevel(level)` - Nivel de potencia (POWER_MIN/LOW/HIGH/MAX)
- `setDataRate(rate)` - Velocidad datos (RATE_250KBPS/1MBPS/2MBPS)
- `setAddresses(txAddr, rxAddr)` - Direcciones de comunicación

#### Gestión de Controles
- `addJoystick(joystick, id)` - Agregar joystick
- `addLever(lever, id)` - Agregar palanca
- `enableJoystick(id, enable)` - Habilitar/deshabilitar joystick
- `enableLever(id, enable)` - Habilitar/deshabilitar palanca

#### Transmisión
- `setAutoSend(enable, interval)` - Envío automático
- `setSendThresholds(joyThreshold, leverThreshold)` - Umbrales de cambio
- `setSendOnlyChanges(enable)` - Enviar solo cambios
- `sendData()` - Enviar datos manualmente
- `update()` - Actualizar estado (llamar en loop)

#### Recepción
- `available()` - Verificar datos disponibles
- `readData(packet)` - Leer paquete completo
- `startListening()` / `stopListening()` - Control de modo

#### Diagnóstico
- `getStats()` - Estadísticas de transmisión
- `printStatus()` - Mostrar estado del sistema
- `scanChannels()` - Escanear interferencias
- `getOptimalChannel()` - Encontrar mejor canal

## ⚙️ Configuración Avanzada

### Zona Muerta del Joystick

```cpp
// Zona muerta circular de 80 unidades
joystick.setDeadZone(80, true);

// Zona muerta rectangular de 50 unidades
joystick.setDeadZone(50, false);
```

### Suavizado de Valores

```cpp
// Suavizado ligero (10%)
joystick.setSmoothing(true, 0.1);

// Suavizado más agresivo (30%)
joystick.setSmoothing(true, 0.3);
```

### Calibración Personalizada

```cpp
// Calibración interactiva
joystick.calibrate();

// Configuración manual si conoces los valores
joystick.setLimits(100, 3995, 80, 4015);
joystick.setCenter(2048, 2047);
```

## 🔧 Consideraciones de Hardware

### Joystick Analógico
- Conectar ejes X e Y a pines ADC
- Botón a pin digital con pull-up interno
- Voltaje de referencia estable recomendado

### Encoder Rotativo
- Conectar a pines con capacidad de interrupción
- Pull-ups internos habilitados automáticamente
- Para mejor rendimiento, usar interrupciones externas

### Palanca Analógica
- Potenciómetro conectado entre VCC y GND
- Cursor a pin ADC
- Filtro capacitivo opcional para estabilidad

## ⚡ Características Avanzadas de NRF24Controller

### Sistema de Paquetes Flexible
```cpp
// Estructura de paquete personalizable
struct ControlData {
    uint8_t id;           // ID del control (0-255)
    ControlType type;     // Tipo (joystick, lever, custom)
    int16_t valueX, valueY; // Valores principales
    uint8_t flags;        // Estados adicionales
    uint32_t timestamp;   // Marca de tiempo
};
```

### Configuración de Potencia Inteligente
```cpp
// Ajuste automático según distancia
nrf.setPowerLevel(POWER_MIN);    // Corto alcance, ahorro energía
nrf.setPowerLevel(POWER_MAX);    // Largo alcance, máxima potencia

// Escaneo de canales para evitar interferencias
uint8_t bestChannel = nrf.getOptimalChannel();
nrf.setChannel(bestChannel);
```

### Manejo Robusto de Conexión
- **Detección automática de pérdida de conexión**
- **Sistema de failsafe configurable**
- **Estadísticas de transmisión en tiempo real**
- **Verificación de integridad con checksums**

## 🚀 Casos de Uso

- **Drones y aviones RC** - Control completo con telemetría
- **Simuladores de vuelo** - Control preciso con trim y múltiples ejes
- **Sistemas de cámara** - Control pan/tilt suave inalámbrico
- **Control de vehículos** - Dirección, aceleración y funciones auxiliares
- **Robótica** - Control remoto de brazos robóticos y rovers
- **Equipos industriales** - Control de grúas, excavadoras, etc.
- **Sistemas de juego** - Controladores inalámbricos personalizados

## � Conexiones NRF24L01

### Conexión Típica Arduino/ESP32
```
NRF24L01    Arduino    ESP32
VCC         3.3V       3.3V
GND         GND        GND
CE          9          22
CSN         10         21
SCK         13         18
MOSI        11         23
MISO        12         19
```

### Configuración en Código
```cpp
// Para Arduino Uno/Nano
NRF24Controller nrf(9, 10);  // CE, CSN

// Para ESP32
NRF24Controller nrf(22, 21); // CE, CSN
```

## 📝 Notas Importantes

- **Compatibilidad**: Arduino, ESP32, ESP8266
- **Resolución ADC**: 10-12 bits soportados automáticamente
- **Alimentación NRF24L01**: Usar 3.3V, capacitor de 10-100µF recomendado
- **Librería RF24**: Instalar desde Library Manager antes de usar
- **Interrupciones**: Para encoders, usar pines con capacidad de interrupción
- **Memoria**: Aproximadamente 2KB RAM y 8KB Flash por proyecto
- **Velocidad**: Hasta 2Mbps de transmisión de datos

## 💡 Tips de Optimización

### Ahorro de Energía
```cpp
// Reducir potencia para ahorro energético
nrf.setPowerLevel(POWER_MIN);

// Envío menos frecuente
nrf.setAutoSend(true, 200); // Cada 200ms

// Power down cuando no se use
nrf.powerDown();
```

### Máximo Rendimiento
```cpp
// Máxima potencia y velocidad
nrf.setPowerLevel(POWER_MAX);
nrf.setDataRate(RATE_2MBPS);

// Envío de alta frecuencia
nrf.setAutoSend(true, 20); // Cada 20ms

// Canal libre de interferencias
uint8_t bestChannel = nrf.getOptimalChannel();
nrf.setChannel(bestChannel);
```

## 🤝 Contribuciones

Si encuentras bugs o tienes sugerencias de mejoras, no dudes en reportarlos o contribuir al código.