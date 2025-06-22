# CypLang 🦉

![Build Status](https://img.shields.io/badge/build-passing-brightgreen)
![Version](https://img.shields.io/badge/version-0.1.0-blue)
![License](https://img.shields.io/badge/license-MIT-lightgrey)

Interpreter and (coming soon) compiler for the CypLang programming language. This project is developed in C as part of an algorithms and compilation course.

## 🚀 Quick Start

### Prerequisites

- A C compiler (like `gcc` or `clang`)
- `CMake` (version 3.10 or higher)
- `Git`

### Compilation

1. Clone the repository:
   ```bash
   git clone https://github.com/YOUR_USERNAME/CypLang.git
   cd CypLang
   ```

2. Create a build folder and compile with CMake:
   ```bash
   mkdir build
   cd build
   cmake ..
   make
   ```
   The `cyplang` executable will be created in the `build` folder.

### Using the Interpreter

To execute a `.cyp` file, run the executable with the file path:

```bash
./cyplang ../examples/hello.cyp
```

## Project Structure
```
cyplang/
├── src/            # Source files (.c)
├── include/        # Header files (.h)
├── examples/       # CypLang code examples
├── build/          # Compilation folder (created locally)
└── CMakeLists.txt  # Configuration script for CMake
```


## 📜 License

This project is under MIT License. See the LICENSE file for more details.