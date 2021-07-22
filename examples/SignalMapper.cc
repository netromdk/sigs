// Example that demonstrates a simplified button using a signal mapper to easily have multiple
// signals.

#include "sigs.h"

#include <iostream>

class Button {
  using ClickFn = void();
  using FocusFn = void();

public:
  Button()
  {
    signals.add<ClickFn>("clicked");
    signals.add<FocusFn>("focused");
  }

  void click()
  {
    signals.invoke<ClickFn>("clicked");
    signals.invoke<FocusFn>("focused");
  }

  void setClickedAction(std::function<ClickFn> fn)
  {
    // Replace any previous handler.
    auto *sig = signals.signal<ClickFn>("clicked");
    sig->clear();
    sig->connect(fn);
  }

  void setFocusedAction(std::function<FocusFn> fn)
  {
    // Replace any previous handler.
    auto *sig = signals.signal<FocusFn>("focused");
    sig->clear();
    sig->connect(fn);
  }

private:
  sigs::SignalMapper<ClickFn /*, FocusFn*/> signals;
};

int main()
{
  Button btn;
  btn.setClickedAction([] { std::cout << "fn: clicked" << std::endl; });
  btn.setFocusedAction([] { std::cout << "fn: focused" << std::endl; });
  btn.click();

  btn.setClickedAction([] { std::cout << "fn2: clicked" << std::endl; });
  btn.setFocusedAction([] { std::cout << "fn2: focused" << std::endl; });
  btn.click();

  return 0;
}
