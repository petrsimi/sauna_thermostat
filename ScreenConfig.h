#pragma once

#include "sauna.h"

#include <Arduino.h>
#include "Screen.h"
#include "WifiWrap.h"
#include <SPFD5408_Adafruit_GFX.h> // Core graphics library
#include <SPFD5408_Adafruit_TFTLCD.h> // Hardware-specific library
#include <SPFD5408_TouchScreen.h>
#include <arduino-timer.h>
#include "LcdKeyboard.h"




class ScreenConfig : public Screen {
    public:
        ScreenConfig(Adafruit_TFTLCD& lcd, WifiWrap& wifi, screen_t& screen);

        void display();
        void displayWifiStatus();

        void handle_buttons(TSPoint& p);

        void tick();

    private:

        Adafruit_TFTLCD& lcd;
        WifiWrap& wifi;
        screen_t& screen;

        Timer<1, millis> timerWifiStatus;

        LcdKeyboard keyboard;
};