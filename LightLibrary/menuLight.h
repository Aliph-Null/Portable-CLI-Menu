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
    #define cursor(x, y) SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), COORD{x, y})
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

#define color_sequence_max_length 22

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

    bool operator==(const color& b) const {
        return this->r == b.r && this->g == b.g && this->b == b.b;
    }

    bool operator!=(const color& b) const{
        return !(*this == b);
    }
};

// Overload << operator for color
std::ostream& operator<<(std::ostream& os, const color& c) {
    os << ESC_COLOR_CODE << FOREGROUND_SEQUENCE
       << static_cast<int>(c.r) << SEQUENCE_ARG_SEPARATOR
       << static_cast<int>(c.g) << SEQUENCE_ARG_SEPARATOR
       << static_cast<int>(c.b) << CLOSE_SEQUENCE;
    return os;
}

struct coords {
    int x;
    int y;
};

namespace AvailableAlignments{
    enum EnumAlignment{
        LEFT,
        CENTER,
        RIGHT
    };
}

namespace beautyPrint{
    inline void print(std::string str){std::cerr<<str;}
    inline void print(std::string str, color c){std::cerr<<c<<str;}
    void print(std::string str, std::function<color(double)> colorFunction){
        std::string str_toPrint = "";
        int len = str.length();
        for(int i = 0; i < len; i++){
            double x = (double)i / (double)len;
            color c = colorFunction(x);
            str_toPrint += ESC_COLOR_CODE;
            str_toPrint += FOREGROUND_SEQUENCE;
            str_toPrint += std::to_string(static_cast<int>(c.r));
            str_toPrint += SEQUENCE_ARG_SEPARATOR;
            str_toPrint += std::to_string(static_cast<int>(c.g));
            str_toPrint += SEQUENCE_ARG_SEPARATOR;
            str_toPrint += std::to_string(static_cast<int>(c.b));
            str_toPrint += CLOSE_SEQUENCE;
            str_toPrint += str[i];
        }
        str_toPrint += RESET_ALL;
        std::cerr << str_toPrint;
    }

    inline void print(coords pos, std::string str){cursor(pos.x, pos.y); print(str);}
    inline void print(coords pos, std::string str, color c){cursor(pos.x, pos.y); print(str, c);}
    void print(coords pos, std::string str, std::function<color(double)> colorFunction){cursor(pos.x, pos.y); print(str, colorFunction);}

    //where x is the width and y is the y
    void print(coords pos, std::string str, AvailableAlignments::EnumAlignment align){
        int start_x = 0;
        switch(align){
            case AvailableAlignments::LEFT:
                break;
            case AvailableAlignments::CENTER:
                start_x = (pos.x/2) - (str.length()/2);
                if(start_x < 0) start_x = 0;
                break;
            case AvailableAlignments::RIGHT:
                start_x = pos.x - str.length();
                if(start_x < 0) start_x = 0;
                break;
            default:
                break;
        }
        print({start_x, pos.y}, str);
    }

    //where x is the width and y is the y
    void print(coords pos, std::string str, AvailableAlignments::EnumAlignment align, color c){
        int start_x = 0;
        switch(align){
            case AvailableAlignments::LEFT:
                break;
            case AvailableAlignments::CENTER:
                start_x = (pos.x/2) - (str.length()/2);
                if(start_x < 0) start_x = 0;
                break;
            case AvailableAlignments::RIGHT:
                start_x = pos.x - str.length();
                if(start_x < 0) start_x = 0;
                break;
            default:
                break;
        }
        print({start_x, pos.y}, str, c);
    }

    //where x is the width and y is the y
    void print(coords pos, std::string str, AvailableAlignments::EnumAlignment align, std::function<color(double)> colorFunction){
        int start_x = 0;
        switch(align){
            case AvailableAlignments::LEFT:
                break;
            case AvailableAlignments::CENTER:
                start_x = (pos.x/2) - (str.length()/2);
                if(start_x < 0) start_x = 0;
                break;
            case AvailableAlignments::RIGHT:
                start_x = pos.x - str.length();
                if(start_x < 0) start_x = 0;
                break;
            default:
                break;
        }
        print({start_x, pos.y}, str, colorFunction);
    }
}


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

