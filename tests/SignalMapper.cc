#include "gtest/gtest.h"

#include "sigs.h"

TEST(SignalMapper, instantiate)
{
  sigs::SignalMapper<void()> m1;
  sigs::SignalMapper<void(), void(const int &)> m2;
  sigs::SignalMapper<int(), void(const int &)> m3;
}

TEST(SignalMapper, addSignal)
{
  sigs::SignalMapper<void()> m;

  sigs::Signal<void()> s;
  ASSERT_TRUE(m.add("s", s));
  EXPECT_FALSE(m.add("s", s)); // Already added.
}

TEST(SignalMapper, addSignalType)
{
  sigs::SignalMapper<void()> m;
  ASSERT_TRUE(m.add<void()>("s"));
  EXPECT_FALSE(m.add<void()>("s")); // Already added.
}

TEST(SignalMapper, remove)
{
  sigs::SignalMapper<void()> m;
  ASSERT_TRUE(m.add<void()>("s"));
  EXPECT_TRUE(m.remove("s"));
  EXPECT_FALSE(m.remove("s")); // Already removed.
}

TEST(SignalMapper, size)
{
  sigs::SignalMapper<void()> m;
  EXPECT_EQ(0, m.size());

  EXPECT_TRUE(m.add<void()>("s"));
  EXPECT_EQ(1, m.size());

  m.clear();
  EXPECT_EQ(0, m.size());
}

TEST(SignalMapper, empty)
{
  sigs::SignalMapper<void()> m;
  EXPECT_TRUE(m.empty());

  EXPECT_TRUE(m.add<void()>("s"));
  EXPECT_FALSE(m.empty());

  m.clear();
  EXPECT_TRUE(m.empty());
}

TEST(SignalMapper, unknownSignals)
{
  sigs::SignalMapper<void()> m;
  EXPECT_EQ(nullptr, m.interface<void()>("unknown"));
  EXPECT_EQ(nullptr, m.signal<void()>("unknown"));
}

TEST(SignalMapper, connectAndInvoke)
{
  sigs::SignalMapper<void()> m;
  ASSERT_TRUE(m.add<void()>("s"));

  auto *sig = m.signal<void()>("s");
  ASSERT_NE(nullptr, sig);

  bool val = false;
  sig->connect([&val] { val = true; });

  (*sig)();
  EXPECT_TRUE(val);
}

TEST(SignalMapper, interface)
{
  sigs::SignalMapper<void()> m;
  ASSERT_TRUE(m.add<void()>("s"));

  auto iface = m.interface<void()>("s");
  ASSERT_NE(nullptr, iface);
  bool val = false;
  iface->connect([&val] { val = true; });

  auto *sig = m.signal<void()>("s");
  ASSERT_NE(nullptr, sig);
  (*sig)();
  EXPECT_TRUE(val);
}
