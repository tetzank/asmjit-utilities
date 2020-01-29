#include "jitdump.h"


#ifdef __linux__

#include <cstring>

#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <unistd.h>
#include <time.h>
//#include <pthread.h>
#include <elf.h>


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
	// address of function for which debug info is provided
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



uint64_t JitDump::getTimestamp() const{
	timespec tp;
	clock_gettime(CLOCK_MONOTONIC, &tp); //FIXME: error handling
	return (uint64_t)tp.tv_sec * 1000000000 + tp.tv_nsec;
}

int JitDump::init(){
	header head;
	head.total_size = sizeof(header);
	head.pid = getpid();
	head.timestamp = getTimestamp();

	char namebuf[1024];
	snprintf(namebuf, sizeof(namebuf), "jit-%d.dump", head.pid);
	// create and open dump file
	int fdi = open(namebuf, O_CREAT | O_TRUNC | O_RDWR, 0666);
	if(fdi == -1) return -1; //TODO: add error msg
	// get memory page size for following mmap
	page_size = sysconf(_SC_PAGESIZE);
	if(page_size == -1) return -1; //TODO: add error msg
	// let perf record know about the jitdump file
	marker = mmap(nullptr, page_size, PROT_READ | PROT_EXEC, MAP_PRIVATE, fdi, 0);
	if(marker == MAP_FAILED) return -1; //TODO: add error msg
	// open file stream for writing
	fd = fdopen(fdi, "wb");
	if(fd == NULL) return -1; //TODO: add error msg

	// write file header
	fwrite(&head, sizeof(header), 1, fd);

	return 0;
}

void JitDump::close(){
	// remove mapping
	munmap(marker, page_size);
	// close stream which also closes file
	fclose(fd);
}

void JitDump::addDebugLine(size_t offset, const char *file_name, int line_number){
	debugEntries.emplace_back(offset, file_name, line_number);
}

void JitDump::addCodeSegment(const char *fn_name, void *fn, uint64_t code_size){
	// debug_info record before code_load record
	if(!debugEntries.empty()){
		// calculate total size
		uint32_t totalSize = uint32_t(sizeof(record_header) + sizeof(record_debug));
		for(const auto &e : debugEntries){
			totalSize += uint32_t(sizeof(debug_entry) + strlen(e.file) + 1);
		}
#ifndef NDEBUG
		printf("total debug size: %u (0x%x)\n", totalSize, totalSize);
#endif

		record_header rh;
		rh.id = JIT_CODE_DEBUG_INFO;
		rh.total_size = totalSize;
		rh.timestamp = getTimestamp();

		uint64_t base_addr = (uint64_t)fn;
		record_debug rdbg;
		rdbg.code_addr = base_addr;
		rdbg.nr_entry = debugEntries.size();

		fwrite(&rh, sizeof(record_header), 1, fd);
		fwrite(&rdbg, sizeof(record_debug), 1, fd);

		debug_entry de;
		de.discrim = 0;
		for(const auto &e : debugEntries){
			// *.so files created from jit dump contain elf header of size 0x40
			// adjust for that
			de.code_addr = base_addr + 0x40 + e.offset;
			de.line = e.line;
			// write
			fwrite(&de, sizeof(debug_entry), 1, fd);
			fwrite(e.file, strlen(e.file)+1, 1, fd);
		}
	}
	// done for this function, clear for next
	debugEntries.clear();

	// code_load record
	size_t name_len = strlen(fn_name);
	record_header rh;
	rh.id = JIT_CODE_LOAD;
	rh.total_size = uint32_t(sizeof(record_header) + sizeof(record_load) + name_len+1 + code_size);
	rh.timestamp = getTimestamp();

	record_load rl;
	rl.pid = getpid();
	//TODO: get OS TID to support multi-threaded applications
	rl.tid = rl.pid; // PID==TID if single-threaded, pthread_self() seems to be wrong
	rl.vma = (uint64_t)fn;
	rl.code_addr = rl.vma;
	rl.code_size = code_size;
	rl.code_index = nextid++;

	// write records
	fwrite(&rh, sizeof(record_header), 1, fd);
	fwrite(&rl, sizeof(record_load), 1, fd);
	fwrite(fn_name, name_len+1, 1, fd);
	fwrite(fn, code_size, 1, fd);
}


#endif // __linux__
