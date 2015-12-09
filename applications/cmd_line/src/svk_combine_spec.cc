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
 *  Utility application for combining DCM freq data into MRS DCM Image.
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
    usemsg += "svk_combine_spec -i inputMRI_DCMs_root -t originalDDF -o output_ddf -c\n";
    usemsg += "Combines the spectra found in the MRI_DCMs and uses the header from original DDF to create a combination DDF and save this in output_ddf. Per default only real files are handled, if -c option not specified.\n";  
    usemsg += "\n";  

    bool    handleComplex = false;
    string inputRoot;
    string originalDDF ;
    string outputFileName;

    svkImageWriterFactory::WriterType dataTypeOut = svkImageWriterFactory::DDF;

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
    while ( ( i = getopt_long(argc, argv, "i:o:t:ch", long_options, &option_index) ) != EOF) {
        switch (i) {
            case 'i':
                inputRoot.assign( optarg );
                break;
            case 'c':
                handleComplex = true;
                break;
            case 'o':
                outputFileName.assign( optarg );
                break;
            case 't':
                originalDDF.assign( optarg );
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
    //  validate that: 
    //      an target name was supplied

    //      
    // ===============================================  
    if ( argc != 0 ||  inputRoot.length() == 0  
         || outputFileName.length() == 0 
         || originalDDF.length() == 0
         || ( dataTypeOut != svkImageWriterFactory::DDF ) 
        ) {
            cout << usemsg << endl;
            exit(1); 
    }

    cout << "file name: " << originalDDF << endl;

    // Get a reader for target DDF
    // Get a reader for the MRI images
    // Save the DDF object with a different name

     // ===============================================  
    //  Use a reader factory to get the correct reader  
    //  type . 
    // ===============================================  
    vtkSmartPointer< svkImageReaderFactory > readerFactory = vtkSmartPointer< svkImageReaderFactory >::New(); 

    svkImageReader2* mrsReader = readerFactory->CreateImageReader2(originalDDF.c_str());
    if (mrsReader == NULL) {
        cerr << "Can not determine appropriate reader for test data: " << originalDDF << endl;
        exit(1);
    }
    

    mrsReader->SetFileName( originalDDF.c_str() );
    mrsReader->Update(); 

    svkMrsImageData* mrsData = svkMrsImageData::SafeDownCast( mrsReader->GetOutput() ); 

    //int numTimePts = mrsData->GetDcmHeader()->GetNumberOfTimePoints(); // Or Freq points?
    int numSpecPts = mrsData->GetDcmHeader()->GetIntValue("DataPointColumns" );

    svkDcmHeader::DimensionVector fullDimensionVector = mrsData->GetDcmHeader()->GetDimensionIndexVector();
    int numChannels = svkDcmHeader::GetDimensionVectorValue(&fullDimensionVector, svkDcmHeader::CHANNEL_INDEX) + 1; 

     if ( numChannels > 1 ) {
        cerr << "Unsuported number of channels found in the header. Currently only 1 channel possible." << endl;
        exit(1);
    }
        
    // include check for the channels
    string currentInputFile;
  //  vtkSmartPointer< svkImageReaderFactory > readerFactory2 = vtkSmartPointer< svkImageReaderFactory >::New(); 

   // numSpecPts = 1024; 


    for (int pnt = 0; pnt < numSpecPts; pnt++){
        cout << "Specpoint:" << pnt << "/" << numSpecPts << endl;
        char numstr[10];
        sprintf(numstr, "%d", pnt);
        currentInputFile.assign(inputRoot.c_str());
        if ( handleComplex ){
        	currentInputFile.append("_real");
        }
        currentInputFile.append(numstr);
        currentInputFile.append(".idf");
       // currentOutputFile.append(".dcm");
        
        svkImageReader2* mriReader = readerFactory->CreateImageReader2(currentInputFile.c_str());

        if (mriReader == NULL) {
            cerr << "Can not determine appropriate resader for input data: " << currentInputFile << endl;
            exit(1);
        }
        mriReader->SetFileName( currentInputFile.c_str() );
        mriReader->OnlyReadOneInputFile();
 //       cout << currentInputFile <<endl;
        mriReader->Update(); 

        svkMriImageData* mriData = svkMriImageData::SafeDownCast( mriReader->GetOutput() ); 
         // just real for now
        mrsData->SetImageComponent(mriData, pnt, 0, 0, 0); 
        mriReader->Delete();
       
        if ( handleComplex ){
            currentInputFile.assign(inputRoot.c_str());
            currentInputFile.append("_imag");
            currentInputFile.append(numstr);
            currentInputFile.append(".idf");
            
            svkImageReader2* mriReader1 = readerFactory->CreateImageReader2(currentInputFile.c_str());

            if (mriReader1 == NULL) {
                cerr << "Can not determine appropriate reader for input data: " << currentInputFile << endl;
                exit(1);
            }
            mriReader1->SetFileName( currentInputFile.c_str() );
            mriReader1->OnlyReadOneInputFile();
            mriReader1->Update(); 

            svkMriImageData* mriData1 = svkMriImageData::SafeDownCast( mriReader1->GetOutput() ); 

            // imag
            mrsData->SetImageComponent(mriData1, pnt, 0, 0, 1); 
            /*
            svkImageWriterFactory::WriterType dataTypeOut1 = svkImageWriterFactory::IDF;
            vtkSmartPointer< svkImageWriterFactory > writerFactory1 = vtkSmartPointer< svkImageWriterFactory >::New(); 
            svkImageWriter* writer1 = static_cast<svkImageWriter*>( writerFactory1->CreateImageWriter( dataTypeOut1 ) ); 
            writer1->SetFileName( "test.ddf");
            writer1->SetInput( svkMriImageData::SafeDownCast( mriData1 ) );
            writer1->Write();*/


            mriReader1->Delete();        
        }

    }
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


    writer->SetFileName( outputFileName.c_str() );
    writer->SetInput( svkMrsImageData::SafeDownCast( mrsData ) );
    writer->Write();


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

