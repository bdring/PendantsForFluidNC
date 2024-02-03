#include "Menu.h"

#if 0
// Confirm scene needs a string to display.  It displays the string on
// a background depicting the red and green buttons.
// It pops when one of those buttons is pressed, on on back and swipe back,
// setting a variable to true iff the green button was pressed
#endif

class ConfirmScene : public Scene {
    String _msg;

public:
    ConfirmScene() : Scene("Confirm") {}
    void onEntry(void* arg) { _msg = (const char*)arg; }
    void reDisplay() {
        drawBackground(BLACK);  // XXX need PNG of red and green buttons
        canvas.fillRoundRect(10, 90, 220, 60, 15, YELLOW);
        centered_text(_msg, 120, BLACK, MEDIUM);

        drawButtonLegends("No", "Yes", "Back");

        refreshDisplay();
    }
    void onRedButtonRelease() { pop_scene(nullptr); }
    void onGreenButtonRelease() { pop_scene((void*)"Confirmed"); }
    void onDialButtonRelease() { pop_scene(nullptr); }
};
ConfirmScene confirmScene;
