#pragma once

#include "pips_game.hpp"

#include <nlohmann/json.hpp>

#include <array>
#include <expected>
#include <string_view>

namespace pips {

class NytJsonProvider
{
public:
    enum class Difficulty { EASY, MEDIUM, HARD };

    static std::expected<NytJsonProvider, std::string> create(std::string_view file_path);

    const Game& get_game(Difficulty difficulty) const;

private:
    NytJsonProvider() noexcept = default;

    // The solution is an array of domino placements,
    // where each placement is a pair of coordinate pairs
    using OfficialSolution = std::vector<std::pair<GridCell, GridCell>>;

    std::expected<void, std::string> load_games_from_json();

    static std::expected<Game, std::string>                parse_game(const nlohmann::json& game_json);
    static std::expected<std::vector<Domino>, std::string> parse_dominoes(const nlohmann::json& dominoes_json);
    static std::expected<std::vector<Zone>, std::string>   parse_zones(const nlohmann::json& regions_json);
    static std::expected<OfficialSolution, std::string>    parse_solution(const nlohmann::json& solution_json);

    static RegionType to_region_type(std::string_view region_str);

    nlohmann::json      m_json_data;
    std::array<Game, 3> m_games;
};

}  // namespace pips