class UI_Option {
public:
    UI_Option(const std::string& str) : text(str) {}
    UI_Option(const std::string& str, void(*f)()): text(str) { Subscribe(f);}
    UI_Option(const std::string& str, color overWrite, void(*f)()): text(str), overwriteColor(overWrite){ Subscribe(f);}

    void Subscribe(void(*func)()) {
        callBackList.emplace_back(func);
    }

    void Unsubscribe(void(*func)()) {
        auto it = std::find(callBackList.begin(), callBackList.end(), func);
        if (it != callBackList.end())
            callBackList.erase(it);
    }

    void Call() const {
        for (auto f : callBackList)
            if (f) f();
    }

    std::string text;
    std::vector<void(*)()> callBackList;
    color overwriteColor = {0,0,0};
};

struct UI_Option_Bar {
    std::string before_option;
    std::string after_option;
    std::string selected_before;
    std::string selected_after;
    color bar_color = {0, 0, 0};
};

const std::vector<UI_Option_Bar> bars = {
    UI_Option_Bar{"", "", "< ", " >", {255, 235, 50}},
    UI_Option_Bar{"", "", "", "\t->", {0, 0, 0}}
};


color defaultGradient(double x){
    return {x*255, sin(x*3.1415) * 200, 255};
}

class subMenu {
private:
    std::string name;
    std::vector<UI_Option> options;
    int selectedOption = 0;

    color selectedColor{255, 255, 0};
    color defaultColor{250, 250, 250};
    color titleColor{0, 0, 0};

    AvailableAlignments::EnumAlignment titleAlignment = AvailableAlignments::CENTER;
    AvailableAlignments::EnumAlignment optionsAlignment = AvailableAlignments::CENTER;
    UI_Option_Bar bar = bars[0];

    std::function<color(double)> colorFunction = defaultGradient;

public:
    /* =========================
       Constructors
       ========================= */

    subMenu(const std::string& n)
        : name(n) {}

    subMenu(const std::string& n, const std::vector<UI_Option>& opts)
        : name(n), options(opts), selectedOption(0) {}

    subMenu(const std::string& n, const std::vector<UI_Option>& opts,
            color def, color sel, color title = {0, 0, 0},
            AvailableAlignments::EnumAlignment align = AvailableAlignments::CENTER,
            const UI_Option_Bar& b = bars[0])
        : name(n),
          options(opts),
          selectedOption(0),
          selectedColor(sel),
          defaultColor(def),
          titleColor(title),
          titleAlignment(align),
          bar(b) {}

    /* =========================
       Setters
       ========================= */

    void setName(const std::string& n) { name = n; }

    void setOptions(const std::vector<UI_Option>& opts) { options = opts; }

    void addOption(const UI_Option& opt) { options.push_back(opt); }

    void addOptions(const std::vector<UI_Option>& new_options) {
        for (const auto& opt : new_options)
            options.push_back(opt);
    }

    void setSelectedColor(color c) { selectedColor = c; }

    void setDefaultColor(color c) { defaultColor = c; }

    void setTitleColor(color c) { titleColor = c; }

    void setTitleColor(std::function<color(double)> new_color_function) { colorFunction = new_color_function; }

    void setTitleAlignment(AvailableAlignments::EnumAlignment a) { titleAlignment = a; }
    void setOptionsAlignment(AvailableAlignments::EnumAlignment a) { optionsAlignment = a; }

    void setBar(const UI_Option_Bar& b) { bar = b; }

    void selectOption(int index) {
        if (index < 0 || index >= static_cast<int>(options.size())) return;
        selectedOption = index;
    }

    /* =========================
       Getters
       ========================= */

    const std::string& getName() const { return name; }

