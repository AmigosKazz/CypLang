# CypLang 🦉

![License](https://img.shields.io/badge/license-MIT-green)
![Version](https://img.shields.io/badge/version-0.1.0-blue)

A compiler for the CypLang programming language — a French pseudo-code language that compiles to native executables via LLVM. Developed in C as part of an algorithms and compilation course.

**Pipeline:** `.cyp` source &rarr; Lexer &rarr; Parser/AST &rarr; IR (three-address code) &rarr; LLVM IR &rarr; native binary

## Quick Start

### Prerequisites

- **gcc** or **clang** (C11)
- **LLVM** development libraries (tested with LLVM 22)
- **make**
- **Git**

#### Install LLVM on macOS (Homebrew)

```bash
brew install llvm
```

> Homebrew installs LLVM as "keg-only". The Makefile auto-detects it at `/opt/homebrew/opt/llvm`. If `llvm-config` is already in your PATH (e.g. on Linux), that works too.

#### Install LLVM on Ubuntu/Debian

```bash
sudo apt install llvm-dev
```

### Build

```bash
git clone https://github.com/AmigosKazz/CypLang.git
cd CypLang
make
```

The `cyplang` executable is created at `build/bin/cyplang`.

### Hello, World!

```bash
# 1. Compile CypLang source to LLVM IR
./build/bin/cyplang compile examples/hello.cyp -o hello.ll

# 2. Compile LLVM IR to a native binary
clang hello.ll -o hello        # use system clang, or:
# /opt/homebrew/opt/llvm/bin/clang hello.ll -o hello   # Homebrew clang on macOS

# 3. Run it
./hello
# Hello, World!
```

### Debug mode (dump everything)

```bash
./build/bin/cyplang examples/hello.cyp
```

This prints the source, AST, three-address IR, and LLVM IR to stdout — useful for understanding the compilation pipeline.

### Run tests

```bash
make test
```

6 integration tests covering arithmetic, unary expressions, variables, floats, functions, and Hello World.

## CypLang Syntax

CypLang uses French keywords. Here's a quick reference:

```
entier x <- 42                    | Integer variable declaration + assignment
reel pi <- 3.14                   | Float variable declaration
afficher("Hello, World!")         | Print to stdout (maps to printf)

debfonc somme(d entier a, d entier b)   | Function declaration
  retourner a + b                        | Return statement
finfonc                                  | End of function

si condition alors                | If
  ...
sinon                             | Else
  ...
finsi                             | End if

tantque condition faire           | While loop
  ...
finfaire                          | End while
```

**Operators:** `+`, `-`, `*`, `/`, `div`, `mod`, `=`, `!=`, `<`, `>`, `<=`, `>=`, `et`, `ou`, `non`

**Types:** `entier` (int), `reel` (float), `chaine` (string), `booleen` (bool)

**Parameter modes:** `d` (data/input), `r` (result/output), `dr` (data-result/inout)

## Project Structure

```
cyplang/
├── examples/                        # CypLang code examples
│   └── hello.cyp                    #   Hello World
├── src/
│   ├── frontend/
│   │   ├── token/token.h            #   Token types and definitions
│   │   ├── lexer/lexer.{c,h}       #   Tokenizer (40+ token types)
│   │   ├── parser/parser.{c,h}     #   Recursive descent parser
│   │   └── ast/ast.{c,h}           #   AST node types and operations
│   ├── middle/
│   │   └── ir_generator.{c,h}      #   Three-address IR generation
│   ├── backend/
│   │   └── llvm_emitter.{c,h}      #   LLVM IR emission (C API)
│   └── main.c                       #   CLI entry point
├── tests/
│   ├── run.sh                       # Bash test harness
│   └── cases/                       # Test cases (.cyp + .expected)
├── Makefile                         # Primary build system
├── CMakeLists.txt                   # CMake (for IDE support)
└── LICENSE                          # MIT
```

## What Works Today

- [x] Full pipeline: `.cyp` &rarr; LLVM IR &rarr; native binary
- [x] Integer and float arithmetic with correct operator precedence
- [x] Variable declarations (`entier`, `reel`)
- [x] User-defined functions with parameters and return
- [x] String literals and `afficher()` (prints via `printf`)
- [x] CLI: `cyplang FILE.cyp` (debug) and `cyplang compile FILE.cyp -o OUT.ll` (compile)
- [x] Integration test suite (`make test`)

## Known Limitations

- Control flow (`si`/`tantque`/`pour`) has a parser bug — not yet emitted to LLVM IR
- All function parameters and return values are typed as `i32` (proper type threading planned)
- Mixed-type arithmetic (int + float in the same expression) is not yet supported
- No runtime library beyond `afficher` &rarr; `printf`
- No interpreter mode — compilation only

## License

This project is licensed under the MIT License — see the [LICENSE](LICENSE) file for details.
