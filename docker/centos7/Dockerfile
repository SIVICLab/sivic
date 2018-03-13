FROM centos:7
ENV container docker
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
RUN mkdir /netopt
RUN mkdir /netopt/tcltk
RUN mkdir -p /opt/src/freeware/clapack
ADD xsede_volume/library /netopt
ADD xsede_volume/tcltk /netopt/tcltk
RUN ln -s /netopt/clapack-3.2.1-CMAKE /opt/src/freeware/clapack/clapack-3.2.1-CMAKE
RUN ln -s /usr/lib64/libXss.so.1 /usr/lib64/libXss.so
RUN ln -s /usr/lib64/libpython2.7.so.1.0 /usr/lib64/libpython2.7.so
RUN ln -s /usr/lib64/libXft.so.2 /usr/lib64/libXft.so
RUN ln -s /usr/lib64/libfontconfig.so.1 /usr/lib64/libfontconfig.so
ENV DCMDICTPATH /netopt/dicom/share/dcmtk/dicom.dic
RUN mkdir -p /HMTRC/development_tutorial
ADD development_tutorial /HMTRC/development_tutorial
ENV PATH="/sivic/applications/cmd_line/Linux_x86_64/:${PATH}"
CMD ["/usr/sbin/init"]
