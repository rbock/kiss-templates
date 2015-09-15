# kiss-templates
Type safe _"Keep it simple, stupid"_ text templates for C++. If you are familiar with the idea of text templates and with C++, you can learn how to use it in just a few minutes.

Branch       | Build Status          |  Build Status
             | clang-3.4 / gcc-4.8   |  VS14 2015
------------ | -------------
Master       | [![Build Status](https://travis-ci.org/rbock/kiss-templates.svg?branch=master)](https://travis-ci.org/rbock/kiss-templates) | [![Build status](https://ci.appveyor.com/api/projects/status/0vhmquucorgyx427/branch/master?svg=true)](https://ci.appveyor.com/project/rbock/kiss-templates/branch/master)


Develop      | [![Build Status](https://travis-ci.org/rbock/kiss-templates.svg?branch=develop)](https://travis-ci.org/rbock/kiss-templates) | [![Build status](https://ci.appveyor.com/api/projects/status/0vhmquucorgyx427/branch/develop?svg=true)](https://ci.appveyor.com/project/rbock/kiss-templates/branch/develop)

## How it works:
Use kiste2cpp to turn text templates into type- and name-safe C++ code. Use this code to generate text from your data and the serializer of choice.

### How templates look like:
Template are a mix of
  - kiss template commands (there are VERY few)
  - C++
  - text

```
%namespace test
%{
  $class Hello

  %auto render() -> void
  %{
    Hello ${data.name}!
  %}

  $endclass
%}
```

### Generating C++ code:
I bet you can guess what this all means (and it is documented below), so let's compile this into a C++ header file:

```
kiste2cpp hello_world.kiste > hello_world.h
```

### Using the generated code:
And now we use it in our C++ project like this

```C++
#include <iostream>
#include <hello_world.h>
#include <kiste/raw.h>

struct Data
{
  std::string name;
};

int main()
{
  const auto data = Data{"World"};
  auto& os = std::cout;
  auto serializer = kiste::raw{os};
  auto hello = test::Hello(data, serializer);

  hello.render();
}
```

### Output:
Compile and run:
```
$ ./examples/0_hello_world/hello_world
    Hello World!
```
Yeah!

## Short Reference:
  - `%<whatever>` C++ code
  - `$class <name>` starts a template class
  - `$class <name> : <base>` starts a template class, which inherits from a base class
  - `member <class> <name>` adds a template as a member
  - `$endclass` ends a template class
  - `${<expression>}` send expression to serializer (which takes care of encoding, quoting, escaping, etc)
  - `$raw{<expression>}` send expression to the ostream directly (no escaping)
  - `$call{<function>}` call a function (do not serialize result)
  - `$|` trim left/right
  - `$$` and `$%` escape `$` and `%` respectively
  - Anything else inside a function of a template class is text

## The kiss template syntax

### C++
Any line that starts with zero or more spaces and a `%` is interpreted as C++.
For example
```
%#include<string>
%namespace
%{
    %auto foo() -> std::string
    %{
      return "bar";
    %}
%}
```
There is really nothing to it, just a `%` at the beginning of the line

### Template classes
All text of the template is located in functions of template classes. Template classes start with `$class <name> [: <base>` and end with `$endclass`.

This is a stand-alone class:
```
$class base
% // Some stuff
$endclass
```
And this is a derived class
```
%#include <base.h>
$class derived : base
% // Some other stuff
$endclass
```
In the generated code, the parent will also be the base of the child. They are linked in such a way that

  - you can access the direct child via a `child` member variable in the parent
  - you can access anything inherited from parent, grandparent, etc via a `parent` member

### Member templates
If you want to reuse some template elements or just want to organize your templates into smaller units and use composition.

A helper class
```
$class Helper
% // Some stuff
$endclass
```

And this is a composite class
```
%#include <Helper.h>
$class composite
$member Helper helper
% // Some other stuff
$endclass
```

In the generated code, the member template will also be a member of the composite. They are linked in such a way that

  - you can access the member via its name in the composite
  - you can access the composite as `child` from the member template


### Serializing data
As you saw in the initial example, the generated template code is initialized with data and a serializer. You can serialize members of that data or in fact any C++ expression by enclosing it in `${}`. For instance

```
%for (const auto& entry : data.entries)
%{
  First name: ${entry.first}
  Last name: ${entry.last}
  Size of names: ${entry.first.size() + entry.last.size()}
%}
```

The serializer takes care of the required escaping, quoting, encoding, etc.

### Raw data
Sometimes you need to actually output some text as is. Then use `$raw{expression}`. It will just pipe whatever you give it to the `ostream` directly.

### Calling functions:
If you want to call a function without serializing the result (e.g. because the function returns `void`), you can enclose the call in `$call{}`.

### Trimming:
  - left-trim of a line: Zero or more spaces/tabs followed by `$|`
  - right-trim of a line (including the trailing return): `$|` at the end of the line

For example:
```
%auto title() -> void
%{
   $| Hello ${data.name}! $|
%}
```
This will get rid of the leading spaces and the trailing return, yielding something like

```
 Hello Mr. Wolf!
```

### Escape sequences:
  - `$$` -> `$`
  - `$%` -> `%`

### Text:
Text is everything else, as long as it is inside a function of a template class.

## Serializer classes:
The interface of a serializer has to have

  - `auto text(const char*) -> void;` This function is called by the kiss templates to serialize their texts.
  - `auto escape(...) -> void;` This function is called with expressions from `${whatever}`. Make it accept whatever you need and like.

Optionally, the serializer might offer

  - `auto raw(...) -> void;` This function is called with expressions from `$raw{whatever}`. Make it accept whatever you need and like.
  - `auto report_exception(long lineNo, const std::string& expression, std::exception_ptr e);` This function gets called if kiste2cpp is called with --report-exceptions. Handle reported exceptions here in any way you seem fit.

## Further education
This is pretty much it.

There are several examples in the `examples` folder. If you have questions, please do not hesitate to open an issue.
