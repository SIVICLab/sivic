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
#include <svkImageAlgorithm.h>
#ifdef WIN32
extern "C" {
#include <getopt.h>
}
#else
#include <getopt.h>
#endif
#define UNDEFINED_TEMP -1111

using namespace svk;



int main (int argc, char** argv)
{

    string usemsg("\n") ; 
    usemsg += "Version " + string(SVK_RELEASE_VERSION) +                                   "\n";   
    usemsg += "svk_gepfile_anon   -i input_file_name                                        \n"; 
    usemsg += "                   (--deid_type type                                         \n";
    usemsg += "                         [ --deid_id id ||                                   \n";
    usemsg += "                           --pat_deid_id patID --study_deid_id studyID ]     \n";
    usemsg += "                         [ --deid_studyUID uid]                              \n";
    usemsg += "                         [ --deid_seriesUID uid]                             \n";
    usemsg += "                   )                                                         \n";
    usemsg += "                   ||                                                        \n";
    usemsg += "                   (-i input_file_name -f field_name [-v value])             \n"; 
    usemsg += "                                                                             \n";  
    usemsg += "   -i                file_name   Name of GE Pfile to modify.                 \n"; 
    usemsg += "   --deid_type       type        Type of deidentification:                   \n";  
    usemsg += "                                     1 = limited                             \n";  
    usemsg += "                                     2 = deidentified(default)               \n";  
    usemsg += "   --deid_id         id          Use the specified study id in place of      \n"; 
    usemsg += "                                 all PHI fields(default = DEIDENTIFIED).     \n"; 
    usemsg += "                                 UIDs are generated or specified separately. \n"; 
    usemsg += "   --pat_deid_id     patID       Use the specified patient id in place of    \n"; 
    usemsg += "                                 patientID fields.                           \n"; 
    usemsg += "   --study_deid_id   studyID     Use the specified study id in place of      \n"; 
    usemsg += "                                 exam fields.                                \n"; 
    usemsg += "   --study_uid       uid         Use the specified Study UID to deidentify.  \n"; 
    usemsg += "   --series_uid      uid         Use the specified Series UID to deidentify. \n"; 
    usemsg += "   -f                field_name  Specific field to modify,                   \n"; 
    usemsg += "                                 e.g. rhs.series_desc                        \n";
    usemsg += "   -v                value       Field value.  If not specified will         \n"; 
    usemsg += "                                 clear the field.                            \n"; 
    usemsg += "   -h                            Print this help mesage.                     \n";  
    usemsg += "                                                                             \n";  
    usemsg += "Program to anonymize or modify a GE PFile.                                   \n"; 
    usemsg += "Progam will either deidentify the file, or modify an individual field.       \n"; 
    usemsg += "With no options specified, the program will deid the specified               \n"; 
    usemsg += "specified raw file                                                           \n"; 
    usemsg += "\n";  

    string inputFileName; 
    svkDcmHeader::PHIType deidType = svkDcmHeader::PHI_DEIDENTIFIED; 
    string deidId      = ""; 
    string patDeidId   = ""; 
    string studyDeidId = ""; 
    string seriesUID   = ""; 
    string studyUID    = ""; 
    string field       = ""; 
    string value       = ""; 

    string cmdLine = svkProvenance::GetCommandLineString( argc, argv ); 

    enum FLAG_NAME {
        FLAG_DEID_TYPE, 
        FLAG_DEID_ID, 
        FLAG_PAT_DEID_ID, 
        FLAG_STUDY_DEID_ID, 
        FLAG_STUDY_UID,
        FLAG_SERIES_UID
    }; 


    static struct option long_options[] =
    {
        // set long flags: 
        {"deid_type",        required_argument, NULL,  FLAG_DEID_TYPE},
        {"deid_id",          required_argument, NULL,  FLAG_DEID_ID},
        {"pat_deid_id",      required_argument, NULL,  FLAG_PAT_DEID_ID},
        {"study_deid_id",    required_argument, NULL,  FLAG_STUDY_DEID_ID},
        {"study_uid",        required_argument, NULL,  FLAG_STUDY_UID}, 
        {"series_uid",       required_argument, NULL,  FLAG_SERIES_UID}, 
        {0, 0, 0, 0}
    };



    // ===============================================  
    //  Process flags and arguments
    // ===============================================  
    int i;
    int option_index = 0; 
    while ( ( i = getopt_long(argc, argv, "i:f:v:h", long_options, &option_index) ) != EOF) {
        switch (i) {
            case 'i':
                inputFileName.assign( optarg );
                break;
            case FLAG_DEID_TYPE:
                deidType = static_cast<svkDcmHeader::PHIType>(atoi( optarg));  
                break;
            case FLAG_DEID_ID:
                deidId.assign( optarg ); 
                break;
            case FLAG_PAT_DEID_ID:
                patDeidId.assign( optarg ); 
                break;
            case FLAG_STUDY_DEID_ID:
                studyDeidId.assign( optarg ); 
                break;
            case FLAG_STUDY_UID:
                studyUID.assign( optarg ); 
                break;
            case FLAG_SERIES_UID:
                seriesUID.assign( optarg ); 
                break;
            case 'f':
                field.assign( optarg );
                break;
            case 'v':
                value.assign( optarg );
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
    //      an input file name was supplied
    //      that either a field or deidType and ID were specified. 
    // ===============================================  
    if ( argc != 0 || inputFileName.length() == 0 ) {
        cout << "An input file must be specified. " << endl; 
        cout << usemsg << endl;
        exit(1); 
    } 

    // ================================================================================  
    // ================================================================================  
    //  Either modify a single field if specified, or 
    //  deintify the file. 
    // ================================================================================  
    // ================================================================================  

    // ===============================================  
    // ===============================================  
    //  Modify a specific field: 
    // ===============================================  
    // ===============================================  
    if ( field.compare("") != 0 ) {
        vtkSmartPointer< svkImageReaderFactory > readerFactory = vtkSmartPointer< svkImageReaderFactory >::New(); 
        readerFactory->QuickParse();
        svkGEPFileReader* reader = svkGEPFileReader::SafeDownCast( 
                readerFactory->CreateImageReader2(inputFileName.c_str()) );
        if (reader == NULL) {
            cerr << "Can not determine appropriate reader for: " << inputFileName << endl;
            exit(1);
        }
        reader->SetFileName( inputFileName.c_str() );
        reader->OnlyReadOneInputFile();
        reader->ModifyRawField(field, value);
        reader->Delete();
        exit(0);
    }


    // ===============================================  
    // ===============================================  
    //  Deidentify the file. 
    // ===============================================  
    // ===============================================  

    // ===============================================  
    //  validate deidentification args:  
    // ===============================================  
    if ( deidType != svkDcmHeader::PHI_DEIDENTIFIED &&
         deidType != svkDcmHeader::PHI_LIMITED)
    {
        cout << "Error: invalid deidentificatin type: " << deidType <<  endl;
        cout << usemsg << endl;
        exit(1); 
    } else {
        cout << "Deidentify data: type = " << deidType << endl; 
    }

    if ( deidId.length() > 0 ) {
        if ( patDeidId.length() > 0 || studyDeidId.length() > 0 ) {
            cout << "Error: specify a global deid param or pat and stud deid params, not both. "<<  endl;
            cout << usemsg << endl;
            exit(1); 
        }
    }
    if ( deidId.length() ==  0 ) {
        if ( patDeidId.length() == 0 || studyDeidId.length() == 0 ) {
            cout << "Error: specify a global deid param or pat and stud deid params, not both. "<<  endl;
            cout << usemsg << endl;
            exit(1); 
        }
    }


    // ===============================================  
    //  Set up deidentification options: 
    // ===============================================  
    vtkSmartPointer< svkImageReaderFactory > readerFactory = vtkSmartPointer< svkImageReaderFactory >::New(); 
    readerFactory->QuickParse();
    svkGEPFileReader* reader = svkGEPFileReader::SafeDownCast( 
                readerFactory->CreateImageReader2(inputFileName.c_str()) );
    if (reader == NULL) {
        cerr << "Can not determine appropriate reader for: " << inputFileName << endl;
        exit(1);
    }
    reader->SetFileName( inputFileName.c_str() );
    reader->OnlyReadOneInputFile();

    //  set type and id
    if ( deidId.compare("") != 0 ) {
        reader->SetDeidentify( deidType, deidId ); 
    } else if ( patDeidId.length() > 0 && studyDeidId.length() > 0 ) {
        reader->SetDeidentify( deidType, patDeidId, studyDeidId ); 
    } else {
        reader->SetDeidentify( deidType ); 
    }

    //  set UIDs 
    if ( studyUID.compare("") != 0 ) {
        reader->SetDeidentificationStudyUID( studyUID ); 
    }
    if ( seriesUID.compare("") != 0 ) {
        reader->SetDeidentificationSeriesUID( seriesUID ); 
    }

    reader->Deidentify();

    return 0; 
}

