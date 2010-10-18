/*
 *  Copyright © 2009-2010 The Regents of the University of California.
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
 */


#include <svkDcmMriVolumeReader.h>
#include <svkMriImageData.h>
//#include <vtkByteSwap.h>
#include <vtkType.h>
#include <vtkDebugLeaks.h>
#include <vtkGlobFileNames.h>
#include <vtkSortFileNames.h>
#include <vtkStringArray.h>
#include <vtkInformation.h>
#include <vtkMath.h>
#include <svkIOD.h>
#include <svkMRIIOD.h>
#include <vtkstd/vector>
#include <vtkstd/utility>
#include <vtkstd/algorithm>
#include <sstream>


using namespace svk;


vtkCxxRevisionMacro(svkDcmMriVolumeReader, "$Rev$");
vtkStandardNewMacro(svkDcmMriVolumeReader);


/*!
 *
 */
svkDcmMriVolumeReader::svkDcmMriVolumeReader()
{
#if VTK_DEBUG_ON
    this->DebugOn();
    vtkDebugLeaks::ConstructClass("svkDcmMriVolumeReader");
#endif
    vtkDebugMacro(<<this->GetClassName() << "::" << this->GetClassName() << "()");
}


/*!
 *
 */
svkDcmMriVolumeReader::~svkDcmMriVolumeReader()
{
    vtkDebugMacro(<<this->GetClassName() << "::~" << this->GetClassName() << "()");
}


/*!
 *  Mandator, must be overridden to get the factory to check for proper
 *  type of vtkImageReader2 to return. 
 */
int svkDcmMriVolumeReader::CanReadFile(const char* fname)
{

    vtkstd::string fileToCheck(fname);

    bool isDcmMri = false; 

    if ( svkDcmHeader::IsFileDICOM( fname ) ) {

        svkImageData* tmp = svkMriImageData::New(); 

        tmp->GetDcmHeader()->ReadDcmFile( fname );
        vtkstd::string SOPClassUID = tmp->GetDcmHeader()->GetStringValue( "SOPClassUID" ) ;

        //verify that this isn't a proprietary use of DICOM MR ImageStorage: 
        if ( this->ContainsProprietaryContent( tmp ) == false ) {
                    
            // Check for MR Image Storage
            if ( SOPClassUID == "1.2.840.10008.5.1.4.1.1.4" ) {           
                this->SetFileName(fname);
                isDcmMri = true; 
            }
        
        }

        tmp->Delete(); 
    }

    if ( isDcmMri ) {
        cout << this->GetClassName() << "::CanReadFile(): It's a DICOM MRI File: " <<  fileToCheck << endl;
        return 1;
    }

    vtkDebugMacro(<<this->GetClassName() << "::CanReadFile() It's Not a DICOM MRI file " << fileToCheck );

    return 0;
}


/*!
 *
 */
bool svkDcmMriVolumeReader::ContainsProprietaryContent( svkImageData* data )
{

    // Check if it contains GE spectroscopy content:
    bool containsProprietaryContent = false; 

    if ( data->GetDcmHeader()->ElementExists("Manufacturer") == true && data->GetDcmHeader()->ElementExists("ImagedNucleus") == true ) {
        vtkstd::string mfg = data->GetDcmHeader()->GetStringValue( "Manufacturer" ) ;
        vtkstd::string imagedNucleus = data->GetDcmHeader()->GetStringValue( "ImagedNucleus" ) ;

        if ( mfg == "GE MEDICAL SYSTEMS" && imagedNucleus == "SPECT" ) {
            containsProprietaryContent = true; 
        } 
    }

    return containsProprietaryContent; 
}


/*!
 *  sort compare utility method 
 */
float GetFloatVal( vtkstd::vector< vtkstd::string > vec ) 
{
    istringstream* posString = new istringstream();

    //  4th element of vector is the projection of the 
    //  ImagePositionPatient on the normal through the origin
    posString->str( vec[4] );    
    float floatPosition; 
    *posString >> floatPosition; 

    delete posString; 

    return floatPosition; 
}


