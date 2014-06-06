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
 *  Utility application for converting between supported file formats. 
 *
 */

#ifdef WIN32
extern "C" {
#include <getopt.h>
}
#else
#include <getopt.h>
#include <unistd.h>
#endif
#include <svkImageReaderFactory.h>
#include <svkImageReader2.h>
#include <svkImageWriter.h>
#include <svkImageWriterFactory.h>
#include <svkImageMathematics.h>
#include <vtkImageThreshold.h>
#include <vtkImageAccumulate.h>


using namespace svk;


int main (int argc, char** argv)
{

    string usemsg("\n") ; 
    usemsg += "Version " + string(SVK_RELEASE_VERSION) + "\n";   
    usemsg += "svk_volume_diff --i1 input_file_name --i2 input_file_name                    \n"; 
    usemsg += "                -o output_file_root                                          \n"; 
    usemsg += "                                                                             \n";  
    usemsg += "   --i1          input_file_name     Name of file1                           \n"; 
    usemsg += "   --i2          input_file_name     Name of file2                           \n"; 
    usemsg += "   --dice                            Compute the Dice similarity coefficient.\n"; 
    usemsg += "   -o            output_file_root    Root name of output (no extension)      \n";  
    usemsg += "   -v                                Verbose output.                         \n";
    usemsg += "   -h                                Print help mesage.                      \n";  
    usemsg += "                                                                             \n";  
    usemsg += "Diffs two volumes                                                            \n";  
    usemsg += "                                                                             \n";  

    string input1FileName; 
    string input2FileName; 
    string outputFileName; 
    bool   verbose = false;
    bool   computeDice = false;
    svkImageWriterFactory::WriterType dataTypeOut = svkImageWriterFactory::UNDEFINED;


    string cmdLine = svkProvenance::GetCommandLineString( argc, argv );
    enum FLAG_NAME {
       FLAG_VOL1 = 0, 
       FLAG_VOL2,  
       FLAG_DICE 
    };

    static struct option long_options[] =
    {
        {"i1",    required_argument, NULL,  FLAG_VOL1},
        {"i2",    required_argument, NULL,  FLAG_VOL2},
        {"dice",  no_argument,       NULL,  FLAG_DICE},
        {0, 0, 0, 0}
    };


    /*
    *   Process flags and arguments
    */
    int i;
    int option_index = 0; 
    while ((i = getopt_long(argc, argv, "o:hv", long_options, &option_index)) != EOF) {
        switch (i) {
            case 'o':
                outputFileName.assign(optarg);
                break;
            case FLAG_VOL1:
                input1FileName.assign( optarg );
                break;
            case FLAG_VOL2:
                input2FileName.assign( optarg );
                break;
            case FLAG_DICE:
                computeDice = true; 
                break;
            case 'v':
                verbose = true;
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

    if ( argc != 0 || input1FileName.length() == 0 || input2FileName.length() == 0 || outputFileName.length() == 0 ) { 
        cout << usemsg << endl;
        exit(1); 
    }


    if( verbose ) {
        cout << "INPUT:   " << input1FileName << endl;
        cout << "INPUT:   " << input2FileName << endl;
        cout << "OUTPUT:  " << outputFileName << endl;
    }

    svkImageReaderFactory* readerFactory = svkImageReaderFactory::New();
    svkImageReader2* reader1 = readerFactory->CreateImageReader2(input1FileName.c_str());
    svkImageReader2* reader2 = readerFactory->CreateImageReader2(input2FileName.c_str());

    //  Get input reader type: 
    if ( reader1->IsA("svkDcmMriVolumeReader") ) {
        cout << "Input DCM MRI " << endl;
        dataTypeOut = svkImageWriterFactory::DICOM_MRI;
    } else if ( reader1->IsA("svkDcmEnhancedVolumeReader") ) {
        cout << "Input DCM Enhanced MRI " << endl;
        dataTypeOut = svkImageWriterFactory::DICOM_ENHANCED_MRI;
    } else if ( reader1->IsA("svkIdfVolumeReader") ) {
        cout << "Input IDF " << endl;
        dataTypeOut = svkImageWriterFactory::IDF;
    }

    readerFactory->Delete(); 
    if (reader1 == NULL || reader2 == NULL ) {
        cerr << "Can not determine appropriate reader for: " << input1FileName << endl;
        exit(1);
    }
    reader1->SetFileName( input1FileName.c_str() );
    reader1->Update(); 
    reader2->SetFileName( input2FileName.c_str() );
    reader2->Update(); 


    //  Set the input command line into the data set provenance:
    reader1->GetOutput()->GetProvenance()->SetApplicationCommand( cmdLine );
    reader2->GetOutput()->GetProvenance()->SetApplicationCommand( cmdLine );

    //  Scale image by constant factor: 
    svkImageMathematics* math = svkImageMathematics::New();
    math->SetInput1( reader1->GetOutput() );
    math->SetInput2( reader2->GetOutput() ); 
    math->SetOperationToSubtract();   
    math->Update();

    //  Statistical Validation of Image Segmentation Quality Based on a Spatial Overlap Index
    //  Kelly H. Zou, PhD, Simon K. Warfield, PhD, Aditya Bharatha, MD, Clare M.C. Tempany, 
    //  MD, Michael R. Kaus, PhD, Steven J. Haker, PhD, William M. Wells, III, PhD, 
    //  Ferenc A. Jolesz, MD, and Ron Kikinis, MD
    if ( computeDice == true ) { 

        //  find the intersection of the 2 volumes 
        svkImageMathematics* intersect = svkImageMathematics::New();
        intersect->SetInput1( reader1->GetOutput() );
        intersect->SetInput2( reader2->GetOutput() ); 
        intersect->SetOperationToMultiply();   
        intersect->Update();

        vtkImageAccumulate* intersectHist = vtkImageAccumulate::New();
        intersectHist->SetInput( intersect->GetOutput() ); 
        intersectHist->IgnoreZeroOn();
        intersectHist->Update();
        int numVoxelsIntersect = intersectHist->GetVoxelCount();
       
        //  Find the num voxels in each of the 2 volumes: 
        vtkImageAccumulate* vol1Hist = vtkImageAccumulate::New();
        vol1Hist->SetInput( reader1->GetOutput() ); 
        vol1Hist->IgnoreZeroOn();
        vol1Hist->Update();
        int numVoxels1 = vol1Hist->GetVoxelCount();

        vtkImageAccumulate* vol2Hist = vtkImageAccumulate::New();
        vol2Hist->SetInput( reader2->GetOutput() ); 
        vol2Hist->IgnoreZeroOn();
        vol2Hist->Update();
        int numVoxels2 = vol2Hist->GetVoxelCount();

        float dice = (2. * numVoxelsIntersect ) / (numVoxels1 + numVoxels2) ; 
        cout << "====================================" << endl;
        cout << "Dice Coefficient: " << dice << endl;
        cout << "   numVoxels Intersection: " << numVoxelsIntersect << endl;
        cout << "   numVoxels vol1:         " << numVoxels1 << endl;
        cout << "   numVoxels vol2:         " << numVoxels2 << endl;
        cout << "====================================" << endl;

    }

    // If the type is supported be svkImageWriterFactory then use it, otherwise use the vtkXMLWriter
    svkImageWriterFactory* writerFactory = svkImageWriterFactory::New();
    svkImageWriter* writer = static_cast<svkImageWriter*>(writerFactory->CreateImageWriter( dataTypeOut ) );

    if ( writer == NULL ) {
        cerr << "Can not determine writer of type: " << dataTypeOut << endl;
        exit(1);
    }

    writerFactory->Delete();
    writer->SetFileName( outputFileName.c_str() );
    writer->SetInput( math->GetOutput() );
    writer->Write();
    writer->Delete();

    reader1->Delete();
    reader2->Delete();
    math->Delete();

    return 0; 
}

