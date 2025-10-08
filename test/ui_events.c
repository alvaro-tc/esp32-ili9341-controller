#include "ui.h"
#include <stdio.h>
#include <string.h>

extern bool brightness_calibration_mode;
extern bool nrf24_calibration_mode;
extern bool palanca1_calibration_mode;
extern bool palanca2_calibration_mode;
extern bool palanca3_calibration_mode;
extern bool palanca4_calibration_mode;
extern bool settings_calibration_mode;
extern bool intensidad_calibration_mode;
extern bool canal_calibration_mode; // << añadido

extern void applyBrightness(int brightness_value);
extern uint8_t getBrightnessLimit(void);
extern void setBrightnessLimit(uint8_t value);
extern void saveCurrentConfig(void);
extern uint64_t getNRFAddress(void);
extern void setNRFAddress(uint64_t address);
extern void getTurnLimits(uint8_t* pos1, uint8_t* pos2, uint8_t* pos3);
extern void setTurnLimits(uint8_t pos1, uint8_t pos2, uint8_t pos3);
extern void getSpeedLimits(uint8_t* pos1, uint8_t* pos2, uint8_t* pos3);
extern void setSpeedLimits(uint8_t pos1, uint8_t pos2, uint8_t pos3);
extern void getBoostLimits(uint8_t* pos1, uint8_t* pos2, uint8_t* pos3);
extern void setBoostLimits(uint8_t pos1, uint8_t pos2, uint8_t pos3);
extern void getExtraLimits(uint8_t* pos1, uint8_t* pos2, uint8_t* pos3);
extern void setExtraLimits(uint8_t pos1, uint8_t pos2, uint8_t pos3);
extern void setActiveProfile(uint8_t profile);
extern uint8_t getActiveProfile(void);
extern void setIntensity(uint8_t intensity);
extern uint8_t getIntensityLimit(void);
extern void clearAllProfiles(void);
extern bool repairProfile(uint8_t profile);
extern void updatePalanca1Vector(void);
extern void updatePalanca2Vector(void);
extern void updatePalanca3Vector(void);
extern void updatePalanca4Vector(void);

// getters/setters para índice 13 (extra config / canal)
extern void setExtraConfig(uint8_t value);
extern uint8_t getExtraConfig(void);

extern uint8_t palanca1[3];
extern uint8_t palanca2[3];
extern uint8_t palanca3[3];
extern uint8_t palanca4[3];

static uint8_t original_brightness = 0;
static uint8_t current_brightness = 0;
static uint64_t original_nrf_address = 0;
static uint8_t original_turn_limits[3] = {0, 0, 0};
static uint8_t current_turn_limits[3] = {0, 0, 0};
static uint8_t current_position = 0;
static uint8_t original_speed_limits[3] = {0, 0, 0};
static uint8_t current_speed_limits[3] = {0, 0, 0};
static uint8_t current_direction_position = 0;
static uint8_t original_boost_limits[3] = {0, 0, 0};
static uint8_t current_boost_limits[3] = {0, 0, 0};
static uint8_t current_boost_position = 0;
static uint8_t original_extra_limits[3] = {0, 0, 0};
static uint8_t current_extra_limits[3] = {0, 0, 0};
static uint8_t current_extra_position = 0;
static uint8_t original_active_profile = 0;
static uint8_t original_intensity = 1;

static uint8_t original_canal = 0;   // << añadido
static uint8_t current_canal = 0;    // << añadido

