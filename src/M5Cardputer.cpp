#include "M5Cardputer.h"

using namespace m5;

M5_CARDPUTER M5Cardputer;

void M5_CARDPUTER::begin(bool enableKeyboard) {
    M5.begin();
    _enableKeyboard = enableKeyboard;
    if (enableKeyboard) {
        Keyboard.begin();
    }
}

void M5_CARDPUTER::begin(m5::M5Unified::config_t cfg, bool enableKeyboard) {
    M5.begin(cfg);
    _enableKeyboard = enableKeyboard;
    if (enableKeyboard) {
        Keyboard.begin();
    }
}

void M5_CARDPUTER::update(void) {
    M5.update();
    if (_enableKeyboard) {
        Keyboard.updateKeyList();
        Keyboard.updateKeysState();
    }
}


