#include "sigs.h"

int main()
{
  sigs::SignalMapper<void(int)> m;

  // Requires void(int) instead of void().
  m.add<void()>("sig");

  return 0;
}
