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

extern Scene homingScene;
extern Scene joggingScene;
extern Scene probingScene;

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

    mainMenu.addItem(new ImageButton("Status", &mainMenu, "/status.png", buttonRadius));
    mainMenu.addItem(new ImageButton("Homing", &homingScene, "/home.png", buttonRadius));
    mainMenu.addItem(new ImageButton("Jog", &joggingScene, "/jog.png", buttonRadius));
    mainMenu.addItem(new ImageButton("Probe", &probingScene, "/probe.png", buttonRadius));
    mainMenu.addItem(new ImageButton("Files", &fileMenu, "/files.png", buttonRadius));
    mainMenu.addItem(new ImageButton("Control", noop, "/control.png", buttonRadius));
    mainMenu.addItem(new ImageButton("Setup", noop, "/setup.png", buttonRadius));
    mainMenu.addItem(new ImageButton("Power", noop, "/power.png", buttonRadius));

    return &mainMenu;
}
