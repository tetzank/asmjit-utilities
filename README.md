# asmjit-utilities
helper classes for profiling and debugging of jitted code


## Profiling with perf on linux

The profiling tool perf on linux can annotate assembly code with profiling information.
This can be done for jit code as well if the jit runtime dumps the generated code in a special file format.
The class `JitDump` in the subdirectory perf adds support for this file format.

Usage in application:
- create `JitDump` object right after `JitRuntime` and call `init()` method
- when assembling a function get the code size from `CodeHolder::codeSize()`
- before calling function, dump it with `addCodeSegment(name, function_ptr, code_size)`
- call `close()` at the end

Profiling steps:
- `perf record -k 1 ./application`
- `perf inject -j -i perf.data -o perf.data.jitted`
- `perf report -i perf.data.jitted`

In `perf report` the generated function should appear with the associated name.
The annotated view can be reached by pressing 'a'.


## Debugging with GDB on linux

The debugging capabilities of GDB for jit compiled code is very limited currently as asmjit does not generate normal object files.
You only get debugging on assembly level within TUI mode of GDB (disassemble command does not work).
Furthermore, to be able to set (pending) breakpoints on a jitted function, you need to patch GDB with the patch included in the gdb subdirectory.

Before calling the generated function in your asmjit application, call `GDBJit::addCodeSegment()` with the function name, the function pointer and the code size (`CodeHolder::codeSize()`).
This tells GDB about the newly generated function and, if patched, makes it possible to set breakpoints on this function.
