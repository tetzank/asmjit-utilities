#ifndef PERFCOMPILER_H_
#define PERFCOMPILER_H_

#include <vector>

#include <asmjit/x86/x86compiler.h>
#include "jitdump.h"


class PerfCompiler : public asmjit::x86::Compiler {
private:
	struct DebugLine {
		const char *file;
		size_t line;

		DebugLine(const char *file, size_t line) : file(file), line(line) {}
	};
	std::vector<DebugLine> debugLines;

	JitDump jd;

public:
	explicit PerfCompiler(asmjit::CodeHolder *code) noexcept;
	virtual ~PerfCompiler();

	// implicitly attached to latest node
	void attachDebugLine(const char *file_name=__builtin_FILE(), int line_number=__builtin_LINE());
	//
	void addCodeSegment(const char *fn_name, void *fn, uint64_t code_size);
	//
	asmjit::Error finalize();
};

#endif
