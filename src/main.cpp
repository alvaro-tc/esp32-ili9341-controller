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
#define BATTERY 3

#define NRF24_CE 6
#define NRF24_CSN 7
#define P1_1 13
#define P1_2 14

#define P2_1 11
#define P2_2 12

#define P3_1 40
#define P3_2 39 

#define P4_1 16
#define P4_2 17


SPIClass nrf_spi(HSPI);  // Usar HSPI para ESP32-S2
RF24 radio(NRF24_CE, NRF24_CSN);

const uint64_t my_radio_pipe = 0xE8E8F0F0E1LL;
struct Data_to_be_sent {
    byte ch1;
    byte ch2;
    byte ch3;
    byte ch4;
    byte ch5;
    byte ch6;
    byte ch7;
};

Data_to_be_sent sent_data;
bool nrf24_available = false;
// Variables para almacenar el estado actual de las palancas
uint8_t palanca1_position = 1; // Posición central por defecto
uint8_t palanca2_position = 1;
uint8_t palanca3_position = 1;
uint8_t palanca4_position = 1;

// Declarar los arrays antes de las funciones que los usan
uint8_t palanca1[3] = {128, 128, 128};
uint8_t palanca2[3] = {128, 128, 128};
uint8_t palanca3[3] = {128, 128, 128};
uint8_t palanca4[3] = {128, 128, 128};

// Función para leer la posición de la palanca 1
uint8_t readPalanca1Position() {
    bool pin1 = digitalRead(P1_1);
    bool pin2 = digitalRead(P1_2);
    
    if (!pin1 && pin2) {
        return 0; // Posición 0
    } else if (pin1 && pin2) {
        return 1; // Posición 1 (centro)
    } else if (pin1 && !pin2) {
        return 2; // Posición 2
    }
    return 1; // Por defecto centro
}

// Función para leer la posición de la palanca 2
uint8_t readPalanca2Position() {
    bool pin1 = digitalRead(P2_1);
    bool pin2 = digitalRead(P2_2);
    
    if (!pin1 && pin2) {
        return 0; // Posición 0
    } else if (pin1 && pin2) {
        return 1; // Posición 1 (centro)
    } else if (pin1 && !pin2) {
        return 2; // Posición 2
    }
    return 1; // Por defecto centro
}

// Función para leer la posición de la palanca 3
uint8_t readPalanca3Position() {
    bool pin1 = digitalRead(P3_1);
    bool pin2 = digitalRead(P3_2);
    
    if (!pin1 && pin2) {
        return 0; // Posición 0
    } else if (pin1 && pin2) {
        return 1; // Posición 1 (centro)
    } else if (pin1 && !pin2) {
        return 2; // Posición 2
    }
    return 1; // Por defecto centro
}

// Función para leer la posición de la palanca 4
uint8_t readPalanca4Position() {
    bool pin1 = digitalRead(P4_1);
    bool pin2 = digitalRead(P4_2);
    
    if (!pin1 && pin2) {
        return 0; // Posición 0
    } else if (pin1 && pin2) {
        return 1; // Posición 1 (centro)
    } else if (pin1 && !pin2) {
        return 2; // Posición 2
    }
    return 1; // Por defecto centro
}

// Función para actualizar todas las posiciones de las palancas
void updatePalancasPositions() {
    palanca1_position = readPalanca1Position();
    palanca2_position = readPalanca2Position();
    palanca3_position = readPalanca3Position();
    palanca4_position = readPalanca4Position();
}

// Función para obtener el valor configurado según la posición de la palanca
uint8_t getPalanca1Value() {
    return palanca1[palanca1_position];
}

uint8_t getPalanca2Value() {
    return palanca2[palanca2_position];
}

uint8_t getPalanca3Value() {
    return palanca3[palanca3_position];
}

uint8_t getPalanca4Value() {
    return palanca4[palanca4_position];
}

static uint8_t original_turn_limits[3] = {0, 0, 0};
static uint8_t current_turn_limits[3] = {0, 0, 0};
static uint8_t current_position = 0;


// Variables estáticas para palanca2 (speed limits)
static uint8_t original_speed_limits[3] = {0, 0, 0};
static uint8_t current_speed_limits[3] = {0, 0, 0};
static uint8_t current_speed_position = 0;


// Variables estáticas para palanca3 (boost limits)
static uint8_t original_boost_limits[3] = {0, 0, 0};
static uint8_t current_boost_limits[3] = {0, 0, 0};
static uint8_t current_boost_position = 0;

// Variables estáticas para palanca4 (extra limits)
static uint8_t original_extra_limits[3] = {0, 0, 0};
static uint8_t current_extra_limits[3] = {0, 0, 0};
static uint8_t current_extra_position = 0;

