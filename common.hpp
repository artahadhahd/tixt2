#ifndef ACOMMON_H
#define ACOMMON_H
#include <dirent.h>
#include <ncurses.h>
#include <signal.h>
#undef sa_handler
#include <sys/ioctl.h>
#include <cstdint>
#include <cstring>
#include <functional>
#include <iostream>
#include <optional>
#include <random>
#include <string>
#include <vector>
#include <filesystem>
namespace fs = std::filesystem;


#define DEFAULT_COLOR      -1
#define CURSOR_COLOR_PAIR   1
#define SYMLINK_COLOR_PAIR  DT_LNK
#define DIR_COLOR_PAIR      DT_DIR
#define FILE_COLOR_PAIR     DT_REG
#define KEY_RETURN          13

#define ctrl(key) (key & 31)

using usize = signed long long int;

int random_number(int s, int e);
// #define RANDOM_COLOR random_number(1, 6)

extern struct TERMINAL {
    usize y, x;
    bool resized = true;
    struct winsize size {};
    /* Returns `true` on failure */
    [[nodiscard]] bool update();
} global_terminal;

extern std::vector<std::function<void()>> renderQueue;
void render();
void exit_from_ncurses(std::function<void()>);

#endif