#ifndef SIGS_SIGNAL_H
#define SIGS_SIGNAL_H

#include <map>
#include <utility>

namespace sigs {
  template <typename Key, typename Func>
  class Signal {
    using FuncMap = std::multimap<Key, Func>;

  public:
    using KeyType = Key;
    using FuncType = Func;

    void connect(const Key &key, const Func &func) {
      funcs.emplace(key, func);
    }

    void connect(Key &&key, const Func &func) {
      funcs.emplace(std::move(key), func);
    }

    void connect(const Key &key, Func &&func) {
      funcs.emplace(key, std::move(func));
    }

    void connect(Key &&key, Func &&func) {
      funcs.emplace(std::move(key), std::move(func));
    }

    template <typename ...Args>
    void trigger(const Key &key, Args &&...args) {
      auto range = funcs.equal_range(key);
      for (auto it = range.first; it != range.second; it++) {
        it->second(std::forward<Args>(args)...);
      }
    }

  private:
    FuncMap funcs;
  };
}

#endif // SIGS_SIGNAL_H