bool brightness_calibration_mode = false;
bool nrf24_calibration_mode = false;
bool palanca1_calibration_mode = false;
bool palanca2_calibration_mode = false;
bool palanca3_calibration_mode = false;
bool palanca4_calibration_mode = false;
bool settings_calibration_mode = false;
bool intensidad_calibration_mode = false;
bool canal_calibration_mode = false; // << añadido

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
    void updatePalanca2Vector();
    void updatePalanca3Vector();
    void updatePalanca4Vector();

    // getters/setters para índice 13 (canal)
    void setExtraConfig(uint8_t value);
    uint8_t getExtraConfig(void);
}

TFT_eSPI tft = TFT_eSPI(); 
static const uint16_t screenWidth  = 320;
static const uint16_t screenHeight = 240;
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[ screenWidth * screenHeight / 10 ];

// Añadir un style global (inicializarlo UNA vez en setup)
static lv_style_t style_bar_indicator;

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
    updatePalanca2Vector();
    updatePalanca3Vector();
    updatePalanca4Vector();
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

void updatePalanca2Vector() {
    uint8_t temp_limits[3];
    getSpeedLimits(&temp_limits[0], &temp_limits[1], &temp_limits[2]);
    palanca2[0] = temp_limits[0];
    palanca2[1] = temp_limits[1];
    palanca2[2] = temp_limits[2];
}

void updatePalanca3Vector() {
    uint8_t temp_limits[3];
    getBoostLimits(&temp_limits[0], &temp_limits[1], &temp_limits[2]);
    palanca3[0] = temp_limits[0];
    palanca3[1] = temp_limits[1];
    palanca3[2] = temp_limits[2];
}

void updatePalanca4Vector() {
    uint8_t temp_limits[3];
    getExtraLimits(&temp_limits[0], &temp_limits[1], &temp_limits[2]);
    palanca4[0] = temp_limits[0];
    palanca4[1] = temp_limits[1];
    palanca4[2] = temp_limits[2];
}

// wrapper para index 13 (extra config / canal)
void setExtraConfig(uint8_t value) {
    config.setExtraConfig(value);
}

uint8_t getExtraConfig(void) {
    return config.getExtraConfig();
}

void setup() {
    pinMode(TFT_LED, OUTPUT);
    pinMode(BATTERY, INPUT);
    Serial.begin(9600);
    
    if (!config.begin()) return;
    
    // Inicializar palancas (vectores)
    uint8_t temp_limits[3];
    getTurnLimits(&temp_limits[0], &temp_limits[1], &temp_limits[2]);
    palanca1[0] = temp_limits[0];
    palanca1[1] = temp_limits[1];
    palanca1[2] = temp_limits[2];
    
    getSpeedLimits(&temp_limits[0], &temp_limits[1], &temp_limits[2]);
    palanca2[0] = temp_limits[0];
    palanca2[1] = temp_limits[1];
    palanca2[2] = temp_limits[2];
    
    getBoostLimits(&temp_limits[0], &temp_limits[1], &temp_limits[2]);
    palanca3[0] = temp_limits[0];
    palanca3[1] = temp_limits[1];
    palanca3[2] = temp_limits[2];
    
    getExtraLimits(&temp_limits[0], &temp_limits[1], &temp_limits[2]);
    palanca4[0] = temp_limits[0];
    palanca4[1] = temp_limits[1];
    palanca4[2] = temp_limits[2];
    
    analogWrite(TFT_LED, config.getBrightnessLimit());

    // Inicializar SPI para TFT y NRF24
    SPI.begin(36, 37, 35, 15);        // TFT (VSPI)
    nrf_spi.begin(14, 12, 13, NRF24_CSN); // NRF24 (HSPI) - Solo para inicializar el módulo

    // Inicializar TFT y LVGL
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

    // Inicializar el style UNA sola vez (antes lo hacías en loop y reiniciaba/duplicaba)
    lv_style_init(&style_bar_indicator);
    lv_style_set_bg_color(&style_bar_indicator, lv_color_hex(0x00FF00));
    lv_style_set_bg_opa(&style_bar_indicator, LV_OPA_COVER);
    // Aplicar el style al indicador de la barra (ui_BarN existen después de ui_init)
    lv_obj_add_style(ui_Bar5,  &style_bar_indicator, LV_PART_INDICATOR);
    lv_obj_add_style(ui_Bar4,  &style_bar_indicator, LV_PART_INDICATOR);
    lv_obj_add_style(ui_Bar9,  &style_bar_indicator, LV_PART_INDICATOR);
    lv_obj_add_style(ui_Bar1,  &style_bar_indicator, LV_PART_INDICATOR);
    lv_obj_add_style(ui_Bar7,  &style_bar_indicator, LV_PART_INDICATOR);
    lv_obj_add_style(ui_Bar6,  &style_bar_indicator, LV_PART_INDICATOR);
    lv_obj_add_style(ui_Bar2,  &style_bar_indicator, LV_PART_INDICATOR);
    lv_obj_add_style(ui_Bar3,  &style_bar_indicator, LV_PART_INDICATOR);
    lv_obj_add_style(ui_Bar8,  &style_bar_indicator, LV_PART_INDICATOR);

    // Inicializar NRF24L01
    pinMode(NRF24_CE, OUTPUT);
    pinMode(NRF24_CSN, OUTPUT);
    digitalWrite(NRF24_CE, LOW);
    digitalWrite(NRF24_CSN, HIGH);
    
    if (radio.begin(&nrf_spi) && radio.isChipConnected()) {
        radio.setAutoAck(false);
        radio.setDataRate(RF24_250KBPS);
        switch (config.getIntensityLimit()) {
            case 1:
            radio.setPALevel(RF24_PA_MIN);
            break;
            case 2:
            radio.setPALevel(RF24_PA_LOW);
            break;
            case 3:
            radio.setPALevel(RF24_PA_HIGH);
            break;
            case 4:
            radio.setPALevel(RF24_PA_MAX);
            break;
            default:
            radio.setPALevel(RF24_PA_LOW);
            break;
        }
        // Valor por defecto 76 (canal 76)
        radio.setChannel(config.getExtraConfig());
        radio.openWritingPipe(config.getNRFAddress());
        radio.stopListening();
        sent_data.ch1 = 0;
        sent_data.ch2 = 0;
        sent_data.ch3 = 0;
        sent_data.ch4 = 0;
        sent_data.ch5 = 0;
        sent_data.ch6 = 0;
        sent_data.ch7 = 0;

        nrf24_available = true;
    }

    // DESPUÉS de inicializar el NRF24, reconfigurar los pines para las palancas
    // Configurar pines de las palancas como INPUT_PULLUP
    pinMode(P1_1, INPUT_PULLUP);  // Pin 13
    pinMode(P1_2, INPUT_PULLUP);  // Pin 14
    pinMode(P2_1, INPUT_PULLUP);  // Pin 11
    pinMode(P2_2, INPUT_PULLUP);  // Pin 12
    pinMode(P3_1, INPUT_PULLUP);  // Pin 1
    pinMode(P3_2, INPUT_PULLUP);  // Pin 39
    pinMode(P4_1, INPUT_PULLUP);  // Pin 16
    pinMode(P4_2, INPUT_PULLUP);  // Pin 17

    // Inicializar joysticks
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
}


