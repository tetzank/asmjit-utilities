function init(){
	var file_header = struct({
		magic      : array(char(), 4),//uint32(),
		version    : uint32(),
		total_size : uint32(),
		elf_match  : uint32(),
		pad1       : uint32(),
		pid        : uint32(),
		timestamp  : uint64(),
		flags      : uint64()
	});
	file_header.name = "file_header";

	var record_type = {
		JIT_CODE_LOAD           : 0, // describing a jitted function
		JIT_CODE_MOVE           : 1, // already jitted function which is moved
		JIT_CODE_DEBUG_INFO     : 2, // debug info for function
		JIT_CODE_CLOSE          : 3, // end of jit runtime marker (optional)
		JIT_CODE_UNWINDING_INFO : 4  // unwinding info for a function
	};

	var debug_entry = struct({
		code_addr : uint64(),
		line      : uint32(),
		discrim   : uint32(),
		name      : string("ascii").set({terminatedBy : 0})
	});
	debug_entry.name = "debug_entry";

	var record = taggedUnion(
		{ // record header
			id         : enumeration("record_type", uint32(), record_type),
			total_size : uint32(),
			timestamp  : uint64()
		},
		[ // alternatives depending on record type marked in id
			alternative(
				// selectIf
				function(){ return this.wasAbleToRead && this.id.value == record_type.JIT_CODE_LOAD; },
				{
					pid           : uint32(),
					tid           : uint32(),
					vma           : uint64(),
					code_addr     : uint64(),
					code_size     : uint64(),
					code_index    : uint64(),
					function_name : string("ascii").set({terminatedBy : 0}),
					native_code   : array(uint8(), function(){ return this.parent.code_size.value; })
				},
				"record_load"
			),
			alternative(
				// selectIf
				function(){ return this.wasAbleToRead && this.id.value == record_type.JIT_CODE_DEBUG_INFO; },
				{
					code_addr : uint64(),
					nr_entry  : uint64(),
					entries   : array(debug_entry, function(){ return this.parent.nr_entry.value; })
				},
				"record_debug"
			),
		]
	);
	record.name = "record";


	var jitdump = struct({
		header  : file_header,
		records : array(record, 100) //FIXME: size hardcoded, no length in format, read until EOF
	});
	jitdump.defaultLockOffset = 0;

	return jitdump;
}
