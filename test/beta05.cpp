#include <SPI.h>
#include <TFT_eSPI.h>    
#include <lvgl.h>
#include <ui.h>  
#include <nRF24L01.h>
#include <RF24.h>

#include "ConfigStorage.h"
#include <Joystick.h>

ConfigStorage config;
Joystick joystick_izquierdo(5, 2, 4);
Joystick joystick_derecho(8, 9, 10);

#define TFT_LED 38

// CONFIGURACION PINS NRF24L01
#define NRF24_CE 6
#define NRF24_CSN 7

// **CREAR INSTANCIA SPI SEPARADA PARA NRF24L01**
SPIClass nrf_spi(HSPI);  // Usar HSPI para ESP32-S2
RF24 radio(NRF24_CE, NRF24_CSN);

const uint64_t my_radio_pipe = 0xE8E8F0F0E1LL;
struct Data_to_be_sent {
  byte ch1;
};

Data_to_be_sent sent_data;
bool nrf24_available = false;

static uint8_t original_turn_limits[3] = {0, 0, 0};
static uint8_t current_turn_limits[3] = {0, 0, 0};
static uint8_t current_position = 0;

uint8_t palanca1[3] = {128, 128, 128};

bool brightness_calibration_mode = false;
bool nrf24_calibration_mode = false;
bool palanca1_calibration_mode = false;
bool palanca2_calibration_mode = false;
bool palanca3_calibration_mode = false;
bool palanca4_calibration_mode = false;
bool settings_calibration_mode = false;
bool intensidad_calibration_mode = false;

extern "C" {
    void applyBrightness(int brightness_value);
    uint8_t getBrightnessLimit();
    void setBrightnessLimit(uint8_t value);
    void saveCurrentConfig();
    uint64_t getNRFAddress();
    void setNRFAddress(uint64_t address);
    void getTurnLimits(uint8_t* pos1, uint8_t* pos2, uint8_t* pos3);
    void setTurnLimits(uint8_t pos1, uint8_t pos2, uint8_t pos3);
    void getSpeedLimits(uint8_t* pos1, uint8_t* pos2, uint8_t* pos3);
    void setSpeedLimits(uint8_t pos1, uint8_t pos2, uint8_t pos3);
    void getBoostLimits(uint8_t* pos1, uint8_t* pos2, uint8_t* pos3);
    void setBoostLimits(uint8_t pos1, uint8_t pos2, uint8_t pos3);
    void getExtraLimits(uint8_t* pos1, uint8_t* pos2, uint8_t* pos3);
    void setExtraLimits(uint8_t pos1, uint8_t pos2, uint8_t pos3);
    void setActiveProfile(uint8_t profile);
    uint8_t getActiveProfile();
    void setIntensity(uint8_t intensity);
    uint8_t getIntensityLimit();
    void clearAllProfiles();
    bool repairProfile(uint8_t profile);
    void updatePalanca1Vector();
}

TFT_eSPI tft = TFT_eSPI(); 
static const uint16_t screenWidth  = 320;
static const uint16_t screenHeight = 240;
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[ screenWidth * screenHeight / 10 ];

void my_disp_flush( lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p )
{
    uint32_t w = ( area->x2 - area->x1 + 1 );
    uint32_t h = ( area->y2 - area->y1 + 1 );

    tft.startWrite();
    tft.setAddrWindow( area->x1, area->y1, w, h );
    tft.pushColors( ( uint16_t * )&color_p->full, w * h, true );
    tft.endWrite();

    lv_disp_flush_ready( disp );
}

void my_touchpad_read( lv_indev_drv_t * indev_driver, lv_indev_data_t * data )
{
    uint16_t touchX = 0, touchY = 0;

    bool touched = tft.getTouch( &touchX, &touchY, 10 );

    if( !touched )
    {
        data->state = LV_INDEV_STATE_REL;
    }
    else
    {
        data->state = LV_INDEV_STATE_PR;
        data->point.x = touchX;
        data->point.y = touchY;
    }
}

void applyBrightness(int brightness_value) {
    if (brightness_value < 0) brightness_value = 0;
    if (brightness_value > 255) brightness_value = 255;
    analogWrite(TFT_LED, brightness_value);
}

uint8_t getBrightnessLimit() {
    return config.getBrightnessLimit();
}

void setBrightnessLimit(uint8_t value) {
    config.setBrightnessLimit(value);
}

void saveCurrentConfig() {
    config.saveCurrentConfig();
}

uint64_t getNRFAddress() {
    return config.getNRFAddress();
}

void setNRFAddress(uint64_t address) {
    config.setNRFAddress(address);
}

