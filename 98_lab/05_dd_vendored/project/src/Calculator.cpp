/**
 * @file Calculator.cpp
 * @brief Implementation of the Calculator class
 */

#include "Calculator.h"
#include <sstream>
#include <stdexcept>
#include <iomanip>

Calculator::Calculator() : m_result(0.0) {
}

Calculator::Calculator(double initialValue) : m_result(initialValue) {
}

Calculator::~Calculator() {
}

Calculator& Calculator::add(double value) {
    m_result += value;
    return *this;
}

Calculator& Calculator::subtract(double value) {
    m_result -= value;
    return *this;
}

Calculator& Calculator::multiply(double value) {
    m_result *= value;
    return *this;
}

Calculator& Calculator::divide(double divisor) {
    if (divisor == 0.0) {
        throw std::invalid_argument("Division by zero");
    }
    m_result /= divisor;
    return *this;
}

double Calculator::getResult() const {
    return m_result;
}

void Calculator::reset() {
    m_result = 0.0;
}

std::string Calculator::toString() const {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << m_result;
    return oss.str();
}
