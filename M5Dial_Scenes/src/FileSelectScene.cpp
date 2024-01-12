// Copyright (c) 2023 - Barton Dring
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#include <Arduino.h>
#include "Scene.h"
#include "FileParser.h"

#define WRAP_FILE_LIST

extern Scene filePreviewScene;

String displayTitle = "Files";

class FileSelectScene : public Scene {
private:
    int _selected_file = 0;

    const char* format_size(size_t size) {
        const int   buflen = 30;
        static char buffer[buflen];
        if (size >= 1000000) {
            int mb      = size / 1000000;
            int residue = size % 1000000;
            snprintf(buffer, buflen, "%d.%03d MB", mb, residue / 1000);
        } else if (size > 1000) {
            int kb      = size / 1000;
            int residue = size % 1000;
            snprintf(buffer, buflen, "%d.%03d KB", kb, residue);
        } else {
            snprintf(buffer, buflen, "%d bytes", size);
        }
        return buffer;
    }

public:
    FileSelectScene() : Scene("Files", 2) {}

    void init(void* arg) {}

    void onDialButtonPress() { pop_scene(); }

    // XXX this should probably be a touch release on the file display
    void onGreenButtonPress() {
        if (fileVector.size()) {
            String dName;
            fileInfo = fileVector[_selected_file];
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
        } else {
            init_file_list();
        }
    }

    void onTouchRelease(int x, int y) { onGreenButtonPress(); }

    void onFilesList() override {
        _selected_file = 0;
        reDisplay();
    }

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

        int fdIter = _selected_file - 1;  // first file in display list

        for (int fx = 0; fx < 3; fx++, fdIter++) {
            int  fi     = box_fi[fx];
            auto middle = box[fi];

#ifdef WRAP_FILE_LIST
            if (fileVector.size() > 2) {
                if (fdIter < 0) {
                    // last file first in list
                    fdIter = fileVector.size() - 1;
                } else if (fdIter > fileVector.size() - 1) {
                    // first file last in list
                    fdIter = 0;
                }
            }
#endif
            if (fdIter < 0) {
                if (yo == 0) {
//                    log_printf("showFiles(): fx:%2d, fdIter:%2d, _selected_file:%2d\r\n", fx, fdIter, _selected_file);
                }
                continue;
            }

            fName = "< no files >";
            if (fileVector.size()) {
                fName = fileVector[fdIter].fileName;
            }
            if (yo == 0 && middle._bg != BLACK) {
                canvas.fillRoundRect(middle._xb, yo + middle._yb, 240, middle._h, middle._h / 2, middle._bg);
            }
            int middle_txt = middle._txt;
            if (fx == 1) {
                String fInfoT = "";  // file info top line
                String fInfoB = "";  // File info bottom line
                int    ext    = fName.lastIndexOf('.');
                float  fs     = 0.0;
                if (fileVector.size()) {
                    switch (fileVector[_selected_file].fileType) {
                        case 0:
                            break;
                        case 1:
                            fInfoB      = "Folder";
                            middle_txt = BLUE;
                            break;
                        case 2:
                            if (ext > 0) {
                                fInfoT = fName.substring(ext, fName.length());
                                fInfoT += " file";
                                fName.remove(ext);
                            }
                            fInfoB = format_size(fileVector[_selected_file].fileSize);
                            break;
                    }
                }
                if (yo == 0) {
                    auto top    = box[fi - 1];
                    auto bottom = box[fi + 1];
                    canvas.fillRoundRect(middle._xb, yo + middle._yb, middle._w, middle._h, middle._h / 2, middle._bg);
                    text(fInfoT, top._xt, yo + top._yt, finfoT_color, top._f, top_center);
                    text(fInfoB, bottom._xt, yo + bottom._yt, bottom._txt, bottom._f, top_center);
                }
            }  // if (fx == 1)

            if ((yo == 0) || (yo < 0 && yo + middle._yt > 45) || (yo > 0 && yo + middle._yt + middle._h < 202)) {
                auto_text(fName, middle._xt, yo + middle._yt, middle._w, (yo) ? WHITE : middle_txt, middle._f, middle_center);
            }
            if (yo == 0) {
                //                log_printf("showFiles(): fx:%2d, fdIter:%2d, _selected_file:%2d - %s\r\n", fx, fdIter, _selected_file, fName.c_str());
            }
            if (fx == 1) {
#ifdef WRAP_FILE_LIST
                if (fileVector.size() > 2) {
                    //                    log_printf("fx:1, size(%d) > 2: continue;\r\n", fileVector.size());
                    continue;
                }
#endif
                if (fdIter >= (int)(fileVector.size() - 1)) {
                    //                    log_printf("fx:1, fdIter >= size(%d): break;\r\n", fileVector.size());
                    break;
                }
            }
        }  // for(fx)
    }

    void scroll(int updown) {
        int nextSelect = _selected_file - updown;
#ifdef WRAP_FILE_LIST
        if (fileVector.size() < 3) {
            if (nextSelect < 0 || nextSelect > (int)(fileVector.size() - 1)) {
                return;
            }
        } else {
            if (nextSelect < 0) {
                nextSelect = fileVector.size() - 1;
            } else if (nextSelect > (int)(fileVector.size() - 1)) {
                nextSelect = 0;
            }
        }
#else
        if (nextSelect < 0 || nextSelect > (int)(fileVector.size() - 1)) {
            return;
        }
#endif

        const int yinc   = updown * 10;
        const int ylimit = 60;
        int       yo     = yinc;

        while (abs(yo) < ylimit) {
            showFiles(yo);
            delay(10);
            yo += yinc;
            refreshDisplay();
        }
        _selected_file = nextSelect;
        //        log_printf("scroll(%d): _selected_file:%2d, files:%2d\r\n", updown, _selected_file, fileVector.size());
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
            switch (fileVector[_selected_file].fileType) {
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

        drawButtonLegends(redText, grnText, "Back");
        refreshDisplay();
    }

    void reDisplay() { displayFileSelect(); }
};
FileSelectScene filesScene;
