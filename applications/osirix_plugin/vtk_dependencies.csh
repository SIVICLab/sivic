#!/bin/csh

#       Needs to be run before creating package.  pkg will include the vtk-5.4 generated directory contents
#
# ~/Library/Application Support/OsiriX/Plugins/SIVIC_MRSI.osirixplugin/Contents/Resources>otool -l sivic | grep -i vt
#

mkdir ./vtk-5.4

cp -Rp /usr/local/lib/vtk-5.4/*Common* ./vtk-5.4
cp -Rp /usr/local/lib/vtk-5.4/*DICOMParser* ./vtk-5.4
cp -Rp /usr/local/lib/vtk-5.4/*Filtering* ./vtk-5.4
cp -Rp /usr/local/lib/vtk-5.4/*Graphics* ./vtk-5.4
cp -Rp /usr/local/lib/vtk-5.4/*Hybrid* ./vtk-5.4
cp -Rp /usr/local/lib/vtk-5.4/*IO* ./vtk-5.4
cp -Rp /usr/local/lib/vtk-5.4/*Imaging* ./vtk-5.4
cp -Rp /usr/local/lib/vtk-5.4/*NetCDF* ./vtk-5.4
cp -Rp /usr/local/lib/vtk-5.4/*Rendering* ./vtk-5.4
cp -Rp /usr/local/lib/vtk-5.4/*VolumeRendering* ./vtk-5.4
cp -Rp /usr/local/lib/vtk-5.4/*Widgets* ./vtk-5.4
cp -Rp /usr/local/lib/vtk-5.4/*exoIIc* ./vtk-5.4
cp -Rp /usr/local/lib/vtk-5.4/*expat* ./vtk-5.4
cp -Rp /usr/local/lib/vtk-5.4/*freetype* ./vtk-5.4
cp -Rp /usr/local/lib/vtk-5.4/*ftgl* ./vtk-5.4
cp -Rp /usr/local/lib/vtk-5.4/*jpeg* ./vtk-5.4
cp -Rp /usr/local/lib/vtk-5.4/*metaio* ./vtk-5.4
cp -Rp /usr/local/lib/vtk-5.4/*png* ./vtk-5.4
cp -Rp /usr/local/lib/vtk-5.4/*sqlite* ./vtk-5.4
cp -Rp /usr/local/lib/vtk-5.4/*sys* ./vtk-5.4
cp -Rp /usr/local/lib/vtk-5.4/*tiff* ./vtk-5.4
cp -Rp /usr/local/lib/vtk-5.4/*verdict* ./vtk-5.4
cp -Rp /usr/local/lib/vtk-5.4/*zlib* ./vtk-5.4

rm ./vtk-5.4/*Python*
rm ./vtk-5.4/*cmake
