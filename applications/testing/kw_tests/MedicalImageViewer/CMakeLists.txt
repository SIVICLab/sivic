cmake_minimum_required(VERSION 2.4)
if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

project(MedicalImageViewer)

# If we are not in the KWWidgets source tree, make sure we can find KWWidgets
# as an external package, and use it. If you are using this CMakeLists.txt 
# file to create your own application based on KWWidgets, you only need the
# FIND_PACKAGE(...) and INCLUDE(...) commands. 

if(NOT KWWidgets_SOURCE_DIR)
  find_package(KWWidgets REQUIRED)
  include(${KWWidgets_USE_FILE})
endif(NOT KWWidgets_SOURCE_DIR)

# The name of our targets (executable or libraries) will simply be based
# on the project name, with an extra prefix and suffix.

set(TARGET_BASE_NAME "KW${PROJECT_NAME}Example")

# We define several classes in this example, and we want to be able to use
# their C++ methods as callbacks for our user interface. To do so, we need to 
# create a library and wrap it automatically for the Tcl language, which
# is used as a bridge between C++ objects at run-time. Note that an
# initialization function is automatically created in this library to allow
# classes and C++ methods to be used as commands and callbacks inside the Tcl
# interpreter; do *not* forget to call this function right after you 
# initialize the Tcl interpreter in your application. The name of this 
# function is built from the library name in lower-case (except for the first
# letter) and suffixed with "_Init" (for example: 
# KWMedicalImageViewerExampleLib => Kwmedicalimageviewerexamplelib_Init)
# This whole process is required to use C++ methods as callbacks; it is not
# needed if you use VTK's C++ command/observer pattern directly, which is
# demonstrated in KWWidgets's C++ 'Callbacks' example.

set(LIB_NAME "KWMedicalImageViewerExampleLib")
set(LIB_SRCS 
  vtkKWMyWindow.cxx
  )

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

include("${KWWidgets_CMAKE_DIR}/KWWidgetsWrappingMacros.cmake")
kwwidgets_wrap_tcl(${LIB_NAME} LIB_TCL_SRCS "${LIB_SRCS}" "")

# Create the library. The library is built in static mode for convenience. 
# Check the 'Callbacks' example for more information about building it in
# shared mode, i.e. without the STATIC keyword (Win32 compilers requires
# an additional header file to setup DLL export symbols correctly).

add_library(${LIB_NAME} STATIC ${LIB_TCL_SRCS} ${LIB_SRCS})
target_link_libraries(${LIB_NAME} ${KWWidgets_LIBRARIES})

# The name of our executable and the corresponding source file.

set(EXE_NAME "${TARGET_BASE_NAME}")
set(EXE_SRCS "${EXE_NAME}.cxx")

# On Win32 platforms, let's create a .rc resource file to setup a decent
# application icon as well as some additional information in the "Version"
# tab of the properties panel.

if(WIN32 AND NOT BORLAND AND NOT CYGWIN)
  include("${KWWidgets_CMAKE_DIR}/KWWidgetsResourceMacros.cmake")
  set(RC_FILENAME "${CMAKE_CURRENT_BINARY_DIR}/${EXE_NAME}.rc")
  kwwidgets_create_rc_file(
    RC_FILENAME "${RC_FILENAME}"
    RC_APPLICATION_NAME "${EXE_NAME}"
    RC_COMPANY_NAME "Kitware, Inc.")
endif(WIN32 AND NOT BORLAND AND NOT CYGWIN)

# This example uses some files from the KWWidgets distribution tree.
# Let's configure KWWidgets's vtkKWWidgetsPaths.h.in into our
# own header file so that we can find the paths to KWWidgets files.

include_directories(${CMAKE_CURRENT_BINARY_DIR})
configure_file(
  ${KWWidgets_TEMPLATES_DIR}/vtkKWWidgetsPaths.h.in 
  ${CMAKE_CURRENT_BINARY_DIR}/vtkKWWidgetsPaths.h)

# Create the executable, and link it against the KWWidgets libraries and our
# own library.

add_executable(${EXE_NAME} WIN32 ${EXE_SRCS} ${RC_FILENAME})
target_link_libraries(${EXE_NAME} ${LIB_NAME})

# If we are building this example as a standalone external project:
# - Generate a few small scripts (.bat, .sh, .csh) that can be sourced to set
# the various environments variables (PATH, TCLLIBPATH, LD_LIBRARY_PATH, etc.) 
# required by this executable and its known third-party dependencies (VTK, ITK,
# SOV, KWWidgets, etc.).
# - Generate a lightweight C launcher for this *specific* executable: It sets
# the above environment variables before launching the executable itself.

if(NOT KWWidgets_SOURCE_DIR)
  include("${KWWidgets_CMAKE_DIR}/KWWidgetsPathsMacros.cmake")
  kwwidgets_generate_setup_paths_scripts("${CMAKE_CURRENT_BINARY_DIR}")
  set(LAUNCHER_EXE_NAME "${EXE_NAME}Launcher")
  kwwidgets_generate_setup_paths_launcher(
    "${CMAKE_CURRENT_BINARY_DIR}" "${LAUNCHER_EXE_NAME}" "" "${EXE_NAME}")
endif(NOT KWWidgets_SOURCE_DIR)

# If needed, copy the Tcl/Tk support files required at run-time 
# to initialize Tcl/Tk. This is only triggered if VTK was built
# against a Tcl/Tk static library.

include("${KWWidgets_CMAKE_DIR}/KWWidgetsTclTkMacros.cmake")
if(NOT KWWidgets_SOURCE_DIR)
  kwwidgets_copy_tcl_tk_support_files("${PROJECT_BINARY_DIR}/lib")
endif(NOT KWWidgets_SOURCE_DIR)

# Install the example target. If you are using this CMakeLists.txt file
# to create your own application based on KWWidgets, you can most likely 
# omit this section or change the installation directory, unless you want
# to install your application right where KWWidgets is already installed.

if(KWWidgets_BUILD_EXAMPLES)
  install_targets(${KWWidgets_INSTALL_BIN_DIR} ${EXE_NAME})
endif(KWWidgets_BUILD_EXAMPLES)

# Register this example as a test. Our executable supports a --test
# configuration option so that it can be run non-interactively as a test.
# If you are using this CMakeLists.txt file to create your own application
# based on KWWidgets, you should omit this section, unless your application
# supports that feature too and you checked how the macro is working.

if(BUILD_TESTING AND KWWidgets_BUILD_TESTING)
  include("${KWWidgets_CMAKE_DIR}/KWWidgetsTestingMacros.cmake")
  kwwidgets_add_test_from_c_example(KWWidgets-${PROJECT_NAME} ${EXE_NAME})
endif(BUILD_TESTING AND KWWidgets_BUILD_TESTING)