//  sort ascending slice order
bool SortAscend( vtkstd::vector< vtkstd::string > first, vtkstd::vector < vtkstd::string> second )
{
    return GetFloatVal( first ) < GetFloatVal( second );  
}


//  sort descending slice order
bool SortDescend( vtkstd::vector< vtkstd::string > first, vtkstd::vector < vtkstd::string> second )
{
    return GetFloatVal( first ) > GetFloatVal( second );  
}


/*!
 *  Sort the list of files in either ascending or descending order by ImagePositionPatient
 */
void svkDcmMriVolumeReader::SortFilesByImagePositionPatient(
        vtkstd::vector< vtkstd::vector< vtkstd::string> >& dcmSeriesAttributes, 
        bool ascending
    )
{

    if ( ascending ) {
        vtkstd::sort( dcmSeriesAttributes.begin(), dcmSeriesAttributes.end(), SortAscend ); 
    } else {
        vtkstd::sort( dcmSeriesAttributes.begin(), dcmSeriesAttributes.end(), SortDescend ); 
    }

    return;
}



void svkDcmMriVolumeReader::InitFileNames()
{
    vtkstd::string dcmFileName( this->GetFileName() );
    vtkstd::string dcmFilePath( this->GetFilePath( this->GetFileName() ) );  

    vtkGlobFileNames* globFileNames = vtkGlobFileNames::New();
    globFileNames->AddFileNames( vtkstd::string( dcmFilePath + "/*").c_str() );

    vtkSortFileNames* sortFileNames = vtkSortFileNames::New();
    sortFileNames->SetInputFileNames( globFileNames->GetFileNames() );
    sortFileNames->NumericSortOn();
    sortFileNames->SkipDirectoriesOn();
    sortFileNames->Update();

    //  Get the reference file's SeriesInstanceUID, ImageOrientationPatient and slice normal.
    //  These are used for parsing the glob'd files.  
    svkImageData* tmp = svkMriImageData::New(); 
    tmp->GetDcmHeader()->ReadDcmFile(  this->GetFileName() );
    vtkstd::string imageOrientationPatient( tmp->GetDcmHeader()->GetStringValue("ImageOrientationPatient"));
    vtkstd::string seriesInstanceUID( tmp->GetDcmHeader()->GetStringValue("SeriesInstanceUID"));
    double normal[3]; 
    tmp->GetDcmHeader()->GetNormalVector( normal );
    tmp->Delete();

    vtkStringArray* fileNames =  sortFileNames->GetFileNames();

    //  1.  Select only DICOM images from glob.  Non-DICOM image parsing will cause problems
    //  2.  Ensure that the list of files all belong to the same series (have the same 
    //      SeriesInstanceUID) as the selected input image. 
    //  3.  Run some checks to ensure that the series comprises data from a single volume: 
    //          - all have same ImageOrientationPatient.
    //          - no duplicated ImagePositionPatient values
    //  4.  Sort the resulting list of files by location (ascending/descending). 


    //  create a container to hold file names and dcm attributes, so 
    //  DCM parsing is only done once: 
    //  fileName, SeriesInstanceUID, ImageOrientationPatient, ImagePositionPatient, projectionOnNormal 
    //      the projection onto the normal of the input image orientation is used for sorting slice 
    //      order. 
    vtkstd::vector < vtkstd::vector< vtkstd::string > > dcmSeriesAttributes; 

    for (int i = 0; i < fileNames->GetNumberOfValues(); i++) {

        //
        //  1.  only include DICOM files 
        //
        if (  svkDcmHeader::IsFileDICOM( fileNames->GetValue(i) ) ) {

            vtkstd::vector< vtkstd::string > dcmFileAttributes;  

            tmp = svkMriImageData::New(); 
            tmp->GetDcmHeader()->ReadDcmFile( fileNames->GetValue(i) );
            dcmFileAttributes.push_back( fileNames->GetValue(i) ); 
            dcmFileAttributes.push_back( tmp->GetDcmHeader()->GetStringValue( "SeriesInstanceUID" ) );
            dcmFileAttributes.push_back( tmp->GetDcmHeader()->GetStringValue( "ImageOrientationPatient" ) );
            dcmFileAttributes.push_back( tmp->GetDcmHeader()->GetStringValue( "ImagePositionPatient" ) );

            double position[3];   
            for (int i = 0; i < 3; i++ ) {
                vtkstd::string pos( tmp->GetDcmHeader()->GetStringValue("ImagePositionPatient", i));
                std::istringstream positionInString(pos);
                positionInString >> position[i];
            }

            //  Project posLPS onto the normal vector through the
            //  origin of world coordinate space (this is the same
            //  reference for both data sets (e.g MRI and MRS).
            double projectionOnNormal = vtkMath::Dot( position, normal );
            ostringstream projectionOss;
            projectionOss << projectionOnNormal;
            dcmFileAttributes.push_back( projectionOss.str() ); 

            //  add the file attributes to the series attribute vector:
            dcmSeriesAttributes.push_back( dcmFileAttributes ); 

            tmp->Delete();

        }
    }

    //  ======================================================================
    //  Now validate that the DICOM files comprise a single volumetric series
    //  wrt seriesUID and orientation: 
    //  ======================================================================
    vtkstd::vector < vtkstd::vector< vtkstd::string > >::iterator seriesIt; 
    seriesIt = dcmSeriesAttributes.begin(); 

    while ( seriesIt != dcmSeriesAttributes.end() ) { 

        vtkstd::string tmpSeriesInstanceUID = (*seriesIt)[1]; 
        vtkstd::string tmpImageOrientationPatient = (*seriesIt)[2]; 
        //  if seriesUID doesnt match, remove it:
        if ( tmpImageOrientationPatient == imageOrientationPatient ) {

            if ( tmpSeriesInstanceUID != seriesInstanceUID ) {
                vtkWarningWithObjectMacro(
                    this, 
                    "SeriesInstanceUID is not the same for all slices in directory , using only specified file "
                );
                seriesIt = dcmSeriesAttributes.erase( seriesIt ); 
            } else {
                seriesIt++; 
            }

        } else {

            //  Orientations don't match, set series to one input file
            vtkWarningWithObjectMacro(this, "ImageOrientationPatient is not the same for all slices, using only specified file ");
            this->OnlyReadInputFile(); 
            return; 
        }
    }

    //  ============================
    //  Is Data multi-volumetric:
    //  ============================
    set < vtkstd::string > uniqueSlices;
    seriesIt = dcmSeriesAttributes.begin(); 
    while ( seriesIt != dcmSeriesAttributes.end() ) { 
        uniqueSlices.insert( (*seriesIt)[3] ); 
        seriesIt++ ;
    }
    if ( dcmSeriesAttributes.size() > uniqueSlices.size() ) {
        //  More than one file per slice:  
        vtkWarningWithObjectMacro(this, "Multi-volumetric data, repeated slice locations"); 
        this->OnlyReadInputFile(); 
        return; 
    }

    //  ======================================================================
    //  Now sort the files according to ImagePositionPatient.
    //  ======================================================================
    this->SortFilesByImagePositionPatient( dcmSeriesAttributes, false); 

    // Finally, repopulate the file list.
    fileNames->SetNumberOfValues( dcmSeriesAttributes.size() );
    for ( int i = 0; i < dcmSeriesAttributes.size(); i++ ) { 
        if (this->GetDebug()) {
            cout << "FNS in series: " << dcmSeriesAttributes[i][0] << endl; 
        }
        fileNames->SetValue( i, dcmSeriesAttributes[i][0] ); 
    }

    // Calling this method will set the DataExtents for the slice direction
    //  should set these from the sorted dcmSeriesAttributes: 
    this->SetFileNames( sortFileNames->GetFileNames() );

    globFileNames->Delete();
    sortFileNames->Delete();

} 


