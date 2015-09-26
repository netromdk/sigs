#ifndef SIGS_SIGNAL_H
#define SIGS_SIGNAL_H

#include <tuple>
#include <mutex>
#include <string>
#include <vector>
#include <utility>

namespace sigs {
  template <typename Slot, typename Tag = std::string>
  class Signal {
    class Entry {
    public:
      Entry(const Slot &slot, const Tag &tag) : slot(slot), tag(tag) { }
      Entry(Slot &&slot, const Tag &tag) : slot(std::move(slot)), tag(tag) { }

      const Slot &getSlot() const { return slot; }
      const Tag &getTag() const { return tag; }

    private:
      Slot slot;
      Tag tag;
    };

    using Cont = std::vector<Entry>;
    using Lock = std::lock_guard<std::mutex>;

  public:
    using SlotType = Slot;

    void connect(const Slot &slot, const Tag &tag = Tag()) {
      Lock lock(entriesMutex);
      entries.emplace_back(Entry(slot, tag));
    }

    void connect(Slot &&slot, const Tag &tag = Tag()) {
      Lock lock(entriesMutex);
      entries.emplace_back(Entry(std::move(slot), tag));
    }

    void disconnect(const Tag &tag = Tag()) {
      Lock lock(entriesMutex);

      if (tag == Tag()) {
        entries.clear();
        return;
      }

      auto end = entries.end();
      for (auto it = entries.begin(); it != end; it++) {
        if (it->getTag() == tag) {
          entries.erase(it);
        }
      }
    }

    template <typename ...Args>
    void operator()(Args &&...args) {
      Lock lock(entriesMutex);
      for (auto &entry : entries) {
        entry.getSlot()(std::forward<Args>(args)...);
      }
    }

  private:
    Cont entries;
    std::mutex entriesMutex;
  };
}

#endif // SIGS_SIGNAL_H
