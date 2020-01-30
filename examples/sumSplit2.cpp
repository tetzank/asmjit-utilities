#include <asmjit/asmjit.h>


void loop(asmjit::x86::Assembler *a){
	asmjit::Label l_loop = a->newLabel();

	// macro for abbreviation, adds debug info for each instruction
//#define DL jd.addDebugLine(a.offset())
#define DL

	a->bind(l_loop);                                                        // loop head
	DL; a->add(asmjit::x86::eax, asmjit::x86::dword_ptr(asmjit::x86::rdi)); // sum += *arr
	DL; a->add(asmjit::x86::rdi, sizeof(int));                              // arr++
	DL; a->dec(asmjit::x86::rsi);                                           // cnt--, sets zero flag
	DL; a->jnz(l_loop);                                                     // goto loop if not zero
}
