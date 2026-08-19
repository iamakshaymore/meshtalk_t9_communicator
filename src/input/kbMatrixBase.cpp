#include "kbMatrixBase.h"
#include "configuration.h"

#ifdef INPUTBROKER_MATRIX_TYPE

const byte keys_cols[] = KEYS_COLS;
const byte keys_rows[] = KEYS_ROWS;

#if INPUTBROKER_MATRIX_TYPE == 1

unsigned char KeyMap[3][sizeof(keys_rows)][sizeof(keys_cols)] = {{{' ', '.', 'm', 'n', 'b', 0xb6},
                                                                  {0x0d, 'l', 'k', 'j', 'h', 0xb4},
                                                                  {'p', 'o', 'i', 'u', 'y', 0xb5},
                                                                  {0x08, 'z', 'x', 'c', 'v', 0xb7},
                                                                  {'a', 's', 'd', 'f', 'g', 0x09},
                                                                  {'q', 'w', 'e', 'r', 't', 0x1a}},
                                                                 {// SHIFT
                                                                  {':', ';', 'M', 'N', 'B', 0xb6},
                                                                  {0x0d, 'L', 'K', 'J', 'H', 0xb4},
                                                                  {'P', 'O', 'I', 'U', 'Y', 0xb5},
                                                                  {0x08, 'Z', 'X', 'C', 'V', 0xb7},
                                                                  {'A', 'S', 'D', 'F', 'G', 0x09},
                                                                  {'Q', 'W', 'E', 'R', 'T', 0x1a}},
                                                                 {// SHIFT-SHIFT
                                                                  {'_', ',', '>', '<', '"', '{'},
                                                                  {'~', '-', '*', '&', '+', '['},
                                                                  {'0', '9', '8', '7', '6', '}'},
                                                                  {'=', '(', ')', '?', '/', ']'},
                                                                  {'!', '@', '#', '$', '%', '\\'},
                                                                  {'1', '2', '3', '4', '5', 0x1a}}};
#elif INPUTBROKER_MATRIX_TYPE == 2
// 4x4 phone-style T9 keypad: digits multi-tap cycle letters, A-D are nav keys,
// */# are Left/Right (this grid has no dedicated arrow keys).
const char KeyMap2[4][4] = {
    {'D', '#', '0', '*'},
    {'C', '9', '8', '7'},
    {'B', '6', '5', '4'},
    {'A', '3', '2', '1'},
};

struct T9Cycle {
    char key;
    const char *chars;
};
const T9Cycle t9Table[] = {
    {'0', " 0"}, {'1', "1.,?!"}, {'2', "abc2"}, {'3', "def3"}, {'4', "ghi4"},
    {'5', "jkl5"}, {'6', "mno6"}, {'7', "pqrs7"}, {'8', "tuv8"}, {'9', "wxyz9"},
};
#define T9_TAP_TIMEOUT_MS 900

static const char *t9CycleFor(char k)
{
    for (auto &entry : t9Table) {
        if (entry.key == k) {
            return entry.chars;
        }
    }
    return nullptr;
}
#endif

KbMatrixBase::KbMatrixBase(const char *name) : concurrency::OSThread(name)
{
    this->_originName = name;
}

