#include <Arduino.h>
#include "M5Dial.h"
#include "logo_img.h"

#define RED_BUTTON GPIO_NUM_13
#define GREEN_BUTTON GPIO_NUM_15
#define BUTTON_DEBOUNCE 20 // milliseconds

enum class MenuName : uint8_t
{
  Main = 0,
  Homing = 1,
  Jogging = 2,
};

long oldPosition = -4;
String stateString = "Idle";
float myAxes[6] = {0};
MenuName menu_number = MenuName::Main; // The menu that is currently active
int jog_axis = 0;                      // the axis currently being jogged
int jog_inc_level = 4;                 // exponent 0=0.01, 2=0.1 ... 5 = 100.00
static m5::touch_state_t prev_state;

void rotateNumberLoop(int &currentVal, int increment, int min, int max);

void main_menu(bool infoUpdate);
void homingMenu();
void joggingMenu();

// draw stuff
void drawDRO(int x, int y, int axis, float value, bool highlighted);
void drawStatus();
void drawButton(int x, int y, int width, int height, int charSize, String text, bool highlighted);
void buttonLegends(String red, String green, String orange);
void greenButtonInt();
void menuTitle();
String M5TouchStateName(m5::touch_state_t state_num);

M5Canvas canvas(&M5Dial.Display);

void setup()
{
  auto cfg = M5.config();
  M5Dial.begin(cfg, true, false);

  pinMode(RED_BUTTON, INPUT_PULLUP);   // Port A SCL
  pinMode(GREEN_BUTTON, INPUT_PULLUP); // Port A SDA

  USBSerial.begin(115200);
  USBSerial.println("M5Dial Pendant");

  HardwareSerial Serial_FNC(1);
  Serial_FNC.begin(115200, SERIAL_8N1, 1, 2); // reassign to the M5Stamp Port B

  M5Dial.Display.clear();
  M5Dial.Display.fillScreen(WHITE);
  M5Dial.Display.drawBitmap(20, 83, 199, 74, logo_img);

  delay(3000);

  myAxes[0] = 123.45;
  myAxes[1] = -1.89;
  myAxes[2] = 7123.45;

  main_menu(true);
}

void loop()
{
  M5Dial.update();
  long newPosition = M5Dial.Encoder.read();
  switch (menu_number)
  {
  case MenuName::Main:
    if (newPosition != oldPosition || M5Dial.BtnA.isPressed())
    {
      main_menu(false);
      oldPosition = newPosition;
    }
    break;
  case MenuName::Homing:
    homingMenu();
    break;
  case MenuName::Jogging:
    joggingMenu();
    break;
  }
}

void main_menu(bool infoUpdate)
{
  static long oldPosition = 0;
  static int menu_item = 0;
  long newPosition = M5Dial.Encoder.read();
  long delta = (newPosition - oldPosition) / 4;
  if (M5Dial.BtnA.isPressed())
  { // if jog dial buttom was press
    switch (menu_item)
    {
    case 0:
    case 1:
    case 2:
      menu_number = MenuName::Jogging;
      jog_axis = menu_item;
      joggingMenu();
      break;
    case 3:
      menu_number = MenuName::Homing;
      homingMenu();
      return;
    }
  }
  if (abs(delta) > 0 || infoUpdate)
  {
    rotateNumberLoop(menu_item, delta, 0, 4);
    oldPosition += delta * 4;
    // M5Dial.Speaker.tone(1000, 30);
    M5Dial.Display.clear();
    drawStatus();
    int y = 68;
    int spacing = 33;
    drawDRO(10, y, 0, myAxes[0], menu_item == 0);
    drawDRO(10, y += spacing, 1, myAxes[1], menu_item == 1);
    drawDRO(10, y += spacing, 2, myAxes[2], menu_item == 2);
    y = 170;
    drawButton(38, y, 74, 30, 12, "Home", menu_item == 3);
    drawButton(128, y, 74, 30, 12, "Probe", menu_item == 4);

    String encoder_button_text = "";
    switch (menu_item)
    {
    case 0:
      encoder_button_text = "Jog X";
      break;
    case 1:
      encoder_button_text = "Jog Y";
      break;
    case 2:
      encoder_button_text = "Jog Z";
      break;
    case 3:
      encoder_button_text = "Home";
      break;
    case 4:
      encoder_button_text = "Probe";
      break;
    }
    String redButtonText = "";
    String greenButtonText = "";
    if (stateString == "Alarm")
    {
      redButtonText = "Reset";
    }
    else if (stateString == "Run")
    {
      redButtonText = "Reset";
      greenButtonText = "Hold";
    }
    else if (stateString.startsWith("Hold"))
    {
      redButtonText = "Reset";
      greenButtonText = "Start";
    }
    else if (stateString == "Jog")
    {
      redButtonText = "Jog Cancel";
    }
    else if (stateString == "Idle")
    {
      // no commands?
    }
    menuTitle();
    buttonLegends(redButtonText, greenButtonText, encoder_button_text);
  }
}

