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
    using Entry = std::tuple<Slot, Tag>;
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
        if (getTag(*it) == tag) {
          entries.erase(it);
        }
      }
    }

    template <typename ...Args>
    void operator()(Args &&...args) {
      Lock lock(entriesMutex);
      for (auto &entry : entries) {
        getSlot(entry)(std::forward<Args>(args)...);
      }
    }

  private:
    inline const Slot &getSlot(const Entry &entry) const {
      return std::get<0>(entry);
    }

    inline const Tag &getTag(const Entry &entry) const {
      return std::get<1>(entry);
    }

    Cont entries;
    std::mutex entriesMutex;
  };
}

#endif // SIGS_SIGNAL_H
