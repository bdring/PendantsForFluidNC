// Copyright (c) 2024 - Mitch Bradley
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#include "Scene.h"
#include <driver/rtc_io.h>

// The M5 Library is broken with respect to deep sleep on M5 Dial
// so we have to do it ourselves.  The problem is that the WAKE
// button is supposed to be the dial button that connects to GPIO42,
// but that can't work because GPIO42 is not an RTC GPIO and thus
// cannot be used as an ext0 wakeup source.

// For some reason, GPIO_1 is the only one of the button pins
// that works right for wakeup.  If GPIO_2 is used, the system
// wakes up immediately without a button press.
#define WAKEUP_GPIO (gpio_num_t) RED_BUTTON_PIN

void deep_sleep(int us) {
    M5.Display.sleep();
    rtc_gpio_pullup_en(WAKEUP_GPIO);
    esp_sleep_enable_ext0_wakeup(WAKEUP_GPIO, false);
    while (digitalRead(WAKEUP_GPIO) == false) {
        delay(10);
    }
    if (us > 0) {
        esp_sleep_enable_timer_wakeup(us);
    } else {
        // esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_TIMER);
    }
    esp_deep_sleep_start();
}

class PowerScene : public Scene {
private:
public:
    PowerScene() : Scene("Power") {}
    void onEntry() { dbg_println("Power init"); }
    void onRedButtonPress() {
        set_disconnected_state();
        deep_sleep(0);
    }
    void reDisplay() {
        drawBackground(BLACK);
        drawButtonLegends("Sleep", "", "");
        refreshDisplay();
    }
} powerScene;
