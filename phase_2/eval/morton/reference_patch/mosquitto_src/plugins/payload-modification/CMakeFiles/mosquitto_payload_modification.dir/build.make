# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.10

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
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
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = "/mnt/c/Users/KolbyO'Malley/Documents/CHESS/morton/challenge/mosquitto_src"

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = "/mnt/c/Users/KolbyO'Malley/Documents/CHESS/morton/challenge/mosquitto_src"

# Include any dependencies generated for this target.
include plugins/payload-modification/CMakeFiles/mosquitto_payload_modification.dir/depend.make

# Include the progress variables for this target.
include plugins/payload-modification/CMakeFiles/mosquitto_payload_modification.dir/progress.make

# Include the compile flags for this target's objects.
include plugins/payload-modification/CMakeFiles/mosquitto_payload_modification.dir/flags.make

plugins/payload-modification/CMakeFiles/mosquitto_payload_modification.dir/mosquitto_payload_modification.c.o: plugins/payload-modification/CMakeFiles/mosquitto_payload_modification.dir/flags.make
plugins/payload-modification/CMakeFiles/mosquitto_payload_modification.dir/mosquitto_payload_modification.c.o: plugins/payload-modification/mosquitto_payload_modification.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir="/mnt/c/Users/KolbyO'Malley/Documents/CHESS/morton/challenge/mosquitto_src/CMakeFiles" --progress-num=$(CMAKE_PROGRESS_1) "Building C object plugins/payload-modification/CMakeFiles/mosquitto_payload_modification.dir/mosquitto_payload_modification.c.o"
	cd "/mnt/c/Users/KolbyO'Malley/Documents/CHESS/morton/challenge/mosquitto_src/plugins/payload-modification" && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/mosquitto_payload_modification.dir/mosquitto_payload_modification.c.o   -c "/mnt/c/Users/KolbyO'Malley/Documents/CHESS/morton/challenge/mosquitto_src/plugins/payload-modification/mosquitto_payload_modification.c"

plugins/payload-modification/CMakeFiles/mosquitto_payload_modification.dir/mosquitto_payload_modification.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/mosquitto_payload_modification.dir/mosquitto_payload_modification.c.i"
	cd "/mnt/c/Users/KolbyO'Malley/Documents/CHESS/morton/challenge/mosquitto_src/plugins/payload-modification" && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E "/mnt/c/Users/KolbyO'Malley/Documents/CHESS/morton/challenge/mosquitto_src/plugins/payload-modification/mosquitto_payload_modification.c" > CMakeFiles/mosquitto_payload_modification.dir/mosquitto_payload_modification.c.i

plugins/payload-modification/CMakeFiles/mosquitto_payload_modification.dir/mosquitto_payload_modification.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/mosquitto_payload_modification.dir/mosquitto_payload_modification.c.s"
	cd "/mnt/c/Users/KolbyO'Malley/Documents/CHESS/morton/challenge/mosquitto_src/plugins/payload-modification" && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S "/mnt/c/Users/KolbyO'Malley/Documents/CHESS/morton/challenge/mosquitto_src/plugins/payload-modification/mosquitto_payload_modification.c" -o CMakeFiles/mosquitto_payload_modification.dir/mosquitto_payload_modification.c.s

plugins/payload-modification/CMakeFiles/mosquitto_payload_modification.dir/mosquitto_payload_modification.c.o.requires:

.PHONY : plugins/payload-modification/CMakeFiles/mosquitto_payload_modification.dir/mosquitto_payload_modification.c.o.requires

plugins/payload-modification/CMakeFiles/mosquitto_payload_modification.dir/mosquitto_payload_modification.c.o.provides: plugins/payload-modification/CMakeFiles/mosquitto_payload_modification.dir/mosquitto_payload_modification.c.o.requires
	$(MAKE) -f plugins/payload-modification/CMakeFiles/mosquitto_payload_modification.dir/build.make plugins/payload-modification/CMakeFiles/mosquitto_payload_modification.dir/mosquitto_payload_modification.c.o.provides.build
