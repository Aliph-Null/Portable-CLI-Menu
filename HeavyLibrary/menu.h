#pragma once

/* Standard includes */
#include <iostream>
#include <string>
#include <cmath>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <locale>
#include <codecvt>
#include <functional>

#ifdef _WIN32
    #include <windows.h>
    #include <conio.h>
#else
    #include <sys/ioctl.h>
    #include <unistd.h>
    #include <conio.h> /* if you use a platform-specific getch implementation, keep it */
#endif

/* Console cursor macro (kept for compatibility) */
#ifdef _WIN32
    #define cursor(x, y) SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), (COORD){x, y})
#else
    #define cursor(x, y) std::cout << "\033[" << (y+1) << ";" << (x+1) << "H"
#endif

/* ANSI sequences and utility macros (kept as macros for compatibility) */
#define START_SEQUENCE "\033["
#define ESC_COLOR_CODE "\033["
#define FOREGROUND_SEQUENCE "38;2;"
#define BACKGROUND_SEQUENCE "48;2;"
#define SEQUENCE_ARG_SEPARATOR ";"
#define CLOSE_SEQUENCE "m"

#define SET_BOLD "\033[1m"
#define RESET_BOLD "\033[22m"

#define SET_BLINKING "\033[5m"
#define RESET_BLINKING "\033[25m"

#define RESET_ALL "\033[0m"

#define ERASE_CONSOLE "\033c"

/* Key codes in use (kept for compatibility) */
#define KEY_UP 72
#define KEY_DOWN 80

/* --------------------------------------------------------------------------
   Simple POD 'color' representing an RGB triple (0..255)
   -------------------------------------------------------------------------- */
/*
 * color - a simple RGB container. Public members for convenient aggregate init.
 */
struct color {
    unsigned char r = 0;
    unsigned char g = 0;
    unsigned char b = 0;
};

/* --------------------------------------------------------------------------
   HSL to RGB conversion
   - kept as a free function to minimize API changes
   -------------------------------------------------------------------------- */
/*
 * HSLtoRGB(h, s, l) -> color
 *
 * h: hue in degrees (any value; normalized internally to [0,360))
 * s: saturation [0..1]
 * l: lightness  [0..1]
 *
 * Returns an 8-bit per channel color.
 */
inline color HSLtoRGB(double h, double s, double l) {
    h = std::fmod(h, 360.0);
    if (h < 0.0) h += 360.0;

    const double c = (1.0 - std::fabs(2.0 * l - 1.0)) * s;
    const double x = c * (1.0 - std::fabs(std::fmod(h / 60.0, 2.0) - 1.0));
    const double m = l - c / 2.0;

    double r1 = 0.0, g1 = 0.0, b1 = 0.0;

    if (h < 60.0)       { r1 = c; g1 = x; b1 = 0.0; }
    else if (h < 120.0) { r1 = x; g1 = c; b1 = 0.0; }
    else if (h < 180.0) { r1 = 0.0; g1 = c; b1 = x; }
    else if (h < 240.0) { r1 = 0.0; g1 = x; b1 = c; }
    else if (h < 300.0) { r1 = x; g1 = 0.0; b1 = c; }
    else                { r1 = c; g1 = 0.0; b1 = x; }

    color result;
    result.r = static_cast<unsigned char>(std::round((r1 + m) * 255.0));
    result.g = static_cast<unsigned char>(std::round((g1 + m) * 255.0));
    result.b = static_cast<unsigned char>(std::round((b1 + m) * 255.0));
    return result;
}

/* --------------------------------------------------------------------------
   c_pixel - color/formatting for a character cell
   - previously everything was public; now fields are private with accessors
   - preserves constructors and existing public API names where possible
   -------------------------------------------------------------------------- */
/*
 * c_pixel
 *
 * Represents the foreground/background colors and attributes (bold/blink).
 */
class c_pixel {
public:
    /* Constructors */
    explicit c_pixel(unsigned char r = 255, unsigned char g = 255, unsigned char b = 255)
        : foreground_{r, g, b}, background_{0,0,0}, blinking_{false}, bold_{false} {}

    c_pixel(unsigned char fr, unsigned char fg, unsigned char fb,
            unsigned char br, unsigned char bg, unsigned char bb)
        : foreground_{fr, fg, fb}, background_{br, bg, bb}, blinking_{false}, bold_{false} {}

    explicit c_pixel(color frgb)
        : foreground_(frgb), background_{0,0,0}, blinking_{false}, bold_{false} {}

    c_pixel(color frgb, color brgb)
        : foreground_(frgb), background_(brgb), blinking_{false}, bold_{false} {}

    /* Getters (const and non-const) */
    const color& foreground() const noexcept { return foreground_; }
    color& foreground() noexcept { return foreground_; }

    const color& background() const noexcept { return background_; }
    color& background() noexcept { return background_; }

    bool blinking() const noexcept { return blinking_; }
    bool bold() const noexcept { return bold_; }

    /* Setters */
    void setForeground(unsigned char r, unsigned char g, unsigned char b) {
        foreground_ = { r, g, b };
    }
    void setForeground(color rgb) {
        foreground_ = rgb;
    }

    void setBackground(unsigned char r, unsigned char g, unsigned char b) {
        background_ = { r, g, b };
    }
    void setBackground(color rgb) {
        background_ = rgb;
    }

    void setPixelColor(unsigned char fr, unsigned char fg, unsigned char fb,
                       unsigned char br, unsigned char bg, unsigned char bb) {
        foreground_ = { fr, fg, fb };
        background_ = { br, bg, bb };
    }
    void setPixelColor(color frgb, color brgb) {
        foreground_ = frgb;
        background_ = brgb;
    }

    void setBlinking(bool on) noexcept { blinking_ = on; }
    void setBold(bool on) noexcept { bold_ = on; }

    /*
     * setTextColor()
     *
     * Writes the ANSI (or equivalent) color/attribute sequences to std::cout.
     * This preserves the original behavior (emits ESC sequences).
     */
    void setTextColor() const {
        std::cout << RESET_ALL;

        std::cout << ESC_COLOR_CODE << FOREGROUND_SEQUENCE
                  << static_cast<int>(foreground_.r) << SEQUENCE_ARG_SEPARATOR
                  << static_cast<int>(foreground_.g) << SEQUENCE_ARG_SEPARATOR
                  << static_cast<int>(foreground_.b)
                  << CLOSE_SEQUENCE;

        std::cout << ESC_COLOR_CODE << BACKGROUND_SEQUENCE
                  << static_cast<int>(background_.r) << SEQUENCE_ARG_SEPARATOR
                  << static_cast<int>(background_.g) << SEQUENCE_ARG_SEPARATOR
                  << static_cast<int>(background_.b)
                  << CLOSE_SEQUENCE;

        if (blinking_) std::cout << SET_BLINKING;
        if (bold_)     std::cout << SET_BOLD;
    }

private:
    color foreground_;
    color background_;
    bool blinking_;
    bool bold_;
};

/* --------------------------------------------------------------------------
   UTF-32 -> UTF-8 conversion utility
   - kept inline and static-like to remain header-only
   -------------------------------------------------------------------------- */
/*
 * to_utf8(u32string) -> string
 *
 * Simple conversion from a std::u32string to a UTF-8 std::string.
 * This implementation is minimal and matches the behavior in the original file.
 */
static inline std::string to_utf8(const std::u32string & src) {
    std::string result;
    result.reserve(src.size() * 4);

    for (char32_t c : src) {
        if (c <= 0x7F) {
            result.push_back(static_cast<char>(c));
        } else if (c <= 0x7FF) {
            result.push_back(static_cast<char>(0xC0 | (c >> 6)));
            result.push_back(static_cast<char>(0x80 | (c & 0x3F)));
        } else if (c <= 0xFFFF) {
            result.push_back(static_cast<char>(0xE0 | (c >> 12)));
            result.push_back(static_cast<char>(0x80 | ((c >> 6) & 0x3F)));
            result.push_back(static_cast<char>(0x80 | (c & 0x3F)));
        } else {
            result.push_back(static_cast<char>(0xF0 | (c >> 18)));
            result.push_back(static_cast<char>(0x80 | ((c >> 12) & 0x3F)));
            result.push_back(static_cast<char>(0x80 | ((c >> 6) & 0x3F)));
            result.push_back(static_cast<char>(0x80 | (c & 0x3F)));
        }
    }
    return result;
}

/* --------------------------------------------------------------------------
   AvailableFonts - enum
   -------------------------------------------------------------------------- */
/*
 * AvailableFonts::EnumFonts - identifies built-in fonts by index.
 */
namespace AvailableFonts {
    enum EnumFonts {
        Mono12,
        Bloody,
        AnsiShadow,
        Aligator2
    };
}

/* --------------------------------------------------------------------------
   rainbowUV - example color function
   - kept as-is but inline
   -------------------------------------------------------------------------- */
/*
 * rainbowUV(x, y) -> c_pixel
 * Produces a bold colored c_pixel using HSLtoRGB; the formula matches the original.
 */
inline c_pixel rainbowUV(double x, double y) {
    color new_color = HSLtoRGB(x * y * 360.0 * 1.0, 0.7, 0.7);
    c_pixel result(new_color);
    result.setBold(true);
    return result;
}

/* --------------------------------------------------------------------------
   Character - representation of a pseudo-font character (multi-line)
   -------------------------------------------------------------------------- */
/*
 * Character
 *
 * Contains:
 *  - representation: the ASCII character that this glyph represents
 *  - width/height: logical dimensions (width could be inferred but kept)
 *  - data: vector of u32 strings (each row)
 */
class Character {
public:
    Character(char r, int w, int h, const std::vector<std::u32string>& d)
        : representation(r), width(w), height(h), data(d) {}

    /* Print glyph to stdout (UTF-8 conversion) */
    void print() const {
        for (const auto & row : data) {
            std::cout << to_utf8(row) << '\n';
        }
    }

    /* Public members kept for compatibility with the rest of the code */
    char representation;
    int width;
    int height;
    std::vector<std::u32string> data;
};

/* --------------------------------------------------------------------------
   Font - collection of Character glyphs
   -------------------------------------------------------------------------- */
/*
 * Font
 *
 * A small wrapper around a vector of Character glyphs.
 * Operator[] overloads return pointers to glyphs by ASCII identifier, or nullptr.
 */
class Font {
public:
    explicit Font(const std::vector<Character>& arr) : characters(arr) {}

    void printChar(int index) const {
        if (index >= 0 && index < static_cast<int>(characters.size()))
            characters[index].print();
        else
            std::cerr << "Invalid character index: " << index << std::endl;
    }

    void printChar(char c) const {
        for (const auto & ch : characters) {
            if (c == ch.representation) {
                ch.print();
                break;
            }
        }
    }

    void printString(const std::string & str) const {
        for (char c : str) printChar(c);
    }

    const Character* operator[](char identifier) const {
        for (const auto& ch : characters)
            if (ch.representation == identifier)
                return &ch;
        return nullptr;
    }