/*!
 *  Reset file names to only the single input file name
 */
void svkDcmMriVolumeReader::OnlyReadInputFile()
{
    vtkStringArray* tmpFileNames = vtkStringArray::New();
    tmpFileNames->SetNumberOfValues(1);
    tmpFileNames->SetValue(0, this->GetFileName() );
    tmpFileNames->Delete();

    // Calling this method will set the DataExtents for the slice direction
    this->SetFileNames( tmpFileNames );

    tmpFileNames->Delete(); 
}


/*!
 *
 */
void svkDcmMriVolumeReader::InitDcmHeader()
{

    this->InitFileNames(); 

    // Read the first file and load the header as the starting point
    this->GetOutput()->GetDcmHeader()->ReadDcmFile( this->GetFileNames()->GetValue(0) );

    vtkstd::string studyInstanceUID( this->GetOutput()->GetDcmHeader()->GetStringValue("StudyInstanceUID"));
    int rows    = this->GetOutput()->GetDcmHeader()->GetIntValue( "Rows" ); // Y
    int columns = this->GetOutput()->GetDcmHeader()->GetIntValue( "Columns" ); // X
     
    //  Now override elements with Multi-Frame sequences and default details:
    svkIOD* iod = svkMRIIOD::New();
    iod->SetDcmHeader( this->GetOutput()->GetDcmHeader());
    iod->SetReplaceOldElements(false); 
    iod->InitDcmHeader();
    iod->Delete();


    this->GetOutput()->GetDcmHeader()->SetValue( "StudyInstanceUID", studyInstanceUID.c_str() );


    //  Now move info from original MRImageStorage header elements to flesh out enhanced
    //  SOP class elements (often this is just a matter of copying elements from the top 
    //  level to a sequence item. 
    this->InitMultiFrameFunctionalGroupsModule(); 

    /*
     *  odds and ends: 
     */
    this->GetOutput()->GetDcmHeader()->SetValue( 
        "Rows", 
        rows 
    );
    this->GetOutput()->GetDcmHeader()->SetValue( 
        "Columns", 
        columns 
    );

    if (this->GetDebug()) {
        this->GetOutput()->GetDcmHeader()->PrintDcmHeader(); 
    }
    
}


