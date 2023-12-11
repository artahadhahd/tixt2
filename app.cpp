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

// copied from
// https://stackoverflow.com/questions/13445688/how-to-generate-a-random-number-in-c
// seems to work well
static int random_number(int s, int e)
{
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> dist(s, e);
    return dist(rng);
}

#define RANDOM_COLOR random_number(1, 6)

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

static void initColor()
{
    init_pair(FILE_COLOR_PAIR, COLOR_WHITE, DEFAULT_COLOR);
    init_pair(SYMLINK_COLOR_PAIR, COLOR_CYAN, DEFAULT_COLOR);
    init_pair(DIR_COLOR_PAIR, COLOR_BLUE, DEFAULT_COLOR);
}

NcursesApp::NcursesApp()
{
    initscr();
    raw();
    noecho();
    keypad(stdscr, TRUE);
    if (has_colors()) {
        start_color();
        use_default_colors();
        initColor();
    }
    // Without this, ctrl+j would be treated as a new line
    nonl();
    SIGWINCH_handler.__sigaction_handler.sa_handler = onTerminalResize;
    signal(SIGWINCH, SIGWINCH_handler.__sigaction_handler.sa_handler);
    global_terminal_size.y = LINES;
    global_terminal_size.x = COLS;
    cursor.win = stdscr;
    cursor.shape = ">";
    cursor.delete_shape = " ";
    cursor.x = 1;
    cursor.y = 0;
    cursor.ymin = 0;
    cursor.hideNcursesCursor(true);
    cursor.setColor(RANDOM_COLOR);
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
    
    const char * path = argc == 1 ? "." : argv[1];
    auto directories_in_dir = getFiles(path, DirectoryFilterBy::Directory);
    auto files_in_directory = getFiles(path, DirectoryFilterBy::File);
    if (files_in_directory && directories_in_dir) {
        auto dirs = directories_in_dir.value();
        {
            auto files = files_in_directory.value();
            std::sort(files.begin(), files.end());
            std::sort(dirs.begin(), dirs.end());
            dirs.insert(dirs.end(), files.begin(), files.end());
        }
        int ch = 0;
        cursor.ymax = dirs.size();
        cursor.bottom = cursor.ymax - 1;
        printFiles(dirs);
        do {
            switch (ch) {
            case KEY_ENTER:
                break;
            case ctrl('j'):
            case KEY_DOWN:
                if (cursor.y > global_terminal_size.y / 2 && cursor.y < (usize)dirs.size() - global_terminal_size.y / 2) {
                    ++renderIndex;
                    // ++cursor.y;
                    printFiles(dirs);
                } else {
                    cursor.moveDown(1);
                }
                break;
            case ctrl('k'):
            case KEY_UP:
                cursor.moveUp(1);
                break;
            case ctrl('c'):
                cursor.toggleWrap();
                break;
            default:
                cursor.setColor(RANDOM_COLOR);
            }
            cursor.render();
        } while ((ch = getch()) != KEY_BACKSPACE && ch != ctrl('q'));
    } else {
        exit_from_ncurses([] { std::cerr << "no such resource"; });
        return 1;
    }
    return 0;
}

std::optional<std::vector<File>> NcursesApp::getFiles(const char * path, DirectoryFilterBy filter)
{
    DIR * d = opendir(path);
    struct dirent * dir;
    if (!d) {
        return std::nullopt;
    }
    std::vector<File> out;
    while ((dir = readdir(d)) != NULL) {
        if ((uint8_t)filter == dir->d_type && strcmp(dir->d_name, "..") && strcmp(dir->d_name, ".")) {
            out.push_back(
            File {
                .name = dir->d_name,
                .filter = filter
            });
        }
    }
    if (filter == DirectoryFilterBy::Directory) {
        for (auto &e : out) {
            e.name += "/";
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

void NcursesApp::printFiles(const std::vector<File> in)
{
    clear();
    usize y = 0;
    for (size_t i = renderIndex; i < in.size(); ++i) {
    if (y < global_terminal_size.y) {
        auto filter = (int)in[i].filter;
        attron(COLOR_PAIR(filter));
        mvprintw(y, 2, "%s", in.at(i).name.c_str());
        attroff(COLOR_PAIR(filter));
        ++y;
    }}
}

// CURSOR

void Cursor::render()
{
    wattron(win, COLOR_PAIR(CURSOR_COLOR_PAIR));
    mvwprintw(win, y, x, "%s", shape.c_str());
    wattroff(win, COLOR_PAIR(CURSOR_COLOR_PAIR));
    refresh();
}

void Cursor::moveUp(int amount)
{
    delete_previous(y);
    if (y - amount > ymin - 1) {
        y -= amount;
    } else if (wrap) {
        y = std::min(ymax, global_terminal_size.y) - 1;
    }
    // render();
}

void Cursor::moveDown(int amount)
{
    delete_previous(y);
    if (y + amount < ymax && y + amount < global_terminal_size.y) {
        y += amount;
    } 
    else {
        if (wrap) {
            y = ymin;
        }
    }
}

void Cursor::moveLeft() {}
void Cursor::moveRight() {}

void Cursor::hideNcursesCursor(bool on)
{
    curs_set(!on);
}

void Cursor::setColor(int color)
{
    init_pair(CURSOR_COLOR_PAIR, color, DEFAULT_COLOR);
    this->color = color;
}

void Cursor::setWrap(bool on)
{
    this->wrap = on;
}

void Cursor::toggleWrap()
{
    this->wrap = !this->wrap;
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