# CypLang 🦉
![License](https://img.shields.io/badge/license-MIT-green)
![Version](https://img.shields.io/badge/version-0.0.1-blue)

Interpreter and compiler for the CypLang programming language. This project is developed in C as part of an algorithms and compilation course.

## 🚀 Quick Start

### Prerequisites

- A C compiler (like `gcc` or `clang`)
- `CMake` (version 3.10 or higher)
- `LLVM` development libraries
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

### Run the interpreter

To execute a `.cyp` file, run the executable with the file path:

```bash
./cyplang ../examples/hello.cyp
```

### Compile to LLVM IR
```bash
./cyplang compile ../examples/hello.cyp -o hello.ll
```

### Compile to executable
```bash
./cyplang compile ../examples/hello.cyp -o hello
```

## Project Structure
```
cyplang/
├── examples/                 # CypLang code examples
├── src/                      # Source files (.c)
│   ├── frontend/             # Front-end components
│   │   ├── lexer.c           # Tokenizer
│   │   ├── parser.c          # Parser (AST builder)
│   │   └── ast.c             # Abstract Syntax Tree
│   ├── middle/               # Middle-end components
│   │   └── ir_generator.c    # IR Generation (in progress)
│   ├── backend/              # Back-end components
│   │   └── llvm_emitter.c    # LLVM IR emission (planned)
│   └── main.c                # Program entry point
├── build/                    # Compilation folder (created locally)
└── CMakeLists.txt            # Configuration script for CMake
```


## 📜 License
This project is licensed under the MIT License - see the LICENSE file for details.