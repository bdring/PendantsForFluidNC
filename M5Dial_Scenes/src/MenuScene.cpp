#include "Menu.h"
#include "PieMenu.h"
#include "FileMenu.h"
#include "System.h"

void noop(void* arg) {}

const int buttonRadius = 30;

FileMenu fileMenu("Files");
PieMenu  axisMenu("Axes", buttonRadius);

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
extern Scene filesScene;
extern Scene controlScene;
extern Scene setupScene;
extern Scene powerScene;

IB statusButton("Status", &statusScene, "/statustp.png");
IB homingButton("Homing", &homingScene, "/hometp.png");
IB jogButton("Jog", &joggingScene, "/jogtp.png");
IB probeButton("Probe", &probingScene, "/probetp.png");
IB filesButton("Files", &filesScene, "/filestp.png");
IB controlButton("Control", &controlScene, "/controltp.png");
IB setupButton("Setup", &setupScene, "/setuptp.png");
IB powerButton("Power", &powerScene, "/powertp.png");

class MenuScene : public PieMenu {
public:
    MenuScene() : PieMenu("Main", buttonRadius) {}
    void init(void* arg) {
        PieMenu::init(arg);
        if (state == Disconnected) {
            log_println("Menu Scene in disconnected state");
            statusButton.disable();
            homingButton.disable();
            jogButton.disable();
            probeButton.disable();
            filesButton.disable();
            controlButton.disable();
            setupButton.enable();
            powerButton.enable();
        } else {
            log_println("Menu Scene in Connected state");
        }
    }
    void onStateChange(state_t state) override {
        if (state != Disconnected) {
            log_println("Menu state change not disconnected");
            statusButton.enable();
            homingButton.enable();
            jogButton.enable();
            probeButton.enable();
            filesButton.enable();
            controlButton.enable();
            setupButton.enable();
            powerButton.enable();
        } else {
            log_println("Menu state change IS disconnected");
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