    Character* operator[](char identifier) {
        for (auto& ch : characters)
            if (ch.representation == identifier)
                return &ch;
        return nullptr;
    }

    std::vector<Character> characters;
};

/* --------------------------------------------------------------------------
   Built-in fonts container placeholder
   - keep same shape as original file: a const vector named 'fonts'
   - replace the placeholder with your actual font definitions
   -------------------------------------------------------------------------- */
/*
 * fonts - placeholder for your full font data.
 * Replace the comment inside the braces with the actual Font initializers.
 */
const std::vector < Font > fonts = {
  Font({
    {
      'A',
      7,
      10,
      {
        U"    ▄▄    ",
        U"   ████   ",
        U"   ████   ",
        U"  ██  ██  ",
        U"  ██████  ",
        U" ▄██  ██▄ ",
        U" ▀▀    ▀▀ ",
        U"          ",
        U"          ",
      }
    },

    {
      'B',
      7,
      10,
      {
        U" ▄▄▄▄▄▄   ",
        U" ██▀▀▀▀██ ",
        U" ██    ██ ",
        U" ███████  ",
        U" ██    ██ ",
        U" ██▄▄▄▄██ ",
        U" ▀▀▀▀▀▀▀  ",
        U"          ",
        U"          ",
      }
    },

    {
      'C',
      7,
      10,
      {
        U"    ▄▄▄▄  ",
        U"  ██▀▀▀▀█ ",
        U" ██▀      ",
        U" ██       ",
        U" ██▄      ",
        U"  ██▄▄▄▄█ ",
        U"    ▀▀▀▀  ",
        U"          ",
        U"          ",
      }
    },
    {
      'D',
      7,
      10,
      {
        U" ▄▄▄▄▄    ",
        U" ██▀▀▀██  ",
        U" ██    ██ ",
        U" ██    ██ ",
        U" ██    ██ ",
        U" ██▄▄▄██  ",
        U" ▀▀▀▀▀    ",
        U"          ",
        U"          ",
      }
    },
    {
      'E',
      7,
      10,
      {
        U" ▄▄▄▄▄▄▄▄ ",
        U" ██▀▀▀▀▀▀ ",
        U" ██       ",
        U" ███████  ",
        U" ██       ",
        U" ██▄▄▄▄▄▄ ",
        U" ▀▀▀▀▀▀▀▀ ",
        U"          ",
        U"          ",
      }
    },
    {
      'F',
      7,
      10,
      {
        U" ▄▄▄▄▄▄▄▄ ",
        U" ██▀▀▀▀▀▀ ",
        U" ██       ",
        U" ███████  ",
        U" ██       ",
        U" ██       ",
        U" ▀▀       ",
        U"          ",
        U"          ",
      }
    },
    {
      'G',
      7,
      10,
      {
        U"    ▄▄▄▄  ",
        U"  ██▀▀▀▀█ ",
        U" ██       ",
        U" ██  ▄▄▄▄ ",
        U" ██  ▀▀██ ",
        U"  ██▄▄▄██ ",
        U"    ▀▀▀▀  ",
        U"          ",
        U"          ",
      }
    },
    {
      'H',
      7,
      10,
      {
        U" ▄▄    ▄▄ ",
        U" ██    ██ ",
        U" ██    ██ ",
        U" ████████ ",
        U" ██    ██ ",
        U" ██    ██ ",
        U" ▀▀    ▀▀ ",
        U"          ",
        U"          ",
      }
    },
    {
      'I',
      7,
      10,
      {
        U"  ▄▄▄▄▄▄  ",
        U"  ▀▀██▀▀  ",
        U"    ██    ",
        U"    ██    ",
        U"    ██    ",
        U"  ▄▄██▄▄  ",
        U"  ▀▀▀▀▀▀  ",
        U"          ",
        U"          ",
      }
    },
    {
      'J',
      7,
      10,
      {
        U"    ▄▄▄▄▄ ",
        U"    ▀▀▀██ ",
        U"       ██ ",
        U"       ██ ",
        U"       ██ ",
        U" █▄▄▄▄▄██ ",
        U"  ▀▀▀▀▀   ",
        U"          ",
        U"          ",
      }
    },
    {
      'K',
      7,
      10,
      {
        U" ▄▄   ▄▄▄ ",
        U" ██  ██▀  ",
        U" ██▄██    ",
        U" █████    ",
        U" ██  ██▄  ",
        U" ██   ██▄ ",
        U" ▀▀    ▀▀ ",
        U"          ",
        U"          ",
      }
    },
    {
      'L',
      7,
      10,
      {
        U" ▄▄       ",
        U" ██       ",
        U" ██       ",
        U" ██       ",
        U" ██       ",
        U" ██▄▄▄▄▄▄ ",
        U" ▀▀▀▀▀▀▀▀ ",
        U"          ",
        U"          ",
      }
    },
    {
      'M',
      7,
      10,
      {
        U" ▄▄▄  ▄▄▄ ",
        U" ███  ███ ",
        U" ████████ ",
        U" ██ ██ ██ ",
        U" ██ ▀▀ ██ ",
        U" ██    ██ ",
        U" ▀▀    ▀▀ ",
        U"          ",
        U"          ",
      }
    },
    {
      'N',
      7,
      10,
      {
        U" ▄▄▄   ▄▄ ",
        U" ███   ██ ",
        U" ██▀█  ██ ",
        U" ██ ██ ██ ",
        U" ██  █▄██ ",
        U" ██   ███ ",
        U" ▀▀   ▀▀▀ ",
        U"          ",
        U"          ",
      }
    },
    {
      'O',
      7,
      10,
      {
        U"   ▄▄▄▄   ",
        U"  ██▀▀██  ",
        U" ██    ██ ",
        U" ██    ██ ",
        U" ██    ██ ",
        U"  ██▄▄██  ",
        U"   ▀▀▀▀   ",
        U"          ",
        U"          ",
      }
    },
    {
      'P',
      7,
      10,
      {
        U" ▄▄▄▄▄▄   ",
        U" ██▀▀▀▀█▄ ",
        U" ██    ██ ",
        U" ██████▀  ",
        U" ██       ",
        U" ██       ",
        U" ▀▀       ",
        U"          ",
        U"          ",
      }
    },
    {
      'Q',
      7,
      10,
      {
        U"   ▄▄▄▄   ",
        U"  ██▀▀██  ",
        U" ██    ██ ",
        U" ██    ██ ",
        U" ██    ██ ",
        U"  ██▄▄██▀ ",
        U"   ▀▀▀██  ",
        U"       ▀  ",
        U"          ",
      }
    },
    {
      'R',
      7,
      10,
      {
        U" ▄▄▄▄▄▄   ",
        U" ██▀▀▀▀██ ",
        U" ██    ██ ",
        U" ███████  ",
        U" ██  ▀██▄ ",
        U" ██    ██ ",
        U" ▀▀    ▀▀▀",
        U"          ",
        U"          ",
      }
    },
    {
      'S',
      7,
      10,
      {
        U"   ▄▄▄▄   ",
        U" ▄█▀▀▀▀█  ",
        U" ██▄      ",
        U"  ▀████▄  ",
        U"      ▀██ ",
        U" █▄▄▄▄▄█▀ ",
        U"  ▀▀▀▀▀   ",
        U"          ",
        U"          ",
      }
    },
    {
      'T',
      7,
      10,
      {
        U" ▄▄▄▄▄▄▄▄ ",
        U" ▀▀▀██▀▀▀ ",
        U"    ██    ",
        U"    ██    ",
        U"    ██    ",
        U"    ██    ",
        U"    ▀▀    ",
        U"          ",
        U"          ",
      }
    },
    {
      'U',
      7,
      10,
      {
        U" ▄▄    ▄▄ ",
        U" ██    ██ ",
        U" ██    ██ ",
        U" ██    ██ ",
        U" ██    ██ ",
        U" ▀██▄▄██▀ ",
        U"   ▀▀▀▀   ",
        U"          ",
        U"          ",
      }
    },
    {
      'V',
      7,
      10,
      {
        U" ▄▄    ▄▄ ",
        U" ▀██  ██▀ ",
        U"  ██  ██  ",
        U"  ██  ██  ",
        U"   ████   ",
        U"   ████   ",
        U"   ▀▀▀▀   ",
        U"          ",
        U"          ",
      }
    },
    {
      'W',
      7,
      10,
      {
        U"▄▄      ▄▄",
        U"██      ██",
        U"▀█▄ ██ ▄█▀",
        U" ██ ██ ██ ",
        U" ███▀▀███ ",
        U" ███  ███ ",
        U" ▀▀▀  ▀▀▀ ",
        U"          ",
        U"          ",
      }
    },
    {
      'X',
      7,
      10,
      {
        U" ▄▄▄  ▄▄▄ ",
        U"  ██▄▄██  ",
        U"   ████   ",
        U"    ██    ",
        U"   ████   ",
        U"  ██  ██  ",
        U" ▀▀▀  ▀▀▀ ",
        U"          ",
        U"          ",
      }
    },
    {
      'Y',
      7,
      10,
      {
        U"▄▄▄    ▄▄▄",
        U" ██▄  ▄██ ",
        U"  ██▄▄██  ",
        U"   ▀██▀   ",
        U"    ██    ",
        U"    ██    ",
        U"    ▀▀    ",
        U"          ",
        U"          ",
      }
    },
    {
      'Z',
      7,
      10,
      {
        U" ▄▄▄▄▄▄▄▄ ",
        U" ▀▀▀▀▀███ ",
        U"     ██▀  ",
        U"   ▄██▀   ",
        U"  ▄██     ",
        U" ███▄▄▄▄▄ ",
        U" ▀▀▀▀▀▀▀▀ ",
        U"          ",
        U"          ",
      }
    },
    {
      '1',
      7,
      10,
      {
        U"   ▄▄▄    ",
        U"  █▀██    ",
        U"    ██    ",
        U"    ██    ",
        U"    ██    ",
        U" ▄▄▄██▄▄▄ ",
        U" ▀▀▀▀▀▀▀▀ ",
        U"          ",
        U"          ",
      }
    },
    {
      '2',
      7,
      10,
      {
        U"  ▄▄▄▄▄   ",
        U" █▀▀▀▀██▄ ",
        U"       ██ ",
        U"     ▄█▀  ",
        U"   ▄█▀    ",
        U" ▄██▄▄▄▄▄ ",
        U" ▀▀▀▀▀▀▀▀ ",
        U"          ",
        U"          ",
      }
    },
    {
      '3',
      7,
      10,
      {
        U"  ▄▄▄▄▄   ",
        U" █▀▀▀▀██▄ ",
        U"      ▄██ ",
        U"   █████  ",
        U"      ▀██ ",
        U" █▄▄▄▄██▀ ",
        U"  ▀▀▀▀▀   ",
        U"          ",
        U"          ",
      }
    },
    {
      '4',
      7,
      10,
      {
        U"     ▄▄▄  ",
        U"    ▄███  ",
        U"   █▀ ██  ",
        U" ▄█▀  ██  ",
        U" ████████ ",
        U"      ██  ",
        U"      ▀▀  ",
        U"          ",
        U"          ",
      }
    },
    {
      '5',
      7,
      10,
      {
        U" ▄▄▄▄▄▄▄  ",
        U" ██▀▀▀▀▀  ",
        U" ██▄▄▄▄   ",
        U" █▀▀▀▀██▄ ",
        U"       ██ ",
        U" █▄▄▄▄██▀ ",
        U"  ▀▀▀▀▀   ",
        U"          ",
        U"          ",
      }
    },
    {
      '6',
      7,
      10,
      {
        U"   ▄▄▄▄   ",
        U"  ██▀▀▀█  ",
        U" ██ ▄▄▄   ",
        U" ███▀▀██▄ ",
        U" ██    ██ ",
        U" ▀██▄▄██▀ ",
        U"   ▀▀▀▀   ",
        U"          ",
        U"          ",
      }
    },
    {
      '7',
      7,
      10,
      {
        U" ▄▄▄▄▄▄▄▄ ",
        U" ▀▀▀▀▀███ ",
        U"     ▄██  ",
        U"     ██   ",
        U"    ██    ",
        U"   ██     ",
        U"  ▀▀      ",
        U"          ",
        U"          ",
      }
    },
    {
      '8',
      7,
      10,
      {
        U"   ▄▄▄▄   ",
        U" ▄██▀▀██▄ ",
        U" ██▄  ▄██ ",
        U"  ██████  ",
        U" ██▀  ▀██ ",
        U" ▀██▄▄██▀ ",
        U"   ▀▀▀▀   ",
        U"          ",
        U"          ",
      }
    },
    {
      '9',
      7,
      10,
      {
        U"   ▄▄▄▄   ",
        U" ▄██▀▀██▄ ",
        U" ██    ██ ",
        U" ▀██▄▄███ ",
        U"   ▀▀▀ ██ ",
        U"  █▄▄▄██  ",
        U"   ▀▀▀▀   ",
        U"          ",
        U"          ",
      }
    },
    {
      '0',
      7,
      10,
      {
        U"   ▄▄▄▄   ",
        U"  ██▀▀██  ",
        U" ██    ██ ",
        U" ██ ██ ██ ",
        U" ██    ██ ",
        U"  ██▄▄██  ",
        U"   ▀▀▀▀   ",
        U"          ",
        U"          ",
      }
    },
    {
      '!',
      7,
      10,
      {
        U"    ▄▄    ",
        U"    ██    ",
        U"    ██    ",
        U"    ██    ",
        U"    ▀▀    ",
        U"    ▄▄    ",
        U"    ▀▀    ",
        U"          ",
        U"          ",
      }
    },
    {
      '@',
      7,
      10,
      {
        U"          ",
        U"  ▄████▄  ",
        U"▄██▀  ▀██ ",
        U"██ ▄█████ ",
        U"██ ██▄▄██ ",
        U"▀█▄ ▀▀▀▀▀ ",
        U" ▀██▄▄▄█▄ ",
        U"   ▀▀▀▀▀  ",
        U"          ",
      }
    },
    {
      '#',
      7,
      10,
      {
        U"    ▄▄ ▄▄ ",
        U"   ▄█  ██ ",
        U" █████████",
        U"  ▄█  ██  ",
        U"█████████ ",
        U" ▄█  ██   ",
        U" ▀▀  ▀    ",
        U"          ",
        U"          ",
      }
    },
    {
      '$',
      7,
      10,
      {
        U"    ▄     ",
        U"  ▄▄█▄▄   ",
        U" ██▀█▀▀   ",
        U" ▀███▄▄   ",
        U"    █▀██  ",
        U" █▄▄█▄██  ",
        U"  ▀▀█▀▀   ",
        U"    ▀     ",
        U"          ",
      }
    },
    {
      '%',
      7,
      10,
      {
        U" ▄▄▄      ",
        U"█   █     ",
        U"▀▄▄▄▀  ▄  ",
        U"   ▄ ▀    ",
        U" ▀  ▄▀▀▀▄ ",
        U"    █   █ ",
        U"     ▀▀▀  ",
        U"          ",
        U"          ",
      }
    },
    {
      '^',
      7,
      10,
      {
        U"   ▄▄▄    ",
        U" ▄██▀██▄  ",
        U"▀▀▀   ▀▀▀ ",
        U"          ",
        U"          ",
        U"          ",
        U"          ",
        U"          ",
        U"          ",
      }
    },
    {
      '&',
      7,
      10,
      {
        U"   ▄▄▄▄   ",
        U"  ██▀▀▀█  ",
        U"  ▀█▄     ",
        U"  ████▄ ▄▄",
        U" ██  ▀█▄██",
        U" ▀██▄▄███ ",
        U"   ▀▀▀▀▀▀▀",
        U"          ",
        U"          ",
      }
    },
    {
      '*',
      7,
      10,
      {
        U"    ▄     ",
        U" ▄▄ █ ▄▄  ",
        U"  █████   ",
        U" ▀▀ █ ▀▀  ",
        U"    ▀     ",
        U"          ",
        U"          ",
        U"          ",
        U"          ",
      }
    },
    {
      '(',
      7,
      10,
      {
        U"     ▄▄   ",
        U"    ██    ",
        U"   ▄█▀    ",
        U"   ██     ",
        U"   ██     ",
        U"   ▀█▄    ",
        U"    ██    ",
        U"     ▀▀   ",
        U"          ",
      }
    },
    {
      ')',
      7,
      10,
      {
        U"  ▄▄      ",
        U"   ██     ",
        U"   ▀█▄    ",
        U"    ██    ",
        U"    ██    ",
        U"   ▄█▀    ",
        U"   ██     ",
        U"  ▀▀      ",
        U"          ",
      }
    },
    {
      '[',
      7,
      10,
      {
        U"   ▄▄▄▄   ",
        U"   ██     ",
        U"   ██     ",
        U"   ██     ",
        U"   ██     ",
        U"   ██     ",
        U"   ██     ",
        U"   ▀▀▀▀   ",
        U"          ",
      }
    },
    {
      ']',
      7,
      10,
      {
        U"  ▄▄▄▄    ",
        U"    ██    ",
        U"    ██    ",
        U"    ██    ",
        U"    ██    ",
        U"    ██    ",
        U"    ██    ",
        U"  ▀▀▀▀    ",
        U"          ",
      }
    },
    {
      '?',
      7,
      10,
      {
        U"  ▄▄▄▄▄   ",
        U" █▀▀▀▀██  ",
        U"     ▄█▀  ",
        U"   ▄██▀   ",
        U"   ██     ",
        U"   ▄▄     ",
        U"   ▀▀     ",
        U"          ",
        U"          ",
      }
    },
    {
      '>',
      7,
      10,
      {
        U"          ",
        U"          ",
        U" █▄▄▄     ",
        U"   ▀▀▀█▄▄ ",
        U"   ▄▄▄█▀▀ ",
        U" █▀▀▀     ",
        U"          ",
        U"          ",
        U"          ",
      }
    },
    {
      '<',
      7,
      10,
      {
        U"          ",
        U"          ",
        U"     ▄▄▄█ ",
        U" ▄▄█▀▀▀   ",
        U" ▀▀█▄▄▄   ",
        U"     ▀▀▀█ ",
        U"          ",
        U"          ",
        U"          ",
      }
    }
  }),
  Font({
    {
      'A',
      10,
      10,
      {
        U" ▄▄▄      ",
        U"▒████▄    ",
        U"▒██  ▀█▄  ",
        U"░██▄▄▄▄██ ",
        U" ▓█   ▓██▒",
        U" ▒▒   ▓▒█░",
        U"  ▒   ▒▒ ░",
        U"  ░   ▒   ",
        U"      ░  ░",
        U"          ",
      }
    },
    {
      'B',
      10,
      8,
      {
        U" ▄▄▄▄   ",
        U"▓█████▄ ",
        U"▒██▒ ▄██",
        U"▒██░█▀  ",
        U"░▓█  ▀█▓",
        U"░▒▓███▀▒",
        U"▒░▒   ░ ",
        U" ░    ░ ",
        U" ░      ",
        U"      ░ ",
      }
    },
    {
      'C',
      10,
      9,
      {
        U" ▄████▄  ",
        U"▒██▀ ▀█  ",
        U"▒▓█    ▄ ",
        U"▒▓▓▄ ▄██▒",
        U"▒ ▓███▀ ░",
        U"░ ░▒ ▒  ░",
        U"  ░  ▒   ",
        U"░        ",
        U"░ ░      ",
        U"░        ",
      }
    },
    {
      'D',
      10,
      8,
      {
        U"▓█████▄ ",
        U"▒██▀ ██▌",
        U"░██   █▌",
        U"░▓█▄   ▌",
        U"░▒████▓ ",
        U" ▒▒▓  ▒ ",
        U" ░ ▒  ▒ ",
        U" ░ ░  ░ ",
        U"   ░    ",
        U" ░      ",
      }
    },
    {
      'E',
      10,
      7,
      {
        U"▓█████ ",
        U"▓█   ▀ ",
        U"▒███   ",
        U"▒▓█  ▄ ",
        U"░▒████▒",
        U"░░ ▒░ ░",
        U" ░ ░  ░",
        U"   ░   ",
        U"   ░  ░",
        U"       ",
      }
    },
    {
      'F',
      10,
      8,
      {
        U"  █████▒",
        U"▓██   ▒ ",
        U"▒████ ░ ",
        U"░▓█▒  ░ ",
        U"░▒█░    ",
        U" ▒ ░    ",
        U" ░      ",
        U" ░ ░    ",
        U"        ",
        U"        ",
      }
    },
    {
      'G',
      10,
      8,
      {
        U"  ▄████ ",
        U" ██▒ ▀█▒",
        U"▒██░▄▄▄░",
        U"░▓█  ██▓",
        U"░▒▓███▀▒",
        U" ░▒   ▒ ",
        U"  ░   ░ ",
        U"░ ░   ░ ",
        U"      ░ ",
        U"        ",
      }
    },
    {
      'H',
      10,
      8,
      {
        U" ██░ ██ ",
        U"▓██░ ██▒",
        U"▒██▀▀██░",
        U"░▓█ ░██ ",
        U"░▓█▒░██▓",
        U" ▒ ░░▒░▒",
        U" ▒ ░▒░ ░",
        U" ░  ░░ ░",
        U" ░  ░  ░",
        U"        ",
      }
    },
    {
      'I',
      10,
      4,
      {
        U" ██▓",
        U"▓██▒",
        U"▒██▒",
        U"░██░",
        U"░██░",
        U"░▓  ",
        U" ▒ ░",
        U" ▒ ░",
        U" ░  ",
        U"    ",
      }
    },
    {
      'J',
      10,
      9,
      {
        U" ▄▄▄██▀▀▀",
        U"   ▒██   ",
        U"   ░██   ",
        U"▓██▄██▓  ",
        U" ▓███▒   ",
        U" ▒▓▒▒░   ",
        U" ▒ ░▒░   ",
        U" ░ ░ ░   ",
        U" ░   ░   ",
        U"         ",
      }
    },
    {
      'K',
      10,
      7,
      {
        U" ██ ▄█▀",
        U" ██▄█▒ ",
        U"▓███▄░ ",
        U"▓██ █▄ ",
        U"▒██▒ █▄",
        U"▒ ▒▒ ▓▒",
        U"░ ░▒ ▒░",
        U"░ ░░ ░ ",
        U"░  ░   ",
        U"       ",
      }
    },
    {
      'L',
      10,
      8,
      {
        U" ██▓    ",
        U"▓██▒    ",
        U"▒██░    ",
        U"▒██░    ",
        U"░██████▒",
        U"░ ▒░▓  ░",
        U"░ ░ ▒  ░",
        U"  ░ ░   ",
        U"    ░  ░",
        U"        ",
      }
    },
    {
      'M',
      10,
      11,
      {
        U" ███▄ ▄███▓",
        U"▓██▒▀█▀ ██▒",
        U"▓██    ▓██░",
        U"▒██    ▒██ ",
        U"▒██▒   ░██▒",
        U"░ ▒░   ░  ░",
        U"░  ░      ░",
        U"░      ░   ",
        U"       ░   ",
        U"           ",
      }
    },
    {
      'N',
      10,
      11,
      {
        U" ███▄    █ ",
        U" ██ ▀█   █ ",
        U"▓██  ▀█ ██▒",
        U"▓██▒  ▐▌██▒",
        U"▒██░   ▓██░",
        U"░ ▒░   ▒ ▒ ",
        U"░ ░░   ░ ▒░",
        U"   ░   ░ ░ ",
        U"         ░ ",
        U"           ",
      }
    },
    {
      'O',
      10,
      9,
      {
        U" ▒█████  ",
        U"▒██▒  ██▒",
        U"▒██░  ██▒",
        U"▒██   ██░",
        U"░ ████▓▒░",
        U"░ ▒░▒░▒░ ",
        U"  ░ ▒ ▒░ ",
        U"░ ░ ░ ▒  ",
        U"    ░ ░  ",
        U"         ",
      }
    },
    {
      'P',
      10,
      9,
      {
        U" ██▓███  ",
        U"▓██░  ██▒",
        U"▓██░ ██▓▒",
        U"▒██▄█▓▒ ▒",
        U"▒██▒ ░  ░",
        U"▒▓▒░ ░  ░",
        U"░▒ ░     ",
        U"░░       ",
        U"         ",
        U"         ",
      }
    },
    {
      'Q',
      10,
      9,
      {
        U"  █████  ",
        U"▒██▓  ██▒",
        U"▒██▒  ██░",
        U"░██  █▀ ░",
        U"░▒███▒█▄ ",
        U"░░ ▒▒░ ▒ ",
        U" ░ ▒░  ░ ",
        U"   ░   ░ ",
        U"    ░    ",
        U"         ",
      }
    },
    {
      'R',
      10,
      9,
      {
        U" ██▀███  ",
        U"▓██ ▒ ██▒",
        U"▓██ ░▄█ ▒",
        U"▒██▀▀█▄  ",
        U"░██▓ ▒██▒",
        U"░ ▒▓ ░▒▓░",
        U"  ░▒ ░ ▒░",
        U"  ░░   ░ ",
        U"   ░     ",
        U"         ",
      }
    },
    {
      'S',
      10,
      9,
      {
        U"  ██████ ",
        U"▒██    ▒ ",
        U"░ ▓██▄   ",
        U"  ▒   ██▒",
        U"▒██████▒▒",
        U"▒ ▒▓▒ ▒ ░",
        U"░ ░▒  ░ ░",
        U"░  ░  ░  ",
        U"      ░  ",
        U"         ",
      }
    },
    {
      'T',
      10,
      9,
      {
        U"▄▄▄█████▓",
        U"▓  ██▒ ▓▒",
        U"▒ ▓██░ ▒░",
        U"░ ▓██▓ ░ ",
        U"  ▒██▒ ░ ",
        U"  ▒ ░░   ",
        U"    ░    ",
        U"  ░      ",
        U"         ",
        U"         ",
      }
    },
    {
      'U',
      10,
      9,
      {
        U" █    ██ ",
        U" ██  ▓██▒",
        U"▓██  ▒██░",
        U"▓▓█  ░██░",
        U"▒▒█████▓ ",
        U"░▒▓▒ ▒ ▒ ",
        U"░░▒░ ░ ░ ",
        U" ░░░ ░ ░ ",
        U"   ░     ",
        U"         ",
      }
    },
    {
      'V',
      10,
      9,
      {
        U" ██▒   █▓",
        U"▓██░   █▒",
        U" ▓██  █▒░",
        U"  ▒██ █░░",
        U"   ▒▀█░  ",
        U"   ░ ▐░  ",
        U"   ░ ░░  ",
        U"     ░░  ",
        U"      ░  ",
        U"     ░   ",
      }
    },
    {
      'W',
      10,
      9,
      {
        U" █     █░",
        U"▓█░ █ ░█░",
        U"▒█░ █ ░█ ",
        U"░█░ █ ░█ ",
        U"░░██▒██▓ ",
        U"░ ▓░▒ ▒  ",
        U"  ▒ ░ ░  ",
        U"  ░   ░  ",
        U"    ░    ",
        U"         ",
      }
    },
    {
      'X',
      10,
      9,
      {
        U"▒██   ██▒",
        U"▒▒ █ █ ▒░",
        U"░░  █   ░",
        U" ░ █ █ ▒ ",
        U"▒██▒ ▒██▒",
        U"▒▒ ░ ░▓ ░",
        U"░░   ░▒ ░",
        U" ░    ░  ",
        U" ░    ░  ",
        U"         ",
      }
    },
    {
      'Y',
      10,
      9,
      {
        U"▓██   ██▓",
        U" ▒██  ██▒",
        U"  ▒██ ██░",
        U"  ░ ▐██▓░",
        U"  ░ ██▒▓░",
        U"   ██▒▒▒ ",
        U" ▓██ ░▒░ ",
        U" ▒ ▒ ░░  ",
        U" ░ ░     ",
        U" ░ ░     ",
      }
    },
    {
      'Z',
      10,
      9,
      {
        U"▒███████▒",
        U"▒ ▒ ▒ ▄▀░",
        U"░ ▒ ▄▀▒░ ",
        U"  ▄▀▒   ░",
        U"▒███████▒",
        U"░▒▒ ▓░▒░▒",
        U"░░▒ ▒ ░ ▒",
        U"░ ░ ░ ░ ░",
        U"  ░ ░    ",
        U"░        ",
      }
    }
  }),
  Font({
    {
      'A',
      7,
      8,
      {
        U" █████╗ ",
        U"██╔══██╗",
        U"███████║",
        U"██╔══██║",
        U"██║  ██║",
        U"╚═╝  ╚═╝",
        U"        ",
      }
    },
    {
      'B',
      7,
      8,
      {
        U"██████╗ ",
        U"██╔══██╗",
        U"██████╔╝",
        U"██╔══██╗",
        U"██████╔╝",
        U"╚═════╝ ",
        U"        ",
      }
    },
    {
      'C',
      7,
      8,
      {
        U" ██████╗",
        U"██╔════╝",
        U"██║     ",
        U"██║     ",
        U"╚██████╗",
        U" ╚═════╝",
        U"        ",
      }
    },
    {
      'D',
      7,
      8,
      {
        U"██████╗ ",
        U"██╔══██╗",
        U"██║  ██║",
        U"██║  ██║",
        U"██████╔╝",
        U"╚═════╝ ",
        U"        ",
      }
    },
    {
      'E',
      7,
      8,
      {
        U"███████╗",
        U"██╔════╝",
        U"█████╗  ",
        U"██╔══╝  ",
        U"███████╗",
        U"╚══════╝",
        U"        ",
      }
    },
    {
      'F',
      7,
      8,
      {
        U"███████╗",
        U"██╔════╝",
        U"█████╗  ",
        U"██╔══╝  ",
        U"██║     ",
        U"╚═╝     ",
        U"        ",
      }
    },
    {
      'G',
      7,
      9,
      {
        U" ██████╗ ",
        U"██╔════╝ ",
        U"██║  ███╗",
        U"██║   ██║",
        U"╚██████╔╝",
        U" ╚═════╝ ",
        U"         ",
      }
    },
    {
      'H',
      7,
      8,
      {
        U"██╗  ██╗",
        U"██║  ██║",
        U"███████║",
        U"██╔══██║",
        U"██║  ██║",
        U"╚═╝  ╚═╝",
        U"        ",
      }
    },
    {
      'I',
      7,
      3,
      {
        U"██╗",
        U"██║",
        U"██║",
        U"██║",
        U"██║",
        U"╚═╝",
        U"   ",
      }
    },
    {
      'J',
      7,
      8,
      {
        U"     ██╗",
        U"     ██║",
        U"     ██║",
        U"██   ██║",
        U"╚█████╔╝",
        U" ╚════╝ ",
        U"        ",
      }
    },
    {
      'K',
      7,
      8,
      {
        U"██╗  ██╗",
        U"██║ ██╔╝",
        U"█████╔╝ ",
        U"██╔═██╗ ",
        U"██║  ██╗",
        U"╚═╝  ╚═╝",
        U"        ",
      }
    },
    {
      'L',
      7,
      8,
      {
        U"██╗     ",
        U"██║     ",
        U"██║     ",
        U"██║     ",
        U"███████╗",
        U"╚══════╝",
        U"        ",
      }
    },
    {
      'M',
      7,
      11,
      {
        U"███╗   ███╗",
        U"████╗ ████║",
        U"██╔████╔██║",
        U"██║╚██╔╝██║",
        U"██║ ╚═╝ ██║",
        U"╚═╝     ╚═╝",
        U"           ",
      }
    },
    {
      'N',
      7,
      10,
      {
        U"███╗   ██╗",
        U"████╗  ██║",
        U"██╔██╗ ██║",
        U"██║╚██╗██║",
        U"██║ ╚████║",
        U"╚═╝  ╚═══╝",
        U"          ",
      }
    },
    {
      'O',
      7,
      9,
      {
        U" ██████╗ ",
        U"██╔═══██╗",
        U"██║   ██║",
        U"██║   ██║",
        U"╚██████╔╝",
        U" ╚═════╝ ",
        U"         ",
      }
    },
    {
      'P',
      7,
      8,
      {
        U"██████╗ ",
        U"██╔══██╗",
        U"██████╔╝",
        U"██╔═══╝ ",
        U"██║     ",
        U"╚═╝     ",
        U"        ",
      }
    },
    {
      'Q',
      7,
      9,
      {
        U" ██████╗ ",
        U"██╔═══██╗",
        U"██║   ██║",
        U"██║▄▄ ██║",
        U"╚██████╔╝",
        U" ╚══▀▀═╝ ",
        U"         ",
      }
    },
    {
      'R',
      7,
      8,
      {
        U"██████╗ ",
        U"██╔══██╗",
        U"██████╔╝",
        U"██╔══██╗",
        U"██║  ██║",
        U"╚═╝  ╚═╝",
        U"        ",
      }
    },
    {
      'S',
      7,
      8,
      {
        U"███████╗",
        U"██╔════╝",
        U"███████╗",
        U"╚════██║",
        U"███████║",
        U"╚══════╝",
        U"        ",
      }
    },
    {
      'T',
      7,
      9,
      {
        U"████████╗",
        U"╚══██╔══╝",
        U"   ██║   ",
        U"   ██║   ",
        U"   ██║   ",
        U"   ╚═╝   ",
        U"         ",
      }
    },
    {
      'U',
      7,
      9,
      {
        U"██╗   ██╗",
        U"██║   ██║",
        U"██║   ██║",
        U"██║   ██║",
        U"╚██████╔╝",
        U" ╚═════╝ ",
        U"         ",
      }
    },
    {
      'V',
      7,
      9,
      {
        U"██╗   ██╗",
        U"██║   ██║",
        U"██║   ██║",
        U"╚██╗ ██╔╝",
        U" ╚████╔╝ ",
        U"  ╚═══╝  ",
        U"         ",
      }
    },
    {
      'W',
      7,
      10,
      {
        U"██╗    ██╗",
        U"██║    ██║",
        U"██║ █╗ ██║",
        U"██║███╗██║",
        U"╚███╔███╔╝",
        U" ╚══╝╚══╝ ",
        U"          ",
      }
    },
    {
      'X',
      7,
      8,
      {
        U"██╗  ██╗",
        U"╚██╗██╔╝",
        U" ╚███╔╝ ",
        U" ██╔██╗ ",
        U"██╔╝ ██╗",
        U"╚═╝  ╚═╝",
        U"        ",
      }
    },
    {
      'Y',
      7,
      9,
      {
        U"██╗   ██╗",
        U"╚██╗ ██╔╝",
        U" ╚████╔╝ ",
        U"  ╚██╔╝  ",
        U"   ██║   ",
        U"   ╚═╝   ",
        U"         ",
      }
    },
    {
      'Z',
      7,
      8,
      {
        U"███████╗",
        U"╚══███╔╝",
        U"  ███╔╝ ",
        U" ███╔╝  ",
        U"███████╗",
        U"╚══════╝",
        U"        ",
      }
    },
    {
      '1',
      7,
      4,
      {
        U" ██╗",
        U"███║",
        U"╚██║",
        U" ██║",
        U" ██║",
        U" ╚═╝",
        U"    ",
      }
    },
    {
      '2',
      7,
      8,
      {
        U"██████╗ ",
        U"╚════██╗",
        U" █████╔╝",
        U"██╔═══╝ ",
        U"███████╗",
        U"╚══════╝",
        U"        ",
      }
    },
    {
      '3',
      7,
      8,
      {
        U"██████╗ ",
        U"╚════██╗",
        U" █████╔╝",
        U" ╚═══██╗",
        U"██████╔╝",
        U"╚═════╝ ",
        U"        ",
      }
    },
    {
      '4',
      7,
      8,
      {
        U"██╗  ██╗",
        U"██║  ██║",
        U"███████║",
        U"╚════██║",
        U"     ██║",
        U"     ╚═╝",
        U"        ",
      }
    },
    {
      '5',
      7,
      8,
      {
        U"███████╗",
        U"██╔════╝",
        U"███████╗",
        U"╚════██║",
        U"███████║",
        U"╚══════╝",
        U"        ",
      }
    },
    {
      '6',
      7,
      9,
      {
        U" ██████╗ ",
        U"██╔════╝ ",
        U"███████╗ ",
        U"██╔═══██╗",
        U"╚██████╔╝",
        U" ╚═════╝ ",
        U"         ",
      }
    },
    {
      '7',
      7,
      8,
      {
        U"███████╗",
        U"╚════██║",
        U"    ██╔╝",
        U"   ██╔╝ ",
        U"   ██║  ",
        U"   ╚═╝  ",
        U"        ",
      }
    },
    {
      '8',
      7,
      8,
      {
        U" █████╗ ",
        U"██╔══██╗",
        U"╚█████╔╝",
        U"██╔══██╗",
        U"╚█████╔╝",
        U" ╚════╝ ",
        U"        ",
      }
    },
    {
      '9',
      7,
      8,
      {
        U" █████╗ ",
        U"██╔══██╗",
        U"╚██████║",
        U" ╚═══██║",
        U" █████╔╝",
        U" ╚════╝ ",
        U"        ",
      }
    },
    {
      '0',
      7,
      9,
      {
        U" ██████╗ ",
        U"██╔═████╗",
        U"██║██╔██║",
        U"████╔╝██║",
        U"╚██████╔╝",
        U" ╚═════╝ ",
        U"         ",
      }
    },
    {
      '!',
      7,
      3,
      {
        U"██╗",
        U"██║",
        U"██║",
        U"╚═╝",
        U"██╗",
        U"╚═╝",
        U"   ",
      }
    },
    {
      '@',
      7,
      9,
      {
        U" ██████╗ ",
        U"██╔═══██╗",
        U"██║██╗██║",
        U"██║██║██║",
        U"╚█║████╔╝",
        U" ╚╝╚═══╝ ",
        U"         ",
      }
    },
    {
      '#',
      7,
      9,
      {
        U" ██╗ ██╗ ",
        U"████████╗",
        U"╚██╔═██╔╝",
        U"████████╗",
        U"╚██╔═██╔╝",
        U" ╚═╝ ╚═╝ ",
        U"         ",
      }
    },
    {
      '$',
      7,
      8,
      {
        U"▄▄███▄▄·",
        U"██╔════╝",
        U"███████╗",
        U"╚════██║",
        U"███████║",
        U"╚═▀▀▀══╝",
        U"        ",
      }
    },
    {
      '%',
      7,
      7,
      {
        U"██╗ ██╗",
        U"╚═╝██╔╝",
        U"  ██╔╝ ",
        U" ██╔╝  ",
        U"██╔╝██╗",
        U"╚═╝ ╚═╝",
        U"       ",
      }
    },
    {
      '^',
      7,
      6,
      {
        U" ███╗ ",
        U"██╔██╗",
        U"╚═╝╚═╝",
        U"      ",
        U"      ",
        U"      ",
        U"      ",
      }
    },
    {
      '&',
      7,
      9,
      {
        U"   ██╗   ",
        U"   ██║   ",
        U"████████╗",
        U"██╔═██╔═╝",
        U"██████║  ",
        U"╚═════╝  ",
        U"         ",
      }
    },
    {
      '*',
      7,
      6,
      {
        U"      ",
        U"▄ ██╗▄",
        U" ████╗",
        U"▀╚██╔▀",
        U"  ╚═╝ ",
        U"      ",
        U"      ",
      }
    },
    {
      '(',
      7,
      4,
      {
        U" ██╗",
        U"██╔╝",
        U"██║ ",
        U"██║ ",
        U"╚██╗",
        U" ╚═╝",
        U"    ",
      }
    },
    {
      ')',
      7,
      4,
      {
        U"██╗ ",
        U"╚██╗",
        U" ██║",
        U" ██║",
        U"██╔╝",
        U"╚═╝ ",
        U"    ",
      }
    },
    {
      '[',
      7,
      4,
      {
        U"███╗",
        U"██╔╝",
        U"██║ ",
        U"██║ ",
        U"███╗",
        U"╚══╝",
        U"    ",
      }
    },
    {
      ']',
      7,
      4,
      {
        U"███╗",
        U"╚██║",
        U" ██║",
        U" ██║",
        U"███║",
        U"╚══╝",
        U"    ",
      }
    },
    {
      '?',
      7,
      8,
      {
        U"██████╗ ",
        U"╚════██╗",
        U"  ▄███╔╝",
        U"  ▀▀══╝ ",
        U"  ██╗   ",
        U"  ╚═╝   ",
        U"        ",
      }
    },
    {
      '>',
      7,
      5,
      {
        U"██╗  ",
        U"╚██╗ ",
        U" ╚██╗",
        U" ██╔╝",
        U"██╔╝ ",
        U"╚═╝  ",
        U"     ",
      }
    },
    {
      '<',
      7,
      5,
      {
        U"  ██╗",
        U" ██╔╝",
        U"██╔╝ ",
        U"╚██╗ ",
        U" ╚██╗",
        U"  ╚═╝",
        U"     ",
      }
    }
  }),
  Font({
    {
      'A',
      7,
      12,
      {
        U"    :::     ",
        U"  :+: :+:   ",
        U" +:+   +:+  ",
        U"+#++:++#++: ",
        U"+#+     +#+ ",
        U"#+#     #+# ",
        U"###     ### ",
      }
    },
    {
      'B',
      7,
      11,
      {
        U":::::::::  ",
        U":+:    :+: ",
        U"+:+    +:+ ",
        U"+#++:++#+  ",
        U"+#+    +#+ ",
        U"#+#    #+# ",
        U"#########  ",
      }
    },
    {
      'C',
      7,
      11,
      {
        U" ::::::::  ",
        U":+:    :+: ",
        U"+:+        ",
        U"+#+        ",
        U"+#+        ",
        U"#+#    #+# ",
        U" ########  ",
      }
    },
    {
      'D',
      7,
      11,
      {
        U":::::::::  ",
        U":+:    :+: ",
        U"+:+    +:+ ",
        U"+#+    +:+ ",
        U"+#+    +#+ ",
        U"#+#    #+# ",
        U"#########  ",
      }
    },
    {
      'E',
      7,
      11,
      {
        U":::::::::: ",
        U":+:        ",
        U"+:+        ",
        U"+#++:++#   ",
        U"+#+        ",
        U"#+#        ",
        U"########## ",
      }
    },
    {
      'F',
      7,
      10,
      {
        U"::::::::::",
        U":+:       ",
        U"+:+       ",
        U":#::+::#  ",
        U"+#+       ",
        U"#+#       ",
        U"###       ",
      }
    },
    {
      'G',
      7,
      11,
      {
        U" ::::::::  ",
        U":+:    :+: ",
        U"+:+        ",
        U":#:        ",
        U"+#+   +#+# ",
        U"#+#    #+# ",
        U" ########  ",
      }
    },
    {
      'H',
      7,
      11,
      {
        U":::    ::: ",
        U":+:    :+: ",
        U"+:+    +:+ ",
        U"+#++:++#++ ",
        U"+#+    +#+ ",
        U"#+#    #+# ",
        U"###    ### ",
      }
    },
    {
      'I',
      7,
      12,
      {
        U"::::::::::: ",
        U"    :+:     ",
        U"    +:+     ",
        U"    +#+     ",
        U"    +#+     ",
        U"    #+#     ",
        U"########### ",
      }
    },
    {
      'J',
      7,
      12,
      {
        U"::::::::::: ",
        U"    :+:     ",
        U"    +:+     ",
        U"    +#+     ",
        U"    +#+     ",
        U"#+# #+#     ",
        U" #####      ",
      }
    },
    {
      'K',
      7,
      11,
      {
        U":::    ::: ",
        U":+:   :+:  ",
        U"+:+  +:+   ",
        U"+#++:++    ",
        U"+#+  +#+   ",
        U"#+#   #+#  ",
        U"###    ### ",
      }
    },
    {
      'L',
      7,
      11,
      {
        U":::        ",
        U":+:        ",
        U"+:+        ",
        U"+#+        ",
        U"+#+        ",
        U"#+#        ",
        U"########## ",
      }
    },
    {
      'M',
      7,
      14,
      {
        U"::::    ::::  ",
        U"+:+:+: :+:+:+ ",
        U"+:+ +:+:+ +:+ ",
        U"+#+  +:+  +#+ ",
        U"+#+       +#+ ",
        U"#+#       #+# ",
        U"###       ### ",
      }
    },
    {
      'N',
      7,
      12,
      {
        U"::::    ::: ",
        U":+:+:   :+: ",
        U":+:+:+  +:+ ",
        U"+#+ +:+ +#+ ",
        U"+#+  +#+#+# ",
        U"#+#   #+#+# ",
        U"###    #### ",
      }
    },
    {
      'O',
      7,
      11,
      {
        U" ::::::::  ",
        U":+:    :+: ",
        U"+:+    +:+ ",
        U"+#+    +:+ ",
        U"+#+    +#+ ",
        U"#+#    #+# ",
        U" ########  ",
      }
    },
    {
      'P',
      7,
      11,
      {
        U":::::::::  ",
        U":+:    :+: ",
        U"+:+    +:+ ",
        U"+#++:++#+  ",
        U"+#+        ",
        U"#+#        ",
        U"###        ",
      }
    },
    {
      'Q',
      7,
      12,
      {
        U" ::::::::   ",
        U":+:    :+:  ",
        U"+:+    +:+  ",
        U"+#+    +:+  ",
        U"+#+  # +#+  ",
        U"#+#   +#+   ",
        U" ###### ### ",
      }
    },
    {
      'R',
      7,
      11,
      {
        U":::::::::  ",
        U":+:    :+: ",
        U"+:+    +:+ ",
        U"+#++:++#:  ",
        U"+#+    +#+ ",
        U"#+#    #+# ",
        U"###    ### ",
      }
    },
    {
      'S',
      7,
      10,
      {
        U" :::::::: ",
        U":+:    :+:",
        U"+:+       ",
        U"+#++:++#++",
        U"       +#+",
        U"#+#    #+#",
        U" ######## ",
      }
    },
    {
      'T',
      7,
      12,
      {
        U"::::::::::: ",
        U"    :+:     ",
        U"    +:+     ",
        U"    +#+     ",
        U"    +#+     ",
        U"    #+#     ",
        U"    ###     ",
      }
    },
    {
      'U',
      7,
      11,
      {
        U":::    ::: ",
        U":+:    :+: ",
        U"+:+    +:+ ",
        U"+#+    +:+ ",
        U"+#+    +#+ ",
        U"#+#    #+# ",
        U" ########  ",
      }
    },
    {
      'V',
      7,
      12,
      {
        U":::     ::: ",
        U":+:     :+: ",
        U"+:+     +:+ ",
        U"+#+     +:+ ",
        U" +#+   +#+  ",
        U"  #+#+#+#   ",
        U"    ###     ",
      }
    },
    {
      'W',
      7,
      14,
      {
        U":::       ::: ",
        U":+:       :+: ",
        U"+:+       +:+ ",
        U"+#+  +:+  +#+ ",
        U"+#+ +#+#+ +#+ ",
        U" #+#+# #+#+#  ",
        U"  ###   ###   ",
      }
    },
    {
      'X',
      7,
      11,
      {
        U":::    ::: ",
        U":+:    :+: ",
        U" +:+  +:+  ",
        U"  +#++:+   ",
        U" +#+  +#+  ",
        U"#+#    #+# ",
        U"###    ### ",
      }
    },
    {
      'Y',
      7,
      10,
      {
        U":::   ::: ",
        U":+:   :+: ",
        U" +:+ +:+  ",
        U"  +#++:   ",
        U"   +#+    ",
        U"   #+#    ",
        U"   ###    ",
      }
    },
    {
      'Z',
      7,
      10,
      {
        U"::::::::: ",
        U"     :+:  ",
        U"    +:+   ",
        U"   +#+    ",
        U"  +#+     ",
        U" #+#      ",
        U"######### ",
      }
    },
    {
      '1',
      7,
      8,
      {
        U"  :::   ",
        U":+:+:   ",
        U"  +:+   ",
        U"  +#+   ",
        U"  +#+   ",
        U"  #+#   ",
        U"####### ",
      }
    },
    {
      '2',
      7,
      11,
      {
        U" ::::::::  ",
        U":+:    :+: ",
        U"      +:+  ",
        U"    +#+    ",
        U"  +#+      ",
        U" #+#       ",
        U"########## ",
      }
    },
    {
      '3',
      7,
      11,
      {
        U" ::::::::  ",
        U":+:    :+: ",
        U"       +:+ ",
        U"    +#++:  ",
        U"       +#+ ",
        U"#+#    #+# ",
        U" ########  ",
      }
    },
    {
      '4',
      7,
      11,
      {
        U"    :::    ",
        U"   :+:     ",
        U"  +:+ +:+  ",
        U" +#+  +:+  ",
        U"+#+#+#+#+#+",
        U"      #+#  ",
        U"      ###  ",
      }
    },
    {
      '5',
      7,
      11,
      {
        U":::::::::: ",
        U":+:    :+: ",
        U"+:+        ",
        U"+#++:++#+  ",
        U"       +#+ ",
        U"#+#    #+# ",
        U" ########  ",
      }
    },
    {
      '6',
      7,
      11,
      {
        U" ::::::::  ",
        U":+:    :+: ",
        U"+:+        ",
        U"+#++:++#+  ",
        U"+#+    +#+ ",
        U"#+#    #+# ",
        U" ########  ",
      }
    },
    {
      '7',
      7,
      12,
      {
        U"::::::::::: ",
        U":+:     :+: ",
        U"       +:+  ",
        U"      +#+   ",
        U"     +#+    ",
        U"    #+#     ",
        U"    ###     ",
      }
    },
    {
      '8',
      7,
      11,
      {
        U" ::::::::  ",
        U":+:    :+: ",
        U"+:+    +:+ ",
        U" +#++:++#  ",
        U"+#+    +#+ ",
        U"#+#    #+# ",
        U" ########  ",
      }
    },
    {
      '9',
      7,
      11,
      {
        U" ::::::::  ",
        U":+:    :+: ",
        U"+:+    +:+ ",
        U" +#++:++#+ ",
        U"       +#+ ",
        U"#+#    #+# ",
        U" ########  ",
      }
    },
    {
      '0',
      7,
      10,
      {
        U" :::::::  ",
        U":+:   :+: ",
        U"+:+  :+:+ ",
        U"+#+ + +:+ ",
        U"+#+#  +#+ ",
        U"#+#   #+# ",
        U" #######  ",
      }
    },
    {
      '!',
      7,
      4,
      {
        U"::: ",
        U":+: ",
        U"+:+ ",
        U"+#+ ",
        U"+#+ ",
        U"    ",
        U"### ",
      }
    },
    {
      '@',
      7,
      18,
      {
        U"   :::::::::::    ",
        U" :+: :+:+:+:+:+:  ",
        U"+:+ +:+   +:+ +:+ ",
        U"+#+ +:+   +#+ +:+ ",
        U"+#+ +#+   +#+ +#+ ",
        U" #+# #+#+#+#+#+   ",
        U"   #####          ",
      }
    },
    {
      '#',
      7,
      16,
      {
        U"   :::   :::    ",
        U"   :+:   :+:    ",
        U"+:+:+:+:+:+:+:+ ",
        U"   +#+   +:+    ",
        U"+#+#+#+#+#+#+#+ ",
        U"   #+#   #+#    ",
        U"   ###   ###    ",
      }
    },
    {
      '$',
      7,
      12,
      {
        U"     :::    ",
        U"  :+:+:+:+: ",
        U"+:+  +:+    ",
        U"  +#++:++#+ ",
        U"     +#+ +#+",
        U"  #+#+#+#+# ",
        U"     ###    ",
      }
    },
    {
      '%',
      7,
      15,
      {
        U":::   :::      ",
        U":+:   :+:      ",
        U"      +:+      ",
        U"      +#+      ",
        U"      +#+      ",
        U"      #+#   #+#",
        U"      ###   ###",
      }
    },
    {
      '^',
      7,
      11,
      {
        U"    :::    ",
        U"  :+: :+:  ",
        U"+:+     +:+",
        U"           ",
        U"           ",
        U"           ",
        U"           ",
      }
    },
    {
      '&',
      7,
      13,
      {
        U" :::::::     ",
        U":+:   :+:    ",
        U" +:+ +:+     ",
        U"  +#++:  ++# ",
        U" +#+ +#+#+#  ",
        U"#+#   #+#+   ",
        U" ##########  ",
      }
    },
    {
      '*',
      7,
      14,
      {
        U"              ",
        U" :+:     :+:  ",
        U"   +:+ +:+    ",
        U"+#++:++#++:++ ",
        U"   +#+ +#+    ",
        U" #+#     #+#  ",
        U"              ",
      }
    },
    {
      '(',
      7,
      6,
      {
        U"  ::: ",
        U" :+:  ",
        U"+:+   ",
        U"+#+   ",
        U"+#+   ",
        U" #+#  ",
        U"  ### ",
      }
    },
    {
      ')',
      7,
      6,
      {
        U":::   ",
        U" :+:  ",
        U"  +:+ ",
        U"  +#+ ",
        U"  +#+ ",
        U" #+#  ",
        U"###   ",
      }
    },
    {
      '[',
      7,
      7,
      {
        U":::::: ",
        U":+:    ",
        U"+:+    ",
        U"+#+    ",
        U"+#+    ",
        U"#+#    ",
        U"###### ",
      }
    },
    {
      ']',
      7,
      7,
      {
        U":::::: ",
        U"   :+: ",
        U"   +:+ ",
        U"   +#+ ",
        U"   +#+ ",
        U"   #+# ",
        U"###### ",
      }
    },
    {
      '?',
      7,
      11,
      {
        U" ::::::::: ",
        U":+:     :+:",
        U"       +:+ ",
        U"      +#+  ",
        U"    +#+    ",
        U"           ",
        U"    ###   #",
      }
    },
    {
      '>',
      7,
      7,
      {
        U":::    ",
        U" :+:   ",
        U"  +:+  ",
        U"   +#+ ",
        U"  +#+  ",
        U" #+#   ",
        U"##     ",
      }
    },
    {
      '<',
      7,
      7,
      {
        U"   ::: ",
        U"  :+:  ",
        U" +:+   ",
        U"+#+    ",
        U" +#+   ",
        U"  #+#  ",
        U"   ### ",
      }
    }
  })
};

