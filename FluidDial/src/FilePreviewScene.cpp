// Copyright (c) 2023 - Barton Dringstarting
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#include <string>
#include "Scene.h"
#include "FileParser.h"

extern Scene menuScene;
extern Scene statusScene;

class FilePreviewScene : public Scene {
    std::string _error_string;
    std::string _filename;
    bool        _needlines;

public:
    FilePreviewScene() : Scene("Preview") {}
    void onEntry(void* arg) {
        if (arg) {
            char* fname = (char*)arg;
            _filename   = fname;
            _needlines  = true;
            request_file_preview(fname);
        } else {
            _needlines = false;
        }
    }
    void onFileLines() {
        _error_string.clear();
        _needlines = false;
        reDisplay();
    }
    void onError(const char* errstr) {
        _error_string = errstr;
        reDisplay();
    }

    void onDialButtonPress() { pop_scene(); }

    void onRedButtonPress() {
        if (state == Idle) {
            pop_scene();
            ackBeep();
        }
    }

    void onDROChange() { reDisplay(); }

    void onGreenButtonPress() {
        if (state == Idle) {
            send_linef("$SD/Run=%s", _filename.c_str());
            ackBeep();
        }
    }

    void onStateChange(state_t old_state) {
        if (state == Cycle) {
            push_scene(&statusScene);
        }
    }

    void reDisplay() {
        background();
        drawMenuTitle(name());

        const char* grnLabel = "";
        const char* redLabel = "";

        if (state == Idle) {
            if (_needlines == false) {
                int y  = 48;
                int tl = 0;
                if (fileLines.size()) {
                    for (auto const& line : fileLines) {
                        text(line.c_str(), 25, y + tl * 22, WHITE, TINY, top_left);
                        ++tl;
                    }
                } else {
                    text("Empty File", 120, 120, WHITE, SMALL, middle_center);
                }
            } else if (_error_string.length()) {
                text(_error_string, 120, 120, WHITE, SMALL, middle_center);
            } else {
                text("Reading File", 120, 120, WHITE, TINY, middle_center);
            }
            grnLabel = "Run";
            redLabel = "Back";
        } else {
            centered_text("Invalid State", 105, WHITE, SMALL);
            centered_text("File Preview", 145, WHITE, SMALL);
        }

        drawButtonLegends(redLabel, grnLabel, "Back");
        drawStatusSmall(21);
        refreshDisplay();
    }
};
FilePreviewScene filePreviewScene;