void Button1_event_cb(lv_event_t * e)
{
    if (brightness_calibration_mode) {
        if (current_brightness > 0) {
            current_brightness--;
            applyBrightness(current_brightness);
            
            char brightness_str[4];
            sprintf(brightness_str, "%d", current_brightness);
            lv_textarea_set_text(ui_TextArea3, brightness_str);
        }
    }
    
    if (palanca1_calibration_mode) {
        if (current_turn_limits[current_position] > 0) {
            current_turn_limits[current_position]--;
            
            palanca1[current_position] = current_turn_limits[current_position];
            
            char position_str[4];
            sprintf(position_str, "%d", current_turn_limits[current_position]);
            lv_textarea_set_text(ui_TextArea3, position_str);
        }
    }
    
    if (palanca2_calibration_mode) {
        if (current_speed_limits[current_direction_position] > 0) {
            current_speed_limits[current_direction_position]--;
            
            palanca2[current_direction_position] = current_speed_limits[current_direction_position];
            
            char direction_str[4];
            sprintf(direction_str, "%d", current_speed_limits[current_direction_position]);
            lv_textarea_set_text(ui_TextArea3, direction_str);
        }
    }
    
    if (palanca3_calibration_mode) {
        if (current_boost_limits[current_boost_position] > 0) {
            current_boost_limits[current_boost_position]--;
            
            palanca3[current_boost_position] = current_boost_limits[current_boost_position];
            
            char boost_str[4];
            sprintf(boost_str, "%d", current_boost_limits[current_boost_position]);
            lv_textarea_set_text(ui_TextArea3, boost_str);
        }
    }
    
    if (palanca4_calibration_mode) {
        if (current_extra_limits[current_extra_position] > 0) {
            current_extra_limits[current_extra_position]--;
            
            palanca4[current_extra_position] = current_extra_limits[current_extra_position];
            
            char extra_str[4];
            sprintf(extra_str, "%d", current_extra_limits[current_extra_position]);
            lv_textarea_set_text(ui_TextArea3, extra_str);
        }
    }

    if (canal_calibration_mode) {
        if (current_canal > 0) {
            current_canal--;
            // aplicar visual en UI
            char canal_str[5];
            sprintf(canal_str, "%d", current_canal);
            lv_textarea_set_text(ui_TextArea3, canal_str);
        }
    }
}

void Button2_event_cb(lv_event_t * e)
{
    if (brightness_calibration_mode) {
        if (current_brightness < 255) {
            current_brightness++;
            applyBrightness(current_brightness);
            
            char brightness_str[4];
            sprintf(brightness_str, "%d", current_brightness);
            lv_textarea_set_text(ui_TextArea3, brightness_str);
        }
    }
    
    if (palanca1_calibration_mode) {
        if (current_turn_limits[current_position] < 255) {
            current_turn_limits[current_position]++;
            
            palanca1[current_position] = current_turn_limits[current_position];
            
            char position_str[4];
            sprintf(position_str, "%d", current_turn_limits[current_position]);
            lv_textarea_set_text(ui_TextArea3, position_str);
        }
    }
    
    if (palanca2_calibration_mode) {
        if (current_speed_limits[current_direction_position] < 255) {
            current_speed_limits[current_direction_position]++;
            
            palanca2[current_direction_position] = current_speed_limits[current_direction_position];
            
            char direction_str[4];
            sprintf(direction_str, "%d", current_speed_limits[current_direction_position]);
            lv_textarea_set_text(ui_TextArea3, direction_str);
        }
    }
    
    if (palanca3_calibration_mode) {
        if (current_boost_limits[current_boost_position] < 255) {
            current_boost_limits[current_boost_position]++;
            
            palanca3[current_boost_position] = current_boost_limits[current_boost_position];
            
            char boost_str[4];
            sprintf(boost_str, "%d", current_boost_limits[current_boost_position]);
            lv_textarea_set_text(ui_TextArea3, boost_str);
        }
    }
    
    if (palanca4_calibration_mode) {
        if (current_extra_limits[current_extra_position] < 255) {
            current_extra_limits[current_extra_position]++;
            
            palanca4[current_extra_position] = current_extra_limits[current_extra_position];
            
            char extra_str[4];
            sprintf(extra_str, "%d", current_extra_limits[current_extra_position]);
            lv_textarea_set_text(ui_TextArea3, extra_str);
        }
    }

    if (canal_calibration_mode) {
        if (current_canal < 125) {
            current_canal++;
            char canal_str[5];
            sprintf(canal_str, "%d", current_canal);
            lv_textarea_set_text(ui_TextArea3, canal_str);
        }
    }
}

