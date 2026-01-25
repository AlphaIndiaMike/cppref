/**
 * @file StringUtils.h
 * @brief Utility functions for string manipulation
 * @author Documentation Demo
 * @date 2026-01-25
 */

#ifndef STRINGUTILS_H
#define STRINGUTILS_H

#include <string>
#include <vector>

/**
 * @class StringUtils
 * @brief Provides utility functions for common string operations
 * 
 * This class contains static methods for string manipulation
 * such as trimming, splitting, and case conversion.
 */
class StringUtils {
public:
    /**
     * @brief Convert string to uppercase
     * @param str The input string
     * @return Uppercase version of the string
     */
    static std::string toUpper(const std::string& str);
    
    /**
     * @brief Convert string to lowercase
     * @param str The input string
     * @return Lowercase version of the string
     */
    static std::string toLower(const std::string& str);
    
    /**
     * @brief Remove leading and trailing whitespace
     * @param str The input string
     * @return Trimmed string
     */
    static std::string trim(const std::string& str);
    
    /**
     * @brief Split a string by delimiter
     * @param str The input string
     * @param delimiter The delimiter character
     * @return Vector of split strings
     */
    static std::vector<std::string> split(const std::string& str, char delimiter);
    
    /**
     * @brief Check if string starts with a prefix
     * @param str The input string
     * @param prefix The prefix to check
     * @return true if string starts with prefix, false otherwise
     */
    static bool startsWith(const std::string& str, const std::string& prefix);
    
    /**
     * @brief Check if string ends with a suffix
     * @param str The input string
     * @param suffix The suffix to check
     * @return true if string ends with suffix, false otherwise
     */
    static bool endsWith(const std::string& str, const std::string& suffix);
    
    /**
     * @brief Reverse a string
     * @param str The input string
     * @return Reversed string
     */
    static std::string reverse(const std::string& str);

private:
    // Prevent instantiation
    StringUtils() = delete;
};

#endif // STRINGUTILS_H