void getTurnLimits(uint8_t* pos1, uint8_t* pos2, uint8_t* pos3) {
    config.getTurnLimits(pos1, pos2, pos3);
}

void setTurnLimits(uint8_t pos1, uint8_t pos2, uint8_t pos3) {
    config.setTurnLimits(pos1, pos2, pos3);
}

void getSpeedLimits(uint8_t* pos1, uint8_t* pos2, uint8_t* pos3) {
    config.getSpeedLimits(pos1, pos2, pos3);
}

void setSpeedLimits(uint8_t pos1, uint8_t pos2, uint8_t pos3) {
    config.setSpeedLimits(pos1, pos2, pos3);
}

void getBoostLimits(uint8_t* pos1, uint8_t* pos2, uint8_t* pos3) {
    config.getBoostLimits(pos1, pos2, pos3);
}

void setBoostLimits(uint8_t pos1, uint8_t pos2, uint8_t pos3) {
    config.setBoostLimits(pos1, pos2, pos3);
}

void getExtraLimits(uint8_t* pos1, uint8_t* pos2, uint8_t* pos3) {
    config.getExtraLimits(pos1, pos2, pos3);
}

void setExtraLimits(uint8_t pos1, uint8_t pos2, uint8_t pos3) {
    config.setExtraLimits(pos1, pos2, pos3);
}

void setActiveProfile(uint8_t profile) {
    config.setActiveProfile(profile);
    updatePalanca1Vector();
}

uint8_t getActiveProfile() {
    return config.getActiveProfile();
}

void setIntensity(uint8_t intensity) {
    config.setIntensity(intensity);
}

uint8_t getIntensityLimit() {
    return config.getIntensityLimit();
}

void clearAllProfiles() {
    config.clearAllProfiles();
}

bool repairProfile(uint8_t profile) {
    return config.repairProfile(profile);
}

void updatePalanca1Vector() {
    uint8_t temp_limits[3];
    getTurnLimits(&temp_limits[0], &temp_limits[1], &temp_limits[2]);
    palanca1[0] = temp_limits[0];
    palanca1[1] = temp_limits[1];
    palanca1[2] = temp_limits[2];
}

