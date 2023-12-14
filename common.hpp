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

enum : std::int8_t {
    DEFAULT_COLOR = -1,
    CURSOR_COLOR_PAIR = 1,
    KEY_RETURN = 13,
    SYMLINK_COLOR_PAIR = DT_LNK,
    DIR_COLOR_PAIR = DT_DIR,
    FILE_COLOR_PAIR = DT_REG
};

constexpr int ctrl(const int key)
{
    return key & 31;
}

using usize = signed long long int;

int random_number(int s, int e);

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