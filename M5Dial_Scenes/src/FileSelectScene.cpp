// Copyright (c) 2023 - Barton Dringstarting
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#include <Arduino.h>
#include "Scene.h"
#include <JsonStreamingParser.h>
#include <JsonListener.h>
#include <vector>

extern Scene statusScene;

extern HardwareSerial Serial_FNC;

enum processmode { starting, fileSelect, fileAction, fileRun, fileShow, noAction, stateChange = -1 };
processmode processMode = fileSelect;
processmode actionMode  = noAction;

String displayTitle = "Files";
String searchType = "ListJSON";
int  selectedFile = -1;

struct fileinfo {
	String fileSys;
	String filePath;
    String fileName;
    int fileType;
    int fileSize;
};
fileinfo fileInfo;
std::vector<fileinfo> fileVector;

int dirLevel = 0;
struct dirinfo {
    String dirName;
    int dirEntries;
    int displayIndex;   // saved here for return from the next level
};
dirinfo dirInfo;    // always in sync with fileVector[dirInfo.dIndex]
std::vector<dirinfo> dirVector;

bool fileinfoCompare(const fileinfo& f1, const fileinfo& f2) {
    if (f1.fileType < f2.fileType)
        return true;
    if (f1.fileType > f2.fileType)
        return false;
    if (f1.fileName.compareTo(f2.fileName) < 0)
        return true;
    return false;    
}

void fileInfo_push() {
    if (fileInfo.fileName.startsWith("/System Volume"))
        return;     // skip this file
    dirInfo.dirEntries++;
    dirVector[dirLevel].dirEntries++;

    fileInfo.fileSys = dirVector[0].dirName; // "$SD/" or "$LocalFS/"
    fileInfo.filePath = (dirLevel) ? dirVector[dirLevel].dirName : "";
    fileInfo.filePath += "/";

    fileVector.push_back(fileInfo);
}

class FileSelectScene : public Scene {
 
  class fileInfoListener: public JsonListener {
    private:
        bool inFileList;
        bool haveNewFile;

    public:
        void whitespace(char c) {
        }

        void startDocument() {
            inFileList = false;
            haveNewFile = false;
        }

        String current_key;

        void key(String key) {
            current_key = key;
            if (current_key == "name") {
                if (haveNewFile)
                    endObject(); // because somehow, it was not already called
                haveNewFile = true; // gets reset in endObject()
            }
        }

        void value(String value) {
            if (inFileList) {
                if (current_key == "name") {
                    fileInfo.fileName = value;
                }
                if (current_key == "size") {
                    fileInfo.fileSize = value.toInt();
                    if (fileInfo.fileSize < 0) {
                        fileInfo.fileType = 1;
                        fileInfo.fileName = String("/" + fileInfo.fileName);
                    }

                    if (fileInfo.fileSize > 0)
                        fileInfo.fileType = 2;
                }
            }
            else { 
                // not inFileList, so process the extra infomation
                if (current_key == "path") {
                    //do something with value
                }
                else if (current_key == "total") {
                    //do something with value
                }
                else if (current_key == "used") {
                    //do something with value
                }
                else if (current_key == "occupation") {
                }
                    //do something with value
                else if (current_key == "something_else") {
                }
                else {
                    USBSerial.printf("unexpected key: %s:%s\r\n", current_key.c_str(), value.c_str());
                }
            }
        }

        void endArray() {
            if (inFileList) {
                inFileList = false;
                std::sort(fileVector.begin(), fileVector.end(), fileinfoCompare);
             }
        }

        void endObject() {
            if (haveNewFile) {
               fileInfo_push();
               haveNewFile = false;
               return;
            }
        }

        void endDocument() {
        }

        void startArray() {
            if (current_key == "files")
                inFileList = true;
        }

        void startObject() {
        }
    };

private:
    int  encoderCounter = 0;
    bool inFileList;
    bool haveNewFile;
    String fncCommand;
 
public:
    FileSelectScene() : Scene("Files") {}

