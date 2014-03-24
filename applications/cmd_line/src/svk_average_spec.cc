/*
 *  Copyright © 2009-2014 The Regents of the University of California.
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
 *
 */

#include <vtkSmartPointer.h>

#include <svkImageReaderFactory.h>
#include <svkImageReader2.h>
#include <svkImageWriterFactory.h>
#include <svkImageWriter.h>
#include <svkDcmHeader.h>
#include <svkMRSAverageSpectra.h>

#ifdef WIN32
extern "C" {
#include <getopt.h>
}
#else
#include <getopt.h>
#endif


using namespace svk;



int main (int argc, char** argv)
{

    string usemsg("\n") ; 
    usemsg += "Version " + string(SVK_RELEASE_VERSION) + "\n";   
    usemsg += "svk_average_spec -i input_file_name -o output_file_root                      \n"; 
    usemsg += "                  --mask mask_file_name                                      \n"; 
    usemsg += "                   [ -v ]                                                    \n";
    usemsg += "                   [ -h ]                                                    \n";
    usemsg += "                                                                             \n";  
    usemsg += "   -i            input_file_name     Name of file to scale                   \n";
    usemsg += "   -o            output_file_root    Root name of output (no extension)      \n";
    usemsg += "   --mask        mask_file_name      Name of mask file                       \n";
    usemsg += "   -v                                Verbose output                          \n"; 
    usemsg += "   -h                                Print this help mesage.                 \n";  
    usemsg += "\n";  
    usemsg += "Application that averages spectral voxles in the specified ROI.  Exports a   \n"; 
    usemsg += "single voxel data set with average spectrum and voxel size = FOV.            \n"; 
    usemsg += "\n";  

    string inputFileName; 
    string outputFileName;
    string maskFileName; 
    svkImageWriterFactory::WriterType dataTypeOut = svkImageWriterFactory::UNDEFINED;
    bool isVerbose = false; 

    string cmdLine = svkProvenance::GetCommandLineString( argc, argv ); 

    enum FLAG_NAME {
        FLAG_MASK = 0
    };

    static struct option long_options[] =
    {
        {"mask",    required_argument, NULL,  FLAG_MASK},
        {0, 0, 0, 0}
    };


    // ===============================================  
    //  Process flags and arguments
    // ===============================================  
    int i;
    int option_index = 0; 
    while ( ( i = getopt_long(argc, argv, "i:o:hv", long_options, &option_index) ) != EOF) {
        switch (i) {
           case 'i':
                inputFileName.assign( optarg );
                break;
            case 'o':
                outputFileName.assign(optarg);
                break;
            case FLAG_MASK:
                maskFileName.assign( optarg );
                break;
            case 'v':
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

    // ===============================================  
    //  validate input: 
    // ===============================================  
    if ( argc != 0 || inputFileName.length() == 0 || outputFileName.length() == 0 
        || maskFileName.length() == 0 ) {
        cout << usemsg << endl;
        exit(1);
    }

    if( isVerbose ) {
        cout << "INPUT:   " << inputFileName << endl;
        cout << "OUTPUT:  " << outputFileName << endl;
        cout << "MASK:    " << maskFileName  << endl;
    }

    // ===============================================  
    //  Use a reader factory to get the correct reader  
    //  type . 
    // ===============================================  
    vtkSmartPointer< svkImageReaderFactory > readerFactory = vtkSmartPointer< svkImageReaderFactory >::New(); 

    svkImageReader2* mrsReader = readerFactory->CreateImageReader2( inputFileName.c_str() );
    if ( mrsReader == NULL ) {
        cerr << "Can not determine appropriate reader for input: " << inputFileName << endl;
        exit(1);
    }

    svkImageReader2* maskReader = readerFactory->CreateImageReader2( maskFileName.c_str() );
    if ( maskReader == NULL ) {
        cerr << "Can not determine appropriate reader for mask: " << maskFileName << endl;
        exit(1);
    }

    mrsReader->SetFileName( inputFileName.c_str() );
    //mrsReader->Update(); 

    maskReader->SetFileName( maskFileName.c_str() );
    //maskReader->Update(); 

    svkMRSAverageSpectra* avSpec = svkMRSAverageSpectra::New(); 
    avSpec->SetInputConnection(0, mrsReader->GetOutputPort() ); 
    avSpec->SetInputConnection(1, maskReader->GetOutputPort() ); 
    avSpec->Update();


/*    

    

    int numVoxels[3]; 
    svk4DImageData::GetSpatialDimensions( &dimensionVector, numVoxels); 

    svk4DImageData::SetDimensionVectorIndex(&singleVolumeDimVec, svkDcmHeader::SLICE_INDEX, numVoxels[2]); 

    
    int totalNumCells = svkDcmHeader::GetNumberOfCells( &dimensionVector );

    string representation   = refData->GetDcmHeader()->GetStringValue( "DataRepresentation" );
    int numComponents = 1;
    if (representation.compare("COMPLEX") == 0) {
        numComponents = 2;
    }

    //  diff each coil separately
    for ( int channel = 0; channel < numChannels; channel++ ) {
        cout << "diff channel " << channel << endl;

        for( int cellID = 0; cellID < numVoxelsPerChannel; cellID++ ) {

            //  Get the dimensions for the single channel.  reset the channel index and get the 
            //  actual cellID for this channel 
            svkDcmHeader::GetDimensionVectorIndexFromCellID(&channelDimensionVector, &indexVector, cellID); 
            svkDcmHeader::SetDimensionValue(&indexVector, svkDcmHeader::CHANNEL_INDEX, channel);
            int absoluteCellID = svkDcmHeader::GetCellIDFromDimensionVectorIndex(&fullDimensionVector, &indexVector); 

            vtkFloatArray* refSpectrum; 
            vtkFloatArray* testSpectrum; 
            float* refPtr; 
            float* testPtr; 

            refSpectrum = static_cast< vtkFloatArray* >( refData->GetSpectrum( absoluteCellID) );
            testSpectrum = static_cast< vtkFloatArray* >( testData->GetSpectrum( absoluteCellID) );
            refPtr = refSpectrum->GetPointer(0);
            testPtr = testSpectrum->GetPointer(0);
            
            bool cellDiff = false; 
            for (int i = 0; i < numSpecPts * numComponents; i++) {
                testPtr[i] = testPtr[i] - refPtr[i]; 
                if ( isVerbose ) {
                    if (testPtr[i] != 0 ) { 
                        cout << "diff: " <<  i  << " " << testPtr[i] << " - " << refPtr[i] << " = " << testPtr[i] << endl;
                        cellDiff = true; 
                    }
                }
            }
            if ( isVerbose && cellDiff ) {
                cout << "Cell contains diffs: " << absoluteCellID << endl;
            }
        }
    }
*/

    // ===============================================  
    //  Write the data out to the specified file type.  
    //  Use an svkImageWriterFactory to obtain the
    //  correct writer type. 
    // ===============================================  

    //  Get input reader type: 
    if ( mrsReader->IsA("svkDcmMrsVolumeReader") ) {
        cout << "Input DCM MRS " << endl;
        dataTypeOut = svkImageWriterFactory::DICOM_MRS;
    } else if ( mrsReader->IsA("svkDdfVolumeReader") ) {
        cout << "Input DDF " << endl;
        dataTypeOut = svkImageWriterFactory::DDF;
    }

    vtkSmartPointer< svkImageWriterFactory > writerFactory = vtkSmartPointer< svkImageWriterFactory >::New(); 
    svkImageWriter* writer = static_cast<svkImageWriter*>( writerFactory->CreateImageWriter( dataTypeOut ) ); 

    if ( writer == NULL ) {
        cerr << "Can not determine writer of type: " << dataTypeOut << endl;
        exit(1);
    }

    writer->SetFileName( outputFileName.c_str() );
    writer->SetInput( svkMrsImageData::SafeDownCast( avSpec->GetOutput()) );

    // ===============================================  
    //  Set the input command line into the data set 
    //  provenance: 
    // ===============================================  
    avSpec->GetOutput()->GetProvenance()->SetApplicationCommand( cmdLine );

    // ===============================================  
    //  Write data to file: 
    // ===============================================  
    writer->Write();

    // ===============================================  
    //  Clean up: 
    // ===============================================  
    writer->Delete();
    mrsReader->Delete();
    maskReader->Delete();

    return 0; 
}

