#include <cstdio>
#include <vector>
#include <numeric>

#include <asmjit/asmjit.h>
#include "../perf/jitdump.h"
#include "../perf/perfcompiler.h"


// AsmJit Compiler example for profiling with perf
// profiling steps:
// $ perf record -k 1 ./sum
// $ perf inject -j -i perf.data -o perf.data.jitted
// $ perf report -i perf.data.jitted

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
	//asmjit::x86::Compiler cc(&code);
	PerfCompiler cc(&code);

	// assemble function
	cc.addFunc(asmjit::FuncSignatureT<int,const int*,size_t>());
	// arguments
	asmjit::x86::Gp r_arr = cc.newIntPtr("arr");
	asmjit::x86::Gp r_cnt = cc.newUInt64("cnt");
	cc.setArg(0, r_arr);
	cc.setArg(1, r_cnt);
	// register for sum
	asmjit::x86::Gp r_sum = cc.newInt32("sum");

	asmjit::Label l_loop = cc.newLabel();
	asmjit::Label l_exit = cc.newLabel();

#define DL cc.attachDebugLine()

	cc.xor_(r_sum, r_sum); DL;                         // sum = 0
	cc.test(r_cnt, r_cnt); DL;                         // if(cnt == 0)
	cc.jz(l_exit); DL;                                 //     goto exit

	cc.bind(l_loop);                                // loop head
	cc.add(r_sum, asmjit::x86::dword_ptr(r_arr)); DL;  // sum += *arr
	cc.add(r_arr, sizeof(int)); DL;                    // arr++
	cc.dec(r_cnt); DL;                                 // cnt--; sets zero flag
	cc.jnz(l_loop); DL;                                // if(cnt != 0) goto loop

	cc.bind(l_exit);
	cc.ret(r_sum); DL;                                 // return sum;

	cc.endFunc();

	//cc.finalize();
	cc.finalize(jd);


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
