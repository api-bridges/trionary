# Task 3 — Expression Improvements in `trn`: Completion Notes

## What Was Done

Task 3 from `plan.md` has been implemented in full. The goal was to allow full arithmetic expressions on the right-hand side of the `trn` pipeline stage, going beyond the old `op NUMBER` / `op VAR_REF` restriction.

---

## Convention

The **pipeline element is the implicit left operand** of the leading operator. The right-hand side of the operator may be any arithmetic expression built from numeric literals and previously-assigned variables.

Examples:
- `trn * 10`          → `element * 10`
- `trn * x`           → `element * x`
- `trn * x + 1`       → `element * (x + 1)` *(right side evaluated first as a full expression)*
- `trn + a * 2`       → `element + (a * 2)`

There is no special `_` placeholder; the element is always implicitly the left operand of the leading operator and the right-hand expression is evaluated independently.

---

## Changes Made

### Modified Files

| File | Change |
|------|--------|
| `include/parser.h` | Moved `ExprType` enum and `Expr` struct definition here (was private in `parser.c` and `exec.c`); replaced `Transform.value / is_var_ref / var_name` fields with a single `Expr* expr` pointer |
| `src/parser.c` | Removed local `ExprType`/`Expr` definitions (now from `parser.h`); updated `parse_primary()` to accept `TOK_VAR_REF` as a variable identifier; replaced the `if (TOK_VAR_REF) … else if (TOK_NUMBER)` trn parsing block with a single `parse_expr()` call; updated `free_ast()` to call `free_expr()` on each transform's expression |
| `src/exec.c` | Removed local `ExprType`/`Expr` definitions (now from `parser.h`); replaced the `is_var_ref` branch in `apply_transform()` with a single `eval_expr(trn->expr, sym)` call |

### New Files

| File | Purpose |
|------|---------|
| `tests/test_trn_expr.tri` | Verifies expression RHS in `trn`: `trn * x + 1`, `trn + a * 2`, and backward-compatible `trn * 10` |

---

## Design Decisions

- **Backward compatibility preserved.** The old `trn * 10` and `trn * a` (compact no-space) syntax continues to work unchanged; they are now parsed as trivial single-node expressions (`EXPR_NUMBER` and `EXPR_VARIABLE` respectively).
- **`Expr` moved to `parser.h`.** This was the minimal change needed to share the type between `parser.c`, `exec.c`, and the new `Transform.expr` field. The `ArithNode*`/`Expr*` cast used by the arithmetic-emit statement is unchanged.
- **`parse_primary()` now accepts `TOK_VAR_REF`.** This token type is emitted by the lexer when an alphabetic identifier immediately follows an operator (e.g. `*x`). Making the expression parser understand it ensures compact and spaced forms are interchangeable.
- **No `_` placeholder.** The leading operator already encodes the application of the pipeline element; a `_` binding would be redundant and would require a symbol-table side-effect during pipeline execution.

---

## Verification

```
$ make clean && make
$ ./tri run tests/test_arith.tri              # → 15 / 14
$ ./tri run tests/test_vars.tri               # → 15 / 70
$ ./tri run tests/test_pipeline.tri           # → 120
$ ./tri run tests/test_all.tri                # all expected values
$ ./tri run tests/test_trn_var.tri            # → 120
$ ./tri run tests/test_multi_pipeline.tri     # → 11 12 13 / 8 10 12
$ ./tri run tests/test_mixed_arith_pipeline.tri
$ ./tri run tests/test_trn_expr.tri           # → 12 16 20 / 21 22 23 / 120
# Error cases — exit code 1 with correct message:
$ ./tri run tests/test_invalid.tri
$ ./tri run tests/test_malformed_trn.tri
$ ./tri run tests/test_trn_undef.tri
```

All checks pass. No pre-existing test output changed.

---

## Files Touched (summary)

```
include/parser.h          ← added ExprType/Expr; Transform.expr replaces old fields
src/parser.c              ← removed local Expr; TOK_VAR_REF in parse_primary; parse_expr() for trn
src/exec.c                ← removed local Expr; apply_transform uses eval_expr
tests/test_trn_expr.tri   ← new
```

---

# Task 2 — Symbol Table Hardening: Completion Notes

## What Was Done

Task 2 from `plan.md` has been implemented in full. The goal was to replace the flat 256-slot variable array with a hash-map-based scope stack, and expose a `scope_push()` / `scope_pop()` API to support upcoming function scopes.

### Modified Files

| File | Change |
|------|--------|
| `include/exec.h` | Removed old `Symbol` / flat-array `SymTable`; added `ScopeEntry`, `Scope` (open-addressing hash table with parent pointer), new `SymTable` (wrapper holding `Scope *current`); added `scope_push()` and `scope_pop()` declarations |
| `src/exec.c` | Replaced linear-scan symtable with djb2 hash + linear-probing open-addressing hash table; `sym_set`, `sym_exists`, `sym_get` now walk the scope chain; added `scope_push`, `scope_pop`, and internal helpers `scope_hash`, `scope_lookup`, `scope_reserve` |

---

## Design Decisions

