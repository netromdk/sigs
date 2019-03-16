// Example of how to use return values of connected slots..

#include "sigs.h"

#include <iostream>

class Calculator {
public:
  void execute()
  {
    std::cout << "Calculating.." << std::endl;

    int sum = 0;
    executeSignal_([&sum](int retVal) {
      std::cout << "Incoming value: " << retVal << std::endl;
      sum += retVal;
    });

    std::cout << "Sum of calculation: " << sum << std::endl;
  }

  [[nodiscard]] auto executeSignal()
  {
    return executeSignal_.interface();
  }

private:
  sigs::Signal<int()> executeSignal_;
};

int main()
{
  Calculator calc;
  auto sig = calc.executeSignal();
  sig->connect([] {
    // Do something and return value..
    return 42;
  });
  sig->connect([] { return 2; });
  calc.execute();

  return 0;
}