void calibrar_brillo(lv_event_t * e)
{
    brightness_calibration_mode = true;
    
    lv_label_set_text(ui_Label4, "Calibrar Brillo");
    
    original_brightness = getBrightnessLimit();
    current_brightness = original_brightness;
    
    char brightness_str[4];
    sprintf(brightness_str, "%d", current_brightness);
    lv_textarea_set_text(ui_TextArea3, brightness_str);
}

void calibrate_nrf24(lv_event_t * e)
{
    nrf24_calibration_mode = true;
    
    original_nrf_address = getNRFAddress();
    
    lv_obj_t * ta = ui_TextArea1;   
    lv_obj_t * kb = ui_Keyboard1;   

    lv_keyboard_set_textarea(kb, ta);
    
    char nrf_str[20];
    sprintf(nrf_str, "0x%010llX", original_nrf_address);
    
    lv_textarea_set_text(ta, nrf_str);
    
    lv_textarea_set_accepted_chars(ta, "0123456789ABCDEFabcdefxX");
    
    lv_textarea_set_cursor_pos(ta, LV_TEXTAREA_CURSOR_LAST);
}

void calibrate_joystick(lv_event_t * e)
{
    palanca1_calibration_mode = true;
    
    getTurnLimits(&original_turn_limits[0], &original_turn_limits[1], &original_turn_limits[2]);
    
    current_turn_limits[0] = original_turn_limits[0];
    current_turn_limits[1] = original_turn_limits[1];
    current_turn_limits[2] = original_turn_limits[2];
}

void calibrar_direccion(lv_event_t * e)
{
    palanca2_calibration_mode = true;
    
    getSpeedLimits(&original_speed_limits[0], &original_speed_limits[1], &original_speed_limits[2]);
    
    current_speed_limits[0] = original_speed_limits[0];
    current_speed_limits[1] = original_speed_limits[1];
    current_speed_limits[2] = original_speed_limits[2];
}

void calibrate_boost(lv_event_t * e)
{
    palanca3_calibration_mode = true;
    
    getBoostLimits(&original_boost_limits[0], &original_boost_limits[1], &original_boost_limits[2]);
    
    current_boost_limits[0] = original_boost_limits[0];
    current_boost_limits[1] = original_boost_limits[1];
    current_boost_limits[2] = original_boost_limits[2];
}

void calibrate_extra(lv_event_t * e)
{
    palanca4_calibration_mode = true;
    
    getExtraLimits(&original_extra_limits[0], &original_extra_limits[1], &original_extra_limits[2]);
    
    current_extra_limits[0] = original_extra_limits[0];
    current_extra_limits[1] = original_extra_limits[1];
    current_extra_limits[2] = original_extra_limits[2];
}

void calibrar_settings(lv_event_t * e)
{
    settings_calibration_mode = true;
    
    original_active_profile = getActiveProfile();
    
    lv_label_set_text(ui_Label23, "Seleccionar Perfil");
    char label_str[32];
    sprintf(label_str, "Perfil %d activo", original_active_profile + 1);
    lv_label_set_text(ui_Label23, label_str);
}

void calibrate_settings1(lv_event_t * e)
{
    if (settings_calibration_mode) {
        setActiveProfile(0);
        
        updatePalanca1Vector();
        updatePalanca2Vector();
        updatePalanca3Vector();
        updatePalanca4Vector();
        
        settings_calibration_mode = false;
        
        lv_label_set_text(ui_Label23, "Perfil 1 Activado");
        lv_textarea_set_text(ui_TextArea3, "1");
    }
}

void calibrate_settings2(lv_event_t * e)
{
    if (settings_calibration_mode) {
        setActiveProfile(1);
        
        updatePalanca1Vector();
        updatePalanca2Vector();
        updatePalanca3Vector();
        updatePalanca4Vector();
        
        settings_calibration_mode = false;
        
        lv_label_set_text(ui_Label23, "Perfil 2 Activado");
        lv_textarea_set_text(ui_TextArea3, "2");
    }
}

void calibrate_settings3(lv_event_t * e)
{
    if (settings_calibration_mode) {
        setActiveProfile(2);
        
        updatePalanca1Vector();
        updatePalanca2Vector();
        updatePalanca3Vector();
        updatePalanca4Vector();
        
        settings_calibration_mode = false;
        
        lv_label_set_text(ui_Label23, "Perfil 3 Activado");
        lv_textarea_set_text(ui_TextArea3, "3");
    }
}

