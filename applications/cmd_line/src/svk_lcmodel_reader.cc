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
#include <svkLCModelCSVReader.h>
#include <svkLCModelCoordReader.h>
#include <svkDICOMMRSWriter.h>
#include <svkDdfVolumeWriter.h>
#include <svkDcmHeader.h>
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


int main (int argc, char** argv)
{

    string usemsg("\n") ; 
    usemsg += "Version " + string(SVK_RELEASE_VERSION) + "\n";   
    usemsg += "svk_lcmodel_reader -i input_file_name -o output_file_root                            \n"; 
    usemsg += "                   --csv csv_file_name --coord coord_file_name --met met_name        \n"; 
    usemsg += "                   --met met_name                                                    \n"; 
    usemsg += "                   [ -bh ]                                                           \n";
    usemsg += "                                                                                     \n";  
    usemsg += "   -i        name        Name of template MRS file.                                  \n"; 
    usemsg += "   -o        name        Root name of outputfile.                                    \n";
    usemsg += "   --csv     name        Name of csv file to convert.                                \n";
    usemsg += "                         (specify 1 and all will be read.)                           \n";
    usemsg += "   --coord   name        Name of coord file to convert                               \n";
    usemsg += "                         (specify 1 and all will be read.)                           \n";
    usemsg += "   --met     met_name    Name of met to convert (specify 1 and all will be read.)    \n";
    usemsg += "   -b                    Set up for selection box analysis only.                     \n";
    usemsg += "   -h                    Print this help mesage.                                     \n";  
    usemsg += "                                                                                     \n";  
    usemsg += "Reads LCModel .csv output and converts to DICOM metabolite maps, and/or reads in     \n"; 
    usemsg += "LCModel .coord files and converts the fitted spectra and phased data into MRS DCM    \n";  
    usemsg += "output files.                                                                        \n";  
    usemsg += "                                                                                     \n";  

    string  inputFileName; 
    string  outputFileName;
    string  csvFileName;
    string  coordFileName;
    string  metName;

    string cmdLine = svkProvenance::GetCommandLineString( argc, argv ); 

    enum FLAG_NAME {
        FLAG_CSV_FILE, 
        FLAG_COORD_FILE, 
        FLAG_MET_NAME, 
    }; 

    static struct option long_options[] =
    {
        /* This option sets a flag. */
        {"csv",         required_argument, NULL,  FLAG_CSV_FILE}, 
        {"coord",       required_argument, NULL,  FLAG_COORD_FILE}, 
        {"met",         required_argument, NULL,  FLAG_MET_NAME}, 
        {0, 0, 0, 0}
    };

    // ===============================================  
    //  Process flags and arguments
    // ===============================================  
    int i;
    int option_index = 0; 
    while ( ( i = getopt_long(argc, argv, "i:o:bh", long_options, &option_index) ) != EOF) {
        switch (i) {
            case 'i':
                inputFileName.assign( optarg );
                break;
            case 'o':
                outputFileName.assign(optarg);
                break;
            case FLAG_CSV_FILE:
                csvFileName.assign( optarg );
                break;
            case FLAG_COORD_FILE:
                coordFileName.assign( optarg );
                break;
            case FLAG_MET_NAME:
                metName.assign( optarg );
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

    if ( 
        argc != 0 ||  inputFileName.length() == 0  
        || outputFileName.length() == 0 
        || ( csvFileName.length() == 0 && coordFileName.length() == 0 ) 
        || metName.length() == 0 
    ) {
        cout << usemsg << endl;
        exit(1); 
    }

    if( ! svkUtils::FilePathExists( inputFileName.c_str() ) ) {
        cerr << "Input file can not be loaded (may not exist) " << inputFileName << endl; 
        exit(1); 
    }

    if ( csvFileName.length() != 0 ) {
        if( ! svkUtils::FilePathExists( csvFileName.c_str() ) ) {
            cerr << "Input file can not be loaded (may not exist) " << csvFileName << endl; 
            exit(1); 
        }
        cout << "file name: " << inputFileName << endl;
        cout << "csv file name: " << inputFileName << endl;

        // ===============================================  
        // ===============================================  
        svkLCModelCSVReader* csvReader = svkLCModelCSVReader::New(); 
        csvReader->SetMetName( metName ); 
        csvReader->SetFileName(csvFileName.c_str() ); 
        csvReader->SetMRSFileName(inputFileName.c_str() ); 
        csvReader->Update(); 
    
        svkImageData* currentImage = svkMriImageData::SafeDownCast( csvReader->GetOutput() ); 

        // ===============================================  
        //  Write the data out to the specified file type.  
        //  Use an svkImageWriterFactory to obtain the
        //  correct writer type. 
        // ===============================================  
        vtkSmartPointer< svkImageWriterFactory > writerFactory = vtkSmartPointer< svkImageWriterFactory >::New(); 
        svkImageWriter* mapWriter = static_cast<svkImageWriter*>( 
            writerFactory->CreateImageWriter( svkImageWriterFactory::DICOM_ENHANCED_MRI) ); 

        if ( mapWriter == NULL ) {
            cerr << "Can not create writer of type: " << svkImageWriterFactory::DICOM_ENHANCED_MRI << endl;
            exit(1);
        }

        mapWriter->SetFileName( outputFileName.c_str() );
        mapWriter->SetInput(  currentImage  );

        // ===============================================  
        //  Set the input command line into the data set 
        //  provenance: 
        // ===============================================  
        csvReader->GetOutput()->GetProvenance()->SetApplicationCommand( cmdLine );

        // ===============================================  
        //  Write data to file: 
        // ===============================================  
        mapWriter->Write();

        // ===============================================  
        //  Clean up: 
        // ===============================================  
        mapWriter->Delete();
        csvReader->Delete();
    }


    if ( coordFileName.length() != 0 ) {
        if( ! svkUtils::FilePathExists( coordFileName.c_str() ) ) {
            cerr << "Input file can not be loaded (may not exist) " << coordFileName << endl; 
            exit(1); 
        }
        cout << "coord file name: " << coordFileName << endl;


        // ===============================================  
        //  Write out the phased input data, fitted data, and residual: 
        // ===============================================  
        vector<string> coordStrings;  
        coordStrings.push_back("phased data points follow"); 
        coordStrings.push_back("fit to the data follow"); 
        vector<string> suffixStrings;  
        suffixStrings.push_back("_LCM_phased"); 
        suffixStrings.push_back("_LCM_fit"); 
        for ( int i = 0; i < coordStrings.size(); i++ ) {

            svkLCModelCoordReader* coordReader = svkLCModelCoordReader::New(); 
            coordReader->SetFileName(coordFileName.c_str() ); 
            coordReader->SetMRSFileName(inputFileName.c_str() ); 
            coordReader->SetDataStartDelimiter( coordStrings[i] );
            coordReader->Update(); 

            // ===============================================  
            //  Write the data out to the specified file type.  
            //  Use an svkImageWriterFactory to obtain the
            //  correct writer type. 
            // ===============================================  
            vtkSmartPointer< svkImageWriterFactory > writerFactory = vtkSmartPointer< svkImageWriterFactory >::New(); 
            svkImageWriter* mrsWriter = static_cast<svkImageWriter*>( 
                writerFactory->CreateImageWriter( svkImageWriterFactory::DICOM_MRS) ); 
    
            if ( mrsWriter == NULL ) {
                cerr << "Can not create writer of type: " << svkImageWriterFactory::DICOM_MRS << endl;
                exit(1);
            }

            string fn = outputFileName + suffixStrings[i]; 
            mrsWriter->SetFileName( fn.c_str()  );
            mrsWriter->SetInput(  coordReader->GetOutput() );

            // ===============================================  
            //  Set the input command line into the data set 
            //  provenance: 
            // ===============================================  
            coordReader->GetOutput()->GetProvenance()->SetApplicationCommand( cmdLine );

            // ===============================================  
            //  Write data to file: 
            // ===============================================  
            mrsWriter->Write();

            // ===============================================  
            //  Clean up: 
            // ===============================================  
            mrsWriter->Delete();
            coordReader->Delete();
        }

    }


    return 0; 
}

