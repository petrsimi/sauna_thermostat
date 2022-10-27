#pragma once

#include "sauna.h"

#include <Arduino.h>
#include <SPFD5408_Adafruit_GFX.h> // Core graphics library
#include <SPFD5408_Adafruit_TFTLCD.h> // Hardware-specific library
#include <SPFD5408_TouchScreen.h>
#include <arduino-timer.h>
#include "gfxfont.h"


class ScreenStatus {
    public:
        ScreenStatus(Adafruit_TFTLCD& lcd, uint16_t& temp, uint8_t& target, state_t& state);

        void display();

        void handle_buttons(TSPoint& p);

        void tick();

    private:
        uint16_t draw_char(uint16_t x, uint16_t y, uint8_t size, uint16_t color, const GFXfont* font, char c);

        void print_temperature(uint16_t temp);
        void print_target();

        Adafruit_GFX_Button btn_plus;
        Adafruit_GFX_Button btn_minus;
        Adafruit_GFX_Button btn_on;
        Adafruit_GFX_Button btn_cfg;

        Timer<1, millis> timer_btn;

        Adafruit_TFTLCD& lcd;
        uint16_t& temp;
        uint8_t& target;
        state_t& state;

        uint16_t temp_last;
        uint8_t target_last;
        state_t state_last;

        char last_temp_str[10];
};