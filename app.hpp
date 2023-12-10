#ifndef APP_H
#define APP_H

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

#define ctrl(key) (key & 31)

using usize = signed long long int;

enum class DirectoryFilterBy : int8_t {
    None = -1,
    Directory = DT_DIR,
    File = DT_REG,
    SymLink = DT_LNK
};

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
private:
    int color = COLOR_WHITE;
    bool wrap = false;
    std::function<void()> on_hit = [] {};
};


void exit_from_ncurses(std::function<void()>);

struct File {
    std::string name;
    DirectoryFilterBy filter;
};

class NcursesApp {
    struct sigaction SIGWINCH_handler;
    usize cursor_pos = 0;
    Cursor cursor;
public:
    NcursesApp();
    ~NcursesApp();

    int run(const int argc, char ** argv);
private:
    int renderIndex = 0;
    [[nodiscard]]
    std::optional<std::vector<File>> getFiles(const char *, DirectoryFilterBy);
    void printFiles(const std::vector<File>);
};

#endif