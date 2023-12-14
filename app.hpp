#ifndef APP_H
#define APP_H

#include "common.hpp"
#include "cursor.hpp"
#include "manager.hpp"

class NcursesApp {
    struct sigaction SIGWINCH_handler;
    usize cursor_pos = 0;
    Cursor cursor;
public:
    explicit NcursesApp();
    ~NcursesApp();

    int run(const int, char **);
private:
    int renderIndex = 0;
    // [[nodiscard]]
    void printFiles(const std::vector<File>);
};

#endif