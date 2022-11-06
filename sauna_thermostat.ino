#include <string.h>

#include <avr/wdt.h>



#include <SPFD5408_Adafruit_GFX.h> // Core graphics library
#include <SPFD5408_Adafruit_TFTLCD.h> // Hardware-specific library
#include <SPFD5408_TouchScreen.h>

//#include <ESP_AT_Debug.h>
//#include <ESP_AT_Lib.h>
//#include <ESP_AT_Lib_Impl.h>

#include <EEPROM.h>

#include <arduino-timer.h>

#include "gfxfont.h"
#include "FreeMonoBold24pt7b.h"

#include "sauna.h"
#include "WifiWrap.h"
#include "Screen.h"
#include "ScreenStatus.h"
#include "ScreenConfig.h"
#include "ScreenConfigSsid.h"
#include "ScreenConfigPwd.h"

#include <OneWire.h>
#include <DallasTemperature.h>

const int XP=6,XM=A2,YP=A1,YM=7; //ID=0x9341

// Temperature hysteresis in degrees
#define HYSTERESIS 0.5

// Output to drive Heater
#define HEATING_OUT     24

// Assign human-readable names to some common 16-bit color values:
#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF
#define GRAY50  0x7BEF

#define LCD_CS A3 // Chip Select goes to Analog 3
#define LCD_CD A2 // Command/Data goes to Analog 2
#define LCD_WR A1 // LCD Write goes to Analog 1
#define LCD_RD A0 // LCD Read goes to Analog 0
#define LCD_RESET A4 // LCD RESET goes to Analog 4

#define TS_MINX 73
#define TS_MINY 135
#define TS_MAXX 860
#define TS_MAXY 900

#define MINPRESSURE 200
#define MAXPRESSURE 1000

#define EEPROM_MAX_SIZE 256
#define EEPROM_FREE_VALUE 255

Adafruit_TFTLCD lcd(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);

// For better pressure precision, we need to know the resistance
// between X+ and X- Use any multimeter to read it
// For the one we're using, its 300 ohms across the X plate
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 320);

// Data wire is conntec to the Arduino digital pin 23
#define ONE_WIRE_BUS 28

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&oneWire);

DeviceAddress sensors_addr;

WifiWrap wifi(&Serial1);

Timer<1, millis> timer_shutdown;

Timer<1, millis> timer_sensor;


const unsigned long shutdown_interval = 3l * 60 * 60 * 1000; // 3 hours in miliseconds

uint16_t temp_out = 0;

uint16_t temp = 0; ///< current temperature

uint8_t target = 30;


state_t state;

screen_t screen, screen_last;

ScreenStatus screenStatus(lcd, temp, target, state, screen);
ScreenConfig screenConfig(lcd, wifi, screen);
ScreenConfigSsid screenConfigSsid(lcd, screen);
ScreenConfigPwd screenConfigPwd(lcd, screen);


Screen* currScreen;


bool pressed_last = false;
bool pressed_curr = false;


TSPoint handleTouch()
{
    digitalWrite(13, HIGH);
    TSPoint p = ts.getPoint();
    // swap axis
    int16_t tmp = p.x;
    p.x = p.y;
    p.y = tmp;
    digitalWrite(13, LOW);


    pinMode(XM, OUTPUT);
    digitalWrite(XM, LOW);
    pinMode(YP, OUTPUT);
    digitalWrite(YP, HIGH);

    // backup previous value
    pressed_last = pressed_curr;

    pressed_curr = false;
    p.x = map(p.x, TS_MINX, TS_MAXX, 0, lcd.width());
    p.y = map(p.y, TS_MAXY, TS_MINY, 0, lcd.height());
    if (p.z > MINPRESSURE && p.z < MAXPRESSURE) {
/*
            Serial.print("X = "); Serial.print(p.x);
            Serial.print("\tY = "); Serial.print(p.y);
            Serial.print("\tPressure = "); Serial.println(p.z);
*/
        pressed_curr = true;
    }

    p.z = pressed_last && pressed_curr;

    return p;
}


// Read out current temperature
bool timer_sensor_cb(void* temp)
{
    uint16_t* tmp = (uint16_t*) temp;

    //sensors.requestTemperaturesByAddress(sensors_addr);
    sensors.requestTemperatures();
    *tmp = sensors.getTemp(sensors_addr);
    // multiply by 9% as the sensor was showing 70 degree while the real temperature was 76
    uint32_t aaa = (*tmp);
    *tmp = aaa * 109/100;
    return true; // to repeat the action - false to stop
}


