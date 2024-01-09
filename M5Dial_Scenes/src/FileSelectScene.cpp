// Copyright (c) 2023 - Barton Dringstarting
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#include <Arduino.h>
#include "Scene.h"
#include "FileParser.h"

String displayTitle = "Files";
int    selectedFile = -1;

class FilePreviewScene : public Scene {
    String _filename;

public:
    FilePreviewScene() : Scene("Preview") {}
    void init(void* arg) {
        char* fname = (char*)arg;
        _filename   = fname;
        request_file_preview(fname);
    }
    void onFileLines() { reDisplay(); }

    void onDialButtonPress() { pop_scene(); }

    void onGreenButtonPress() {
        String command = "$SD/Run=" + dirName + "/" + fileInfo.fileName;
        send_line(command.c_str());
    }
    void reDisplay() {
        canvas.createSprite(240, 240);
        drawBackground(BLACK);
        drawMenuTitle(name());

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

        drawButtonLegends("", "Run", "Back");
        refreshDisplay();
    }
};
FilePreviewScene filePreviewScene;

class FileSelectScene : public Scene {
private:
    int _displayIndex = -1;

    const char* format_size(size_t size) {
        const int   buflen = 30;
        static char buffer[buflen];
        if (size > 999999.9) {
            snprintf(buffer, buflen, "%.3f MB", size / 1000000.0);
        } else if (size > 999.9) {
            snprintf(buffer, buflen, "%.3f KB", size / 1000.0);
        } else {
            snprintf(buffer, buflen, "%.0f bytes", size);
        }
        return buffer;
    }

public:
    FileSelectScene() : Scene("Files", 2) {}

    void init(void* arg) {}

    void onDialButtonPress() { pop_scene(); }

    // XXX this should probably be a touch release on the file display
    void onGreenButtonPress() {
        if (selectedFile > -1) {
            String dName;
            fileInfo = fileVector[selectedFile];
            switch (fileInfo.fileType) {
                case 1:  //directory
                    enter_directory(fileInfo.fileName);
                    break;
                case 2:  // file
                    push_scene(&filePreviewScene, (void*)fileInfo.fileName.c_str());
                    break;
            }
        }
    }

    // XXX maybe a touch on the top of the screen i.e. the dirname field
    void onRedButtonPress() {
        if (dirLevel) {
            exit_directory();
            _displayIndex = -1;
        } else {
            init_file_list();
        }
    }

    void onTouchRelease(int x, int y) { onGreenButtonPress(); }

    void onFilesList() override { reDisplay(); }

    void onEncoder(int delta) override { scroll(delta); }

    void showFiles(int yo) {
        struct {
            int       _xb;
            int       _xt;
            int       _yb;  // y for box
            int       _yt;  // y for text center
            int       _w;
            int       _h;
            fontnum_t _f;    // text font
            int       _bg;   // bg color
            int       _txt;  // text color
        } box[] {
            //    _xb, _xt, _yb, _yt,  _w, _h, _f,      _bg,  _txt
            { 0, 119, 22, 31, 156, 18, TINY, BLACK, WHITE },         // [0] "SD Files", "localFS", "etc...";
            { 20, 120, 45, 59, 200, 23, SMALL, BLACK, WHITE },       // [1]file[0] fileName
            { 0, 120, 77, 82, 240, 18, SMALL, LIGHTGREY, BLUE },     // [2]file[1] Info Line Top
            { 0, 120, 78, 120, 240, 80, MEDIUM, LIGHTGREY, BLACK },  // [3]file[1] fileName
            { 0, 120, 136, 139, 240, 18, TINY, LIGHTGREY, BLACK },   // [4]file[1] Info Line Bottom
            { 20, 120, 168, 181, 200, 23, SMALL, BLACK, WHITE },     // [5]file[2] fileName
        };
        int box_fi[] = { 1, 3, 5 };

        drawBackground(BLACK);
        displayTitle = current_scene->name();
        drawMenuTitle(displayTitle);
        String fName;
        int    finfoT_color = BLUE;

        int fdIter   = _displayIndex;  // first file in display list
        selectedFile = -1;

        for (int fx = 0; fx < 3; fx++, fdIter++) {
            int  fi     = box_fi[fx];
            auto middle = box[fi];

            if (fdIter < 0) {
                continue;
            }

            fName = "< no files >";
            if (fileVector.size()) {
                fName = fileVector[fdIter].fileName;
            }
            if (yo == 0 && middle._bg != BLACK) {
                canvas.fillRoundRect(middle._xb, yo + middle._yb, 240, middle._h, middle._h / 2, middle._bg);
            }
            if (fx == 1) {
                String fInfoT = "";  // file info top line
                String fInfoB = "";  // File info bottom line
                int    ext    = fName.lastIndexOf('.');
                float  fs     = 0.0;
                if (fileVector.size()) {
                    selectedFile = fdIter;
                    if (selectedFile > -1) {
                        switch (fileVector[selectedFile].fileType) {
                            case 0:
                                break;
                            case 1:
                                fInfoT       = "Folder";
                                finfoT_color = GREENYELLOW;
                                break;
                            case 2:
                                if (ext > 0) {
                                    fInfoT = fName.substring(ext, fName.length());
                                    fInfoT += " file";
                                    fName.remove(ext);
                                }
                                fInfoB = format_size(fileVector[selectedFile].fileSize);
                                break;
                        }
                    }
                }
                if (yo == 0) {
                    auto top    = box[fi - 1];
                    auto bottom = box[fi + 1];
                    canvas.fillRoundRect(middle._xb, yo + middle._yb, middle._w, middle._h, middle._h / 2, middle._bg);
                    text(fInfoT, top._xt, yo + top._yt, finfoT_color, top._f, top_center);
                    text(fInfoB, bottom._xt, yo + bottom._yt, bottom._txt, bottom._f, top_center);
                }
            }

            if ((yo == 0) || (yo < 0 && yo + middle._yt > 45) || (yo > 0 && yo + middle._yt + middle._h < 202)) {
                auto_text(fName, middle._xt, yo + middle._yt, middle._w, (yo) ? WHITE : middle._txt, middle._f, middle_center);
            }

            if (fdIter >= fileVector.size() - 1) {
                break;
            }
        }
    }