/* --------------------------------------------------------------------------
   coords - simple integer 2D coordinate
   -------------------------------------------------------------------------- */
struct coords {
    int x;
    int y;
};

/* --------------------------------------------------------------------------
   UI_Option - a selectable option with callbacks
   -------------------------------------------------------------------------- */
/*
 * UI_Option
 *
 * Holds:
 *  - a display string (text)
 *  - a list of function pointers (void(*)()) to call
 *  - optional override color for display
 */
class UI_Option {
public:
    UI_Option(const std::string& str, void(*f)())
        : text(str), overwriteColor_huh(false), overwiteColor(255,255,255) {
        Subscribe(f);
    }

    void Subscribe(void(*func)()) {
        functions.emplace_back(func);
    }

    void Unsubscribe(void(*func)()) {
        auto it = std::find(functions.begin(), functions.end(), func);
        if (it != functions.end())
            functions.erase(it);
    }

    void Call() const {
        for (auto f : functions)
            if (f) f();
    }

    /* Display text */
    std::string text;

    /* Callback list */
    std::vector<void(*)()> functions;

    /* Optional override color (kept name/behavior) */
    bool overwriteColor_huh;
    c_pixel overwiteColor;
};

/* --------------------------------------------------------------------------
   UI_Option_Bar - formatting pieces for the option list
   -------------------------------------------------------------------------- */
