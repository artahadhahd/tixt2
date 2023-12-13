#ifndef APP_H
#define APP_H

#include "common.hpp"
#include "cursor.hpp"

#define ctrl(key) (key & 31)

enum class DirectoryFilterBy : int8_t {
    None = -1,
    Directory = DT_DIR,
    File = DT_REG,
    SymLink = DT_LNK
};


void exit_from_ncurses(std::function<void()>);

struct File {
    std::string name;
    DirectoryFilterBy filter;
    friend bool operator<(const File &lhs, const File &rhs) {
        return lhs.name < rhs.name;
    }
};

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
    [[nodiscard]]
    std::optional<std::vector<File>> getFiles(const char *, DirectoryFilterBy);
    void printFiles(const std::vector<File>);
};

#endif