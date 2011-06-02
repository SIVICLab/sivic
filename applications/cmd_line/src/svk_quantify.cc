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
 *  Utility application for converting between supported file formats. 
 *
 */


#include <vtkSmartPointer.h>

#include <svkImageReaderFactory.h>
#include <svkImageReader2.h>
#include <svkImageWriterFactory.h>
#include <svkImageWriter.h>
#include <svkIntegratePeak.h>
#include <svkExtractMRIFromMRS.h>
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
    usemsg += "svk_quantify -i input_file_name -o output_file_name -t output_data_type [-h] \n";
    usemsg += "\n";  
    usemsg += "   -i input_file_name        name of file to convert.            \n"; 
    usemsg += "   -o output_file_name       name of outputfile.                 \n";  
    usemsg += "   -t output_data_type       target data type:                   \n";  
    usemsg += "                                 3 = UCSF IDF                    \n";  
    usemsg += "                                 6 = DICOM_MRI                   \n";  
    usemsg += "   --peak_center       ppm   Chemical shift of peak center       \n";
    usemsg += "   --peak_width        ppm   Width in ppm of peak integration    \n";
    usemsg += "   --peak_name         name  String label name for peak          \n"; 
    usemsg += "   --verbose                 Prints pk ht and integrals for each voxel to stdout. \n"; 
    usemsg += "   --algo                    Quantification algorithm :          \n"; 
    usemsg += "                                 1 = Peak Ht (default)           \n";  
    usemsg += "                                 2 = Integration                 \n";  
    usemsg += "   -h                        print help mesage.                  \n";  
    usemsg += " \n";  
    usemsg += "Generates metabolite map volume by direct integration of input spectra over the specified chemical shift range. \n";
    usemsg += "\n";  

    string inputFileName; 
    string outputFileName; 
    svkImageWriterFactory::WriterType dataTypeOut = svkImageWriterFactory::UNDEFINED; 
    float  peakCenterPpm;
    float  peakWidthPpm;
    string peakName;
    bool   isVerbose = false;   
    svkExtractMRIFromMRS::algorithm algo = svkExtractMRIFromMRS::PEAK_HT; 
    int algoInt; 


    string cmdLine = svkProvenance::GetCommandLineString( argc, argv );

    enum FLAG_NAME {
        FLAG_PEAK_CENTER = 0,
        FLAG_PEAK_WIDTH, 
        FLAG_PEAK_NAME, 
        FLAG_ALGORITHM, 
        FLAG_VERBOSE  
    };


    static struct option long_options[] =
    {
        /* This option sets a flag. */
        {"peak_center",      required_argument, NULL,  FLAG_PEAK_CENTER},
        {"peak_width",       required_argument, NULL,  FLAG_PEAK_WIDTH},
        {"peak_name",        required_argument, NULL,  FLAG_PEAK_NAME},
        {"algorithm",        required_argument, NULL,  FLAG_ALGORITHM},
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
                outputFileName.assign(optarg);
                break;
            case 't':
                dataTypeOut = static_cast<svkImageWriterFactory::WriterType>( atoi(optarg) );
                break;
           case FLAG_PEAK_CENTER:
                peakCenterPpm = atof( optarg);
                break;
           case FLAG_PEAK_WIDTH:
                peakWidthPpm = atof( optarg);
                break;
           case FLAG_PEAK_NAME:
                peakName.assign( optarg);
                break;
           case FLAG_VERBOSE:
                isVerbose = true; 
                break;
           case FLAG_ALGORITHM:
                algoInt = atoi(optarg); 
                if ( algoInt == 1 ) {
                    algo = svkExtractMRIFromMRS::PEAK_HT; 
                } else if ( algoInt == 2 ) {
                    algo = svkExtractMRIFromMRS::INTEGRATE; 
                } else {
                    cout << "ERROR: invalid algorithm: " << algoInt << endl;
                    cout << usemsg << endl;
                    exit(1); 
                }
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


    if ( argc != 0 ||  inputFileName.length() == 0 || outputFileName.length() == 0 ||
        dataTypeOut < 0 || dataTypeOut >= svkImageWriterFactory::LAST_TYPE || 
        peakCenterPpm == UNDEFINED_VAL || peakWidthPpm == UNDEFINED_VAL || peakName.length() == 0 ) {
        cout << usemsg << endl;
        exit(1); 
    }

    //cout << inputFileName << endl;
    //cout << outputFileName << endl;
    //cout << dataTypeOut << endl;
    //cout << peakCenterPpm << endl;
    //cout << peakWidthPpm << endl;
    //cout << peakName<< endl;


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

    svkExtractMRIFromMRS* quant = svkExtractMRIFromMRS::New();
    quant->SetInput( reader->GetOutput() ); 
    if ( isVerbose ) { 
        quant->SetVerbose( isVerbose ); 
    }    
    quant->SetPeakPosPPM( peakCenterPpm );
    quant->SetPeakWidthPPM( peakWidthPpm );

    if ( isVerbose || algo == svkExtractMRIFromMRS::INTEGRATE ) {
        quant->SetAlgorithmToIntegrate();
        quant->SetSeriesDescription( peakName + "area metabolite map" ); 
        quant->Update();
    }

    if ( isVerbose || algo == svkExtractMRIFromMRS::PEAK_HT) {
        quant->SetAlgorithmToPeakHeight();
        quant->SetSeriesDescription( peakName + "peak ht metabolite map" ); 
        quant->Update();
    }

    //quant->GetOutput()->GetDcmHeader()->PrintDcmHeader();
    //cout << *( quant->GetOutput() ) << endl;

    // ===============================================  
    //  Write the data out to the specified file type.  
    //  Use an svkImageWriterFactory to obtain the
    //  correct writer type. 
    // ===============================================  
    if ( isVerbose == false ) {

        vtkSmartPointer< svkImageWriterFactory > writerFactory = vtkSmartPointer< svkImageWriterFactory >::New(); 
        svkImageWriter* writer = static_cast<svkImageWriter*>( writerFactory->CreateImageWriter( dataTypeOut ) ); 
    
        if ( writer == NULL ) {
            cerr << "Can not determine writer of type: " << dataTypeOut << endl;
            exit(1);
        }
   
        writer->SetFileName( outputFileName.c_str() );
        writer->SetInput( quant->GetOutput() );

        //  Set the input command line into the data set provenance:
        reader->GetOutput()->GetProvenance()->SetApplicationCommand( cmdLine );

        writer->Write();
    }

    quant->Delete(); 
    reader->Delete();

    return 0; 
}



