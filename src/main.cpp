#include "pips_data.hpp"

int main()
{
    pips::NytGameHandler nytGames{"data/pips.json"};
    nytGames.print();

    return 0;
}
