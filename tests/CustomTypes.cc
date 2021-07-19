#include <mutex>

#include "gtest/gtest.h"

#include "sigs.h"

class Mutex {
public:
  void lock()
  {
    locks++;
  }

  void unlock()
  {
    unlocks++;
  }

  int locks = 0, unlocks = 0;
};

template <typename Mutex>
class Lock {
public:
  using mutex_type = Mutex;

  explicit Lock(Mutex &m_) : m(m_)
  {
    m.lock();
  }

  ~Lock()
  {
    m.unlock();
    EXPECT_EQ(m.locks, m.unlocks);
    EXPECT_GT(m.locks, 0);
  }

  Mutex &m;
};

using LockMutex = Lock<Mutex>;

template <typename T>
using CustomSignal = sigs::BasicSignal<T, LockMutex>;

TEST(CustomTypes, signal)
{
  CustomSignal<void()> s;
  s.connect([] {});
  s();
}

TEST(CustomTypes, blocker)
{
  CustomSignal<void()> s;
  s.connect([] {});

  sigs::SignalBlocker sb(s);
  s();
}
