#ifndef SIGS_SIGNAL_SLOT_H
#define SIGS_SIGNAL_SLOT_H

#include <mutex>
#include <string>
#include <vector>
#include <utility>
#include <functional>
#include <initializer_list>

namespace sigs {
  template <typename>
  class Signal;

  template <typename Ret, typename ...Args>
  class Signal<Ret(Args...)> {
    using Slot = std::function<Ret(Args...)>;

    class Entry {
    public:
      Entry(const Slot &slot) : slot(slot), signal(nullptr) { }
      Entry(Slot &&slot) : slot(std::move(slot)), signal(nullptr) { }
      Entry(Signal *signal) : signal(signal) { }

      const Slot &getSlot() const { return slot; }
      Signal *getSignal() const { return signal; }

    private:
      Slot slot;
      Signal *signal;
    };

    using Cont = std::vector<Entry>;
    using Lock = std::lock_guard<std::mutex>;

  public:
    using ReturnType = Ret;
    using SlotType = Slot;

    Signal() { }

    Signal(const Signal &rhs) {
      Lock lock1(entriesMutex);
      Lock lock2(const_cast<Signal&>(rhs).entriesMutex);
      entries = rhs.entries;
    }

    Signal &operator=(const Signal &rhs) {
      Lock lock1(entriesMutex);
      Lock lock2(const_cast<Signal&>(rhs).entriesMutex);
      entries = rhs.entries;
      return *this;
    }

    Signal(Signal &&rhs) = default;
    Signal &operator=(Signal &&rhs) = default;

    void connect(const Slot &slot) {
      Lock lock(entriesMutex);
      entries.emplace_back(Entry(slot));
    }

    void connect(Slot &&slot) {
      Lock lock(entriesMutex);
      entries.emplace_back(Entry(std::move(slot)));
    }

    /** In the case of connecting a member function with one or more parameters,
        then pass std::placeholders::_1, std::placeholders::_2 etc. */
    template <typename Instance, typename MembFunc, typename ...Plchs>
    void connect(Instance *instance, MembFunc Instance::*mf, Plchs &&...plchs) {
      Lock lock(entriesMutex);
      Slot slot = std::bind(mf, instance, std::forward<Plchs>(plchs)...);
      entries.emplace_back(Entry(slot));
    }

    /// Connecting a signal will trigger all of its slots when this signal is
    /// triggered.
    void connect(Signal &signal) {
      Lock lock(entriesMutex);
      entries.emplace_back(Entry(&signal));
    }

    /*
    void disconnect(const TagList &tags = TagList()) {
      Lock lock(entriesMutex);

      if (tags.size() == 0) {
        entries.clear();
        return;
      }

      for (const auto &tag : tags) {
        auto end = entries.end();
        for (auto it = entries.begin(); it != end; it++) {
          if (it->getTag() == tag) {
            entries.erase(it);
          }
        }
      }
    }
    */

    void operator()(Args &&...args) {
      Lock lock(entriesMutex);
      for (auto &entry : entries) {
        auto *sig = entry.getSignal();
        if (sig) {
          (*sig)(std::forward<Args>(args)...);
        }
        else {
          entry.getSlot()(std::forward<Args>(args)...);
        }
      }
    }

  private:
    Cont entries;
    std::mutex entriesMutex;
  };
}

#endif // SIGS_SIGNAL_SLOT_H
