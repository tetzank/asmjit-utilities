#include <gdb/jit-reader.h>

#include <stdlib.h>
#include <stdint.h>
#include <string.h>


GDB_DECLARE_GPL_COMPATIBLE_READER


static uint64_t read_uint64(char *ptr){
	uint64_t val;
	memcpy(&val, ptr, sizeof(uint64_t));
	return val;
}

enum gdb_status read_debug_info(__attribute__((unused)) struct gdb_reader_funcs *self, struct gdb_symbol_callbacks *cb, void *memory, __attribute__((unused)) long memory_sz){
	char *ptr = memory;
	// get begin and end of code segment
	GDB_CORE_ADDR begin = *(GDB_CORE_ADDR*)ptr;             // begin address
	ptr += sizeof(GDB_CORE_ADDR);
	GDB_CORE_ADDR end = *(GDB_CORE_ADDR*)ptr;               // end address
	ptr += sizeof(GDB_CORE_ADDR);
	// get name of function, just a single one per file
	const char *name = (const char*)ptr;                    // symbol name
	//ptr += strlen(name);
	//// potentially unaligned from here on
	//uint64_t num_files = read_uint64(ptr);
	//ptr += sizeof(uint64_t);
	//for(uint64_t i=0; i<num_files; ++i){
	//	
	//}

	struct gdb_object *obj = cb->object_open(cb);

#if 1
	struct gdb_symtab *symtab = cb->symtab_open(cb, obj, /*"gdbjit"*/"/home/frank/programming/asmjit-utilities_git/examples/sum.cpp");
	// returned value has no use
	/*struct gdb_block *block = */cb->block_open(cb, symtab, NULL, begin, end, name);

	struct gdb_line_mapping mapping[] = {
		{42, begin+0},
		{43, begin+2},
		{44, begin+5},
		{47, begin+11},
		{48, begin+13},
		{49, begin+17},
		{50, begin+20}
	};
	cb->line_mapping_add(cb, symtab, sizeof(mapping)/sizeof(struct gdb_line_mapping), mapping);

	cb->symtab_close(cb, symtab);
#else
	struct gdb_symtab *symtab = cb->symtab_open(cb, obj, "/home/frank/programming/asmjit-utilities_git/examples/sumSplit.cpp");
	// at least one block required
	/*struct gdb_block *parent = */cb->block_open(cb, symtab, NULL, begin, end, name);
// 	cb->block_open(cb, symtab, /*HACK*/(struct gdb_block*)0x1, begin, begin+11, "prolog");

	struct gdb_line_mapping mapping[] = {
		{43, begin+0},
		{44, begin+2},
		{45, begin+5},
// 		{50, begin+22}
	};
	cb->line_mapping_add(cb, symtab, sizeof(mapping)/sizeof(struct gdb_line_mapping), mapping);
	cb->symtab_close(cb, symtab);

#if 1
	struct gdb_symtab *symtab2 = cb->symtab_open(cb, obj, "/home/frank/programming/asmjit-utilities_git/examples/sumSplit2.cpp");
	// at least one block required
// 	cb->block_open(cb, symtab2, /*HACK*/(struct gdb_block*)0x1, begin+11, end, "loop");
	cb->block_open(cb, symtab2, NULL, begin+11, end, name);
	struct gdb_line_mapping mapping2[] = {
		{12, begin+11},
		{13, begin+13},
		{14, begin+17},
		{15, begin+20}
	};
	cb->line_mapping_add(cb, symtab2, sizeof(mapping2)/sizeof(struct gdb_line_mapping), mapping2);

	cb->symtab_close(cb, symtab2);
#endif

#endif

	cb->object_close(cb, obj);
	return GDB_SUCCESS;
}

enum gdb_status unwind(__attribute__((unused)) struct gdb_reader_funcs *self, __attribute__((unused)) struct gdb_unwind_callbacks *cb){
	//TODO
	return GDB_FAIL;
}

struct gdb_frame_id get_frame_id(__attribute__((unused)) struct gdb_reader_funcs *self, __attribute__((unused)) struct gdb_unwind_callbacks *cb){
	//TODO
	struct gdb_frame_id frame = {0, 0};
	return frame;
}

void destroy(struct gdb_reader_funcs *self){
	free(self);
}


struct gdb_reader_funcs *gdb_init_reader(void){
	struct gdb_reader_funcs *funcs = malloc(sizeof(struct gdb_reader_funcs));
	funcs->reader_version = GDB_READER_INTERFACE_VERSION;
	funcs->priv_data = NULL;

	funcs->read = read_debug_info;
	funcs->unwind = unwind;
	funcs->get_frame_id = get_frame_id;
	funcs->destroy = destroy;

	return funcs;
}
