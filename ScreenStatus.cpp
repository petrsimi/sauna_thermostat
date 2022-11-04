#include "ScreenStatus.h"
#include <EEPROM.h>
#include "FreeMonoBold24pt7b.h"


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


#define EEPROM_MAX_SIZE 256
#define EEPROM_FREE_VALUE 255

static bool timer_btm_cb(void* temp)
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


ScreenStatus::ScreenStatus(Adafruit_TFTLCD& lcd, uint16_t& temp, uint8_t& target, state_t& state, screen_t& screen) :
   lcd(lcd), temp(temp), target(target), state(state), screen(screen)
{
    target_last = 0;
    memset(last_temp_str, 0, sizeof(last_temp_str));
}

void ScreenStatus::display()
{
    lcd.fillScreen(BLACK);

    btn_minus.initButton(&lcd, 40, 210, 60, 60, WHITE, BLACK, WHITE, "-", 4);
    btn_minus.drawButton();

    btn_plus.initButton(&lcd, 190, 210, 60, 60, WHITE, BLACK, WHITE, "+", 4);
    btn_plus.drawButton();

    btn_on.initButton(&lcd, 280, 210, 60, 60, WHITE, BLACK, WHITE, "ON", 3);
    btn_on.drawButton(state != OFF);

    btn_cfg.initButton(&lcd, 280, 150, 60, 40, WHITE, BLACK, WHITE, "Conf", 2);
    btn_cfg.drawButton(false);

    print_target();
    print_temperature();
}


uint16_t ScreenStatus::draw_char(uint16_t x, uint16_t y, uint8_t size, uint16_t color, const GFXfont* font, char c)
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


void ScreenStatus::print_temperature()
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


void ScreenStatus::print_target() {
    lcd.fillRect(73, 195, 84, 35, BLACK);

    char buff[10];

    snprintf(buff, 10, "%d", target);
    uint16_t offset = 85;
    if (target >= 100) {
        offset = 73;
    }
    for (int i = 0; buff[i] != 0; i++) {
        offset += draw_char(offset, 195, 1, WHITE, &FreeMonoBold24pt7b, buff[i]);
    }
}


void ScreenStatus::tick()
{
    timer_btn.tick();

    // Refresh ON/OFF button
    if (state != state_last && (state == OFF || state_last == OFF)) {
        btn_on.drawButton(state != OFF);
    }

    // Update displayed target temperature
    if (target != target_last) {
        target_last = target;
        print_target();
    }

    // Update displayed temperature
    if (state_last != state || temp != temp_last) {
        temp_last = temp;
        state_last = state;
        print_temperature();
    }

}

void ScreenStatus::handle_buttons(TSPoint& p)
{
    if (p.z != 0) {
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
        if (btn_cfg.contains(p.x, p.y)) {
            btn_cfg.press(true);
        }
    } else {
        btn_plus.press(false);
        btn_minus.press(false);
        btn_on.press(false);
        btn_cfg.press(false);
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
                state = ON;
                break;
            default:
                state = OFF;
                break;
        }
    }

    if (btn_cfg.justPressed()) {
        btn_cfg.drawButton(true);
    } else if (btn_cfg.justReleased()) {
        btn_cfg.drawButton(false);
        screen = SCREEN_CONFIG;
    }
}