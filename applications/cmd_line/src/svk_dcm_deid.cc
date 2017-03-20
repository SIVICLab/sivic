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
#include <svkDICOMRawDataWriter.h>
#include <svkImageReaderFactory.h> 


using namespace svk;


int main (int argc, char** argv)
{

    string usemsg("\n") ; 
    usemsg += "Version " + string(SVK_RELEASE_VERSION) +                                   "\n";   
    usemsg += "svk_dcm_deid -i input_file_name                                              \n"; 
    usemsg += "             [ -r deid_id | -p pat_deid_id -s study_deid_id ]                \n"; 
    usemsg += "             [ --stu UID ] [ --seu UID ] [ --fru UID ] -h                    \n"; 
    usemsg += "             | -guid                                                         \n"; 
    usemsg += "                                                                             \n";  
    usemsg += "   -i    input_file_name     Name of dcm file to deidentify                  \n"; 
    usemsg += "   -r    deid_id             replacement id (patient and study level)        \n"; 
    usemsg += "   -p    pat_deid_id         replacement patient id                          \n"; 
    usemsg += "   -s    study_deid_id       replacement study id                            \n"; 
    usemsg += "   --stu UID                 use the specified StudyInstanceUID              \n";
    usemsg += "   --seu UID                 use the specified SeriesInstanceUID             \n";
    usemsg += "   --fru UID                 use the specified FrameOfReferenceUID           \n";
    usemsg += "   --guid                    generate a new unique UID, do nothing else.     \n";
    usemsg += "   -h                        Print help mesage.                              \n";  
    usemsg += "                                                                             \n";  
    usemsg += "Deidentify a DICOM file.                                                     \n"; 
    usemsg += "\n";  

    string inputFileName; 
    string deidID = ""; 
    string patDeidID = ""; 
    string studyDeidID = ""; 
    string studyUID = ""; 
    string seriesUID = ""; 
    string frameOfRefUID = ""; 
    bool   generateUID = false;  

    string cmdLine = svkProvenance::GetCommandLineString( argc, argv );

    enum FLAG_NAME {
        FLAG_STUDY_UID = 0,
        FLAG_SERIES_UID, 
        FLAG_FRAME_UID, 
        FLAG_GENERATE_UID
    };


    static struct option long_options[] =
    {
        /* This option sets a flag. */
        {"stu",             required_argument, NULL,  FLAG_STUDY_UID},
        {"seu",             required_argument, NULL,  FLAG_SERIES_UID},
        {"fru",             required_argument, NULL,  FLAG_FRAME_UID},
        {"guid",            no_argument,       NULL,  FLAG_GENERATE_UID},
        {0, 0, 0, 0}
    };


    /*
    *   Process flags and arguments
    */
    int i;
    int option_index = 0; 
    while ((i = getopt_long(argc, argv, "i:r:p:s:h", long_options, &option_index)) != EOF) {
        switch (i) {
            case 'i':
                inputFileName.assign( optarg );
                break;
            case 'r':
                deidID.assign( optarg );
                break;
            case 'p':
                patDeidID.assign( optarg );
                break;
            case 's':
                studyDeidID.assign( optarg );
                break;
            case FLAG_STUDY_UID:
                studyUID.assign(optarg); 
                break;
            case FLAG_SERIES_UID:
                seriesUID.assign( optarg ); 
                break;
            case FLAG_FRAME_UID:
                frameOfRefUID.assign( optarg ); 
                break;
            case FLAG_GENERATE_UID:
                generateUID = true; 
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
        ( ( inputFileName.length() == 0 ) && ( generateUID == false ) ) ||  
        ( ( inputFileName.length() != 0 ) && ( generateUID == true  ) )  )
    {  
        cout << "ERROR:  Specify either an input file or the guid flag" << endl;
        cout << usemsg << endl;
        exit(1); 
    }

    if ( ( ( patDeidID.length() != 0 ) || ( studyDeidID.length() != 0 ) ) && (patDeidID.length() * studyDeidID.length() == 0 ) ) {
        cout << "ERROR:  must specify both pat_deid_id and study_deid_id" << endl;
        cout << usemsg << endl;
        exit(1); 
    }
    if ( deidID.length() != 0 &&  patDeidID.length() != 0 ) {
        cout << "ERROR:  specify either deid_id, or pat_deid_id and study_deid_id" << endl;
        cout << usemsg << endl;
        exit(1); 
    }
    if ( argc != 0 ) {
        cout << usemsg << endl;
        exit(1); 
    }

    if ( generateUID == true ) {
        svkImageData* image = svkMriImageData::New();
        string newUID = image->GetDcmHeader()->GenerateUniqueUID(); 
        cout << "NEW UID: " << newUID << endl;
        return 0; 
    }

    svkImageData* image = svkMriImageData::New(); 
    image->GetDcmHeader()->ReadDcmFile( inputFileName, VTK_UNSIGNED_INT_MAX );
    if ( deidID.length() != 0 ) {
        image->GetDcmHeader()->Deidentify(svkDcmHeader::PHI_DEIDENTIFIED, deidID); 
    } else if ( patDeidID.length() !=0 ) {
        image->GetDcmHeader()->Deidentify(svkDcmHeader::PHI_DEIDENTIFIED, patDeidID, studyDeidID); 
    } else {
        image->GetDcmHeader()->Deidentify(svkDcmHeader::PHI_DEIDENTIFIED); 
    }
    if ( studyUID.size() == 0 ) {
        image->GetDcmHeader()->InsertUniqueUID("StudyInstanceUID");
    } else {
        image->GetDcmHeader()->SetValue("StudyInstanceUID", studyUID );
    }
    if ( seriesUID.size() == 0 ) {
        image->GetDcmHeader()->InsertUniqueUID("SeriesInstanceUID");
    } else {
        image->GetDcmHeader()->SetValue("SeriesInstanceUID", seriesUID );
    }
    if ( frameOfRefUID.size() == 0 ) {
        image->GetDcmHeader()->InsertUniqueUID("FrameOfReferenceUID");
    } else {
        image->GetDcmHeader()->SetValue("FrameOfReferenceUID", frameOfRefUID );
    }

    string newUID = "";
    newUID = image->GetDcmHeader()->GenerateUniqueUID();
    image->GetDcmHeader()->ModifyValueRecursive( "MediaStorageSOPInstanceUID",    newUID);
    image->GetDcmHeader()->ModifyValueRecursive( "SOPInstanceUID",                newUID);
    if ( image->GetDcmHeader()->GetOriginalXFerSyntax() == 22 ) {
        image->GetDcmHeader()->WriteDcmFileCompressed(inputFileName); 
    } else {
        image->GetDcmHeader()->WriteDcmFile(inputFileName); 
    }
    image->Delete(); 
        

    return 0; 
}

