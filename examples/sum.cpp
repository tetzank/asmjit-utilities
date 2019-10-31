#include <cstdio>
#include <vector>
#include <numeric>

#include <asmjit/asmjit.h>
#include "../perf/jitdump.h"


// AsmJit Assembler example for profiling with perf
// 
// profiling steps:
// $ perf record -k 1 ./sum
// $ perf inject -j -i perf.data -o perf.data.jitted
// $ perf report -i perf.data.jitted
//
// look for the function "foo" and press "a" to view annotated assembly

int main(){
	// function signature
	using SumFunc = int (*)(const int *arr, size_t cnt);

	// init asmjit
	asmjit::JitRuntime rt;
	JitDump jd;
	jd.init();

	asmjit::CodeHolder code;
	code.init(rt.codeInfo());
	asmjit::FileLogger logger(stdout); // write to stdout
	code.setLogger(&logger);
	asmjit::x86::Assembler a(&code);

	// assemble function
	// arr - rdi, cnt - rsi
	asmjit::Label l_loop = a.newLabel();
	asmjit::Label l_exit = a.newLabel();

	// macro for abbreviation, adds debug info for each instruction
#define DL jd.addDebugLine(a.offset())

	DL; a.xor_(asmjit::x86::eax, asmjit::x86::eax);                        // sum = 0
	DL; a.test(asmjit::x86::rsi, asmjit::x86::rsi);                        // if(cnt == 0)
	DL; a.jz(l_exit);                                                      //     goto exit

	a.bind(l_loop);                                                        // loop head
	DL; a.add(asmjit::x86::eax, asmjit::x86::dword_ptr(asmjit::x86::rdi)); // sum += *arr
	DL; a.add(asmjit::x86::rdi, sizeof(int));                              // arr++
	DL; a.dec(asmjit::x86::rsi);                                           // cnt--, sets zero flag
	DL; a.jnz(l_loop);                                                     // goto loop if not zero

	a.bind(l_exit);
	DL; a.ret();                                                           // return sum

	SumFunc fn;
	asmjit::Error err = rt.add(&fn, &code);
	if(err){
		fprintf(stderr, "runtime add failed with CodeCompiler\n");
		std::exit(1);
	}

	jd.addCodeSegment("foo", (void*)fn, code.codeSize());

	// generate some data
	std::vector<int> data(1 << 25);
	std::iota(data.begin(), data.end(), 0);
	// execute
	int result = fn(data.data(), data.size());
	printf("%d\n", result);

	jd.close();
	rt.release(fn);

	return 0;
}