struct UI_Option_Bar {
    std::string top;
    std::string before_option;
    std::string after_option;
    std::string between_gap;
    std::string selected;
    bool gap = false;
};

/* --------------------------------------------------------------------------
   subMenu - a menu with options and appearance settings
   -------------------------------------------------------------------------- */
/*
 * subMenu
 *
 * - name: menu title
 * - options: vector of UI_Option
 * - selectedOption: index
 * - colors: selected/default/bar colors
 * - barStyle: formatting tokens for option bar drawing
 * - titleFont: pointer into fonts[]
 * - colorFunction: optional function that returns c_pixel given normalized coords
 */
class subMenu {
public:
    explicit subMenu(const std::string& str)
        : name(str),
          selectedOption(0),
          selectedColor{255,255,0},
          defaultColor{128,128,128},
          barColor{255,50,255},
          barStyle(UI_Option_Bar{"-------------------", "\t-", "", "", "\t    ==> " , false}),
          titleFont(nullptr),
          colorFunction(nullptr)
    {
        /* default titleFont points to Mono12 if available */
        if (!fonts.empty()) titleFont = &fonts[AvailableFonts::Mono12];
    }

    /* Add a single option (by reference copy) */
    void addOption(const UI_Option & opt) {
        options.push_back(opt);
    }

