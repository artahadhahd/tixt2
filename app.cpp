#include "app.hpp"

static struct {
    usize y, x;
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
} global_terminal_size = {
    0, 0
};


void onTerminalResize([[maybe_unused]] int)
{
    if (global_terminal_size.update()) {
        endwin();
        exit_from_ncurses([] {
            std::cerr << "unable to resize terminal properly\n";
            exit_curses(1);
        });
    }
    refresh();
}

NcursesApp::NcursesApp()
{
    initscr();
    raw();
    noecho();
    keypad(stdscr, TRUE);
    SIGWINCH_handler.__sigaction_handler.sa_handler = onTerminalResize;
    // Prevents the app from exiting when terminal resizes.
    signal(SIGWINCH, SIGWINCH_handler.__sigaction_handler.sa_handler);
}

NcursesApp::~NcursesApp()
{
    clear();
    refresh();
    endwin();
}

int NcursesApp::run(const int argc, char ** argv)
{
    if (argc > 2) {
        exit_from_ncurses([] { std::cerr << "too many arguments"; });
        return 1;
    }
    
    auto files_in_directory = getFiles(argc == 1 ? "." : argv[1], DirectoryFilterBy::File);
    if (files_in_directory) {
        auto files = files_in_directory.value();
        int ch;
        while ((ch = getch()) != KEY_BACKSPACE) {
            printFiles(files);
        }
    } else {
        exit_from_ncurses([] { std::cerr << "no such resource"; });
        return 1;
    }
    return 0;
}

std::optional<std::vector<std::string>> NcursesApp::getFiles(const char * path, DirectoryFilterBy filter)
{
    DIR * d = opendir(path);
    struct dirent * dir;
    if (!d) {
        return std::nullopt;
    }
    std::vector<std::string> out;
    while ((dir = readdir(d)) != NULL) {
        if (filter == DirectoryFilterBy::None) {
            out.push_back(dir->d_name);
        } else {
            if ((uint8_t)filter == dir->d_type) {
                out.push_back(dir->d_name);
            }
        }
    }
    closedir(d);
    return out;
}

// Assume we are still in NCURSES
void exit_from_ncurses(std::function<void()> after)
{
    clear();
    refresh();
    endwin();
    std::cerr << "tixt: ";
    after();
}

void NcursesApp::printFiles(const std::vector<std::string> in)
{
    clear();
    usize y = 0;
    while (y < in.size() && y < global_terminal_size.y) {
        mvprintw(y, 1, "%s", in[y].c_str());
        ++y;
    }
}
