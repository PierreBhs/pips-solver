#include "display.hpp"

#include <array>
#include <format>
#include <print>
#include <string_view>
#include <vector>

namespace {

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

std::vector<std::string_view> assign_region_colors(const pips::Game& game)
{
    if (game.zones.empty()) {
        return {};
    }

    std::vector<std::string_view> region_colors(game.zones.size());
    size_t                        color_idx = 0;

    // Assign a unique color to each region in order
    for (size_t i = 0; i < game.zones.size(); ++i) {
        region_colors[i] = REGION_COLORS[color_idx % REGION_COLORS.size()];
        color_idx++;
    }

    return region_colors;
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

    std::vector<std::vector<int>> pips_grid(game.dim.rows, std::vector<int>(game.dim.cols, -1));
    std::vector<std::vector<int>> region_grid(game.dim.rows, std::vector<int>(game.dim.cols, -1));

    for (const auto& placement : solution) {
        pips_grid[placement.placement1.cell.row][placement.placement1.cell.col] = placement.placement1.pip;
        pips_grid[placement.placement2.cell.row][placement.placement2.cell.col] = placement.placement2.pip;
    }

    for (size_t i = 0; i < game.zones.size(); ++i) {
        for (const auto& cell : game.zones[i].indices) {
            region_grid[cell.row][cell.col] = static_cast<int>(i);
        }
    }

    auto region_colors = assign_region_colors(game);

    std::print("{}{}{}", BORDER_COLOR, "\u250F", RESET_COLOR);
    for (size_t c = 0; c < game.dim.cols; ++c)
        std::print("{}{}{}", BORDER_COLOR, "\u2501\u2501\u2501", RESET_COLOR);
    std::println("{}{}{}", BORDER_COLOR, "\u2513", RESET_COLOR);

    for (size_t r = 0; r < game.dim.rows; ++r) {
        std::print("{}{}{}", BORDER_COLOR, "\u2503", RESET_COLOR);
        for (size_t c = 0; c < game.dim.cols; ++c) {
            const int pip = pips_grid[r][c];
            if (pip != -1) {
                const int        region_idx = region_grid[r][c];
                std::string_view color = RESET_COLOR;
                if (region_idx != -1 && !region_colors.empty()) {
                    color = region_colors[static_cast<size_t>(region_idx)];
                }
                std::print("{}{}{:^3}{}", color, DICE_COLOR, pip, RESET_COLOR);
            } else {
                std::print("{}{:^3}{}", HOLE_COLOR, " ", RESET_COLOR);
            }
        }
        std::println("{}{}{}", BORDER_COLOR, "\u2503", RESET_COLOR);
    }

    std::print("{}{}{}", BORDER_COLOR, "\u2517", RESET_COLOR);
    for (size_t c = 0; c < game.dim.cols; ++c)
        std::print("{}{}{}", BORDER_COLOR, "\u2501\u2501\u2501", RESET_COLOR);
    std::println("{}{}{}", BORDER_COLOR, "\u251B", RESET_COLOR);

    if (!region_colors.empty()) {
        for (size_t i = 0; i < game.zones.size(); ++i) {
            std::string_view type_str = to_string(game.zones[i].type);
            std::string target_str = game.zones[i].target ? std::format(" (target: {})", *game.zones[i].target) : "";
            std::println("  {}{:^3}{} : {}{}", region_colors[i], " ", RESET_COLOR, type_str, target_str);
        }
    }
}
