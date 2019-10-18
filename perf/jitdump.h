#ifndef JITDUMP_HPP_
#define JITDUMP_HPP_


#ifdef __linux__

#include <cstdio>
#include <cstdint>
#include <vector>

#include <elf.h>


namespace asmjit{

#define JITDUMP_MAGIC 0x4a695444
//#define JITDUMP_MAGIC 0x4454694a

struct header{
	// "JiTD"
	uint32_t magic = JITDUMP_MAGIC; //FIXME: change if architecture is big-endian
	uint32_t version = 1;
	// size in bytes of file header //FIXME: header is fixed-size, what is this?
	uint32_t total_size;
	// ELF architecture encoding, see /usr/include/elf.h, x86_64 == EM_X86_64
	uint32_t elf_mach = EM_X86_64; //FIXME: hardcoded architecture
	// padding
	uint32_t pad1 = 0;
	// JIT runtime pid
	uint32_t pid;
	// timestamp of when the file was created
	uint64_t timestamp;
	// bitmask of flags
	uint64_t flags = 0;
};


// flags
#define JITDUMP_FLAGS_ARCH_TIMESTAMP 0x1

struct record_header{
	// record type
	uint32_t id;
	// size in bytes of record including header
	uint32_t total_size;
	// creation timestamp of record
	uint64_t timestamp;
};

// record type
enum record_type{
	JIT_CODE_LOAD           = 0, // describing a jitted function
	JIT_CODE_MOVE           = 1, // already jitted function which is moved
	JIT_CODE_DEBUG_INFO     = 2, // debug info for function
	JIT_CODE_CLOSE          = 3, // end of jit runtime marker (optional)
	JIT_CODE_UNWINDING_INFO = 4  // unwinding info for a function
};

struct record_load{
	uint32_t pid;
	uint32_t tid;
	// virtual addess of jitted code start
	uint64_t vma;
	// code start address, default: vma == code_addr
	uint64_t code_addr;
	// size in bytes of jitted code
	uint64_t code_size;
	// unique identifier
	uint64_t code_index;

	// function name, null-terminated string
	// native code
};

struct record_debug{
	// address of function for which debug info in provided
	uint64_t code_addr;
	// number of debug entries
	uint64_t nr_entry;
	// array of debug_entry
	//debug_entry[n]
};
struct debug_entry{
	uint64_t code_addr;
	// source line
	uint32_t line;
	// column discriminator
	uint32_t discrim;
	// source file name, null-terminated
	//char name[n];
};

//TODO: other record types


class JitDump{
private:
	uint32_t nextid=0;
	FILE *fd;
	void *marker;
	long page_size;

	struct debugInfo{
		size_t offset;
		const char *file;
		int line;

		debugInfo(size_t offset, const char *file, int line) : offset(offset), file(file), line(line) {}
	};
	std::vector<debugInfo> debugEntries;

	uint64_t getTimestamp() const;

public:
	int init();
	void close();

	// add source line information
	void addDebugLine(size_t offset, const char *file_name=__builtin_FILE(), int line_number=__builtin_LINE());
	// dump function with associated function name
	void addCodeSegment(const char *fn_name, void *fn, uint64_t code_size);
};

}

#endif // __linux__

#endif
