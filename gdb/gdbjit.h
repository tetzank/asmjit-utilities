#ifndef GDBJIT_H_
#define GDBJIT_H_

namespace asmjit {

class GDBJit{

public:
	// register code with GDB with function name and function address + size
	static void addCodeSegment(const char *name, uint64_t addr, uint64_t size);
};

} // namespace asmjit

#endif
