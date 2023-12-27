// Copyright (c) 2023 - Mitch Bradley
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#pragma once
#include "Menu.h"

void selectFile(void* arg);

class FileItem : public Item {
private:
public:
    FileItem(const String& name) : Item(name) {}
    void   show(const Point& where) override;
    void   invoke(void* arg = nullptr) override;
    bool   isDirectory() { return name().endsWith("/"); }
    String baseName();
};

class FileMenu : public Menu {
private:
    int    _first;
    String _dirname = "/";

public:
    FileMenu(const char* name) : Menu(name) {}
    void reDisplay() override;
    void rotate(int delta) override;
    int  touchedItem(int x, int y) override;
    void menuBackground() override;
    void onTouchFlick(int x, int y, int dx, int dy) override;
    void setFolder(const String& name) { _dirname = name; }
};
