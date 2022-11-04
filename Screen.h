#pragma once

#include "sauna.h"

#include <Arduino.h>
#include <SPFD5408_Adafruit_GFX.h> // Core graphics library
#include <SPFD5408_Adafruit_TFTLCD.h> // Hardware-specific library
#include <SPFD5408_TouchScreen.h>




class Screen {
    public:
        virtual void display() = 0;

        virtual void handle_buttons(TSPoint& p) = 0;

        virtual void tick() = 0;
};