void calibrate_settings4(lv_event_t * e)
{
    if (settings_calibration_mode) {
        setActiveProfile(3);
        
        updatePalanca1Vector();
        updatePalanca2Vector();
        updatePalanca3Vector();
        updatePalanca4Vector();
        
        settings_calibration_mode = false;
        
        lv_label_set_text(ui_Label23, "Perfil 4 Activado");
        lv_textarea_set_text(ui_TextArea3, "4");
    }
}

void calibrate_posicion1(lv_event_t * e)
{
    if (palanca1_calibration_mode) {
        current_position = 0;
        
        lv_label_set_text(ui_Label4, "Calibracion Posicion 1");
        
        char position_str[4];
        sprintf(position_str, "%d", palanca1[0]);
        lv_textarea_set_text(ui_TextArea3, position_str);
        
        current_turn_limits[0] = palanca1[0];
    }
    
    if (palanca2_calibration_mode) {
        current_direction_position = 0;
        
        lv_label_set_text(ui_Label4, "Calibracion Direccion 1");
        
        char direction_str[4];
        sprintf(direction_str, "%d", current_speed_limits[0]);
        lv_textarea_set_text(ui_TextArea3, direction_str);
    }
    
    if (palanca3_calibration_mode) {
        current_boost_position = 0;
        
        lv_label_set_text(ui_Label4, "Calibracion Boost 1");
        
        char boost_str[4];
        sprintf(boost_str, "%d", current_boost_limits[0]);
        lv_textarea_set_text(ui_TextArea3, boost_str);
    }
    
    if (palanca4_calibration_mode) {
        current_extra_position = 0;
        
        lv_label_set_text(ui_Label4, "Calibracion Extra 1");
        
        char extra_str[4];
        sprintf(extra_str, "%d", current_extra_limits[0]);
        lv_textarea_set_text(ui_TextArea3, extra_str);
    }
}

void calibrate_posicion2(lv_event_t * e)
{
    if (palanca1_calibration_mode) {
        current_position = 1;
        
        lv_label_set_text(ui_Label4, "Calibracion Posicion 2");
        
        char position_str[4];
        sprintf(position_str, "%d", palanca1[1]);
        lv_textarea_set_text(ui_TextArea3, position_str);
        
        current_turn_limits[1] = palanca1[1];
    }
    
    if (palanca2_calibration_mode) {
        current_direction_position = 1;
        
        lv_label_set_text(ui_Label4, "Calibracion Direccion 2");
        
        char direction_str[4];
        sprintf(direction_str, "%d", current_speed_limits[1]);
        lv_textarea_set_text(ui_TextArea3, direction_str);
    }
    
    if (palanca3_calibration_mode) {
        current_boost_position = 1;
        
        lv_label_set_text(ui_Label4, "Calibracion Boost 2");
        
        char boost_str[4];
        sprintf(boost_str, "%d", current_boost_limits[1]);
        lv_textarea_set_text(ui_TextArea3, boost_str);
    }
    
    if (palanca4_calibration_mode) {
        current_extra_position = 1;
        
        lv_label_set_text(ui_Label4, "Calibracion Extra 2");
        
        char extra_str[4];
        sprintf(extra_str, "%d", current_extra_limits[1]);
        lv_textarea_set_text(ui_TextArea3, extra_str);
    }
}

