#pragma once

#include "sauna.h"

#include <Arduino.h>
#include "Screen.h"
#include <SPFD5408_Adafruit_GFX.h> // Core graphics library
#include <SPFD5408_Adafruit_TFTLCD.h> // Hardware-specific library
#include <SPFD5408_TouchScreen.h>
#include <arduino-timer.h>
#include "LcdKeyboard.h"




class ScreenConfigPwd : public Screen {
    public:
        ScreenConfigPwd(Adafruit_TFTLCD& lcd, screen_t& screen);

        void display();

        void handle_buttons(TSPoint& p);

        void tick();

    private:

        void displayPwd(bool invert);

        Adafruit_TFTLCD& lcd;
        screen_t& screen;

        LcdKeyboard keyboard;

        String pwd;
};