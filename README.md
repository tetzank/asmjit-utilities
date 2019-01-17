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
