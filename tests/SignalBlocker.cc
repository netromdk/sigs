#include <iostream>

#include "gtest/gtest.h"

#include "sigs.h"

TEST(SignalBlocker, instantiate)
{
  sigs::Signal<void()> s;
  sigs::SignalBlocker sb(s);
  sigs::SignalBlocker<void()> sb2(s);
  sigs::SignalBlocker sb3(&s);
}

TEST(SignalBlocker, block)
{
  sigs::Signal<void()> s;
  ASSERT_FALSE(s.blocked());

  sigs::SignalBlocker sb(s);
  ASSERT_TRUE(s.blocked());
}

TEST(SignalBlocker, unblock)
{
  sigs::Signal<void()> s;
  ASSERT_FALSE(s.blocked());

  sigs::SignalBlocker sb(s);
  ASSERT_TRUE(s.blocked());

  sb.unblock();
  ASSERT_FALSE(s.blocked());
}

TEST(SignalBlocker, reblock)
{
  sigs::Signal<void()> s;
  ASSERT_FALSE(s.blocked());

  sigs::SignalBlocker sb(s);
  ASSERT_TRUE(s.blocked());

  sb.unblock();
  ASSERT_FALSE(s.blocked());

  sb.reblock();
  ASSERT_TRUE(s.blocked());
}

TEST(SignalBlocker, scopedBlock)
{
  sigs::Signal<void()> s;
  ASSERT_FALSE(s.blocked());

  {
    sigs::SignalBlocker sb(s);
    ASSERT_TRUE(s.blocked());
  }

  ASSERT_FALSE(s.blocked());
}

TEST(SignalBlocker, scopedBlockPointer)
{
  sigs::Signal<void()> s;
  ASSERT_FALSE(s.blocked());

  {
    sigs::SignalBlocker sb(&s);
    ASSERT_TRUE(s.blocked());
  }

  ASSERT_FALSE(s.blocked());
}

/// Unblocking before scope ends must still result in being unblocked after scope ends.
TEST(SignalBlocker, scopedUnblock)
{
  sigs::Signal<void()> s;
  ASSERT_FALSE(s.blocked());

  {
    sigs::SignalBlocker sb(s);
    ASSERT_TRUE(s.blocked());

    sb.unblock();
  }

  ASSERT_FALSE(s.blocked());
}

/// Unblocking and reblocking before scope ends must still result in being unblocked after scope
/// ends.
TEST(SignalBlocker, scopedUnblockReblock)
{
  sigs::Signal<void()> s;
  ASSERT_FALSE(s.blocked());

  {
    sigs::SignalBlocker sb(s);
    ASSERT_TRUE(s.blocked());

    sb.unblock();
    sb.reblock();
  }

  ASSERT_FALSE(s.blocked());
}

TEST(SignalBlocker, scopedBlockPrevious)
{
  sigs::Signal<void()> s;
  ASSERT_FALSE(s.blocked());

  {
    sigs::SignalBlocker sb(s);
    ASSERT_TRUE(s.blocked());

    {
      // Already blocked.
      sigs::SignalBlocker sb2(s);
      ASSERT_TRUE(s.blocked());
    }

    // Must still be blocked at this point due to `sb`.
    ASSERT_TRUE(s.blocked());
  }

  ASSERT_FALSE(s.blocked());
}
