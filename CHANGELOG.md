# Changelog

All notable changes to Trionary are documented in this file.

---

## [v0.3.0]

### Added

- **Centralised error system** (`src/error.c`, `include/error.h`): single `error_at(line, fmt, …)` helper used by all error paths. Format: `Error: <message> at line <N>`. Fail-fast — process exits on the first error.
- **Hash-map symbol table with scope stack** (`include/exec.h`, `src/exec.c`): replaced the flat 256-slot variable array with a djb2-hashed open-addressing table (128 slots per scope level). Added `scope_push()` / `scope_pop()` to support function-local scopes.
- **Expression RHS in `trn`**: the transform operator's right-hand side now accepts a full arithmetic expression (e.g. `trn * x + 1`), not just a literal or single variable. The pipeline element remains the implicit left operand.
- **Named functions (`fn … end`)**: define reusable pure functions with positional parameters. Up to 8 parameters; body is a single arithmetic expression. Functions can be called inside arithmetic expressions and pipeline transforms.
- **Built-in modules (`use`)**: `use math` provides `floor`, `ceil`, `abs`, `sqrt`, `pow`; `use io` provides `print` and `read_line`. All modules are compiled in — no file-system access required.
- **CLI argument variables**: `arg0`, `arg1`, … automatically hold command-line arguments (coerced to `double`). `argc` holds the argument count. The `??` operator supplies a fallback for missing arguments (`arg0 ?? 0`).
- **Automated test suite** (`tests/run_tests.sh`, `tests/*.expected`): `make test` runs every `test_*.tri` file and diffs output against its `.expected` file. Reports PASS/FAIL per test.

### Changed

- `trn` parser rule extended to call `parse_expr()` instead of consuming a single token.
- `Expr` / `ExprType` moved from private `parser.c` / `exec.c` definitions into `include/parser.h` so they are shared.
- `execute()` signature updated to accept a `FuncTable*` parameter (threaded from `main.c`).
- Lexer extended to allow underscores in identifiers (enables `read_line`).
- `TOK_NEWLINE` now emitted by the lexer; used as the param/body separator inside `fn` blocks.
- `Makefile` updated: added `src/error.c`, `src/modules/math.c`, `src/modules/io.c` to `SRCS`; added `make test` target.
- Version string updated from `v0` to `v0.3.0` in `main.c`.

### Fixed

- All error output now consistently uses the `Error: … at line N` format (previously some paths used bare `fprintf`).
- `free_functable()` no longer attempts to free the `body` expression of built-in functions.

### Backward Compatibility

All v0.1.0 and v0.2.0 programs produce byte-for-byte identical stdout, stderr, and exit codes. The `argc` variable is always defined (value `0` when no extra arguments are supplied) and is reserved; existing programs that do not reference it are completely unaffected.

---

## [v0.2.0]

### Added

- **Variable references in `trn`**: the transform keyword now accepts a variable name as its operand in addition to a literal number (e.g. `trn *scale`). The variable is resolved at execution time.
- **Line-numbered error messages**: all runtime errors include the source line number (e.g. `Error: Undefined variable 'x' at line 3`).
- **Multiple pipelines per file**: a single `.tri` file may contain any number of `lst … emt` pipelines. The symbol table is shared across all pipelines in the file; pipeline-internal state (filter, transform, sum flag) is reset between runs.

### Error-Message Format

```
Error: <description> at line <N>
```

The process exits with code `1` on error, `0` on success.

### Backward Compatibility

All v0.1.0 behaviours are unchanged:

- All five keyword spellings (`lst`, `whn`, `trn`, `sum`, `emt`) are immutable.
- No existing `TokenType` value was removed or renumbered.
- `emit_value()` output format (`%.6g`) is unchanged.
- Pipeline execution order (filter → transform → aggregate/emit) is unchanged.
- All v0.1.0 regression test files produce byte-for-byte identical stdout, stderr, and exit codes.

---

## [v0.1.0]

Initial release.

### Features

- Five keywords: `lst`, `whn`, `trn`, `sum`, `emt`.
- Pipeline-oriented data transformations.
- Arithmetic expressions with PEMDAS precedence.
- Variable assignment and reference.
- Single-pass pipeline execution.
- Zero external dependencies; pure C11 binary.
