# SIVIC

This is an early stage relese of SIVIC.  All comments/questions are welcome.  This is a community project and 
participation is encouraged.  Please see the following links for further information. 

1.  General help and project information:

    http://sourceforge.net/projects/sivic/

    http://sourceforge.net/apps/trac/sivic/


2. The Doxygen C++ API for SIVIC is published on Sourceforge:

    http://sivic.sourceforge.net/libsvk/html/index.html


3.  Mail List
    
    https://lists.sourceforge.net/lists/listinfo/sivic-users
    

NOTES FOR WINDOWS USERS:
    * To run sivic simply double click on sivic_windows/bin/sivic.bat, or run from the msdos prompt.
    * This version of sivic should run on Windows Vista, Windows 7, and Windows 2003 Server.
    * Some features are disabled in the Windows version (using the "exam" button and the quantification demo). These features should be available soon.
    * Command line tools are not yet available, but also should be soon.
    * Please contact us via the sourceforge site above if you have any problems.

## HOW TO BUILD SIVIC
### BUILDING DEPENDENCIES    
#### This was Built using CentOS 7.9 ( GCC 4.8.5 / GLIBC 2.17). Build is forward compatible with RHEL8 and RHEL9. Needs to build on a redhat 7 sever .

#### 1.Build and install TCL and TK packages
https://sourceforge.net/projects/tcl/files/Tcl/8.5.19/tcl8.5.19-src.tar.gz/download  
https://sourceforge.net/projects/tcl/files/Tcl/8.5.19/tk8.5.19-src.tar.gz/download  
* Open up terminal and CD into TCL
* cd into unix director
* ./configure --prefix=/usr/local/sivic/build/deps/tcl8.5.19 --enable-shared
* Make -j
* Make install
* Open up terminal and CD into TK
* cd into unix directory
* ./configure --with-x --prefix=/usr/local/sivic/build/deps/tk8.5.19 --with-tcl=/usr/local/sivic/build/deps/tcl8.5.19/lib --enable-shared
* make -j
* make install

#### 2.Build and install DCMTK 3.6.7  
https://dicom.offis.de/download/dcmtk/dcmtk367/dcmtk-3.6.7.tar.gz
* Open up terminal and CD into DCMTK
* cmake .
* ccmake .
  - BUILD_SHARED_LIBS=ON
  - CMAKE_INSTALL_PREFIX=/usr/local/sivic/build/deps/dcmtk-3.6.7
* make -j
* make install


#### 3.Build and install ITK 4.4.1
https://github.com/InsightSoftwareConsortium/ITK/archive/refs/tags/v4.4.1.tar.gz
*	Open up terminal and CD into ITK
*	mkdir ITK-Sandbox
*	cd ITK-sandbox
*	git clone https://github.com/InsightSoftwareConsortium/ITK.git
*	mkdir ITK-build
*	cd ITK-build
*	cmake .
*	ccmake . 
      - BUILD_EXAMPLES=OFF
      - BUILD_SHARED_LIBS=ON
      - BUILD_TESTING=OFF
      - CMAKE_INSTALL_PREFIX=/usr/local/sivic/build/deps/ITK-4.4.1
* Make 
* Make install

#### 4.Build and install clapack 3.2.1
https://www.netlib.org/clapack/clapack-3.2.1-CMAKE.tgz
* Open up terminal and cd into clapack
*	cmake .
*	ccmake .
        - BUILD_TESTING=OFF
        - CMAKE_INSTALL_PREFIX=/usr/local/sivic/build/deps/clapack-3.2.1
        - USE_BLAS_WRAP=ON
* Make 
* Make install
*	bash-4.2$ cd /usr/local/sivic/build/deps/clapack-3.2.1/lib
*	bash-4.2$ ln -s liblapack.a libclapack.a
*	bash-4.2$ ln -s libblas.a libcblas.a

