// Setup for the ESP32 S2 Mini with ILI9341 display
#define USER_SETUP_ID 70

#define ILI9341_DRIVER

// Pines según tu conexión
#define TFT_CS   15    // Chip Select display
#define TFT_MOSI 35    // SDI / T_DIN
#define TFT_SCLK 36    // SCK / T_CLK
#define TFT_MISO 37    // SDO / T_DO
#define TFT_DC   33    // Data/Command
#define TFT_RST  34    // Reset

// Touch controller (XPT2046 normalmente)
#define TOUCH_CS 18    // Chip Select touch
#define TOUCH_IRQ 21   // Interrupción táctil opcional

// Fuentes y opciones
#define LOAD_GLCD
#define LOAD_FONT2
#define LOAD_FONT4
#define LOAD_FONT6
#define LOAD_FONT7
#define LOAD_FONT8
#define LOAD_GFXFF
#define SMOOTH_FONT

// Puerto SPI (FSPI por defecto en ESP32-S2)
#define USE_HSPI_PORT

// Velocidades
#define SPI_FREQUENCY       40000000   // Máx para ILI9341
#define SPI_READ_FREQUENCY   6000000   // Lectura máx recomendada
#define SPI_TOUCH_FREQUENCY  2500000   // Para XPT2046

// SCK/ T_CLK 36
// SDI/T_DIN 35
// SDO/T_DO 37
// CS 15
// T_CS 18
// DC 33
// RESET 34
// T_IRQ 21
// VCC 3.3
// GND a GND 
// LED 38