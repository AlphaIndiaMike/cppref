#ifndef MY_PROJECT_STRING_UTILS_H_
#define MY_PROJECT_STRING_UTILS_H_

#include <string>

namespace my_project {

// Converts string to uppercase
std::string ToUpper(const std::string& input);

// Converts string to lowercase
std::string ToLower(const std::string& input);

// Trims whitespace from both ends
std::string Trim(const std::string& input);

// Checks if string is empty or whitespace only
bool IsBlank(const std::string& input);

}  // namespace my_project

#endif  // MY_PROJECT_STRING_UTILS_H_
