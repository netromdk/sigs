# sigs
Simple thread-safe signal/slot C++11 library, which is templated and include-only. No linking required. Just include the header file "*sigs.h*".

In all its simplicity, the class `sigs::Signal` implements a signal that can be triggered when some event occurs. To receive the signal slots can be connected to it. A slot can be any callable type: lambda, functor, function, or member function. Slots can be disconnected when not needed anymore.

A signal is triggered by invoking its `operator()()` with an optional amount of arguments to be forwarded to each of the connected slots' invocations. But they must conform with the parameter types of `sigs::Signal::SlotType`, which reflects the first template argument given when instantiating a `sigs::Signal`.

# Examples
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

# Ambiguous types
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