    void init(void *arg) {
        USBSerial.printf("FileSelectSceneV2: init(%d)\r\n", (int)arg);

        dirVector.clear();
        dirLevel = 0;
        if (arg)
            dirInfo.dirName = String(*(String *)arg);
        else
            dirInfo.dirName = "$SD";
        dirInfo.displayIndex = -1;
        dirVector.push_back(dirInfo);
        
        processMode = fileSelect;
        actionMode  = noAction;

        if (state != Idle) {
            processMode = starting;
            displayStateChange("Starting");
        }
        else
            requestJSON();
    }

    int text_in_count;
    int text_line_count = 0;
    const int text_line_max_len = 32;
    const int text_line_max_count = 7;
    String text_lines[7];

    void parse_text( char c) {
        if (text_in_count == 0)
            text_lines[0] = "";
        if (text_line_count < text_line_max_count) {
            if (c == '\r')
                return;
            if (c == '\n') {
                if (text_lines[text_line_count].length())
                    if (++text_line_count < text_line_max_count)
                        text_lines[text_line_count] = "";
                return;
            }
            if (text_lines[text_line_count].length() < text_line_max_len)
                text_lines[text_line_count] += c;
        }
    }

    String parsed_message = "";
    bool message_complete = false;

    void parse_message( char c) {
       if (c == '\r' || c == '\n')
            message_complete = true;
        if (!message_complete || text_in_count < text_line_max_len) {
            parsed_message += c;
        }
        else
            message_complete = true;
    }
//enum processmode { starting, fileSelect, fileAction, fileRun, fileShow, noAction, stateChange = -1 };

    void readFromFNC() {
        const int wait_MS = 200;
        const int timeout_MS = 20;
        USBSerial.printf("readFromFNC( %d, %d): processMode: %d, actionMode: %d",
                         wait_MS, timeout_MS, processMode, actionMode);

        text_in_count = 0;
        // parse_message
        parsed_message = "";
        message_complete = false;
        // parse_text
        text_line_count = 0;

        JsonStreamingParser parser;
        fileInfoListener listener;

        parser.setListener(&listener);
        parser.reset();

        int timeoutstart = millis();
        int timeoutduration = wait_MS;  // normally < 100ms for response to start

        while ((millis() - timeoutstart) < timeoutduration) {
        // wait for a bit to see if more data becomes available
            while(!Serial_FNC.available()) {
                if ((millis() - timeoutstart) > timeoutduration) { // no more timely daya
                    USBSerial.printf("\r\nreadFromFNC():timeout: %dms, processed %d bytes\r\n",
                        millis() - timeoutstart, text_in_count);
                    return;
                }
            }
//            USBSerial.printf("Waited for Serial_FNC: %dms\r\n", millis() - timeoutstart);
            while(Serial_FNC.available()) {
                char c = Serial_FNC.read();
                ++text_in_count;
//                USBSerial.write(c); // debug
                switch ( processMode ) {
                    case fileSelect:
                        parser.parse(c);
                        break;
                    case fileAction:
                        if (actionMode == fileRun) {
                            parse_message(c);
                            // just allow timeout -- waiting for "ok"
                        }
                        if (actionMode == fileShow) {
                            parse_text(c);
                        }
                        break;
                    default:
                        USBSerial.write(c);
                        // ignore until queue is empty
                        break;
                }
            }
//            USBSerial.printf("Serial_FNC.available():failed after %d ms, %d bytes\r\n",
//                    millis() - timeoutstart, text_in_count);
            USBSerial.write('.'); // retry
            timeoutstart = millis();
            timeoutduration = timeout_MS;  // once started, usually < 10ms of delay
        }

    }

