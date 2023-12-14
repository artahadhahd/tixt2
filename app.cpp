#include "app.hpp"
#include "manager.hpp"

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
    render();
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
    FileManager manager = FileManager {path};
    if (!manager.exists()) {
        exit_from_ncurses([] { std::cerr << "no such resource\n"; });
        return 1;
    }
    manager.get_file();
    return 0;
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