- **Open addressing, power-of-2 capacity.** `SCOPE_CAPACITY = 128` slots per scope level, djb2 hash folded via `& (SCOPE_CAPACITY - 1)`, linear probing for collisions.  Deletion is never needed (no variable removal in the language), so no tombstones are required.
- **Scope chain (`parent` pointer).** Each `Scope` carries a `parent` pointer so `sym_exists` and `sym_get` walk from the innermost scope outward; `sym_set` always writes into the current (innermost) scope, giving isolated function-local variables when `scope_push` is used.
- **Global scope is never popped.** `scope_pop` is a no-op when the current scope has no parent, preventing accidental destruction of the global scope.
- **No behaviour change for v0.2 programs.** All programs run with a single scope (the global one); the hash-map lookup is semantically identical to the old linear scan for any program that never calls `scope_push`.
- **Full memory cleanup.** `free_symtable` walks the scope chain and frees every level; `scope_pop` frees the popped level immediately.
- **Error on full table.** If all 128 slots in a scope level are occupied, `error_at()` is called with a descriptive message rather than silently dropping the variable (improvement over the old silent drop at `count >= 256`).

---

## Verification

```
$ make clean && make
$ ./tri run tests/test_arith.tri              # → 15 / 14
$ ./tri run tests/test_vars.tri               # → 15 / 70
$ ./tri run tests/test_pipeline.tri           # → 120
$ ./tri run tests/test_all.tri                # all expected values
$ ./tri run tests/test_trn_var.tri            # → 120
$ ./tri run tests/test_multi_pipeline.tri     # → 11 12 13 / 8 10 12
$ ./tri run tests/test_mixed_arith_pipeline.tri
# Error cases — exit code 1 with correct message:
$ ./tri run tests/test_invalid.tri            # Error: Unexpected character '"' at line 1
$ ./tri run tests/test_malformed_trn.tri      # Error: Expected number or variable …
$ ./tri run tests/test_trn_undef.tri          # Error: Undefined variable 'undef' at line 2
```

All checks pass. No callers outside `exec.c` required any modification.

---

## Files Touched (summary)

```
include/exec.h   ← replaced Symbol/flat SymTable with ScopeEntry/Scope/SymTable; added scope_push/scope_pop
src/exec.c       ← replaced linear-scan implementation with hash-map + scope-chain implementation
```

---

# Task 1 — Error System Upgrade: Completion Notes

## What Was Done

Task 1 from `plan.md` has been implemented in full. The goal was to improve runtime and parse-time errors with line numbers, meaningful messages, and fail-fast behaviour.

---

## Changes Made

### New Files

| File | Purpose |
|------|---------|
| `include/error.h` | Declares the `error_at(int line, const char *fmt, ...)` helper |
| `src/error.c` | Implements `error_at`: prints `Error: <message> at line <N>` to stderr then calls `exit(1)` |

### Modified Files

| File | Change |
|------|--------|
| `Makefile` | Added `src/error.c` to the `SRCS` list so it is compiled and linked |
| `include/parser.h` | Added `int line` field to `ArithNode`, `AssignNode`, and `PipelineNode` |
| `src/lexer.c` | Replaced bare `fprintf(stderr, ...)` for unknown characters with `error_at()` |
| `src/parser.c` | Removed the local `error()` helper and `error_occurred` flag; replaced every `error(msg, line)` call with `error_at(line, msg)`; propagated line numbers into `AssignNode` and `PipelineNode`; updated `parse_pipeline()` signature to accept `lst_line` |
| `src/exec.c` | Replaced every `fprintf(stderr, ...)` and bare `exit(1)` with `error_at()` calls that carry the correct source line number |

---

## Design Decisions

- **Single exit point for errors.** All error output now goes through `error_at()`, ensuring the message format `Error: <message> at line <N>` is consistent everywhere.
- **Fail-fast.** `error_at()` calls `exit(1)` immediately, so the interpreter never continues executing after the first error. The now-redundant `error_occurred` flag and the `if (error_occurred) return NULL` block in `parse()` were removed.
- **No behaviour change for valid programs.** Every existing test (`test_arith`, `test_vars`, `test_pipeline`, `test_all`, `test_trn_var`, `test_multi_pipeline`, `test_mixed_arith_pipeline`) produces byte-for-byte identical stdout. Error test cases (`test_invalid`, `test_malformed_trn`, `test_trn_undef`) still exit with code 1 and print the same error format.
- **`sym_get` fallback.** The `sym_get` path that was only reachable through a defensive code path now calls `error_at(0, ...)` (line 0 indicates "unknown") instead of a silent `fprintf`. In practice this path remains unreachable because all callers first call `sym_exists`.

---

## Verification

```
$ make clean && make
$ ./tri run tests/test_arith.tri    # → 15 / 14
$ ./tri run tests/test_vars.tri     # → 15 / 70
$ ./tri run tests/test_pipeline.tri # → 120
$ ./tri run tests/test_all.tri      # all expected values
$ ./tri run tests/test_trn_var.tri  # → 120
$ ./tri run tests/test_multi_pipeline.tri
$ ./tri run tests/test_mixed_arith_pipeline.tri
# Error cases — exit code 1 with correct message:
$ ./tri run tests/test_invalid.tri  # Error: Unexpected character '"' at line 1
$ ./tri run tests/test_malformed_trn.tri
$ ./tri run tests/test_trn_undef.tri
```

All checks pass.

---

## Files Touched (summary)

```
include/error.h         ← new
src/error.c             ← new
include/parser.h        ← added line fields to AST nodes
src/lexer.c             ← error_at() replaces fprintf
src/parser.c            ← error_at() replaces local error(); line propagation
src/exec.c              ← error_at() replaces fprintf + exit(1)
Makefile                ← src/error.c added to SRCS
```
