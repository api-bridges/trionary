# Trionary

> A minimal, readable programming language for pipeline-based data transformations.

Trionary is a statically-structured scripting language built around five keywords. It is intentionally small: no runtime, no dependencies, no ambiguity. The entire language is a single C11 binary.

---

## Table of Contents

- [Features](#features)
- [Installation](#installation)
  - [Prerequisites](#prerequisites)
  - [Build from Source](#build-from-source)
- [Usage](#usage)
- [Language Reference](#language-reference)
  - [Comments](#comments)
  - [Variables](#variables)
  - [Arithmetic](#arithmetic)
  - [Keywords](#keywords)
  - [Pipelines](#pipelines)
  - [Functions](#functions)
  - [Modules](#modules)
  - [CLI Input](#cli-input)
- [Examples](#examples)
- [Project Structure](#project-structure)
- [Design Principles](#design-principles)
- [Contributing](#contributing)
- [Changelog](#changelog)
- [License](#license)

---

## Features

- **8 keywords** — `lst`, `whn`, `trn`, `sum`, `emt`, `fn`, `end`, `use`
- **Pipeline-oriented** — chain filters, transforms, and aggregations
- **Named functions** — define reusable pure functions with `fn … end`
- **Built-in modules** — load math and I/O helpers with `use math` / `use io`
- **Zero dependencies** — pure C11, no libraries beyond the C standard math library
- **Single binary** — one executable, no installer or runtime required
- **POSIX-compatible** — runs on Linux, macOS, and any POSIX-compliant system
- **Clear error messages** — line-number-aware diagnostics

---

## Installation

### Prerequisites

| Tool | Minimum Version |
|------|----------------|
| GCC (or any C11-compatible compiler) | 4.8+ |
| GNU Make | 3.81+ |

### Build from Source

```bash
git clone https://github.com/api-bridges/trionary.git
cd trionary
make
```

This produces a `tri` binary in the project root.

To remove the compiled binary:

```bash
make clean
```

---

## Usage

```
tri run <file.tri> [arg0 arg1 ...]
```

| Argument | Description |
|----------|-------------|
| `run` | Execute a Trionary source file |
| `<file.tri>` | Path to the source file |
| `arg0 arg1 …` | Optional script arguments (accessible inside the script as `arg0`, `arg1`, …) |

**Example:**

```bash
tri run tests/demo.tri
tri run tests/test_cli_args.tri 7 3
```

---

## Language Reference

Trionary source files use the `.tri` extension.

### Comments

```tri
# This is a comment — everything after # is ignored
```

### Variables

Variables are assigned with `=`. Only numeric values (integer or float) are supported. A variable must be assigned before it is used.

```tri
price    = 100
discount = 20
```

### Arithmetic

Standard arithmetic expressions with proper operator precedence (PEMDAS). Emit the result with `-> emt`.

```tri
5 + 10 -> emt          # 15
3 * 4 + 2 -> emt       # 14  (* binds tighter than +)
```

Variables can appear in expressions:

```tri
price = 100
discount = 20
price - discount -> emt   # 80
```

### Keywords

| Keyword | Role | Description |
|---------|------|-------------|
| `lst` | Source | Opens a pipeline with a literal list of values |
| `whn` | Filter | Keeps only elements that satisfy a condition |
| `trn` | Transform | Applies an arithmetic operation to each element |
| `sum` | Aggregate | Reduces the pipeline to a single sum |
| `emt` | Output | Prints the current value(s) to stdout |
| `fn` | Definition | Opens a named function definition |
| `end` | Definition | Closes a `fn` block |
| `use` | Module | Loads a built-in module's functions into scope |

#### `lst` — List source

```tri
lst [1, 2, 3, 4, 5]
```

Begins a pipeline with a comma-separated list of numbers.

#### `whn` — Filter

```tri
whn >5
whn <3
```

Keeps elements that satisfy the given comparison. Supported operators: `>`, `<`, `>=`, `<=`, `=`.

#### `trn` — Transform

```tri
trn *2
trn +10
trn -1
trn * x + 1
```

Applies an arithmetic operation to every element in the pipeline. The pipeline element is the implicit left operand of the leading operator; the right-hand side may be any full arithmetic expression using literals and variables.

#### `sum` — Aggregate

```tri
sum
```

Collapses the pipeline into a single value equal to the sum of all remaining elements.

#### `emt` — Emit

```tri
emt
-> emt
```

Prints the current pipeline value(s) or the result of an expression to stdout.

### Pipelines

Stages are chained with `|`. A pipeline begins with `lst` and ends with `emt` (either standalone on the next line, or inline with `-> emt`).

```tri
lst [values] | stage | stage | ... -> emt
```

Or, when using `sum` as the final reduction stage:

```tri
lst [values] | stage | stage | sum
emt
```

---

### Functions

Named pure functions are defined with `fn … end`. The function name and parameter names appear on the same line as `fn`; the body is a single arithmetic expression on the next line.

```tri
fn add x y
  x + y
end
```

Functions are called by writing the name followed by arguments:

```tri
fn double n
  n * 2
end
double 5 -> emt       # 10
```

Functions may be called inside arithmetic expressions and inside pipeline transform stages:

```tri
fn square n
  n * n
end
lst [1,2,3,4,5] | trn * square 1 -> emt   # 1 4 9 16 25
```

- Up to **8 parameters** per function.
- Function bodies may reference previously-defined variables from the enclosing scope.
- Recursive calls are not supported in v0.3.

---

### Modules

Built-in modules are loaded with `use`. All functions from the module become available immediately after the directive.

```tri
use math
use io
```

#### `math` module

| Function | Params | Description |
|----------|--------|-------------|
| `floor`  | 1 | Round down to nearest integer |
| `ceil`   | 1 | Round up to nearest integer |
| `abs`    | 1 | Absolute value |
| `sqrt`   | 1 | Square root |
| `pow`    | 2 | `pow x y` → x raised to the power y |

#### `io` module

| Function    | Params | Description |
|-------------|--------|-------------|
| `print`     | 1 | Emits the argument to stdout and returns it |
| `read_line` | 1 | Reads a double from stdin; the argument is returned as default on EOF/error |

> **Note on `print`:** When used as a standalone arithmetic statement (e.g. `print 42`), the value is emitted once by `print` and once by the implicit emit in `STMT_ARITH`, producing two output lines. Use `print` inside pipeline transforms or assign its result to avoid duplication.

> **Note on `read_line`:** Takes one mandatory default value argument (`read_line 0`) that is returned when stdin is empty or at EOF.

---

### CLI Input

Arguments passed after the file name are available as `arg0`, `arg1`, … inside the script. They are automatically coerced to numbers. The built-in variable `argc` holds the count of script arguments.

```bash
tri run my_script.tri 10 20
```

Inside `my_script.tri`:

```tri
argc -> emt          # 2  (number of arguments)
arg0 -> emt          # 10 (first argument)
arg1 -> emt          # 20 (second argument)
arg0 + arg1 -> emt   # 30 (arithmetic with auto-coerced values)
```

#### Default values with `??`

The `??` operator returns its left operand if it is a defined variable, and the right operand (fallback) otherwise. This is useful when an argument may or may not be supplied:

```tri
arg0 ?? 1 -> emt     # arg0 if provided, else 1
arg1 ?? 0 -> emt     # arg1 if provided, else 0
```

`??` is right-associative and has lower precedence than all arithmetic operators.

---

## Examples

### Hello, arithmetic

```tri
10 + 5 -> emt        # 15
3 * 4 + 2 -> emt     # 14
```

### Variables

```tri
a = 10
b = 7
a + 5  -> emt    # 15
a * b  -> emt    # 70
```

### Filter a list

```tri
# Print numbers greater than 5
lst [1,2,3,4,5,6,7,8,9,10] | whn >5 -> emt
# Output: 6  7  8  9  10
```

### Filter and transform

```tri
# Double every number greater than 5
lst [1,2,3,4,5,6,7,8,9,10] | whn >5 | trn *2 -> emt
# Output: 12  14  16  18  20
```

### Sum a list

```tri
lst [1,2,3,4,5] | sum -> emt    # 15
```

### Complex pipeline

```tri
# Keep numbers >2, multiply each by 10, then sum: (3+4+5)*10 = 120
lst [1,2,3,4,5] | whn >2 | trn *10 | sum
emt
# Output: 120
```

### Named functions

```tri
fn add x y
  x + y
end

fn mul x y
  x * y
end

add 3 4 -> emt    # 7
mul 6 7 -> emt    # 42
```

### Functions in pipelines

```tri
fn inc n
  n + 1
end

# Add 1 to every element greater than 2
lst [1,2,3,4,5] | whn >2 | trn + inc 0 -> emt
# Output: 4  5  6
```

### Built-in modules

```tri
use math

sqrt 16 -> emt          # 4
pow 2 10 -> emt         # 1024
abs -7 -> emt           # 7
floor 3.9 -> emt        # 3
ceil 3.1 -> emt         # 4
```

### CLI arguments

```bash
tri run calc.tri 10 20
```

```tri
# calc.tri
argc -> emt          # 2
arg0 + arg1 -> emt   # 30
arg2 ?? 0 -> emt     # 0 (not provided, falls back to 0)
```

---

## Project Structure

```
trionary/
├── src/
│   ├── main.c         # CLI entry point and statement execution loop
│   ├── reader.c       # File reading into memory buffer
│   ├── lexer.c        # Tokenizer / lexical analyser
│   ├── parser.c       # Pattern-based parser with AST generation
│   ├── exec.c         # Execution engine with symbol and function tables
│   ├── output.c       # Output formatting
│   ├── error.c        # Centralised error_at() helper
│   └── modules/
│       ├── math.c     # Built-in math module (floor, ceil, abs, sqrt, pow)
│       └── io.c       # Built-in io module (print, read_line)
├── include/
│   ├── reader.h
│   ├── lexer.h
│   ├── parser.h
│   ├── exec.h
│   ├── output.h
│   └── error.h
├── tests/
│   ├── run_tests.sh          # Automated test runner
│   ├── demo.tri              # Full feature demonstration
│   ├── test_arith.tri        # Arithmetic tests
│   ├── test_vars.tri         # Variable tests
│   ├── test_pipeline.tri     # Pipeline tests
│   ├── test_all.tri          # Combined test suite
│   ├── test_trn_expr.tri     # Expression RHS in trn
│   ├── test_fn.tri           # Function definition and calls
│   ├── test_modules.tri      # Built-in module usage
│   ├── test_cli_args.tri     # CLI argument variables
│   ├── test_error.tri        # Error case: missing emit
│   ├── test_invalid.tri      # Error case: invalid character
│   └── test_malformed.tri    # Error case: malformed syntax
├── Makefile
├── CHANGELOG.md
└── IMPLEMENTATION_SUMMARY.md
```

---

## Design Principles

| Principle | Description |
|-----------|-------------|
| **Readable like English** | Keywords are short but self-describing |
| **One concept = one keyword** | No synonyms, no aliases |
| **Strict structure** | Every statement has exactly one valid form |
| **Clarity over flexibility** | Predictable behaviour is preferred over expressive power |
| **Zero dependencies** | The binary links only against the C standard math library |
| **Single executable** | No installer, no runtime, no package manager |

---

## Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/your-feature`)
3. Make your changes
4. Build and verify: `make && ./tri run tests/test_all.tri`
5. Open a pull request

Please keep changes focused and minimal. New language features should follow the existing design principles.

---

## Changelog

### v0.2.0

#### New Features

| # | Feature | Description |
|---|---------|-------------|
| 1 | **Variable references in `trn`** | The `trn` keyword now accepts a variable name as its operand in addition to a literal number (e.g. `trn *scale`). The variable is resolved against the symbol table at execution time, so it always reflects the value most recently assigned before the pipeline runs. |
| 2 | **Line-numbered error messages** | All runtime error messages now include the source line number (e.g. `Error: Undefined variable 'x' at line 3`). This applies to undefined-variable errors in both arithmetic expressions and pipeline transforms. The lexer already included line numbers; this change makes the executor consistent. |
| 3 | **Multiple pipelines per file** | A single `.tri` file may now contain any number of complete `lst … emt` pipelines. Each pipeline's internal state (filter, transform, sum flag) is reset between runs while the symbol table remains shared, so variables assigned before or between pipelines are visible to all subsequent pipelines. |

#### Error-Message Format

Errors are written to `stderr` and follow this format:

```
Error: <description> at line <N>
```

**Examples:**

```
Error: Undefined variable 'scale' at line 5
Error: Expected number or variable after transform operator at line 3
Error: Unexpected character '"' at line 1
```

The program exits with code `1` on fatal errors and `0` on success.

#### Strict Constraints Preserved

The following v0.1.0 behaviours are **unchanged** in v0.2.0:

- All five keyword spellings (`lst`, `whn`, `trn`, `sum`, `emt`) are immutable.
- No existing `TokenType` value was removed or renumbered.
- `SymTable` maximum capacity (256 variables) and linear-scan semantics are unchanged.
- `emit_value()` output format (integers without decimal point, floats with `%.6g`) is unchanged.
- Pipeline execution order (filter → transform → aggregate/emit) is unchanged.
- `assign_stmt` still accepts only `IDENT = NUMBER` (literal number on the right-hand side).
- Errors continue going to `stderr`; normal output goes to `stdout`.
- All v0.1.0 regression test files produce byte-for-byte identical stdout, stderr, and exit codes.

---

### v0.3.0

#### New Features

| # | Feature | Description |
|---|---------|-------------|
| 1 | **Centralised error system** | All errors go through a single `error_at(line, msg, …)` helper in `src/error.c`. Consistent format: `Error: <message> at line <N>`. Fail-fast: the process exits with code 1 on the first error. |
| 2 | **Hash-map symbol table with scope stack** | The flat 256-slot variable array is replaced by a djb2-hashed open-addressing table with a `scope_push()` / `scope_pop()` API, enabling isolated function scopes. |
| 3 | **Expression RHS in `trn`** | The right-hand side of a `trn` operator may now be a full arithmetic expression (e.g. `trn * x + 1`), not just a literal or single variable. |
| 4 | **Named functions (`fn … end`)** | Define pure, reusable functions with positional parameters. Functions may be called in arithmetic expressions and in pipeline transform stages. |
| 5 | **Built-in modules (`use`)** | `use math` loads `floor`, `ceil`, `abs`, `sqrt`, `pow`. `use io` loads `print` and `read_line`. All modules are compiled in — no file-system access. |
| 6 | **CLI argument variables** | `arg0`, `arg1`, … hold script arguments (auto-coerced to `double`). `argc` holds the argument count. The `??` operator provides default values for missing arguments. |
| 7 | **Automated test suite** | `tests/run_tests.sh` runs all `test_*.tri` files and diffs against `.expected` output. `make test` executes the full suite. |

#### Backward Compatibility

All v0.2 programs run without change. The `argc` variable is always defined (value `0` when no extra arguments are supplied) and is reserved. Existing programs that do not reference `argc`, `arg0`…`argN`, `fn`, `end`, or `use` are completely unaffected.

---

## License

This project is open source. See [LICENSE](LICENSE) for details.
