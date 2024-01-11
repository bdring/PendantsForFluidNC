// Copyright (c) 2023 Mitch Bradley
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#include <Arduino.h>
#include "Scene.h"

Scene* current_scene;

Button greenButton;
Button redButton;
Button dialButton;

std::vector<Scene*> scene_stack;

void activate_scene(Scene* scene, void* arg) {
    current_scene = scene;
    current_scene->init(arg);
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
        activate_scene(last_scene);
    }
}
void activate_at_top_level(Scene* scene, void* arg) {
    scene_stack.clear();
    activate_scene(scene, arg);
}

void dispatch_events() {
    static m5::touch_state_t last_touch_state = {};

    M5Dial.update();

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

    bool this_button;
    if (dialButton.changed(this_button)) {
        if (this_button) {
            current_scene->onDialButtonPress();
        } else {
            current_scene->onDialButtonRelease();
        }
    }

    if (redButton.changed(this_button)) {
        if (this_button) {
            current_scene->onRedButtonPress();
        } else {
            current_scene->onRedButtonRelease();
        }
    }

    if (greenButton.changed(this_button)) {
        if (this_button) {
            current_scene->onGreenButtonPress();
        } else {
            current_scene->onGreenButtonRelease();
        }
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
void Scene::setPref(const char* base_name, int axis, int value) {
    if (!_prefs) {
        return;
    }
    String setting_name = base_name + axisNumToString(axis);
    nvs_set_i32(_prefs, setting_name.c_str(), value);
}
void Scene::getPref(const char* base_name, int axis, int* value) {
    if (!_prefs) {
        return;
    }
    String setting_name = base_name + axisNumToString(axis);
    nvs_get_i32(_prefs, setting_name.c_str(), value);
}
bool Scene::initPrefs() {
    if (_prefs) {
        return false;  // Already open
    }
    esp_err_t err = nvs_open(name().c_str(), NVS_READWRITE, &_prefs);
    return err == ESP_OK;
}

int Scene::scale_encoder(int delta) {
    _encoder_accum += delta;
    int res = _encoder_accum / _encoder_scale;
    _encoder_accum %= _encoder_scale;
    return res;
}
