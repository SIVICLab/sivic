# CMake generated Testfile for 
# Source directory: /data/lhst3/sivic/svn/sivic/trunk/tests/src
# Build directory: /data/lhst3/sivic/svn/sivic/trunk/tests/src
# 
# This file replicates the SUBDIRS() and ADD_TEST() commands from the source
# tree CMakeLists.txt file, skipping any SUBDIRS() or ADD_TEST() commands
# that are excluded by CMake control structures, i.e. IF() commands.
ADD_TEST(TRUNK_TEST "echo" "test")
ADD_TEST(VTK_RENDER_LOOP "../Linux_x86_64/vtkRenderLoopTest")
ADD_TEST(RUNTIME_EXCEPTION "../Linux_x86_64/runtimeException")
