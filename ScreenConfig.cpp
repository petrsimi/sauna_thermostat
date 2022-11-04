#include "ScreenConfig.h"


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


static bool timerWifiStatus_cb(void* ptr)
{
    ScreenConfig* screenConfig = ptr;
    screenConfig->displayWifiStatus();
}


ScreenConfig::ScreenConfig(Adafruit_TFTLCD& lcd, WifiWrap& wifi, screen_t& screen) :
   lcd(lcd), wifi(wifi), screen(screen), keyboard(lcd)
{
    timerWifiStatus.every(15000, timerWifiStatus_cb, this);
}


void ScreenConfig::displayWifiStatus()
{
    lcd.fillRect(10, 10, 300, 7, BLACK);
    lcd.setCursor(10, 10);
    lcd.setTextSize(1);
    lcd.setTextColor(BLUE);

    String status;
    wifi.getWifiStatus(status);
    lcd.print(status);
}

void ScreenConfig::display()
{
    lcd.fillScreen(BLACK);

    keyboard.draw();

    displayWifiStatus();
}


void ScreenConfig::tick()
{
    timerWifiStatus.tick();
}

void ScreenConfig::handle_buttons(TSPoint& p)
{
    char key = keyboard.handlePress(p.x, p.y, p.z);

    if (key == LcdKeyboard::KEY_ESC) {
        screen = SCREEN_STATUS;
    }
}