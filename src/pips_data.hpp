#pragma once

#include <nlohmann/json.hpp>

#include <cstdint>
#include <optional>
#include <string_view>
#include <vector>

namespace pips {

using Domino = std::pair<std::uint8_t, std::uint8_t>;
using Dominoes = std::vector<Domino>;

enum class RegionType { EMPTY, EQUALS, SUM, LESS, GREATER, UNEQUAL };
RegionType to_region_type(std::string_view);

struct Zone
{
    RegionType                                         type;
    std::optional<std::uint8_t>                        target;
    std::vector<std::pair<std::uint8_t, std::uint8_t>> indices;
};

using Zones = std::vector<Zone>;
using BoardDim = std::pair<std::uint8_t, std::uint8_t>;

// The solution is an array of domino placements,
// where each placement is a pair of coordinate pairs
using GridCell = std::pair<std::uint8_t, std::uint8_t>;
using DominoPlacement = std::pair<GridCell, GridCell>;
using Solution = std::vector<DominoPlacement>;

struct Game
{
    Dominoes dominoes;
    Zones    zones;
    Solution solution;
    BoardDim dim;
};

// Holds the 3 pips daily games from NYT
class NytGameHandler
{
public:
    enum class Difficulty { EASY, MEDIUM, HARD };
    explicit NytGameHandler(std::string_view);

    const Dominoes& get_dominoes(Difficulty) const;

    void print() const;

private:
    void from_json();

    [[nodiscard]] constexpr std::size_t      difficulty_to_index(Difficulty) const;
    [[nodiscard]] constexpr std::string_view difficulty_to_string(Difficulty) const;

    template <typename PairType>
    std::vector<PairType> json_to_vector_of_pairs(const nlohmann::json& j);
    Solution              parse_solution(const nlohmann::json& j);

    nlohmann::json           m_json_data{};
    std::array<Game, 3>      m_games{};
};

void download_file(std::string_view url, std::string_view output_path);
// pips::download_file("https://www.nytimes.com/svc/pips/v1/2025-10-25.json", nyt_file);

}  // namespace pips
