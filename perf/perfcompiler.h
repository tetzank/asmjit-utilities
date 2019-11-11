#ifndef PERFCOMPILER_H_
#define PERFCOMPILER_H_

#include <vector>

#include <asmjit/x86/x86compiler.h>

// just forward declare, no need for full header
class JitDump;

class PerfCompiler : public asmjit::x86::Compiler {
private:
	struct DebugLine {
		const char *file;
		size_t line;

		DebugLine(const char *file, size_t line) : file(file), line(line) {}
	};
	std::vector<DebugLine> debugLines;

public:
	explicit PerfCompiler(asmjit::CodeHolder *code=nullptr) noexcept : asmjit::x86::Compiler(code) {}
	virtual ~PerfCompiler() {}

	// implicitly attached to latest node
	// adds file and line number as debug info to last created node (Compiler::cursor())
	void attachDebugLine(const char *file_name=__builtin_FILE(), int line_number=__builtin_LINE());

	// serialize sequence of nodes to machine code with Assembler
	// additionally get offset of instruction and store it to debug information
	// all debug info is stored in the passed JitDump object
	asmjit::Error finalize(JitDump &jd);
};

#endif
