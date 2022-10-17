#ifndef LCD_KEYBOARD_H
#define LCD_KEYBOARD_H

#include <Arduino.h>

#include <SPFD5408_Adafruit_GFX.h> // Core graphics library
#include <SPFD5408_Adafruit_TFTLCD.h> // Hardware-specific library


class LcdKeyboard {
    private:
        static const int  KEYS = 53;

        static const char KEY_ESC = -1;
        static const char KEY_BACKSPACE = -2;
        static const char KEY_ENTER = -3;
        static const char KEY_SHIFT = -4;
        static const char KEY_CAPS = -5;

        static const char keymap_low[KEYS];
                                            
        static const char keymap_high[KEYS];

    public:
        LcdKeyboard(Adafruit_TFTLCD& lcd);
        void draw();
        char handlePress(int16_t x, int16_t y, bool pressed);


    private:
        Adafruit_GFX_Button key[KEYS];
        bool shift;
        bool capslock;

};

#endif
