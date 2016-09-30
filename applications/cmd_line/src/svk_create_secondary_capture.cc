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
#include <svkDICOMImageWriter.h>
#include <vtkXMLImageDataWriter.h>
#include <svkDICOMSCWriter.h>


using namespace svk;


int main (int argc, char** argv)
{

    string usemsg("\n") ; 
    usemsg += "Version " + string(SVK_RELEASE_VERSION) + "\n";   
    usemsg += "svk_create_secondary_capture -i input_file_name -o output_file_name                \n";
    usemsg += "                             -r reference_dicom [-vh]                              \n";
    usemsg += "                                                                                   \n";
    usemsg += "   -i            input_file_name     Input image to convert to DICOM SC.           \n";
    usemsg += "   -r            reference_dicom     Reference DICOM image to get attributes from. \n";
    usemsg += "   -o            output_file_name    Name of outputfile.                           \n";
    usemsg += "   -v                                Verbose output.                         \n";
    usemsg += "   -h                                Print help mesage.                      \n";  
    usemsg += "                                                                             \n";  
    usemsg += "Creates a DICOM Secondary Capture from an input image (jpeg, tiff, png) and a\n";
    usemsg += "reference DICOM image from which patient and study information is taken.     \n";
    usemsg += "                                                                             \n";

    string inputFileName; 
    string outputFileName; 
    string referenceFileName;

    bool   verbose = false;

    string cmdLine = svkProvenance::GetCommandLineString( argc, argv );

    /*
    *   Process flags and arguments
    */
    int i;
    while ((i = getopt(argc, argv, "i:o:r:bhv")) != EOF) {
        switch (i) {
            case 'i':
                inputFileName.assign( optarg );
                break;
            case 'o':
                outputFileName.assign(optarg);
                break;
            case 'r':
                referenceFileName.assign(optarg);
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


    if( ! svkUtils::FilePathExists( inputFileName.c_str() ) ) {
        cerr << "Input file can not be loaded (may not exist) " << inputFileName << endl;
        exit(1); 
    }

    if( ! svkUtils::FilePathExists( referenceFileName.c_str() ) ) {
        cerr << "Reference file can not be loaded (may not exist) " << referenceFileName << endl;
        exit(1);
    }


    if( verbose ) {
        cout << inputFileName << endl;
        cout << outputFileName << endl;
        cout << referenceFileName << endl;
    }

    // Read in the reference image
    svkImageReaderFactory* referenceImageReaderFactory = svkImageReaderFactory::New();
    svkImageReader2* referenceImageReader = referenceImageReaderFactory->CreateImageReader2(referenceFileName.c_str());
    referenceImageReaderFactory->Delete();

    if (referenceImageReader == NULL) {
        cerr << "Can not determine appropriate reader for: " << inputFileName << endl;
        exit(1);
    }
    referenceImageReader->SetFileName(referenceFileName.c_str());
    referenceImageReader->Update();

    // Read in the image
    vtkImageReader2Factory* imageReaderFactory = vtkImageReader2Factory::New();
    vtkImageReader2* inputImageReader = imageReaderFactory->CreateImageReader2(inputFileName.c_str());
    imageReaderFactory->Delete();

    if (inputImageReader == NULL) {
        cerr << "Can not determine appropriate reader for: " << inputFileName << endl;
        exit(1);
    }

    inputImageReader->SetFileName( inputFileName.c_str() );
    inputImageReader->Update();
    vtkImageData* currentImage =  inputImageReader->GetOutput();
    // Copy data and header into an svkImageReader
    svkMriImageData* imageWithHeader = svkMriImageData::New();
    imageWithHeader->SetDcmHeader(referenceImageReader->GetOutput()->GetDcmHeader());
    imageWithHeader->DeepCopy(currentImage);

    //  Set the input command line into the data set provenance:
    imageWithHeader->GetProvenance()->SetApplicationCommand( cmdLine );

    svkImageWriter* writer = svkDICOMSCWriter::New();
    writer->SetSeriesDescription("Secondary Capture Generated By SIVIC");
    writer->SetSeriesNumber(1234);


    writer->SetFileName( outputFileName.c_str() );
    writer->SetInputData( imageWithHeader );

    writer->Write();
    writer->Delete();
    imageWithHeader->Delete();
    referenceImageReader->Delete();
    inputImageReader->Delete();

    return 0; 
}