void calibrate_posicion3(lv_event_t * e)
{
    if (palanca1_calibration_mode) {
        current_position = 2;
        
        lv_label_set_text(ui_Label4, "Calibracion Posicion 3");
        
        char position_str[4];
        sprintf(position_str, "%d", palanca1[2]);
        lv_textarea_set_text(ui_TextArea3, position_str);
        
        current_turn_limits[2] = palanca1[2];
    }
    
    if (palanca2_calibration_mode) {
        current_direction_position = 2;
        
        lv_label_set_text(ui_Label4, "Calibracion Direccion 3");
        
        char direction_str[4];
        sprintf(direction_str, "%d", current_speed_limits[2]);
        lv_textarea_set_text(ui_TextArea3, direction_str);
    }
    
    if (palanca3_calibration_mode) {
        current_boost_position = 2;
        
        lv_label_set_text(ui_Label4, "Calibracion Boost 3");
        
        char boost_str[4];
        sprintf(boost_str, "%d", current_boost_limits[2]);
        lv_textarea_set_text(ui_TextArea3, boost_str);
    }
    
    if (palanca4_calibration_mode) {
        current_extra_position = 2;
        
        lv_label_set_text(ui_Label4, "Calibracion Extra 3");
        
        char extra_str[4];
        sprintf(extra_str, "%d", current_extra_limits[2]);
        lv_textarea_set_text(ui_TextArea3, extra_str);
    }
}

void touch_calibrate(lv_event_t * e)
{
    static uint32_t press_time = 0;
    static bool pressed = false;
    static uint8_t debug_counter = 0;
    
    lv_event_code_t code = lv_event_get_code(e);
    
    if (code == LV_EVENT_PRESSED) {
        press_time = lv_tick_get();
        pressed = true;
        lv_label_set_text(ui_Label4, "Diagnosticando...");
    }
    else if (code == LV_EVENT_RELEASED) {
        uint32_t hold_time = lv_tick_get() - press_time;
        
        if (pressed && hold_time > 3000) {
            lv_label_set_text(ui_Label4, "LIMPIANDO PERFILES!");
            clearAllProfiles();
            lv_label_set_text(ui_Label4, "PERFILES LIMPIADOS");
            lv_textarea_set_text(ui_TextArea3, "RESTART");
        }
        else if (pressed && hold_time > 1000) {
            debug_counter++;
            
            setBrightnessLimit(50 + (debug_counter * 10));
            setIntensity((debug_counter % 4) + 1);
            
            saveCurrentConfig();
            
            char debug_msg[50];
            sprintf(debug_msg, "TEST %d - B:%d I:%d", debug_counter, 
                   getBrightnessLimit(), getIntensityLimit());
            lv_label_set_text(ui_Label4, debug_msg);
            
            char counter_str[10];
            sprintf(counter_str, "%d", debug_counter);
            lv_textarea_set_text(ui_TextArea3, counter_str);
        }
        else {
            uint8_t profile = getActiveProfile();
            uint8_t brightness = getBrightnessLimit();
            uint8_t intensity = getIntensityLimit();
            
            char info_msg[50];
            sprintf(info_msg, "P:%d B:%d I:%d", profile, brightness, intensity);
            lv_label_set_text(ui_Label4, info_msg);
            
            char profile_str[10];
            sprintf(profile_str, "%d", profile);
            lv_textarea_set_text(ui_TextArea3, profile_str);
        }
        pressed = false;
    }
}

