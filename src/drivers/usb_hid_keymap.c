// 
//  Created by Lorenz on 31/03/2022.
// 

#include <stdint.h>

// [KEYCODE][LAYER]
// Layers:
//  - 0 none
//  - 1 shift
//  - 2 alt right

// TODO: Add keymap type with a layer count and only define necessary layers in keymaps

// Qwerty keymap (Modified, original from: https://github.com/hathach/tinyusb/blob/ae531a79f654d566790a4daae350730cdc0a01e9/src/class/hid/hid.h)
uint8_t usb_hid_keycode_to_ascii_qwerty[104][3] = {
        {0, 0, 0},          //  0x00
        {0, 0, 0},          //  0x01
        {0, 0, 0},          // 0x02
        {0, 0, 0},          // 0x03
        {'a', 'A', 0},      // 0x04
        {'b', 'B', 0},      // 0x05
        {'c', 'C', 0},      // 0x06
        {'d', 'D', 0},      // 0x07
        {'e', 'E', 0},      // 0x08
        {'f', 'F', 0},      // 0x09
        {'g', 'G', 0},      // 0x0a
        {'h', 'H', 0},      // 0x0b
        {'i', 'I', 0},      // 0x0c
        {'j', 'J', 0},      // 0x0d
        {'k', 'K', 0},      // 0x0e
        {'l', 'L', 0},      // 0x0f
        {'m', 'M', 0},      // 0x10
        {'n', 'N', 0},      // 0x11
        {'o', 'O', 0},      // 0x12
        {'p', 'P', 0},      // 0x13
        {'q', 'Q', 0},      // 0x14
        {'r', 'R', 0},      // 0x15
        {'s', 'S', 0},      // 0x16
        {'t', 'T', 0},      // 0x17
        {'u', 'U', 0},      // 0x18
        {'v', 'V', 0},      // 0x19
        {'w', 'W', 0},      // 0x1a
        {'x', 'X', 0},      // 0x1b
        {'y', 'Y', 0},      // 0x1c
        {'z', 'Z', 0},      // 0x1d
        {'1', '!', 0},      // 0x1e
        {'2', '@', 0},      // 0x1f
        {'3', '#', 0},      // 0x20
        {'4', '$', 0},      // 0x21
        {'5', '%', 0},      // 0x22
        {'6', '^', 0},      // 0x23
        {'7', '&', 0},      // 0x24
        {'8', '*', 0},      // 0x25
        {'9', '(', 0},      // 0x26
        {'0', ')', 0},      // 0x27
        {'\r', '\r', 0 },       // 0x28
        {'\x1b', '\x1b', 0 },   // 0x29
        {'\b', '\b', 0 },       // 0x2a
        {'\t', '\t', 0 },       // 0x2b
        {' ', ' ', 0},      // 0x2c
        {'-', '_', 0},      // 0x2d
        {'=', '+', 0},      // 0x2e
        {'[', '{', 0},      // 0x2f
        {']', '}', 0},      // 0x30
        {'\\', '|', 0},     // 0x31
        {'#', '~', 0},      // 0x32
        {';', ':', 0},      // 0x33
        {'\'', '\"',  0 },      // 0x34
        {'`', '~', 0},      // 0x35
        {',', '<', 0},      // 0x36
        {'.', '>', 0},      // 0x37
        {'/', '?', 0},      // 0x38
        {0, 0, 0},          // 0x39
        {0, 0, 0},          // 0x3a
        {0, 0, 0},          // 0x3b
        {0, 0, 0},          // 0x3c
        {0, 0, 0},          // 0x3d
        {0, 0, 0},          // 0x3e
        {0, 0, 0},          // 0x3f
        {0, 0, 0},          // 0x40
        {0, 0, 0},          // 0x41
        {0, 0, 0},          // 0x42
        {0, 0, 0},          // 0x43
        {0, 0, 0},          // 0x44
        {0, 0, 0},          // 0x45
        {0, 0, 0},          // 0x46
        {0, 0, 0},          // 0x47
        {0, 0, 0},          // 0x48
        {0, 0, 0},          // 0x49
        {0, 0, 0},          // 0x4a
        {0, 0, 0},          // 0x4b
        {0, 0, 0},          // 0x4c
        {0, 0, 0},          // 0x4d
        {0, 0, 0},          // 0x4e
        {0, 0, 0},          // 0x4f
        {0, 0, 0},          // 0x50
        {0, 0, 0},          // 0x51
        {0, 0, 0},          // 0x52
        {0, 0, 0},          // 0x53
        {'/', '/', 0},      // 0x54
        {'*', '*', 0},      // 0x55
        {'-', '-', 0},      // 0x56
        {'+', '+', 0},      // 0x57
        {'\r', '\r', 0},    // 0x58
        {'1', 0, 0},        // 0x59
        {'2', 0, 0},        // 0x5a
        {'3', 0, 0},        // 0x5b
        {'4', 0, 0},        // 0x5c
        {'5', '5', 0},      // 0x5d
        {'6', 0, 0},        // 0x5e
        {'7', 0, 0},        // 0x5f
        {'8', 0, 0},        // 0x60
        {'9', 0, 0},        // 0x61
        {'0', 0, 0},        // 0x62
        {'.', 0, 0},        // 0x63
        {0, 0, 0},          // 0x64
        {0, 0, 0},          // 0x65
        {0, 0, 0},          // 0x66
        {'=', '=', 0},      // 0x67
};

