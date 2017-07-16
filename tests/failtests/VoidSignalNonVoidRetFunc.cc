#include "sigs.h"

int main()
{
  sigs::Signal<void()> s;
  s.connect([] { /* do nothing */ });

  // Must trigger static assertion because return type is void and not int!
  s([](int retVal) {});

  return 0;
}
