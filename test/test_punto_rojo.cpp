#include <SPI.h>
#include <TFT_eSPI.h>    
#include <lvgl.h>
#include <ui.h>  
#define TFT_LED 38


TFT_eSPI tft = TFT_eSPI(); 
void touch_calibrate();

static const uint16_t screenWidth  = 320;
static const uint16_t screenHeight = 240;
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[ screenWidth * screenHeight / 10 ];
/*
SPI GENERAL
MOSI = 11
MISO = 12
SCK = 13

NRF24L01
#define NRF24_CE 6
#define NRF24_SCK 13
#define NRF24_MISO 12
#define NRF24_CNS 7
#define NRF24_MOSI 11

ILI9341 
#define TFT_CS   15    // Chip Select display
#define TFT_MOSI 35    // SDI / T_DIN
#define TFT_SCLK 36    // SCK / T_CLK
#define TFT_MISO 37    // SDO / T_DO
#define TFT_DC   33    // Data/Command
#define TFT_RST  34    // Reset

TOUCH ILI9341
#define TOUCH_CS 18    // Chip Select touch
#define TOUCH_IRQ 21   // Interrupción táctil opcional

JOYSTICK IZQUIERDO
#define JOYSTICK_1_X 2
#define JOYSTICK_1_Y 5
#define JOYSTICK_1_BTN 4

JOYSTICK DERECHO
#define JOYSTICK_2_X 9
#define JOYSTICK_2_Y 8
#define JOYSTICK_2_BTN 10

PALANCAS INFERIORES
#define DIGITAL_LEVER_INF_1_R 11
#define DIGITAL_LEVER_INF_1_L 12
#define DIGITAL_LEVER_INF_2_R 13
#define DIGITAL_LEVER_INF_2_L 14

PALANCAS SUPERIORES
#define DIGITAL_LEVER_SUP_1_R 16
#define DIGITAL_LEVER_SUP_1_L 17
#define DIGITAL_LEVER_SUP_2_R 39
#define DIGITAL_LEVER_SUP_2_L 1
*/



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

        /*Set the coordinates*/
        data->point.x = touchX;
        data->point.y = touchY;

        Serial.print( "Data x " );
        Serial.println( touchX );

        Serial.print( "Data y " );
        Serial.println( touchY );
    }
}




void setup() {
  pinMode(TFT_LED, OUTPUT);
  digitalWrite(TFT_LED, HIGH); 
  
  Serial.begin(9600);


  lv_init();
  tft.init();
  tft.setRotation(1);

  uint16_t calData[5] = { 140, 3820, 250, 3600, 7 };
  tft.setTouch(calData);

  lv_disp_draw_buf_init( &draw_buf, buf, NULL, screenWidth * screenHeight / 10 );

    /*Initialize the display*/
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init( &disp_drv );
    /*Change the following line to your display resolution*/
    disp_drv.hor_res = screenWidth;
    disp_drv.ver_res = screenHeight;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register( &disp_drv );

    /*Initialize the (dummy) input device driver*/
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init( &indev_drv );
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = my_touchpad_read;
    lv_indev_drv_register( &indev_drv );


    ui_init();
}

//------------------------------------------------------------------------------------------

void loop(void) {
    lv_timer_handler(); 
    uint16_t x = 0, y = 0; 
    // Pressed will be set true is there is a valid touch on the screen
    bool pressed = tft.getTouch(&x, &y);


    if (pressed) {
    tft.fillCircle(x, y, 2, TFT_RED);

  }
}

//------------------------------------------------------------------------------------------

// Code to run a screen calibration, not needed when calibration values set in setup()
void touch_calibrate()
{
  static uint16_t calData[5];  
  uint8_t calDataOK = 0;
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(20, 0);
  tft.setTextFont(2);
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.println("Touch corners as indicated");
  tft.setTextFont(1);
  tft.println();
  tft.calibrateTouch(calData, TFT_MAGENTA, TFT_BLACK, 15);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.setTouch(calData);// Devolver el puntero al array
}