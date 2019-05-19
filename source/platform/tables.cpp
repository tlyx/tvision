#define Uses_TKeys
#define Uses_TEvent
#include <tvision/tv.h>

#include <unordered_map>
#include <string>
#include <ncurses.h>
#include <platform.h>
using std::unordered_map;
using std::string;

/* The Turbo Vision library has all its characters encoded in code page 437.
 * While Unicode support is not added, it's better to just translate them
 * with a lookup table. The following table allows translating the characters
 * stored by Turbo Vision into the corresponding UTF-8 mulibyte characters.
 * Taken from https://en.wikipedia.org/wiki/Code_page_437 */

const char* cp437toUtf8[256] = {
    "\0", "☺", "☻", "♥", "♦", "♣", "♠", "•", "◘", "○", "◙", "♂", "♀", "♪", "♫", "☼",
    "►", "◄", "↕", "‼", "¶", "§", "▬", "↨", "↑", "↓", "→", "←", "∟", "↔", "▲", "▼",
    " ", "!", "\"", "#", "$", "%", "&", "'", "(", ")", "*", "+", ",", "-", ".", "/",
    "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", ":", ";", "<", "=", ">", "?",
    "@", "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O",
    "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z", "[", "\\", "]", "^", "_",
    "`", "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o",
    "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z", "{", "|", "}", "~", "⌂",
    "Ç", "ü", "é", "â", "ä", "à", "å", "ç", "ê", "ë", "è", "ï", "î", "ì", "Ä", "Å",
    "É", "æ", "Æ", "ô", "ö", "ò", "û", "ù", "ÿ", "Ö", "Ü", "¢", "£", "¥", "₧", "ƒ",
    "á", "í", "ó", "ú", "ñ", "Ñ", "ª", "º", "¿", "⌐", "¬", "½", "¼", "¡", "«", "»",
    "░", "▒", "▓", "│", "┤", "╡", "╢", "╖", "╕", "╣", "║", "╗", "╝", "╜", "╛", "┐",
    "└", "┴", "┬", "├", "─", "┼", "╞", "╟", "╚", "╔", "╩", "╦", "╠", "═", "╬", "╧",
    "╨", "╤", "╥", "╙", "╘", "╒", "╓", "╫", "╪", "┘", "┌", "█", "▄", "▌", "▐", "▀",
    "α", "ß", "Γ", "π", "Σ", "σ", "µ", "τ", "Φ", "Θ", "Ω", "δ", "∞", "φ", "ε", "∩",
    "≡", "±", "≥", "≤", "⌠", "⌡", "÷", "≈", "°", "∙", "·", "√", "ⁿ", "²", "■", " "
};

/* The reverse version of the table above. Gets initialized at runtime from
 * THardwareInfo's constructor. */

unordered_map<string, char> Utf8toCp437;

/* Turbo Vision is designed to work with BIOS key codes. Mnemonics for some
 * key codes are defined in tkeys.h. Until this is not changed, it is
 * necessary to translate ncurses keys to key codes. */

/* Turbo Vision stores key events in a KeyDownEvent struct, defined in
 * system.h. Its first field is a key code (16 bit), which can be decomposed
 * into the ASCII equivalent (lower byte) and a scan code (higher byte).
 * It has a second field with the state of the modifier keys, which can be
 * retrieved by performing a bit-wise AND with the kbShift, kbCtrlShift and
 * kbAltShift bitmasks. Turbo Vision expects this field to be filled even
 * if the key code is already named Shift/Ctrl/Alt+something. */

/* The support for key combinations is the following:
   - PrintScreen, Break are not likely to be captured by the terminal, but
     Ctrl+C could be used as a replacement of the Ctrl+Break interrupt.
   - Ctrl/Alt+F(n) don't work on the linux console and I strongly advice against
     using them.
   - Ctrl+Letter works, except for ^H, ^I, ^J and ^M, which have a special
     meaning.
   - Alt+Letter/Number seem to work quite well.
   - Ctrl+Backspace/Enter can't be recognized on terminal emulators.
   - Shift/Ctrl+Ins/Del/Home/End/PgDn/PgUp seem to work, too.
   - Arrow keys work, as well as combined with Shift, but Turbo Vision doesn't
     support Ctrl+Up/Down.
   - Tab and Backtab are supported too, although the linux console confuses the
     latter with Alt+Tab.
   - Some other key combinations are supported on terminal but not in Turbo Vision.
 * Still, it's up to your luck that ncurses manages to grab any of these
 * combinations from your terminal application. */

