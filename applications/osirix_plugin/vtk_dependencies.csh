#!/bin/csh

#       Needs to be run before creating package.  pkg will include the vtk-5.6 generated directory contents
#
# ~/Library/Application Support/OsiriX/Plugins/SIVIC_MRSI.osirixplugin/Contents/Resources>otool -l sivic | grep -i vt
#

mkdir ./vtk-5.6

cp -Rp /usr/local/lib/vtk-5.6/*Common* ./vtk-5.6
cp -Rp /usr/local/lib/vtk-5.6/*DICOMParser* ./vtk-5.6
cp -Rp /usr/local/lib/vtk-5.6/*Filtering* ./vtk-5.6
cp -Rp /usr/local/lib/vtk-5.6/*Graphics* ./vtk-5.6
cp -Rp /usr/local/lib/vtk-5.6/*Hybrid* ./vtk-5.6
cp -Rp /usr/local/lib/vtk-5.6/*IO* ./vtk-5.6
cp -Rp /usr/local/lib/vtk-5.6/*Imaging* ./vtk-5.6
cp -Rp /usr/local/lib/vtk-5.6/*NetCDF* ./vtk-5.6
cp -Rp /usr/local/lib/vtk-5.6/*Rendering* ./vtk-5.6
cp -Rp /usr/local/lib/vtk-5.6/*VolumeRendering* ./vtk-5.6
cp -Rp /usr/local/lib/vtk-5.6/*Widgets* ./vtk-5.6
cp -Rp /usr/local/lib/vtk-5.6/*exoIIc* ./vtk-5.6
cp -Rp /usr/local/lib/vtk-5.6/*expat* ./vtk-5.6
cp -Rp /usr/local/lib/vtk-5.6/*freetype* ./vtk-5.6
cp -Rp /usr/local/lib/vtk-5.6/*ftgl* ./vtk-5.6
cp -Rp /usr/local/lib/vtk-5.6/*jpeg* ./vtk-5.6
cp -Rp /usr/local/lib/vtk-5.6/*metaio* ./vtk-5.6
cp -Rp /usr/local/lib/vtk-5.6/*png* ./vtk-5.6
cp -Rp /usr/local/lib/vtk-5.6/*sqlite* ./vtk-5.6
cp -Rp /usr/local/lib/vtk-5.6/*sys* ./vtk-5.6
cp -Rp /usr/local/lib/vtk-5.6/*tiff* ./vtk-5.6
cp -Rp /usr/local/lib/vtk-5.6/*verdict* ./vtk-5.6
cp -Rp /usr/local/lib/vtk-5.6/*zlib* ./vtk-5.6

rm ./vtk-5.6/*Python*
rm ./vtk-5.6/*cmake
