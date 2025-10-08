# Controll_Beta01

Control remoto basado en ESP32-S2 con pantalla TFT, interfaz LVGL y radio nRF24L01.  
Permite configurar perfiles y 4 palancas físicas (3 posiciones) que ajustan límites para movimiento, velocidad, boost y una salida extra.

Resumen rápido
- MCU: ESP32-S2 (Adafruit Feather ESP32-S2)
- Pantalla: TFT_eSPI con touch (LVGL UI)
- Radio: nRF24L01 (RF24)
- Joysticks analógicos + 4 palancas digitales de 3 posiciones
- Almacenamiento: Preferences (lib ConfigStorage) para perfiles y límites

Características
- 4 palancas con 3 posiciones cada una, cargadas desde configuración (perfiles)
- Interfaz táctil para calibrar brillo, NRF address, palancas y perfiles
- Lectura de dos joysticks analógicos y mapeo a canales
- Transmisión periódica de estructura de datos por nRF24L01
- Guardado/cancelación de cambios desde la UI

Pinout (definidos en src/main.cpp)
- TFT LED: 38
- Lectura batería (analógico): 1
- nRF24 CE / CSN: 6 / 7
- Palanca 1: P1_1 = 13, P1_2 = 14
- Palanca 2: P2_1 = 11, P2_2 = 12
- Palanca 3: P3_1 = 40, P3_2 = 39
- Palanca 4: P4_1 = 16, P4_2 = 17
- Joysticks: joystick_izquierdo(AxPins: 5,2,4) joystick_derecho(8,9,10)
- Nota: HSPI nrf_spi usa pines SPI (14,12,13) durante init. El código reconfigura esos pines a INPUT_PULLUP después de inicializar nrf_spi / radio.

Riesgos conocidos y recomendaciones
- Conflicto de pines SPI: nrf_spi.begin(14,12,13,...) configura 13/12/14 como SPI. El proyecto reconfigura dichos pines a INPUT_PULLUP tras inicializar el módulo radio para leer palancas. Si hay cortocircuito al encender, revisar cableado físico (pines Vcc/GND), y evitar usar pines ocupados por periféricos a menos que estén correctamente desconectados.
- Evitar usar pin 1 como digital si lo usas para serial o alimentación. Verifica conexiones de batería y conversores.
- Si ves comportamiento extraño con palancas, mueve las declaraciones de arrays y llamadas pinMode en setup() según la secuencia de inicialización (nRF primero, luego pinMode INPUT_PULLUP).

Compilación y flasheo (PlatformIO, Windows)
- Abrir en VS Code con PlatformIO.
- Compilar: pio run
- Flashear a la placa: pio run -t upload
- Monitor serie: pio device monitor --baud 115200

Estructura del proyecto
- src/main.cpp — lógica principal, lectura palancas/joysticks, transmisión nRF y UI bridge
- lib/ConfigStorage — clase para gestionar perfiles y persistencia en Preferences
- .pio/libdeps/.../ui — código LVGL generado (pantallas y callbacks)
- README.md — (este archivo)

Uso básico
1. Conectar pantalla, nRF24L01 y palancas según wiring.
2. Flashear firmware con PlatformIO.
3. Abrir monitor serie (9600) para mensajes de debug.
4. Entrar a configuración en la UI para calibrar palancas, brillo y dirección nRF.
5. Guardar cambios para que queden persistidos por perfil.

Cómo contribuir
- Reportar issues en el repo con: logs serie, descripción del hardware y fotos de wiring si hay problemas eléctricos.
- Pull requests: mantener estilo del proyecto y probar en hardware.

Licencia
- Añadir la licencia que prefieras en LICENCE.md (no incluida por defecto).

Contacto
- Mantener issues y PRs en este repositorio.

```// filepath: c:\Users\alvar\OneDrive\Documents\PlatformIO\Projects\Controll_Beta01\README.md
# Controll_Beta01

Control remoto basado en ESP32-S2 con pantalla TFT, interfaz LVGL y radio nRF24L01.  
Permite configurar perfiles y 4 palancas físicas (3 posiciones) que ajustan límites para movimiento, velocidad, boost y una salida extra.

Resumen rápido
- MCU: ESP32-S2 (Adafruit Feather ESP32-S2)
- Pantalla: TFT_eSPI con touch (LVGL UI)
- Radio: nRF24L01 (RF24)
- Joysticks analógicos + 4 palancas digitales de 3 posiciones
- Almacenamiento: Preferences (lib ConfigStorage) para perfiles y límites

Características
- 4 palancas con 3 posiciones cada una, cargadas desde configuración (perfiles)
- Interfaz táctil para calibrar brillo, NRF address, palancas y perfiles
- Lectura de dos joysticks analógicos y mapeo a canales
- Transmisión periódica de estructura de datos por nRF24L01
- Guardado/cancelación de cambios desde la UI

Pinout (definidos en src/main.cpp)
- TFT LED: 38
- Lectura batería (analógico): 1
- nRF24 CE / CSN: 6 / 7
- Palanca 1: P1_1 = 13, P1_2 = 14
- Palanca 2: P2_1 = 11, P2_2 = 12
- Palanca 3: P3_1 = 40, P3_2 = 39
- Palanca 4: P4_1 = 16, P4_2 = 17
- Joysticks: joystick_izquierdo(AxPins: 5,2,4) joystick_derecho(8,9,10)
- Nota: HSPI nrf_spi usa pines SPI (14,12,13) durante init. El código reconfigura esos pines a INPUT_PULLUP después de inicializar nrf_spi / radio.

Riesgos conocidos y recomendaciones
- Conflicto de pines SPI: nrf_spi.begin(14,12,13,...) configura 13/12/14 como SPI. El proyecto reconfigura dichos pines a INPUT_PULLUP tras inicializar el módulo radio para leer palancas. Si hay cortocircuito al encender, revisar cableado físico (pines Vcc/GND), y evitar usar pines ocupados por periféricos a menos que estén correctamente desconectados.
- Evitar usar pin 1 como digital si lo usas para serial o alimentación. Verifica conexiones de batería y conversores.
- Si ves comportamiento extraño con palancas, mueve las declaraciones de arrays y llamadas pinMode en setup() según la secuencia de inicialización (nRF primero, luego pinMode INPUT_PULLUP).

Compilación y flasheo (PlatformIO, Windows)
- Abrir en VS Code con PlatformIO.
- Compilar: pio run
- Flashear a la placa: pio run -t upload
- Monitor serie: pio device monitor --baud 115200

Estructura del proyecto
- src/main.cpp — lógica principal, lectura palancas/joysticks, transmisión nRF y UI bridge
- lib/ConfigStorage — clase para gestionar perfiles y persistencia en Preferences
- .pio/libdeps/.../ui — código LVGL generado (pantallas y callbacks)
- README.md — (este archivo)

Uso básico
1. Conectar pantalla, nRF24L01 y palancas según wiring.
2. Flashear firmware con PlatformIO.
3. Abrir monitor serie (115200) para mensajes de debug.
4. Entrar a configuración en la UI para calibrar palancas, brillo y dirección nRF.
5. Guardar cambios para que queden persistidos por perfil.

Cómo contribuir
- Reportar issues en el repo con: logs serie, descripción del hardware y fotos de wiring si hay problemas eléctricos.
- Pull requests: mantener estilo del proyecto y probar en hardware.

Licencia
- Añadir la licencia que prefieras en LICENCE.md (no incluida por defecto).

Contacto
- Mantener issues y PRs en este repositorio.
