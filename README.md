# Bay-Waves

Bay-Waves is a simple, plain-English programming language written in C.

Bay is designed to make programming code readable and approachable for beginners and kids, while still being a powerful compiled and interpreted language.

## Example

```bay
variable name equals "World".
variable age equals 15.

say "Hello, \name!".
say "You are \age years old".
```

Output:

```text
Hello, World!
You are 15 years old
```

## Installing Bay

Bay currently runs on Linux.

To install Bay:

```bash
git clone https://github.com/lightbowlt/Bay-Waves.git
cd Bay-Waves/Bay-Waves
make
make install
```

### Windows

Windows support is currently provided through Windows Subsystem for Linux (WSL).

Install WSL, open a Linux terminal, and run the same commands as above.

## Starting Bay

Run a Bay program using the interpreter:

```bash
bay run hello.bay
```

Compile a Bay program to native code:

```bash
bay compile hello.bay
```

You can also specify a compilation target:

```bash
bay compile hello.bay --target linux-x64
bay compile hello.bay --target windows-x64
bay compile hello.bay --target macos-arm64
```

## Bay v0.2

Bay currently supports:

- Variables
- Integers
- Arithmetic
- Strings
- String interpolation
- Comments
- User input
- Booleans
- Comparisons
- `and`, `or`, `not`
- `if` / `otherwise`
- `repeat`
- `while`
- Functions
- Function parameters
- Return values
- Function-local scope

## Variables and Arithmetic

```bay
variable x equals 10.
variable y equals 5.

variable result equals x + y * 2.

say the result end.
```

Output:

```text
20
```

## Conditions

```bay
variable age equals 16.

if age is greater than or equal to 18 then
    say "Adult".
otherwise
    say "Not an adult".
end.
```

## Loops

### Repeat

```bay
repeat 3 times
    say "Hello".
end.
```

### While

```bay
variable number equals 0.

while number is less than 3
    say the number end.
    variable number equals number + 1.
end.
```

## Functions

```bay
function add with a and b
    return a + b.
end.

variable result equals add with 10 and 20.

say the result end.
```

Output:

```text
30
```

## Input

```bay
ask the name.

say "Hello, \name!".
```

## Waves

Bay and Waves are separate programs.

- **Bay** — programming language, interpreter, and compiler
- **Waves** — package manager

Waves will provide package installation and package management.

## Architecture

Bay uses a shared frontend for both execution modes:

```text
             .bay source
                  |
                  v
                Lexer
                  |
                  v
                Parser
                  |
                  v
                 AST
               /     \
              /       \
     Interpreter     Compiler
                       |
                       v
                  Native Code
```

The interpreter executes Bay programs directly.

The compiler generates native executables.

## Compilation Targets

The long-term target list is:

```text
Windows x64
Windows ARM64

Linux x64
Linux ARM64

macOS x64
macOS ARM64
```

Native compilation is being implemented incrementally.

## Building

Bay is written in C.

After building, you can check the available commands with:

```bash
bay --help
```

## Roadmap

### v0.1

- [x] Lexer
- [x] Parser
- [x] AST
- [x] Interpreter
- [x] Variables
- [x] Strings
- [x] Arithmetic
- [x] Basic `say`
- [x] Native compiler foundation

### v0.2

- [x] Input
- [x] Booleans
- [x] Comparisons
- [x] Logical operators
- [x] Conditions
- [x] Loops
- [x] Functions
- [x] Return values
- [x] Function scope

### Future

- [ ] `include`
- [ ] Modules
- [ ] Package support
- [ ] Waves integration
- [ ] More native compilation targets
- [ ] Standard library
- [ ] More data structures

---

**Bay — programming that reads like plain English.**
