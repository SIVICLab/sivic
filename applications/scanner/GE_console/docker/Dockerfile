FROM centos:6.8 
MAINTAINER The CentOS Project <cloud-ops@centos.org>

RUN yum groupinstall "Development Tools" -y
RUN yum install cmake -y
RUN yum install xorg-x11-xauth xterm -y
RUN yum groupinstall "X Window System" -y
RUN yum install libX11-devel -y
RUN yum install freeglut-devel -y
RUN yum install libXt-devel -y
#RUN yum -y install hdf5 -y
RUN yum install zlib-devel  -y
RUN yum install zlib -y
RUN yum install fontconfig -y
RUN yum install libXScrnSaver -y


#RUN curl  -o ./freeware/VTK-6.3.0.tar.gz https://www.vtk.org/files/release/6.3/VTK-6.3.0.tar.gz 
#RUN curl  -o ./freeware/tk8.5.2.tar.gz   ftp://ftp.tcl.tk/pub/tcl/tcl8_5/tk8.5.2-src.tar.gz 
#RUN curl  -o ./freeware/tcl8.5.2.tar.gz  ftp://ftp.tcl.tk/pub/tcl/tcl8_5/tcl8.5.2-src.tar.gz 
#   cd freeware; git clone https://github.com/SIVICLab/KWWidgets.git
#   cd freeware; git clone https://github.com/SIVICLab/sivic.git
#   scp -r jasonc@atom:/netopt/InsightToolkit/InsightToolkit-4.4.1 ./netopt/InsightToolkit/
#   scp -r jasonc@atom:/netopt/dicom  ./netopt/
#   scp -r jasonc@atom:/opt/src/freeware/dcmtk-3.6.0 freeware 
#   mkdir -p ./opt/src/freeware/clapack/
#   scp -r jasonc@atom:/opt/src/freeware/clapack/clapack-3.2.1-CMAKE ./opt/src/freeware/clapack/

# Default command
CMD ["/bin/bash"]
