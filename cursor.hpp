#ifndef CURSOR_H
#define CURSOR_H
#include "common.hpp"

struct Cursor {
    std::string shape, delete_shape{};
    usize x, y, xmin, xmax, ymin, ymax, bottom;
    WINDOW * win;
    void moveUp(int);
    void moveDown(int);
    void moveLeft();
    void moveRight();
    void render();
    void hideNcursesCursor(bool);
    void setColor(int);
    void setWrap(bool);
    void toggleWrap();
    void delete_previous(int);
    void create_delete_shape();
    void clamp();
private:
    int color = COLOR_WHITE;
    bool wrap = false;
    std::function<void()> on_hit = [] {};
};

#endif