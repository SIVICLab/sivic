# SIVIC M1 BUILD Instructions
## BUILD_LIBRARY ONLY

### Install X11
1. Download [Homebrew](https://brew.sh/)
2. Brew Install libx11

### Build and Install TCL
1. Download [TCL](https://sourceforge.net/projects/tcl/files/Tcl/8.5.19/tcl8.5.19-src.tar.gz/download)
2. Open up terminal and CD into TCL
3. CD into Unix folder
4. Put this in terminal export ac_cv_func_strtod=yes
5. Put this in terminal export tcl_cv_strtod_buggy=1
6. ./configure --prefix=/Users/ernestodiaz/Desktop/sivic-master/build/deps/tcl8.5.19 --enable-shared
7.  change typedef int ptrdiff_t in tclInt.h to typedef long ptrdiff_t
8.  make
9.  make install

### Build and Install TK
1. Download [TK](https://sourceforge.net/projects/tcl/files/Tcl/8.5.19/tk8.5.19-src.tar.gz/download)
2. Comment out the whole file fixstrtod.c
3. Change line 54 in tclint.h from ‘typedef int ptrdiff_t’ to typedef long ptrdiff_t
4. Open up terminal and CD into TK
5. CD into Unix folder
6. 	./configure \
	--with-x \
	--prefix=/usr/local/sivic/build/deps/tk8.5.19 \
	--with-tcl=/usr/local/sivic/build/deps/tcl8.5.19/lib \
	--enable-shared \
	--x-includes="/opt/homebrew/Cellar/libx11/1.8.10/include/" \
	--x-libraries=/opt/homebrew/Cellar/libx11/1.8.10/lib/
7. make
8. make install

### Build and Install DCMTK
1. Download [DCMTK](https://dicom.offis.de/download/dcmtk/dcmtk367/dcmtk-3.6.7.tar.gz)
2. Open up terminal and CD into DCMTK
3. In the terminal cmake .
4. In the terminal ccmake .
   - BUILD_SHARED_LIBS=ON
   - CMAKE_INSTALL_PREFIX=/usr/local/sivic/build/deps/dcmtk-3.6.7
5. make
6. make install

### Build and Install VTK
1. Download [VTK](https://gitlab.kitware.com/vtk/vtk/-/archive/v6.3.0/vtk-v6.3.0.tar.gz)
2. Open up terminal and CD into VTK
3. cmake .
4. ccmake .
   - BUILD_SHARED_LIBS=ON
   - BUILD_TESTING=OFF
   - CMAKE_INSTALL_PREFIX=/usr/local/sivic/build/deps/VTK-6.3.0
   - VTK_Group_Tk=ON
   - VTK_USE_TK=ON
   - VTK_WRAP_TCL =ON
   - Module_vtkWrappingTcl=ON
   - Module_vtkTclTk=ON
   - TCL_LIBRARY=/usr/local/sivic/build/deps/tcl8.5.19/lib/libtcl8.5.so
   - TCL_TCLSH=/usr/local/sivic/build/deps/tcl8.5.19/bin/tclsh8.5
   - TK_INCLUDE_PATH=/usr/local/sivic/build/deps/tk8.5.19/include
   - TK_LIBRARY=/usr/local/sivic/build/deps/tk8.5.19/lib/libtk8.5.so
6. Change ftmac.c to remove deprecated macos APIs ATSFontGetFileReference & ATSFontFindFromName
7. make
8. make install<br/>

Other notes(might need it or not):

Source Code Fixes
1.	In H5LT.c
    1. Add function prototypes (There may be another solution to this as well)
    - int snprintf ( char * s, size_t n, const char * format, ... );
	  - char * strdup( const char *str1 );
2.	In H5LTanalyze.c
    1. Add function prototype
    - char * strdup( const char *str1 );
3.	In H5LTparse.c
    1.	Add function prototype
    - char * strdup( const char *str1 );
4.	In H5TB.c
     1.	Add function prototypes
     - int snprintf ( char * s, size_t n, const char * format, ... );
5.	In vtk-v6.3.0/Rendering/OpenGL/CMakeLists.txt
    1. Add vtk_module_link_libraries(vtkRenderingOpenGL LINK_PUBLIC "-framework OpenGL")
    2. Under VTK_USE_X
6. In vtk-v6.3.0/Wrapping/Tcl/CMakeLists.txt
   1. Add /opt/X11/include to include path (This is a hack – Need to further understand this issue)

### Building SIVIC
1. Download [SIVIC](https://github.com/SIVICLab/sivic)
2. Open up termianl
3. CD into SIVIC
4. mkdir Build/Dep inside SIVIC
5. cmake .
6. ccmake 
   - BUILD_APPS=OFF
   - BUILD_CREATE_RAW=OFF
   - BUILD_CLAPACK=OFF
   - BUILD_ITK = OFF
   - BUILD_LIBRARY = ON
   - BUILD_VIZ_LIBRARY = OFF
   - CMAKE_INSTALL_PREFIX=/usr/local/sivic/sivic
   - DCMTK_DIR=/usr/local/sivic/build/deps/dcmtk-3.6.7
   - VTK_DIR=/usr/local/sivic/build/deps/VTK-6.3.0/lib/cmake/vtk-6.3
7. Make
8. Make install

## BUILDING ALL OF SIVIC (UNTESTED)   

### Build and Install ITK
1. Download [ITK](https://github.com/InsightSoftwareConsortium/ITK/archive/refs/tags/v4.4.1.tar.gz)
2. CD into ITK
3. mkdir ITK-Sandbox
4. CD ITK-Sandbox
5. git clone https://github.com/InsightSoftwareConsortium/ITK.git
6. mkdir ITK-build
7. cmake .
8. ccmake ..
   - press t for advance mode
   - BUILD_EXAMPLES=OF  
   - BUILD_SHARED_LIBS=ON -BUILD_TESTING=OFF  
   - CMAKE_INSTALL_PREFIX=/usr/local/sivic/build/deps/ITK-4.4.1  
9. make
10. make install<br/>

Other notes (might need it or not)

- ITK_USE_SYSTEM_TIFF = ON
- CFLAGS: -std=c11
- CXXFLAGS: -std:c++11

Source Code Fixes
1. ITK-4.4.1/Modules/ThirdParty/VNL/src/vxl/v3p/netlib/linalg/lsqrBase.cxx
   - Comparison between pointer and 0 (Changed to NULL)
2. Need to fix ITK-4.4.1/Modules/Core/Common/src/itkProcessObject.cxx
   - Exact changes still need to be understood
3. Need to fix ITK-4.4.1/Modules/Numerics/Optimizers/src/itkCumulativeGaussianOptimizer.cxx
   - Changes to stream to stream.str()
4. Need to fix  ITK-4.4.1/Modules/Core/Common/include/itkImageAlgorithm.h
   - TrueType() issue

### Build and Install libtiff
1. git clone https://gitlab.com/libtiff/libtiff
2. git checkout 4.5.1
3. cmake .
4. ccmake .
   - CMAKE_INSTALL_PREFIX
5. make
6. make install

### Build and Install OpenSSl
1. Download [OpenSSl](https://github.com/openssl/openssl/archive/refs/tags/openssl-3.0.7.tar.gz)
2. ../configure --prefix=/usr/local/sivic/build/deps/openssl-openssl-3.0.7 
3. make
4. make install
5. cd /usr/local/sivic/build/deps/openssl-openssl-3.0.7
6. ln -s lib64 lib



