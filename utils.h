#pragma once
#include <filesystem>

namespace fs = std::filesystem;

namespace mb {
bool is_binary_file(const fs::path& path);

bool contains_regex_chars(const std::string& query);

size_t get_threads_number();
} // namespace mb
