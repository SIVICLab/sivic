# SIVIC RHEL 9

 How to build:
 1. git clone git@github.com:SIVICLab/sivic.git
 2. Make sure it's sivic_redhat 9 branch
 3. cd /home/erdiaz/Desktop/sivic_next/sivic-sivic_redhat9/build3
 4. ccmake .. gives you access to Cmake Configurations
 5. Currently Cmake Build Type is in DEBUG, change it to None or just leave it as Debug
 6. Press C to configure, then g to generate
 7. do make
 8. then make install
 9. The options to the command lines is Build Apps = ON, Build_Libs = On, and Build_Viz_LIb = ON.
    
