# LLVM IR Generation Demo - LLVM 20 Compatible

This is a modern LLVM IR generation demo program, built against LLVM 20.

It implements a small Kaleidoscope-style language: a bison/flex front end builds
an AST, the AST generates LLVM IR, and anonymous expressions are JIT-compiled and
executed with ORC.

## Major Changes

- Updated CMakeLists.txt to use modern CMake syntax and LLVM 20
- Updated all LLVM API calls to be compatible with LLVM 20
- Replaced deprecated ExecutionEngine with modern ORC JIT
- Fixed migration from SequentialType to PointerType (LLVM 20 uses opaque pointers)
- Fixed LLJIT lookup API changes
- Updated bison grammar to support bison 3.8+
- Fixed all deprecated API calls

## System Requirements

- CMake 3.15 or higher
- LLVM 20
- Bison 3.0 or higher
- Flex 2.6 or higher
- C++17 compatible compiler

## Installing Dependencies

### macOS (using Homebrew)

```bash
# Install LLVM 20
brew install llvm@20

# Install Bison: the system one on macOS is 2.3, too old for this grammar
# (it requires >= 3.0). Installing is not enough on its own -- step 1 has to
# put Homebrew's bison first on PATH, or CMake picks up /usr/bin/bison.
brew install bison

# Install Flex (if not installed)
brew install flex
```

### Linux (Ubuntu/Debian)

```bash
# Install LLVM 20
wget https://apt.llvm.org/llvm.sh
chmod +x llvm.sh
sudo ./llvm.sh 20

# Install Bison, Flex and development libraries
sudo apt-get update
sudo apt-get install -y bison flex libfl-dev

# Install CMake
sudo apt-get install cmake
```

## Build Steps

### 1. Set Environment Variables

```bash
# macOS
export CMAKE_PREFIX_PATH="$(brew --prefix llvm@20)"
export PATH="$(brew --prefix bison)/bin:$PATH"

# Linux (apt.llvm.org installs LLVM 20 under /usr/lib/llvm-20, which is not on
# CMake's default search path, so the hint is needed here too)
export CMAKE_PREFIX_PATH="$(llvm-config-20 --prefix)"
```

### 2. Create Build Directory and Configure

```bash
mkdir build
cd build
cmake ..
```

### 3. Build

```bash
make -j4
```

### 4. Run

```bash
# In the build directory
./src/kscope
```

## Usage Examples

Statements are separated by `;` and `#` starts a comment that runs to the end of
the line. Every value is an IEEE double.

### Define Function

```
ready> def foo(a, b) a + b;
```

### Call Function

```
ready> foo(4, 5);
Evaluated to: 9.000000
```

### Use Variables

```
ready> def bar(a) var x = a in x * x;
```

Bindings are mutable, so `=` also works as an expression:

```
ready> def bump(a) var x = a in x = x + 1;
ready> bump(41);
Evaluated to: 42.000000
```

### Operators

`+`, `-`, `*` and `/`, plus the comparisons `<` and `>`, which yield 1.0 or 0.0.
Literals may carry any number of decimal places (`3.14159`).

### Conditional Statement

```
ready> def max(a, b) if a < b then b else a;
```

### Loop

```
ready> def test(n) for i = 1, n+1, 1 in i * i;
ready> test(5);
Evaluated to: 0.000000
```

The form is `for <var> = <start>, <end>[, <step>] in <body>`, and `<end>` is
exclusive — `for i = 1, 4, 1` visits 1, 2 and 3. A `for` expression always
evaluates to 0.0: the body runs for its side effects and its value is discarded,
so call `printd` inside it to observe the iterations.

### External Function Call

```
ready> extern printd(x);
ready> printd(42);
```

`printd` and `putchard` are registered with the JIT, so the `extern` line is
optional for them.

### Exiting

```
ready> quit;
```

`exit` works as well; both are case-insensitive.

## Environment Variables Explanation

- `CMAKE_PREFIX_PATH`: Points to LLVM installation directory, used by CMake to find LLVM
- `PATH`: Adds bison path to ensure the correct version of bison is used

## Troubleshooting

### LLVM Not Found

If CMake reports that LLVM cannot be found, ensure that the correct `CMAKE_PREFIX_PATH` is set:

```bash
# macOS (use brew --prefix for correct path)
export CMAKE_PREFIX_PATH="$(brew --prefix llvm@20)"

# Linux
export CMAKE_PREFIX_PATH="/usr/lib/llvm-20"
```

### Bison Version Error

If you encounter a bison version error, ensure you are using bison 3.0 or higher:

```bash
# Check bison version
bison --version

# If version is too old, install a new version
brew install bison  # macOS
sudo apt-get install bison  # Linux

# On macOS that is not sufficient by itself: /usr/bin/bison is still 2.3, so
# Homebrew's copy must come first on PATH before configuring:
export PATH="$(brew --prefix bison)/bin:$PATH"
```

### Link Errors

CMake locates LLVM through `find_package(LLVM REQUIRED CONFIG)`, so export
`CMAKE_PREFIX_PATH` (see step 1) rather than `LDFLAGS`/`CPPFLAGS` — CMake does
not read those for this lookup.

```bash
# macOS (use brew --prefix for correct path)
export CMAKE_PREFIX_PATH="$(brew --prefix llvm@20)"

# Linux
export CMAKE_PREFIX_PATH="$(llvm-config-20 --prefix)"
```

## Technical Details

### Release Artifacts

The project uses GitHub Actions to build and release binaries for the following platforms:

- **Linux**: `kscope-linux-x86_64-{version}.tar.gz`
- **macOS**: `kscope-macos-arm64-{version}.tar.gz` (Apple Silicon)

Releases are automatically created when a version tag (e.g., `v0.1`, `v1.0.0`) is pushed to the repository.

### LLVM API Notes

These are the LLVM 20 APIs this code actually uses:

1. **ExecutionEngine → ORC JIT**: The old ExecutionEngine has been replaced by
   `llvm::orc::LLJIT`.
2. **LLJIT lookup**: `lookup()` yields an `ExecutorAddr` rather than a raw pointer.
3. **Opaque pointers**: LLVM 20 has no typed pointers, so a load states its type
   explicitly (`CreateLoad(Ty, Ptr, ...)`) and allocas are queried through
   `getAllocatedType()`.

### Compiler Flags

The project uses the following compiler flags:
- `-std=c++17`: C++17 standard
- `-Wall -Wextra`: Enable extra warnings
- `-fvisibility-inlines-hidden`: Hide inline function symbols

These are applied with `add_compile_options()`, so they are appended to whatever
you pass in `CMAKE_CXX_FLAGS` rather than replacing it.

### Build Type

The build defaults to `Debug`, which keeps LLVM's internal assertions enabled.
Override it when you want an optimised binary:

```bash
cmake -DCMAKE_BUILD_TYPE=Release ..
```

### Generated Sources

Bison and Flex output (`bison_parser.cc`, `bison_parser.hh`, `flex_lexer.cc`,
`stack.hh`) is written into the **build** directory, not into `src/parsing`.
The source tree is therefore never modified by a build.

## License

This project follows the license of the original LLVM Kaleidoscope tutorial.

## References

- [LLVM Official Documentation](https://llvm.org/docs/)
- [Kaleidoscope Tutorial](https://llvm.org/docs/tutorial/)
- [LLVM ORC JIT Documentation](https://llvm.org/docs/ORCv2.html)