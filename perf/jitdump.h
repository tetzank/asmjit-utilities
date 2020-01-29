#ifndef JITDUMP_HPP_
#define JITDUMP_HPP_


#ifdef __linux__

#include <cstdio>
#include <cstdint>
#include <vector>


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

#endif // __linux__

#endif
