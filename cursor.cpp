#include "cursor.hpp"


void Cursor::render()
{
    wattron(win, COLOR_PAIR(CURSOR_COLOR_PAIR));
    mvwprintw(win, y, x, "%s", shape.c_str());
    wattroff(win, COLOR_PAIR(CURSOR_COLOR_PAIR));
    refresh();
}

void Cursor::moveUp(int amount)
{
    delete_previous(y);
    if (y - amount > ymin - 1) {
        y -= amount;
    } else if (wrap) {
        y = std::min(ymax, global_terminal.y) - 1;
    }
}

int curry = 0;

void Cursor::moveDown(int amount)
{
    delete_previous(y);
    if (y + amount + curry < ymax && y + amount + curry < global_terminal.y) {
        y += amount;
    }
    else {
        if (wrap) {
            y = ymin;
        }
    }
}

void Cursor::moveLeft() {}
void Cursor::moveRight() {}

void Cursor::hideNcursesCursor(bool on)
{
    curs_set(!on);
}

void Cursor::setColor(int color)
{
    init_pair(CURSOR_COLOR_PAIR, color, DEFAULT_COLOR);
    this->color = color;
}

void Cursor::setWrap(bool on)
{
    this->wrap = on;
}

void Cursor::toggleWrap()
{
    this->wrap = !this->wrap;
}

void Cursor::delete_previous(int at)
{
    create_delete_shape();
    mvwprintw(win, at, x, "%s", delete_shape.c_str());
}

void Cursor::create_delete_shape()
{
    if (delete_shape.size() == 0) {
        for ([[maybe_unused]] auto _ : shape) {
            delete_shape += ' ';
        }
    }
}

void Cursor::clamp()
{
    y = std::min(y, global_terminal.y);
}