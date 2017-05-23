/*
 *  Copyright © 2009-2017 The Regents of the University of California.
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
 *      Bjoern Menze, Ph.D
 *      Jason C. Crane, Ph.D.
 *      Beck Olson
 *
 *  Utility application for applying an HSVD filter to an MRS data set. 
 *
 */

#include <vtkSmartPointer.h>

#include <svkImageReaderFactory.h>
#include <svkImageWriterFactory.h>
#include <svkImageWriter.h>
#include <svkDcmHeader.h>
#include <svkHSVD.h>
#include <svkSpecPoint.h>


#ifdef WIN32
extern "C" {
#include <getopt.h>
}
#else
#include <getopt.h>
#endif


using namespace svk;

void initCustomFilterVector( vector< vector<float> >* customFilter, float ppmValue ); 


int main (int argc, char** argv)
{

    int exitstatus = 1; // error!

    string usemsg("\n") ; 
    usemsg += "Version " + string(SVK_RELEASE_VERSION) +                                       "\n";   
    usemsg += "svk_hsvd -i input_file_name -o output_file_root [ -t output_data_type ]          \n"; 
    usemsg += "         [ -fb ] [ -m order ] [ -wl ] [ --single ]                               \n"; 
    usemsg += "         [ --ppm1 downfield_ppm  --ppm2 upfield_ppm ] [ --error mode ]           \n"; 
    usemsg += "         [ -h ]                                                                  \n"; 
    usemsg += "                                                                                 \n";  
    usemsg += "   -i                name   Name of file to convert.                             \n"; 
    usemsg += "   -o                root   Root name of outputfile.                             \n";
    usemsg += "   -t                type   Target data type:                                    \n";
    usemsg += "                                 2 = UCSF DDF                                    \n";
    usemsg += "                                 4 = DICOM_MRS (default)                         \n";
    usemsg += "   -f                       Write out filter image                               \n";
    usemsg += "   -q                       Write out success map image                          \n";
    usemsg += "   -b                       only filter spectra in selection box, others         \n"; 
    usemsg += "                            are zeroed out.                                      \n"; 
    usemsg += "   -m                order  model order (default = 25)                           \n"; 
    usemsg += "   -w                       remove water (downfield of 4.2PPM )                  \n"; 
    usemsg += "   -l                       remove lipid (upfield of 1.8PPM )                    \n"; 
    usemsg += "   --single                 Only transform specified file if multiple in series  \n";
    usemsg += "   --ppm1            ppm1   Downfield ppm limit for custom filter range          \n";
    usemsg += "   --ppm2            ppm2   Upfield ppm limit for custom filter range            \n";
    usemsg += "   --error           mode   On error set to zero the spectrum of:                \n"; 
    usemsg += "                                 0 = SET_FILTER_TO_ZERO (input signal unchanged) \n";
    usemsg += "                                 1 = SET_SIGNAL_TO_ZERO (default)                \n";
    usemsg += "                                 2 = IGNORE_ERROR                                \n";
    usemsg += "   --singleThreaded         Execute the algorithm as a single                    \n";
    usemsg += "                            thread, e.g. on grids (default is multi-threaded).   \n";
    usemsg += "   -h                       Print this help message.                             \n";
    usemsg += "                                                                                 \n";  
    usemsg += "HSVD filter to remove baseline components from spectra.                          \n";  
    usemsg += "Default is to remove water by filtering all frequencies downfield                \n"; 
    usemsg += "4.2 PPM. On error the input signal is set to 0 (default),                        \n"; 
    usemsg += "left unchanged or filtered.                                                      \n"; 
    usemsg += "\n";  


    string  inputFileName; 
    string  outputFileName;
    bool    writeFilter = false; 
    bool    writeFitSuccessMap = false;
    int     modelOrder = 25; 
    bool    limitToSelectionBox = false; 
    bool    filterWater = false; 
    bool    filterLipid = false; 
    bool    onlyFilterSingleFile = false;
    bool    executeSingleThreaded = false;
    vector< vector <float> > customFilter; 

    svkImageWriterFactory::WriterType dataTypeOut = svkImageWriterFactory::DICOM_MRS;
    
    string cmdLine = svkProvenance::GetCommandLineString( argc, argv ); 

    svkHSVD::HSVDBehaviorOnError errorBehavior = svkHSVD::SET_SIGNAL_TO_ZERO;
    enum FLAG_NAME {
        FLAG_SINGLE, 
        FLAG_PPM1,
        FLAG_PPM2,
        FLAG_ERROR,
        FLAG_SINGLETHREAD
    }; 


    static struct option long_options[] =
    {
        /* This option sets a flag. */
        {"single",    no_argument,       NULL,  FLAG_SINGLE},
        {"ppm1",      required_argument, NULL,  FLAG_PPM1},
        {"ppm2",      required_argument, NULL,  FLAG_PPM2},
        {"error",     required_argument, NULL,  FLAG_ERROR},
        {"singleThreaded", no_argument,  NULL,  FLAG_SINGLETHREAD},
        {0, 0, 0, 0}
    };



    // ===============================================  
    //  Process flags and arguments
    // ===============================================  
    int i;
    int option_index = 0; 
    while ( ( i = getopt_long(argc, argv, "i:o:t:m:fbqwlh", long_options, &option_index) ) != EOF) {
        switch (i) {
            case 'i':
                inputFileName.assign( optarg );
                break;
            case 'o':
                outputFileName.assign(optarg);
                //  Make sure the file name doesn't contain an extension: 
                outputFileName = svkImageReader2::GetFileRoot(outputFileName.c_str()); 
                break;
            case 't':
                dataTypeOut = static_cast<svkImageWriterFactory::WriterType>( atoi(optarg) );
                break;
            case 'f':
                writeFilter = true; 
                break;
            case 'q':
                writeFitSuccessMap = true;
                break;
            case 'b':
                limitToSelectionBox = true; 
                break;
            case 'm':
                modelOrder = atoi( optarg ); 
                break;
            case 'w':
                filterWater = true; 
                break;
            case 'l':
                filterLipid = true; 
                break;
            case FLAG_SINGLE:
                onlyFilterSingleFile = true;
                break;
            case FLAG_PPM1:
                initCustomFilterVector( &customFilter, atof( optarg ) ); 
                break;
            case FLAG_PPM2:
                initCustomFilterVector( &customFilter, atof( optarg ) ); 
                break;           
            case FLAG_ERROR:
                errorBehavior = static_cast<svkHSVD::HSVDBehaviorOnError>(atoi( optarg )); 
                break;
            case FLAG_SINGLETHREAD:
                executeSingleThreaded = true;
                break;
            case 'h':
                cout << usemsg << endl;
                exit(exitstatus);
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
    cout << "file name: " << inputFileName << endl;
    cout << "file name: " << outputFileName << endl;
    cout << "argc: " << argc << endl;
    if ( argc != 0 ||  inputFileName.length() == 0  
         || outputFileName.length() == 0 
         || ( dataTypeOut != svkImageWriterFactory::DICOM_MRS && dataTypeOut != svkImageWriterFactory::DDF ) 
         
    ) {
        cout << usemsg << endl;
        exit(exitstatus);
    }

    //  Validate filters: 
    for ( int k = 0; k < customFilter.size(); k++ ) {
        if ( customFilter[k].size() != 2 ) {
            cout << "Error, custom filter must be specfied with ppm1,ppm2 pairs " << endl;
            cout << usemsg << endl;
            exit(exitstatus);
        }
        cout << "Custom filter: " << customFilter[k][0] << " - " << customFilter[k][1] << endl;
    }

    if( ! svkUtils::FilePathExists( inputFileName.c_str() ) ) {
        cerr << "Input file can not be loaded (may not exist) " << inputFileName << endl;
        exit(exitstatus);
    }

    cout << "file name: " << inputFileName << endl;

    //  if no filters specified, then just filter water:
    if ( !filterWater && !filterLipid && (customFilter.size() == 0) ) {
        filterWater = true; 
    }
   
    // check if correct error handling was selected
    if (errorBehavior != svkHSVD::SET_FILTER_TO_ZERO 
        &&  errorBehavior != svkHSVD::SET_SIGNAL_TO_ZERO 
        && errorBehavior != svkHSVD::IGNORE_ERROR ){
        errorBehavior = svkHSVD::SET_SIGNAL_TO_ZERO;
        cout << "Specified error behavior not found. On error the input signal will be set to zero" << endl;
    }

    // ===============================================  
    //  Use a reader factory to get the correct reader  
    //  type . 
    // ===============================================  
    vtkSmartPointer< svkImageReaderFactory > readerFactory = vtkSmartPointer< svkImageReaderFactory >::New(); 
    svkImageReader2* reader = readerFactory->CreateImageReader2(inputFileName.c_str());

    if (reader == NULL) {
        cerr << "Can not determine appropriate reader for: " << inputFileName << endl;
        exit(exitstatus);
    }

    reader->SetFileName( inputFileName.c_str() );
    if ( onlyFilterSingleFile == true ) {
        reader->OnlyReadOneInputFile();
    }
    reader->Update(); 

    svkDcmHeader* hdr = reader->GetOutput()->GetDcmHeader(); 
    svkSpecPoint* point = svkSpecPoint::New();
    point->SetDcmHeader( hdr ); 
    int numFreqPoints = hdr->GetIntValue( "DataPointColumns" );

    // ===============================================  
    //  HSVD DATA   
    // ===============================================  
    svkHSVD* hsvd = svkHSVD::New();
    if ( writeFilter ) {
        hsvd->ExportFilterImage(); 
    }
    hsvd->SetInputData( reader->GetOutput() ); 
    hsvd->SetModelOrder( modelOrder ); 
    if ( limitToSelectionBox ) {
        hsvd->OnlyFitSpectraInVolumeLocalization(); 
    }
    if ( filterWater ) {
        cout << "Filter Water " << endl;
        hsvd->RemoveH20On();
    }
    if ( filterLipid ) {
        cout << "Filter Lipid " << endl;
        hsvd->RemoveLipidOn();
    }
    for ( int k = 0; k < customFilter.size(); k++ ) {
        hsvd->AddPPMFrequencyFilterRule( customFilter[k][0], customFilter[k][1] ); 
    }
 
    hsvd->SetErrorHandlingBehavior( errorBehavior );

    if ( executeSingleThreaded ) {
        cout << "Executing as a singe thread " << endl;
        hsvd->SetSingleThreaded();
    }

    hsvd->Update();

    bool bAllCellsFit = hsvd->GetFitSuccessStatus();

    // ===============================================  
    //  Write the data out to the specified file type.  
    //  Use an svkImageWriterFactory to obtain the
    //  correct writer type. 
    // ===============================================  
    vtkSmartPointer< svkImageWriterFactory > writerFactory = vtkSmartPointer< svkImageWriterFactory >::New(); 
    svkImageWriter* writer = static_cast<svkImageWriter*>( writerFactory->CreateImageWriter( dataTypeOut ) ); 

    if ( writer == NULL ) {
        cerr << "Can not determine writer of type: " << dataTypeOut << endl;
        exit(exitstatus);
    }


    writer->SetFileName( outputFileName.c_str() );
    writer->SetInputData( svkMrsImageData::SafeDownCast( reader->GetOutput() ) );

    // ===============================================  
    //  Set the input command line into the data set 
    //  provenance: 
    // ===============================================  
    reader->GetOutput()->GetProvenance()->SetApplicationCommand( cmdLine );

    // ===============================================  
    //  Write data to file: 
    // ===============================================  
    writer->Write();


    //  Write out the filter image if requested.
    if ( writeFilter ) {

        svkImageWriter* filterWriter = static_cast<svkImageWriter*>( writerFactory->CreateImageWriter( dataTypeOut ) ); 
        if ( filterWriter == NULL ) {
            cerr << "Can not determine filter writer of type: " << dataTypeOut << endl;
            exit(exitstatus);
        }
        string filterImageName = outputFileName; 
        filterImageName.append("_filter"); 
        filterWriter->SetFileName( filterImageName.c_str() );
        filterWriter->SetInputData( hsvd->GetFilterImage() ); 

        // ===============================================  
        //  Set the input command line into the data set 
        //  provenance: 
        // ===============================================  
        hsvd->GetFilterImage()->GetProvenance()->SetApplicationCommand( cmdLine );

        // ===============================================  
        //  Write data to file: 
        // ===============================================  
        filterWriter->Write();
        filterWriter->Delete();
        
    }

    //  Write out the fit success map if requested.
    if ( writeFitSuccessMap ) {

        svkImageWriter* fitSuccessMapWriter = static_cast<svkImageWriter*>( writerFactory->CreateImageWriter( svkImageWriterFactory::IDF ) );
        if ( fitSuccessMapWriter == NULL ) {
            cerr << "Can not determine filter writer of type: " << dataTypeOut << endl;
            exit(exitstatus);
        }
        string fitSuccessMapImageName = outputFileName;
        fitSuccessMapImageName.append("_successmap");
        fitSuccessMapWriter->SetFileName( fitSuccessMapImageName.c_str() );
        fitSuccessMapWriter->SetInputData( hsvd->GetFitSuccessImage() );

        // ===============================================
        //  Set the input command line into the data set
        //  provenance:
        // ===============================================
        hsvd->GetFitSuccessImage()->GetProvenance()->SetApplicationCommand( cmdLine );

        // ===============================================
        //  Write data to file:
        // ===============================================
        fitSuccessMapWriter->Write();
        fitSuccessMapWriter->Delete();

    }

    // ===============================================  
    //  Clean up: 
    // ===============================================  
    writer->Delete();
    reader->Delete();
    hsvd->Delete();

    if (bAllCellsFit){
        exitstatus = 0;
    }

    return exitstatus;
}


void initCustomFilterVector( vector< vector<float> >* customFilter, float ppmValue ) 
{
    int currentFilterIndex = customFilter->size() - 1; 
    bool isFilterComplete = true; 
    if ( currentFilterIndex >= 0 ) {
        int numRanges = (*customFilter)[currentFilterIndex].size();  
        if ( numRanges == 1 ) {
            isFilterComplete = false; 
        }
    }

    if ( isFilterComplete == true ) {
        //   if the filter doesn't exist, then add a new one. 
        vector< float >  tmpVector; 
        tmpVector.push_back( ppmValue ); 
        customFilter->push_back( tmpVector );  
    } else if ( isFilterComplete == false ) {
        //   if a filter exists but isn't complete, then complete it with the 2nd value: 
        (*customFilter)[currentFilterIndex].push_back(ppmValue); 
    } 
        
}
