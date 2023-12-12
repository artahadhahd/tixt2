#include "app.hpp"

void onTerminalResize([[maybe_unused]] int)
{
    if (global_terminal.update()) {
        endwin();
        exit_from_ncurses([] {
            std::cerr << "unable to resize terminal properly\n";
            exit_curses(1);
        });
    }
    global_terminal.resized = true;
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
    global_terminal.y = LINES;
    global_terminal.x = COLS;
    cursor.win = stdscr;
    cursor.shape = ">";
    cursor.delete_shape = " ";
    cursor.x = 1;
    cursor.y = 0;
    cursor.ymin = 0;
    cursor.hideNcursesCursor(true);
    cursor.setColor(random_number(1, 6));
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
            if (global_terminal.resized) {
                terminalResizeEvent([this, &dirs] {
                    printFiles(dirs);
                });
            }
            switch (ch) {
            case KEY_ENTER:
                break;
            case ctrl('j'):
            case KEY_DOWN:
                if (cursor.y > global_terminal.y / 2 && cursor.y < (usize)dirs.size() - global_terminal.y / 2) {
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
                cursor.setColor(random_number(0, 6));
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
    if (y < global_terminal.y) {
        auto filter = (int)in[i].filter;
        attron(COLOR_PAIR(filter));
        mvprintw(y, 2, "%s", in.at(i).name.c_str());
        attroff(COLOR_PAIR(filter));
        ++y;
    }}
}

void NcursesApp::terminalResizeEvent(std::function<void()> f)
{
    global_terminal.resized = false;
    f();
}