void homingMenu()
{
  stateString = "Home";
  static long oldPosition = 0;
  static int menu_item = 0;
  long newPosition = M5Dial.Encoder.read();
  long delta = (newPosition - oldPosition) / 4;
  if (abs(delta) > 0)
  {
    oldPosition += delta * 4;
    M5Dial.Display.clear();
    drawStatus();

    int x = 30;
    int y = 65;
    int gap = 30;
    int width = 120;
    int height = 26;
    drawButton(x, y, width, height, 24, "Home All", true);
    drawButton(x, y += gap, width, height, 24, "Home X", false);
    drawButton(x, y += gap, width, height, 24, "Home Y", false);
    drawButton(x, y += gap, width, height, 24, "Home Z", false);

    x = 153;
    y = 65;
    width = 40;
    drawButton(x, y, width, height, 24, "0", false);
    drawButton(x, y += gap, width, height, 24, "0", false);
    drawButton(x, y += gap, width, height, 24, "0", false);
    drawButton(x, y += gap, width, height, 24, "0", false);

    menuTitle();
    buttonLegends("Rset", "Main", "Home All");
  }
}

void joggingMenu()
{
  static long oldPosition = 0;
  long newPosition = M5Dial.Encoder.read();
  long delta = (newPosition - oldPosition) / 4;
  bool touch = false;

  // Dial Button handling

  // A touch allows you to rotate through the axis being jogged 
  auto t = M5Dial.Touch.getDetail();
  if (prev_state != t.state)
  {
    if (t.state == m5::touch_state_t::touch_end)
    {
      rotateNumberLoop(jog_axis, 1, 0, 2);
      touch = true;
    }   
    USBSerial.printf("%s\r\n", M5TouchStateName(t.state));
    prev_state = t.state;
  }



  if (abs(delta) > 0 || touch)
  {
    oldPosition += delta * 4;

    M5Dial.Display.clear();
    drawStatus();

    drawDRO(10, 71, 0, myAxes[0], jog_axis == 0);
    drawDRO(10, 104, 1, myAxes[1], jog_axis == 1);
    drawDRO(10, 137, 2, myAxes[2], jog_axis == 2);

    M5Dial.Display.setTextFont(&fonts::FreeMonoBold12pt7b);
    M5Dial.Display.setTextColor(WHITE);
    M5Dial.Display.setTextDatum(middle_center);
    char buffer[20]; // Enough room for the digits you want and more to be safe
    float jog_increment = pow(10.0, jog_inc_level) / 100.0;
    dtostrf(jog_increment, 6, 2, buffer);
    String foo(buffer);
    foo = "Jog Inc:" + foo;
    M5Dial.Display.drawString(foo, 120, 185);

    menuTitle();
    buttonLegends("Inc+", "Inc-", "Main");
  }
}

void drawStatus()
{

  int rect_color = WHITE;

  if (stateString == "Idle")
  {
    rect_color = WHITE;
  }
  else if (stateString == "Alarm")
  {
    rect_color = RED;
  }
  else if (stateString == "Run")
  {
    rect_color = GREEN;
  }
  else if (stateString == "Jog")
  {
    rect_color = CYAN;
  }
  else if (stateString == "Home")
  {
    rect_color = CYAN;
  }
  else if (stateString.startsWith("Hold"))
  {
    rect_color = YELLOW;
  }

  static constexpr int x = 100;
  static constexpr int y = 24;
  static constexpr int width = 140;
  static constexpr int height = 34;
  M5Dial.Display.fillRoundRect(120 - width / 2, y, width, height, 5, rect_color);
  M5Dial.Display.setTextFont(&fonts::FreeSansBold18pt7b);
  M5Dial.Display.setTextColor(BLACK);
  M5Dial.Display.setTextDatum(middle_center);
  M5Dial.Display.drawString(stateString, 120, y + height / 2 + 3);
}

