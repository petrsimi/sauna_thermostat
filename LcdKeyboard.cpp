#include  "LcdKeyboard.h"

#define KEY_WIDTH 28
#define KEY_HEIGHT 28

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
#define GRAY    0xE71C

static const char LcdKeyboard::keymap_low[KEYS]  = {'1', '2', '3', '4', '5', '6', '7', '8', '9', '0', KEY_ESC,
                                                    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', KEY_BACKSPACE,
                                                    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', KEY_ENTER,
                                                    'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', KEY_SHIFT,
                                                    '`', '-', '=', '[', ']', '\\', '\'', ' ', KEY_CAPS};
                                            
static const char LcdKeyboard::keymap_high[KEYS] = {'!', '@', '#', '$', '%', '^', '&', '*', '(', ')', KEY_ESC,
                                                    'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', KEY_BACKSPACE,
                                                    'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', KEY_ENTER,
                                                    'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', KEY_SHIFT,
                                                    '~', '_', '+', '{', '}', '|', '"', ' ', KEY_CAPS};




LcdKeyboard::LcdKeyboard(Adafruit_TFTLCD& lcd)
{
    shift = false;
    capslock = false;

    // First row (numbers)
    key[ 0].initButton(&lcd, KEY_WIDTH/2 + (KEY_WIDTH+1) *  0, 96 + KEY_HEIGHT/2 + (KEY_HEIGHT+1) * 0, KEY_WIDTH, KEY_HEIGHT, WHITE, BLACK, WHITE, "1!", 2);
    key[ 1].initButton(&lcd, KEY_WIDTH/2 + (KEY_WIDTH+1) *  1, 96 + KEY_HEIGHT/2 + (KEY_HEIGHT+1) * 0, KEY_WIDTH, KEY_HEIGHT, WHITE, BLACK, WHITE, "2@", 2);
    key[ 2].initButton(&lcd, KEY_WIDTH/2 + (KEY_WIDTH+1) *  2, 96 + KEY_HEIGHT/2 + (KEY_HEIGHT+1) * 0, KEY_WIDTH, KEY_HEIGHT, WHITE, BLACK, WHITE, "3#", 2);
    key[ 3].initButton(&lcd, KEY_WIDTH/2 + (KEY_WIDTH+1) *  3, 96 + KEY_HEIGHT/2 + (KEY_HEIGHT+1) * 0, KEY_WIDTH, KEY_HEIGHT, WHITE, BLACK, WHITE, "4$", 2);
    key[ 4].initButton(&lcd, KEY_WIDTH/2 + (KEY_WIDTH+1) *  4, 96 + KEY_HEIGHT/2 + (KEY_HEIGHT+1) * 0, KEY_WIDTH, KEY_HEIGHT, WHITE, BLACK, WHITE, "5%", 2);
    key[ 5].initButton(&lcd, KEY_WIDTH/2 + (KEY_WIDTH+1) *  5, 96 + KEY_HEIGHT/2 + (KEY_HEIGHT+1) * 0, KEY_WIDTH, KEY_HEIGHT, WHITE, BLACK, WHITE, "6^", 2);
    key[ 6].initButton(&lcd, KEY_WIDTH/2 + (KEY_WIDTH+1) *  6, 96 + KEY_HEIGHT/2 + (KEY_HEIGHT+1) * 0, KEY_WIDTH, KEY_HEIGHT, WHITE, BLACK, WHITE, "7&", 2);
    key[ 7].initButton(&lcd, KEY_WIDTH/2 + (KEY_WIDTH+1) *  7, 96 + KEY_HEIGHT/2 + (KEY_HEIGHT+1) * 0, KEY_WIDTH, KEY_HEIGHT, WHITE, BLACK, WHITE, "8*", 2);
    key[ 8].initButton(&lcd, KEY_WIDTH/2 + (KEY_WIDTH+1) *  8, 96 + KEY_HEIGHT/2 + (KEY_HEIGHT+1) * 0, KEY_WIDTH, KEY_HEIGHT, WHITE, BLACK, WHITE, "9(", 2);
    key[ 9].initButton(&lcd, KEY_WIDTH/2 + (KEY_WIDTH+1) *  9, 96 + KEY_HEIGHT/2 + (KEY_HEIGHT+1) * 0, KEY_WIDTH, KEY_HEIGHT, WHITE, BLACK, WHITE, "0)", 2);
    key[10].initButton(&lcd, KEY_WIDTH/2 + (KEY_WIDTH+1) * 10, 96 + KEY_HEIGHT/2 + (KEY_HEIGHT+1) * 0, KEY_WIDTH, KEY_HEIGHT, WHITE, RED, BLACK, "Esc", 1);

    // Second row (qwertyuiop)
    key[11].initButton(&lcd, KEY_WIDTH/2 + (KEY_WIDTH+1) *  0, 96 + KEY_HEIGHT/2 + (KEY_HEIGHT+1) * 1, KEY_WIDTH, KEY_HEIGHT, WHITE, BLACK, WHITE, "Q", 2);
    key[12].initButton(&lcd, KEY_WIDTH/2 + (KEY_WIDTH+1) *  1, 96 + KEY_HEIGHT/2 + (KEY_HEIGHT+1) * 1, KEY_WIDTH, KEY_HEIGHT, WHITE, BLACK, WHITE, "W", 2);
    key[13].initButton(&lcd, KEY_WIDTH/2 + (KEY_WIDTH+1) *  2, 96 + KEY_HEIGHT/2 + (KEY_HEIGHT+1) * 1, KEY_WIDTH, KEY_HEIGHT, WHITE, BLACK, WHITE, "E", 2);
    key[14].initButton(&lcd, KEY_WIDTH/2 + (KEY_WIDTH+1) *  3, 96 + KEY_HEIGHT/2 + (KEY_HEIGHT+1) * 1, KEY_WIDTH, KEY_HEIGHT, WHITE, BLACK, WHITE, "R", 2);
    key[15].initButton(&lcd, KEY_WIDTH/2 + (KEY_WIDTH+1) *  4, 96 + KEY_HEIGHT/2 + (KEY_HEIGHT+1) * 1, KEY_WIDTH, KEY_HEIGHT, WHITE, BLACK, WHITE, "T", 2);
    key[16].initButton(&lcd, KEY_WIDTH/2 + (KEY_WIDTH+1) *  5, 96 + KEY_HEIGHT/2 + (KEY_HEIGHT+1) * 1, KEY_WIDTH, KEY_HEIGHT, WHITE, BLACK, WHITE, "Y", 2);
    key[17].initButton(&lcd, KEY_WIDTH/2 + (KEY_WIDTH+1) *  6, 96 + KEY_HEIGHT/2 + (KEY_HEIGHT+1) * 1, KEY_WIDTH, KEY_HEIGHT, WHITE, BLACK, WHITE, "U", 2);
    key[18].initButton(&lcd, KEY_WIDTH/2 + (KEY_WIDTH+1) *  7, 96 + KEY_HEIGHT/2 + (KEY_HEIGHT+1) * 1, KEY_WIDTH, KEY_HEIGHT, WHITE, BLACK, WHITE, "I", 2);
    key[19].initButton(&lcd, KEY_WIDTH/2 + (KEY_WIDTH+1) *  8, 96 + KEY_HEIGHT/2 + (KEY_HEIGHT+1) * 1, KEY_WIDTH, KEY_HEIGHT, WHITE, BLACK, WHITE, "O", 2);
    key[20].initButton(&lcd, KEY_WIDTH/2 + (KEY_WIDTH+1) *  9, 96 + KEY_HEIGHT/2 + (KEY_HEIGHT+1) * 1, KEY_WIDTH, KEY_HEIGHT, WHITE, BLACK, WHITE, "P", 2);
    key[21].initButton(&lcd, KEY_WIDTH/2 + (KEY_WIDTH+1) * 10, 96 + KEY_HEIGHT/2 + (KEY_HEIGHT+1) * 1, KEY_WIDTH, KEY_HEIGHT, WHITE, YELLOW, BLACK, "Back", 1);

    // Third row (asdfghjkl)
    key[22].initButton(&lcd, KEY_WIDTH/2 + (KEY_WIDTH+1) *  0, 96 + KEY_HEIGHT/2 + (KEY_HEIGHT+1) * 2, KEY_WIDTH, KEY_HEIGHT, WHITE, BLACK, WHITE, "A", 2);
    key[23].initButton(&lcd, KEY_WIDTH/2 + (KEY_WIDTH+1) *  1, 96 + KEY_HEIGHT/2 + (KEY_HEIGHT+1) * 2, KEY_WIDTH, KEY_HEIGHT, WHITE, BLACK, WHITE, "S", 2);
    key[24].initButton(&lcd, KEY_WIDTH/2 + (KEY_WIDTH+1) *  2, 96 + KEY_HEIGHT/2 + (KEY_HEIGHT+1) * 2, KEY_WIDTH, KEY_HEIGHT, WHITE, BLACK, WHITE, "D", 2);
    key[25].initButton(&lcd, KEY_WIDTH/2 + (KEY_WIDTH+1) *  3, 96 + KEY_HEIGHT/2 + (KEY_HEIGHT+1) * 2, KEY_WIDTH, KEY_HEIGHT, WHITE, BLACK, WHITE, "F", 2);
    key[26].initButton(&lcd, KEY_WIDTH/2 + (KEY_WIDTH+1) *  4, 96 + KEY_HEIGHT/2 + (KEY_HEIGHT+1) * 2, KEY_WIDTH, KEY_HEIGHT, WHITE, BLACK, WHITE, "G", 2);
    key[27].initButton(&lcd, KEY_WIDTH/2 + (KEY_WIDTH+1) *  5, 96 + KEY_HEIGHT/2 + (KEY_HEIGHT+1) * 2, KEY_WIDTH, KEY_HEIGHT, WHITE, BLACK, WHITE, "H", 2);
    key[28].initButton(&lcd, KEY_WIDTH/2 + (KEY_WIDTH+1) *  6, 96 + KEY_HEIGHT/2 + (KEY_HEIGHT+1) * 2, KEY_WIDTH, KEY_HEIGHT, WHITE, BLACK, WHITE, "J", 2);
    key[29].initButton(&lcd, KEY_WIDTH/2 + (KEY_WIDTH+1) *  7, 96 + KEY_HEIGHT/2 + (KEY_HEIGHT+1) * 2, KEY_WIDTH, KEY_HEIGHT, WHITE, BLACK, WHITE, "K", 2);
    key[30].initButton(&lcd, KEY_WIDTH/2 + (KEY_WIDTH+1) *  8, 96 + KEY_HEIGHT/2 + (KEY_HEIGHT+1) * 2, KEY_WIDTH, KEY_HEIGHT, WHITE, BLACK, WHITE, "L", 2);
    key[31].initButton(&lcd, KEY_WIDTH/2 + (KEY_WIDTH+1) *  9, 96 + KEY_HEIGHT/2 + (KEY_HEIGHT+1) * 2, KEY_WIDTH, KEY_HEIGHT, WHITE, BLACK, WHITE, ";:", 2);
    key[32].initButton(&lcd, KEY_WIDTH/2 + (KEY_WIDTH+1) * 10, 96 + KEY_HEIGHT/2 + (KEY_HEIGHT+1) * 2, KEY_WIDTH, KEY_HEIGHT, WHITE, GREEN, BLACK, "Ent", 1);
    // Fourth row (zxcvbnm,.)
    key[33].initButton(&lcd, KEY_WIDTH/2 + (KEY_WIDTH+1) *  0, 96 + KEY_HEIGHT/2 + (KEY_HEIGHT+1) * 3, KEY_WIDTH, KEY_HEIGHT, WHITE, BLACK, WHITE, "Z", 2);
    key[34].initButton(&lcd, KEY_WIDTH/2 + (KEY_WIDTH+1) *  1, 96 + KEY_HEIGHT/2 + (KEY_HEIGHT+1) * 3, KEY_WIDTH, KEY_HEIGHT, WHITE, BLACK, WHITE, "X", 2);
    key[35].initButton(&lcd, KEY_WIDTH/2 + (KEY_WIDTH+1) *  2, 96 + KEY_HEIGHT/2 + (KEY_HEIGHT+1) * 3, KEY_WIDTH, KEY_HEIGHT, WHITE, BLACK, WHITE, "C", 2);
    key[36].initButton(&lcd, KEY_WIDTH/2 + (KEY_WIDTH+1) *  3, 96 + KEY_HEIGHT/2 + (KEY_HEIGHT+1) * 3, KEY_WIDTH, KEY_HEIGHT, WHITE, BLACK, WHITE, "V", 2);
    key[37].initButton(&lcd, KEY_WIDTH/2 + (KEY_WIDTH+1) *  4, 96 + KEY_HEIGHT/2 + (KEY_HEIGHT+1) * 3, KEY_WIDTH, KEY_HEIGHT, WHITE, BLACK, WHITE, "B", 2);
    key[38].initButton(&lcd, KEY_WIDTH/2 + (KEY_WIDTH+1) *  5, 96 + KEY_HEIGHT/2 + (KEY_HEIGHT+1) * 3, KEY_WIDTH, KEY_HEIGHT, WHITE, BLACK, WHITE, "N", 2);
    key[39].initButton(&lcd, KEY_WIDTH/2 + (KEY_WIDTH+1) *  6, 96 + KEY_HEIGHT/2 + (KEY_HEIGHT+1) * 3, KEY_WIDTH, KEY_HEIGHT, WHITE, BLACK, WHITE, "M", 2);
    key[40].initButton(&lcd, KEY_WIDTH/2 + (KEY_WIDTH+1) *  7, 96 + KEY_HEIGHT/2 + (KEY_HEIGHT+1) * 3, KEY_WIDTH, KEY_HEIGHT, WHITE, BLACK, WHITE, ",<", 2);
    key[41].initButton(&lcd, KEY_WIDTH/2 + (KEY_WIDTH+1) *  8, 96 + KEY_HEIGHT/2 + (KEY_HEIGHT+1) * 3, KEY_WIDTH, KEY_HEIGHT, WHITE, BLACK, WHITE, ".>", 2);
    key[42].initButton(&lcd, KEY_WIDTH/2 + (KEY_WIDTH+1) *  9, 96 + KEY_HEIGHT/2 + (KEY_HEIGHT+1) * 3, KEY_WIDTH, KEY_HEIGHT, WHITE, BLACK, WHITE, "/?", 2);
    key[43].initButton(&lcd, KEY_WIDTH/2 + (KEY_WIDTH+1) * 10, 96 + KEY_HEIGHT/2 + (KEY_HEIGHT+1) * 3, KEY_WIDTH, KEY_HEIGHT, WHITE, BLACK, WHITE, "Shi", 1);

    // Fifth row (special chars)
    key[44].initButton(&lcd, KEY_WIDTH/2 + (KEY_WIDTH+1) *  0, 96 + KEY_HEIGHT/2 + (KEY_HEIGHT+1) * 4, KEY_WIDTH, KEY_HEIGHT, WHITE, BLACK, WHITE, "`~", 2);
    key[45].initButton(&lcd, KEY_WIDTH/2 + (KEY_WIDTH+1) *  1, 96 + KEY_HEIGHT/2 + (KEY_HEIGHT+1) * 4, KEY_WIDTH, KEY_HEIGHT, WHITE, BLACK, WHITE, "-_", 2);
    key[46].initButton(&lcd, KEY_WIDTH/2 + (KEY_WIDTH+1) *  2, 96 + KEY_HEIGHT/2 + (KEY_HEIGHT+1) * 4, KEY_WIDTH, KEY_HEIGHT, WHITE, BLACK, WHITE, "=+", 2);
    key[47].initButton(&lcd, KEY_WIDTH/2 + (KEY_WIDTH+1) *  3, 96 + KEY_HEIGHT/2 + (KEY_HEIGHT+1) * 4, KEY_WIDTH, KEY_HEIGHT, WHITE, BLACK, WHITE, "[{", 2);
    key[48].initButton(&lcd, KEY_WIDTH/2 + (KEY_WIDTH+1) *  4, 96 + KEY_HEIGHT/2 + (KEY_HEIGHT+1) * 4, KEY_WIDTH, KEY_HEIGHT, WHITE, BLACK, WHITE, "]}", 2);
    key[49].initButton(&lcd, KEY_WIDTH/2 + (KEY_WIDTH+1) *  5, 96 + KEY_HEIGHT/2 + (KEY_HEIGHT+1) * 4, KEY_WIDTH, KEY_HEIGHT, WHITE, BLACK, WHITE, "\\|", 2);
    key[50].initButton(&lcd, KEY_WIDTH/2 + (KEY_WIDTH+1) *  6, 96 + KEY_HEIGHT/2 + (KEY_HEIGHT+1) * 4, KEY_WIDTH, KEY_HEIGHT, WHITE, BLACK, WHITE, "'\"", 2);
    key[51].initButton(&lcd, KEY_WIDTH/2 + (KEY_WIDTH+1) *  8, 96 + KEY_HEIGHT/2 + (KEY_HEIGHT+1) * 4, KEY_WIDTH*3 + 2, KEY_HEIGHT, WHITE, BLACK, WHITE, "Space", 1);
    key[52].initButton(&lcd, KEY_WIDTH/2 + (KEY_WIDTH+1) * 10, 96 + KEY_HEIGHT/2 + (KEY_HEIGHT+1) * 4, KEY_WIDTH, KEY_HEIGHT, WHITE, BLACK, WHITE, "Caps", 1);
}


