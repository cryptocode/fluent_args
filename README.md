This is a C++17 experiment to get named parameters in a fluent way. It seems like an idea worth exploring for certain use-cases (while we wait for [meta classes](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p0707r4.pdf) so we can do these things correctly.) This is similar to the Named Parameter Idiom, but applied to functions rather than object construction.

## Ingredients

 * A tiny, optional macro for convenience
 * A function implementation pattern for fluent argument passing
 * C++17 copy elision guarantees
 * [[nodiscard]] combined with -Werror=unused-result so the call operator won't be forgotten.
 
## Example

```C++
#include <fluent.hpp>
#include <iostream>

struct MyThing {
    int simple(int num);
    auto complicatedProcessing() -> auto;
};

int MyThing::simple(int num) 
{ 
    return num * 2; 
}

auto MyThing::complicatedProcessing() -> auto {
    struct action {
        fluent_arg(double, wage);
        fluent_arg(int, id);
        fluent_arg(bool, admin);
    
        // This is where your logic goes. You can add statically required (unnamed) arguments if you wish,
        // and any return type will do.
        int operator ()() {
            // In this implementation, we refer to fluent arguments using an underscore
            std::cout << "Processing: " << _id << std::endl;
    
            // More code...
            return 5;
        }
    };
  
    return action();
}

int main() {
    MyThing thing;
    thing.simple(2);
    int result = thing.complicatedProcessing().id(5).admin(true)();

    // nodiscard ensures it's an error to forget the final call operator (assuming -Werror=unused-result):
    // thing.complicatedProcessing().id(5).admin(true);

    // We get partial application for free:
    auto process = thing.complicatedProcessing().id(6);
    // ...time passes
    process = process.admin(false);
    // ...we're ready to process
    result = process();
}

```
## The pattern

The pattern is simple enough: you create an `action` type to handle argument passing, and you make sure these are all `[[nodiscard]]`. The `fluent_arg` macro helps you get that right. The actual logic sits in the call operator. Since we require `-Werror=unused-result`, you'll get an error if you forget to call it, because all the argument handlers are `[[nodiscard]]`!

As already stated, the macro is just for convience. In fact, it's sometimes desirable to write the argument-passing function yourself to add logging, validation and so on.

## Benefits

* Arguments are named at the call site, especially important for booleans. Great readability.
* Optional arguments by design (which is also a downside bullet point)
* Can hook into an error propagation system, possibly form monadic chains.
* Offers an obvious place to put argument validation.
* Partial application, i.e. add arguments over time as they become available, call function when ready.
* Common actions (parameter sets) can be shared between related functions. Put `action` into a namespace, make the call operator virtual, then specialize in various related functions who share the parameter set by deriving and overloading operator ()

## Drawbacks

* Arguments not in interface, as they're replaced by auto-return. However, IDEs pick up the argument functions for intellisense purposes. Doxygen may or may not be helpful: https://github.com/doxygen/doxygen/issues/5492
* Adds verbosity to the function implementation. May or may not be worth it to get a nice call site/partial application.
* Cannot statically force certain named parameter methods to be called, though unnamed parameters in the call operator is possible. A `maybe` type to propagate errors may be helpful, such as Boost Outcome.
* Macro-induced fields may be hard to rename/change type of through IDE refactorings. Expanding a macro temporarily to refactor isn't hard though, but definitely inconvenient.