    void scroll(int updown) {
        int nextIndex = _displayIndex + updown;
        if (nextIndex < -1 || nextIndex > (int)(fileVector.size() - 2)) {
            return;
        }
        const int yinc   = updown * 10;
        const int ylimit = 60;
        int       yo     = yinc;

        while (abs(yo) < ylimit) {
            showFiles(yo);
            delay(10);
            yo += yinc;
            refreshDisplay();
        }
        _displayIndex = nextIndex;
        reDisplay();
    }

#if 0
    void displayFileAction() {
        canvas.createSprite(240, 240);
        drawBackground(BLACK);
        displayTitle = "Action";
        drawMenuTitle(displayTitle);

        String s_n, s_e;
        // break filename into pieces

        s_n = fileInfo.fileName;

        int ix = s_n.lastIndexOf(".");
        if (ix > -1) {
            s_e = s_n.substring(ix, s_n.length());    // extension
            s_n.remove(s_n.length() - s_e.length());  // name
        }

        fontnum_t f       = MEDIUM;
        bool      oneline = false;

        if (!oneline && text_fits(s_n, f, 240)) {
            oneline = true;
        }
        if (!oneline) {
            f = SMALL;
            if (text_fits(s_n, f, 240)) {
                oneline = true;
            }
        }
        if (oneline) {
            text(s_n, 120, 110, WHITE, f);
        } else {
            text(s_n.substring(0, s_n.length() / 2) + "...", 120, 85, WHITE, f);
            text(s_n.substring(s_n.length() / 2, s_n.length()), 120, 115, WHITE, f);
        }
        auto_text(s_e, 120, 136, 240, WHITE, SMALL);

        String redText = "Cancel";
        String grnText = "";

        if (actionMode == fileRun) {
            grnText = "Run";
        }
        if (actionMode == fileShow) {
            grnText = "Show";
        }

        drawButtonLegends(redText, grnText, "Back");
        refreshDisplay();
    }
#endif

    void displayFileSelect() {
        canvas.createSprite(240, 240);

        showFiles(0);

        String grnText = "";
        String redText = dirLevel ? "Up..." : "Refresh";
        if (fileVector.size()) {
            if (selectedFile > -1) {
                switch (fileVector[selectedFile].fileType) {
                    case 0:
                        break;
                    case 1:
                        grnText = "Load";
                        break;
                    case 2:
                        grnText = "Select";
                        break;
                }
            }
        }

        drawButtonLegends(redText, grnText, "Back");
        refreshDisplay();
    }

    void reDisplay() { displayFileSelect(); }
};
FileSelectScene filesScene;
