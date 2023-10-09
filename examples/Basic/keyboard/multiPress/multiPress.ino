/**
 * @file multiPress.ino
 * @author SeanKwok (shaoxiang@m5stack.com)
 * @brief M5Cardputer multi key test
 * @version 0.1
 * @date 2023-10-09
 *
 *
 * @Hardwares: M5Cardputer
 * @Platform Version: Arduino M5Stack Board Manager v2.0.7
 * @Dependent Library:
 * M5GFX: https://github.com/m5stack/M5GFX
 * M5Unified: https://github.com/m5stack/M5Unified
 */

#include "M5Cardputer.h"

void setup() {
    auto cfg = M5.config();
    M5Cardputer.begin(cfg, true);
    M5Cardputer.Display.setRotation(1);
    M5Cardputer.Display.setTextColor(GREEN);
    M5Cardputer.Display.setTextDatum(middle_center);
    M5Cardputer.Display.setTextFont(&fonts::FreeSerifBoldItalic18pt7b);
    M5Cardputer.Display.setTextSize(1);
}

void loop() {
    M5Cardputer.update();
    // max press 3 button at the same time
    if (M5Cardputer.Keyboard.isChange()) {
        if (M5Cardputer.Keyboard.isPressed()) {
            Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();
            String keys                      = "";
            for (auto i : status.values) {
                if (keys != "") {
                    keys = keys + "+" + i;
                } else {
                    keys += i;
                }
            }
            M5Cardputer.Display.clear();
            M5Cardputer.Display.drawString(keys,
                                           M5Cardputer.Display.width() / 2,
                                           M5Cardputer.Display.height() / 2);
        } else {
            M5Cardputer.Display.clear();
            M5Cardputer.Display.drawString("Press Any Key",
                                           M5Cardputer.Display.width() / 2,
                                           M5Cardputer.Display.height() / 2);
        }
    }
}
