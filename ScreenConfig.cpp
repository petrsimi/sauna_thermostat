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
   lcd(lcd), wifi(wifi), screen(screen)
{
    timerWifiStatus.every(15000, timerWifiStatus_cb, this);
    btnConnect.initButton(&lcd, 160, 160, 250, 40, WHITE, BLACK, WHITE, "Nastavit", 2);
    btnExit.initButton(&lcd, 160, 210, 250, 40, WHITE, BLACK, WHITE, "Zrusit", 2);
}


void ScreenConfig::displayWifiStatus()
{
    lcd.fillRect(0, 0, 320, 60, BLACK);
    lcd.setCursor(0, 0);
    lcd.setTextSize(2);
    lcd.setTextColor(WHITE);

    String status;
    wifi.getWifiStatus(status);
    lcd.print(status);
}

void ScreenConfig::display()
{
    lcd.fillScreen(BLACK);

    displayWifiStatus();
    btnConnect.drawButton();
    btnExit.drawButton();
}


void ScreenConfig::tick()
{
    timerWifiStatus.tick();
}

void ScreenConfig::handle_buttons(TSPoint& p)
{
    if (p.z != 0) {
        if (btnExit.contains(p.x, p.y)) {
            btnExit.press(true);
        }
        if (btnConnect.contains(p.x, p.y)) {
            btnConnect.press(true);
        }
    } else {
        btnExit.press(false);
        btnConnect.press(false);
    }

    if (btnExit.justPressed()) {
        btnExit.drawButton(true);
    } else if (btnExit.justReleased()) {
        btnExit.drawButton(false);
        screen = SCREEN_STATUS;
    }

    if (btnConnect.justPressed()) {
        btnConnect.drawButton(true);
    } else if (btnConnect.justReleased()) {
        btnConnect.drawButton(false);
        screen = SCREEN_CONFIG_SSID;
    }
}