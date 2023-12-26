#include "Menu.h"

void noop(void* arg) {}

const int buttonRadius = 35;

PieMenu jogMenu("Jogging", buttonRadius);
PieMenu mainMenu("Main", buttonRadius);

class LB : public RoundButton {
public:
    LB(const char* text, callback_t callback, color_t base_color) :
        RoundButton(text, callback, buttonRadius, base_color, GREEN, BLUE, WHITE) {}
    LB(const char* text, Scene* scene, color_t base_color) : RoundButton(text, scene, buttonRadius, base_color, GREEN, BLUE, WHITE) {}
};

Scene* initMenus() {
    jogMenu.addItem(new LB("XAxis", noop, RED));
    jogMenu.addItem(new LB("YAxis", noop, RED));
    jogMenu.addItem(new LB("ZAxis", noop, RED));
    jogMenu.addItem(new LB("<Back", pop_scene, RED));

    mainMenu.addItem(new LB("Home", noop, RED));
    mainMenu.addItem(new LB("Jog", (Scene*)&jogMenu, DARKCYAN));
    mainMenu.addItem(new LB("Set", noop, MAROON));
    mainMenu.addItem(new LB("Probe", noop, ORANGE));
    mainMenu.addItem(new LB("Files", noop, OLIVE));
    mainMenu.addItem(new LB("Config", noop, BLUE));

    return &mainMenu;
}
