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

    svkImageData* tmp = svkMriImageData::New(); 
    bool isDcmMri = false; 

    if ( tmp->GetDcmHeader()->IsFileDICOM( fname ) ) {

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
    }

    tmp->Delete(); 
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
 *
 */
struct svkDcmMriVolumeReaderSort_lt_pair_double_string
{
  bool operator()(const vtkstd::pair<double, vtkstd::string> s1, 
                  const vtkstd::pair<double, vtkstd::string> s2) const
  {
    return s1.first < s2.first;
  }
};


/*!
 *
 */
struct svkDcmMriVolumeReaderSort_gt_pair_double_string
{
  bool operator()(const vtkstd::pair<double, vtkstd::string> s1, 
                  const vtkstd::pair<double, vtkstd::string> s2) const
  {
    return s1.first > s2.first;
  }
};


/*!
 * Sort the list of files in either ascending or descending order by ImagePositionPatient
 * and SeriesInstanceUID.  If the SeriesInstanceUID is not the same as the input, the file
 * is removed from the list.  Also return true if this is a multi-volume series. 
 */
bool svkDcmMriVolumeReaderSortFilesByImagePositionPatient(const vtkstd::string & seriesInstanceUID, 
    vtkStringArray* fileNames, bool ascending)
{
    bool multiVolumeSeries = false;
    vtkstd::vector<vtkstd::pair<double, vtkstd::string> > positionFilePairVector;
    double imagePosition = VTK_DOUBLE_MAX; // initialize to an "infinite" value
    
    for (int i = 0; i < fileNames->GetNumberOfValues(); i++) {
        svkImageData* tmp = svkMriImageData::New();
        double position[3];
        double row[3];
        double col[3];
        double normal[3];
        double tmpImagePosition = 0;
        tmp->GetDcmHeader()->ReadDcmFile(  fileNames->GetValue(i) );
        vtkstd::string tmpSeriesInstanceUID( tmp->GetDcmHeader()->GetStringValue("SeriesInstanceUID"));
        vtkstd::string xPosition( tmp->GetDcmHeader()->GetStringValue("ImagePositionPatient",0));
        std::istringstream xPositionInString(xPosition);
  	    xPositionInString >> position[0];
        vtkstd::string yPosition( tmp->GetDcmHeader()->GetStringValue("ImagePositionPatient",1));
        std::istringstream yPositionInString(yPosition);
  	    yPositionInString >> position[1];
        vtkstd::string zPosition( tmp->GetDcmHeader()->GetStringValue("ImagePositionPatient",2));
        std::istringstream zPositionInString(zPosition);
  	    zPositionInString >> position[2];
        vtkstd::string xRow( tmp->GetDcmHeader()->GetStringValue("ImageOrientationPatient",0));
        std::istringstream xRowInString(xRow);
  	    xRowInString >> row[0];
        vtkstd::string yRow( tmp->GetDcmHeader()->GetStringValue("ImageOrientationPatient",1));
        std::istringstream yRowInString(yRow);
  	    yRowInString >> row[1];
        vtkstd::string zRow( tmp->GetDcmHeader()->GetStringValue("ImageOrientationPatient",2));
        std::istringstream zRowInString(zRow);
  	    zRowInString >> row[2];
        vtkstd::string xCol( tmp->GetDcmHeader()->GetStringValue("ImageOrientationPatient",3));
        std::istringstream xColInString(xCol);
  	    xColInString >> col[0];
        vtkstd::string yCol( tmp->GetDcmHeader()->GetStringValue("ImageOrientationPatient",4));
        std::istringstream yColInString(yCol);
  	    yColInString >> col[1];
        vtkstd::string zCol( tmp->GetDcmHeader()->GetStringValue("ImageOrientationPatient",5));
        std::istringstream zColInString(zCol);
  	    zColInString >> col[2];
        tmp->Delete();
        // Generate the normal.
        vtkMath::Cross(row,col,normal);
        // If input series UID matches this series UID, add to list for sorting later.
        if ( seriesInstanceUID == tmpSeriesInstanceUID ) {
            vtkstd::pair<double, vtkstd::string> positionFilePair;
            tmpImagePosition = (normal[0]*position[0]) + (normal[1]*position[1]) 
                + (normal[2]*position[2]);
            // If the image position is the same for any given slice, then we have a multi-volume series.
            if( tmpImagePosition == imagePosition ) {
                multiVolumeSeries = true;
                return multiVolumeSeries;
            } else {
                imagePosition = tmpImagePosition;
            }
            positionFilePair.first = imagePosition;
            positionFilePair.second = fileNames->GetValue(i);
            positionFilePairVector.push_back(positionFilePair);
        } 
    }
    // Sort according to ascending/descending order.
    if (ascending) {
        vtkstd::sort(positionFilePairVector.begin(), 
            positionFilePairVector.end(), svkDcmMriVolumeReaderSort_lt_pair_double_string());
    } else {
        vtkstd::sort(positionFilePairVector.begin(), 
            positionFilePairVector.end(), svkDcmMriVolumeReaderSort_gt_pair_double_string());
    }
    // Finally, repopulate the file list.
    fileNames->SetNumberOfValues(positionFilePairVector.size());
    for (int i = 0; i < positionFilePairVector.size(); i++) {
        fileNames->SetValue(i, positionFilePairVector[i].second.c_str() );
#if VTK_DEBUG_ON
        cout << "FN: " << fileNames->GetValue(i) << endl;
#endif
    } 
    return multiVolumeSeries;
}

