/**
 * @file button.ino
 * @author SeanKwok (shaoxiang@m5stack.com)
 * @brief M5Cardputer Button Test
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
    M5Cardputer.begin(cfg);
    M5Cardputer.Display.setRotation(1);
    M5Cardputer.Display.setTextColor(GREEN);
    M5Cardputer.Display.setTextDatum(middle_center);
    M5Cardputer.Display.setTextFont(&fonts::Orbitron_Light_32);
    M5Cardputer.Display.setTextSize(1);
    M5Cardputer.Display.drawString("Button Test",
                                   M5Cardputer.Display.width() / 2,
                                   M5Cardputer.Display.height() / 2);
}

void loop() {
    M5Cardputer.update();
    if (M5Cardputer.BtnA.wasPressed()) {
        M5Cardputer.Speaker.tone(8000, 20);
        M5Cardputer.Display.clear();
        M5Cardputer.Display.drawString("Pressed",
                                       M5Cardputer.Display.width() / 2,
                                       M5Cardputer.Display.height() / 2);
    }
    if (M5Cardputer.BtnA.wasReleased()) {
        M5Cardputer.Speaker.tone(8000, 20);
        M5Cardputer.Display.clear();
        M5Cardputer.Display.drawString("Released",
                                       M5Cardputer.Display.width() / 2,
                                       M5Cardputer.Display.height() / 2);
    }
}