void cancelar_cambios(lv_event_t * e)
{
    if (brightness_calibration_mode) {
        current_brightness = original_brightness;
        applyBrightness(original_brightness);
        
        char brightness_str[4];
        sprintf(brightness_str, "%d", original_brightness);
        lv_textarea_set_text(ui_TextArea3, brightness_str);
        
        brightness_calibration_mode = false;
    }
    
    if (nrf24_calibration_mode) {
        char nrf_str[20];
        sprintf(nrf_str, "0x%010llX", original_nrf_address);
        lv_textarea_set_text(ui_TextArea1, nrf_str);
        
        nrf24_calibration_mode = false;
    }
    
    if (palanca1_calibration_mode) {
        current_turn_limits[0] = original_turn_limits[0];
        current_turn_limits[1] = original_turn_limits[1];
        current_turn_limits[2] = original_turn_limits[2];
        
        palanca1[0] = original_turn_limits[0];
        palanca1[1] = original_turn_limits[1];
        palanca1[2] = original_turn_limits[2];
        
        char position_str[4];
        sprintf(position_str, "%d", palanca1[current_position]);
        lv_textarea_set_text(ui_TextArea3, position_str);
        
        palanca1_calibration_mode = false;
    }
    
    if (palanca2_calibration_mode) {
        current_speed_limits[0] = original_speed_limits[0];
        current_speed_limits[1] = original_speed_limits[1];
        current_speed_limits[2] = original_speed_limits[2];
        
        palanca2[0] = original_speed_limits[0];
        palanca2[1] = original_speed_limits[1];
        palanca2[2] = original_speed_limits[2];
        
        char direction_str[4];
        sprintf(direction_str, "%d", palanca2[current_direction_position]);
        lv_textarea_set_text(ui_TextArea3, direction_str);
        
        palanca2_calibration_mode = false;
    }
    
    if (palanca3_calibration_mode) {
        current_boost_limits[0] = original_boost_limits[0];
        current_boost_limits[1] = original_boost_limits[1];
        current_boost_limits[2] = original_boost_limits[2];
        
        palanca3[0] = original_boost_limits[0];
        palanca3[1] = original_boost_limits[1];
        palanca3[2] = original_boost_limits[2];
        
        char boost_str[4];
        sprintf(boost_str, "%d", palanca3[current_boost_position]);
        lv_textarea_set_text(ui_TextArea3, boost_str);
        
        palanca3_calibration_mode = false;
    }
    
    if (palanca4_calibration_mode) {
        current_extra_limits[0] = original_extra_limits[0];
        current_extra_limits[1] = original_extra_limits[1];
        current_extra_limits[2] = original_extra_limits[2];
        
        palanca4[0] = original_extra_limits[0];
        palanca4[1] = original_extra_limits[1];
        palanca4[2] = original_extra_limits[2];
        
        char extra_str[4];
        sprintf(extra_str, "%d", palanca4[current_extra_position]);
        lv_textarea_set_text(ui_TextArea3, extra_str);
        
        palanca4_calibration_mode = false;
    }
    
    if (intensidad_calibration_mode) {
        setIntensity(original_intensity);
        
        char intensity_str[4];
        sprintf(intensity_str, "%d", original_intensity);
        lv_textarea_set_text(ui_TextArea3, intensity_str);
        
        intensidad_calibration_mode = false;
    }
    
    if (canal_calibration_mode) {
        // restaurar valor original
        setExtraConfig(original_canal);
        char canal_str[5];
        sprintf(canal_str, "%d", original_canal);
        lv_textarea_set_text(ui_TextArea3, canal_str);
        canal_calibration_mode = false;
    }

    if (settings_calibration_mode) {
        setActiveProfile(original_active_profile);
        
        char profile_str[4];
        sprintf(profile_str, "%d", original_active_profile);
        lv_textarea_set_text(ui_TextArea3, profile_str);
        
        settings_calibration_mode = false;
    }
}

void guardarCambios(lv_event_t * e)
{
    if (brightness_calibration_mode) {
        setBrightnessLimit(current_brightness);
        saveCurrentConfig();
        
        brightness_calibration_mode = false;
    }
    
    if (nrf24_calibration_mode) {
        const char* nrf_text = lv_textarea_get_text(ui_TextArea1);
        
        uint64_t new_nrf_address = 0;
        if (nrf_text != NULL && strlen(nrf_text) > 2) {
            if (nrf_text[0] == '0' && (nrf_text[1] == 'x' || nrf_text[1] == 'X')) {
                sscanf(nrf_text, "%llx", &new_nrf_address);
            } else {
                sscanf(nrf_text, "%llx", &new_nrf_address);
            }
        }
        
        if (new_nrf_address != 0) {
            setNRFAddress(new_nrf_address);
            saveCurrentConfig();
        }
        
        nrf24_calibration_mode = false;
    }
    
    if (palanca1_calibration_mode) {
        setTurnLimits(palanca1[0], palanca1[1], palanca1[2]);
        saveCurrentConfig();
        
        original_turn_limits[0] = palanca1[0];
        original_turn_limits[1] = palanca1[1];
        original_turn_limits[2] = palanca1[2];
        
        palanca1_calibration_mode = false;
    }
    
    if (palanca2_calibration_mode) {
        setSpeedLimits(palanca2[0], palanca2[1], palanca2[2]);
        saveCurrentConfig();
        
        original_speed_limits[0] = palanca2[0];
        original_speed_limits[1] = palanca2[1];
        original_speed_limits[2] = palanca2[2];
        
        palanca2_calibration_mode = false;
    }
    
    if (palanca3_calibration_mode) {
        setBoostLimits(palanca3[0], palanca3[1], palanca3[2]);
        saveCurrentConfig();
        
        original_boost_limits[0] = palanca3[0];
        original_boost_limits[1] = palanca3[1];
        original_boost_limits[2] = palanca3[2];
        
        palanca3_calibration_mode = false;
    }
    
    if (palanca4_calibration_mode) {
        setExtraLimits(palanca4[0], palanca4[1], palanca4[2]);
        saveCurrentConfig();
        
        original_extra_limits[0] = palanca4[0];
        original_extra_limits[1] = palanca4[1];
        original_extra_limits[2] = palanca4[2];
        
        palanca4_calibration_mode = false;
    }
    
    if (canal_calibration_mode) {
        // asegurar rango y guardar
        if (current_canal > 125) current_canal = 125;
        setExtraConfig(current_canal);
        saveCurrentConfig();
        canal_calibration_mode = false;
    }

    if (settings_calibration_mode) {
        saveCurrentConfig();
        
        settings_calibration_mode = false;
    }

    if (intensidad_calibration_mode) {
        saveCurrentConfig();
        
        intensidad_calibration_mode = false;
    }
}

