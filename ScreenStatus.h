#pragma once

#include "sauna.h"

#include <Arduino.h>
#include <SPFD5408_Adafruit_GFX.h> // Core graphics library
#include <SPFD5408_Adafruit_TFTLCD.h> // Hardware-specific library
#include <SPFD5408_TouchScreen.h>
#include <arduino-timer.h>


class ScreenStatus {
    public:
        ScreenStatus(Adafruit_TFTLCD& lcd, uint8_t& target, state_t& state);

        void display();
        void displayOnOffBtn(bool pressed);

        void handle_buttons(TSPoint& p);

        void tick();

    private:
        Adafruit_GFX_Button btn_plus;
        Adafruit_GFX_Button btn_minus;
        Adafruit_GFX_Button btn_on;
        Adafruit_GFX_Button btn_cfg;

        Timer<1, millis> timer_btn;

        Adafruit_TFTLCD& lcd;
        uint8_t& target;
        state_t& state;
};