    const std::vector<UI_Option>& getOptions() const { return options; }

    int getSelectedIndex() const { return selectedOption; }

    const UI_Option* getSelectedOption() const {
        if (options.empty()) return nullptr;
        return &options[selectedOption];
    }

    color getSelectedColor() const { return selectedColor; }

    color getDefaultColor() const { return defaultColor; }

    color getTitleColor() const { return titleColor; }

    color getTitleColor(double x) {return colorFunction ? colorFunction(x) : titleColor;}

    AvailableAlignments::EnumAlignment getTitleAlignment() const { return titleAlignment; }

    AvailableAlignments::EnumAlignment getOptionsAlignment() const { return optionsAlignment; }

    const UI_Option_Bar& getBar() const { return bar; }

    /* =========================
       Navigation
       ========================= */

    void incrementOption() {
        if (options.empty()) return;
        ++selectedOption;
        if (selectedOption >= static_cast<int>(options.size()))
            selectedOption = 0;
    }

    void decrementOption() {
        if (options.empty()) return;
        --selectedOption;
        if (selectedOption < 0)
            selectedOption = static_cast<int>(options.size()) - 1;
    }

    /* =========================
       Action
       ========================= */

    void CallSelectedOption() {
        if (!options.empty())
            options[selectedOption].Call();
    }
};

class cli_menu {
private:
    std::vector<subMenu> submenus;
    int selectedSubMenu = 0;

    int width = -1;
    int height = -1;

    bool exit_var = false;

public:
    /* =========================
       Constructors
       ========================= */
    void init(bool disableSysCallSync = false) {
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

        if(disableSysCallSync){
            //For faster printing
            std::ios::sync_with_stdio(false);
            std::cin.tie(nullptr);
        }
    }

    cli_menu() {init();};

    cli_menu(const std::vector<subMenu>& subs)
        : submenus(subs), selectedSubMenu(0) {init();}

    /* =========================
       Setters
       ========================= */

    void setSubMenus(const std::vector<subMenu>& subs) { submenus = subs; }

    void addSubMenu(const subMenu& sm) { submenus.push_back(sm); }

    void addSubMenus(const std::vector<subMenu>& new_subs) {
        for (const auto& sm : new_subs)
            submenus.push_back(sm);
    }

    void removeSubMenu(int index) {
        if (index < 0 || index >= static_cast<int>(submenus.size())) return;
        submenus.erase(submenus.begin() + index);
        if (selectedSubMenu >= static_cast<int>(submenus.size()))
            selectedSubMenu = std::max(0, static_cast<int>(submenus.size()) - 1);
    }

    void clearSubMenus() {
        submenus.clear();
        selectedSubMenu = 0;
    }

    void selectSubMenu(int index) {
        if (index < 0 || index >= static_cast<int>(submenus.size())) return;
        selectedSubMenu = index;
    }

    void selectSubMenu(std::string index) {
        for(int i = submenus.size()-1; i >= 0; i--){
            if(submenus[i].getName() == index){
                selectedSubMenu = i;
            }
        }
    }

    /* =========================
       Getters
       ========================= */

    const std::vector<subMenu>& getSubMenus() const { return submenus; }

    std::vector<subMenu>& getSubMenus() { return submenus; } // non-const version

    int getSelectedIndex() const { return selectedSubMenu; }

    const subMenu* getSelectedSubMenu() const {
        if (submenus.empty()) return nullptr;
        return &submenus[selectedSubMenu];
    }

    subMenu* getSelectedSubMenu() {
        if (submenus.empty()) return nullptr;
        return &submenus[selectedSubMenu];
    }

    int getSubMenuCount() const { return static_cast<int>(submenus.size()); }

    int getHeight(){return height;}
    int getWidth(){return width;}

    /* =========================
       Navigation
       ========================= */

    void incrementSubMenu() {
        if (submenus.empty()) return;
        ++selectedSubMenu;
        if (selectedSubMenu >= static_cast<int>(submenus.size()))
            selectedSubMenu = 0;
    }

