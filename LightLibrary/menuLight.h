#pragma once

#include <iostream>
#include <string>
#include <cmath>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <locale>
#include <codecvt>
#include <functional>
#include <conio.h>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <sys/ioctl.h>
    #include <unistd.h>
#endif

#ifdef _WIN32
    #define cursor(x, y) SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), (COORD){x, y})
#else
    #define cursor(x, y) std::cout << "\033[" << (y+1) << ";" << (x+1) << "H"
#endif

#define START_SEQUENCE "\033["
#define ESC_COLOR_CODE "\033["
#define FOREGROUND_SEQUENCE "38;2;"
#define BACKGROUND_SEQUENCE "48;2;"
#define SEQUENCE_ARG_SEPARATOR ";"
#define CLOSE_SEQUENCE "m"

#define Color_sequence_max_length 22

#define SET_BOLD "\033[1m"
#define RESET_BOLD "\033[22m"

#define SET_BLINKING "\033[5m"
#define RESET_BLINKING "\033[25m"

#define RESET_ALL "\033[0m"

#define ERASE_CONSOLE "\033c"

#define KEY_UP 72
#define KEY_DOWN 80

class Color {
private:
    unsigned char r, g, b;

public:
    Color(unsigned char red = 0, unsigned char green = 0, unsigned char blue = 0)
        : r(red), g(green), b(blue) {}

    unsigned char R() const { return r; }
    unsigned char G() const { return g; }
    unsigned char B() const { return b; }

    void setR(unsigned char red) { r = red; }
    void setG(unsigned char green) { g = green; }
    void setB(unsigned char blue) { b = blue; }

    bool operator==(const Color& other) const {
        return r == other.r && g == other.g && b == other.b;
    }

    bool operator!=(const Color& other) const {
        return !(*this == other);
    }

    virtual std::ostream& print(std::ostream& os) const {
        os << ESC_COLOR_CODE << FOREGROUND_SEQUENCE
           << static_cast<int>(r) << SEQUENCE_ARG_SEPARATOR
           << static_cast<int>(g) << SEQUENCE_ARG_SEPARATOR
           << static_cast<int>(b) << CLOSE_SEQUENCE;
        return os;
    }
};

class ColorBackground : public Color {
private:
    unsigned char br, bg, bb;

public:
    ColorBackground(unsigned char red = 0, unsigned char green = 0, unsigned char blue = 0,
                    unsigned char bgRed = 0, unsigned char bgGreen = 0, unsigned char bgBlue = 0)
        : Color(red, green, blue), br(bgRed), bg(bgGreen), bb(bgBlue) {}

    unsigned char Br() const { return br; }
    unsigned char Bg() const { return bg; }
    unsigned char Bb() const { return bb; }

    void setBr(unsigned char val) { br = val; }
    void setBg(unsigned char val) { bg = val; }
    void setBb(unsigned char val) { bb = val; }

    std::ostream& print(std::ostream& os) const override {
        os << ESC_COLOR_CODE << FOREGROUND_SEQUENCE
           << static_cast<int>(this->R()) << SEQUENCE_ARG_SEPARATOR
           << static_cast<int>(this->G()) << SEQUENCE_ARG_SEPARATOR
           << static_cast<int>(this->B()) << CLOSE_SEQUENCE
           << ESC_COLOR_CODE << BACKGROUND_SEQUENCE
           << static_cast<int>(this->Br()) << SEQUENCE_ARG_SEPARATOR
           << static_cast<int>(this->Bg()) << SEQUENCE_ARG_SEPARATOR
           << static_cast<int>(this->Bb()) << CLOSE_SEQUENCE;
        return os;
    }
};