uint8_t usb_hid_keycode_to_ascii_azerty[104][3] = {
        {0, 0, 0},          //  0x00
        {0, 0, 0},          //  0x01
        {0, 0, 0},          // 0x02
        {0, 0, 0},          // 0x03
        {'q', 'Q', 0},      // 0x04
        {'b', 'B', 0},      // 0x05
        {'c', 'C', 0},      // 0x06
        {'d', 'D', 0},      // 0x07
        {'e', 'E', 0},      // 0x08
        {'f', 'F', 0},      // 0x09
        {'g', 'G', 0},      // 0x0a
        {'h', 'H', 0},      // 0x0b
        {'i', 'I', 0},      // 0x0c
        {'j', 'J', 0},      // 0x0d
        {'k', 'K', 0},      // 0x0e
        {'l', 'L', 0},      // 0x0f
        {',', '?', 0},      // 0x10
        {'n', 'N', 0},      // 0x11
        {'o', 'O', 0},      // 0x12
        {'p', 'P', 0},      // 0x13
        {'a', 'A', 0},      // 0x14
        {'r', 'R', 0},      // 0x15
        {'s', 'S', 0},      // 0x16
        {'t', 'T', 0},      // 0x17
        {'u', 'U', 0},      // 0x18
        {'v', 'V', 0},      // 0x19
        {'z', 'Z', 0},      // 0x1a
        {'x', 'X', 0},      // 0x1b
        {'y', 'Y', 0},      // 0x1c
        {'w', 'W', 0},      // 0x1d
        {'&', '1', '|'},    // 0x1e
        {'e', '2', '@'},    // 0x1f  é char not in standard ASCIi
        {'"', '3', '#'},    // 0x20
        {'\'', '4', '{'},   // 0x21
        {'(', '5', '['},    // 0x22
        {0, '6', '^'},      // 0x23 § char not in standard ASCII
        {'e', '7', 0},      // 0x24 è char not in standard ASCII
        {'!', '8', 0},      // 0x25
        {0, '9', '{'},      // 0x26 ç char not in standard ASCII
        {'a', '0', '}'},      // 0x27 à char not in standard ASCII
        {'\r', '\r', 0 },       // 0x28
        {'\x1b', '\x1b', 0 },   // 0x29
        {'\b', '\b', 0 },       // 0x2a
        {'\t', '\t', 0 },       // 0x2b
        {' ', ' ', 0},      // 0x2c
        {')', 0, 0},        // 0x2d ° char not in standard ASCII
        {'-', '_', 0},      // 0x2e
        {'^', 0, '['},      // 0x2f ¨ char not in standard ASCII
        {'$', '*', ']'},    // 0x30
        {0, 0, '`'},        // 0x31 µ,£ char not in standard ASCII
        {0, 0, 0},          // 0x32
        {'m', 'M', 0},      // 0x33
        {0, '%',  0 },      // 0x34 ù,´ char no in standard ASCII
        {0, 0, 0},          // 0x35 ²,³ char not in standard ASCII
        {';', '.', 0},      // 0x36
        {':', '/', 0},      // 0x37
        {'=', '+', '~'},      // 0x38
        {0, 0, 0},          // 0x39
        {0, 0, 0},          // 0x3a
        {0, 0, 0},          // 0x3b
        {0, 0, 0},          // 0x3c
        {0, 0, 0},          // 0x3d
        {0, 0, 0},          // 0x3e
        {0, 0, 0},          // 0x3f
        {0, 0, 0},          // 0x40
        {0, 0, 0},          // 0x41
        {0, 0, 0},          // 0x42
        {0, 0, 0},          // 0x43
        {0, 0, 0},          // 0x44
        {0, 0, 0},          // 0x45
        {0, 0, 0},          // 0x46
        {0, 0, 0},          // 0x47
        {0, 0, 0},          // 0x48
        {0, 0, 0},          // 0x49
        {0, 0, 0},          // 0x4a
        {0, 0, 0},          // 0x4b
        {0, 0, 0},          // 0x4c
        {0, 0, 0},          // 0x4d
        {0, 0, 0},          // 0x4e
        {0, 0, 0},          // 0x4f
        {0, 0, 0},          // 0x50
        {0, 0, 0},          // 0x51
        {0, 0, 0},          // 0x52
        {0, 0, 0},          // 0x53
        {'/', '/', 0},      // 0x54
        {'*', '*', 0},      // 0x55
        {'-', '-', 0},      // 0x56
        {'+', '+', 0},      // 0x57
        {'\r', '\r', 0},    // 0x58
        {'1', 0, 0},        // 0x59
        {'2', 0, 0},        // 0x5a
        {'3', 0, 0},        // 0x5b
        {'4', 0, 0},        // 0x5c
        {'5', '5', 0},      // 0x5d
        {'6', 0, 0},        // 0x5e
        {'7', 0, 0},        // 0x5f
        {'8', 0, 0},        // 0x60
        {'9', 0, 0},        // 0x61
        {'0', 0, 0},        // 0x62
        {'.', 0, 0},        // 0x63
        {0, 0, 0},          // 0x64
        {0, 0, 0},          // 0x65
        {0, 0, 0},          // 0x66
        {'=', '=', 0},      // 0x67
};