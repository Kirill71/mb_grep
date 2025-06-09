#include "matcher.h"

#include <algorithm>

namespace mb {
RegexMatcher::RegexMatcher(const std::string& query, const bool ignore_case) {
    std::regex::flag_type flags = std::regex::ECMAScript;
    if (ignore_case) {
        flags |= std::regex::icase;
    }
    pattern_ = std::regex(query, flags);
}

bool RegexMatcher::match(const std::string& line) const {
    return std::regex_search(line, pattern_);
}

SubstringMatcher::SubstringMatcher(std::string query, const bool ignore_case)
    : query_(std::move(query)), ignore_case_(ignore_case) {
    if (ignore_case_) {
        std::ranges::transform(query_, query_.begin(), tolower);
    }
}

bool SubstringMatcher::match(const std::string& line) const {
    if (ignore_case_) {
        std::string lower_line = line;
        std::ranges::transform(lower_line, lower_line.begin(), tolower);
        return lower_line.find(query_) != std::string::npos;
    }
    return line.find(query_) != std::string::npos;
}

} // namespace mb
