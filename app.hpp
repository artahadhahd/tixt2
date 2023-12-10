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

void exit_from_ncurses(std::function<void()>);

class NcursesApp {
    struct sigaction SIGWINCH_handler;
    [[nodiscard]]
    std::optional<std::vector<std::string>> getFiles(const char *, DirectoryFilterBy);
    void printFiles(const std::vector<std::string>);
public:
    NcursesApp();
    ~NcursesApp();

    int run(const int argc, char ** argv);
};

#endif