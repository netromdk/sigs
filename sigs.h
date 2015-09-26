#ifndef SIGS_SIGNAL_SLOT_H
#define SIGS_SIGNAL_SLOT_H

#include <tuple>
#include <mutex>
#include <string>
#include <vector>
#include <utility>
#include <functional>

namespace sigs {
  template <typename Slot = std::function<void()>, typename Tag = std::string>
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
    using TagType = Tag;

    void connect(const Slot &slot, const Tag &tag = Tag()) {
      Lock lock(entriesMutex);
      entries.emplace_back(Entry(slot, tag));
    }

    void connect(Slot &&slot, const Tag &tag = Tag()) {
      Lock lock(entriesMutex);
      entries.emplace_back(Entry(std::move(slot), tag));
    }

    /** In the case of connecting a member function with one or more parameters,
        then pass std::placeholders::_1, std::placeholders::_2 etc. But in that
        case a tag is required, too. */
    template <typename Instance, typename MembFunc, typename ...Plchs>
    void connect(Instance *instance, MembFunc mf, const Tag &tag,
                 Plchs &&...plchs) {
      Lock lock(entriesMutex);
      Slot slot = std::bind(mf, instance, std::forward<Plchs>(plchs)...);
      entries.emplace_back(Entry(slot, tag));
    }

    /// Convenience method for void() member functions with no tag.
    template <typename Instance, typename MembFunc>
    void connect(Instance *instance, MembFunc mf) {
      connect(instance, mf, Tag());
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

#endif // SIGS_SIGNAL_SLOT_H
