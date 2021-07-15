#include <iostream>

#include "gtest/gtest.h"

#include "sigs.h"

TEST(SignalBlocker, instantiate)
{
  sigs::Signal<void()> s;
  sigs::SignalBlocker<void()> sb1(s);
  sigs::SignalBlocker<void()> sb2(&s);
}

TEST(SignalBlocker, block)
{
  sigs::Signal<void()> s;
  ASSERT_FALSE(s.blocked());

  sigs::SignalBlocker<void()> sb(s);
  ASSERT_TRUE(s.blocked());
}

TEST(SignalBlocker, unblock)
{
  sigs::Signal<void()> s;
  ASSERT_FALSE(s.blocked());

  sigs::SignalBlocker<void()> sb(s);
  ASSERT_TRUE(s.blocked());

  sb.unblock();
  ASSERT_FALSE(s.blocked());
}

TEST(SignalBlocker, reblock)
{
  sigs::Signal<void()> s;
  ASSERT_FALSE(s.blocked());

  sigs::SignalBlocker<void()> sb(s);
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
    sigs::SignalBlocker<void()> sb(s);
    ASSERT_TRUE(s.blocked());
  }

  ASSERT_FALSE(s.blocked());
}

TEST(SignalBlocker, scopedBlockPointer)
{
  sigs::Signal<void()> s;
  ASSERT_FALSE(s.blocked());

  {
    sigs::SignalBlocker<void()> sb(&s);
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
    sigs::SignalBlocker<void()> sb(s);
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
    sigs::SignalBlocker<void()> sb(s);
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
    sigs::SignalBlocker<void()> sb(s);
    ASSERT_TRUE(s.blocked());

    {
      // Already blocked.
      sigs::SignalBlocker<void()> sb2(s);
      ASSERT_TRUE(s.blocked());
    }

    // Must still be blocked at this point due to `sb`.
    ASSERT_TRUE(s.blocked());
  }

  ASSERT_FALSE(s.blocked());
}

TEST(SignalBlocker, moveConstructible)
{
  sigs::Signal<void()> s;

  sigs::SignalBlocker<void()> sb(s);
  ASSERT_TRUE(s.blocked());

  sigs::SignalBlocker<void()> sb2(std::move(sb));
  ASSERT_TRUE(s.blocked());
}

TEST(SignalBlocker, moveAssignable)
{
  sigs::Signal<void()> s, s2;

  sigs::SignalBlocker<void()> sb(s);
  ASSERT_TRUE(s.blocked());

  sigs::SignalBlocker<void()> sb2(s2);
  ASSERT_TRUE(s2.blocked());

  sb2 = std::move(sb);
  ASSERT_TRUE(s.blocked());

  // `s2` is unblocked when `sb` was moved to `sb2` since they block different signals.
  ASSERT_FALSE(s2.blocked());
}

TEST(SignalBlocker, moveAssignableSameSignal)
{
  sigs::Signal<void()> s;

  sigs::SignalBlocker<void()> sb(s);
  ASSERT_TRUE(s.blocked());

  sigs::SignalBlocker<void()> sb2(s);
  ASSERT_TRUE(s.blocked());

  sb2 = std::move(sb);

  // Stays blocked because `sb` and `sb2` were blocking the same signal.
  ASSERT_TRUE(s.blocked());
}
