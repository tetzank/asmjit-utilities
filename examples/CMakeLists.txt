cmake_minimum_required(VERSION 3.5)

project(examples CXX)

# find asmjit
find_path(ASMJIT_INCLUDE_DIR
	asmjit/asmjit.h
	HINTS ENV ASMJIT_ROOT
	PATH_SUFFIXES src
)
find_library(ASMJIT_LIBRARIES
	asmjit
	HINTS ENV ASMJIT_ROOT
	PATH_SUFFIXES build
)
include_directories(${ASMJIT_INCLUDE_DIR})

# for profiling with perf
include_directories(../perf/)
find_library(ASMJIT_PERF_LIBRARIES
	asmjit_perf
	HINTS ../perf/
	PATH_SUFFIXES build
)

add_executable(sum sum.cpp)
target_link_libraries(sum ${ASMJIT_LIBRARIES} ${ASMJIT_PERF_LIBRARIES})

add_executable(sumCompiler sumCompiler.cpp)
target_link_libraries(sumCompiler ${ASMJIT_LIBRARIES} ${ASMJIT_PERF_LIBRARIES})