    /* Add many options (copy) */
    void addOptions(const std::vector<UI_Option> & new_options) {
        for (const auto& opt : new_options)
            options.push_back(opt);
    }

    /* Setters for appearance */
    void setFontFromDefault(AvailableFonts::EnumFonts fontToUse) {
        if (fontToUse >= 0 && fontToUse < static_cast<AvailableFonts::EnumFonts>(fonts.size()))
            titleFont = &(fonts[fontToUse]);
        else if (!fonts.empty())
            titleFont = &fonts[AvailableFonts::Mono12];
    }

    void setSelectedColor(color c) { selectedColor = c; }
    void setDefaultColor(color c) { defaultColor = c; }
    void setBarColor(color c)     { barColor = c; }

    void setBarStyle(const std::string& top,
                     const std::string& beforeOption,
                     const std::string& afterOption,
                     const std::string& gap = "",
                     const std::string& selected = "\t",
                     bool hasGap = false)
    {
        barStyle = UI_Option_Bar{top, beforeOption, afterOption, gap, selected, hasGap};
    }

    void setBarStyle(const UI_Option_Bar & newBarStyle) {
        barStyle = newBarStyle;
    }

    void incrementOption() {
        if (options.empty()) return;
        ++selectedOption;
        if (selectedOption >= static_cast<int>(options.size())) selectedOption = 0;
    }

