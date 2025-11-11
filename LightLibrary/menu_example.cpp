#include <iostream>
using namespace std;

#include "menuLight.h"
cli_menu* global_menu = nullptr;

void f_start()
{
    global_menu->selectSubMenu("Select a world to start your adventure");
}

void f_settings()
{
    cout << RESET_ALL << ERASE_CONSOLE;
    int width = global_menu->getWidth();
    int top_padding = 5;
    string settings = "Settings not implemented";
    cursor(width / 2 - settings.length() / 2, top_padding);
    cout << settings;
    for (int i = 0; i < 5; i++)
    {
        top_padding += 2;
        string placeHolder = "Place holder";
        cursor(width / 2 - placeHolder.length() / 2, top_padding);
        cout << placeHolder;
    }
    cursor(0, global_menu->getHeight() - 1);
    cout << "press enter to get back";
    _getch();
}

void f_exit()
{
    global_menu->exit();
}

void f_back()
{
    global_menu->selectSubMenu("Main menu example");
}

Color rainbowColor(double x)
{
    return HSLtoRGB(x * 720, 1.0, 0.5);
}

int main()
{
    cout << "Resize the console and set your desired font size (ctrl + + or ctrl + scroll wheel)\nThe interactive menu will get resize acordingly.";
    _getch();
    cli_menu menu({
        subMenu("Main menu example", {
                    UI_Option("Start", f_start),
                    UI_Option("Settings", f_settings),
                    UI_Option("Exit", {255, 15, 15} ,f_exit)
                    }, {255, 255, 255}, {255, 155, 255}),
        subMenu("Select a world to start your adventure", {
                    UI_Option("The Lord of The Rings"),
                    UI_Option("Starwars"),
                    UI_Option("Minecraft universe"),
                    UI_Option("Fantasy World"),
                    UI_Option("Fantasy World #2"),
                    UI_Option("A DnD campaign"),
                    UI_Option("Your favourite book"),
                    UI_Option("Dreamworld"),
                    UI_Option("Sky Castle"),
                    UI_Option("Back", {255, 15, 15} ,f_back)
                    }, {155, 155, 155}, {155, 155, 255}),
        });

    global_menu = &menu;

    (global_menu->findSubMenuByName("Select a world to start your adventure"))->setTitleColor(rainbowColor);

    menu.startLoop();

    return 0;
}