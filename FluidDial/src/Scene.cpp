// Copyright (c) 2023 Mitch Bradley
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#include <Arduino.h>
#include "Scene.h"
#include "Drawing.h"

Scene* current_scene = nullptr;

std::vector<Scene*> scene_stack;

void activate_scene(Scene* scene, void* arg) {
    if (current_scene) {
        current_scene->onExit();
    }
    current_scene = scene;
    current_scene->onEntry(arg);
    current_scene->reDisplay();
}
void push_scene(Scene* scene, void* arg) {
    scene_stack.push_back(current_scene);
    activate_scene(scene, arg);
}
void pop_scene(void* arg) {
    if (scene_stack.size()) {
        Scene* last_scene = scene_stack.back();
        scene_stack.pop_back();
        activate_scene(last_scene, arg);
    }
}
void activate_at_top_level(Scene* scene, void* arg) {
    scene_stack.clear();
    activate_scene(scene, arg);
}

void dispatch_events() {
    static m5::touch_state_t last_touch_state = {};

    M5Dial.update();
    auto ms = m5gfx::millis();

    // The red and green buttons are active low
    redButton.setRawState(ms, !m5gfx::gpio_in(RED_BUTTON_PIN));
    greenButton.setRawState(ms, !m5gfx::gpio_in(GREEN_BUTTON_PIN));

    static int16_t oldEncoder   = 0;
    int16_t        newEncoder   = get_encoder();
    int16_t        encoderDelta = newEncoder - oldEncoder;
    if (encoderDelta) {
        oldEncoder = newEncoder;

        int16_t scaledDelta = current_scene->scale_encoder(encoderDelta);
        if (scaledDelta) {
            current_scene->onEncoder(scaledDelta);
        }
    }

    if (dialButton.wasPressed()) {
        current_scene->onDialButtonPress();
    } else if (dialButton.wasReleased()) {
        current_scene->onDialButtonRelease();
    }
    if (redButton.wasPressed()) {
        current_scene->onRedButtonPress();
    } else if (redButton.wasReleased()) {
        current_scene->onRedButtonRelease();
    }
    if (greenButton.wasPressed()) {
        current_scene->onGreenButtonPress();
    } else if (greenButton.wasReleased()) {
        current_scene->onGreenButtonRelease();
    }

    auto this_touch = touch.getDetail();
    if (this_touch.state != last_touch_state) {
        last_touch_state = this_touch.state;
        // debugPort.printf("Touch %d\r\n", this_touch.state);
        if (this_touch.state == m5::touch_state_t::touch) {
            //speaker.tone(1800, 50);
            current_scene->onTouchPress(this_touch.x, this_touch.y);
        } else if (this_touch.wasClicked()) {
            current_scene->onTouchRelease(this_touch.x, this_touch.y);
        } else if (this_touch.wasHold()) {
            current_scene->onTouchHold(this_touch.x, this_touch.y);
        } else if (this_touch.state == m5::touch_state_t::flick_end) {
            current_scene->onTouchFlick(this_touch.x, this_touch.y, this_touch.distanceX(), this_touch.distanceY());
        }
    }

    if (!fnc_is_connected()) {
        if (state != Disconnected) {
            set_disconnected_state();
            extern Scene menuScene;
            activate_at_top_level(&menuScene);
        }
    }
}

void Scene::setPref(const char* name, int value) {
    if (!_prefs) {
        return;
    }
    nvs_set_i32(_prefs, name, value);
}
void Scene::getPref(const char* name, int* value) {
    if (!_prefs) {
        return;
    }
    nvs_get_i32(_prefs, name, value);
}
void Scene::setPref(const char* name, float value) {
    if (!_prefs) {
        return;
    }
    union {
        int32_t i;
        float   f;
    } val;
    val.f = value;
    nvs_set_i32(_prefs, name, val.i);
}
void Scene::getPref(const char* name, float* value) {
    if (!_prefs) {
        return;
    }
    union {
        int32_t i;
        float   f;
    } val;

    esp_err_t err = nvs_get_i32(_prefs, name, &val.i);
    if (err == ESP_OK) {
        *value = val.f;
    }
}
static const char* setting_name(const char* base_name, int axis) {
    static char name[32];
    sprintf(name, "%s%c", base_name, axisNumToChar(axis));
    return name;
}
void Scene::setPref(const char* base_name, int axis, int value) {
    if (!_prefs) {
        return;
    }
    nvs_set_i32(_prefs, setting_name(base_name, axis), value);
}
void Scene::getPref(const char* base_name, int axis, int* value) {
    if (!_prefs) {
        return;
    }
}
bool Scene::initPrefs() {
    if (_prefs) {
        return false;  // Already open
    }
    esp_err_t err = nvs_open(name(), NVS_READWRITE, &_prefs);
    return err == ESP_OK;
}

int Scene::scale_encoder(int delta) {
    _encoder_accum += delta;
    int res = _encoder_accum / _encoder_scale;
    _encoder_accum %= _encoder_scale;
    return res;
}
