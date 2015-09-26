#ifndef SIGS_SIGNAL_H
#define SIGS_SIGNAL_H

#include <vector>
#include <utility>

namespace sigs {
  template <typename Slot>
  class Signal {
    using SlotCont = std::vector<Slot>;

  public:
    using SlotType = Slot;

    void connect(const Slot &slot) {
      slots.emplace_back(slot);
    }

    void connect(Slot &&slot) {
      slots.emplace_back(std::move(slot));
    }

    template <typename ...Args>
    void operator()(Args &&...args) {
      for (auto &slot : slots) {
        slot(std::forward<Args>(args)...);
      }
    }

  private:
    SlotCont slots;
  };
}

#endif // SIGS_SIGNAL_H
