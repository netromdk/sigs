#ifndef SIGS_SIGNAL_SLOT_H
#define SIGS_SIGNAL_SLOT_H

#include <tuple>
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
    using Tag = std::string;

    class Entry {
    public:
      Entry(const Slot &slot, const Tag &tag)
        : slot(slot), tag(tag), signal(nullptr) { }

      Entry(Slot &&slot, const Tag &tag)
        : slot(std::move(slot)), tag(tag), signal(nullptr) { }

      Entry(Signal *signal, const Tag &tag)
        : signal(signal), tag(tag) { }

      const Slot &getSlot() const { return slot; }
      const Tag &getTag() const { return tag; }
      Signal *getSignal() const { return signal; }

    private:
      Slot slot;
      Tag tag;
      Signal *signal;
    };

    using Cont = std::vector<Entry>;
    using Lock = std::lock_guard<std::mutex>;
    using TagList = std::initializer_list<Tag>;

  public:
    using ReturnType = Ret;
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
    void connect(Instance *instance, MembFunc Instance::*mf, const Tag &tag,
                 Plchs &&...plchs) {
      Lock lock(entriesMutex);
      Slot slot = std::bind(mf, instance, std::forward<Plchs>(plchs)...);
      entries.emplace_back(Entry(slot, tag));
    }

    /// Convenience method for void() member functions with no tag.
    template <typename Instance, typename MembFunc>
    void connect(Instance *instance, MembFunc Instance::*mf) {
      connect(instance, mf, Tag());
    }

    /// Connecting a signal will trigger all of its slots when this signal is
    /// triggered.
    void connect(Signal &signal, const Tag &tag = Tag()) {
      Lock lock(entriesMutex);
      entries.emplace_back(Entry(&signal, tag));
    }

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

    void disconnect(const Tag &tag) {
      disconnect({tag});
    }

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
