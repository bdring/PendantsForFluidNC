// Copyright (c) 2023 - Barton Dringstarting
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#include <Arduino.h>
#include "Scene.h"
#include "FileParser.h"

class FilePreviewScene : public Scene {
    String _filename;
    bool   _needlines;

public:
    FilePreviewScene() : Scene("Preview") {}
    void init(void* arg) {
        char* fname = (char*)arg;
        _filename   = fname;
        _needlines  = true;
        request_file_preview(fname);
    }
    void onFileLines() {
        _needlines = false;
        reDisplay();
    }

    void onDialButtonPress() { pop_scene(); }

    void onGreenButtonPress() {
        String command = "$SD/Run=" + dirName + "/" + fileInfo.fileName;
        send_line(command.c_str());
    }
    void reDisplay() {
        canvas.createSprite(240, 240);
        drawBackground(BLACK);
        drawMenuTitle(name());

        if (_needlines == false) {
            int y  = 36;
            int tl = 0;
            if (fileLines.size()) {
                for (auto const& line : fileLines) {
                    text(line, 25, y + tl * 22, WHITE, TINY, top_left);
                    ++tl;
                }
            } else {
                text("No Text", 120, 120, WHITE, SMALL, middle_center);
            }
        } else {
            text("Reading File", 120, 120, WHITE, TINY, middle_center);
        }

        drawButtonLegends("", "Run", "Back");
        refreshDisplay();
    }
};
FilePreviewScene filePreviewScene;
