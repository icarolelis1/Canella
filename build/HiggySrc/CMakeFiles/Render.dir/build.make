# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.22

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/i/Documentos/icaroDev/Higgy

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/i/Documentos/icaroDev/Higgy/build

# Include any dependencies generated for this target.
include HiggySrc/CMakeFiles/Render.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include HiggySrc/CMakeFiles/Render.dir/compiler_depend.make

# Include the progress variables for this target.
include HiggySrc/CMakeFiles/Render.dir/progress.make

# Include the compile flags for this target's objects.
include HiggySrc/CMakeFiles/Render.dir/flags.make

HiggySrc/CMakeFiles/Render.dir/Render/src/Render.cpp.o: HiggySrc/CMakeFiles/Render.dir/flags.make
HiggySrc/CMakeFiles/Render.dir/Render/src/Render.cpp.o: ../HiggySrc/Render/src/Render.cpp
HiggySrc/CMakeFiles/Render.dir/Render/src/Render.cpp.o: HiggySrc/CMakeFiles/Render.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/i/Documentos/icaroDev/Higgy/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object HiggySrc/CMakeFiles/Render.dir/Render/src/Render.cpp.o"
	cd /home/i/Documentos/icaroDev/Higgy/build/HiggySrc && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT HiggySrc/CMakeFiles/Render.dir/Render/src/Render.cpp.o -MF CMakeFiles/Render.dir/Render/src/Render.cpp.o.d -o CMakeFiles/Render.dir/Render/src/Render.cpp.o -c /home/i/Documentos/icaroDev/Higgy/HiggySrc/Render/src/Render.cpp

HiggySrc/CMakeFiles/Render.dir/Render/src/Render.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/Render.dir/Render/src/Render.cpp.i"
	cd /home/i/Documentos/icaroDev/Higgy/build/HiggySrc && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/i/Documentos/icaroDev/Higgy/HiggySrc/Render/src/Render.cpp > CMakeFiles/Render.dir/Render/src/Render.cpp.i

HiggySrc/CMakeFiles/Render.dir/Render/src/Render.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/Render.dir/Render/src/Render.cpp.s"
	cd /home/i/Documentos/icaroDev/Higgy/build/HiggySrc && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/i/Documentos/icaroDev/Higgy/HiggySrc/Render/src/Render.cpp -o CMakeFiles/Render.dir/Render/src/Render.cpp.s

HiggySrc/CMakeFiles/Render.dir/Device/src/Device.cpp.o: HiggySrc/CMakeFiles/Render.dir/flags.make
HiggySrc/CMakeFiles/Render.dir/Device/src/Device.cpp.o: ../HiggySrc/Device/src/Device.cpp
HiggySrc/CMakeFiles/Render.dir/Device/src/Device.cpp.o: HiggySrc/CMakeFiles/Render.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/i/Documentos/icaroDev/Higgy/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object HiggySrc/CMakeFiles/Render.dir/Device/src/Device.cpp.o"
	cd /home/i/Documentos/icaroDev/Higgy/build/HiggySrc && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT HiggySrc/CMakeFiles/Render.dir/Device/src/Device.cpp.o -MF CMakeFiles/Render.dir/Device/src/Device.cpp.o.d -o CMakeFiles/Render.dir/Device/src/Device.cpp.o -c /home/i/Documentos/icaroDev/Higgy/HiggySrc/Device/src/Device.cpp

HiggySrc/CMakeFiles/Render.dir/Device/src/Device.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/Render.dir/Device/src/Device.cpp.i"
	cd /home/i/Documentos/icaroDev/Higgy/build/HiggySrc && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/i/Documentos/icaroDev/Higgy/HiggySrc/Device/src/Device.cpp > CMakeFiles/Render.dir/Device/src/Device.cpp.i

HiggySrc/CMakeFiles/Render.dir/Device/src/Device.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/Render.dir/Device/src/Device.cpp.s"
	cd /home/i/Documentos/icaroDev/Higgy/build/HiggySrc && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/i/Documentos/icaroDev/Higgy/HiggySrc/Device/src/Device.cpp -o CMakeFiles/Render.dir/Device/src/Device.cpp.s

# Object files for target Render
Render_OBJECTS = \
"CMakeFiles/Render.dir/Render/src/Render.cpp.o" \
"CMakeFiles/Render.dir/Device/src/Device.cpp.o"

# External object files for target Render
Render_EXTERNAL_OBJECTS =

HiggySrc/libRender.a: HiggySrc/CMakeFiles/Render.dir/Render/src/Render.cpp.o
HiggySrc/libRender.a: HiggySrc/CMakeFiles/Render.dir/Device/src/Device.cpp.o
HiggySrc/libRender.a: HiggySrc/CMakeFiles/Render.dir/build.make
HiggySrc/libRender.a: HiggySrc/CMakeFiles/Render.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/i/Documentos/icaroDev/Higgy/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking CXX static library libRender.a"
	cd /home/i/Documentos/icaroDev/Higgy/build/HiggySrc && $(CMAKE_COMMAND) -P CMakeFiles/Render.dir/cmake_clean_target.cmake
	cd /home/i/Documentos/icaroDev/Higgy/build/HiggySrc && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/Render.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
HiggySrc/CMakeFiles/Render.dir/build: HiggySrc/libRender.a
.PHONY : HiggySrc/CMakeFiles/Render.dir/build

HiggySrc/CMakeFiles/Render.dir/clean:
	cd /home/i/Documentos/icaroDev/Higgy/build/HiggySrc && $(CMAKE_COMMAND) -P CMakeFiles/Render.dir/cmake_clean.cmake
.PHONY : HiggySrc/CMakeFiles/Render.dir/clean

HiggySrc/CMakeFiles/Render.dir/depend:
	cd /home/i/Documentos/icaroDev/Higgy/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/i/Documentos/icaroDev/Higgy /home/i/Documentos/icaroDev/Higgy/HiggySrc /home/i/Documentos/icaroDev/Higgy/build /home/i/Documentos/icaroDev/Higgy/build/HiggySrc /home/i/Documentos/icaroDev/Higgy/build/HiggySrc/CMakeFiles/Render.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : HiggySrc/CMakeFiles/Render.dir/depend