// Valores de calibración para la lectura del ADC
const int ADC_bajo = 4850;   // ADC medido con batería baja (~3.3V)
const int ADC_alto = 6140;   // ADC medido con batería cargada (4.17V)
const float V_bajo = 3.3;    // Voltaje correspondiente al ADC_bajo
const float V_alto = 4.17;   // Voltaje correspondiente al ADC_alto

// Calculamos pendiente y offset
const float m = (V_alto - V_bajo) / (ADC_alto - ADC_bajo);
const float b = V_bajo - m * ADC_bajo;


// Función que devuelve el voltaje real (lectura suavizada para evitar fluctuaciones)
float leerVoltaje() {
  // filtro exponencial (EMA) - ajusta alpha entre 0.02..0.15 según suavidad deseada
  const float alpha = 0.08f; // 0 = muy suave / 1 = sin suavizado
  static float filtered_v = 0.0f;
  static bool initialized = false;

  int lectura = analogRead(BATTERY);
  float v_raw = m * lectura + b;

  if (!initialized) {
    filtered_v = v_raw;
    initialized = true;
  } else {
    filtered_v += alpha * (v_raw - filtered_v);
  }

  return filtered_v;
}

// Función que devuelve el porcentaje de batería
int leerPorcentaje() {
  float v = leerVoltaje();
  int pct = (v - V_bajo) / (V_alto - V_bajo) * 100;
  if (pct > 100) pct = 100;
  if (pct < 0) pct = 0;
  return pct;
}