    void decrementSubMenu() {
        if (submenus.empty()) return;
        --selectedSubMenu;
        if (selectedSubMenu < 0)
            selectedSubMenu = static_cast<int>(submenus.size()) - 1;
    }

    /* =========================
       Actions
       ========================= */

    void CallSelectedSubMenuOption() {
        if (submenus.empty()) return;
        submenus[selectedSubMenu].CallSelectedOption();
    }

    /* Optional convenience: retrieve submenu by name */
    subMenu* findSubMenuByName(const std::string& name) {
        for (auto& sm : submenus)
            if (sm.getName() == name)
                return &sm;
        return nullptr;
    }

    const subMenu* findSubMenuByName(const std::string& name) const {
        for (const auto& sm : submenus)
            if (sm.getName() == name)
                return &sm;
        return nullptr;
    }


    /* =========================
       Implementation
       ========================= */
    void exit(){
        exit_var = true;
    }

    void clearConsole(){
        //Use cerr to bypass buffering
        std::cerr << RESET_ALL << RESET_BLINKING << RESET_BOLD << ERASE_CONSOLE;
    }

    void startLoop(){
        exit_var = submenus.empty();
        while (!exit_var) {
            DrawMenu();
            int c = 0;
            switch ((c = getch())) {
                case KEY_UP:
                    submenus[selectedSubMenu].decrementOption();
                    break;
                case KEY_DOWN:
                    submenus[selectedSubMenu].incrementOption();
                    break;
                case 13:
                    submenus[selectedSubMenu].CallSelectedOption();
                    break;
                default:
                    std::cout << std::endl << "null" << std::endl;
                    break;
            }
        }
    }




