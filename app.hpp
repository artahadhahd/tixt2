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
#include <string>
#include <vector>

using usize = std::size_t;

enum class DirectoryFilterBy : int8_t {
    None = -1,
    Directory = DT_DIR,
    File = DT_REG,
    SymLink = DT_LNK
};

struct Cursor {
    std::string shape, delete_shape{};
    usize x, y, xmin, xmax, ymin, ymax;
    WINDOW * win;
    void moveUp();
    void moveDown();
    void moveLeft();
    void moveRight();
    void render();
private:
    void delete_previous(int);
    void create_delete_shape();
};


void exit_from_ncurses(std::function<void()>);

class NcursesApp {
    struct sigaction SIGWINCH_handler;
    usize cursor_pos = 0;
    Cursor cursor;
public:
    NcursesApp();
    ~NcursesApp();

    int run(const int argc, char ** argv);
private:
    [[nodiscard]]
    std::optional<std::vector<std::string>> getFiles(const char *, DirectoryFilterBy);
    void printFiles(const std::vector<std::string>);
};

#endif