#ifndef MY_PROJECT_CALCULATOR_H_
#define MY_PROJECT_CALCULATOR_H_

namespace my_project {

class Calculator {
 public:
    Calculator() = default;
    ~Calculator() = default;

    int Add(int a, int b);
    int Subtract(int a, int b);
    int Multiply(int a, int b);
    double Divide(int a, int b);
};

}  // namespace my_project

#endif  // MY_PROJECT_CALCULATOR_H_