int32_t KbMatrixBase::runOnce()
{
    if (!INPUTBROKER_MATRIX_TYPE) {
        // Input device is not requested.
        return disable();
    }

    if (firstTime) {
        // This is the first time the OSThread library has called this function, so do port setup
        firstTime = 0;
        for (byte i = 0; i < sizeof(keys_rows); i++) {
            pinMode(keys_rows[i], OUTPUT);
            digitalWrite(keys_rows[i], HIGH);
        }
        for (byte i = 0; i < sizeof(keys_cols); i++) {
            pinMode(keys_cols[i], INPUT_PULLUP);
        }
    }

    key = 0;

#if INPUTBROKER_MATRIX_TYPE == 1
    {
        // scan for keypresses
        for (byte i = 0; i < sizeof(keys_rows); i++) {
            digitalWrite(keys_rows[i], LOW);
            for (byte j = 0; j < sizeof(keys_cols); j++) {
                if (digitalRead(keys_cols[j]) == LOW) {
                    key = KeyMap[shift][i][j];
                }
            }
            digitalWrite(keys_rows[i], HIGH);
        }
        // debounce
        if (key != prevkey) {
            if (key != 0) {
                LOG_DEBUG("Key 0x%x pressed", key);
                // reset shift now that we have a keypress
                InputEvent e = {};
                e.inputEvent = INPUT_BROKER_NONE;
                e.source = this->_originName;
                switch (key) {
                case 0x1b: // ESC
                    e.inputEvent = INPUT_BROKER_CANCEL;
                    break;
                case 0x08: // Back
                    e.inputEvent = INPUT_BROKER_BACK;
                    e.kbchar = 0;
                    break;
                case 0xb5: // Up
                    e.inputEvent = INPUT_BROKER_UP;
                    break;
                case 0xb6: // Down
                    e.inputEvent = INPUT_BROKER_DOWN;
                    break;
                case 0xb4: // Left
                    e.inputEvent = INPUT_BROKER_LEFT;
                    e.kbchar = 0;
                    break;
                case 0xb7: // Right
                    e.inputEvent = INPUT_BROKER_RIGHT;
                    e.kbchar = 0;
                    break;
                case 0x0d: // Enter
                    e.inputEvent = INPUT_BROKER_SELECT;
                    break;
                case 0x00: // nopress
                    e.inputEvent = INPUT_BROKER_NONE;
                    break;
                case 0x1a: // Shift
                    shift++;
                    if (shift > 2) {
                        shift = 0;
                    }
                    break;
                default: // all other keys
                    e.inputEvent = INPUT_BROKER_ANYKEY;
                    e.kbchar = key;
                    break;
                }
                if (e.inputEvent != INPUT_BROKER_NONE) {
                    this->notifyObservers(&e);
                }
            }
            prevkey = key;
        }
    }
#elif INPUTBROKER_MATRIX_TYPE == 2
    {
        // scan for keypresses
        for (byte i = 0; i < sizeof(keys_rows); i++) {
            digitalWrite(keys_rows[i], LOW);
            for (byte j = 0; j < sizeof(keys_cols); j++) {
                if (digitalRead(keys_cols[j]) == LOW) {
                    key = KeyMap2[i][j];
                }
            }
            digitalWrite(keys_rows[i], HIGH);
        }
        // debounce
        if (key != prevkey) {
            if (key != 0) {
                LOG_DEBUG("T9 key '%c' pressed", key);
                InputEvent e = {};
                e.inputEvent = INPUT_BROKER_NONE;
                e.source = this->_originName;

                switch (key) {
                case 'A':
                    e.inputEvent = INPUT_BROKER_UP;
                    t9LastKey = 0;
                    break;
                case 'B':
                    e.inputEvent = INPUT_BROKER_DOWN;
                    t9LastKey = 0;
                    break;
                case 'C':
                    e.inputEvent = INPUT_BROKER_SELECT;
                    t9LastKey = 0;
                    break;
                case 'D':
                    e.inputEvent = INPUT_BROKER_BACK;
                    t9LastKey = 0;
                    break;
                case '*':
                    e.inputEvent = INPUT_BROKER_LEFT;
                    t9LastKey = 0;
                    break;
                case '#':
                    e.inputEvent = INPUT_BROKER_RIGHT;
                    t9LastKey = 0;
                    break;
                default: {
                    // Multi-tap T9: repeated taps of the same key within
                    // T9_TAP_TIMEOUT_MS cycle through its letters/digit; a
                    // different key or a pause commits the char and starts fresh.
                    const char *cycle = t9CycleFor(key);
                    if (cycle != nullptr) {
                        uint32_t now = millis();
                        if (key == t9LastKey && (now - t9LastMs) < T9_TAP_TIMEOUT_MS) {
                            t9Index++;
                            // undo the char inserted by the previous tap of this key
                            InputEvent back = {};
                            back.inputEvent = INPUT_BROKER_BACK;
                            back.source = this->_originName;
                            this->notifyObservers(&back);
                        } else {
                            t9Index = 0;
                        }
                        uint8_t len = strlen(cycle);
                        if (t9Index >= len) {
                            t9Index = 0;
                        }
                        e.inputEvent = INPUT_BROKER_ANYKEY;
                        e.kbchar = cycle[t9Index];
                        t9LastKey = key;
                        t9LastMs = now;
                    }
                    break;
                }
                }

                if (e.inputEvent != INPUT_BROKER_NONE) {
                    this->notifyObservers(&e);
                }
            }
            prevkey = key;
        }
    }
#else
    LOG_WARN("Unknown kb_model 0x%02x", INPUTBROKER_MATRIX_TYPE);
    return disable();
#endif
    return 50; // Keyscan every 50msec to avoid key bounce
}

#endif // INPUTBROKER_MATRIX_TYPE