void drawDRO(int x, int y, int axis, float value, bool highlighted)
{
  int color_value, color_hightlight;
  M5Dial.Display.setTextFont(&fonts::FreeMonoBold18pt7b);

  String axis_label = String("XYZABC").substring(axis, axis + 1);

  static constexpr int width = 220;
  static constexpr int height = 32;

  color_value = WHITE;
  if (highlighted)
  {
    color_hightlight = BLUE;
  }
  else
  {
    color_hightlight = NAVY;
  }

  M5Dial.Display.setTextColor(WHITE);

  M5Dial.Display.fillRoundRect(x, y, width, height, 5, color_hightlight);
  M5Dial.Display.drawRoundRect(x, y, width, height, 5, color_value);

  M5Dial.Display.setTextColor(WHITE);
  M5Dial.Display.setTextDatum(middle_left);
  M5Dial.Display.drawString(axis_label, x + 5, y + height / 2 + 2);
  char buffer[20]; // Enough room for the digits you want and more to be safe
  dtostrf(value, 9, 2, buffer);
  M5Dial.Display.setTextDatum(middle_right);
  M5Dial.Display.drawString(String(buffer), x + width - 5, y + height / 2 + 2);
}

void rotateNumberLoop(int &currentVal, int increment, int min, int max)
{
  currentVal += increment;
  if (currentVal > max)
  {
    currentVal = min + (increment - 1);
  }
  if (currentVal < min)
  {
    currentVal = max - (increment + 1);
  }
}

void drawButton(int x, int y, int width, int height, int charSize, String text, bool highlighted)
{
  int color_value, color_hightlight;
  if (charSize = 12)
  {
    M5Dial.Display.setTextFont(&fonts::FreeSansBold12pt7b);
  }
  else
  {
    M5Dial.Display.setTextFont(&fonts::FreeSansBold18pt7b);
  }

  color_value = WHITE;
  if (highlighted)
  {
    color_hightlight = BLUE;
  }
  else
  {
    color_hightlight = NAVY;
  }
  M5Dial.Display.fillRoundRect(x, y, width, height, 5, color_hightlight);
  M5Dial.Display.drawRoundRect(x, y, width, height, 5, color_value);
  M5Dial.Display.setTextColor(WHITE);
  M5Dial.Display.setTextDatum(middle_center);
  M5Dial.Display.drawString(text, x + width / 2, y + height / 2 + 2);
}

void buttonLegends(String red, String green, String orange)
{
  M5Dial.Display.setTextFont(&fonts::FreeMonoBold9pt7b);
  M5Dial.Display.setTextDatum(middle_center);
  M5Dial.Display.setTextColor(RED);
  M5Dial.Display.drawString(red, 80, 209);
  M5Dial.Display.setTextColor(GREEN);
  M5Dial.Display.drawString(green, 160, 209);
  M5Dial.Display.setTextColor(ORANGE);
  M5Dial.Display.drawString(orange, 120, 224);
}

void menuTitle()
{
  String menu_names[] = {"Main", "Homing", "Jogging"};

  M5Dial.Display.setTextFont(&fonts::FreeMonoBold9pt7b);
  M5Dial.Display.setTextDatum(middle_center);
  M5Dial.Display.setTextColor(WHITE);

  M5Dial.Display.drawString(menu_names[(int)menu_number], 120, 12);
}

String M5TouchStateName(m5::touch_state_t state_num) {
   static constexpr const char *state_name[16] = {
        "none", "touch", "touch_end", "touch_begin",
        "___", "hold", "hold_end", "hold_begin",
        "___", "flick", "flick_end", "flick_begin",
        "___", "drag", "drag_end", "drag_begin"};

  return String(state_name[state_num]);
}
