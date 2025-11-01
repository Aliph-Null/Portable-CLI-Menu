#include <iostream>
#include "menu.h"

cliMenu menu;


//---------------For the gamling
#include <thread>
#include <chrono>

using namespace std;

c_pixel GOLDRED(double x, double y){
    color new_color = HSLtoRGB(x*y * 30, 1, 0.5);
    c_pixel result = c_pixel(new_color);

    return result;
}

void gamble(){
    srand(time(0));
    //Clear Buffer
    menu.init();

    const Character* pch = (fonts[AvailableFonts::Aligator2])['A'];

    int menu_width = menu.width;
    int middle_y = menu.height / 2 - (pch->height/2);

    const int how_many_numers = 3;
    int displacement = menu.width / (how_many_numers+1) - (pch->width/2);

    string available_chars = "X123456789";
    u32string border = U"♥♦♣♠";
    vector<color> colors = {
        {255, 25, 25},
        {255, 255, 25},
        {60, 120, 255},
        {60, 255, 60},
        {60, 60, 255}
    };

    vector<color> border_colors = {
        {25, 25, 255},
        {255, 25, 25}
    };


    int frame_multiplier = 3;
    vector<int> numbers_to_shuffle = {6*frame_multiplier, 12*frame_multiplier, 18*frame_multiplier};
    int delay_millis = 1000/numbers_to_shuffle[0];
    char selected[3] = {'1', '2', '3'};

    for(int frame = 0; frame < numbers_to_shuffle[2]; frame++){
        //Clear Buffer
        //menu.init();
        cout << ERASE_CONSOLE;
        cout << "\a";

        //Logic ...... no logic it is gambling
        for(int i = 0; i < 3; i++){
            char& ch = selected[i];
            if(frame <= numbers_to_shuffle[i])
                ch = available_chars[rand() % available_chars.length()];

            coords pos{displacement * (i+1), middle_y};

            const Character* pch = (fonts[AvailableFonts::Mono12])[ch];
            if(!pch)
                continue;

            color col = colors[rand() % colors.size()];
            if(frame >= numbers_to_shuffle[i]-2)
                col = color{255, 215, 0};

            menu.DrawOneChar(pos, pch, col);
        }

        for(int i = 0; i < menu.width; i++){
            coords pos1{i, 0}, pos2{i, menu.height-1};
            menu.rawBufferDrawChar(pos1, border[(pos1.x + pos1.y) % border.length()]);
            menu.rawBufferDrawChar(pos2, border[(pos1.x + pos1.y) % border.length()]);

            menu.rawBufferDrawColor(pos1, c_pixel(border_colors[(pos1.x + pos1.y + frame) % 2]));
            menu.rawBufferDrawColor(pos2, c_pixel(border_colors[(pos2.x + pos2.y + frame) % 2]));
        }

        for(int i = 0; i < menu.height; i++){
            coords pos1{0, i}, pos2{menu.width-1, i};
            menu.rawBufferDrawChar(pos1, border[(pos1.x + pos1.y) % border.length()]);
            menu.rawBufferDrawChar(pos2, border[(pos1.x + pos1.y) % border.length()]);

            menu.rawBufferDrawColor(pos1, c_pixel(border_colors[(pos1.x + pos1.y + frame) % 2]));
            menu.rawBufferDrawColor(pos2, c_pixel(border_colors[(pos2.x + pos2.y + frame) % 2]));
        }

        menu.printBuffer();
        std::this_thread::sleep_for(std::chrono::milliseconds(delay_millis));
    }

    vector<vector<char>> winning_conditions = {
        {'3', '2', '1'},
        {'1', '2', '3'},
        {'4', '2', '0'},
        {'6', '9', '6'},
        {'9', '6', '9'}
    };// + daca toate sunt la fel

    bool hasWon = false;
    hasWon = (selected[0] == selected[1] && selected[1] == selected[2]);
    if(!hasWon){
        for(vector<char>& winning_condition : winning_conditions){
            if(selected[0] == winning_condition[0] && selected[1] == winning_condition[1] && selected[2] == winning_condition[2]){
                hasWon = true;
                break;
            }
        }
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(1500));
    //Clear Buffer
    menu.init();
    const Font* font = &fonts[AvailableFonts::Bloody];
    string congrats = "LOSS";
    if(hasWon){
        congrats = "WINNER";
        font = &fonts[AvailableFonts::AnsiShadow];
    }


    coords middle{menu.width/2, menu.height/2};
    if(hasWon){
        menu.DrawStringCenterCords(middle, congrats, font, rainbowUV);
    }else{
        menu.DrawStringCenterCords(middle, congrats, font, GOLDRED);
    }
    menu.printBuffer();


    std::this_thread::sleep_for(std::chrono::milliseconds(3000));
}


void HelpUserSetScreenSize(){
    cout << "Resize Screen and font size, menu will detect them but will NOT update if console changes. \n Press enter after you are done.";
    getch();
    menu.init();
}

void goToGame2(){
    menu.SelectSubMenu("PLAY");
}

void exit(){
    menu.exit = true;
}

void setari(){
    cout << RESET_ALL << ERASE_CONSOLE;
    cout << "Debug stuff\n";
    cout << "-------------------------------------\n";
    cout << "Window size (in chars): height: " << menu.height << "\twidth: " << menu.width << "\n";
    cout << "Current selected menu index: " << menu.currentMenu << "\t nume " << menu.submenus[menu.currentMenu].name << "\n";
    //cout << "\n-------------------------------------\n";
    cout << "-------------------------------------\n";
    cout << "Cate Submeniuri avem? : " << menu.submenus.size() << "\n";
    for(subMenu& submenu : menu.submenus){
        cout << "Nume: " << submenu.name << " cu " << submenu.options.size() << " optiuni\n";
        for(UI_Option& option: submenu.options){
            cout << "\t - " << option.text << " -\n";
        }
    }
    cout << "-------------------------------------\n";

    getch();
}

void returnToMainMenu(){
    menu.SelectSubMenu(0);
}

int main() {

    HelpUserSetScreenSize();

    menu.addBorder();

    subMenu welcome(">LASVEGAS<");
    welcome.colorFunction = rainbowUV;

    UI_Option exit_option("Exit", exit);
    exit_option.overwriteColor_huh = true;
    exit_option.overwiteColor = c_pixel(255, 25, 25);

    welcome.addOptions({
        UI_Option("Start", goToGame2),
        UI_Option("Debug menu", setari),
        exit_option
                       });

    welcome.setFontFromDefault(AvailableFonts::AnsiShadow);
    menu.submenus.emplace_back(welcome);


    subMenu play("PLAY");
    play.colorFunction = GOLDRED;
    play.addOptions({
        UI_Option("Gamble", gamble),
        UI_Option("Return to main menu", returnToMainMenu)
                       });
    //play.setFontFromDefault(AvailableFonts::Mono12);
    menu.submenus.emplace_back(play);

    menu.startLoop();

    return 0;
}