    void decrementOption() {
        if (options.empty()) return;
        --selectedOption;
        if (selectedOption < 0) selectedOption = static_cast<int>(options.size()) - 1;
    }

    void selectOption(int index) {
        if (index < 0 || index >= static_cast<int>(options.size())) return;
        selectedOption = index;
    }

    void CallSelectedOption() {
        if (!options.empty())
            options[selectedOption].Call();
    }

    /* Public members */
    std::string name;
    std::vector<UI_Option> options;
    int selectedOption;

    color selectedColor;
    color defaultColor;
    color barColor;

    UI_Option_Bar barStyle;

    const Font* titleFont;
    std::function<c_pixel(double, double)> colorFunction;
};

/* --------------------------------------------------------------------------
   cliMenu - main interactive menu system
   -------------------------------------------------------------------------- */
/*
 * cliMenu
 *
 * - Manages a character buffer, color buffer and prints to the console.
 * - Preserves original public API (init, addBorder, DrawMenu, startLoop, etc).
 */
class cliMenu {
public:
    cliMenu() : width(0), height(0), borderEnabled(false), currentMenu(0), exit(false) {
        init();
    }

    /* Print only changed cells (keeps original behavior) */
    void printChanges() {
        static std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> conv;

        for (int row = 0; row < height; ++row) {
            for (int col = 0; col < width; ++col) {
                if (!isChanged[row][col]) continue;

                cursor(col, row);

                char32_t c = buffer[row][col];
                color_buffer[row][col].setTextColor();
                std::cout << conv.to_bytes(c);

                isChanged[row][col] = false;
            }
        }
    }

