#include "app.hpp"

static struct {
    usize y = 0, x = 0;
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
    // hide the cursor
    curs_set(0);
    SIGWINCH_handler.__sigaction_handler.sa_handler = onTerminalResize;
    // Prevents the app from exiting when terminal resizes.
    signal(SIGWINCH, SIGWINCH_handler.__sigaction_handler.sa_handler);
    global_terminal_size.y = LINES;
    global_terminal_size.x = COLS;
    cursor.win = stdscr;
    cursor.shape = ">";
    cursor.delete_shape = " ";
    cursor.x = 1;
    cursor.y = 0;
    cursor.ymin = 0;
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
        const auto files = files_in_directory.value();
        int ch = 0;
        cursor.ymax = files.size();
        do {
            printFiles(files);
            switch (ch) {
            case KEY_DOWN:
                cursor.moveDown();
                break;
            case KEY_UP:
                cursor.moveUp();
                break;
            }
            
            cursor.render();
        } while ((ch = getch()) != KEY_BACKSPACE);
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
        mvprintw(y, 2, "%s", in[y].c_str());
        ++y;
    }
}

// CURSOR

void Cursor::render()
{
    mvwprintw(win, y, x, "%s", shape.c_str());
    refresh();
}

void Cursor::moveUp()
{
    if (y > ymin) {
        --y;
        render();
    }
}

void Cursor::moveDown()
{
    if (y + 1 < ymax && y + 1 < global_terminal_size.y) {
        ++y;
        render();
    }
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

void Cursor::moveLeft() {}
void Cursor::moveRight() {}