#ifndef SIGS_SIGNAL_SLOT_H
#define SIGS_SIGNAL_SLOT_H

#include <mutex>
#include <memory>
#include <string>
#include <vector>
#include <utility>
#include <functional>
#include <initializer_list>

namespace sigs {
  class Connection {
    template <typename>
    friend class Signal;

  public:
    void disconnect() {
      if (deleter) deleter();
    }

  private:
    std::function<void()> deleter;
  };

  using ConnPtr = std::shared_ptr<Connection>;

  template <typename>
  class Signal;

  template <typename Ret, typename ...Args>
  class Signal<Ret(Args...)> {
    using Slot = std::function<Ret(Args...)>;

    class Entry {
    public:
      Entry(const Slot &slot, ConnPtr conn)
        : slot(slot), conn(conn), signal(nullptr) { }

      Entry(Slot &&slot, ConnPtr conn)
        : slot(std::move(slot)), conn(conn), signal(nullptr) { }

      Entry(Signal *signal, ConnPtr conn)
        : conn(conn), signal(signal) { }

      const Slot &getSlot() const { return slot; }
      Signal *getSignal() const { return signal; }
      ConnPtr getConn() const { return conn; }

    private:
      Slot slot;
      ConnPtr conn;
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

    ConnPtr connect(const Slot &slot) {
      Lock lock(entriesMutex);
      auto conn = makeConnection();
      entries.emplace_back(Entry(slot, conn));
      return conn;
    }

    ConnPtr connect(Slot &&slot) {
      Lock lock(entriesMutex);
      auto conn = makeConnection();
      entries.emplace_back(Entry(std::move(slot), conn));
      return conn;
    }

    /** In the case of connecting a member function with one or more parameters,
        then pass std::placeholders::_1, std::placeholders::_2 etc. */
    template <typename Instance, typename MembFunc, typename ...Plchs>
    ConnPtr connect(Instance *instance, MembFunc Instance::*mf,
                    Plchs &&...plchs) {
      Lock lock(entriesMutex);
      Slot slot = std::bind(mf, instance, std::forward<Plchs>(plchs)...);
      auto conn = makeConnection();
      entries.emplace_back(Entry(slot, conn));
      return conn;
    }

    /// Connecting a signal will trigger all of its slots when this signal is
    /// triggered.
    ConnPtr connect(Signal &signal) {
      Lock lock(entriesMutex);
      auto conn = makeConnection();
      entries.emplace_back(Entry(&signal, conn));
      return conn;
    }

    void clear() {
      Lock lock(entriesMutex);
      entries.clear();
    }

    void disconnect(ConnPtr conn = nullptr) {
      if (!conn) {
        clear();
        return;
      }

      Lock lock(entriesMutex);

      auto end = entries.end();
      for (auto it = entries.begin(); it != end; it++) {
        if (it->getConn() == conn) {
          entries.erase(it);
        }
      }
    }

    void disconnect(Signal &signal) {
      Lock lock(entriesMutex);
      auto end = entries.end();
      for (auto it = entries.begin(); it != end; it++) {
        if (it->getSignal() == &signal) {
          entries.erase(it);
        }
      }
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
    ConnPtr makeConnection() {
      auto conn = std::make_shared<Connection>();
      conn->deleter = [this, conn]{
        this->disconnect(conn);
      };
      return conn;
    }

    Cont entries;
    std::mutex entriesMutex;
  };
}

#endif // SIGS_SIGNAL_SLOT_H