    /* Print full buffer optimized into a single string (original frame builder) */
    void printBuffer() {
        static std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> conv;

        std::string frame;
        frame.reserve(static_cast<size_t>(width) * static_cast<size_t>(height) * 8);

        frame += ERASE_CONSOLE; /* clear screen */
        frame += START_SEQUENCE "H"; /* cursor home */

        for (int row = 0; row < height; ++row) {
            for (int col = 0; col < width; ++col) {
                const c_pixel & pix = color_buffer[row][col];
                char32_t c = buffer[row][col];

                /* Foreground */
                frame += ESC_COLOR_CODE;
                frame += FOREGROUND_SEQUENCE
                      + std::to_string(static_cast<int>(pix.foreground().r)) + SEQUENCE_ARG_SEPARATOR
                      + std::to_string(static_cast<int>(pix.foreground().g)) + SEQUENCE_ARG_SEPARATOR
                      + std::to_string(static_cast<int>(pix.foreground().b)) + CLOSE_SEQUENCE;

                /* Background */
                frame += ESC_COLOR_CODE;
                frame += BACKGROUND_SEQUENCE
                      + std::to_string(static_cast<int>(pix.background().r)) + SEQUENCE_ARG_SEPARATOR
                      + std::to_string(static_cast<int>(pix.background().g)) + SEQUENCE_ARG_SEPARATOR
                      + std::to_string(static_cast<int>(pix.background().b)) + CLOSE_SEQUENCE;

                if (pix.bold())     frame += SET_BOLD;
                if (pix.blinking()) frame += SET_BLINKING;

                frame += conv.to_bytes(c);

                isChanged[row][col] = false;
            }
            frame += '\n';
        }

        frame += RESET_ALL;
        std::cout << frame << std::flush;
    }

