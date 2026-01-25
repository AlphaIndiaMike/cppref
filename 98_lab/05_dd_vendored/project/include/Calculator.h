/**
 * @file Calculator.h
 * @brief A simple calculator class for basic arithmetic operations
 * @author Documentation Demo
 * @date 2026-01-25
 */

#ifndef CALCULATOR_H
#define CALCULATOR_H

#include <string>

/**
 * @class Calculator
 * @brief Performs basic arithmetic operations
 * 
 * This class provides methods for addition, subtraction,
 * multiplication, and division operations.
 */
class Calculator {
public:
    /**
     * @brief Default constructor
     */
    Calculator();
    
    /**
     * @brief Constructor with initial value
     * @param initialValue The starting value for calculations
     */
    explicit Calculator(double initialValue);
    
    /**
     * @brief Destructor
     */
    ~Calculator();
    
    /**
     * @brief Add a value to the current result
     * @param value The value to add
     * @return Reference to this calculator for chaining
     */
    Calculator& add(double value);
    
    /**
     * @brief Subtract a value from the current result
     * @param value The value to subtract
     * @return Reference to this calculator for chaining
     */
    Calculator& subtract(double value);
    
    /**
     * @brief Multiply the current result by a value
     * @param value The multiplier
     * @return Reference to this calculator for chaining
     */
    Calculator& multiply(double value);
    
    /**
     * @brief Divide the current result by a value
     * @param divisor The divisor (must not be zero)
     * @return Reference to this calculator for chaining
     * @throws std::invalid_argument if divisor is zero
     */
    Calculator& divide(double divisor);
    
    /**
     * @brief Get the current result
     * @return The current calculation result
     */
    double getResult() const;
    
    /**
     * @brief Reset the calculator to zero
     */
    void reset();
    
    /**
     * @brief Get string representation of the result
     * @return String representation of current result
     */
    std::string toString() const;

private:
    double m_result;  ///< Current calculation result
};

#endif // CALCULATOR_H
