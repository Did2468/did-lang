# did-lang

A statically typed, procedural programming language that compiles to AArch64 assembly — built from scratch in C, designed to run on bare-metal AArch64 hardware.

```
fn fib(n: i64) -> i64 {
    let a: i64 = 0;
    let b: i64 = 1;
    let i: i64 = 0;
    while i < n {
        let tmp: i64 = a + b;
        a = b;
        b = tmp;
        i = i + 1;
    }
    return a;
}
```

---

## What This Is

`did-lang` is a ground-up compiler implementation targeting AArch64 (ARM64) assembly. The end goal is a language whose compiled output runs natively on a bare-metal AArch64 OS — no libc, no runtime, no abstractions underneath.

The compiler is written in C with zero external dependencies. No LLVM, no Flex, no Bison. Every phase — lexing, parsing, type checking, IR generation, and code generation — is hand-written.

**Status:** Lexer ✅ · Parser ✅ · Type Checker 🔧 · IR 🔧 · AArch64 Codegen 🔧

---

## Language Features

**Currently supported:**
- Primitive types: `i64`, `bool`
- Variable declarations with explicit types: `let x: i64 = 42;`
- Arithmetic operators: `+`, `-`, `*`, `/`
- Comparison operators: `==`, `!=`, `<`, `>`, `<=`, `>=`
- Logical operators: `&&`, `||`, `!`
- Control flow: `if`/`else`, `while`
- Functions with typed parameters and return types
- Function calls
- Assignment: `x = expr`
- Line comments: `// ...`

**Planned:**
- Pointers and memory operations
- Arrays
- Structs
- A syscall layer targeting a bare-metal AArch64 OS

---

## Compiler Pipeline

```
Source (.mylang)
      │
      ▼
   Lexer                 → Token stream
      │
      ▼
   Parser                → Abstract Syntax Tree (AST)
      │
      ▼
   Semantic Analysis     → Type checking, scope resolution
      │
      ▼
   IR Generation         → Three-address linear IR
      │
      ▼
   AArch64 Codegen       → Assembly text (.s)
      │
      ▼
   GNU as / llvm-mc      → Object file
      │
      ▼
   Linker                → Binary (runs on bare-metal OS)
```

All AST nodes are allocated from a single arena allocator — one `malloc` for the slab, one `free` when compilation is done. No per-node heap allocation.

---

## Project Structure

```
did-lang/
├── src/
│   ├── main.c          # Driver — wires all phases together
│   ├── lexer.h / .c    # Hand-written lexer
│   ├── arena.h / .c    # Arena (bump) allocator
│   ├── ast.h / .c      # AST node definitions + debug printer
│   ├── parser.h / .c   # Recursive descent parser
│   ├── sema.h / .c     # Semantic analysis + type checker   [WIP]
│   ├── ir.h / .c       # Three-address IR                  [WIP]
│   └── codegen.h / .c  # AArch64 code generator            [WIP]
├── tests/
│   └── sample.mylang   # Example source file
└── Makefile
```

---

## Building

**Requirements:**
- A C11-capable compiler (`cc`, `clang`, or `gcc`)
- `make`

```bash
git clone https://github.com/yourusername/did-lang
cd did-lang
make
```

This produces a `mycc` binary in the project root.

---

## Usage

```bash
# Lex and dump the token stream
./mycc tokens tests/sample.mylang

# Parse and dump the AST
./mycc ast tests/sample.mylang

# Compile to AArch64 assembly  [coming soon]
./mycc compile tests/sample.mylang -o output.s
```

---

## Running on QEMU (target platform)

The compiler targets QEMU's `virt` AArch64 machine. Once codegen is complete, output assembly will be assembled and linked for bare-metal execution:

```bash
aarch64-elf-as output.s -o output.o
aarch64-elf-ld output.o -o output.elf
qemu-system-aarch64 -M virt -cpu cortex-a57 -kernel output.elf -nographic
```

---

## Design Decisions

**Hand-written recursive descent parser** — the grammar is written down explicitly before any parsing code. Operator precedence is encoded in the grammar's call depth, not a table. This is how production compilers (Clang, Go) handle it.

**Arena allocator** — all AST nodes come from a single memory slab. Allocation is a pointer bump. Freeing the entire AST is one `free()`. Recursive AST destructors do not exist.

**Pointer-into-source lexemes** — `Token` stores a `(char*, len)` pair pointing directly into the source buffer, not a heap-copied string. Zero allocations in the lexer.

**No external dependencies** — the compiler relies only on the C standard library. This keeps the build trivial and forces every design decision to be explicit.

---

## Roadmap

- [x] Lexer with full token set and error recovery
- [x] Recursive descent parser → AST
- [x] Arena allocator
- [x] AST pretty printer
- [ ] Semantic analysis — type checker and symbol table
- [ ] Three-address IR with explicit control flow
- [ ] AArch64 code generator (AAPCS64 calling convention)
- [ ] Stack frame layout and local variable allocation
- [ ] Function calls with correct prologue/epilogue
- [ ] Integration with bare-metal AArch64 OS

---

## Context

This compiler is being built alongside a bare-metal AArch64 OS targeting QEMU's `virt` machine. The long-term goal is a language that compiles to assembly that runs directly on that OS — no host OS, no libc, just the hardware.

Both projects are developed on an Apple M1 MacBook Air using an `aarch64-elf` cross-toolchain via Homebrew.

---

## License

MIT
