# LLVM IR Generation Demo - LLVM 20 Compatible

This is a modern LLVM IR generation demo program, updated to support LLVM 20 (the latest stable version).

## Major Changes

- Updated CMakeLists.txt to use modern CMake syntax and LLVM 20
- Updated all LLVM API calls to be compatible with LLVM 20
- Replaced deprecated ExecutionEngine with modern ORC JIT
- Removed legacy::FunctionPassManager, using the new PassManager system
- Fixed migration from SequentialType to PointerType (LLVM 20 uses opaque pointers)
- Fixed CloneModule API changes
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

# Install Bison (if system version is too old)
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

# Install Bison and Flex
sudo apt-get install bison flex

# Install CMake
sudo apt-get install cmake
```

## Build Steps

### 1. Set Environment Variables

```bash
# macOS
export CMAKE_PREFIX_PATH="/usr/local/opt/llvm@20"
export PATH="/usr/local/opt/bison/bin:$PATH"

# Linux
export CMAKE_PREFIX_PATH="/usr/lib/llvm-20"
export PATH="/usr/local/opt/bison/bin:$PATH"
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

### Define Function

```
ready> def foo(a b) a + b;
```

### Call Function

```
ready> foo(4, 5);
```

### Use Variables

```
ready> def bar(a) var x = a in x * x;
```

### Conditional Statement

```
ready> def max(a b) if a < b then b else a;
```

### Loop

```
ready> def sum(n) var i = 0, result = 0 in for i = 0, n, 1 in result = result + i;
```

### External Function Call

```
ready> extern printd(x);
ready> printd(42);
```

## Environment Variables Explanation

- `CMAKE_PREFIX_PATH`: Points to LLVM installation directory, used by CMake to find LLVM
- `PATH`: Adds bison path to ensure the correct version of bison is used

## Troubleshooting

### LLVM Not Found

If CMake reports that LLVM cannot be found, ensure that the correct `CMAKE_PREFIX_PATH` is set:

```bash
export CMAKE_PREFIX_PATH="/usr/local/opt/llvm@20"  # macOS
export CMAKE_PREFIX_PATH="/usr/lib/llvm-20"       # Linux
```

### Bison Version Error

If you encounter a bison version error, ensure you are using bison 3.0 or higher:

```bash
# Check bison version
bison --version

# If version is too old, install a new version
brew install bison  # macOS
sudo apt-get install bison  # Linux
```

### Link Errors

If you encounter link errors, ensure the LLVM library path is correct:

```bash
# macOS
export LDFLAGS="-L/usr/local/opt/llvm@20/lib"
export CPPFLAGS="-I/usr/local/opt/llvm@20/include"

# Linux
export LDFLAGS="-L/usr/lib/llvm-20/lib"
export CPPFLAGS="-I/usr/lib/llvm-20/include"
```

## Technical Details

### LLVM API Changes

1. **ExecutionEngine → ORC JIT**: The old ExecutionEngine has been replaced by ORC JIT
2. **legacy::FunctionPassManager**: Has been replaced by the new PassManager system
3. **SequentialType**: LLVM 20 uses opaque pointers, requiring the use of PointerType
4. **CloneModule**: Return value changed from raw pointer to unique_ptr
5. **LLJIT lookup**: Uses ExecutorAddr instead of raw pointers

### Compiler Flags

The project uses the following compiler flags:
- `-std=c++17`: C++17 standard
- `-Wall -Wextra`: Enable extra warnings
- `-fvisibility-inlines-hidden`: Hide inline function symbols
- `-stdlib=libc++`: Use libc++ standard library

## License

This project follows the license of the original LLVM Kaleidoscope tutorial.

## References

- [LLVM Official Documentation](https://llvm.org/docs/)
- [Kaleidoscope Tutorial](https://llvm.org/docs/tutorial/)
- [LLVM ORC JIT Documentation](https://llvm.org/docs/ORCv2.html)