#pragma once
#include <filesystem>

namespace fs = std::filesystem;

namespace mb {
/**
 * @brief Determines whether a file is binary.
 *
 * @param path The path to the file to check.
 * @return true If the file appears to be binary.
 * @return false If the file appears to be text.
 */
bool is_binary_file(const fs::path& path);

/**
 * @brief Checks whether a string contains characters typically used in regular expressions.
 *
 * This function scans the input string for common regex metacharacters such as `. ^ $ * + ? { } [ ] \ | ( )`.
 * It's useful for heuristically determining if a pattern is likely intended to be a regex.
 *
 * @param query The input string to check.
 * @return true If the string contains any regex-specific characters.
 * @return false If the string does not appear to contain regex metacharacters.
 */
bool contains_regex_chars(const std::string& query);

/**
 * @brief Determines the number of worker threads to use.
 *
 * This function returns the number of available hardware threads,
 * minus a reserved number (e.g., 2) to leave system resources available
 * for other processes. Ensures at least one thread is used.
 *
 * @return size_t The number of threads to be used by the thread pool.
 */
size_t get_threads_number();
} // namespace mb
