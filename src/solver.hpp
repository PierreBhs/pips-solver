#pragma once

#include "pips_game.hpp"

namespace pips {

class Solver
{
public:
    explicit Solver(const Game&);

    void solve();

private:
    const Game& m_game;
};

}  // namespace pips
