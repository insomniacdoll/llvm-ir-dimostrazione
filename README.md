# LLVM IR Generation Demo - LLVM 20 Compatible

这是一个现代化的LLVM IR生成演示程序，已更新以支持LLVM 20（目前最新的稳定版本）。

## 主要更改

- 更新CMakeLists.txt以使用现代CMake语法和LLVM 20
- 将所有LLVM API调用更新为LLVM 20兼容的API
- 用现代ORC JIT替换了已弃用的ExecutionEngine
- 移除了legacy::FunctionPassManager，使用新的PassManager系统
- 修复了SequentialType到PointerType的迁移（LLVM 20使用opaque pointers）
- 修复了CloneModule API的变更
- 修复了LLJIT lookup API的变更
- 更新了bison语法以支持bison 3.8+
- 修复了所有已弃用的API调用

## 系统要求

- CMake 3.15或更高版本
- LLVM 20
- Bison 3.0或更高版本
- Flex 2.6或更高版本
- C++17兼容的编译器

## 安装依赖

### macOS (使用Homebrew)

```bash
# 安装LLVM 20
brew install llvm@20

# 安装Bison（如果系统版本太旧）
brew install bison

# 安装Flex（如果未安装）
brew install flex
```

### Linux (Ubuntu/Debian)

```bash
# 安装LLVM 20
wget https://apt.llvm.org/llvm.sh
chmod +x llvm.sh
sudo ./llvm.sh 20

# 安装Bison和Flex
sudo apt-get install bison flex

# 安装CMake
sudo apt-get install cmake
```

## 编译步骤

### 1. 设置环境变量

```bash
# macOS
export CMAKE_PREFIX_PATH="/usr/local/opt/llvm@20"
export PATH="/usr/local/opt/bison/bin:$PATH"

# Linux
export CMAKE_PREFIX_PATH="/usr/lib/llvm-20"
export PATH="/usr/local/opt/bison/bin:$PATH"
```

### 2. 创建构建目录并配置

```bash
mkdir build
cd build
cmake ..
```

### 3. 编译

```bash
make -j4
```

### 4. 运行

```bash
# 在build目录中
./src/kscope
```

## 使用示例

### 定义函数

```
ready> def foo(a b) a + b;
```

### 调用函数

```
ready> foo(4, 5);
```

### 使用变量

```
ready> def bar(a) var x = a in x * x;
```

### 条件语句

```
ready> def max(a b) if a < b then b else a;
```

### 循环

```
ready> def sum(n) var i = 0, result = 0 in for i = 0, n, 1 in result = result + i;
```

### 外部函数调用

```
ready> extern printd(x);
ready> printd(42);
```

## 环境变量说明

- `CMAKE_PREFIX_PATH`: 指向LLVM安装目录，用于CMake查找LLVM
- `PATH`: 添加bison的路径，确保使用正确版本的bison

## 故障排除

### 找不到LLVM

如果CMake报告找不到LLVM，请确保设置了正确的`CMAKE_PREFIX_PATH`：

```bash
export CMAKE_PREFIX_PATH="/usr/local/opt/llvm@20"  # macOS
export CMAKE_PREFIX_PATH="/usr/lib/llvm-20"       # Linux
```

### Bison版本错误

如果遇到bison版本错误，请确保使用bison 3.0或更高版本：

```bash
# 检查bison版本
bison --version

# 如果版本太旧，安装新版本
brew install bison  # macOS
sudo apt-get install bison  # Linux
```

### 链接错误

如果遇到链接错误，请确保LLVM库路径正确：

```bash
# macOS
export LDFLAGS="-L/usr/local/opt/llvm@20/lib"
export CPPFLAGS="-I/usr/local/opt/llvm@20/include"

# Linux
export LDFLAGS="-L/usr/lib/llvm-20/lib"
export CPPFLAGS="-I/usr/lib/llvm-20/include"
```

## 技术细节

### LLVM API变更

1. **ExecutionEngine → ORC JIT**: 旧的ExecutionEngine已被ORC JIT取代
2. **legacy::FunctionPassManager**: 已被新的PassManager系统取代
3. **SequentialType**: LLVM 20使用opaque pointers，需要使用PointerType
4. **CloneModule**: 返回值从原始指针改为unique_ptr
5. **LLJIT lookup**: 使用ExecutorAddr而不是原始指针

### 编译器标志

项目使用以下编译器标志：
- `-std=c++17`: C++17标准
- `-Wall -Wextra`: 启用额外警告
- `-fvisibility-inlines-hidden`: 隐藏内联函数符号
- `-stdlib=libc++`: 使用libc++标准库

## 许可证

本项目遵循原始LLVM Kaleidoscope教程的许可证。

## 参考资料

- [LLVM官方文档](https://llvm.org/docs/)
- [Kaleidoscope教程](https://llvm.org/docs/tutorial/)
- [LLVM ORC JIT文档](https://llvm.org/docs/ORCv2.html)