void setup() {
    pinMode(TFT_LED, OUTPUT);
    Serial.begin(9600);
    delay(2000);
    Serial.println("=== INICIANDO SETUP ESP32-S2 ===");
    
    if (!config.begin()) {
        Serial.println("Error: Config no inicializado");
        return;
    }
    
    uint8_t temp_limits[3];
    getTurnLimits(&temp_limits[0], &temp_limits[1], &temp_limits[2]);
    palanca1[0] = temp_limits[0];
    palanca1[1] = temp_limits[1];
    palanca1[2] = temp_limits[2];
    
    analogWrite(TFT_LED, config.getBrightnessLimit());

    // *** INICIALIZAR SPI SEPARADOS ***
    Serial.println("*** CONFIGURANDO SPI SEPARADOS ***");
    
    // SPI para TFT (VSPI por defecto)
    Serial.println("Inicializando SPI para TFT (VSPI)...");
    SPI.begin(36, 37, 35, 15);  // SCK, MISO, MOSI, CS para TFT
    Serial.println("TFT SPI configurado en VSPI");
    
    // SPI separado para NRF24L01 (HSPI)
    Serial.println("Inicializando SPI para NRF24L01 (HSPI)...");
    nrf_spi.begin(14, 12, 13, NRF24_CSN);  // Pines HSPI estándar
    Serial.println("NRF24 SPI configurado en HSPI");

    // *** INICIALIZAR TFT Y LVGL PRIMERO ***
    Serial.println("Inicializando TFT y LVGL...");
    lv_init();
    tft.init();
    tft.setRotation(1);

    uint16_t calData[5] = { 140, 3820, 250, 3600, 7 };
    tft.setTouch(calData);

    lv_disp_draw_buf_init(&draw_buf, buf, NULL, screenWidth * screenHeight / 10);

    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = screenWidth;
    disp_drv.ver_res = screenHeight;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);

    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = my_touchpad_read;
    lv_indev_drv_register(&indev_drv); 

    ui_init();
    Serial.println("TFT y LVGL inicializados OK");

    // *** INICIALIZAR NRF24L01 CON SU PROPIO SPI ***
    Serial.println("*** INICIALIZANDO NRF24L01 CON SPI SEPARADO ***");
    
    // Configurar pines de control
    pinMode(NRF24_CE, OUTPUT);
    pinMode(NRF24_CSN, OUTPUT);
    digitalWrite(NRF24_CE, LOW);
    digitalWrite(NRF24_CSN, HIGH);
    delay(100);
    
    bool nrf_success = false;
    
    // Intentar inicializar con SPI separado
    for (int attempt = 1; attempt <= 3; attempt++) {
        Serial.printf("NRF24 Intento %d/3 con HSPI...\n", attempt);
        
        // Reset
        digitalWrite(NRF24_CE, LOW);
        digitalWrite(NRF24_CSN, HIGH);
        delay(50);
        
        // Inicializar con SPI separado
        if (radio.begin(&nrf_spi)) {
            Serial.printf("radio.begin(&nrf_spi) OK en intento %d\n", attempt);
            delay(100);
            
            if (radio.isChipConnected()) {
                Serial.printf("¡NRF24L01 DETECTADO CON HSPI EN INTENTO %d!\n", attempt);
                
                // Configuración completa
                radio.setAutoAck(false);
                radio.setDataRate(RF24_250KBPS);
                radio.setPALevel(RF24_PA_LOW);
                radio.setChannel(76);
                radio.openWritingPipe(my_radio_pipe);
                radio.stopListening();
                sent_data.ch1 = 0;
                
                delay(50);
                if (radio.isChipConnected()) {
                    Serial.println("*** NRF24L01 CONFIGURADO CON HSPI ***");
                    radio.printPrettyDetails();
                    nrf_success = true;
                    break;
                } else {
                    Serial.printf("Configuración perdida en intento %d\n", attempt);
                }
            } else {
                Serial.printf("Chip no responde con HSPI en intento %d\n", attempt);
            }
        } else {
            Serial.printf("radio.begin(&nrf_spi) FALLÓ en intento %d\n", attempt);
        }
        
        delay(300 * attempt);
    }
    
    nrf24_available = nrf_success;

    // *** MOSTRAR RESULTADO ***
    if (nrf24_available) {
        tft.fillScreen(TFT_GREEN);
        tft.setTextColor(TFT_BLACK);
        tft.drawString("NRF24 OK!", 50, 50, 4);
        tft.drawString("HSPI SEPARADO", 30, 100, 2);
        Serial.println("✅ NRF24L01 Y TFT FUNCIONANDO CON SPI SEPARADOS");
    } else {
        tft.fillScreen(TFT_RED);
        tft.setTextColor(TFT_WHITE);
        tft.drawString("NRF24 ERROR", 40, 50, 3);
        tft.drawString("TFT OK", 80, 100, 2);
        Serial.println("❌ SOLO TFT FUNCIONANDO");
        
        // Información de debug
        Serial.println("VERIFICAR CONEXIONES NRF24L01:");
        Serial.println("VCC → 3.3V + capacitor 100µF");
        Serial.println("GND → GND");
        Serial.println("CE → Pin 6");
        Serial.println("CSN → Pin 7");
        Serial.println("SCK → Pin 14 (HSPI)");
        Serial.println("MISO → Pin 12 (HSPI)");
        Serial.println("MOSI → Pin 13 (HSPI)");
    }
    
    delay(3000);

    // INICIALIZAR JOYSTICKS
    Serial.println("Inicializando joysticks...");
    joystick_izquierdo.begin();
    joystick_izquierdo.setCenter(5520, 5160);
    joystick_izquierdo.setDeadZone(100, true);
    joystick_izquierdo.setLimits(60, 8180, 65, 8180);
    joystick_izquierdo.invertAxis(false, false);

    joystick_derecho.begin();
    joystick_derecho.setCenter(5060, 4970);
    joystick_derecho.setDeadZone(100, true);
    joystick_derecho.setLimits(60, 8180, 65, 8180);
    joystick_derecho.invertAxis(false, false);
    Serial.println("Joysticks OK");
    
    tft.fillScreen(TFT_BLACK);
    Serial.println("=== SETUP COMPLETADO ===");
}

