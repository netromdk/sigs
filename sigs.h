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
  class ConnectionBase {
    template <typename>
    friend class Signal;

  public:
    void disconnect() {
      if (deleter) deleter();
    }

  private:
    std::function<void()> deleter;
  };

  using Connection = std::shared_ptr<ConnectionBase>;

  template <typename>
  class Signal;

  template <typename Ret, typename ...Args>
  class Signal<Ret(Args...)> {
    using Slot = std::function<Ret(Args...)>;

    class Entry {
    public:
      Entry(const Slot &slot, Connection conn)
        : slot(slot), conn(conn), signal(nullptr) { }

      Entry(Slot &&slot, Connection conn)
        : slot(std::move(slot)), conn(conn), signal(nullptr) { }

      Entry(Signal *signal, Connection conn)
        : conn(conn), signal(signal) { }

      const Slot &getSlot() const { return slot; }
      Signal *getSignal() const { return signal; }
      Connection getConn() const { return conn; }

    private:
      Slot slot;
      Connection conn;
      Signal *signal;
    };

    using Cont = std::vector<Entry>;
    using Lock = std::lock_guard<std::mutex>;

  public:
    using ReturnType = Ret;
    using SlotType = Slot;

    Signal() { }

    virtual ~Signal() {
      Lock lock(entriesMutex);
      for (auto &entry : entries) {
        auto conn = entry.getConn();
        if (conn) {
          conn->deleter = nullptr;
        }
      }
    }

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

    Connection connect(const Slot &slot) {
      Lock lock(entriesMutex);
      auto conn = makeConnection();
      entries.emplace_back(Entry(slot, conn));
      return conn;
    }

    Connection connect(Slot &&slot) {
      Lock lock(entriesMutex);
      auto conn = makeConnection();
      entries.emplace_back(Entry(std::move(slot), conn));
      return conn;
    }

    /** In the case of connecting a member function with one or more parameters,
        then pass std::placeholders::_1, std::placeholders::_2 etc. */
    template <typename Instance, typename MembFunc, typename ...Plchs>
    Connection connect(Instance *instance, MembFunc Instance::*mf,
                    Plchs &&...plchs) {
      Lock lock(entriesMutex);
      Slot slot = std::bind(mf, instance, std::forward<Plchs>(plchs)...);
      auto conn = makeConnection();
      entries.emplace_back(Entry(slot, conn));
      return conn;
    }

    /// Connecting a signal will trigger all of its slots when this signal is
    /// triggered.
    Connection connect(Signal &signal) {
      Lock lock(entriesMutex);
      auto conn = makeConnection();
      entries.emplace_back(Entry(&signal, conn));
      return conn;
    }

    void clear() {
      Lock lock(entriesMutex);
      entries.clear();
    }

    void disconnect(Connection conn = nullptr) {
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
    Connection makeConnection() {
      auto conn = std::make_shared<ConnectionBase>();
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