void LcdKeyboard::draw()
{
    for (int i=0; i<KEYS; i++) {
        key[i].drawButton();
    }
}

char LcdKeyboard::handlePress(int16_t x, int16_t y, bool pressed)
{
    char ret = -1;

    for (int i=0; i<KEYS; i++) {
        // Update "press" status for all keys
        if (pressed && key[i].contains(x, y)) {
            key[i].press(true);
        } else {
            key[i].press(false);
        }

        // Handle the press event of the key
        if (key[i].justPressed()) {
            key[i].drawButton(true);
            if (shift || capslock) {
                ret = keymap_high[i];
            } else {
                ret = keymap_low[i];
            }

            if (ret == KEY_SHIFT) {
                shift = !shift;
            } else if (shift) {
                shift = false;
                key[43].drawButton(shift);
            }
            if (ret == KEY_CAPS) {
                capslock = !capslock;
            }
            Serial.print("Key: ");
            Serial.println(ret);
        }

        // Handle the release event of the key
        if (key[i].justReleased()) {
            switch(keymap_low[i]) {
                case KEY_SHIFT:
                    key[i].drawButton(shift);
                    break;
                case KEY_CAPS:
                    key[i].drawButton(capslock);
                    break;
                default:
                    // Release the key
                    key[i].drawButton(false);
                    break;
            }
        }

    }

    return ret;
}