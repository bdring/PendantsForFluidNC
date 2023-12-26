#include "Menu.h"

void noop(void* arg) {}

PieMenu* pieMenu0;
PieMenu* sm1;
void     runSubmenu1(void* arg) {
    push_scene(sm1);
}
void initPieMenu() {
    const int pmRadius = 40;

    pieMenu0 = new PieMenu("PinMenu0", 50, display.width() / 2 - pmRadius - 2);

    pieMenu0->addItem(new RoundButton("A", noop, pmRadius, RED, GREEN, BLUE, WHITE));
    pieMenu0->addItem(new RoundButton("B", runSubmenu1, pmRadius, DARKCYAN, GREEN, BLUE, WHITE));
    pieMenu0->addItem(new RoundButton("C", noop, pmRadius, MAROON, GREEN, BLUE, WHITE));
    pieMenu0->addItem(new RoundButton("D", noop, pmRadius, ORANGE, GREEN, BLUE, WHITE));
    pieMenu0->addItem(new RoundButton("E", noop, pmRadius, OLIVE, GREEN, BLUE, WHITE));
    pieMenu0->addItem(new RoundButton("F", noop, pmRadius, BLUE, GREEN, BLUE, WHITE));
    pieMenu0->addItem(new RoundButton("G", noop, pmRadius, PINK, GREEN, BLUE, WHITE));
    // pieMenu0->addItem(new RoundButton("H", noop, pmRadius, SILVER, GREEN, BLUE, WHITE));
    //    pieMenu0->addItem(new RoundButton("F", noop, pmRadius, BLUE, GREEN, BLUE, WHITE));

    sm1 = new PieMenu("Submenu1", 50, display.width() / 2 - pmRadius - 2);
    sm1->addItem(new RoundButton("X", noop, pmRadius, RED, GREEN, BLUE, WHITE));
    sm1->addItem(new RoundButton("Y", noop, pmRadius, RED, GREEN, BLUE, WHITE));
    sm1->addItem(new RoundButton("Z", pop_scene, pmRadius, RED, GREEN, BLUE, WHITE));
}
