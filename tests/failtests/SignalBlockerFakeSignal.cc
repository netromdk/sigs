#include "sigs.h"

class FakeSignal {
public:
  using RetArgs = void();
  using LockType = int;
};

int main()
{
  FakeSignal fake;
  sigs::SignalBlocker sb(fake);
  return 0;
}