unordered_map<char, KeyDownEvent> fromNonPrintableAscii = {
    { '\x01',       {{kbCtrlA},     kbCtrlShift}},
    { '\x02',       {{kbCtrlB},     kbCtrlShift}},
    { '\x03',       {{kbCtrlC},     kbCtrlShift}},
    { '\x04',       {{kbCtrlD},     kbCtrlShift}},
    { '\x05',       {{kbCtrlE},     kbCtrlShift}},
    { '\x06',       {{kbCtrlF},     kbCtrlShift}},
    { '\x07',       {{kbCtrlG},     kbCtrlShift}},
    { '\x08',       {{kbBack},      0}          }, // ^H, Backspace
    { '\x09',       {{kbTab},       0}          }, // ^I, Tab
    { '\x0A',       {{kbEnter},     0}          }, // ^J, Line Feed
    { '\x0B',       {{kbCtrlK},     kbCtrlShift}},
    { '\x0C',       {{kbCtrlL},     kbCtrlShift}},
    { '\x0D',       {{kbEnter},     0}          }, // ^M, Carriage Return
    { '\x0E',       {{kbCtrlN},     kbCtrlShift}},
    { '\x0F',       {{kbCtrlO},     kbCtrlShift}},
    { '\x10',       {{kbCtrlP},     kbCtrlShift}},
    { '\x11',       {{kbCtrlQ},     kbCtrlShift}},
    { '\x12',       {{kbCtrlR},     kbCtrlShift}},
    { '\x13',       {{kbCtrlS},     kbCtrlShift}},
    { '\x14',       {{kbCtrlT},     kbCtrlShift}},
    { '\x15',       {{kbCtrlU},     kbCtrlShift}},
    { '\x16',       {{kbCtrlV},     kbCtrlShift}},
    { '\x17',       {{kbCtrlW},     kbCtrlShift}},
    { '\x18',       {{kbCtrlX},     kbCtrlShift}},
    { '\x19',       {{kbCtrlY},     kbCtrlShift}},
    { '\x1A',       {{kbCtrlZ},     kbCtrlShift}},
    { '\x1B',       {{kbEsc},       0}          }, // ^[, Escape
    { '\x7F',       {{kbBack},      0}          }  // ^?, Delete
};

unordered_map<char, ushort> AltKeyCode = {
    { ' ', kbAltSpace },
    { 'Q', kbAltQ }, { 'W', kbAltW }, { 'E', kbAltE }, { 'R', kbAltR },
    { 'T', kbAltT }, { 'Y', kbAltY }, { 'U', kbAltU }, { 'I', kbAltI },
    { 'O', kbAltO }, { 'P', kbAltP }, { 'A', kbAltA }, { 'S', kbAltS },
    { 'D', kbAltD }, { 'F', kbAltF }, { 'G', kbAltG }, { 'H', kbAltH },
    { 'J', kbAltJ }, { 'K', kbAltK }, { 'L', kbAltL }, { 'Z', kbAltZ },
    { 'X', kbAltX }, { 'C', kbAltC }, { 'V', kbAltV }, { 'B', kbAltB },
    { 'N', kbAltN }, { 'M', kbAltM }, { '1', kbAlt1 }, { '2', kbAlt2 },
    { '3', kbAlt3 }, { '4', kbAlt4 }, { '5', kbAlt5 }, { '6', kbAlt6 },
    { '7', kbAlt7 }, { '8', kbAlt8 }, { '9', kbAlt9 }, { '0', kbAlt0 },
    { '-', kbAltMinus }, { '=', kbAltEqual }, { '\x08', kbAltBack }
};

