#include "display.hpp"

#include <array>
#include <format>
#include <map>
#include <print>
#include <string>
#include <string_view>
#include <vector>

namespace {

// ANSI color codes for terminal display
constexpr std::array<std::string_view, 32> REGION_COLORS = {
    "\033[48;2;255;105;180m", "\033[48;2;255;215;0m",   "\033[48;2;138;43;226m",  "\033[48;2;0;255;255m",
    "\033[48;2;255;0;255m",   "\033[48;2;50;205;50m",   "\033[48;2;255;165;0m",   "\033[48;2;255;69;0m",
    "\033[48;2;75;0;130m",    "\033[48;2;0;250;154m",   "\033[48;2;255;20;147m",  "\033[48;2;0;191;255m",
    "\033[48;2;218;112;214m", "\033[48;2;255;127;80m",  "\033[48;2;127;255;0m",   "\033[48;2;210;105;30m",
    "\033[48;2;173;216;230m", "\033[48;2;240;230;140m", "\033[48;2;147;112;219m", "\033[48;2;0;255;127m",
    "\033[48;2;255;99;71m",   "\033[48;2;64;224;208m",  "\033[48;2;255;140;0m",   "\033[48;2;123;104;238m",
    "\033[48;2;255;192;203m", "\033[48;2;221;160;221m", "\033[48;2;135;206;250m", "\033[48;2;244;164;96m",
    "\033[48;2;152;251;152m", "\033[48;2;255;250;205m", "\033[48;2;255;182;193m", "\033[48;2;255;228;181m",
};

constexpr std::string_view RESET_COLOR = "\033[0m";
constexpr std::string_view HOLE_COLOR = "\033[48;2;40;40;40m";
constexpr std::string_view DICE_COLOR = "\033[38;2;255;255;255m";
constexpr std::string_view BORDER_COLOR = "\033[38;2;150;150;150m";

struct DisplayCell
{
    std::string      content = " ";
    std::string_view fg = RESET_COLOR;
    std::string_view bg = RESET_COLOR;
};

std::string format_time(const std::chrono::duration<double>& duration)
{
    if (duration < std::chrono::microseconds(1)) {
        return std::format("{}ns", std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count());
    }
    if (duration < std::chrono::milliseconds(1)) {
        return std::format("{}us", std::chrono::duration_cast<std::chrono::microseconds>(duration).count());
    }
    if (duration < std::chrono::seconds(1)) {
        return std::format("{}ms", std::chrono::duration_cast<std::chrono::milliseconds>(duration).count());
    }
    if (duration < std::chrono::minutes(1)) {
        return std::format("{:.2f}s", duration.count());
    }
    auto minutes = std::chrono::duration_cast<std::chrono::minutes>(duration);
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration - minutes);
    return std::format("{}m {}s", minutes.count(), seconds.count());
}

std::string_view to_string(pips::RegionType type)
{
    switch (type) {
        case pips::RegionType::EMPTY:
            return "Empty";
        case pips::RegionType::EQUALS:
            return "Equals";
        case pips::RegionType::SUM:
            return "Sum";
        case pips::RegionType::LESS:
            return "Less";
        case pips::RegionType::GREATER:
            return "Greater";
        case pips::RegionType::UNEQUAL:
            return "Unequal";
    }
    return "Unknown";
}

}  // namespace

