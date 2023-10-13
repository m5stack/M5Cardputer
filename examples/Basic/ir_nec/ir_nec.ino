/**
 * @file ir_nec.ino
 * @author SeanKwok (shaoxiang@m5stack.com)
 * @brief M5Cardputer IR NEC test
 * @version 0.1
 * @date 2023-10-13
 *
 *
 * @Hardwares: M5Cardputer
 * @Platform Version: Arduino M5Stack Board Manager v2.0.7
 * @Dependent Library:
 * M5GFX: https://github.com/m5stack/M5GFX
 * M5Unified: https://github.com/m5stack/M5Unified
 * IRremote: https://github.com/Arduino-IRremote/Arduino-IRremote
 */

#define DISABLE_CODE_FOR_RECEIVER  // Disables restarting receiver after each
                                   // send. Saves 450 bytes program memory and
                                   // 269 bytes RAM if receiving functions are
                                   // not used.
#define SEND_PWM_BY_TIMER
#define IR_TX_PIN 44

#include "M5Cardputer.h"
#include <IRremote.hpp>  // include the library

uint8_t sCommand = 0x34;
uint8_t sRepeats = 0;

void setup() {
    auto cfg = M5.config();
    M5Cardputer.begin(cfg, true);
    M5Cardputer.Display.setRotation(1);
    M5Cardputer.Display.setTextColor(GREEN);
    M5Cardputer.Display.setTextDatum(middle_center);
    M5Cardputer.Display.setTextFont(&fonts::Orbitron_Light_24);
    M5Cardputer.Display.setTextSize(1);

    IrSender.begin(DISABLE_LED_FEEDBACK);  // Start with IR_SEND_PIN as send pin
    IrSender.setSendPin(IR_TX_PIN);
}

void loop() {
    Serial.println();
    Serial.print(F("Send now: address=0x1111, command=0x"));
    Serial.print(sCommand, HEX);
    Serial.print(F(", repeats="));
    Serial.print(sRepeats);
    Serial.println();

    M5Cardputer.Display.clear();
    M5Cardputer.Display.drawString("IR NEC SEND",
                                   M5Cardputer.Display.width() / 2,
                                   M5Cardputer.Display.height() / 2 - 40);

    M5Cardputer.Display.drawString("ADDR:0x1111",
                                   M5Cardputer.Display.width() / 2,
                                   M5Cardputer.Display.height() / 2);

    M5Cardputer.Display.drawString("CMD:0x" + String(sCommand, HEX),
                                   M5Cardputer.Display.width() / 2,
                                   M5Cardputer.Display.height() / 2 + 40);

    Serial.println(F("Send standard NEC with 16 bit address"));

    M5Cardputer.Display.fillCircle(32, 105, 8, GREEN);
    IrSender.sendNEC(0x1111, sCommand, sRepeats);
    // IrSender.sendOnkyo(0x1111, 0x2223, sRepeats);
    /*
     * Increment send values
     */
    sCommand += 1;
    delay(500);
    M5Cardputer.Display.fillCircle(32, 105, 8, YELLOW);
    delay(500);
}
