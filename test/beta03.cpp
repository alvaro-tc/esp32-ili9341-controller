#include <SPI.h>
#include <TFT_eSPI.h>    
#include <lvgl.h>
#include <ui.h>  

#include "ConfigStorage.h"
#include <Joystick.h>
ConfigStorage config;
Joystick joystick(2, 5, 4);

#define TFT_LED 38

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
    
    joystick.begin();
    joystick.setDeadZone(80, true);
    joystick.setSmoothing(true, 0.2); 
    joystick.invertAxis(false, true);

    Serial.begin(9600);
    
    if (!config.begin()) {
        return;
    }
    
    uint8_t temp_limits[3];
    getTurnLimits(&temp_limits[0], &temp_limits[1], &temp_limits[2]);
    palanca1[0] = temp_limits[0];
    palanca1[1] = temp_limits[1];
    palanca1[2] = temp_limits[2];
    
    analogWrite(TFT_LED, config.getBrightnessLimit());

    lv_init();
    tft.init();
    tft.setRotation(1);

    uint16_t calData[5] = { 140, 3820, 250, 3600, 7 };
    tft.setTouch(calData);

    lv_disp_draw_buf_init( &draw_buf, buf, NULL, screenWidth * screenHeight / 10 );

    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init( &disp_drv );
    disp_drv.hor_res = screenWidth;
    disp_drv.ver_res = screenHeight;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register( &disp_drv );

    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init( &indev_drv );
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = my_touchpad_read;
    lv_indev_drv_register( &indev_drv );

    ui_init();
}

void loop(void) {
    int rawX = analogRead(8);
    char buf[32];
    snprintf(buf, sizeof(buf), "X: %d", rawY);
    lv_label_set_text(ui_Label1, buf);
    Serial.println(buf);
    lv_timer_handler(); 
}
