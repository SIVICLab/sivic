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

#include <vtkSmartPointer.h>

#include <svkImageReaderFactory.h>
#include <svkImageReader2.h>
#include <svkDdfVolumeReader.h>
#include <svkDcmVolumeReader.h>
#include <svkImageWriterFactory.h>
#include <svkImageWriter.h>
#include <svkDICOMMRSWriter.h>
#include <svkDdfVolumeWriter.h>
#include <svkCorrectDCOffset.h>
#include <svkDcmHeader.h>
#include <svkGEPFileReader.h>
#include <svkGEPFileMapper.h>
#include <svkDdfVolumeReader.h>
#include <svkImageAlgorithm.h>
#include <svkEPSIReorder.h>
#include <svkVariableFlipDatReader.h> 
#include <svkTypeUtils.h>
#ifdef WIN32
extern "C" {
#include <getopt.h>
}
#else
#include <getopt.h>
#endif
#define UNDEFINED_TEMP -1111

#define UNDEFINED -1

using namespace svk;

void ApplyScaling(svkMrsImageData* data, string datFileName, float minimumScalingFactor );

int main (int argc, char** argv)
{

    string usemsg("\n") ; 
    usemsg += "Version " + string(SVK_RELEASE_VERSION) + "\n";   
    usemsg += "svk_variable_flip_scaler -i input_file_name -o output_file_name [ -t output_data_type ]    \n"; 
    usemsg += "                   [ --dat name ] [ -h ]                                             \n";
    usemsg += "                   --------------------------------------------------------------    \n"; 
    usemsg += "   -i                name    Name of file to convert.                                \n"; 
    usemsg += "   -m                float   Minimum scaling factor.                                 \n";
    usemsg += "   --dat             name    Name of dat file with scaling factors.                  \n"; 
    usemsg += "   -o                name    Name of outputfile.                                     \n";
    usemsg += "   -t                type    Target data type:                                       \n";
    usemsg += "                                 2 = UCSF DDF                                        \n";
    usemsg += "                                 4 = DICOM_MRS (default)                             \n";
    usemsg += "   --single                  Only converts specified file if                         \n";
    usemsg += "---------------------------------------------------------------------------------    \n"; 
    usemsg += "   -h                        Print this help mesage.                                 \n";  
    usemsg += "\n";  
    usemsg += "Scales a variable flip angle MRS acquisition.                                        \n"; 
    usemsg += "\n";  


    string  inputFileName; 
    string  outputFileName;
    string  datFileName;
    float   minimumScalingFactor = 0;
    svkImageWriterFactory::WriterType dataTypeOut = svkImageWriterFactory::DICOM_MRS;
    bool   onlyLoadSingleFile = false;


    string cmdLine = svkProvenance::GetCommandLineString( argc, argv ); 

    enum FLAG_NAME {
        FLAG_DAT_FILE = 0, 
        FLAG_SINGLE
    }; 


    static struct option long_options[] =
    {
        /* This option sets a flag. */
        {"dat",                required_argument, NULL,  FLAG_DAT_FILE},
        {"single",             no_argument,       NULL,  FLAG_SINGLE},
        {0, 0, 0, 0}
    };

    // ===============================================  
    //  Process flags and arguments
    // ===============================================  
    int i;
    int option_index = 0; 
    while ( ( i = getopt_long(argc, argv, "i:o:t:m:h", long_options, &option_index) ) != EOF) {
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
            case FLAG_DAT_FILE:
                datFileName.assign(optarg);
                break;
            case FLAG_SINGLE:
                onlyLoadSingleFile = true;
                break;
            case 'm':
                minimumScalingFactor = svkTypeUtils::StringToFloat(optarg);
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
    //      an output name was supplied
    //      that not suppresses and unsuppressed were both specified 
    //      that only the supported output types was requested. 
    //      
    // ===============================================  
    if ( 
        argc != 0 ||  inputFileName.length() == 0  
        || outputFileName.length() == 0 
        || ( dataTypeOut != svkImageWriterFactory::DICOM_MRS && dataTypeOut != svkImageWriterFactory::DDF ) 
        ) 
    {
            cout << usemsg << endl;
            exit(1); 
    }

    if( ! svkUtils::FilePathExists( inputFileName.c_str() ) ) {
        cerr << "Input file can not be loaded (may not exist) " << inputFileName << endl; 
        exit(1); 
    }

    if( ! svkUtils::FilePathExists( datFileName.c_str() ) ) {
        cerr << "Dat file can not be loaded (may not exist) " << datFileName << endl; 
        exit(1); 
    }

    cout << "file name: " << inputFileName << endl;

    // ===============================================  
    //  Get a GEPFile Reader. 
    // ===============================================  
    svkImageReaderFactory* readerFactory = svkImageReaderFactory::New();
    svkImageReader2* reader = readerFactory->CreateImageReader2(inputFileName.c_str());
    readerFactory->Delete();

    if (reader == NULL) {
        cerr << "Can not determine appropriate reader for: " << inputFileName << endl;
        exit(1);
    }

    reader->SetFileName( inputFileName.c_str() );
    if ( onlyLoadSingleFile == true ) {
        reader->OnlyReadOneInputFile();
    }
    reader->Update(); 

    svkImageData* currentImage = svkMrsImageData::SafeDownCast( reader->GetOutput() ); 

    ApplyScaling(svkMrsImageData::SafeDownCast(currentImage), datFileName, minimumScalingFactor);


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
    writer->SetInputData( svkMrsImageData::SafeDownCast( currentImage ) );

    // ===============================================  
    //  Set the input command line into the data set 
    //  provenance: 
    // ===============================================  
    reader->GetOutput()->GetProvenance()->SetApplicationCommand( cmdLine );

    // ===============================================  
    //  Write data to file: 
    // ===============================================  
    writer->Write();

    // ===============================================  
    //  Clean up: 
    // ===============================================  
    writer->Delete();
    reader->Delete();

    return 0; 
}

void ApplyScaling(svkMrsImageData* data, string datFileName, float minimumScalingFactor )
{
    svkVariableFlipDatReader* datReader =  svkVariableFlipDatReader::New();
    datReader->SetFileName( datFileName.c_str() );
    //datReader->CanReadFile(datName.c_str());
    datReader->Update();
    cout << "DAT NTP: " << datReader->GetNumTimePoints() << endl;;
    cout << "DAT NTP: " << datReader->GetProfileNumPoints() << endl;;
    vtkFloatArray* signalScale = vtkFloatArray::New();
    svkDcmHeader::DimensionVector inDimVector = data->GetDcmHeader()->GetDimensionIndexVector();
    svkDcmHeader::DimensionVector loopVector = inDimVector;
    int numInputCells = svkDcmHeader::GetNumberOfCells( &inDimVector );
    int numCoils = data->GetDcmHeader()->GetNumberOfCoils();
    int numTimePts = data->GetDcmHeader()->GetNumberOfTimePoints();
    int numVoxels[3];
    data->GetNumberOfVoxels(numVoxels);
    for (int timePt = 0; timePt < numTimePts; timePt++) {
        datReader->GetSignalScaling(timePt+1, signalScale);
        for (int coilNum = 0; coilNum < numCoils; coilNum++) {
            for (int z = 0; z < numVoxels[2]; z++) {
                for (int y = 0; y < numVoxels[1]; y++) {
                    for (int x = 0; x < numVoxels[0]; x++) {
                        vtkDataArray* spectrum = data->GetSpectrum(x,y,z,timePt,coilNum);
                        for( int f = 0; f < spectrum->GetNumberOfTuples(); f++) {
                            for( int c = 0; c < spectrum->GetNumberOfComponents(); c++) {
                                if( signalScale->GetTuple1(f) > minimumScalingFactor ) {
                                    spectrum->SetComponent(f,c, ( spectrum->GetComponent(f,c) / signalScale->GetTuple1(f) ));
                                    //spectrum->SetComponent(f,c, ( spectrum->GetComponent(f,c) / signalScale->GetTuple1(f) )/20.0);
                                } else if( minimumScalingFactor > 0 ) {
                                    spectrum->SetComponent(f,c, ( spectrum->GetComponent(f,c) / minimumScalingFactor ));
                                    //spectrum->SetComponent(f,c, ( spectrum->GetComponent(f,c) / minimumScalingFactor )/20.0);
                                }
                                //spectrum->SetComponent(f,c, signalScale->GetTuple1(f)*1000000000);
                            }
                        }
                    }
                }
            }
        }
    }
    for ( int i = 0; i < datReader->GetProfileNumPoints(); i++ ) {
        cout << "CHECK THE SCALE time pt 7: " << i << " " << *signalScale->GetTuple(i) << endl;
    }
}
