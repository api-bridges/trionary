# Trionary v0.3.0 Implementation Summary

## Overview
Trionary v0.3.0 is a minimal, readable programming language with a focus on pipeline-based data transformations. The implementation is a single, zero-dependency C11 binary.

## Project Structure
```
trionary/
├── src/
│   ├── main.c         - CLI entry point and statement execution loop
│   ├── reader.c       - File reading into memory buffer
│   ├── lexer.c        - Tokenizer/lexical analyzer
│   ├── parser.c       - Pattern-based parser with AST generation
│   ├── exec.c         - Execution engine with symbol and function tables
│   ├── output.c       - Output formatting system
│   ├── error.c        - Centralised error_at() helper (new in v0.3)
│   └── modules/
│       ├── math.c     - Built-in math module (new in v0.3)
│       └── io.c       - Built-in io module (new in v0.3)
├── include/
│   ├── reader.h
│   ├── lexer.h
│   ├── parser.h
│   ├── exec.h
│   ├── output.h
│   └── error.h        - (new in v0.3)
├── tests/
│   ├── run_tests.sh          - Automated test runner (new in v0.3)
│   └── test_*.tri            - Test files (with *.expected counterparts)
├── Makefile
├── CHANGELOG.md       - (new in v0.3)
└── IMPLEMENTATION_SUMMARY.md
```

## Keywords (8 total)
1. **lst** - List source: Initiates a pipeline with literal values
2. **whn** - Filter: Applies condition to filter elements
3. **trn** - Transform: Applies arithmetic operation to elements (full expr RHS in v0.3)
4. **sum** - Summarise: Reduces pipeline to single sum
5. **emt** - Emit: Outputs result to stdout
6. **fn** - Function definition open (new in v0.3)
7. **end** - Function definition close (new in v0.3)
8. **use** - Module load directive (new in v0.3)

## AST Node Types

### Statement nodes (`NodeType`)
| Node | Description |
|------|-------------|
| `NODE_ASSIGN` | Variable assignment (`IDENT = NUMBER`) |
| `NODE_ARITH` | Arithmetic-emit statement (`expr -> emt`) |
| `NODE_PIPELINE` | Full pipeline (`lst … emt`) |
| `NODE_FN_DEF` | Function definition (`fn … end`) — v0.3 |
| `NODE_USE` | Module load (`use <name>`) — v0.3 |

### Expression nodes (`ExprType`)
| Node | Description |
|------|-------------|
| `EXPR_NUMBER` | Numeric literal |
| `EXPR_VARIABLE` | Variable reference |
| `EXPR_BINOP` | Binary arithmetic operation (`+`, `-`, `*`, `/`) |
| `EXPR_CALL` | Function call (`name arg1 arg2 …`) — v0.3 |
| `EXPR_NULLCOAL` | Null-coalescing / default-value operator (`??`) — v0.3 |

## Language Features

### Arithmetic
```tri
5 + 10 -> emt          # Output: 15
3 * 4 + 2 -> emt       # Output: 14 (precedence: * before +)
```

### Variables
```tri
a = 10
a + 5 -> emt           # Output: 15
```

### Pipelines
```tri
lst [1,2,3,4,5] | whn >2 | trn *10 | sum
emt                    # Output: 120
```

### Expression RHS in `trn` (v0.3)
```tri
scale = 3
lst [1,2,3,4] | trn * scale + 1 -> emt   # 4 7 10 13
```

### Named functions (v0.3)
```tri
fn add x y
  x + y
end
add 3 4 -> emt         # Output: 7
```

### Built-in modules (v0.3)
```tri
use math
sqrt 16 -> emt         # Output: 4
pow 2 8  -> emt        # Output: 256
```

### CLI arguments (v0.3)
```bash
tri run script.tri 10 20
```
```tri
argc -> emt            # Output: 2
arg0 + arg1 -> emt     # Output: 30
arg2 ?? 0 -> emt       # Output: 0 (not provided)
```

## Implementation Details

### Lexer
- Single-pass O(n) character scanner
- Recognizes 8 keywords (v0.3: added `fn`, `end`, `use`)
- Supports identifiers with underscores (enabling `read_line`)
- Handles comments (`#` to end of line)
- Emits `TOK_NEWLINE` tokens (used as param/body separator in `fn` blocks)
- Error reporting via `error_at()` with line numbers

### Parser
- Fixed-pattern matching (not recursive descent for statements)
- Recursive-descent expression parser with full operator precedence
- Five statement types (v0.3 adds `STMT_FN_DEF`, `STMT_USE`)
- `parse_primary()` detects function calls when IDENT is followed by arguments

### Execution Engine
- Direct AST walk (no bytecode or VM)
- Hash-map symbol table with scope stack (`scope_push` / `scope_pop`) — v0.3
- Separate function table (`FuncTable`, max 64 entries) — v0.3
- Built-in function dispatch via `BuiltinFn` function pointer — v0.3
- Single-pass pipeline execution (O(1) memory for aggregate operations)
- Processes each element sequentially through filter → transform → sum

### Error System (v0.3)
- Single `error_at(int line, const char* fmt, …)` helper in `src/error.c`
- All errors: `Error: <message> at line <N>` to stderr, then `exit(1)`
- Fail-fast: execution never continues after the first error

### Output System
- Integers printed without decimal point
- Floats printed with up to 6 significant figures (`%.6g`)
- All output to stdout with newline

## Test Results

### Core tests (v0.1 / v0.2)
```
test_arith.tri       → 15 / 14         ✓
test_vars.tri        → 15 / 70         ✓
test_pipeline.tri    → 120             ✓
test_all.tri         → all expected    ✓
```

### v0.3 feature tests
```
test_trn_expr.tri    → 12 16 20 / 21 22 23 / 120   ✓
test_fn.tri          → 7 / 42 / 10 / 10 11 12       ✓
test_modules.tri     → 3 / 4 / 5 / 3 / 8 / 14       ✓
test_cli_args.tri    → 2 / 10 / 20 / 30              ✓
```

### Error cases (exit code 1)
```
test_invalid.tri     → Error: Unexpected character '"' at line 1   ✓
test_trn_undef.tri   → Error: Undefined variable 'undef' at line 2 ✓
```

## Build & Run
```bash
# Compile (all sources)
make

# Run a script
./tri run file.tri

# Run with CLI arguments
./tri run file.tri 10 20

# Run the full test suite
make test

# Clean build artifacts
make clean
```

## Design Principles Met
✓ Readable like English, parsed like code
✓ Minimal vocabulary (8 keywords, each serving a distinct role)
✓ No synonyms (one concept = one keyword)
✓ Strict structure (no ambiguity)
✓ Clarity > flexibility
✓ Zero external dependencies
✓ Single binary executable
✓ POSIX-compatible
✓ Full backward compatibility with v0.1 and v0.2 programs