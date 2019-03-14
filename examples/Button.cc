// Example that demonstrates a simplified button with specified action-on-click.

#include "sigs.h"

#include <iostream>

class Button {
  using ClickFn = void();

public:
  void click()
  {
    clickSignal();
  }

  void setAction(std::function<ClickFn> fn)
  {
    // Replace any previous handler.
    clickSignal.clear();
    clickSignal.connect(fn);
  }

private:
  sigs::Signal<ClickFn> clickSignal;
};

int main()
{
  Button btn;
  btn.setAction([] { std::cout << "fn: clicked" << std::endl; });
  btn.click();

  btn.setAction([] { std::cout << "fn2: clicked" << std::endl; });
  btn.click();

  return 0;
}