inline std::ostream& operator<<(std::ostream& os, const Color& c) {
    return c.print(os);
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
    inline void print(std::string str, Color c){std::cerr<<c<<str;}
    void print(std::string str, std::function<Color(double)> ColorFunction){
        std::string str_toPrint = "";
        int len = str.length();
        for(int i = 0; i < len; i++){
            double x = (double)i / (double)len;
            Color c = ColorFunction(x);
            str_toPrint += ESC_COLOR_CODE;
            str_toPrint += FOREGROUND_SEQUENCE;
            str_toPrint += std::to_string(static_cast<int>(c.R()));
            str_toPrint += SEQUENCE_ARG_SEPARATOR;
            str_toPrint += std::to_string(static_cast<int>(c.G()));
            str_toPrint += SEQUENCE_ARG_SEPARATOR;
            str_toPrint += std::to_string(static_cast<int>(c.B()));
            str_toPrint += CLOSE_SEQUENCE;
            str_toPrint += str[i];
        }
        str_toPrint += RESET_ALL;
        std::cerr << str_toPrint;
    }

    inline void print(coords pos, std::string str){cursor(pos.x, pos.y); print(str);}
    inline void print(coords pos, std::string str, Color c){cursor(pos.x, pos.y); print(str, c);}
    void print(coords pos, std::string str, std::function<Color(double)> ColorFunction){cursor(pos.x, pos.y); print(str, ColorFunction);}

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
    void print(coords pos, std::string str, AvailableAlignments::EnumAlignment align, Color c){
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
    void print(coords pos, std::string str, AvailableAlignments::EnumAlignment align, std::function<Color(double)> ColorFunction){
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
        print({start_x, pos.y}, str, ColorFunction);
    }
}


/* --------------------------------------------------------------------------
   HSL to RGB conversion
   - kept as a free function to minimize API changes
   -------------------------------------------------------------------------- */
/*
 * HSLtoRGB(h, s, l) -> Color
 *
 * h: hue in degrees (any value; normalized internally to [0,360))
 * s: saturation [0..1]
 * l: lightness  [0..1]
 *
 * Returns an 8-bit per channel Color.
 */
inline Color HSLtoRGB(double h, double s, double l) {
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

    Color result;
    result.setR(static_cast<unsigned char>(std::round((r1 + m) * 255.0)));
    result.setG(static_cast<unsigned char>(std::round((g1 + m) * 255.0)));
    result.setB(static_cast<unsigned char>(std::round((b1 + m) * 255.0)));
    return result;
}

class UI_Option {
public:
    UI_Option(const std::string& str) : text(str) {}
    UI_Option(const std::string& str, void(*f)()): text(str) { Subscribe(f);}
    UI_Option(const std::string& str, Color overWrite, void(*f)()): text(str), overwriteColor(overWrite){ Subscribe(f);}

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
    Color overwriteColor = {0,0,0};
};

struct UI_Option_Bar {
    std::string before_option;
    std::string after_option;
    std::string selected_before;
    std::string selected_after;
    Color bar_Color = {0, 0, 0};
};

const std::vector<UI_Option_Bar> bars = {
    UI_Option_Bar{"", "", "< ", " >", {255, 235, 50}},
    UI_Option_Bar{"", "", "", "\t->", {0, 0, 0}}
};


Color defaultGradient(double x){
    return {x*255, sin(x*3.1415) * 200, 255};
}

class subMenu {
private:
    std::string name;
    std::vector<UI_Option> options;
    int selectedOption = 0;

    Color selectedColor{255, 255, 0};
    Color defaultColor{250, 250, 250};
    Color titleColor{0, 0, 0};

    AvailableAlignments::EnumAlignment titleAlignment = AvailableAlignments::CENTER;
    AvailableAlignments::EnumAlignment optionsAlignment = AvailableAlignments::CENTER;
    UI_Option_Bar bar = bars[0];

    std::function<Color(double)> ColorFunction = defaultGradient;

public:
    /* =========================
       Constructors
       ========================= */

    subMenu(const std::string& n)
        : name(n) {}

    subMenu(const std::string& n, const std::vector<UI_Option>& opts)
        : name(n), options(opts), selectedOption(0) {}

    subMenu(const std::string& n, const std::vector<UI_Option>& opts,
            Color def, Color sel, Color title = {0, 0, 0},
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

    void setSelectedColor(Color c) { selectedColor = c; }

    void setDefaultColor(Color c) { defaultColor = c; }

    void setTitleColor(Color c) { titleColor = c; }

    void setTitleColor(std::function<Color(double)> new_Color_function) { ColorFunction = new_Color_function; }

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

    Color getSelectedColor() const { return selectedColor; }

    Color getDefaultColor() const { return defaultColor; }

    Color getTitleColor() const { return titleColor; }

    Color getTitleColor(double x) {return ColorFunction ? ColorFunction(x) : titleColor;}

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

        //print title ~ Colored
        std::string char_title = _menu.getName();
        int title_length = char_title.length();
        int title_abs_length = (Color_sequence_max_length + 1) * title_length;
        std::string title; title.reserve(title_abs_length);
        for(int i = 0; i < title_length; i++){
            Color char_Color = _menu.getTitleColor();
            if(char_Color == Color{0, 0, 0}){
                double x = (double)i / double(title_length);
                char_Color = _menu.getTitleColor(x);
            }
            title += ESC_COLOR_CODE;
            title += FOREGROUND_SEQUENCE;
            title += std::to_string(static_cast<int>(char_Color.R()));
            title += SEQUENCE_ARG_SEPARATOR;
            title += std::to_string(static_cast<int>(char_Color.G()));
            title += SEQUENCE_ARG_SEPARATOR;
            title += std::to_string(static_cast<int>(char_Color.B()));
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
            //Bar Color
            Color c = _menu.getSelectedColor();
            if(bar.bar_Color != Color{0, 0, 0}){
                c = bar.bar_Color;
            }
            //bar left
            std::string bar_left = bar.before_option;
            if(i == _menu.getSelectedIndex()){
                bar_left = bar.selected_before;
            }

            str_toPrint += ESC_COLOR_CODE;
            str_toPrint += FOREGROUND_SEQUENCE;
            str_toPrint += std::to_string(static_cast<int>(c.R()));
            str_toPrint += SEQUENCE_ARG_SEPARATOR;
            str_toPrint += std::to_string(static_cast<int>(c.G()));
            str_toPrint += SEQUENCE_ARG_SEPARATOR;
            str_toPrint += std::to_string(static_cast<int>(c.B()));
            str_toPrint += CLOSE_SEQUENCE;
            str_toPrint += bar_left;
            option_length += bar_left.length();
            //text Color
            c = _menu.getDefaultColor();
            if(i == _menu.getSelectedIndex()){
                c = _menu.getSelectedColor();
            }
            if(opt.overwriteColor != Color{0, 0, 0}){
                c = opt.overwriteColor;
            }

            //text
            str_toPrint += ESC_COLOR_CODE;
            str_toPrint += FOREGROUND_SEQUENCE;
            str_toPrint += std::to_string(static_cast<int>(c.R()));
            str_toPrint += SEQUENCE_ARG_SEPARATOR;
            str_toPrint += std::to_string(static_cast<int>(c.G()));
            str_toPrint += SEQUENCE_ARG_SEPARATOR;
            str_toPrint += std::to_string(static_cast<int>(c.B()));
            str_toPrint += CLOSE_SEQUENCE;
            str_toPrint += opt.text;
            option_length += opt.text.length();
            //bar Color
            c = _menu.getSelectedColor();
            if(bar.bar_Color != Color{0, 0, 0}){
                c = bar.bar_Color;
            }
            //bar right
            std::string bar_right = bar.after_option;
            if(i == _menu.getSelectedIndex()){
                bar_right = bar.selected_after;
            }

            str_toPrint += ESC_COLOR_CODE;
            str_toPrint += FOREGROUND_SEQUENCE;
            str_toPrint += std::to_string(static_cast<int>(c.R()));
            str_toPrint += SEQUENCE_ARG_SEPARATOR;
            str_toPrint += std::to_string(static_cast<int>(c.G()));
            str_toPrint += SEQUENCE_ARG_SEPARATOR;
            str_toPrint += std::to_string(static_cast<int>(c.B()));
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

unsigned int hue_from_string_int(const std::string &s) {
    uint32_t h = fnv1a32(s);
    return h % 360u; // 0..359
}

double hue_from_string_double(const std::string &s) {
    uint32_t h = fnv1a32(s);
    return (static_cast<double>(h) / 4294967296.0) * 360.0;
}

void printMessage(string sender, string msg){
    cout << "<" << SET_BOLD << HSLtoRGB(hue_from_string_double(sender), 0.9, 0.69) << sender << RESET_ALL << "> " << msg << "\n";
}
