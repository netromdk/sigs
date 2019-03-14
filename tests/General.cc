#include <iostream>

#include "gtest/gtest.h"

#include "sigs.h"

TEST(General, instantiate)
{
  sigs::Signal<void()> s;
  sigs::Signal<void(int)> s2;
  sigs::Signal<int()> s3;
}

inline void addOne(int &i)
{
  i++;
}

TEST(General, function)
{
  sigs::Signal<void(int &)> s;
  s.connect(addOne);

  int i = 0;
  s(i);

  EXPECT_EQ(i, 1);
}

TEST(General, multipleFunctions)
{
  sigs::Signal<void(int &)> s;
  s.connect(addOne);
  s.connect(addOne);
  s.connect(addOne);

  int i = 0;
  s(i);

  EXPECT_EQ(i, 3);
}

TEST(General, functor)
{
  class AddOneFunctor {
  public:
    void operator()(int &i)
    {
      i++;
    }
  };

  sigs::Signal<void(int &)> s;
  s.connect(AddOneFunctor());

  int i = 0;
  s(i);

  EXPECT_EQ(i, 1);
}

TEST(General, instanceMethod)
{
  class Foo {
  public:
    void test(int &i)
    {
      i++;
    }
  };

  sigs::Signal<void(int &)> s;

  Foo foo;
  s.connect(&foo, &Foo::test);

  int i = 0;
  s(i);

  EXPECT_EQ(i, 1);
}

TEST(General, lambda)
{
  sigs::Signal<void(int &)> s;
  s.connect([](int &i) { i++; });

  int i = 0;
  s(i);
  EXPECT_EQ(i, 1);
}

TEST(General, connectionDisconnectDirectly)
{
  sigs::Signal<void(int &)> s;
  auto conn = s.connect([](int &i) { i++; });

  int i = 0;
  s(i);
  EXPECT_EQ(i, 1);

  conn->disconnect();

  s(i);
  EXPECT_EQ(i, 1);
}

TEST(General, connectionDisconnectOnSignal)
{
  sigs::Signal<void(int &)> s;
  auto conn = s.connect([](int &i) { i++; });

  int i = 0;
  s(i);
  EXPECT_EQ(i, 1);

  s.disconnect(conn);

  s(i);
  EXPECT_EQ(i, 1);
}

TEST(General, connectSignalToSignal)
{
  sigs::Signal<void(int &)> s1;
  s1.connect([](int &i) { i++; });

  decltype(s1) s2;
  s2.connect(s1);

  int i = 0;
  s2(i);
  EXPECT_EQ(i, 1);
}

TEST(General, disconnectSignalFromSignal)
{
  sigs::Signal<void(int &)> s1;
  s1.connect([](int &i) { i++; });

  decltype(s1) s2;
  s2.connect(s1);
  s2.disconnect(s1);

  int i = 0;
  s2(i);
  EXPECT_EQ(i, 0);
}

TEST(General, ambiguousMembers)
{
  class Ambiguous {
  public:
    void foo(int i)
    {
      value += i;
    }

    void foo(float j)
    {
      value += static_cast<int>(j);
    }

    int value = 0;
  };

  sigs::Signal<void(int)> s;

  Ambiguous amb;
  auto conn = s.connect(&amb, sigs::Use<int>::overloadOf(&Ambiguous::foo));
  s(42);
  EXPECT_EQ(amb.value, 42);
  conn->disconnect();

  // This one only works because int can be coerced into float.
  s.connect(&amb, sigs::Use<float>::overloadOf(&Ambiguous::foo));
  s(1);
  EXPECT_EQ(amb.value, 43);
}

TEST(General, returnValues)
{
  sigs::Signal<int()> s;
  s.connect([] { return 1; });
  s.connect([] { return 2; });
  s.connect([] { return 3; });

  int sum = 0;
  s([&sum](int retVal) { sum += retVal; });

  EXPECT_EQ(sum, 1 + 2 + 3);
}
