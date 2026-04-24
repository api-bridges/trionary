# Trionary v0.3.0 — Implementation Plan

> Design rule: Minimal + Fast + Predictable > Feature-rich
> Constraint: Full backward compatibility with v0.2 programs.

---

## Task 1 — Error System Upgrade

**Goal:** Improve runtime and parse-time errors with line numbers, meaningful messages, and fail-fast behavior.

**Scope:**
- Add a `line` field to every AST node and propagate it through `lexer.c` / `parser.c`.
- Centralise all error output in a single `error_at(line, msg, ...)` helper (new file `src/error.c` + `include/error.h`).
- Replace every bare `fprintf(stderr, ...)` call in `parser.c` and `exec.c` with the helper.
- Ensure the process exits with a non-zero code on the first error (fail-fast).

**Files changed:** `lexer.c`, `lexer.h`, `parser.c`, `parser.h`, `exec.c`, `src/error.c` *(new)*, `include/error.h` *(new)*

---

## Task 2 — Symbol Table Hardening

**Goal:** Make the variable / symbol table robust enough to support upcoming function scopes.

**Scope:**
- Replace the flat 256-slot array in `exec.c` with a small hash-map or open-addressing table.
- Add a `scope_push()` / `scope_pop()` API so future function calls can create isolated scopes.
- Keep the current single-scope behaviour identical (no behaviour change for v0.2 programs).

**Files changed:** `exec.c`, `exec.h`

---

## Task 3 — Expression Improvements in `trn`

**Goal:** Allow limited arithmetic expressions inside `trn` (e.g. `trn * x + 1`) without changing the pipeline syntax.

**Scope:**
- Extend the `trn` parser rule to accept a full arithmetic expression (using the existing expression parser) instead of only `op NUMBER`.
- The implicit pipeline element remains bound to the identifier `_` (or the first operand position) — decide and document the convention.
- Add test file `tests/test_trn_expr.tri`.

**Files changed:** `parser.c`, `exec.c`

---

## Task 4 — Function System (Declaration & Call)

**Goal:** Introduce named, pure functions with positional parameters and a single-expression body.

**Syntax:**
```
fn add x y
  x + y
end
```

**Scope:**
- Add `fn` and `end` keywords to `lexer.c`.
- Add a `FN_DEF` AST node and a `FN_CALL` expression node to `parser.h` / `parser.c`.
- Store function definitions in a separate function table in `exec.c` (independent from variable scopes).
- At call time, push a new scope (Task 2 API), bind parameters, evaluate body, pop scope.
- Functions may be called inside arithmetic expressions and inside pipeline stages.
- Add test file `tests/test_fn.tri`.

**Files changed:** `lexer.c`, `lexer.h`, `parser.c`, `parser.h`, `exec.c`, `exec.h`

---

## Task 5 — Module System (Built-in Modules Only)

**Goal:** Provide a `use <module>` directive that loads a curated set of built-in helpers without adding external dependencies.

**Initial modules:** `math` (floor, ceil, abs, sqrt, pow), `io` (read_line, print)

**Scope:**
- Add `use` keyword to `lexer.c`.
- Add a `USE_STMT` AST node to `parser.c`.
- In `exec.c`, implement a static module registry: each module is a C struct containing a list of built-in function descriptors.
- When `use math` is parsed, register its functions into the function table.
- No file system loading in v0.3 — all modules are compiled in.
- Add test file `tests/test_modules.tri`.

**Files changed:** `lexer.c`, `lexer.h`, `parser.c`, `parser.h`, `exec.c`, `exec.h`, `src/modules/math.c` *(new)*, `src/modules/io.c` *(new)*

---

## Task 6 — CLI Input Improvements

**Goal:** Make `arg0`, `arg1`, … more ergonomic and add basic type coercion.

**Scope:**
- Auto-coerce CLI arguments to numbers when used in arithmetic (current behaviour: string-only).
- Add `argc` as a built-in read-only variable containing the argument count.
- Allow a default value syntax: `arg0 ?? 0` — if the argument is absent, use the fallback.
- Document the feature in `README.md`.

**Files changed:** `main.c`, `exec.c`, `parser.c` (for `??` operator), `README.md`

---

## Task 7 — Regression Test Suite & CI Script

**Goal:** Ensure all v0.2 programs and new v0.3 features are verified automatically.

**Scope:**
- Create `tests/run_tests.sh`: iterates over every `test_*.tri`, runs `./tri run $file`, diffs against a `test_*.expected` file, reports PASS/FAIL.
- Add `.expected` output files for all existing tests (`test_arith`, `test_vars`, `test_pipeline`) and for every new test added in Tasks 3–6.
- Add a `make test` target to `Makefile`.
- CI must pass on a clean build (`make && make test`).

**Files changed:** `tests/run_tests.sh` *(new)*, `tests/*.expected` *(new)*, `Makefile`

---

## Task 8 — Documentation & Release Prep

**Goal:** Update all documentation to reflect v0.3.0 and prepare the release.

**Scope:**
- Rewrite the `README.md` language reference section with the new keywords (`fn`, `end`, `use`), expression syntax in `trn`, CLI input improvements, and module list.
- Update `IMPLEMENTATION_SUMMARY.md` to reflect new files, AST node types, and keyword count.
- Add a `CHANGELOG.md` with sections for v0.2.0 and v0.3.0.
- Bump any version strings in source or build files.
- Tag release as `v0.3.0` after all tasks pass CI.

**Files changed:** `README.md`, `IMPLEMENTATION_SUMMARY.md`, `CHANGELOG.md` *(new)*, `Makefile`

---

## Deferred to v0.4 / v1.0

| Feature | Reason deferred |
|---|---|
| VM / bytecode | Violates strict constraints; complexity not yet justified |
| Mutable variables inside functions | Risks breaking single-pass model |
| User-defined modules (file loading) | File I/O complexity; security surface area |
| String type | Significant parser/exec changes; out of scope |
| Loops / recursion | Breaks single-pass model; design decision needed |

---

## Priority Order

1. Task 1 — Error system (foundational; unblocks all others)
2. Task 2 — Symbol table hardening (required before Task 4)
3. Task 7 — Test suite (run in parallel; catches regressions early)
4. Task 3 — Expression improvements in `trn`
5. Task 4 — Function system
6. Task 5 — Module system
7. Task 6 — CLI input improvements
8. Task 8 — Documentation & release prep
