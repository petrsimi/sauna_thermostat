#include "ScreenStatus.h"
#include <EEPROM.h>


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


ScreenStatus::ScreenStatus(Adafruit_TFTLCD& lcd, uint8_t& target, state_t& state) :
   lcd(lcd), target(target), state(state)
{
  
}

void ScreenStatus::display()
{
    lcd.fillScreen(BLACK);

    btn_minus.initButton(&lcd, 40, 210, 60, 60, WHITE, BLACK, WHITE, "-", 4);
    btn_minus.drawButton();

    btn_plus.initButton(&lcd, 190, 210, 60, 60, WHITE, BLACK, WHITE, "+", 4);
    btn_plus.drawButton();

    btn_on.initButton(&lcd, 280, 210, 60, 60, WHITE, BLACK, WHITE, "ON", 3);
    btn_on.drawButton(false);

    btn_cfg.initButton(&lcd, 280, 150, 60, 40, WHITE, BLACK, WHITE, "Conf", 2);
    btn_cfg.drawButton(false);
}


void ScreenStatus::displayOnOffBtn(bool pressed)
{
    btn_on.drawButton(pressed);
}

void ScreenStatus::tick()
{
    timer_btn.tick();
}

void ScreenStatus::handle_buttons(TSPoint& p) {


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
                btn_on.drawButton(true);
                state = ON;
                break;
            default:
                btn_on.drawButton(false);
                state = OFF;
                break;
        }
    }

    if (btn_cfg.justPressed()) {
        btn_cfg.drawButton(true);
    } else if (btn_cfg.justReleased()) {
        btn_cfg.drawButton(false);
    }
}