    void parseJSON() {
        String path(dirVector[0].dirName + "/" + searchType + "=");
        if (dirLevel)
            path += dirInfo.dirName;
        else
            path += "/";

        USBSerial.println(path.c_str());
//        Serial_FNC.write('?');                // clear any junk from parseGRBL
        Serial_FNC.println(path.c_str());

        readFromFNC();       
    }

    void requestJSON() {

        dirInfo.dirEntries = 0;
        dirVector[dirLevel] = dirInfo;
//        dirVector[dirLevel].dirEntries = dirInfo.dirEntries;

        fileInfo.fileType = 0;
        fileInfo.fileName = "";
        fileInfo.fileSize = 0;
        fileVector.clear();
        
        parseJSON(); // load the new list

        int ix = 0;
        USBSerial.println("Folders");
        for (auto const& vdir : dirVector) {
            USBSerial.printf("[%d] files: %2d, dI: %d:\"%s\"\r\n", ix++,
                vdir.dirEntries, vdir.displayIndex, vdir.dirName.c_str());
        }
        ix = 0;
        USBSerial.println("Files");
        for (auto const& vi : fileVector) {
            USBSerial.printf("[%d] type: %d:%s:\"%s\", size: %d\r\n", ix++,
                vi.fileType, (vi.fileType == 2) ? "file" : "dir ", vi.fileName.c_str(), vi.fileSize);
        }
    }

    void onStateChange(state_t new_state) {
        USBSerial.printf("FileSelectScene(): onStateChange(%d)\r\n", new_state);
        displayStateChange("state changed");
        reDisplay();
    }

    void onDialButtonPress() {
        switch (processMode) {
            case starting:
                pop_scene();
                break;
            case stateChange:
                if (savedMode == fileSelect) {
                    pop_scene();
                    return;
                }
                processMode = savedMode;
                reDisplay();
                break;
            case fileRun:
                activate_scene(&statusScene);
                break;
            case fileShow:
                processMode = fileAction;
                actionMode = fileShow;
                reDisplay();
                break;
            case fileSelect:
                if (dirLevel)
                    onRedButtonPress();
                else
                    pop_scene();
                break;
            case fileAction:
            default:
                processMode = fileSelect;
                reDisplay();
                break;
        }
    }

    void onGreenButtonPress() {
        switch ( processMode ) {
            case fileSelect:
                if (state != Idle) {
                    displayStateChange("Select");
                    return;
                }
                if (selectedFile > -1) {
                    String dName;
                    fileInfo = fileVector[selectedFile];
                    switch (fileInfo.fileType) {
                        case 1: //directory
                            if (dirLevel > 0)
                                dName = dirVector[dirLevel].dirName;
                            dName += fileInfo.fileName;

                            dirVector[dirLevel] = dirInfo;
                            ++dirLevel;
                            dirInfo.dirName = dName;
                            dirInfo.dirEntries = 0;
                            dirInfo.displayIndex = -1;
                            dirVector.push_back(dirInfo);

                            requestJSON();

                            reDisplay();
                            break;
                        case 2: // file
                            processMode = fileAction;
                            actionMode = fileShow;
                            reDisplay();
                            break;
                    }
                }
                break;
            case fileShow:
                if (state != Idle) {
                    displayStateChange("Run");
                    return;
                }
                actionMode = fileRun;
           case fileAction:
                if (state != Idle) {
                    displayStateChange(actionMode == (actionMode == fileRun) ? "Run" : "Show");
                    actionMode = noAction;
                    return;
                }
                if (actionMode == fileRun) {
                    fncCommand = fileInfo.fileSys + "/Run=" + fileInfo.filePath + fileInfo.fileName;
                    USBSerial.println(fncCommand.c_str());
                    Serial_FNC.println(fncCommand.c_str());

                    readFromFNC();

                    if (parsed_message.length())
                        USBSerial.printf("parsed_message: %s\r\n", parsed_message.c_str());       

                    activate_scene(&statusScene);
                    return;
                }
                if (actionMode == fileShow) {
                        fncCommand = fileInfo.fileSys + "/Show=" + fileInfo.filePath + fileInfo.fileName;
                        USBSerial.println(fncCommand.c_str());
                        Serial_FNC.println(fncCommand.c_str());

                        readFromFNC();

                        processMode = fileShow;
                        actionMode = fileShow;       
                        reDisplay();
                    }
                break;
            case stateChange:
                processMode = savedMode;
                reDisplay();
                break;
            case starting:
                pop_scene();
                return;
             default:
                // do nothing
                break;
        }
    }

