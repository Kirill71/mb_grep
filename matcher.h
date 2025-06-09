#pragma once
#include <regex>
#include <string>

namespace mb {
/**
 * @class IMatcher
 * @brief Abstract base class for matchers.
 *
 * Defines the interface for line matchers such as regex or substring.
 */
class IMatcher {
public:
    /**
     * @brief Check if a line matches the query.
     *
     * @param line The line to check.
     * @return true if the line matches.
     */
    virtual bool match(const std::string& line) const = 0;

    virtual ~IMatcher() = default;
};

/**
 * @class RegexMatcher
 * @brief Regex-based matcher implementation.
 */
class RegexMatcher final : public IMatcher {
public:
    /**
     * @brief Constructs a RegexMatcher.
     *
     * @param query Regex pattern.
     * @param ignore_case If true, performs case-insensitive matching.
     */
    explicit RegexMatcher(const std::string& query, bool ignore_case = false);
    /**
     * @brief Check if a line matches the query.
     *
     * @param line The line to check.
     * @return true if the line matches.
     */
    bool match(const std::string& line) const override;

private:
    std::regex pattern_;
};

/**
 * @class SubstringMatcher
 * @brief Substring-based matcher implementation.
 */
class SubstringMatcher final : public IMatcher {
public:
    /**
     * @brief Constructs a SubstringMatcher.
     *
     * @param query Substring pattern.
     * @param ignore_case If true, performs case-insensitive matching.
     */
    explicit SubstringMatcher(std::string query, bool ignore_case = false);
    /**
     * @brief Check if a line matches the query.
     *
     * @param line The line to check.
     * @return true if the line matches.
     */
    bool match(const std::string& line) const override;

private:
    std::string query_;
    bool ignore_case_{};
};
} // namespace mb