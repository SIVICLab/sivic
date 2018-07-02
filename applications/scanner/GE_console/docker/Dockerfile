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

################################################
#    get VNC running
################################################
#RUN yum groupinstall "GNOME Desktop"
#RUN yum groupinstall "Desktop"
#RUN yum install tigervnc-server
#RUN  yum install x11vnc 
#RUN  yum install x11-apps
#RUN  yum install xvfb 
#run  yum install firefox
#RUN  mkdir ~/.vnc
RUN yum install -y --setopt=tsflags=nodocs \
                   tigervnc-server \
               xorg-x11-server-utils \
                   xorg-x11-server-Xvfb \
                   xorg-x11-fonts-* \
                   xterm && \
                   yum clean all && \
                   rm -rf /var/cache/yum
RUN yum install -y --setopt=tsflags=nodocs \
                  openmotif \
                  xterm \
                  firefox \
                  yum clean all && \
                  rm -rf /var/cache/yum/*

RUN /bin/dbus-uuidgen --ensure
RUN useradd -u 1001 -r -g 0 -d ${HOME} -s /bin/bash -c "Kiosk User" kioskuser

ADD xstartup ${HOME}/.vnc/
ARG vncpassword=password
RUN echo "${vncpassword}" | vncpasswd -f > ${HOME}/.vnc/passwd
# RUN /bin/echo "/usr/bin/firefox" >> /home/1001/.vnc/xstartup
RUN touch /home/1001/.Xauthority

RUN chown -R 1001:0 ${HOME} && \
    chmod 775 ${HOME}/.vnc/xstartup && \
    chmod 600 ${HOME}/.vnc/passwd

EXPOSE 5901
WORKDIR ${HOME}
USER 1001
ENTRYPOINT ["/usr/bin/vncserver","-fg"]
################################################

RUN ln -sf /usr/lib64/libfontconfig.so.1.4.4 /usr/lib64/libfontconfig.so 
RUN ln -sf /usr/lib64/libXss.so.1 /usr/lib64/libXss.so
RUN ln -sf /usr/lib64/libXft.so.2 /usr/lib64/libXft.so
ENV PATH "$PATH:./"

#   local source files
#RUN curl  -o ./freeware/VTK-6.3.0.tar.gz https://www.vtk.org/files/release/6.3/VTK-6.3.0.tar.gz 
#RUN curl  -o ./freeware/tk8.5.10.tar.gz   ftp://ftp.tcl.tk/pub/tcl/tcl8_5/tk8.5.10-src.tar.gz 
#RUN curl  -o ./freeware/tcl8.5.10.tar.gz  ftp://ftp.tcl.tk/pub/tcl/tcl8_5/tcl8.5.10-src.tar.gz 
#   cd freeware; git clone https://github.com/SIVICLab/KWWidgets.git
#   cd freeware; git clone https://github.com/SIVICLab/sivic.git
#   scp -r jasonc@atom:/netopt/InsightToolkit/InsightToolkit-4.4.1 ./netopt/InsightToolkit/
#   scp -r jasonc@atom:/netopt/dicom  ./netopt/
#   scp -r jasonc@atom:/opt/src/freeware/dcmtk-3.6.0 freeware 
#   mkdir -p ./opt/src/freeware/clapack/
#   scp -r jasonc@atom:/opt/src/freeware/clapack/clapack-3.2.1-CMAKE ./opt/src/freeware/clapack/

# Default command
CMD ["/bin/bash"]