    void onRedButtonPress() {
       switch (processMode) {
            case fileSelect:
                if (state != Idle) {
                    displayStateChange("Select");
                    return;
                }
                if (dirLevel) {
                    dirLevel--;
                    dirVector.pop_back();
                    dirInfo = dirVector[dirLevel];
        
                    requestJSON();

                    reDisplay();
                }
                break;
            case fileShow:
                processMode = fileAction;
                reDisplay();
                break;
            case fileAction:
                processMode = fileSelect;
                reDisplay();
                break;
        }
    }

    void onTouchRelease(m5::touch_detail_t t) {
        onGreenButtonPress();
    }
   
    void onEncoder(int delta) {
        if (abs(delta) == 0)
            return;
        encoderCounter += delta;
        if (encoderCounter % 4 != 0)
            return;

        if (processMode == fileAction) {
            // a kludge, because only two options
            if (actionMode == fileShow)
                actionMode = fileRun;
            else
                actionMode = fileShow;
            reDisplay();
        }
        if (processMode == fileSelect) {
            if (delta < 0) {
                if (dirInfo.displayIndex > -1) {
                    scroll(1);
                    reDisplay();
                 }
            }
            else {
                if (dirInfo.displayIndex < dirInfo.dirEntries-2) {
                    scroll(-1);
                    reDisplay();
                 }
            }
        }
    }

