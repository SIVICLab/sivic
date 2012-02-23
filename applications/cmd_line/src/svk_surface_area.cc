/*
 *  Copyright © 2009-2011 The Regents of the University of California.
 *  All Rights Reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *  •   Redistributions of source code must retain the above copyright notice,
 *      this list of conditions and the following disclaimer.
 *  •   Redistributions in binary form must reproduce the above copyright notice,
 *      this list of conditions and the following disclaimer in the documentation
 *      and/or other materials provided with the distribution.
 *  •   None of the names of any campus of the University of California, the name
 *      "The Regents of the University of California," or the names of any of its
 *      contributors may be used to endorse or promote products derived from this
 *      software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 *  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 *  IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 *  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 *  NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 *  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 *  WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 *  OF SUCH DAMAGE. 
 */     


/*
 *  $URL$
 *  $Rev$
 *  $Author$
 *  $Date$
 *
 *  Authors:
 *      Jason C. Crane, Ph.D.
 *      Beck Olson
 *
 */


#include <vtkSmartPointer.h>
#include <vtkImageToPolyDataFilter.h>
#include <vtkTriangleFilter.h>
#include <vtkPolyDataWriter.h>
#include <vtkXMLPolyDataWriter.h> 
#include <vtkContourFilter.h>
#include <vtkMassProperties.h>


#include <svkImageReaderFactory.h>
#include <svkImageReader2.h>
#include <svkImageWriterFactory.h>
#include <svkImageWriter.h>
#include <svkIdfVolumeWriter.h>
#include <svkIntegratePeak.h>
#include <svkMetaboliteMap.h>
#include <svkQuantifyMetabolites.h>
#include <svkLookupTable.h>

#ifdef WIN32
extern "C" {
#include <getopt.h>
}
#else
#include <getopt.h>
#endif


#define UNDEFINED_VAL -99999


using namespace svk;


int main (int argc, char** argv)
{

    string usemsg("\n") ; 
    usemsg += "Version " + string(SVK_RELEASE_VERSION) + "\n";   
    usemsg += "svk_surface_area -i input_file_name  \n";
    usemsg += "             [--verbose ] [ -h ]                                             \n"; 
    usemsg += "\n";  
    usemsg += "   -i input_file_name        name of file to convert.            \n"; 
    usemsg += "   -o output_file_name       name of output polydata file.            \n"; 
    usemsg += "   -t threshold              contour threshold .            \n"; 
    usemsg += "   --verbose                 Prints pk ht and integrals for each voxel to stdout. \n"; 
    usemsg += "   -h                        print help mesage.                  \n";  
    usemsg += " \n";  
    usemsg += "Calculates the surface area of an ROI.  Returns the surface area and the surface/volume ratio\n";
    usemsg += "\n";  

    string inputFileName; 
    string outputFileName; 
    int    contourThreshold = 500;  
    bool   isVerbose = false;   

    string cmdLine = svkProvenance::GetCommandLineString( argc, argv );

    enum FLAG_NAME {
        FLAG_VERBOSE  
    };


    static struct option long_options[] =
    {
        /* This option sets a flag. */
        {"verbose",          no_argument      , NULL,  FLAG_VERBOSE},
        {0, 0, 0, 0}
    };


    // ===============================================
    //  Process flags and arguments
    // ===============================================
    int i;
    int option_index = 0;
    while ( ( i = getopt_long(argc, argv, "i:o:t:h", long_options, &option_index) ) != EOF) {
        switch (i) {
            case 'i':
                inputFileName.assign( optarg );
                break;
            case 'o':
                outputFileName.assign( optarg );
                break;
            case 't':
                contourThreshold = atoi( optarg);
                break;
           case FLAG_VERBOSE:
                isVerbose = true; 
                break;
            case 'h':
                cout << usemsg << endl;
                exit(1);  
                break;
            default:
                ;
        }
    }

    argc -= optind;
    argv += optind;


    if ( argc != 0 ) {
        cout << usemsg << endl;
        exit(1); 
    }
  
    
    if ( inputFileName.length() == 0  || outputFileName.length() == 0 ) { 
        cout << usemsg << endl;
        exit(1); 
    }
  
    cout << inputFileName << endl;

    // ===============================================
    //  Use a reader factory to get the correct reader
    //  type .
    // ===============================================
    vtkSmartPointer< svkImageReaderFactory > readerFactory = vtkSmartPointer< svkImageReaderFactory >::New();
    svkImageReader2* reader = readerFactory->CreateImageReader2(inputFileName.c_str());

    if (reader == NULL) {
        cerr << "Can not determine appropriate reader for: " << inputFileName << endl;
        exit(1);
    }

    // ===============================================
    //  Use the reader to read the data into an
    //  svkMrsImageData object and set any reading
    //  behaviors (e.g. average suppressed data).
    // ===============================================
    reader->SetFileName( inputFileName.c_str() );
    reader->Update(); 

    vtkContourFilter* cont = vtkContourFilter::New();
    cont->SetInputConnection( reader->GetOutputPort() );
    cont->SetValue(0, contourThreshold);
/*
    vtkImageToPolyDataFilter* i2pd = vtkImageToPolyDataFilter::New();
    i2pd->SetInputConnection(cont->GetOutputPort()); 
    svkLookupTable* lut = svkLookupTable::New(); 
    lut->SetLUTType(  svkLookupTable::GREY_SCALE ); 
    i2pd->SetLookupTable ( lut ); 
    i2pd->SetColorModeToLUT();
    i2pd->SetOutputStyleToPolygonalize();
    i2pd->SetError( 0 ); 
    i2pd->DecimationOn();
    i2pd->SetDecimationError(0.0);
    i2pd->SetSubImageSize(5);
*/

    //Need a triangle filter because the polygons are complex and concave
    vtkTriangleFilter* tf = vtkTriangleFilter::New();
    tf->SetInputConnection(cont->GetOutputPort());

    vtkSmartPointer<vtkXMLPolyDataWriter> writer =  
        vtkSmartPointer<vtkXMLPolyDataWriter>::New();

    vtkMassProperties* mass = vtkMassProperties::New();
    mass->SetInput( tf->GetOutput() ); 
    double vol = mass->GetVolume(); 
    //  vol = ( 4./3.) * vtkMath::Pi() * r3
    
    double radius = ( vol * 3. ) / (4. * vtkMath::Pi() ) ; 

    radius = pow (radius, 1./3.); 

    double surfaceArea = mass->GetSurfaceArea(); 

    double sphericalSurfaceArea = 4 * vtkMath::Pi() * radius * radius; 

    cout << "VOLUME:                    " << vol << endl;
    cout << "RADIUS:                    " << radius<< endl;
    cout << "SURFACE:                   " << surfaceArea << endl;
    cout << "SURFACE/SPHERICAL_SURFACE: " << surfaceArea/sphericalSurfaceArea << endl;

    writer->SetInputConnection( tf->GetOutputPort() ); 
    //writer->SetInputConnection( cont->GetOutputPort() ); 
    //writer->SetInputConnection( i2pd->GetOutputPort() ); 
    //writer->SetInputConnection( tf->GetOutputPort() ); 
    writer->SetFileName( outputFileName.c_str() );
    writer->Write();

    writer->Delete();
    //tf->Delete();
    //i2pd->Delete();
    reader->Delete();

    return 0; 
}