unordered_map<int, KeyDownEvent> NcursesInput::fromCursesKeyCode = {
    { KEY_DOWN,         {{kbDown},      0}          },
    { KEY_UP,           {{kbUp},        0}          },
    { KEY_LEFT,         {{kbLeft},      0}          },
    { KEY_RIGHT,        {{kbRight},     0}          },
    { KEY_HOME,         {{kbHome},      0}          },
    { KEY_BACKSPACE,    {{kbBack},      0}          },
    { KEY_DC,           {{kbDel},       0}          },
    { KEY_IC,           {{kbIns},       0}          },
    { KEY_SF,           {{kbDown},      kbShift}    },
    { KEY_SR,           {{kbUp},        kbShift}    },
    { KEY_NPAGE,        {{kbPgDn},      0}          },
    { KEY_PPAGE,        {{kbPgUp},      0}          },
    { KEY_ENTER,        {{kbEnter},     0}          },
    { KEY_BTAB,         {{kbShiftTab},  kbShift}    },
    { KEY_END,          {{kbEnd},       0}          },
    { KEY_SDC,          {{kbShiftDel},  kbShift}    },
    { KEY_SEND,         {{kbEnd},       kbShift}    },
    { KEY_SHOME,        {{kbHome},      kbShift}    },
    { KEY_SIC,          {{kbShiftIns},  kbShift}    },
    { KEY_SLEFT,        {{kbLeft},      kbShift}    },
    { KEY_SRIGHT,       {{kbRight},     kbShift}    },
    // Function keys F1-F12
    { KEY_F0 + 1,       {{kbF1},        0}          },
    { KEY_F0 + 2,       {{kbF2},        0}          },
    { KEY_F0 + 3,       {{kbF3},        0}          },
    { KEY_F0 + 4,       {{kbF4},        0}          },
    { KEY_F0 + 5,       {{kbF5},        0}          },
    { KEY_F0 + 6,       {{kbF6},        0}          },
    { KEY_F0 + 7,       {{kbF7},        0}          },
    { KEY_F0 + 8,       {{kbF8},        0}          },
    { KEY_F0 + 9,       {{kbF9},        0}          },
    { KEY_F0 + 10,      {{kbF10},       0}          },
    { KEY_F0 + 11,      {{kbF11},       0}          },
    { KEY_F0 + 12,      {{kbF12},       0}          },
    // Shift+F1-F12
    { KEY_F0 + 13,      {{kbShiftF1},   kbShift}    },
    { KEY_F0 + 14,      {{kbShiftF2},   kbShift}    },
    { KEY_F0 + 15,      {{kbShiftF3},   kbShift}    },
    { KEY_F0 + 16,      {{kbShiftF4},   kbShift}    },
    { KEY_F0 + 17,      {{kbShiftF5},   kbShift}    },
    { KEY_F0 + 18,      {{kbShiftF6},   kbShift}    },
    { KEY_F0 + 19,      {{kbShiftF7},   kbShift}    },
    { KEY_F0 + 20,      {{kbShiftF8},   kbShift}    },
    { KEY_F0 + 21,      {{kbShiftF9},   kbShift}    },
    { KEY_F0 + 22,      {{kbShiftF10},  kbShift}    },
    { KEY_F0 + 23,      {{kbShiftF11},  kbShift}    },
    { KEY_F0 + 24,      {{kbShiftF12},  kbShift}    },
    /* Linux console support for function keys ends here, so please
     * avoid using any of the following: */
    // Ctrl+F1-F12
    { KEY_F0 + 25,      {{kbCtrlF1},    kbCtrlShift}},
    { KEY_F0 + 26,      {{kbCtrlF2},    kbCtrlShift}},
    { KEY_F0 + 27,      {{kbCtrlF3},    kbCtrlShift}},
    { KEY_F0 + 28,      {{kbCtrlF4},    kbCtrlShift}},
    { KEY_F0 + 29,      {{kbCtrlF5},    kbCtrlShift}},
    { KEY_F0 + 30,      {{kbCtrlF6},    kbCtrlShift}},
    { KEY_F0 + 31,      {{kbCtrlF7},    kbCtrlShift}},
    { KEY_F0 + 32,      {{kbCtrlF8},    kbCtrlShift}},
    { KEY_F0 + 33,      {{kbCtrlF9},    kbCtrlShift}},
    { KEY_F0 + 34,      {{kbCtrlF10},   kbCtrlShift}},
    { KEY_F0 + 35,      {{kbCtrlF11},   kbCtrlShift}},
    { KEY_F0 + 36,      {{kbCtrlF12},   kbCtrlShift}},
    // Ctrl+Shift+F(n) supported by ncurses but not Turbo Vision
    // Alt+F1-F12
    { KEY_F0 + 49,      {{kbAltF1},     kbAltShift} },
    { KEY_F0 + 50,      {{kbAltF2},     kbAltShift} },
    { KEY_F0 + 51,      {{kbAltF3},     kbAltShift} },
    { KEY_F0 + 52,      {{kbAltF4},     kbAltShift} },
    { KEY_F0 + 53,      {{kbAltF5},     kbAltShift} },
    { KEY_F0 + 54,      {{kbAltF6},     kbAltShift} },
    { KEY_F0 + 55,      {{kbAltF7},     kbAltShift} },
    { KEY_F0 + 56,      {{kbAltF8},     kbAltShift} },
    { KEY_F0 + 57,      {{kbAltF9},     kbAltShift} },
    { KEY_F0 + 58,      {{kbAltF10},    kbAltShift} },
    { KEY_F0 + 59,      {{kbAltF11},    kbAltShift} },
    { KEY_F0 + 60,      {{kbAltF12},    kbAltShift} }
};

unordered_map<string, KeyDownEvent> NcursesInput::fromCursesHighKey = {
    /* These keys are identified by name. The int value is not known
     * at compilation time.
     * ncurses supports all sorts of Shift/Ctrl/Alt combinations for these
     * but Turbo Vision doesn't support them. */
    { "kDC5",       {{kbCtrlDel},       kbCtrlShift}},
    { "kEND5",      {{kbCtrlEnd},       kbCtrlShift}},
    { "kHOM5",      {{kbCtrlHome},      kbCtrlShift}},
    { "kIC5",       {{kbCtrlIns},       kbCtrlShift}},
    { "kLFT5",      {{kbCtrlLeft},      kbCtrlShift}},
    { "kNXT5",      {{kbCtrlPgDn},      kbCtrlShift}},
    { "kPRV5",      {{kbCtrlPgUp},      kbCtrlShift}},
    { "kRIT5",      {{kbCtrlRight},     kbCtrlShift}}
    // Surprisingly, no Ctrl+Up/Down support in Turbo Vision.
};

