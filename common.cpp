#include "common.hpp"
int random_number(int s, int e)
{
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> dist(s, e);
    return dist(rng);
}

TERMINAL global_terminal = {0, 0};