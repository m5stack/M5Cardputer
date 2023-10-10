/**
 * @file keyboard.h
 * @author Forairaaaaa
 * @brief
 * @version 0.1
 * @date 2023-09-22
 *
 * @copyright Copyright (c) 2023
 *
 */
#pragma once
#include <iostream>
#include <vector>
#include "Arduino.h"
#include "Keyboard_def.h"

struct Chart_t {
    uint8_t value;
    uint8_t x_1;
    uint8_t x_2;
};

struct Point2D_t {
    int x;
    int y;
};

const std::vector<int> output_list = {8, 9, 11};
const std::vector<int> input_list  = {13, 15, 3, 4, 5, 6, 7};

const Chart_t X_map_chart[7] = {{1, 0, 1},   {2, 2, 3},  {4, 4, 5},
                                {8, 6, 7},   {16, 8, 9}, {32, 10, 11},
                                {64, 12, 13}};

struct KeyValue_t {
    const char* value_first;
    const int value_num_first;
    const char* value_second;
    const int value_num_second;
};

const KeyValue_t _key_value_map[4][14] = {
    {{"`", KB_KEY_BACK_QUOTE, "~", KB_KEY_TILDE},
     {"1", KB_KEY_1, "!", KB_KEY_BANG},
     {"2", KB_KEY_2, "@", KB_KEY_AT},
     {"3", KB_KEY_3, "#", KB_KEY_HASH},
     {"4", KB_KEY_4, "$", KB_KEY_DOLLAR},
     {"5", KB_KEY_5, "%", KB_KEY_PERCENT},
     {"6", KB_KEY_6, "^", KB_KEY_CARET},
     {"7", KB_KEY_7, "&", KB_KEY_AND},
     {"8", KB_KEY_8, "*", KB_KEY_ASTERISK},
     {"9", KB_KEY_9, "(", KB_KEY_L_PARENTHESES},
     {"0", KB_KEY_0, ")", KB_KEY_R_PARENTHESES},
     {"-", KB_KEY_MINUS, "_", KB_KEY_UNDERSCORE},
     {"=", KB_KEY_EQUAL, "+", KB_KEY_PLUS},
     {"del", KB_KEY_DEL, "del", KB_KEY_DEL}},
    {{"tab", KB_KEY_TAB, "tab", KB_KEY_TAB},
     {"q", KB_KEY_L_Q, "Q", KB_KEY_U_Q},
     {"w", KB_KEY_L_W, "W", KB_KEY_U_W},
     {"e", KB_KEY_L_E, "E", KB_KEY_U_E},
     {"r", KB_KEY_L_R, "R", KB_KEY_U_R},
     {"t", KB_KEY_L_T, "T", KB_KEY_U_T},
     {"y", KB_KEY_L_Y, "Y", KB_KEY_U_Y},
     {"u", KB_KEY_L_U, "U", KB_KEY_U_U},
     {"i", KB_KEY_L_I, "I", KB_KEY_U_I},
     {"o", KB_KEY_L_O, "O", KB_KEY_U_O},
     {"p", KB_KEY_L_P, "P", KB_KEY_U_P},
     {"[", KB_KEY_L_PARENTHESES, "{", KB_KEY_L_CURLY_BRACKETS},
     {"]", KB_KEY_R_PARENTHESES, "}", KB_KEY_R_CURLY_BRACKETS},
     {"\\", KB_KEY_BACK_SLASK, "|", KB_KEY_BAR}},
    {{"fn", KB_KEY_FN, "fn", KB_KEY_FN},
     {"shift", KB_KEY_SHIFT, "shift", KB_KEY_SHIFT},
     {"a", KB_KEY_L_A, "A", KB_KEY_U_A},
     {"s", KB_KEY_L_S, "S", KB_KEY_U_S},
     {"d", KB_KEY_L_D, "D", KB_KEY_U_D},
     {"f", KB_KEY_L_F, "F", KB_KEY_U_F},
     {"g", KB_KEY_L_G, "G", KB_KEY_U_G},
     {"h", KB_KEY_L_H, "H", KB_KEY_U_H},
     {"j", KB_KEY_L_J, "J", KB_KEY_U_J},
     {"k", KB_KEY_L_K, "K", KB_KEY_U_K},
     {"l", KB_KEY_L_L, "L", KB_KEY_U_L},
     {";", KB_KEY_SEMICOLON, ":", KB_KEY_COLON},
     {"'", KB_KEY_SIGLE_QUOTE, "\"", KB_KEY_QUOTE},
     {"enter", KB_KEY_ENTER, "enter", KB_KEY_ENTER}},
    {{"ctrl", KB_KEY_CTRL, "ctrl", KB_KEY_CTRL},
     {"opt", KB_KEY_OPT, "opt", KB_KEY_OPT},
     {"alt", KB_KEY_ALT, "alt", KB_KEY_ALT},
     {"z", KB_KEY_L_Z, "Z", KB_KEY_U_Z},
     {"x", KB_KEY_L_X, "X", KB_KEY_U_X},
     {"c", KB_KEY_L_C, "C", KB_KEY_U_C},
     {"v", KB_KEY_L_V, "V", KB_KEY_U_V},
     {"b", KB_KEY_L_B, "B", KB_KEY_U_B},
     {"n", KB_KEY_L_N, "N", KB_KEY_U_N},
     {"m", KB_KEY_L_M, "M", KB_KEY_U_M},
     {",", KB_KEY_COMMA, "<", KB_KEY_L_ANGLE_BRACKETS},
     {".", KB_KEY_DOT, ">", KB_KEY_R_ANGLE_BRACKETS},
     {"/", KB_KEY_SLASH, "?", KB_KEY_QUESTION},
     {"space", KB_KEY_SPACE, "space", KB_KEY_SPACE}}};

class Keyboard_Class {
   public:
    struct KeysState {
        bool tab   = false;
        bool fn    = false;
        bool shift = false;
        bool ctrl  = false;
        bool opt   = false;
        bool alt   = false;
        bool del   = false;
        bool enter = false;
        bool space = false;

        std::vector<char> values;

        void reset() {
            tab   = false;
            fn    = false;
            shift = false;
            ctrl  = false;
            opt   = false;
            alt   = false;
            del   = false;
            enter = false;
            space = false;

            values.clear();
        }
    };

   private:
    std::vector<Point2D_t> _key_list_buffer;
    std::vector<Point2D_t> _key_values_without_special_keys;
    KeysState _keys_state_buffer;
    bool _is_caps_locked;
    uint8_t _last_key_size;

    void _set_output(const std::vector<int>& pinList, uint8_t output);
    uint8_t _get_input(const std::vector<int>& pinList);

   public:
    Keyboard_Class() : _is_caps_locked(false) {
    }

    void begin();
    int getKeyCode(Point2D_t keyCoor);

    void updateKeyList();
    inline std::vector<Point2D_t>& keyList() {
        return _key_list_buffer;
    }

    inline KeyValue_t getKeyValue(const Point2D_t& keyCoor) {
        return _key_value_map[keyCoor.y][keyCoor.x];
    }

    uint8_t isPressed();
    bool isChange();
    bool isKeyPressed(int keyCode);

    void updateKeysState();
    inline KeysState& keysState() {
        return _keys_state_buffer;
    }

    inline bool capslocked(void) {
        return _is_caps_locked;
    }
    inline void setCapsLocked(bool isLocked) {
        _is_caps_locked = isLocked;
    }
};
