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


add_executable(sum sum.cpp ../perf/jitdump.cpp)
target_link_libraries(sum ${ASMJIT_LIBRARIES})