    void DrawMenu(){
        clearConsole();

        subMenu &_menu = submenus[selectedSubMenu];
        int top_offset = 2;

        //print title ~ colored
        std::string char_title = _menu.getName();
        int title_length = char_title.length();
        int title_abs_length = (color_sequence_max_length + 1) * title_length;
        std::string title; title.reserve(title_abs_length);
        for(int i = 0; i < title_length; i++){
            color char_color = _menu.getTitleColor();
            if(char_color == color{0, 0, 0}){
                double x = (double)i / double(title_length);
                char_color = _menu.getTitleColor(x);
            }
            title += ESC_COLOR_CODE;
            title += FOREGROUND_SEQUENCE;
            title += std::to_string(static_cast<int>(char_color.r));
            title += SEQUENCE_ARG_SEPARATOR;
            title += std::to_string(static_cast<int>(char_color.g));
            title += SEQUENCE_ARG_SEPARATOR;
            title += std::to_string(static_cast<int>(char_color.b));
            title += CLOSE_SEQUENCE;
            title += char_title[i];
        }
        title += RESET_ALL;

        int start_x = 0;
        switch(_menu.getTitleAlignment()){
        case AvailableAlignments::LEFT:
            break;
        case AvailableAlignments::CENTER:
            start_x = (width/2) - (title_length/2);
            if(start_x < 0) start_x = 0;
            break;
        case AvailableAlignments::RIGHT:
            start_x = width - title_length;
            if(start_x < 0) start_x = 0;
            break;
        default:
            break;
        }
        cursor(start_x, top_offset);
        std::cerr << title;
        top_offset++;

        //print options
        int nr_options = _menu.getOptions().size();
        std::vector<UI_Option> options = _menu.getOptions();
        for(int i = 0; i < nr_options; i++){
            int option_length = 0;
            UI_Option &opt = options[i];
            std::string str_toPrint = "";
            UI_Option_Bar bar = _menu.getBar();
            //Bar color
            color c = _menu.getSelectedColor();
            if(bar.bar_color != color{0, 0, 0}){
                c = bar.bar_color;
            }
            //bar left
            std::string bar_left = bar.before_option;
            if(i == _menu.getSelectedIndex()){
                bar_left = bar.selected_before;
            }

            str_toPrint += ESC_COLOR_CODE;
            str_toPrint += FOREGROUND_SEQUENCE;
            str_toPrint += std::to_string(static_cast<int>(c.r));
            str_toPrint += SEQUENCE_ARG_SEPARATOR;
            str_toPrint += std::to_string(static_cast<int>(c.g));
            str_toPrint += SEQUENCE_ARG_SEPARATOR;
            str_toPrint += std::to_string(static_cast<int>(c.b));
            str_toPrint += CLOSE_SEQUENCE;
            str_toPrint += bar_left;
            option_length += bar_left.length();
            //text color
            c = _menu.getDefaultColor();
            if(i == _menu.getSelectedIndex()){
                c = _menu.getSelectedColor();
            }
            if(opt.overwriteColor != color{0, 0, 0}){
                c = opt.overwriteColor;
            }

            //text
            str_toPrint += ESC_COLOR_CODE;
            str_toPrint += FOREGROUND_SEQUENCE;
            str_toPrint += std::to_string(static_cast<int>(c.r));
            str_toPrint += SEQUENCE_ARG_SEPARATOR;
            str_toPrint += std::to_string(static_cast<int>(c.g));
            str_toPrint += SEQUENCE_ARG_SEPARATOR;
            str_toPrint += std::to_string(static_cast<int>(c.b));
            str_toPrint += CLOSE_SEQUENCE;
            str_toPrint += opt.text;
            option_length += opt.text.length();
            //bar color
            c = _menu.getSelectedColor();
            if(bar.bar_color != color{0, 0, 0}){
                c = bar.bar_color;
            }
            //bar right
            std::string bar_right = bar.after_option;
            if(i == _menu.getSelectedIndex()){
                bar_right = bar.selected_after;
            }

            str_toPrint += ESC_COLOR_CODE;
            str_toPrint += FOREGROUND_SEQUENCE;
            str_toPrint += std::to_string(static_cast<int>(c.r));
            str_toPrint += SEQUENCE_ARG_SEPARATOR;
            str_toPrint += std::to_string(static_cast<int>(c.g));
            str_toPrint += SEQUENCE_ARG_SEPARATOR;
            str_toPrint += std::to_string(static_cast<int>(c.b));
            str_toPrint += CLOSE_SEQUENCE;
            str_toPrint += bar_right;

            option_length += bar_right.length();

            start_x = 0;
            switch(_menu.getOptionsAlignment()){
            case AvailableAlignments::LEFT:
                break;
            case AvailableAlignments::CENTER:
                start_x = (width/2) - (option_length/2);
                if(start_x < 0) start_x = 0;
                break;
            case AvailableAlignments::RIGHT:
                start_x = width - option_length;
                if(start_x < 0) start_x = 0;
                break;
            default:
                break;
            }
            cursor(start_x, top_offset);
            std::cerr << str_toPrint;
            top_offset++;
        }
        cursor(0, height-1);
    }

};


// FNV-1a 32-bit
uint32_t fnv1a32(const std::string &s) {
    const uint32_t FNV_PRIME = 0x01000193u;
    uint32_t hash = 0x811C9DC5u; // FNV offset basis
    for (unsigned char c : s) {
        hash ^= static_cast<uint32_t>(c);
        hash *= FNV_PRIME;
    }
    return hash;
}

// Integer hue 0..359
unsigned int hue_from_string_int(const std::string &s) {
    uint32_t h = fnv1a32(s);
    return h % 360u; // 0..359
}

// Floating-point hue 0..360 (continuous)
double hue_from_string_double(const std::string &s) {
    uint32_t h = fnv1a32(s);
    // map [0, 2^32-1] -> [0.0, 360.0)
    return (static_cast<double>(h) / 4294967296.0) * 360.0;
}

void printMessage(string sender, string msg){
    cout << "<" << SET_BOLD << HSLtoRGB(hue_from_string_double(sender), 0.9, 0.69) << sender << RESET_ALL << "> " << msg << "\n";
}
