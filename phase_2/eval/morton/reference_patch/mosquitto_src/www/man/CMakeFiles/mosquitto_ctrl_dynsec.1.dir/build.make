# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.16

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
CMAKE_SOURCE_DIR = /home/user/Downloads/mosquitto

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/user/Downloads/mosquitto

# Utility rule file for mosquitto_ctrl_dynsec.1.

# Include the progress variables for this target.
include man/CMakeFiles/mosquitto_ctrl_dynsec.1.dir/progress.make

man/CMakeFiles/mosquitto_ctrl_dynsec.1: man/mosquitto_ctrl_dynsec.1


man/mosquitto_ctrl_dynsec.1: man/mosquitto_ctrl_dynsec.1.xml
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/home/user/Downloads/mosquitto/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Generating mosquitto_ctrl_dynsec.1"
	cd /home/user/Downloads/mosquitto/man && xsltproc /home/user/Downloads/mosquitto/man/mosquitto_ctrl_dynsec.1.xml -o /home/user/Downloads/mosquitto/man/

mosquitto_ctrl_dynsec.1: man/CMakeFiles/mosquitto_ctrl_dynsec.1
mosquitto_ctrl_dynsec.1: man/mosquitto_ctrl_dynsec.1
mosquitto_ctrl_dynsec.1: man/CMakeFiles/mosquitto_ctrl_dynsec.1.dir/build.make

.PHONY : mosquitto_ctrl_dynsec.1

# Rule to build all files generated by this target.
man/CMakeFiles/mosquitto_ctrl_dynsec.1.dir/build: mosquitto_ctrl_dynsec.1

.PHONY : man/CMakeFiles/mosquitto_ctrl_dynsec.1.dir/build

man/CMakeFiles/mosquitto_ctrl_dynsec.1.dir/clean:
	cd /home/user/Downloads/mosquitto/man && $(CMAKE_COMMAND) -P CMakeFiles/mosquitto_ctrl_dynsec.1.dir/cmake_clean.cmake
.PHONY : man/CMakeFiles/mosquitto_ctrl_dynsec.1.dir/clean

man/CMakeFiles/mosquitto_ctrl_dynsec.1.dir/depend:
	cd /home/user/Downloads/mosquitto && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/user/Downloads/mosquitto /home/user/Downloads/mosquitto/man /home/user/Downloads/mosquitto /home/user/Downloads/mosquitto/man /home/user/Downloads/mosquitto/man/CMakeFiles/mosquitto_ctrl_dynsec.1.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : man/CMakeFiles/mosquitto_ctrl_dynsec.1.dir/depend

