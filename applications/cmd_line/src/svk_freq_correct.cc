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
 *  Utility application for phasing spectroscopic data. 
 *
 */

#include <vtkSmartPointer.h>

#include <svkImageReaderFactory.h>
#include <svkImageReader2.h>
#include <svkSpecPoint.h>

#include <svkImageWriterFactory.h>
#include <svkImageWriter.h>
#include <svkDcmHeader.h>
#include <svkFreqCorrect.h>
#include <svkPhaseSpec.h>


#ifdef WIN32
extern "C" {
#include <getopt.h>
}
#else
#include <getopt.h>
#endif

#define UNDEFINED -1

using namespace svk;



int main (int argc, char** argv)
{

    string usemsg("\n") ; 
    usemsg += "Version " + string(SVK_RELEASE_VERSION) +                                   "\n";   
    usemsg += "svk_freq_correct -i input_file_name -o output_root [ -t output_data_type ]   \n"; 
    usemsg += "                 ( -s shift | --map mapFile ) [ --fs ]                       \n"; 
    usemsg += "                 [ -uh ] [ --single ]                                        \n"; 
    usemsg += "                                                                             \n"; 
    usemsg += "   -i        name        Name of input data file to phase.                   \n"; 
    usemsg += "   -o        root_name   Name of input data file to phase.                   \n"; 
    usemsg += "   -t        type        Target data type:                                   \n";
    usemsg += "                         2 = UCSF DDF                                        \n";
    usemsg += "                         4 = DICOM_MRS (default)                             \n";
    usemsg += "   -u        units       Units for -s or  --map                              \n";
    usemsg += "                         1 = PPM                                             \n";
    usemsg += "                         2 = HZ                                              \n";
    usemsg += "                         3 = PTS (default)                                   \n";
    usemsg += "   -s        shift       Global freq shift +/- (pts)                         \n"; 
    usemsg += "   --fs                  Apply by fourier shift rather t han point shift.    \n"; 
    usemsg += "                         applying a phase shift in time.                     \n"; 
    usemsg += "   --map     map_file    Image with values of freq shifts                    \n"; 
    usemsg += "   --single              Only phase the specified file if multiple in series.\n";
    usemsg += "   -h                    Print this help mesage.                             \n";  
    usemsg += "\n";  
    usemsg += "Applies frequency corrections ta dataset from a map or a global value.       \n"; 
    usemsg += "\n";  


    string  inputFileName; 
    string  outputFileName;
    svkImageWriterFactory::WriterType dataTypeOut = svkImageWriterFactory::DICOM_MRS;
    svkSpecPoint::UnitType units = svkSpecPoint::PTS;
    string  mapFileName = "";
    float   fourierShift = 0;
    bool    onlyCorrectSingleFile = false;
    float   shift = 0; 

    string cmdLine = svkProvenance::GetCommandLineString( argc, argv ); 

    enum FLAG_NAME {
        FLAG_MAP = 0, 
        FLAG_FOURIER, 
        FLAG_SINGLE 
    };


    static struct option long_options[] =
    {
        {"map",     required_argument, NULL,  FLAG_MAP},
        {"fs",      no_argument,       NULL,  FLAG_FOURIER},
        {"single",  no_argument,       NULL,  FLAG_SINGLE},
        {0, 0, 0, 0}
    };


    // ===============================================  
    //  Process flags and arguments
    // ===============================================  
    int unitsInt;
    int i;
    int option_index = 0; 
    while ( ( i = getopt_long(argc, argv, "i:o:t:u:s:h", long_options, &option_index) ) != EOF) {
        
        switch (i) {
            case 'i':
                inputFileName.assign( optarg );
                break;
            case 'o':
                outputFileName.assign(optarg);
                break;
            case 't':
                dataTypeOut = static_cast<svkImageWriterFactory::WriterType>( atoi(optarg) );
                break;
            case FLAG_MAP:
                mapFileName.assign( optarg );
                break;
            case FLAG_FOURIER:
                fourierShift = true;
            case FLAG_SINGLE:
                onlyCorrectSingleFile = true;
                break;
            case 'u':
                unitsInt = atoi(optarg); 
                if ( unitsInt == 1 ) {
                    units = svkSpecPoint::PPM; 
                } else if ( unitsInt == 2 ) {
                    units = svkSpecPoint::Hz; 
                } else if ( unitsInt == 3 ) {
                    units = svkSpecPoint::PTS; 
                }
                break;
            case 's':
                shift = atof(optarg); 
                break;
            case 'h':
                cout << usemsg << endl;
                exit(1);  
                break;
            default:
                ;
        }
    }


    // ===============================================  
    //  validate input: 
    // ===============================================  
    if ( 
        outputFileName.length() == 0 ||
        inputFileName.length() == 0  ||
        ( dataTypeOut != svkImageWriterFactory::DICOM_MRS && dataTypeOut != svkImageWriterFactory::DDF ) || 
        ( shift == 0 && mapFileName.length() == 0 ) || 
        ( shift == 0 && mapFileName.length() != 0 ) 
    ) {
            cout << "ERROR with arguments!" << endl;
            cout << usemsg << endl;
            exit(1); 
    }

    if( ! svkUtils::FilePathExists( inputFileName.c_str() ) ) {
        cerr << "Input file can not be loaded (may not exist) " << inputFileName << endl;
        exit(1);
    }

    cout << "input file name:  " << inputFileName << endl;
    cout << "output file root: " << outputFileName << endl;
    if  ( mapFileName.length() != 0 ) { 
        if( ! svkUtils::FilePathExists( mapFileName.c_str() ) ) {
            cerr << "map file can not be loaded (may not exist) " << mapFileName << endl;
            exit(1);
        }
        cout << "map file name:   " << mapFileName << endl;
    } else {    
        cout << "shift:   " << shift << endl;
    }
    cout << "units: " << units << endl;


    // ===============================================  
    //  Use a reader factory to get the correct reader  
    //  type and load data. 
    // ===============================================  
    vtkSmartPointer< svkImageReaderFactory > mrsReaderFactory = vtkSmartPointer< svkImageReaderFactory >::New(); 
    // ===============================================  
    //  Input Spec data    
    // ===============================================  
    svkImageReader2* reader = mrsReaderFactory->CreateImageReader2(inputFileName.c_str());
    if (reader == NULL) {
        cerr << "Can not determine appropriate reader for: " << inputFileName << endl;
        exit(1);
    }
    reader->SetFileName( inputFileName.c_str() );
    if ( onlyCorrectSingleFile == true ) {
        reader->OnlyReadOneInputFile();
    }
    reader->Update(); 


    // ===============================================  
    //  Optional freq correct map image 
    // ===============================================  
    vtkSmartPointer< svkImageReaderFactory > mriReaderFactory = vtkSmartPointer< svkImageReaderFactory >::New(); 
    svkImageReader2* mapReader;
    if  ( mapFileName.length() != 0 ) { 
        mapReader = mriReaderFactory->CreateImageReader2( mapFileName.c_str() );
        if ( mapReader == NULL ) {
            cerr << "Can not determine appropriate reader for: " << mapFileName << endl;
            exit(1);
        }
        mapReader->SetFileName( mapFileName.c_str() );
        if ( onlyCorrectSingleFile == true ) {
            mapReader->OnlyReadOneInputFile();
        }
    }


    // ===============================================  
    //  Freq correct the data.: 
    // ===============================================  
    svkFreqCorrect* freqCorrect = svkFreqCorrect::New();
    //svkMrsImageData* mrsData = svkMrsImageData::New();
    //mrsData->DeepCopy( svkMrsImageData::SafeDownCast(reader->GetOutput()) );
    freqCorrect->SetInputData( reader->GetOutput() );
    freqCorrect->SetUnits(units);

    if ( shift != 0 ) {
        if (fourierShift == true){
            freqCorrect->UseFourierShift(); 
        } 
        freqCorrect->SetGlobalFrequencyShift( shift ); 
    } else if ( mapFileName.length() != 0 ) {
        freqCorrect->SetInputConnection( 1, mapReader->GetOutputPort() ); 
    }
    freqCorrect->Update();

    // ===============================================  
    //  Write the data out to the specified file type.  
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
    writer->SetInputData( freqCorrect->GetOutput() );

    // ===============================================  
    //  Set the input command line into the data set 
    //  provenance: 
    // ===============================================  
    freqCorrect->GetOutput()->GetProvenance()->SetApplicationCommand( cmdLine );

    // ===============================================  
    //  Write data to file: 
    // ===============================================  
    writer->Write();

    // ===============================================  
    //  Clean up: 
    // ===============================================  
    writer->Delete();
    reader->Delete();
    freqCorrect->Delete(); 

    return 0; 
}

