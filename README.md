# sigs
Simple thread-safe signal/slot C++11 library, which is templated and include-only. No linking required. Just include the header file "*sigs.h*".

The library consists of the class `sigs::Signal` on which you can connect and disconnect slots to be invoked when the signal is triggered. Upon triggering, any number of optional arguments can be forwarded to the slots as long as they conform with the parameter types of `sigs::Signal::SlotType`.

# Examples
The most simple use case is having a `void()` invoked:

```c++
sigs::Signal<> s;
s.connect([]{ std::cout << "Hello, signals. I'm an invoked slot.\n"; });
s(); // Trigger it, which will call the function.
```

As mentioned above you can pass arbitrary arguments to the slots but the types will be enforced at compile-time.

```c++
sigs::Signal<std::function<void(int, std::string)>> s;
s.connect([](int n, const std::string &str) {
  std::cout << "I received " << n << " and " << str << std::endl;
});

// Prints "I received 42 and I like lambdas!".
s(42, "I like lambdas!");

// Error! "no known conversion from 'char const[5]' to 'int' for 1st argument".
s("hmm?", "I like lambdas!");
```

Slots can be disconnected by the use of tags. The default tag type is `std::string` and can be retrieved by `sigs::Signal::TagType`.

```c++
sigs::Signal<> s;
s.connect([]{ std::cout << "Hi"; });
s.connect([]{ std::cout << " there!\n"; }, "the tag");

// Prints "Hi there!".
s();

// Disconnect second slot by tag name.
s.disconnect("the tag");

// Prints "Hi".
s();
```

Note that all slots can be disconnected by giving no tag. And since `sigs::Signal::disconnect` takes an `std::initializer_list` you can disconnect several at the same time, e.g. `s.disconnect({"tag1", "tag2"})`.

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

sigs::Signal<> s;
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

However, if connecting a member function taking one or more arguments then you have to hint at the number (not the type or value).

```c++
class Foo {
public:
  void test(int i, int j) {
    std::cout << "Called member function: i=" << i << ", j=" << j << std::endl;
  }
};

sigs::Signal<std::function<void(int,int)>> s;

Foo foo;
s.connect(&foo, &Foo::test, "tag", std::placeholders::_1, std::placeholders::_2);

// Prints "Called member function: i=42, j=1105".
s(42, 1105);
```

Note the use of `std::placeholders::_1` and `std::placeholders::_2` because `Foo::test` takes two arguments, and that the tag becomes required.