    void showFiles(int yo) {
       struct {
            int _xb;
            int _xt;
            int _yb; // y for box
            int _yt; // y for text center
            int _w;
            int _h;
            fontnum_t _f;   // text font
            int _bg;        // bg color
            int _txt;       // text color
        } bx[] {
        //    _xb, _xt, _yb, _yt,  _w, _h, _f,      _bg,  _txt
            {   0, 119,  22,  31, 156, 18, TINY,    BLACK, WHITE },        // [0] "SD Files", "localFS", "etc...";
            {  20, 120,  45,  59, 200, 23, SMALL,   BLACK, WHITE },        // [1]file[0] fileName
            {   0, 120,  77,  82, 240, 18, SMALL,   LIGHTGREY, BLUE },     // [2]file[1] Info Line Top
            {   0, 120,  78, 120, 240, 80, MEDIUM,  LIGHTGREY, BLACK },    // [3]file[1] fileName
            {   0, 120, 136, 139, 240, 18, TINY,    LIGHTGREY, BLACK },    // [4]file[1] Info Line Bottom
            {  20, 120, 168, 181, 200, 23, SMALL,   BLACK, WHITE },        // [5]file[2] fileName
        };
        int bx_fi[] = { 1, 3, 5 };
    
        drawBackground(BLACK);
        displayTitle = current_scene->name();
        drawMenuTitle(displayTitle);
        String fName;
        int finfoT_color = BLUE;
        
        if (dirLevel)
            fName = dirInfo.dirName;
        else
            fName = ""; // was dirVector[0].dirName;

        if (fName.startsWith("/"))
            fName.remove(0,1);
                
        auto_text(fName, bx[0]._xt, bx[0]._yt, bx[0]._w, bx[0]._txt, bx[0]._f, middle_center, true, true);
    

        int fdIter = dirInfo.displayIndex;  // first file in display list
        selectedFile = -1;
//        USBSerial.printf("reDisplay(): fdIter:%d, selectedFile: %d, entries:%d\r\n",
//            fdIter, selectedFile, dirInfo.dirEntries);

        for (int fx = 0; fx < 3; fx++, fdIter++) {
            int fi = bx_fi[fx];

            if (fdIter < 0)
                continue;

            fName = "< no files >";
            if (dirInfo.dirEntries > 0)
                fName = fileVector[fdIter].fileName;
//            USBSerial.printf("display(%s): fi[%d], fdIter:%d, selectedFile: %d, entries:%d\r\n",
//                fName.c_str(), fi, fdIter, selectedFile, dirInfo.dirEntries);
            if (yo == 0 && bx[fi]._bg != BLACK)
                canvas.fillRoundRect(bx[fi]._xb, yo+bx[fi]._yb, 240, bx[fi]._h, bx[fi]._h/2, bx[fi]._bg);
 
            if (fName.startsWith("/"))
                fName.remove(0,1);
            if (fx == 1) {
                String fInfoT = "";      // file info top line
                String fInfoB = "";      // File info bottom line
                int ext = fName.lastIndexOf('.');
                float fs = 0.0;
                char buffer[30];
                if (dirInfo.dirEntries) {
                    selectedFile = fdIter;
//                    USBSerial.printf("selectedFile: [%d]: %s\r\n", selectedFile, fName.c_str());
                    if (selectedFile > -1) {
                        switch (fileVector[selectedFile].fileType) {
                            case 0:
                                break;
                            case 1:
                                fInfoT = "Folder";
                                finfoT_color = GREENYELLOW;
                                break;
                            case 2:
                                if (ext > 0) {
                                    fInfoT = fName.substring(ext, fName.length());
                                    fInfoT += " file";
                                    fName.remove(ext);
                                }
                                fs = fileVector[selectedFile].fileSize;
                                if (fs > 999999.9) {
                                    snprintf(buffer, sizeof(buffer),
                                            "%.3f MB", fs / 1000000.0);
                                    fInfoB = buffer;
                                }
                                else if (fs > 999.9) {
                                    snprintf(buffer, sizeof(buffer),
                                            "%.3f KB", fs / 1000.0);
                                    fInfoB = buffer;
                                }
                                else {
                                    snprintf(buffer, sizeof(buffer),
                                            "%.0f bytes", fs);
                                    fInfoB = buffer;
                                }
                                break;
                        }
                    }

                }
                if (yo == 0) {
                    int it = fi - 1;
                    canvas.fillRoundRect(bx[fi]._xb, yo+bx[fi]._yb, bx[fi]._w, bx[fi]._h, bx[fi]._h/2, bx[fi]._bg);
                    text(fInfoT, bx[it]._xt, yo+bx[it]._yt, finfoT_color, bx[it]._f, top_center);
                    int ib = fi + 1;
                    text(fInfoB, bx[ib]._xt, yo+bx[ib]._yt, bx[ib]._txt, bx[ib]._f, top_center);
                }
            }

            if ((yo == 0)
             || (yo < 0 && yo+bx[fi]._yt > 45)
             || (yo > 0 && yo+bx[fi]._yt + bx[fi]._h < 202)) {
                auto_text(fName, bx[fi]._xt, yo+bx[fi]._yt,
                                bx[fi]._w, (yo) ? WHITE : bx[fi]._txt, bx[fi]._f, middle_center);
            }

            if (fdIter >= dirInfo.dirEntries-1)
                break;
        }
    }

    void scroll(int updown) {
        const int yinc = updown * 10;
        const int ylimit = 60;
        int yo = yinc;

        while (abs(yo) < ylimit) {
            showFiles(yo);
            delay(10);
            yo += yinc;
            refreshDisplay();
        }
        dirInfo.displayIndex -= updown;
    }

    processmode savedMode;

