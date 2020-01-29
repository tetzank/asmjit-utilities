#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cstdint>

#include "gdbjit.h"


enum jit_actions : uint32_t {
	JIT_NOACTION = 0,
	JIT_REGISTER_FN,
	JIT_UNREGISTER_FN,
};

struct jit_code_entry {
	struct jit_code_entry *next_entry;
	struct jit_code_entry *prev_entry;
	const char *symfile_addr;
	uint64_t symfile_size;
};

struct jit_descriptor {
	uint32_t version;
	jit_actions action_flag;
	struct jit_code_entry *relevant_entry;
	struct jit_code_entry *first_entry;
};


extern "C" {

// GDB puts a breakpoint in this function.
void __attribute__((noinline)) __jit_debug_register_code() {}

// Make sure to specify the version statically, because the
// debugger may check the version before we can set it.
struct jit_descriptor __jit_debug_descriptor = { 1, JIT_NOACTION, NULL, NULL };

} // extern "C"


void GDBJit::addCodeSegment(const char *name, uint64_t addr, uint64_t size){
#ifndef NDEBUG
	puts("register_code called");
	printf("name: %s; addr: %lu; size: %lu\n", name, addr, size);
#endif

	uint64_t name_size = strlen(name)+1; // including null terminator
	uint64_t symfile_size = name_size + 2*sizeof(uint64_t);
	char *symfile = (char*)malloc(symfile_size);
	char *ptr = symfile;
	// begin address
	*(uint64_t*)ptr = addr;
	ptr += sizeof(uint64_t);
	// end address
	*(uint64_t*)ptr = addr + size;
	ptr += sizeof(uint64_t);
	// function/symbol name
	(void)memcpy(ptr, name, name_size);

	// create entry
	jit_code_entry *n = new jit_code_entry;
	n->next_entry = nullptr;
	n->prev_entry = nullptr;
	n->symfile_addr = symfile;
	n->symfile_size = symfile_size;
	// insert into linked list
	jit_code_entry *entry = __jit_debug_descriptor.first_entry;
	n->next_entry = entry;
	if(entry){
		entry->prev_entry = n;
	}
	__jit_debug_descriptor.first_entry = n;
	// let GDB know about the new entry
	__jit_debug_descriptor.action_flag = JIT_REGISTER_FN;
	__jit_debug_descriptor.relevant_entry = n;

	puts("calling GDB");
	__jit_debug_register_code();

	free(symfile);
	delete n;
}
