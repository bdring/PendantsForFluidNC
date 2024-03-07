// Copyright (c) 2023 Mitch Bradley
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#include "Scene.h"

class HelpScene : public Scene {
public:
    HelpScene() : Scene("Help") {}
    void onEntry(void* arg) {
        const char** msg = static_cast<const char**>(arg);
        const char*  line;
        drawBackground(BROWN);
        int pos = 20;
        for (; line = *msg, line; ++msg) {
            centered_text(line, pos, WHITE, TINY);
            pos += 30;
        }
        drawButtonLegends("", "", "Back");
        refreshDisplay();
    }
    void onDialButtonRelease() { pop_scene(nullptr); }
} helpScene;
