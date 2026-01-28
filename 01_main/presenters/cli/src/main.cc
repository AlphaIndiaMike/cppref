#include <iostream>

#include "calculator.h"
#include "string_utils.h"

int main() {
    my_project::Calculator calc;
    
    std::cout << "Calculator Demo\n";
    std::cout << "2 + 3 = " << calc.Add(2, 3) << "\n";
    std::cout << "10 / 4 = " << calc.Divide(10, 4) << "\n";
    
    std::cout << "\nString Utils Demo\n";
    std::cout << "ToUpper('hello') = " << my_project::ToUpper("hello") << "\n";
    std::cout << "Trim('  world  ') = '" << my_project::Trim("  world  ") << "'\n";
    
    return 0;
}
