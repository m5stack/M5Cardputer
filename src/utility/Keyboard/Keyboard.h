/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include "KeyboardReader/KeyboardReader.h"
#include "Keyboard_def.h"
#include <Arduino.h>
#include <vector>
#include <memory>

struct KeyValue_t {
    const char value_first;
    const char value_second; // Aa layer
    const char value_third;  // fn layer
};

const KeyValue_t _key_value_map[4][14] = {{{'`', '~', KEY_ESCAPE},
                                           {'1', '!', KEY_F1},
                                           {'2', '@', KEY_F2},
                                           {'3', '#', KEY_F3},
                                           {'4', '$', KEY_F4},
                                           {'5', '%', KEY_F5},
                                           {'6', '^', KEY_F6},
                                           {'7', '&', KEY_F7},
                                           {'8', '*', KEY_F8},
                                           {'9', '(', KEY_F9},
                                           {'0', ')', KEY_F10},
                                           {'-', '_', KEY_F11},
                                           {'=', '+', KEY_F12},
                                           {KEY_BACKSPACE, KEY_BACKSPACE, KEY_DELETE}},
                                          {{KEY_TAB, KEY_TAB, KEY_NONE},
                                           {'q', 'Q', KEY_NONE},
                                           {'w', 'W', KEY_NONE},
                                           {'e', 'E', KEY_NONE},
                                           {'r', 'R', KEY_NONE},
                                           {'t', 'T', KEY_NONE},
                                           {'y', 'Y', KEY_NONE},
                                           {'u', 'U', KEY_NONE},
                                           {'i', 'I', KEY_NONE},
                                           {'o', 'O', KEY_NONE},
                                           {'p', 'P', KEY_NONE},
                                           {'[', '{', KEY_NONE},
                                           {']', '}', KEY_NONE},
                                           {'\\', '|', KEY_NONE}},
                                          {{KEY_FN, KEY_FN, KEY_FN},
                                           {KEY_LEFT_SHIFT, KEY_LEFT_SHIFT, KEY_NONE},
                                           {'a', 'A', KEY_NONE},
                                           {'s', 'S', KEY_NONE},
                                           {'d', 'D', KEY_NONE},
                                           {'f', 'F', KEY_NONE},
                                           {'g', 'G', KEY_NONE},
                                           {'h', 'H', KEY_NONE},
                                           {'j', 'J', KEY_NONE},
                                           {'k', 'K', KEY_NONE},
                                           {'l', 'L', KEY_NONE},
                                           {';', ':', KEY_UP},
                                           {'\'', '\"', KEY_NONE},
                                           {KEY_ENTER, KEY_ENTER, KEY_NONE}},
                                          {{KEY_LEFT_CTRL, KEY_LEFT_CTRL, KEY_NONE},
                                           {KEY_OPT, KEY_OPT, KEY_NONE},
                                           {KEY_LEFT_ALT, KEY_LEFT_ALT, KEY_NONE},
                                           {'z', 'Z', KEY_NONE},
                                           {'x', 'X', KEY_NONE},
                                           {'c', 'C', KEY_NONE},
                                           {'v', 'V', KEY_NONE},
                                           {'b', 'B', KEY_NONE},
                                           {'n', 'N', KEY_NONE},
                                           {'m', 'M', KEY_NONE},
                                           {',', '<', KEY_LEFT},
                                           {'.', '>', KEY_DOWN},
                                           {'/', '?', KEY_RIGHT},
                                           {' ', ' ', KEY_NONE}}};

class Keyboard_Class {
public:
    struct KeysState {
        bool tab          = false;
        bool fn           = false;
        bool shift        = false;
        bool ctrl         = false;
        bool opt          = false;
        bool alt          = false;
        bool backspace    = false;
        bool del          = false;
        bool enter        = false;
        bool space        = false;
        bool esc          = false;
        bool f1           = false;
        bool f2           = false;
        bool f3           = false;
        bool f4           = false;
        bool f5           = false;
        bool f6           = false;
        bool f7           = false;
        bool f8           = false;
        bool f9           = false;
        bool f10          = false;
        bool f11          = false;
        bool f12          = false;
        bool up           = false;
        bool left         = false;
        bool down         = false;
        bool right        = false;
        uint8_t modifiers = 0;

        std::vector<char> word;
        std::vector<uint8_t> hid_keys;
        std::vector<uint8_t> modifier_keys;

        void reset()
        {
            tab       = false;
            fn        = false;
            shift     = false;
            ctrl      = false;
            opt       = false;
            alt       = false;
            backspace = false;
            del       = false;
            enter     = false;
            space     = false;
            esc       = false;
            f1        = false;
            f2        = false;
            f3        = false;
            f4        = false;
            f5        = false;
            f6        = false;
            f7        = false;
            f8        = false;
            f9        = false;
            f10       = false;
            f11       = false;
            f12       = false;
            up        = false;
            left      = false;
            down      = false;
            right     = false;
            modifiers = 0;
            word.clear();
            hid_keys.clear();
            modifier_keys.clear();
        }
    };

    Keyboard_Class() : _is_caps_locked(false)
    {
    }

    void begin();
    void begin(std::unique_ptr<KeyboardReader> reader);
    uint8_t getKey(Point2D_t keyCoor);

    void updateKeyList();
    inline const std::vector<Point2D_t>& keyList()
    {
        if (_keyboard_reader) {
            return _keyboard_reader->keyList();
        }
        static const std::vector<Point2D_t> empty_list;
        return empty_list;
    }

    inline KeyValue_t getKeyValue(const Point2D_t& keyCoor)
    {
        return _key_value_map[keyCoor.y][keyCoor.x];
    }

    uint8_t isPressed();
    bool isChange();
    bool isKeyPressed(char c);

    void updateKeysState();
    inline KeysState& keysState()
    {
        return _keys_state_buffer;
    }

    inline bool capslocked(void)
    {
        return _is_caps_locked;
    }
    inline void setCapsLocked(bool isLocked)
    {
        _is_caps_locked = isLocked;
    }

private:
    std::unique_ptr<KeyboardReader> _keyboard_reader;
    std::vector<Point2D_t> _key_pos_print_keys;     // only text: eg A,B,C
    std::vector<Point2D_t> _key_pos_hid_keys;       // print key + space, enter, del
    std::vector<Point2D_t> _key_pos_modifier_keys;  // modifier key: eg shift, ctrl, alt
    KeysState _keys_state_buffer;
    bool _is_caps_locked;
    uint8_t _last_key_size;
};
