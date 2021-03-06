#include <string.h>

#include <SPFD5408_Adafruit_GFX.h> // Core graphics library
#include <SPFD5408_Adafruit_TFTLCD.h> // Hardware-specific library
#include <SPFD5408_TouchScreen.h>

#include <EEPROM.h>

#include <arduino-timer.h>

#include "gfxfont.h"
#include "FreeMonoBold24pt7b.h"

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

#define TS_MINX 160
#define TS_MINY 120
#define TS_MAXX 920
#define TS_MAXY 850

#define MINPRESSURE 100
#define MAXPRESSURE 1000

#define EEPROM_MAX_SIZE 256
#define EEPROM_FREE_VALUE 255

Adafruit_TFTLCD lcd(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);

// For better pressure precision, we need to know the resistance
// between X+ and X- Use any multimeter to read it
// For the one we're using, its 300 ohms across the X plate
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

// Data wire is conntec to the Arduino digital pin 23
#define ONE_WIRE_BUS 28

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&oneWire);

DeviceAddress sensors_addr;

Timer<1, millis> timer_shutdown;

Timer<1, millis> timer_sensor;

Adafruit_GFX_Button btn_plus;
Adafruit_GFX_Button btn_minus;
Adafruit_GFX_Button btn_on;

Timer<1, millis> timer_btn;

const unsigned long shutdown_interval = 3l * 60 * 60 * 1000; // 3 hours in miliseconds

uint16_t temp_out = 0;

uint16_t temp = 0; ///< current temperature
uint16_t last_temp = 0; ///<last temperature

uint8_t target = 30;
uint8_t last_target = 0;

char last_temp_str[10] = {0};

enum {
    ON,
    OFF,
    HEATING,
    WAITING,
} state, last_state;


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
    return true;
}


// Store the thetarget temperature to the EEPROM
bool timer_btm_cb(void* temp)
{
    uint8_t* tmp = (uint8_t*)temp;
    bool found = false;
    int addr = 0;

    // try to find the free space
    for (addr = 0; addr < EEPROM_MAX_SIZE; addr++) {
        uint8_t data = EEPROM.read(addr);
        if (data == EEPROM_FREE_VALUE) {
            found = true;
            break;
        }
    }

    // was the free space found?
    if (found == false) {
        Serial.println("EEPROM is full. We need to clear it");
        for (addr = 0; addr < EEPROM_MAX_SIZE; addr++) {
            EEPROM.write(addr, EEPROM_FREE_VALUE);
        }
        addr = 0;
    }

    // store the settings
    EEPROM.write(addr, *tmp);

    Serial.print("Save to EEPROM addr: ");
    Serial.print(addr);
    Serial.print(" value: ");
    Serial.println(*tmp);

    Serial.println(*tmp);

    return false;
}


uint16_t draw_char(uint16_t x, uint16_t y, uint8_t size, uint16_t color, const GFXfont* font, char c)
{
    y += 29 * size;

    c -= font->first;

    GFXglyph *glyph = &font->glyph[c];
    uint8_t *bitmap = font->bitmap;

    uint16_t bo = pgm_read_word(&glyph->bitmapOffset);
    uint8_t w = pgm_read_byte(&glyph->width);
    uint8_t h = pgm_read_byte(&glyph->height);
    int8_t xo = pgm_read_byte(&glyph->xOffset);
    int8_t yo = pgm_read_byte(&glyph->yOffset);
    uint8_t xx, yy, bits = 0, bit = 0;
    int16_t xo16 = 0, yo16 = 0;

    uint8_t size_x = size;
    uint8_t size_y = size;

    if (size_x > 1 || size_y > 1) {
        xo16 = xo;
        yo16 = yo;
    }

    for (yy = 0; yy < h; yy++) {
        for (xx = 0; xx < w; xx++) {
            if (!(bit++ & 7)) {
                bits = pgm_read_byte(&bitmap[bo++]);
            }
            if (bits & 0x80) {
                if (size_x == 1 && size_y == 1) {
                    lcd.drawPixel(x + xo + xx, y + yo + yy, color);
                } else {
                    lcd.fillRect(x + (xo16 + xx) * size_x, y + (yo16 + yy) * size_y,
                            size_x, size_y, color);
                }
            }
            bits <<= 1;
        }
    }

    return pgm_read_byte(&glyph->xAdvance) * size;
}


void print_temperature(uint16_t temp)
{
    // convert raw temperature to fixed point Celsius
    uint8_t div = temp / 128;
    uint8_t mod = (temp % 128) * 10 / 128; // we care only about one digit
 
    // convert to string
    char buff[10];
    if (div < 100) {
        snprintf(buff, 10, "%u.%u", div, mod);
    } else {
        snprintf(buff, 10, "%u", div);
    }
    Serial.print("Temperature: ");
    Serial.println(buff);

    uint16_t color;
    switch (state) {
        case HEATING:
            color = RED;
            break;
        case WAITING:
            color = GREEN;
            break;
        default:
            color = WHITE;
            break;
    }

    uint16_t offset_x = 20;
    uint16_t offset_y = 10;
    uint8_t size = 3;
    const GFXfont* font = &FreeMonoBold24pt7b;
    for (int i = 0; buff[i] != 0; i++) {
        if (buff[i] != last_temp_str[i]) {
            if (last_temp_str[i] >= ' ') {
                if (last_temp_str[i] == '.') {
                    // clear the rest of the line, because , has different width than the rest
                    lcd.fillRect(offset_x, offset_y, 320 - offset_x, pgm_read_byte(&font->yAdvance) * size, BLACK);
                } else {
                    lcd.fillRect(offset_x, offset_y, pgm_read_byte(&font->glyph[last_temp_str[i] - font->first].xAdvance) * size, pgm_read_byte(&font->yAdvance) * size, BLACK);
                }
            }
            last_temp_str[i] = buff[i];
            
        }
        offset_x += draw_char(offset_x, offset_y, size, color, font, buff[i]);
    }
}


