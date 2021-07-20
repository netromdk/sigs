[![Test](https://github.com/netromdk/sigs/workflows/Test/badge.svg?branch=master)](https://github.com/netromdk/sigs/actions)
[![Clang Sanitizers](https://github.com/netromdk/sigs/workflows/Clang%20Sanitizers/badge.svg?branch=master)](https://github.com/netromdk/sigs/actions)
[![CodeQL](https://github.com/netromdk/sigs/workflows/CodeQL/badge.svg?branch=master)](https://github.com/netromdk/sigs/security/code-scanning)

# sigs
Simple thread-safe signal/slot C++17 library, which is templated and include-only. No linking required. Just include the header file "*sigs.h*".

In all its simplicity, the class `sigs::Signal` implements a signal that can be triggered when some event occurs. To receive the signal slots can be connected to it. A slot can be any callable type: lambda, functor, function, or member function. Slots can be disconnected when not needed anymore.

A signal is triggered by invoking its `operator()()` with an optional amount of arguments to be forwarded to each of the connected slots' invocations. But they must conform with the parameter types of `sigs::Signal::SlotType`, which reflects the first template argument given when instantiating a `sigs::Signal`.

Table of contents
=================

* [Examples](#examples)
* [Ambiguous types](#ambiguous-types)
* [Return values](#return-values)
* [Signal interface](#signal-interface)
* [Blocking signals and slots](#blocking-signals-and-slots)
* [Customizing lock and mutex types](#customizing-lock-and-mutex-types)

Examples
========
The most simple use case is having a `void()` invoked:

```c++
sigs::Signal<void()> s;
s.connect([]{ std::cout << "Hello, signals. I'm an invoked slot.\n"; });
s(); // Trigger it, which will call the function.
```

As mentioned above you can pass arbitrary arguments to the slots but the types will be enforced at compile-time.

```c++
sigs::Signal<void(int, const std::string&)> s;
s.connect([](int n, const std::string &str) {
  std::cout << "I received " << n << " and " << str << std::endl;
});

// Prints "I received 42 and I like lambdas!".
s(42, "I like lambdas!");

// Error! "no known conversion from 'char const[5]' to 'int' for 1st argument".
s("hmm?", "I like lambdas!");
```

When connecting a slot the result is a `sigs::Connection`, and the connection can be disconnected by calling `sigs::Connection::disconnect()` or `sigs::Signal::disconnect(sigs::Connection)`.

```c++
sigs::Signal<void()> s;
s.connect([]{ std::cout << "Hi"; });
auto conn = s.connect([]{ std::cout << " there!\n"; });

// Prints "Hi there!".
s();

// Disconnect second slot.
conn->disconnect();

// Or by using the signal: s.disconnect(conn);

// Prints "Hi".
s();
```

Note that all slots can be disconnected by giving no arguments to `sigs::Signal::disconnect()`, or by calling `sigs::Signal::clear()`.

Slots can be any callable type: lambda, functor, or function. Even member functions.

```c++
void func() {
  std::cout << "Called function\n";
}

class Functor {
public:
  void operator()() {
    std::cout << "Called functor\n";
  }
};

class Foo {
public:
  void test() {
    std::cout << "Called member fuction\n";
  }
};

sigs::Signal<void()> s;
s.connect(func);
s.connect([]{ std::cout << "Called lambda\n"; });
s.connect(Functor());

Foo foo;
s.connect(&foo, &Foo::test);

s();

/* Prints:
Called function
Called lambda
Called functor
Called member funtion
*/
```

Another useful feature is the ability to connect signals to signals. If a first signal is connected to a second signal, and the second signal is triggered, then all of the slots of the first signal are triggered as well - and with the same arguments.

```c++
sigs::Signal<void()> s1;
s1.connect([]{ std::cout << "Hello 1 from s1\n"; });
s1.connect([]{ std::cout << "Hello 2 from s1\n"; });

decltype(s1) s2;
s2.connect(s1);

s2();

/* Prints:
Hello 1 from s1
Hello 2 from s1
*/
```

A signal can be disconnected by using `sigs::Signal::disconnect(sigs::Signal&)`, or the regular `sigs::Connection::disconnect()`.

Ambiguous types
===============
Sometimes there are several overloads for a given function and then it's not enough to just specify `&Class::functionName` because the compiler does not know which overload to choose.

Consider the following code:

```c++
class Ambiguous {
public:
  void foo(int i, int j) { std::cout << "Ambiguous::foo(int, int)\n"; }

  void foo(int i, float j) { std::cout << "Ambiguous::foo(int, float)\n"; }
};

sigs::Signal<void(int, int)> s;

Ambiguous amb;
s.connect(&amb, &Ambiguous::foo); // <-- Will fail!
```

Instead we must use the `sigs::Use<>::overloadOf()` construct:

```c++
s.connect(&amb, sigs::Use<int, int>::overloadOf(&Ambiguous::foo));
s(42, 48);

/* Prints:
Ambiguous::foo(int, int)
*/
```

Without changing the signal we can also connect the second overload `foo(int, float)`:

```c++
// This one only works because int can be coerced into float.
s.connect(&amb, sigs::Use<int, float>::overloadOf(&Ambiguous::foo));
s(12, 34);

/* Prints:
Ambiguous::foo(int, int)
Ambiguous::foo(int, float)
*/
```

Return values
=============
If slots have return values they can be gathered by triggering the signal with a function. But the argument type must be the same as the return type!

The following example adds together the integers from each connected slot:
```c++
sigs::Signal<int()> s;
s.connect([] { return 1; });
s.connect([] { return 2; });
s.connect([] { return 3; });

int sum = 0;
s([&sum](int retVal) { sum += retVal; });
// sum is now = 1 + 2 + 3 = 6
```

Signal interface
================
When a signal is used in an abstraction one most often doesn't want it exposed directly as a public member since it destroys encapsulation. `sigs::Signal::interface()` can be used instead to only expose connect and disconnect methods of the signal - it is a `std::unique_ptr<sigs::Signal::Interface>` wrapper instance.

The example shows a button abstraction where actions can easily be added or removed while preserving the encapsulation of the signal:
```c++
class Button {
public:
  void click()
  {
    clickSignal_();
  }

  [[nodiscard]] auto clickSignal()
  {
    return clickSignal_.interface();
  }

private:
  sigs::Signal<void()> clickSignal_;
};

int main()
{
  Button btn;
  btn.clickSignal()->connect([] { std::cout << "direct fn" << std::endl; });
  btn.clickSignal()->connect([] { std::cout << "direct fn 2" << std::endl; });

  auto conn = btn.clickSignal()->connect([] { std::cout << "you won't see me" << std::endl; });
  conn->disconnect();

  btn.click();
  return 0;
}
```

Blocking signals and slots
==========================
Sometimes it is necessary to block a signal, and any recursive signals, from triggering. That is achieved through `sigs::Signal::setBlocked(bool)` and `sigs::Signal::blocked()`:
```c++
sigs::Signal<void()> s;
s.connect([] { /* .. */ });
s.connect([] { /* .. */ });
s.setBlocked(true);

// No slots will be triggered since the signal is blocked.
s();
```

To make things simpler, the `sigs::SignalBlocker` class utilizes the RAII idiom to block/unblock via its own scoped lifetime:
```c++
sigs::Signal<void()> s;
s.connect([] { /* .. */ });
s.connect([] { /* .. */ });

{
  sigs::SignalBlocker<void()> blocker(s);

  // No slots will be triggered since the signal is blocked.
  s();
}

// All connected slots are triggered since the signal is no longer blocked.
s();
```

Customizing lock and mutex types
================================

The default signal type `sigs::Signal<T>` is actually short for `sigs::BasicSignal<T, sigs::BasicLock>` (with `sigs::BasicLock = std::lock_guard<std::mutex>`). Thus the lock type is `std::lock_guard` and the mutex type is `std::mutex`.

Custom lock and mutex types can be supplied by defining a new type, for instance:
```c++
template <typename T>
using MySignal = sigs::BasicSignal<T, MyLock>;
```

The required lock and mutex interfaces are as follows:
```c++
class Mutex {
public:
  void lock();
  void unlock();
};

template <typename Mutex>
class Lock {
public:
  using mutex_type = Mutex;

  explicit Lock(Mutex &);
};
```

The lock type is supposed to lock/unlock following the RAII idiom.
