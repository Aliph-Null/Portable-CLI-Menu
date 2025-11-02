#include <iostream>
#include <cmath>
#include <thread>
#include <chrono>

using namespace std;

#include "menuLight.h"
using namespace beautyPrint;
using namespace AvailableAlignments;
cli_menu* global_menu = nullptr;


/*
 * Example color functions to test gradient support
 */
color rainbowGradient(double x) {
    double r = std::sin(6.2831 * x) * 127 + 128;
    double g = std::sin(6.2831 * x + 2.094) * 127 + 128;
    double b = std::sin(6.2831 * x + 4.188) * 127 + 128;
    return {(unsigned char)r, (unsigned char)g, (unsigned char)b};
}

color blueToPurple(double x) {
    return {(unsigned char)(128 + 127 * std::sin(x * 3.1415)),
            (unsigned char)(0),
            (unsigned char)(255 * x)};
}

int main()
{
    cout << "Resize the console and set your desired font size (ctrl + + or ctrl + scroll wheel)\nThe interactive menu will get resize acordingly.";
    getch();
    cli_menu menu;
    menu.clearConsole();

    // Basic prints
    print("Hello from beautyPrint!");
    std::cerr << "\n\n";

    print("Hello from beautyPrint!", {255, 50, 50});
    std::cerr << "\n\n";

    print("Gradient demonstration", rainbowGradient);
    std::cerr << "\n\n";

    // Absolute position prints
    print({10, 5}, "Positioned text (10,5)");
    print({5, 7}, "Positioned text (5,7)", {0, 200, 200});
    print({0, 9}, "Gradient demonstration (0,9)", blueToPurple);
    std::cerr << "\n\n";

    // Alignment examples
    print({menu.getWidth(), 12}, "Alignment Test (LEFT)", LEFT);
    print({menu.getWidth(), 13}, "Alignment Test (CENTER)", CENTER);
    print({menu.getWidth(), 14}, "Alignment Test (RIGHT)", RIGHT);
    std::cerr << "\n\n";

    // Alignment + color / gradient
    print({menu.getWidth(), 16}, "Colored Center Example", CENTER, {255, 0, 0});
    print({menu.getWidth(), 17}, "Gradient Right Example", RIGHT, rainbowGradient);
    std::cerr << "\n\n";

    // Animated gradient demo
    std::string anim = "Animated Rainbow Gradient";
    for (int frame = 0; frame < 6000; ++frame) {
        cursor(0, 20);
        print(anim, [&](double x) {
            return HSLtoRGB(fmod((x + frame * 0.05) * 360.0, 360.0), 1.0, 0.5);
        });
        std::this_thread::sleep_for(std::chrono::milliseconds(60));
    }

    cursor(0, menu.getHeight() - 1);
    std::cerr << RESET_ALL << "\n";

    return 0;
}
