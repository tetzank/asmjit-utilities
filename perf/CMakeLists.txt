cmake_minimum_required(VERSION 3.5)

project(asmjit_perf CXX)

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

add_library(asmjit_perf jitdump.cpp perfcompiler.cpp)
target_link_libraries(asmjit_perf ${ASMJIT_LIBRARIES})
