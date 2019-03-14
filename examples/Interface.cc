// Example that demonstrates a simplified button with signal interface that doesn't expose anything
// else than connect and disconnect methods.

#include "sigs.h"

#include <iostream>

class Button {
public:
  void click()
  {
    clickSignal_();
  }

  [[nodiscard]] auto clickSignal()
  {
    return clickSignal_.interface();
  }

private:
  sigs::Signal<void()> clickSignal_;
};

int main()
{
  Button btn;
  btn.clickSignal()->connect([] { std::cout << "direct fn" << std::endl; });
  btn.clickSignal()->connect([] { std::cout << "direct fn 2" << std::endl; });

  auto conn = btn.clickSignal()->connect([] { std::cout << "you won't see me" << std::endl; });
  conn->disconnect();

  btn.click();
  return 0;
}