void loop() {

    // ANTES: se inicializaba y añadía el style en cada iteración -> provoca fugas / corrupción LVGL
    // AHORA: solo actualizamos valor y color del style (sin re-inicializar ni re-adjuntar)
    
    int pct = leerPorcentaje();

    // Actualizar todos los indicadores de batería con el mismo valor
    lv_bar_set_value(ui_Bar5, pct, LV_ANIM_ON);
    lv_bar_set_value(ui_Bar4, pct, LV_ANIM_ON);
    lv_bar_set_value(ui_Bar9, pct, LV_ANIM_ON);
    lv_bar_set_value(ui_Bar1, pct, LV_ANIM_ON);
    lv_bar_set_value(ui_Bar7, pct, LV_ANIM_ON);
    lv_bar_set_value(ui_Bar6, pct, LV_ANIM_ON);
    lv_bar_set_value(ui_Bar2, pct, LV_ANIM_ON);
    lv_bar_set_value(ui_Bar3, pct, LV_ANIM_ON);
    lv_bar_set_value(ui_Bar8, pct, LV_ANIM_ON);

    // Cambiamos color según porcentaje (solo actualizar el style ya registrado)
    if (pct < 15) {
        lv_style_set_bg_color(&style_bar_indicator, lv_color_hex(0xFF0000));
    } else {
        lv_style_set_bg_color(&style_bar_indicator, lv_color_hex(0x00FF00));
    }


    int val_izquierdo_Y = joystick_izquierdo.readY();
    if (val_izquierdo_Y > 0) {
        if(joystick_derecho.isPressed()){
            uint16_t max_val = palanca1[readPalanca1Position()] + palanca3[readPalanca3Position()];
            if (max_val > 255) max_val = 255;
            sent_data.ch1 = map(val_izquierdo_Y, 0, 255, 0, max_val);
            sent_data.ch2 = 0;
        }else{
            sent_data.ch1 = map(val_izquierdo_Y, 0, 255, 0, palanca1[readPalanca1Position()] );
            sent_data.ch2 = 0;
        }

        lv_bar_set_value(ui_BarJoystickIzquierdoSup1, val_izquierdo_Y, LV_ANIM_ON);
        lv_bar_set_start_value(ui_BarJoystickIzquierdoSup2, 255, LV_ANIM_ON);

    } else if (val_izquierdo_Y < 0) {
        if(joystick_derecho.isPressed()){
            uint16_t max_val = palanca1[readPalanca1Position()] + palanca3[readPalanca3Position()];
            if (max_val > 255) max_val = 255;
            sent_data.ch2 = map(val_izquierdo_Y, 0, -255, 0, max_val);
            sent_data.ch1 = 0;
        }else{
            sent_data.ch2 = map(val_izquierdo_Y, 0, -255, 0, palanca1[readPalanca1Position()] );
            sent_data.ch1 = 0;
        }

        lv_bar_set_value(ui_BarJoystickIzquierdoSup1, 0, LV_ANIM_ON);
        lv_bar_set_start_value(ui_BarJoystickIzquierdoSup2, map(val_izquierdo_Y, 0, -255, 255, 0), LV_ANIM_ON);
    } else {
        sent_data.ch1 = 0;
        sent_data.ch2 = 0;
        lv_bar_set_value(ui_BarJoystickIzquierdoSup1, 0, LV_ANIM_ON);
        lv_bar_set_start_value(ui_BarJoystickIzquierdoSup2, 255, LV_ANIM_ON);
    }

    // Joystick izquierdo X
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

    // Joystick derecho Y
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

    // Joystick derecho X
    int val_Derecho_X = joystick_derecho.readX();
    if (val_Derecho_X > 0) {
        sent_data.ch3 = map(val_Derecho_X, 0, 255, 0, palanca2[readPalanca2Position()]);
        sent_data.ch4 = 0;
        lv_bar_set_value(ui_BarJoystickIzquierdoSup7, val_Derecho_X, LV_ANIM_ON);
        lv_bar_set_start_value(ui_BarJoystickIzquierdoSup8, 255, LV_ANIM_ON);
    } else if (val_Derecho_X < 0) {
        sent_data.ch4 = map(val_Derecho_X, 0, -255, 0, palanca2[readPalanca2Position()]);
        sent_data.ch3 = 0;
        lv_bar_set_value(ui_BarJoystickIzquierdoSup7, 0, LV_ANIM_ON);
        lv_bar_set_start_value(ui_BarJoystickIzquierdoSup8, map(val_Derecho_X, 0, -255, 255, 0), LV_ANIM_ON);
    } else {
        sent_data.ch3 = 0;
        sent_data.ch4 = 0;
        lv_bar_set_value(ui_BarJoystickIzquierdoSup7, 0, LV_ANIM_ON);
        lv_bar_set_start_value(ui_BarJoystickIzquierdoSup8, 255, LV_ANIM_ON);
    }


  
    int val_palanca4 = palanca4[readPalanca4Position()];
    if (val_palanca4 >= 0) {
        sent_data.ch5 = map(val_palanca4, 0, 255, 0, 255);
        sent_data.ch6 = 0;
    } else {
        sent_data.ch5 = 0;
        sent_data.ch6 = map(val_palanca4, -255, 0, 255, 0);
    }
    // Transmisión NRF24
    if (nrf24_available) {
        static unsigned long last_nrf_time = 0;
        if (millis() - last_nrf_time >= 50) {
            
            radio.write(&sent_data, sizeof(Data_to_be_sent));
            last_nrf_time = millis();
        }
    }

    lv_timer_handler(); 
}
