# c-bug-lab

A collection of small, broken C functions, each with a verified fix, built to teach how to **read code, find the bug and prove the fix**.

The project started on a 2016 ASUS laptop (Intel Celeron N3160, 7.6 GB RAM, Debian + i3) as a side experiment of the "What can a 2016 laptop still do?" series. Its first exercise came from a real failure: a 0.5B-parameter local model wrote a string-reversal function with an off-by-one bug, then declared it "correct".

## Why this exists

Reading someone else's code and locating a defect is a different skill from writing code, and it is rarely practiced. Rustlings and Ziglings do this for Rust and Zig. This lab applies the same idea to C, where the bugs are subtle and the failure modes are severe: silent memory corruption, leaks, undefined behavior.

Three design goals:

1. **Every bug is known and labeled.** An exercise is never "some code that looks wrong". It has a category, a location and an explanation.
2. **Every exercise is machine-verified.** The broken version must fail a test, and the fixed version must pass the same test. No exercise is accepted on trust.
3. **Hints are cheap and optional.** A small local model can write hints. If a hint is wrong, that is a teaching moment, not a defect in the lab.

## Design: three roles, kept separate

| Role | Done by | Why |
|---|---|---|
| **Bug injection** | A deterministic script (mutation operators) | The injected bug is exactly known. A small language model cannot reliably break code on purpose; it often returns correct code, code that does not compile, or a different bug than requested. |
| **Verification (the judge)** | Compiler, sanitizers, tests | Only execution decides whether a bug is real and a fix is correct. This is the same compile, test and norminette loop planned for the judge-station machine. |
| **Hints and explanations** | A local LLM (optional) | Natural-language explanation is what small models are acceptable at. Output is labeled as a hint and never used as ground truth. |

### Mutation operators

Starting from a correct function (for example a libft function), a script applies one mechanical mutation and records which one.

| Operator | Example mutation | Bug category |
|---|---|---|
| Boundary shift | `i < n` becomes `i <= n` | Off-by-one |
| Size error | `size - 1` becomes `size`, or `len + 1` becomes `len` | Buffer overflow, missing terminator |
| Delete assignment | remove `dst[i] = '\0';` | Missing null terminator |
| Delete call | remove a `free(p);` | Memory leak |
| Reorder statements | move `free(p);` before the last use of `p` | Use-after-free |
| Delete initializer | `int count = 0;` becomes `int count;` | Uninitialized variable |
| Type swap | `int` becomes `size_t` in a comparison against a possibly negative value | Signed/unsigned comparison |
| Pointer step | forget `end--` after scanning to `'\0'` | Pointer off-by-one |

### Why the judge decides, not the script

A mutation does not always produce a bug. Two cases must be filtered out:

- **Non-compiling mutants** are discarded.
- **Equivalent mutants** compile and behave identically to the original (for example, changing a condition in dead code). With no failing test, they are not exercises and are discarded.

This is standard mutation-testing practice. It is also why the verification step is mandatory: the acceptance rule below is the only thing that turns a mutant into an exercise.

## Detection matrix

Different bugs are caught by different tools. That is part of the curriculum: students learn which tool to reach for.

| Category | Compiler warnings | Sanitizers / tools | Test needed |
|---|---|---|---|
| Off-by-one, in-bounds (such as the first exercise) | none | none (the write stays inside the array) | **Yes** |
| Heap buffer overflow | sometimes | AddressSanitizer | optional |
| Stack buffer overflow | sometimes | AddressSanitizer | optional |
| Use-after-free | rarely | AddressSanitizer | optional |
| Memory leak | none | LeakSanitizer (part of ASan on Linux), Valgrind | optional |
| Uninitialized variable | `-Wmaybe-uninitialized` (not reliable) | Valgrind (`--track-origins=yes`) | usually |
| Signed/unsigned comparison | `-Wsign-compare` (in `-Wextra`) | UBSan in some cases | usually |
| Signed integer overflow | `-Wall` in simple cases | UBSan | optional |
| Missing null terminator | none | Valgrind, ASan on over-read | **Yes** |

The first exercise is deliberately in the "test needed" row: the compiler and the sanitizers say nothing, and only running the code against an expectation reveals the bug.

## Repository layout

```
c-bug-lab/
  README.md
  exercises/
    off_by_one_01/
      broken.c        # the buggy function plus a main for manual runs
      fixed.c         # the verified fix
      test.c          # shared test: returns non-zero on failure
      meta.txt        # category, line of the bug, one-paragraph explanation
  tools/
    mutate.py         # (planned) applies one mutation operator, records it
    judge.sh          # (planned) builds and runs broken/fixed, applies the rule
```

### `meta.txt` format

```
category: off-by-one (pointer)
location: reverseString, before the swap loop
symptom: prints an empty string
explanation: after the length loop, `end` points at the terminating '\0',
  one past the last character. The first swap moves '\0' to str[0].
fix: decrement `end` once before swapping.
```

## Acceptance rule

An exercise is accepted only if all of this holds:

1. `broken.c` compiles with `-Wall -Wextra`.
2. `broken.c` linked with `test.c` **fails** (non-zero exit code or a sanitizer report).
3. `fixed.c` linked with `test.c` **passes** (exit code 0, no sanitizer report).
4. The diff between `broken.c` and `fixed.c` is as small as possible and matches the category in `meta.txt`.

Reference build and run command:

```
gcc -Wall -Wextra -g -fsanitize=address,undefined exercise.c test.c -o exercise && ./exercise
```

## Using it for teaching

- Give students `broken.c` and `test.c` only. Ask them to predict the failure before running it, then locate and fix the bug.
- Show which tool catches which bug (see the detection matrix). Several exercises should fail silently under the sanitizers on purpose.
- Optionally let a small local model produce a hint, and ask students to decide whether the hint is right. At 3 tokens per second on the original laptop, the model's output appears word by word, which makes the next-token mechanism visible. The model is a fallible assistant, not an authority.
- Exercises are compatible with 42-style constraints (no external libraries, small functions), so they can double as warm-up for C evaluations.

## Exercises

| ID | Category | Origin | Detected by |
|---|---|---|---|
| `off_by_one_01` | Pointer off-by-one (string reverse) | Written by `qwen2.5-coder:0.5b`, which also claimed it was correct | Test only |

## Roadmap

- [ ] Move `off_by_one_01` into the `exercises/` layout above, with `test.c` and `meta.txt`
- [ ] Write `judge.sh` implementing the acceptance rule
- [ ] Add 10 hand-written exercises covering every category in the table
- [ ] Write `mutate.py` with the operators above, and filter non-compiling and equivalent mutants
- [ ] Optional: generate hints with a local model and label them as unverified
- [ ] Optional: run the judge as a service on the old laptop

## Status

Early stage. One exercise, manually verified. Contributions of real bugs found in the wild are welcome; real examples are better than generated ones.
