#include "common.hpp"

TERMINAL global_terminal = {0, 0};
std::vector<std::function<void()>> renderQueue;

int random_number(int s, int e)
{
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> dist(s, e);
    return dist(rng);
}

bool TERMINAL::update()
{
    if (ioctl(0, TIOCGWINSZ, &size) < 0) {
        return true;
    }
    this->x = size.ws_col;
    this->y = size.ws_row;
    return false;
}

void render()
{
    for (auto f : renderQueue) {
        f();
    }
}

// Assume we are still in NCURSES
void exit_from_ncurses(std::function<void()> after)
{
    clear();
    refresh();
    endwin();
    std::cout << "tixt: ";
    after();
    exit(1);
}