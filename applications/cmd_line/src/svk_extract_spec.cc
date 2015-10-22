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
 *  $URL: svn+ssh://stojan-m@svn.code.sf.net/p/sivic/code/trunk/applications/cmd_line/src/svk_extract_spec.cc $
 *  $Rev: 1919 $
 *  $Author: jccrane $
 *  $Date: 2014-05-06 15:49:42 -0700 (Tue, 06 May 2014) $
 *
 *  Authors:
 *      Jason C. Crane, Ph.D.
 *      Beck Olson
 *      Stojan Maleschlijski
 *
 *  Utility application for extracting MRSI data from DCM and saving as DCM Image.
 */

#include <vtkSmartPointer.h>

#include <svkImageReaderFactory.h>
#include <svkImageReader2.h>
#include <svkDcmVolumeReader.h>
#include <svkImageWriterFactory.h>
#include <svkImageWriter.h>
#include <svkDICOMMRSWriter.h>
#include <svkDcmHeader.h>

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
    usemsg += "svk_extract_spec -i input_file_name -o output_root -c\n";
    usemsg += "Extracts the MRS data (consisting of N frequency points for each voxel) from a ddf image and saves it as N DCM images, each containing a map of a single frequency point. -c specifies to convert both real and imag components. Default is real only.\n";  
    usemsg += "\n";  

    bool   handleComplex = false;
    string inputFileName; 
    string outputFileName;

    svkImageWriterFactory::WriterType dataTypeOut = svkImageWriterFactory::IDF;

    string cmdLine = svkProvenance::GetCommandLineString( argc, argv ); 

    static struct option long_options[] =
    {
        /* This option sets a flag. */
        {0, 0, 0, 0}
    };
    // ===============================================  
    //  Process flags and arguments
    // ===============================================  
    int i;
    int option_index = 0; 
    while ( ( i = getopt_long(argc, argv, "i:o:ch", long_options, &option_index) ) != EOF) {
        switch (i) {
            case 'i':
                inputFileName.assign( optarg );
                break;
            case 'o':
                outputFileName.assign(optarg);
                break;
            case 'c':
                handleComplex = true;
                break;
            default:
                ;
        }
    }

    argc -= optind;
    argv += optind;

    // ===============================================  
    //  validate that: 
    //      an output name was supplied

    //      
    // ===============================================  
    if ( argc != 0 ||  inputFileName.length() == 0  
         || outputFileName.length() == 0 
         || ( dataTypeOut != svkImageWriterFactory::IDF ) 
        ) {
            cout << usemsg << endl;
            exit(1); 
    }

    cout << "file name: " << inputFileName << endl;

     // ===============================================  
    //  Use a reader factory to get the correct reader  
    //  type . 
    // ===============================================  
    vtkSmartPointer< svkImageReaderFactory > readerFactory = vtkSmartPointer< svkImageReaderFactory >::New(); 

    svkImageReader2* mrsReader = readerFactory->CreateImageReader2(inputFileName.c_str());
    if (mrsReader == NULL) {
        cerr << "Can not determine appropriate reader for test data: " << inputFileName << endl;
        exit(1);
    }
    mrsReader->SetFileName( inputFileName.c_str() );
    mrsReader->Update(); 
    svkMrsImageData* mrsData = svkMrsImageData::SafeDownCast( mrsReader->GetOutput() ); 

    //int numTimePts = mrsData->GetDcmHeader()->GetNumberOfTimePoints(); // Or Freq points?
    int numSpecPts = mrsData->GetDcmHeader()->GetIntValue("DataPointColumns" );

    // ===============================================   
    //  Use an svkImageWriterFactory to obtain the
    //  correct writer type. 
    // ===============================================  
    vtkSmartPointer< svkImageWriterFactory > writerFactory = vtkSmartPointer< svkImageWriterFactory >::New(); 
    svkImageWriter* writer = static_cast<svkImageWriter*>( writerFactory->CreateImageWriter( dataTypeOut ) ); 

    if ( writer == NULL ) {
        cerr << "Can not determine writer of type: " << dataTypeOut << endl;
        exit(1);
    }

    svkDcmHeader::DimensionVector fullDimensionVector = mrsData->GetDcmHeader()->GetDimensionIndexVector();
    int numChannels = svkDcmHeader::GetDimensionVectorValue(&fullDimensionVector, svkDcmHeader::CHANNEL_INDEX) + 1; 

     if ( numChannels > 1 ) {
        cerr << "Unsuported number of channels found in the header. Currently only 1 channel possible." << endl;
        exit(1);
    }

    // include check for the channels
    string currentOutputFile;

    svkMriImageData* tmpImage = svkMriImageData::New();
   // numSpecPts = 1; 
    for (int pnt = 0; pnt < numSpecPts; pnt++){
        cout << "Specpoint:" << pnt << "/" << numSpecPts << endl;
        // pseudo code
        char numstr[10];
        sprintf(numstr, "%d", pnt);
        currentOutputFile.assign(outputFileName.c_str());
        currentOutputFile.append("_real");
        currentOutputFile.append(numstr);
       // currentOutputFile.append(".dcm");
        // just real for now
        mrsData->GetImage(tmpImage, pnt, 0, 0, 0, ""); 

        writer->SetFileName( currentOutputFile.c_str() );
        writer->SetInput( svkMriImageData::SafeDownCast( tmpImage ) );
        writer->Write();
        if ( handleComplex ){
            currentOutputFile.assign(outputFileName.c_str());
            currentOutputFile.append("_imag");
            currentOutputFile.append(numstr);

            // imaginary
            mrsData->GetImage(tmpImage, pnt, 0, 0, 1, ""); 

            writer->SetFileName( currentOutputFile.c_str() );
            writer->SetInput( svkMriImageData::SafeDownCast( tmpImage ) );
            writer->Write();
        }

    }
    tmpImage->Delete();

// Obtain representaiton

    // ===============================================  
    //  Set the input command line into the data set 
    //  provenance: 
    // ===============================================  
    mrsData->GetProvenance()->SetApplicationCommand( cmdLine );


    // ===============================================  
    //  Clean up: 
    // ===============================================  
    writer->Delete();
    mrsReader->Delete();


    return 0; 
}