void loop(void) {
    // Código de joysticks y barras LVGL (sin cambios)
    int val_izquierdo_Y = joystick_izquierdo.readY();
    if (val_izquierdo_Y > 0) {
        lv_bar_set_value(ui_BarJoystickIzquierdoSup1, val_izquierdo_Y, LV_ANIM_ON);
        lv_bar_set_start_value(ui_BarJoystickIzquierdoSup2, 255, LV_ANIM_ON);
    } else if (val_izquierdo_Y < 0) {
        lv_bar_set_value(ui_BarJoystickIzquierdoSup1, 0, LV_ANIM_ON);
        lv_bar_set_start_value(ui_BarJoystickIzquierdoSup2, map(val_izquierdo_Y, 0, -255, 255, 0), LV_ANIM_ON);
    } else {
        lv_bar_set_value(ui_BarJoystickIzquierdoSup1, 0, LV_ANIM_ON);
        lv_bar_set_start_value(ui_BarJoystickIzquierdoSup2, 255, LV_ANIM_ON);
    }

    int val_izquierdo_X = joystick_izquierdo.readX();
    if (val_izquierdo_X > 0) {
        lv_bar_set_value(ui_BarJoystickIzquierdoSup5, val_izquierdo_X, LV_ANIM_ON);
        lv_bar_set_start_value(ui_BarJoystickIzquierdoSup6, 255, LV_ANIM_ON);
    } else if (val_izquierdo_X < 0) {
        lv_bar_set_value(ui_BarJoystickIzquierdoSup5, 0, LV_ANIM_ON);
        lv_bar_set_start_value(ui_BarJoystickIzquierdoSup6, map(val_izquierdo_X, 0, -255, 255, 0), LV_ANIM_ON);
    } else {
        lv_bar_set_value(ui_BarJoystickIzquierdoSup5, 0, LV_ANIM_ON);
        lv_bar_set_start_value(ui_BarJoystickIzquierdoSup6, 255, LV_ANIM_ON);
    }

    int val_Derecho_Y = joystick_derecho.readY();
    if (val_Derecho_Y > 0) {
        lv_bar_set_value(ui_BarJoystickIzquierdoSup3, val_Derecho_Y, LV_ANIM_ON);
        lv_bar_set_start_value(ui_BarJoystickIzquierdoSup4, 255, LV_ANIM_ON);
    } else if (val_Derecho_Y < 0) {
        lv_bar_set_value(ui_BarJoystickIzquierdoSup3, 0, LV_ANIM_ON);
        lv_bar_set_start_value(ui_BarJoystickIzquierdoSup4, map(val_Derecho_Y, 0, -255, 255, 0), LV_ANIM_ON);
    } else {
        lv_bar_set_value(ui_BarJoystickIzquierdoSup3, 0, LV_ANIM_ON);
        lv_bar_set_start_value(ui_BarJoystickIzquierdoSup4, 255, LV_ANIM_ON);
    }

    int val_Derecho_X = joystick_derecho.readX();
    if (val_Derecho_X > 0) {
        lv_bar_set_value(ui_BarJoystickIzquierdoSup7, val_Derecho_X, LV_ANIM_ON);
        lv_bar_set_start_value(ui_BarJoystickIzquierdoSup8, 255, LV_ANIM_ON);
    } else if (val_Derecho_X < 0) {
        lv_bar_set_value(ui_BarJoystickIzquierdoSup7, 0, LV_ANIM_ON);
        lv_bar_set_start_value(ui_BarJoystickIzquierdoSup8, map(val_Derecho_X, 0, -255, 255, 0), LV_ANIM_ON);
    } else {
        lv_bar_set_value(ui_BarJoystickIzquierdoSup7, 0, LV_ANIM_ON);
        lv_bar_set_start_value(ui_BarJoystickIzquierdoSup8, 255, LV_ANIM_ON);
    }

    // *** TRANSMISIÓN NRF24 CON SPI SEPARADO - SIN INTERFERENCIAS ***
    if (nrf24_available) {
        static unsigned long last_nrf_time = 0;
        static uint8_t tx_counter = 0;
        static uint8_t error_count = 0;
        unsigned long now = millis();
        
        if (now - last_nrf_time >= 50) { // 20Hz
            sent_data.ch1 = map(analogRead(8), 0, 4095, 0, 255);
            
            // Transmitir usando el SPI separado (nrf_spi)
            if (radio.write(&sent_data, sizeof(Data_to_be_sent))) {
                tx_counter++;
                error_count = 0; // Reset errores
                
                // Status cada 100 transmisiones exitosas
                if (tx_counter % 100 == 0) {
                    Serial.printf("✅ NRF24 TX OK - %d paquetes enviados\n", tx_counter);
                }
            } else {
                error_count++;
                
                // Solo mostrar errores ocasionalmente para no saturar Serial
                if (error_count % 20 == 1) {
                    Serial.printf("❌ Error NRF24 TX (%d errores)\n", error_count);
                    
                    // Si hay muchos errores, verificar conexión
                    if (error_count > 50) {
                        if (!radio.isChipConnected()) {
                            Serial.println("⚠️  NRF24L01 desconectado - verificar alimentación");
                            nrf24_available = false; // Deshabilitar hasta reinicio
                        }
                        error_count = 0; // Reset contador
                    }
                }
            }
            
            last_nrf_time = now;
        }
    }

    lv_timer_handler(); 
}
