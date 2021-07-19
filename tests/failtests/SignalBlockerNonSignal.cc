#include "sigs.h"

class Foo {
};

int main()
{
  Foo foo;
  sigs::SignalBlocker sb(foo);
  return 0;
}
