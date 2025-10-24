#pragma once

#include <string_view>

namespace pips {

void download_file(std::string_view url, std::string_view output_path);

}