/*! 
 *  Init the Shared and PerFrame sequences and load the pixel data into the svkImageData object. 
 */
void svkDcmMriVolumeReader::LoadData( svkImageData* data )
{
    void *imageData = data->GetScalarPointer();

    int rows    = this->GetOutput()->GetDcmHeader()->GetIntValue( "Rows" ); 
    int columns = this->GetOutput()->GetDcmHeader()->GetIntValue( "Columns" ); 
    int numPixelsInSlice = rows * columns; 

    /*
     *  Iterate over slices (frames) and copy ImagePositions
     */
    for (int i = 0; i < this->GetFileNames()->GetNumberOfValues(); i++) { 
        svkImageData* tmpImage = svkMriImageData::New(); 
        tmpImage->GetDcmHeader()->ReadDcmFile( this->GetFileNames()->GetValue( i ) ); 
        tmpImage->GetDcmHeader()->GetShortValue( "PixelData", ((short *)imageData) + (i * numPixelsInSlice), numPixelsInSlice );
        tmpImage->Delete(); 
    }

}


/*!
 *
 */
void svkDcmMriVolumeReader::InitMultiFrameFunctionalGroupsModule()
{

    this->numFrames =  this->GetFileNames()->GetNumberOfValues(); 

    this->GetOutput()->GetDcmHeader()->SetValue(
        "NumberOfFrames",
        this->numFrames
    );

    this->InitSharedFunctionalGroupMacros();
    this->InitPerFrameFunctionalGroupMacros();

}


/*!
 *
 */
void svkDcmMriVolumeReader::InitSharedFunctionalGroupMacros()
{
    this->InitPixelMeasuresMacro();
    this->InitPlaneOrientationMacro();
    this->InitMRReceiveCoilMacro();
}


/*!
 *  Pixel Spacing:
 */
void svkDcmMriVolumeReader::InitPixelMeasuresMacro()
{

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "SharedFunctionalGroupsSequence",
        0,
        "PixelMeasuresSequence"
    );

    svkImageData* tmp = svkMriImageData::New(); 
    tmp->GetDcmHeader()->ReadDcmFile(  this->GetFileNames()->GetValue( 0 ) ); 

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "PixelMeasuresSequence",
        0,
        "PixelSpacing",
        tmp->GetDcmHeader()->GetStringValue( "PixelSpacing" ), 
        "SharedFunctionalGroupsSequence",
        0
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "PixelMeasuresSequence",
        0,
        "SliceThickness",
        tmp->GetDcmHeader()->GetStringValue( "SliceThickness" ), 
        "SharedFunctionalGroupsSequence",
        0
    );

    tmp->Delete(); 
}


