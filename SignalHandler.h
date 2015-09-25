#ifndef SIGNAL_HANDLER_H
#define SIGNAL_HANDLER_H

#include <map>
#include <utility>

template <typename Key, typename Func>
class SignalHandler {
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

#endif // SIGNAL_HANDLER_H
