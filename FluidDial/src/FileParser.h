// Copyright (c) 2023 - Mitch Bradley
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#include <Arduino.h>
#include <vector>

typedef void (*callback_t)(void*);

struct fileinfo {
    // String fileSys;
    // String filePath;
    String fileName;
    int    fileType;
    int    fileSize;
};

extern String dirName;
extern int    dirLevel;

extern fileinfo              fileInfo;
extern std::vector<fileinfo> fileVector;

extern void request_file_list();

extern std::vector<String> fileLines;

extern void request_file_preview(const char* name);

extern String current_filename;

void init_listener();
void init_file_list();

void enter_directory(const String& dirname);
void exit_directory();