    void displayStateChange(const String &msg) {
        USBSerial.printf("displayStateChange(\"%s\") processMode: %d, action: %d, state: %d\r\n",
                          msg.c_str(), processMode, actionMode, state);
        if (processMode != stateChange) {
            savedMode = processMode;
            processMode = stateChange;
        }

        if (state == Idle) {
            processMode = savedMode;
            reDisplay();
            return;
        }

        canvas.createSprite(240, 240);
        drawBackground(BLACK);
        drawMenuTitle(displayTitle);

         drawStatus();

        text(msg, 120, 100, WHITE, SMALL, middle_center);
        text("Must be Idle", 120, 150, WHITE, SMALL, middle_center);

        drawButtonLegends("", "Ok", "Back");
        refreshDisplay();
    }

    void displayFileShow() {
        canvas.createSprite(240, 240);
        drawBackground(BLACK);
        displayTitle = "Show";
        drawMenuTitle(displayTitle);
 
        int y = 36;
        int tl = 0;
        while (tl < text_line_max_count) {
            text(text_lines[tl], 25, y + tl*22, WHITE, TINY, top_left);
            ++tl;
        }
        if (text_line_count == 0)
            text("No Text", 120, 120, WHITE, SMALL, middle_center);

        drawButtonLegends("Cancel", (state == Idle) ? "Run" : "", "Back");
        refreshDisplay();
    }
 
    void displayFileRun() {
        canvas.createSprite(240, 240);
        drawBackground(BLACK);
        displayTitle = "Run";
        drawMenuTitle(displayTitle);

        drawButtonLegends("", "", "Back");
        refreshDisplay();
    }
 
    void displayFileAction() {
        canvas.createSprite(240, 240);
        drawBackground(BLACK);
        displayTitle = "Action";
        drawMenuTitle(displayTitle);

        if (state != Idle)
            drawStatus();

        String  s_n, s_e;
        // break filename into pieces

        s_n = fileInfo.fileName;

        int ix = s_n.lastIndexOf(".");
        if (ix > -1) {
            s_e = s_n.substring(ix, s_n.length()); // extennsion
            s_n.remove(s_n.length()-s_e.length()); // name
        }

//        USBSerial.printf("fileSys:  %s\r\n", fileInfo.fileSys.c_str());
//        USBSerial.printf("filePath: %s\r\n", fileInfo.filePath.c_str());
//        USBSerial.printf("filename: %s\r\n", fileInfo.fileName.c_str());
//        USBSerial.printf("s_n:      %s\r\n", s_n.c_str());
//        USBSerial.printf("s_e:      %s\r\n", s_e.c_str());

        fontnum_t f = MEDIUM;
        bool oneline = false;

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
        }
        else {
            text(s_n.substring(0, s_n.length()/2) + "...", 120,  85, WHITE, f);
            text(s_n.substring(s_n.length()/2, s_n.length()), 120, 115, WHITE, f);
        }
        auto_text(s_e, 120, 136, 240, WHITE, SMALL);
 
        String redText = "Cancel";
        String grnText = "";

        if (actionMode == fileRun)
            if (state == Idle)
                grnText = "Run";
        if (actionMode == fileShow)
            grnText = "Show";

        drawButtonLegends(redText, grnText, "Back");
        refreshDisplay();
    }

    void displayFileSelect() {
        canvas.createSprite(240, 240);
 
        showFiles(0);
    
        String grnText = "";
        String redText = "";

        if (dirLevel)
            redText = "Up...";
        if (dirInfo.dirEntries) {
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

    void reDisplay() {
//        USBSerial.printf("FileSelectScene: reDisplay(): processMode: %d\r\n", processMode);
        switch (processMode) {
            case stateChange:
//                displayStateChange("");
                break;
            case fileSelect:
                displayFileSelect();
                break;
            case fileAction:
                displayFileAction();
                break;
            case fileRun:
                displayFileRun();
                break;
            case fileShow:
                displayFileShow();
                break;
            default:
                break;
        }
    }
};
FileSelectScene filesScene;
