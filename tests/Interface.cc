#include "gtest/gtest.h"

#include "sigs.h"

TEST(Interface, instantiate)
{
  sigs::Signal<void()> s;
  [[maybe_unused]] const auto interf = s.interface();
}

inline void addOne(int &i)
{
  i++;
}

TEST(Interface, function)
{
  sigs::Signal<void(int &)> s;
  s.interface()->connect(addOne);

  int i = 0;
  s(i);

  EXPECT_EQ(i, 1);
}

TEST(Interface, multipleFunctions)
{
  sigs::Signal<void(int &)> s;
  s.interface()->connect(addOne);
  s.interface()->connect(addOne);
  s.interface()->connect(addOne);

  int i = 0;
  s(i);

  EXPECT_EQ(i, 3);
}

TEST(Interface, functor)
{
  class AddOneFunctor {
  public:
    void operator()(int &i) const
    {
      i++;
    }
  };

  sigs::Signal<void(int &)> s;
  s.interface()->connect(AddOneFunctor());

  int i = 0;
  s(i);

  EXPECT_EQ(i, 1);
}

TEST(Interface, instanceMethod)
{
  class Foo {
  public:
    void test(int &i) const
    {
      i++;
    }
  };

  sigs::Signal<void(int &)> s;

  Foo foo;
  s.interface()->connect(&foo, &Foo::test);

  int i = 0;
  s(i);

  EXPECT_EQ(i, 1);
}

TEST(Interface, lambda)
{
  sigs::Signal<void(int &)> s;
  s.interface()->connect([](auto &i) { i++; });

  int i = 0;
  s(i);
  EXPECT_EQ(i, 1);
}

TEST(Interface, connectionDisconnectOnSignal)
{
  sigs::Signal<void(int &)> s;
  auto conn = s.connect([](int &i) { i++; });

  int i = 0;
  s(i);
  EXPECT_EQ(i, 1);

  s.interface()->disconnect(conn);

  s(i);
  EXPECT_EQ(i, 1);
}

TEST(Interface, disconnectSignalFromSignal)
{
  sigs::Signal<void(int &)> s1;
  s1.connect([](int &i) { i++; });

  decltype(s1) s2;
  s2.connect(s1);
  s2.interface()->disconnect(s1);

  int i = 0;
  s2(i);
  EXPECT_EQ(i, 0);
}