bool timer_shutdown_cb(void*)
{
    Serial.println("Timeout expired. Turning off the heating.");
    state = OFF;
    return false;
}




void loop() {
    wdt_reset();

    timer_sensor.tick();
    timer_shutdown.tick();
    wifi.handleHttpReq(target, temp, state);

    TSPoint p = handleTouch();
    currScreen->handle_buttons(p);

    // State machine
    switch (state) {
        case ON:
            timer_shutdown.in(shutdown_interval, timer_shutdown_cb, NULL);
            // PASS THROUGH
        case HEATING:
            if (temp >= target * 128) {
                state = WAITING;
            } else {
                state = HEATING;
            }
            break;
        case WAITING:
            if (temp < target * 128 - HYSTERESIS * 128) {
                state = HEATING;
            }
            break;
        case OFF:
            timer_shutdown.cancel();
            break;
    }

    // Drive the output signal to the heater
    if (state == HEATING) {
        digitalWrite(HEATING_OUT, HIGH);
    } else {
        digitalWrite(HEATING_OUT, LOW);
    }

    currScreen->tick();

    // Display the whole screen
    if (screen_last != screen) {
        switch(screen) {
            case SCREEN_STATUS:
                currScreen = &screenStatus;
                break;
            case SCREEN_CONFIG:
                currScreen = &screenConfig;
                if (screenConfigPwd.pwdAvailable) {
                    screenConfigPwd.pwdAvailable = false;
                    if (!wifi.joinAp(screenConfigSsid.ssid, screenConfigPwd.pwd)) {
                        Serial.println("Failed to connect to the AP.");
                    }
                }
                break;
            case SCREEN_CONFIG_SSID:
                currScreen = &screenConfigSsid;
                break;
            case SCREEN_CONFIG_PWD:
                currScreen = &screenConfigPwd;
                break;
        }
        currScreen->display();
        screen_last = screen;
    }
}



void setup() {
    // Confirm watchdog  
    wdt_enable(WDTO_8S);


    pinMode(HEATING_OUT, OUTPUT);
    digitalWrite(HEATING_OUT, LOW);

    Serial.begin(9600); // open the serial port at 9600 bps
    Serial1.begin(115200);
    Serial1.setTimeout(1000);
    state = OFF;
    screen = SCREEN_STATUS;
    currScreen = &screenStatus;


    // Init TFT
    uint16_t identifier = lcd.readID();
    if (identifier == 0x0101) identifier = 0x9341;
    lcd.begin(identifier);
    lcd.setRotation(3);
    lcd.fillScreen(BLACK);
    lcd.setTextColor(WHITE);
    lcd.println(F("Booting..."));


/*
    keyboard.draw();

    while(1) {


        TSPoint p = handleTouch();

        keyboard.handlePress(p.x, p.y, p.z);


    }
*/

    // Read the target value from EEPROM
    for (int i = 0; i < EEPROM_MAX_SIZE; i++) {
        uint8_t value = EEPROM.read(i);
        /*
        Serial.print(i);
        Serial.print(' ');
        Serial.println(value);
        */
        if (value != 255) {
            target = value;
        } else {
            break;
        }
    }

    // Init temperature sensor
    sensors.begin();
    sensors.setWaitForConversion(false);
    if (!sensors.getAddress(sensors_addr, 0)) {
        Serial.println(F("ERROR: temperature sensor was not found"));
        lcd.setTextColor(RED);
        lcd.println(F("ERROR: Teplotni cidlo nebylo nenalezeno."));
        lcd.setTextColor(WHITE);
    }

    // monitor temperature every 1000 ms
    timer_sensor.every(1000, timer_sensor_cb, &temp);

    // Test the comminication with WiFi module
    if (!wifi.isAlive()) {
        Serial.println(F("ERROR: WiFi module was not detected"));
        lcd.setTextColor(RED);
        lcd.println(F("ERROR: WiFi modul nebyl detekovan."));
        lcd.setTextColor(WHITE);
    }

    // Start HTTP server
    wifi.startHttpSrv();

    Serial.println("Initialized");

    currScreen->display();
}