void print_target() {
    lcd.fillRect(73, 185, 84, 35, BLACK);

    char buff[10];

    snprintf(buff, 10, "%d", target);
    uint16_t offset = 85;
    if (target >= 100) {
        offset = 73;
    }
    for (int i = 0; buff[i] != 0; i++) {
        offset += draw_char(offset, 185, 1, WHITE, &FreeMonoBold24pt7b, buff[i]);
    }

/*
    if (target < 100) {
        lcd.setCursor(90, 185);
    } else {
        lcd.setCursor(80, 185);
    }
    lcd.setTextColor(WHITE);
    lcd.setTextSize(4);

    lcd.print(target);
*/
}


void handle_buttons(void) {
    digitalWrite(13, HIGH);
    TSPoint p = ts.getPoint();
    digitalWrite(13, LOW);

    pinMode(XM, OUTPUT);
    digitalWrite(XM, LOW);
    pinMode(YP, OUTPUT);
    digitalWrite(YP, HIGH);


    if (p.z > MINPRESSURE && p.z < MAXPRESSURE) {
        p.x = map(p.x, TS_MINX, TS_MAXX, 0, lcd.width());
        p.y = map(p.y, TS_MINY, TS_MAXY, 0, lcd.height());

        Serial.print("X = "); Serial.print(p.x);
        Serial.print("\tY = "); Serial.print(p.y);
        Serial.print("\tPressure = "); Serial.println(p.z);

        if (btn_plus.contains(p.x, p.y)) {
            btn_plus.press(true);
            timer_btn.cancel();
            timer_btn.in(5000, timer_btm_cb, &target);
        }
        if (btn_minus.contains(p.x, p.y)) {
            btn_minus.press(true);
            timer_btn.cancel();
            timer_btn.in(5000, timer_btm_cb, &target);
        }
        if (btn_on.contains(p.x, p.y)) {
            btn_on.press(true);
        }
    } else {
        btn_plus.press(false);
        btn_minus.press(false);
        btn_on.press(false);
    }

    if (btn_plus.justPressed()) {
        target++;
        btn_plus.drawButton(true);
    } else if (btn_plus.justReleased()) {
        btn_plus.drawButton(false);
    }

    if (btn_minus.justPressed()) {
        target--;
        btn_minus.drawButton(true);
    } else if (btn_minus.justReleased()) {
        btn_minus.drawButton(false);
    }

    if (btn_on.justPressed()) {
        switch (state) {
            case OFF:
                btn_on.drawButton(true);
                state = ON;
                timer_shutdown.in(shutdown_interval, timer_shutdown_cb, NULL);
                break;
            default:
                btn_on.drawButton(false);
                state = OFF;
                timer_shutdown.cancel();
                break;
        }
    }

}


void loop() {
    timer_sensor.tick();
    timer_btn.tick();
    timer_shutdown.tick();

    handle_buttons();

    // State machine
    switch (state) {
        case ON:
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
    }

    // Drive the output signal to the heater
    if (state == HEATING) {
        digitalWrite(HEATING_OUT, HIGH);
    } else {
        digitalWrite(HEATING_OUT, LOW);
    }

    // Update displayed temperature
    if (last_state != state || temp != last_temp) {
        last_temp = temp;
        print_temperature(temp);
    }

    // Update displayed target temperature
    if (target != last_target) {
        last_target = target;
        print_target();
    }

    last_state = state;
}


void setup() {
    pinMode(HEATING_OUT, OUTPUT);
    digitalWrite(HEATING_OUT, LOW);

    Serial.begin(9600); // open the serial port at 9600 bps:

    last_state = OFF;
    state = OFF;

    // Init TFT
    uint16_t identifier = lcd.readID();
    if (identifier == 0x0101) identifier = 0x9341;
    lcd.begin(identifier);
    lcd.setRotation(3);
    lcd.fillScreen(BLACK);

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
        Serial.println("device was not found");
    }

    // monitor temperature every 1000 ms
    timer_sensor.every(1000, timer_sensor_cb, &temp);

    Serial.println("Initialized");


    btn_minus.initButton(&lcd, 40, 200, 60, 60, WHITE, BLACK, WHITE, "-", 4);
    btn_minus.drawButton();

    btn_plus.initButton(&lcd, 190, 200, 60, 60, WHITE, BLACK, WHITE, "+", 4);
    btn_plus.drawButton();

    btn_on.initButton(&lcd, 280, 200, 60, 60, WHITE, BLACK, WHITE, "ON", 3);
    btn_on.drawButton(false);
}
