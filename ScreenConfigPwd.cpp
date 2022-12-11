#include "ScreenConfigPwd.h"


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


ScreenConfigPwd::ScreenConfigPwd(Adafruit_TFTLCD& lcd, screen_t& screen) :
   lcd(lcd), screen(screen), keyboard(lcd)
{
    pwdAvailable = false;
}


void ScreenConfigPwd::display()
{
    lcd.fillScreen(BLACK);
    lcd.setCursor(0, 0);
    lcd.setTextSize(2);
    lcd.setTextColor(CYAN);

    lcd.print(F("Zadejte heslo:"));

    keyboard.draw();
}

void ScreenConfigPwd::displayPwd(bool invert)
{
    lcd.setTextSize(2);
    lcd.setCursor(0, 20);
    lcd.setTextColor(invert ? BLACK : WHITE);
    lcd.print(pwd);
}

void ScreenConfigPwd::tick()
{

}

void ScreenConfigPwd::handle_buttons(TSPoint& p)
{
    char key = keyboard.handlePress(p.x, p.y, p.z);

    if (key == LcdKeyboard::KEY_ESC) {
        screen = SCREEN_CONFIG;
        pwd = "";
    } else if (key == LcdKeyboard::KEY_ENTER) {
        screen = SCREEN_CONFIG;
        pwdAvailable = true;
    } else if (key == LcdKeyboard::KEY_BACKSPACE) {
        // clear the old text
        displayPwd(true);
        // remove the character
        pwd.remove(pwd.length()-1);
        // and display the SSID
        displayPwd(false);
    } else if (key > 0) {
        pwd += key;
        displayPwd(false);
    }
}