/*!
 *
 */
void svkDcmMriVolumeReader::InitPlaneOrientationMacro()
{

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "SharedFunctionalGroupsSequence",
        0,
        "PlaneOrientationSequence"
    );

    svkImageData* tmp = svkMriImageData::New(); 
    tmp->GetDcmHeader()->ReadDcmFile(  this->GetFileNames()->GetValue( 0 ) ); 

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "PlaneOrientationSequence",
        0,
        "ImageOrientationPatient",
        tmp->GetDcmHeader()->GetStringValue( "ImageOrientationPatient" ), 
        "SharedFunctionalGroupsSequence",
        0
    );

    tmp->Delete(); 

}


/*!
 *  Receive Coil:
 */
void svkDcmMriVolumeReader::InitMRReceiveCoilMacro()
{

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "SharedFunctionalGroupsSequence",
        0,
        "MRReceiveCoilSequence"
    );

    svkImageData* tmp = svkMriImageData::New(); 
    tmp->GetDcmHeader()->ReadDcmFile(  this->GetFileNames()->GetValue( 0 ) ); 

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRReceiveCoilSequence",
        0,
        "ReceiveCoilName",
        tmp->GetDcmHeader()->GetStringValue( "ReceiveCoilName" ), 
        "SharedFunctionalGroupsSequence",
        0
    );

    tmp->Delete(); 
}


/*!
 *
 */
void svkDcmMriVolumeReader::InitPerFrameFunctionalGroupMacros()
{
    this->InitFrameContentMacro();
    this->InitPlanePositionMacro();
}


/*!
 *  Mandatory, Must be a per-frame functional group
 */
void svkDcmMriVolumeReader::InitFrameContentMacro()
{
    for (int i = 0; i < this->numFrames; i++) {

        this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
            "PerFrameFunctionalGroupsSequence",
            i,
            "FrameContentSequence"
        );

        this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
            "FrameContentSequence",
            0,
            "FrameAcquisitionDatetime",
            "EMPTY_ELEMENT",
            "PerFrameFunctionalGroupsSequence",
            i
        );

        this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
            "FrameContentSequence",
            0,
            "FrameReferenceDatetime",
            "EMPTY_ELEMENT",
            "PerFrameFunctionalGroupsSequence",
            i
        );

        this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
            "FrameContentSequence",
            0,
            "FrameAcquisitionDuration",
            "-1",
            "PerFrameFunctionalGroupsSequence",
            i
        );
    }
}


/*!
 *  The FDF toplc is the center of the first voxel.
 */
void svkDcmMriVolumeReader::InitPlanePositionMacro()
{

    /*
     *  Iterate over slices (frames) and copy ImagePositions
     */
    for (int i = 0; i < this->GetFileNames()->GetNumberOfValues(); i++) { 

        this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
            "PerFrameFunctionalGroupsSequence",
            i,
            "PlanePositionSequence"
        );

        svkImageData* tmpImage = svkMriImageData::New(); 
        tmpImage->GetDcmHeader()->ReadDcmFile( this->GetFileNames()->GetValue( i ) ); 

        this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
            "PlanePositionSequence",
            0,
            "ImagePositionPatient", 
            tmpImage->GetDcmHeader()->GetStringValue( "ImagePositionPatient" ),
            "PerFrameFunctionalGroupsSequence",
            i
        );

        tmpImage->Delete(); 
    }
}


/*!
 *  Returns the pixel type 
 */
svkDcmHeader::DcmPixelDataFormat svkDcmMriVolumeReader::GetFileType()
{
    return svkDcmHeader::UNSIGNED_INT_2;
}


/*!
 *
 */
int svkDcmMriVolumeReader::FillOutputPortInformation( int vtkNotUsed(port), vtkInformation* info )
{
    info->Set(vtkDataObject::DATA_TYPE_NAME(), "svkMriImageData");
    return 1;
}