    /* Initialize console and buffers */
    void init() {
        std::cout << RESET_ALL << ERASE_CONSOLE;

        #ifdef _WIN32
            /* Switch Windows console to UTF-8 code page */
            system("chcp 65001 >nul");
        #endif

        /* Query console size */
        #ifdef _WIN32
            CONSOLE_SCREEN_BUFFER_INFO csbi;
            if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
                width  = csbi.srWindow.Right  - csbi.srWindow.Left + 1;
                height = csbi.srWindow.Bottom - csbi.srWindow.Top  + 1;
            } else {
                width = height = -1;
            }
        #else
            struct winsize w;
            if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0) {
                width  = w.ws_col;
                height = w.ws_row;
            } else {
                width = height = -1;
            }
        #endif

        if (width < 1 || height < 1) {
            std::cout << "Error getting console size";
            return;
        }

        /* Reserve one row to avoid terminal bottomline overlapping */
        --height;

        buffer.assign(height, std::vector<char32_t>(width, U' '));
        color_buffer.assign(height, std::vector<c_pixel>(width, c_pixel(color{255,255,255})));
        isChanged.assign(height, std::vector<bool>(width, false));
    }

    /* Add a box border around the buffer and apply gradient */
    void addBorder() {
        borderEnabled = true;

        buffer[0][0] = U'╔';
        buffer[0][width-1] = U'╗';
        buffer[height-1][0] = U'╚';
        buffer[height-1][width-1] = U'╝';

        for (int i = 1; i < width - 1; ++i) {
            buffer[0][i] = U'═';
            buffer[height-1][i] = U'═';
        }
        for (int i = 1; i < height - 1; ++i) {
            buffer[i][0] = U'║';
            buffer[i][width-1] = U'║';
        }
        addGradient();
    }

    /* Create a simple background gradient in color_buffer */
    void addGradient() {
        for (int row = 0; row < height; ++row) {
            for (int col = 0; col < width; ++col) {
                double perc_y = static_cast<double>(row) / static_cast<double>(height);
                double perc_x = static_cast<double>(col) / static_cast<double>(width);

                unsigned char r = static_cast<unsigned char>(perc_y * 255.0);
                unsigned char g = static_cast<unsigned char>(perc_x * (1.0 - perc_y) * 255.0);
                unsigned char b = 250;

                color new_color{r, g, b};
                color_buffer[row][col].setForeground(new_color);
            }
        }
    }

    /* Select submenu by name (first match) */
    void SelectSubMenu(const std::string& str) {
        for (size_t i = 0; i < submenus.size(); ++i) {
            if (submenus[i].name == str) {
                currentMenu = static_cast<int>(i);
                return;
            }
        }
    }

    /* Select submenu by index */
    void SelectSubMenu(int index) {
        if (index < 0 || index >= static_cast<int>(submenus.size())) return;
        currentMenu = index;
    }

    /* Draw the full menu (title + options) to the terminal */
    void DrawMenu() {
        std::cout << ERASE_CONSOLE << RESET_ALL;

        /* Reset the buffer to spaces */
        buffer.assign(height, std::vector<char32_t>(width, U' '));

        if (borderEnabled) addBorder();

        const subMenu & menu = submenus.at(static_cast<size_t>(currentMenu));

        /* Compute title metrics (how many columns in total, and title height) */
        const int nr_chars = static_cast<int>(menu.name.length());
        int total_length_in_Chars = 0;
        int title_height_in_Chars = 0;

        for (int i = 0; i < nr_chars; ++i) {
            const Character* pch = (*(menu.titleFont))[menu.name[i]];
            if (pch == nullptr) continue;

            int char_width = 0;
            int char_height = pch->height;
            for (int r = 0; r < char_height; ++r) {
                int line_length = static_cast<int>(pch->data[r].size());
                char_width = std::max(char_width, line_length);
            }

            total_length_in_Chars += char_width;
            title_height_in_Chars = std::max(title_height_in_Chars, char_height);
        }

        int absolute_top_left_x = static_cast<int>((buffer[0].size() / 2) - (total_length_in_Chars / 2));
        int absolute_top_right_x = static_cast<int>((width / 2) + (total_length_in_Chars / 2));

        int top_padding = 1;
        if (borderEnabled) ++top_padding;
        int absolute_bottom_y = title_height_in_Chars + top_padding;

        /* Draw title glyphs into the buffer */
        int start_x_position = absolute_top_left_x;
        for (int i = 0; i < nr_chars; ++i) {
            const Character* pch = (*(menu.titleFont))[menu.name[i]];
            if (pch == nullptr) continue;

            coords top_left_char_corner{ start_x_position, top_padding };

            int char_width = 0;
            for (size_t r = 0; r < pch->data.size(); ++r) {
                int chr_length = static_cast<int>(pch->data[r].length());
                char_width = std::max(char_width, chr_length);
            }

            start_x_position += char_width;
            DrawOneChar(top_left_char_corner, pch);
        }

        /* Optional per-title color function (fills title bounding box with colors) */
        if (menu.colorFunction) {
            for (int row = top_padding; row < title_height_in_Chars; ++row) {
                for (int col = absolute_top_left_x; col < absolute_top_right_x; ++col) {
                    double x = static_cast<double>(col) / static_cast<double>(absolute_top_right_x - absolute_top_left_x);
                    double y = static_cast<double>(row) / static_cast<double>(title_height_in_Chars - top_padding);

                    coords pos{ col, row };
                    rawBufferDrawColor(pos, menu.colorFunction(x, y));
                }
            }
        }

        /* Print buffer to console */
        printBuffer();

        /* Draw the options listing on top (cursor-based printing) */
        int option_y_level = absolute_bottom_y;
        int option_x_level = top_padding;
        c_pixel bar_color(menu.barColor);
        cursor(option_x_level, option_y_level++);
        bar_color.setTextColor();
        std::cout << menu.barStyle.top;

        for (size_t i = 0; i < menu.options.size(); ++i) {
            if (menu.barStyle.gap) {
                cursor(option_x_level, option_y_level++);
                bar_color.setTextColor();
                std::cout << menu.barStyle.between_gap;
            }

            cursor(option_x_level, option_y_level++);
            bar_color.setTextColor();

            if (static_cast<int>(i) == menu.selectedOption) {
                std::cout << menu.barStyle.selected;
            } else {
                std::cout << menu.barStyle.before_option;
            }

            c_pixel option_color = (static_cast<int>(i) == menu.selectedOption)
                                     ? c_pixel(menu.selectedColor)
                                     : c_pixel(menu.defaultColor);

            if (menu.options[i].overwriteColor_huh)
                option_color = menu.options[i].overwiteColor;

            option_color.setTextColor();
            std::cout << menu.options[i].text;
            bar_color.setTextColor();
            std::cout << menu.barStyle.after_option;
        }
    }

    /* Remove title glyphs by writing space into the same region */
    void removeTitleFromBuffer() {
        const subMenu & menu = submenus.at(static_cast<size_t>(currentMenu));
        const int nr_chars = static_cast<int>(menu.name.length());
        int total_length_in_Chars = 0;
        int title_height_in_Chars = 0;

        for (int i = 0; i < nr_chars; ++i) {
            const Character* pch = (*(menu.titleFont))[menu.name[i]];
            if (pch == nullptr) continue;

            int char_width = 0;
            int char_height = pch->height;
            for (int r = 0; r < char_height; ++r)
                char_width = std::max(char_width, static_cast<int>(pch->data[r].size()));

            total_length_in_Chars += char_width;
            title_height_in_Chars = std::max(title_height_in_Chars, char_height);
        }

        int absolute_top_left_x = static_cast<int>((buffer[0].size() / 2) - (total_length_in_Chars / 2));
        int absolute_top_right_x = static_cast<int>((width / 2) + (total_length_in_Chars / 2));

        int top_padding = 1;
        if (borderEnabled) ++top_padding;
        int absolute_bottom_y = title_height_in_Chars + top_padding;

        int start_x_position = absolute_top_left_x;
        for (int i = 0; i < nr_chars; ++i) {
            const Character* pch = (*(menu.titleFont))[menu.name[i]];
            if (pch == nullptr) continue;

            coords top_left_char_corner{ start_x_position, top_padding };

            int char_width = 0;
            for (size_t r = 0; r < pch->data.size(); ++r) {
                char_width = std::max(char_width, static_cast<int>(pch->data[r].length()));
            }

            start_x_position += char_width;
            DrawOnMask(top_left_char_corner, pch, U' ');
        }
    }

    /* Draw a glyph into the buffer but replacing glyph pixels with a given mask char */
    void DrawOnMask(coords start, const Character* pToPrint, char32_t char_to_print) {
        const Character& toPrint = *pToPrint;
        for (int char_y = 0; char_y < static_cast<int>(toPrint.data.size()); ++char_y) {
            for (int char_x = 0; char_x < static_cast<int>(toPrint.data[char_y].length()); ++char_x) {
                coords pos{ start.x + char_x, start.y + char_y };
                rawBufferDrawChar(pos, char_to_print);
            }
        }
    }

    /* Draw a centered string using a Font, then apply colorFunction to the title bounding box */
    void DrawStringCenterCords(coords middle, const std::string & str, const Font* font_to_use, std::function<c_pixel(double, double)> colorFunction) {
        const int nr_chars = static_cast<int>(str.length());
        int total_length_in_Chars = 0;
        int title_height_in_Chars = 0;

        for (int i = 0; i < nr_chars; ++i) {
            const Character* pch = (*font_to_use)[str[i]];
            if (pch == nullptr) continue;

            int char_width = 0;
            int char_height = pch->height;
            for (int r = 0; r < char_height; ++r)
                char_width = std::max(char_width, static_cast<int>(pch->data[r].size()));

            total_length_in_Chars += char_width;
            title_height_in_Chars = std::max(title_height_in_Chars, char_height);
        }

        int absolute_left_x = middle.x - (total_length_in_Chars / 2);
        int absolute_right_x = middle.x + (total_length_in_Chars / 2);
        int absolute_top_y = middle.y - (title_height_in_Chars / 2);

        int start_x_position = absolute_left_x;
        for (int i = 0; i < nr_chars; ++i) {
            const Character* pch = (*font_to_use)[str[i]];
            if (pch == nullptr) continue;

            coords top_left_char_corner{ start_x_position, absolute_top_y };
            int char_width = 0;
            for (size_t r = 0; r < pch->data.size(); ++r)
                char_width = std::max(char_width, static_cast<int>(pch->data[r].length()));

            start_x_position += char_width;
            DrawOneChar(top_left_char_corner, pch);
        }

        int absolute_bottom_y = absolute_top_y + title_height_in_Chars;
        for (int row = absolute_top_y; row <= absolute_bottom_y; ++row) {
            for (int col = absolute_left_x; col <= absolute_right_x; ++col) {
                double x = static_cast<double>(col - absolute_right_x) / static_cast<double>(absolute_right_x - absolute_left_x);
                double y = static_cast<double>(row - absolute_top_y) / static_cast<double>(absolute_bottom_y - absolute_top_y);
                coords pos{ col, row };
                rawBufferDrawColor(pos, colorFunction(x, y));
            }
        }
    }

    /* Draw one glyph into the buffer (no color changes) */
    void DrawOneChar(coords start, const Character* pToPrint) {
        const Character& toPrint = *pToPrint;
        for (int char_y = 0; char_y < static_cast<int>(toPrint.data.size()); ++char_y) {
            for (int char_x = 0; char_x < static_cast<int>(toPrint.data[char_y].length()); ++char_x) {
                char32_t char_to_print = toPrint.data[char_y][char_x];
                coords pos{ start.x + char_x, start.y + char_y };
                rawBufferDrawChar(pos, char_to_print);
            }
        }
    }

    /* Draw one glyph and set its foreground color for every glyph cell (uses c_pixel(rgb)) */
    void DrawOneChar(coords start, const Character* pToPrint, color rgb) {
        const Character& toPrint = *pToPrint;
        for (int char_y = 0; char_y < static_cast<int>(toPrint.data.size()); ++char_y) {
            for (int char_x = 0; char_x < static_cast<int>(toPrint.data[char_y].length()); ++char_x) {
                char32_t char_to_print = toPrint.data[char_y][char_x];
                coords pos{ start.x + char_x, start.y + char_y };
                rawBufferDrawChar(pos, char_to_print);
                rawBufferDrawColor(pos, c_pixel(rgb));
            }
        }
    }

    /* Raw buffer writers (boundary-checked) */
    void rawBufferDrawChar(coords pos, char32_t character) {
        if (pos.x < 0 || pos.x >= width || pos.y < 0 || pos.y >= height) return;
        buffer[pos.y][pos.x] = character;
        isChanged[pos.y][pos.x] = true;
    }

    void rawBufferDrawColor(coords pos, c_pixel new_color) {
        if (pos.x < 0 || pos.x >= width || pos.y < 0 || pos.y >= height) return;
        color_buffer[pos.y][pos.x] = new_color;
        isChanged[pos.y][pos.x] = true;
    }

    void rawBufferDraw(coords pos, char32_t character, c_pixel color) {
        if (pos.x < 0 || pos.x >= width || pos.y < 0 || pos.y >= height) return;
        buffer[pos.y][pos.x] = character;
        color_buffer[pos.y][pos.x] = color;
        isChanged[pos.y][pos.x] = true;
    }

    /* Placeholder for image printing */
    void printImage() {
        /* Intentionally left blank: implement as needed */
    }

    /* Event loop (blocking) - simple getch handling (up/down/enter) */
    void startLoop() {
        while (!exit) {
            DrawMenu();
            int c = 0;
            switch ((c = getch())) {
                case KEY_UP:
                    submenus[currentMenu].decrementOption();
                    break;
                case KEY_DOWN:
                    submenus[currentMenu].incrementOption();
                    break;
                case 13:
                    submenus[currentMenu].CallSelectedOption();
                    break;
                default:
                    std::cout << std::endl << "null" << std::endl;
                    break;
            }
        }
    }

    /* Public state */
    int width;
    int height;
    bool borderEnabled;

    int currentMenu;
    std::vector<subMenu> submenus;

    std::vector<std::vector<char32_t>> buffer;
    std::vector<std::vector<c_pixel>> color_buffer;
    std::vector<std::vector<bool>> isChanged;

    bool exit;
};