.PHONY : plugins/payload-modification/CMakeFiles/mosquitto_payload_modification.dir/mosquitto_payload_modification.c.o.provides

plugins/payload-modification/CMakeFiles/mosquitto_payload_modification.dir/mosquitto_payload_modification.c.o.provides.build: plugins/payload-modification/CMakeFiles/mosquitto_payload_modification.dir/mosquitto_payload_modification.c.o


# Object files for target mosquitto_payload_modification
mosquitto_payload_modification_OBJECTS = \
"CMakeFiles/mosquitto_payload_modification.dir/mosquitto_payload_modification.c.o"

# External object files for target mosquitto_payload_modification
mosquitto_payload_modification_EXTERNAL_OBJECTS =

plugins/payload-modification/mosquitto_payload_modification.so: plugins/payload-modification/CMakeFiles/mosquitto_payload_modification.dir/mosquitto_payload_modification.c.o
plugins/payload-modification/mosquitto_payload_modification.so: plugins/payload-modification/CMakeFiles/mosquitto_payload_modification.dir/build.make
plugins/payload-modification/mosquitto_payload_modification.so: plugins/payload-modification/CMakeFiles/mosquitto_payload_modification.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir="/mnt/c/Users/KolbyO'Malley/Documents/CHESS/morton/challenge/mosquitto_src/CMakeFiles" --progress-num=$(CMAKE_PROGRESS_2) "Linking C shared library mosquitto_payload_modification.so"
	cd "/mnt/c/Users/KolbyO'Malley/Documents/CHESS/morton/challenge/mosquitto_src/plugins/payload-modification" && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/mosquitto_payload_modification.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
plugins/payload-modification/CMakeFiles/mosquitto_payload_modification.dir/build: plugins/payload-modification/mosquitto_payload_modification.so

.PHONY : plugins/payload-modification/CMakeFiles/mosquitto_payload_modification.dir/build

plugins/payload-modification/CMakeFiles/mosquitto_payload_modification.dir/requires: plugins/payload-modification/CMakeFiles/mosquitto_payload_modification.dir/mosquitto_payload_modification.c.o.requires

.PHONY : plugins/payload-modification/CMakeFiles/mosquitto_payload_modification.dir/requires

plugins/payload-modification/CMakeFiles/mosquitto_payload_modification.dir/clean:
	cd "/mnt/c/Users/KolbyO'Malley/Documents/CHESS/morton/challenge/mosquitto_src/plugins/payload-modification" && $(CMAKE_COMMAND) -P CMakeFiles/mosquitto_payload_modification.dir/cmake_clean.cmake
.PHONY : plugins/payload-modification/CMakeFiles/mosquitto_payload_modification.dir/clean

plugins/payload-modification/CMakeFiles/mosquitto_payload_modification.dir/depend:
	cd "/mnt/c/Users/KolbyO'Malley/Documents/CHESS/morton/challenge/mosquitto_src" && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" "/mnt/c/Users/KolbyO'Malley/Documents/CHESS/morton/challenge/mosquitto_src" "/mnt/c/Users/KolbyO'Malley/Documents/CHESS/morton/challenge/mosquitto_src/plugins/payload-modification" "/mnt/c/Users/KolbyO'Malley/Documents/CHESS/morton/challenge/mosquitto_src" "/mnt/c/Users/KolbyO'Malley/Documents/CHESS/morton/challenge/mosquitto_src/plugins/payload-modification" "/mnt/c/Users/KolbyO'Malley/Documents/CHESS/morton/challenge/mosquitto_src/plugins/payload-modification/CMakeFiles/mosquitto_payload_modification.dir/DependInfo.cmake" --color=$(COLOR)
.PHONY : plugins/payload-modification/CMakeFiles/mosquitto_payload_modification.dir/depend

