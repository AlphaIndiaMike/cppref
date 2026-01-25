/**
 * @file main.cpp
 * @brief Demo application showing Calculator and StringUtils usage
 * @author Documentation Demo
 * @date 2026-01-25
 * 
 * This program demonstrates the usage of Calculator and StringUtils classes.
 * It performs some calculations and string operations, displaying the results.
 */

#include "Calculator.h"
#include "StringUtils.h"
#include <iostream>
#include <iomanip>

/**
 * @brief Demonstrates calculator operations
 * 
 * Performs a series of arithmetic operations using the Calculator class
 * and displays the results.
 */
void demonstrateCalculator() {
    std::cout << "=== Calculator Demo ===" << std::endl;
    
    Calculator calc;
    
    // Chain operations
    calc.add(10).multiply(2).subtract(5).divide(3);
    
    std::cout << "Result of (10 + 0) * 2 - 5 / 3 = " 
              << calc.toString() << std::endl;
    
    // Reset and try another calculation
    calc.reset();
    calc.add(100).divide(4).subtract(10);
    
    std::cout << "Result of 100 / 4 - 10 = " 
              << calc.getResult() << std::endl;
    
    std::cout << std::endl;
}

/**
 * @brief Demonstrates string utility operations
 * 
 * Shows various string manipulation functions from the StringUtils class.
 */
void demonstrateStringUtils() {
    std::cout << "=== StringUtils Demo ===" << std::endl;
    
    std::string text = "  Hello World  ";
    
    std::cout << "Original: '" << text << "'" << std::endl;
    std::cout << "Trimmed:  '" << StringUtils::trim(text) << "'" << std::endl;
    std::cout << "Upper:    '" << StringUtils::toUpper(text) << "'" << std::endl;
    std::cout << "Lower:    '" << StringUtils::toLower(text) << "'" << std::endl;
    std::cout << "Reversed: '" << StringUtils::reverse(StringUtils::trim(text)) << "'" << std::endl;
    
    std::cout << std::endl;
    
    // Split demo
    std::string csv = "apple,banana,cherry,date";
    std::cout << "Splitting: '" << csv << "'" << std::endl;
    auto parts = StringUtils::split(csv, ',');
    for (size_t i = 0; i < parts.size(); ++i) {
        std::cout << "  [" << i << "] = " << parts[i] << std::endl;
    }
    
    std::cout << std::endl;
    
    // Prefix/suffix checks
    std::string filename = "document.pdf";
    std::cout << "Filename: " << filename << std::endl;
    std::cout << "  Starts with 'doc': " 
              << (StringUtils::startsWith(filename, "doc") ? "Yes" : "No") << std::endl;
    std::cout << "  Ends with '.pdf': " 
              << (StringUtils::endsWith(filename, ".pdf") ? "Yes" : "No") << std::endl;
    
    std::cout << std::endl;
}

/**
 * @brief Main entry point
 * @return Exit code (0 for success)
 */
int main() {
    std::cout << std::fixed << std::setprecision(2);
    
    std::cout << "╔════════════════════════════════════════╗" << std::endl;
    std::cout << "║   C++ Documentation Demo Application   ║" << std::endl;
    std::cout << "╚════════════════════════════════════════╝" << std::endl;
    std::cout << std::endl;
    
    demonstrateCalculator();
    demonstrateStringUtils();
    
    std::cout << "Demo completed successfully!" << std::endl;
    
    return 0;
}
