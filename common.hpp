#ifndef ACOMMON_H
#define ACOMMON_H
#include <dirent.h>
#include <ncurses.h>
#include <signal.h>
#undef sa_handler
#include <sys/ioctl.h>
#include <cstdint>
#include <functional>
#include <iostream>
#include <optional>
#include <random>
#include <string>
#include <vector>
#include <cstring>


#define DEFAULT_COLOR      -1
#define CURSOR_COLOR_PAIR   1
#define SYMLINK_COLOR_PAIR  DT_LNK
#define DIR_COLOR_PAIR      DT_DIR
#define FILE_COLOR_PAIR     DT_REG

using usize = signed long long int;

int random_number(int s, int e);
// #define RANDOM_COLOR random_number(1, 6)

extern struct TERMINAL {
    usize y, x;
    bool resized = false;
    struct winsize size {};
    /* Returns `true` on failure */
    [[nodiscard]] bool update()
    {
        if (ioctl(0, TIOCGWINSZ, &size) < 0) {
            return true;
        }
        this->x = size.ws_col;
        this->y = size.ws_row;
        return false;
    }
} global_terminal;

#endif