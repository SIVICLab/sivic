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
    
SIVIC RHEL 9
How to build:

git clone git@github.com:SIVICLab/sivic.git
Make sure it's sivic_redhat 9 branch
cd /home/erdiaz/Desktop/sivic_next/sivic-sivic_redhat9/build3
ccmake .. gives you access to Cmake Configurations
Currently Cmake Build Type is in DEBUG, change it to None or just leave it as Debug
Press C to configure, then g to generate
do make
then make install
The options to the command lines is Build Apps = ON, Build_Libs = On, and Build_Viz_LIb = ON.
NOTES FOR WINDOWS USERS:


## Citations

SIVIC.  Available online at: https://sourceforge.net/p/sivic/sivicwiki/Home/  DOI: [10.5281/zenodo.4777197](https://doi.org/10.5281/zenodo.4777197)

Crane, Jason C., Marram P. Olson, and Sarah J. Nelson. “SIVIC: Open-Source, Standards-Based Software for DICOM MR Spectroscopy Workflows.” International Journal of Biomedical Imaging 2013 (July 18, 2013): e169526. https://doi.org/10.1155/2013/169526.
