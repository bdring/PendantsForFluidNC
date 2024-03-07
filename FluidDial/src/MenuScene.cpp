#include "Menu.h"
#include "PieMenu.h"
#ifdef USE_WMB_FSS
#    include "FileMenu.h"
#endif
#include "System.h"

void noop(void* arg) {}

const int buttonRadius = 30;

PieMenu axisMenu("Axes", buttonRadius);

class LB : public RoundButton {
public:
    LB(const char* text, callback_t callback, color_t base_color) :
        RoundButton(text, callback, buttonRadius, base_color, GREEN, BLUE, WHITE) {}
    LB(const char* text, Scene* scene, color_t base_color) : RoundButton(text, scene, buttonRadius, base_color, GREEN, BLUE, WHITE) {}
};

constexpr int LIGHTYELLOW = 0xFFF0;
class IB : public ImageButton {
public:
    IB(const char* text, callback_t callback, const char* filename) : ImageButton(text, callback, filename, buttonRadius, WHITE) {}
    IB(const char* text, Scene* scene, const char* filename) : ImageButton(text, scene, filename, buttonRadius, WHITE) {}
};

extern Scene homingScene;
extern Scene joggingScene;
extern Scene probingScene;
extern Scene statusScene;
#ifdef USE_WMB_FSS
extern Scene wmbFileSelectScene;
#else
extern Scene fileSelectScene;
#endif

extern Scene controlScene;
extern Scene setupScene;
extern Scene powerScene;

IB statusButton("Status", &statusScene, "statustp.png");
IB homingButton("Homing", &homingScene, "hometp.png");
IB jogButton("Jog", &joggingScene, "jogtp.png");
IB probeButton("Probe", &probingScene, "probetp.png");
#ifdef USE_WMB_FSS
IB filesButton("Files", &wmbFileSelectScene, "filestp.png");
#else
IB           filesButton("Files", &fileSelectScene, "filestp.png");
#endif

IB controlButton("Control", &controlScene, "controltp.png");
IB setupButton("Setup", &setupScene, "setuptp.png");
IB powerButton("Power", &powerScene, "powertp.png");

class MenuScene : public PieMenu {
public:
    MenuScene() : PieMenu("Main", buttonRadius) {}
    void onEntry(void* arg) {
        PieMenu::onEntry(arg);
        if (state == Disconnected) {
            statusButton.disable();
            homingButton.disable();
            jogButton.disable();
            probeButton.disable();
            filesButton.disable();
            controlButton.disable();
            setupButton.enable();
            powerButton.enable();
        }
    }
    void onStateChange(state_t old_state) override {
        if (state != Disconnected) {
            statusButton.enable();
            homingButton.enable();
            jogButton.enable();
            probeButton.enable();
            filesButton.enable();
            controlButton.enable();
            setupButton.enable();
            powerButton.enable();
        }
        reDisplay();
    }
} menuScene;

Scene* initMenus() {
    menuScene.addItem(&statusButton);
    menuScene.addItem(&homingButton);
    menuScene.addItem(&jogButton);
    menuScene.addItem(&probeButton);
    menuScene.addItem(&filesButton);
    menuScene.addItem(&controlButton);
    menuScene.addItem(&setupButton);
    menuScene.addItem(&powerButton);

    return &menuScene;
}