void pips::print_game_solution(const pips::Game&                         game,
                               const std::vector<pips::DominoPlacement>& solution,
                               const std::chrono::duration<double>&      solver_time,
                               pips::NytJsonProvider::Difficulty         difficulty)
{
    auto difficulty_to_string = [](pips::NytJsonProvider::Difficulty d) {
        switch (d) {
            case pips::NytJsonProvider::Difficulty::EASY:
                return "Easy";
            case pips::NytJsonProvider::Difficulty::MEDIUM:
                return "Medium";
            case pips::NytJsonProvider::Difficulty::HARD:
                return "Hard";
        }
        return "Unknown";
    };

    std::println("\n╔═══════════════════════════════════════════╗");
    std::println("║   GAME: {:^31}   ║", difficulty_to_string(difficulty));
    std::println("╚═══════════════════════════════════════════╝");
    std::println("\nSolver Time: {}", format_time(solver_time));

    // Pre-computation
    std::vector<std::vector<int>> pips_grid(game.dim.rows, std::vector<int>(game.dim.cols, -1));
    std::map<GridCell, int>       domino_id_map;

    for (int i = 0; const auto& p : solution) {
        pips_grid[p.placement1.cell.row][p.placement1.cell.col] = p.placement1.pip;
        pips_grid[p.placement2.cell.row][p.placement2.cell.col] = p.placement2.pip;
        domino_id_map[p.placement1.cell] = i;
        domino_id_map[p.placement2.cell] = i++;
    }

    std::vector<std::string_view>        region_colors(game.zones.size());
    std::map<GridCell, std::string_view> cell_colors;
    for (std::size_t i = 0; const auto& zone : game.zones) {
        region_colors[i] = REGION_COLORS[i % REGION_COLORS.size()];
        for (const auto& cell : zone.indices) {
            cell_colors[cell] = region_colors[i];
        }
        i++;
    }

    // Grid Canvas Construction
    const std::size_t                     canvas_rows = game.dim.rows * 2 + 1;
    const std::size_t                     canvas_cols = game.dim.cols * 4 + 1;
    std::vector<std::vector<DisplayCell>> canvas(canvas_rows, std::vector<DisplayCell>(canvas_cols));

    for (std::uint8_t r = 0; r < game.dim.rows; ++r) {
        for (std::uint8_t c = 0; c < game.dim.cols; ++c) {
            const int pip = pips_grid[r][c];

            // Canvas coordinates for the center of the cell
            const std::size_t canvas_r = r * 2 + 1;
            const std::size_t canvas_c = c * 4 + 2;

            if (pip == -1) {  //  hole
                for (std::size_t i = 0; i < 3; ++i)
                    canvas[canvas_r][canvas_c - 1 + i].bg = HOLE_COLOR;
            } else {  // domino part
                auto color = cell_colors.at({r, c});
                canvas[canvas_r][canvas_c - 1].bg = color;
                canvas[canvas_r][canvas_c].content = std::to_string(pip);
                canvas[canvas_r][canvas_c].fg = DICE_COLOR;
                canvas[canvas_r][canvas_c].bg = color;
                canvas[canvas_r][canvas_c + 1].bg = color;
            }
        }
    }

    // Border and Separator Drawing
    for (std::size_t r = 0; r < canvas_rows; ++r) {
        for (std::size_t c = 0; c < canvas_cols; ++c) {
            const bool is_row_sep = (r % 2 == 0);
            const bool is_col_sep = (c % 4 == 0);

            if (!is_row_sep && !is_col_sep)
                continue;

            canvas[r][c].fg = BORDER_COLOR;

            if (is_row_sep && is_col_sep) {  // Junctions
                canvas[r][c].content = "┼";
            } else if (is_row_sep) {
                canvas[r][c].content = "─";
            } else {
                canvas[r][c].content = "│";
            }
        }
    }

    // Erase internal domino borders
    for (const auto& [cell, id] : domino_id_map) {
        // Horizontal check
        if (domino_id_map.count({cell.row, (std::uint8_t)(cell.col + 1)}) &&
            domino_id_map.at({cell.row, (uint8_t)(cell.col + 1)}) == id) {
            for (std::size_t i = 0; i < 3; ++i)
                canvas[cell.row * 2 + i][cell.col * 4 + 4] = {};
        }
        // Vertical check
        if (domino_id_map.count({(std::uint8_t)(cell.row + 1), cell.col}) &&
            domino_id_map.at({(std::uint8_t)(cell.row + 1), cell.col}) == id) {
            for (std::size_t i = 0; i < 5; ++i)
                canvas[cell.row * 2 + 2][cell.col * 4 + i] = {};
        }
    }

    // Render
    for (std::size_t r = 0; r < canvas_rows; ++r) {
        for (std::size_t c = 0; c < canvas_cols; ++c) {
            const auto& cell = canvas[r][c];
            if (cell.bg != RESET_COLOR)
                std::print("{}", cell.bg);
            if (cell.fg != RESET_COLOR)
                std::print("{}", cell.fg);

            std::print("{}", cell.content);
            if (cell.fg != RESET_COLOR || cell.bg != RESET_COLOR)
                std::print("{}", RESET_COLOR);
        }
        std::println("");
    }

    std::println("");

    for (std::size_t i = 0; i < game.zones.size(); ++i) {
        if (game.zones[i].type == RegionType::EMPTY)
            continue;

        std::string target_str = game.zones[i].target ? std::format(" (target: {})", *game.zones[i].target) : "";
        std::println(
            "  {}{:^3}{} : {}{}", region_colors[i], " ", RESET_COLOR, to_string(game.zones[i].type), target_str);
    }
}