void calibrar_intensidad(lv_event_t * e)
{
    intensidad_calibration_mode = true;
    
    original_intensity = getIntensityLimit();
    
    char intensity_label[32];
    switch(original_intensity) {
        case 1:
            sprintf(intensity_label, "Intensidad Actual: Minimo");
            break;
        case 2:
            sprintf(intensity_label, "Intensidad Actual: Moderado");
            break;
        case 3:
            sprintf(intensity_label, "Intensidad Actual: Normal");
            break;
        case 4:
            sprintf(intensity_label, "Intensidad Actual: Maximo");
            break;
        default:
            sprintf(intensity_label, "Intensidad Actual: Minimo");
            break;
    }
    lv_label_set_text(ui_Label24, intensity_label);
}

void calibrate_intensidad1(lv_event_t * e)
{
    if (intensidad_calibration_mode) {
        setIntensity(1);
        saveCurrentConfig();
        
        intensidad_calibration_mode = false;
        
        lv_label_set_text(ui_Label24, "Intensidad Actual: Minimo");
    }
}

void calibrate_intensidad2(lv_event_t * e)
{
    if (intensidad_calibration_mode) {
        setIntensity(2);
        saveCurrentConfig();
        
        intensidad_calibration_mode = false;
        
        lv_label_set_text(ui_Label24, "Intensidad Actual: Moderado");
    }
}

void calibrate_intensidad3(lv_event_t * e)
{
    if (intensidad_calibration_mode) {
        setIntensity(3);
        saveCurrentConfig();
        
        intensidad_calibration_mode = false;
        
        lv_label_set_text(ui_Label24, "Intensidad Actual: Normal");
    }
}

void calibrate_intensidad4(lv_event_t * e)
{
    if (intensidad_calibration_mode) {
        setIntensity(4);
        saveCurrentConfig();
        
        intensidad_calibration_mode = false;
        
        lv_label_set_text(ui_Label24, "Intensidad Actual: Maximo");
    }
}

void salir_configuracion(lv_event_t * e)
{
    // Cancelar todos los modos de calibración
    brightness_calibration_mode = false;
    nrf24_calibration_mode = false;
    palanca1_calibration_mode = false;
    palanca2_calibration_mode = false;
    palanca3_calibration_mode = false;
    palanca4_calibration_mode = false;
    settings_calibration_mode = false;
    intensidad_calibration_mode = false;
    canal_calibration_mode = false; // << añadido
    
    // Aquí puedes agregar código para cambiar de pantalla o cerrar la configuración
    // Por ejemplo, si tienes una función para ir a la pantalla principal:
    // lv_scr_load(ui_Screen1);
    
    // O simplemente mostrar un mensaje

}