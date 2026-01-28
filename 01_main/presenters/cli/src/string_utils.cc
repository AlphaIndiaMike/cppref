#include "string_utils.h"

#include <algorithm>
#include <cctype>

namespace my_project {

std::string ToUpper(const std::string& input) {
    std::string result = input;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::toupper(c); });
    return result;
}

std::string ToLower(const std::string& input) {
    std::string result = input;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return result;
}

std::string Trim(const std::string& input) {
    auto start = std::find_if_not(input.begin(), input.end(),
                                   [](unsigned char c) { return std::isspace(c); });
    auto end = std::find_if_not(input.rbegin(), input.rend(),
                                 [](unsigned char c) { return std::isspace(c); }).base();
    
    return (start < end) ? std::string(start, end) : std::string();
}

bool IsBlank(const std::string& input) {
    return std::all_of(input.begin(), input.end(),
                       [](unsigned char c) { return std::isspace(c); });
}

}  // namespace my_project
