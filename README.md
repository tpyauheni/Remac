# Remac

Remac is a simple programming language, written in C++.

It can be compiled to bytecode for future executing.

At this moment Remac is in early development stage, so some features may not work like expected, or not work at all.

## Building

To build this repository, you need to install Python.

Then in command prompt, simply write: `python build.py` or `py build.py` (relatively to operating system). After this repository is built, executable file will automatically run by default.

## Examples

Here is a simple one:
```
Print("Hello, World!")
```

Here is a more compilcated example:
```
a = [10, 942]
b = ReadInteger()

if (b) {
    ShowMessage("You chosen: " + String(b))
} else {
    ShowMessage("Error " + String(a[RandomRange(0, Length(a) - 1)]) + ": " + String(b) + " must be non-zero")
}
```
