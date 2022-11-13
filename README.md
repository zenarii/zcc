# Zcc
---
Zcc is a C compiler written in C. It features a handwritten lexer and parser, and compiles C code into x64 assembly, which is then compiled by GCC. The compiler is rigourously tested against the programs found in the [test_programs folder](/test_programs). Each folder's tests can be run by the respective bash script.

## Building

The compiler can be built by running the **build.sh** script.

## Running

The compiler can be run with `zcc <c_file>`. The compiler currently supports the following options, which must be put before the name of the C file to be compiled.
```
-o <output executable name> - the name for the executable, must be put before the file to be compiled.
--delete-asm - delete the intermediate generated asm.
```