/*!
 *
 */
void svkDcmMriVolumeReader::InitDcmHeader()
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

    // If ImageOrientationPatient is not the same for all of the slices,
    // then just use the specified file.
    svkImageData* tmp = svkMriImageData::New(); 
    tmp->GetDcmHeader()->ReadDcmFile(  this->GetFileName() );
    vtkstd::string imageOrientationPatient( tmp->GetDcmHeader()->GetStringValue("ImageOrientationPatient"));
    vtkstd::string seriesInstanceUID( tmp->GetDcmHeader()->GetStringValue("SeriesInstanceUID"));
    tmp->Delete();
    vtkStringArray* fileNames =  sortFileNames->GetFileNames();
    for (int i = 0; i < fileNames->GetNumberOfValues(); i++) {
        tmp = svkMriImageData::New(); 
        tmp->GetDcmHeader()->ReadDcmFile(  fileNames->GetValue(i) );
        vtkstd::string tmpImageOrientationPatient( tmp->GetDcmHeader()->GetStringValue("ImageOrientationPatient"));
        tmp->Delete();
        if ( imageOrientationPatient != tmpImageOrientationPatient ) {
            
            vtkWarningWithObjectMacro(this, "ImageOrientationPatient is not the same for all slices, using only specified file ");

            vtkStringArray* tmpFileNames = vtkStringArray::New(); 
            tmpFileNames->SetNumberOfValues(1);
            tmpFileNames->SetValue(0, this->GetFileName() );
            sortFileNames->SetInputFileNames( tmpFileNames );
            tmpFileNames->Delete();
            break; 
        }
    }

    // Now sort the files according to SeriesInstanceUID and ImagePositionPatient.
    fileNames =  sortFileNames->GetFileNames();
    if ( svkDcmMriVolumeReaderSortFilesByImagePositionPatient(seriesInstanceUID, fileNames, true) ){
        vtkWarningWithObjectMacro(this, "Multi-volume DICOM series are currently not supported, using only specified file ");

        vtkStringArray* tmpFileNames = vtkStringArray::New(); 
        tmpFileNames->SetNumberOfValues(1);
        tmpFileNames->SetValue(0, this->GetFileName() );
        sortFileNames->SetInputFileNames( tmpFileNames );
        tmpFileNames->Delete();
    }
    // Calling this method will set the DataExtents for the slice direction
    this->SetFileNames( sortFileNames->GetFileNames() );

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
    
    globFileNames->Delete();
    sortFileNames->Delete();
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
        // Do I need to byte swap?
        //if ( this->GetSwapBytes() ) {
	//    vtkByteSwap::SwapVoidRange((void *)((short *)imageData + (i * numPixelsInSlice)), numPixelsInSlice, sizeof(short));
        //}  
    }

    //this->GetOutput()->GetDcmHeader()->PrintDcmHeader(); 
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
 *  Returns the file root without extension
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