#### 5.Build and install VTK 6.3.0 
https://gitlab.kitware.com/vtk/vtk/-/archive/v6.3.0/vtk-v6.3.0.tar.gz
* Open up terminal and cd into VTK]
* cmake .
* ccmake ..
  - BUILD_SHARED_LIBS=ON
  - BUILD_TESTING=OFF
  - CMAKE_INSTALL_PREFIX=/usr/local/sivic/build/deps/VTK-6.3.0
  - VTK_Group_Tk=ON
  - VTK_USE_TK=ON
  - VTK_TCL_TK_STATIC=ON
  - Module_vtkWrappingTcl=ON
   - Module_vtkTclTk=ON
   - TCL_LIBRARY=/usr/local/sivic/build/deps/tcl8.5.19/lib/libtcl8.5.so
   - TCL_TCLSH=/usr/local/sivic/build/deps/tcl8.5.19/bin/tclsh8.5
   - TK_INCLUDE_PATH=/usr/local/sivic/build/deps/tk8.5.19/include
  - TK_LIBRARY=/usr/local/sivic/build/deps/tk8.5.19/lib/libtk8.5.so
* Make
* Make install

#### 6.set environment variables for DCMTK_DIR and VTK_DIR
* export DCMTK_DIR=/usr/local/sivic/build/deps/dcmtk-3.6.7
* export VTK_DIR=/usr/local/sivic/build/deps/VTK-6.3.0

### 7.Build and Install KWWidgets 
* git@github.com:SIVICLab/KWWidgets.git
* open up and terminal
* CD into KWWidgets
* cmake .
* cmake ..
  - BUILD_TESTING = OFF
  - CMAKE_INSTALL_PREFIX=/usr/local/sivic/build/deps/KWWidgets
  - VTK_DIR=/usr/local/sivic/build/deps/VTK-6.3.0/lib/cmake/vtk-6.3
  - TCL_INCLUDE_PATH=/usr/local/sivic/build/deps/tcl8.5.19/include
  - TCL_LIBRARY=/usr/local/sivic/build/deps/tcl8.5.19/lib/libtcl8.5.so
  - TCL_TCLSH=/usr/local/sivic/build/deps/tcl8.5.19/bin/tclsh8.5
  - TK_INCLUDE_PATH=/usr/local/sivic/build/deps/tk8.5.19/include
  - TK_LIBRARY=/usr/local/sivic/build/deps/tk8.5.19/lib/libtk8.5.so
* make
* make install

#### 8.Build and install openssl
https://github.com/openssl/openssl/archive/refs/tags/openssl-3.0.7.tar.gz
* 	../configure --prefix=/usr/local/sivic/build/deps/openssl-openssl-3.0.7
* make -j 4
* make install
* cd /usr/local/sivic/build/deps/openssl-openssl-3.0.7
* ln -s lib64 lib

### BUILDING SIVIC
* In redhat 7 server, open up terminal,
* mkdir sivic-rh9
* Git clone https://github.com/SIVICLab/sivic.git
* mkdir lnx68
* cd lnx68
* ccmake ..
* press [c] to configure
  - BUILD_APPS=ON
  - BUILD_CREATE_RAW=ON
  - BUILD_CLAPACK=ON
  - BUILD_ITK = ON
  - BUILD_LIBRARY = ON
  - BUILD_VIZ_LIBRARY = ON 
  - CLAPACK_DIR=/usr/local/sivic/build/deps/clapack-3.2.1/lib
  - CMAKE_INSTALL_PREFIX=/usr/local/sivic/sivic
  - DCMTK_DIR=/usr/local/sivic/build/deps/dcmtk-3.6.7
  - ITK_DIR=/sivic/build/deps/ITK-4.4.1/lib/cmake/ITK-4.4 
  - KWWidgets DIR =/usr/local/sivic/build/deps/KWWidgets/lib/KWWidgets
  - VTK_DIR=/usr/local/sivic/build/deps/VTK-6.3.0/lib/cmake/vtk-6.3
  - OPENSSL_DIR=/usr/local/sivic/build/deps/openssl-openssl-3.0.7
  - UCSF_INTERNAL=ON
* Make
* Make install


## Citations

SIVIC.  Available online at: https://sourceforge.net/p/sivic/sivicwiki/Home/  DOI: [10.5281/zenodo.4777197](https://doi.org/10.5281/zenodo.4777197)

Crane, Jason C., Marram P. Olson, and Sarah J. Nelson. “SIVIC: Open-Source, Standards-Based Software for DICOM MR Spectroscopy Workflows.” International Journal of Biomedical Imaging 2013 (July 18, 2013): e169526. https://doi.org/10.1155/2013/169526.
