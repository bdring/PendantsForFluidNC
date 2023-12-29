#include "Menu.h"
#include "FileMenu.h"

void noop(void* arg) {}

const int buttonRadius = 30;

FileMenu fileMenu("Files");
PieMenu  axisMenu("Axes", buttonRadius);
PieMenu  mainMenu("Main", buttonRadius);

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
extern Scene mainScene;

Scene* initMenus() {
    fileMenu.addItem(new FileItem("BigTree.nc"));
    fileMenu.addItem(new FileItem("BotCustom.nc"));
    fileMenu.addItem(new FileItem("Engraving.gcode"));
    fileMenu.addItem(new FileItem("Fixtures/"));
    fileMenu.addItem(new FileItem("Pucks.nc"));
    fileMenu.addItem(new FileItem("TopCutout.nc"));
    fileMenu.addItem(new FileItem("TopTLines.nc"));
    fileMenu.setFolder("/");

    axisMenu.addItem(new LB("XAxis", noop, RED));
    axisMenu.addItem(new LB("YAxis", noop, RED));
    axisMenu.addItem(new LB("ZAxis", noop, RED));
    axisMenu.addItem(new LB("<Back", pop_scene, RED));

    mainMenu.addItem(new IB("Status", &mainScene, "/statustp.png"));
    mainMenu.addItem(new IB("Homing", &homingScene, "/hometp.png"));
    mainMenu.addItem(new IB("Jog", &joggingScene, "/jogtp.png"));
    mainMenu.addItem(new IB("Probe", &probingScene, "/probetp.png"));
    mainMenu.addItem(new IB("Files", &fileMenu, "/filestp.png"));
    mainMenu.addItem(new IB("Control", noop, "/controltp.png"));
    mainMenu.addItem(new IB("Setup", noop, "/setuptp.png"));
    mainMenu.addItem(new IB("Power", noop, "/powertp.png"));

    return &mainMenu;
}
