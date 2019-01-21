#include <gdb/jit-reader.h>

#include <stdlib.h>


GDB_DECLARE_GPL_COMPATIBLE_READER


enum gdb_status read_debug_info(struct gdb_reader_funcs *self, struct gdb_symbol_callbacks *cb, void *memory, long memory_sz){
	// get begin and end of code segment
	GDB_CORE_ADDR begin = *(GDB_CORE_ADDR*)memory;
	memory += sizeof(GDB_CORE_ADDR);
	GDB_CORE_ADDR end = *(GDB_CORE_ADDR*)memory;
	memory += sizeof(GDB_CORE_ADDR);
	// get name of function, just a single one per file
	const char *name = (const char*)memory;

	struct gdb_object *obj = cb->object_open(cb);
	struct gdb_symtab *symtab = cb->symtab_open(cb, obj, "gdbjit");
	// returned value has no use
	/*struct gdb_block *block = */cb->block_open(cb, symtab, NULL, begin, end, name);

	cb->symtab_close(cb, symtab);
	cb->object_close(cb, obj);
	return GDB_SUCCESS;
}

enum gdb_status unwind(struct gdb_reader_funcs *self, struct gdb_unwind_callbacks *cb){
	//TODO
	return GDB_FAIL;
}

struct gdb_frame_id get_frame_id(struct gdb_reader_funcs *self, struct gdb_unwind_callbacks *cb){
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
