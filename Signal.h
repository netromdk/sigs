#ifndef SIGS_SIGNAL_H
#define SIGS_SIGNAL_H

#include <mutex>
#include <vector>
#include <utility>

namespace sigs {
  template <typename Slot>
  class Signal {
    using Cont = std::vector<Slot>;
    using Lock = std::lock_guard<std::mutex>;

  public:
    using SlotType = Slot;

    void connect(const Slot &slot) {
      Lock lock(slotsMutex);
      slots.emplace_back(slot);
    }

    void connect(Slot &&slot) {
      Lock lock(slotsMutex);
      slots.emplace_back(std::move(slot));
    }

    void disconnect() {
      Lock lock(slotsMutex);
      slots.clear();
    }

    template <typename ...Args>
    void operator()(Args &&...args) {
      Lock lock(slotsMutex);
      for (auto &slot : slots) {
        slot(std::forward<Args>(args)...);
      }
    }

  private:
    Cont slots;
    std::mutex slotsMutex;
  };
}

#endif // SIGS_SIGNAL_H
