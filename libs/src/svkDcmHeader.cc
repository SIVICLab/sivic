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
 */



#include <svkDcmHeader.h>
#include <svkDICOMParser.h>
#include <vector>
#include <set>
#include "svkTypeUtils.h"
#include "svkDcmHeader.h"


using namespace svk;


//vtkCxxRevisionMacro(svkDcmHeader, "$Rev$");


const float svkDcmHeader::UNKNOWN_TIME = -1;
const string svkDcmHeader::UNKNOWN_STRING = "UNKNOWN";



/*!
 *
 */
svkDcmHeader::svkDcmHeader()
{

#if VTK_DEBUG_ON
    this->DebugOn();
#endif
    vtkDebugMacro(<<this->GetClassName()<<":svkDcmHeader");

    this->lastUpdateTime = this->GetMTime(); 
    this->dataSliceOrder = svkDcmHeader::INCREMENT_ALONG_POS_NORMAL;
    this->numTimePts = 1;

    this->orientation[0][0] = 0; 
    this->orientation[0][1] = 0; 
    this->orientation[0][2] = 0; 
    this->orientation[1][0] = 0; 
    this->orientation[1][1] = 0; 
    this->orientation[1][2] = 0; 
    this->origin0[0] = 0; 
    this->origin0[1] = 0; 
    this->origin0[2] = 0; 
    
}


/*!
 *
 */
svkDcmHeader::~svkDcmHeader()
{
    vtkDebugMacro(<<this->GetClassName()<<"~svkDcmHeader");
}


/*!
 *  Converts a string representation of a patient name to a DICOM VR = PN representation. 
 *      "[lastName[^firstName[^middleName[^namePrefix[^nameSuffix]]]]"
 *
 *      \param PatientName space delimited patient name string 
 */
string svkDcmHeader::GetDcmPatientName(string PatientName)
{

    size_t delim;

    const string dcmPNDelim("^");

    // There should be no more than 5 elements to the name (4 delims):
    // replace spaces with ^, or add ^ between elements not present
    for (int i = 0; i < 4; i++) {
        if ( (delim = PatientName.find_first_of(' ') ) != string::npos ) {
            PatientName.replace( delim, 1, dcmPNDelim);
        } else {
            PatientName.push_back('^');
        }
    }
    return PatientName;
}


/*!
 *  Converts a string representation of a patient name to a DICOM VR = PN representation
 *  and sets the value in the DICOM header: 
 *      "[lastName[^firstName[^middleName[^namePrefix[^nameSuffix]]]]"
 *
 *      \param PatientName space delimited patient name string 
 */
void svkDcmHeader::SetDcmPatientName(string PatientName)
{

    string dcmPatientName = this->GetDcmPatientName( PatientName ); 

    this->SetValue(
        "PatientName",
        dcmPatientName
    );

}


/*!
 *  Set the format that specifies the word size and representation in the DICOM PixelData.
 *  Note that this represents the in-memory pixel representation and supports 8, 16 and 32 bit
 *  char, int and float representations.  When writing to DICOM MRI only 8 and 16 bit ints are 
 *  supported, so data may require scaling (see DICOM RescaleIntercept and RescaleSlope attributes).   
 */
void svkDcmHeader::SetPixelDataType(DcmPixelDataFormat dataType)
{

    if ( dataType == svkDcmHeader::UNSIGNED_INT_1 ) {
        this->SetValue( "BitsAllocated", 8 );
        this->SetValue( "BitsStored", 8 );
        this->SetValue( "HighBit", 7 );
        this->SetValue( "PixelRepresentation", 0 ); //unsigned
    } else if ( dataType == svkDcmHeader::UNSIGNED_INT_2 ) {
        this->SetValue( "BitsAllocated", 16 );
        this->SetValue( "BitsStored", 16 );
        this->SetValue( "HighBit", 15 );
        this->SetValue( "PixelRepresentation", 0 ); //unsigned
    } else if ( dataType == svkDcmHeader::SIGNED_INT_2 ) {
        this->SetValue( "BitsAllocated", 16 );
        this->SetValue( "BitsStored", 16 );
        this->SetValue( "HighBit", 15 );
        this->SetValue( "PixelRepresentation", 1 ); //signed
    } else if ( dataType == svkDcmHeader::SIGNED_FLOAT_4 ) {
        this->SetValue( "BitsAllocated", 32 );
        this->SetValue( "BitsStored", 32 );
        this->SetValue( "HighBit", 31 );
        this->SetValue( "PixelRepresentation", 1 ); //signed
    } else if ( dataType == svkDcmHeader::SIGNED_FLOAT_8 ) {
        this->SetValue( "BitsAllocated", 64 );
        this->SetValue( "BitsStored", 64 );
        this->SetValue( "HighBit", 63 );
        this->SetValue( "PixelRepresentation", 1 ); //signed
    } else {
        throw runtime_error("Unsupported data type representation.");
    }
}


/*!
 *  Get the format that specifies the word size and representation in the DICOM PixelData.
 *  4Byte representations are ambiguous and can be float or int, thus the vtkDataType arg.   
 */
int svkDcmHeader::GetPixelDataType( int vtkDataType )
{
    int bitsPerWord = this->GetIntValue( "BitsAllocated" );
    int pixRep = this->GetIntValue( "PixelRepresentation");
    //cout << "GETPIXELDATATYPE: " << bitsPerWord << " " << pixRep << " " << vtkDataType << endl;
    if ( bitsPerWord == 8 && pixRep == 0 ) {
        return svkDcmHeader::UNSIGNED_INT_1;
    } else if ( bitsPerWord == 16 && pixRep == 0 ) {
        return svkDcmHeader::UNSIGNED_INT_2;
    } else if ( bitsPerWord == 16 && pixRep == 1 ) {
        return svkDcmHeader::SIGNED_INT_2;
    } else if ( bitsPerWord == 32 && pixRep == 1 && vtkDataType == VTK_FLOAT ) {
        return svkDcmHeader::SIGNED_FLOAT_4;
    } else if ( bitsPerWord == 64 && pixRep == 1 && vtkDataType == VTK_DOUBLE ) {
        return svkDcmHeader::SIGNED_FLOAT_8;
    } else {
        cout << "Unknown Pixel Data Type " << endl;
        return -1;
    }
}


/*!
 *  Get the origin of the data set. 
 */
int svkDcmHeader::GetOrigin(double origin[3], int sliceNumber)
{

    int status = 0; 

    if ( sliceNumber == 0 && this->WasModified() ) {
        this->UpdateSpatialParams();
    }

    if (sliceNumber == 0) {

        origin[0] = this->origin0[0]; 
        origin[1] = this->origin0[1]; 
        origin[2] = this->origin0[2]; 

    } else {

        if ( this->ElementExists( "ImagePositionPatient", "PlanePositionSequence") == true ) {

            istringstream* iss = new istringstream();
            string originString; 

            //  Iterate of all 3 positions of the ImagePositionPatient element;
            for (int i = 0; i < 3; i++) {
                originString = this->GetStringSequenceItemElement(
                    "PlanePositionSequence",
                    0,
                    "ImagePositionPatient",
                    i, 
                    "PerFrameFunctionalGroupsSequence",
                    sliceNumber
                );
                iss->str(originString);
                *iss >> origin[i];
                iss->clear();
            }
            delete iss;

        } else {

            if (this->GetDebug()) {
                vtkWarningWithObjectMacro(
                    this, 
                    "::GetOrigin(): ImagePositionPatient not defined, can't return origin." 
                );    
            }
            status = 1; 
        }
    }

    //cout << "Origin: " << origin[0] << " " << origin[1] << " " << origin[2] << endl;
    return status; 
}


/*!
 *  Get the size of the voxels.  
 */
void svkDcmHeader::GetPixelSize(double size[3])
{

    if ( this->WasModified() ) {
        this->UpdateSpatialParams();
    }

    size[0] = this->pixelSize[0]; 
    size[1] = this->pixelSize[1]; 
    size[2] = this->pixelSize[2]; 
}


/*!
 *  Get the spacing of the voxels.  This accounts for any gaps between samples.  It is the actial
 *  spacing between samples.  May differ from pixel(sample) size(thickness).
 */
void svkDcmHeader::GetPixelSpacing(double spacing[3])
{
    if ( this->WasModified() ) {
        this->UpdateSpatialParams();
    }

    spacing[0] = this->pixelSpacing[0]; 
    spacing[1] = this->pixelSpacing[1]; 
    spacing[2] = this->pixelSpacing[2]; 

}


/*!
 *  Get the DICOM orientation vectors (in plane vector in direction of row and column data) 
 *  from the DICOM header. 
 */
void svkDcmHeader::GetOrientation(double orientation[2][3])
{

    if ( this->WasModified() ) {
        this->UpdateSpatialParams();
    }

    orientation[0][0] = this->orientation[0][0]; 
    orientation[0][1] = this->orientation[0][1]; 
    orientation[0][2] = this->orientation[0][2]; 
    orientation[1][0] = this->orientation[1][0]; 
    orientation[1][1] = this->orientation[1][1]; 
    orientation[1][2] = this->orientation[1][2]; 

    //cout << "orientation: " << orientation[0][0] << " " << orientation[0][1] << " " << orientation[0][2] << endl;
    //cout << "orientation: " << orientation[1][0] << " " << orientation[1][1] << " " << orientation[1][2] << endl;
}



/*!
 *
 *  This method is used for determining the index of the dcos matrix that corresponds to the 
 *  normal vector of a given user defined slice orientation in LPS coordinates.
 *
 *
 * Each row of the dcos represents the COLUMN, ROW, or SLICE dimension of the data
 * as defined by the data ordering.
 *
 * We are going to check them each sequentially to make sure that if two rows have the same
 * magnitude in a given direction they are assigned to different slice view orientations.
 *
 * EXAMPLE 1----------------------------------------------------------------------
 *
 * 
 *
 * For example if we have the dcos:
 *
 *                       LR        AP        SI
 *
 *     COLUMN INDEX       1         0         0 
 *     ROW INDEX          0         1         0 
 *     SLICE INDEX        0         0         1 
 *
 * If user requests the index of a coronal view of the data:
 *
 *    int coronalIndex = GetOrientationIndex( svkDcmHeader::CORONAL )
 *
 * First we will find the AXIAL direction. This is defined as the greatest absolute value
 * in the SI column. It is clear that this is the SLICE INDEX. Second we search for
 * the CORONAL direction-- this is the greatest absolute value in the AP column and 
 * is clearly the COLUMN INDEX. Lastly we look for the SAGITTAL directions which is
 * again clear and is the ROW INDEX
 *     Result:
 *         AXIAL    index = 2 <-- This is the data's orientation type
 *         CORONAL  index = 1
 *         SAGITTAL index = 0
 *
 * And the method would return the value 1, for CORONAL 
 *
 *    GetOrientationIndex( svkDcmHeader::CORONAL ) = 1;
 *
 * EXAMPLE 2----------------------------------------------------------------------
 *
 * For Example if we have the dcos:
 *
 *                       LR        AP        SI
 *
 *     COLUMN INDEX       0         1         0 
 *     ROW INDEX          1         0         0 
 *     SLICE INDEX        0         0         1 
 *
 * If user requests the index of a sagittal view of the data:
 *
 *    int axialIndex = GetOrientationIndex( svkDcmHeader::AXIAL )
 *
 * First we will find the AXIAL direction. This is defined as the greatest absolute value
 * in the SI column. It is clear that this is the SLICE INDEX. Second we search for
 * the CORONAL direction-- this is the greatest absolute value in the AP column and 
 * is clearly the ROW INDEX. Lastly we look for the SAGITTAL directions which is
 * again clear and is the COLUMN INDEX
 *
 *     Result:
 *         AXIAL    index = 2 <-- This is the data's orientation type
 *         CORONAL  index = 0
 *         SAGITTAL index = 1
 *
 * And the method would return the value 2, for AXIAL 
 *
 *    GetOrientationIndex( svkDcmHeader::AXIAL ) = 2;
 *
 *
 * EXAMPLE 3----------------------------------------------------------------------
 *
 * For Example if we have the dcos:
 *
 *                        LR        AP        SI
 *
 *     COLUMN INDEX    0.7071    0.5000    0.5000
 *     ROW INDEX            0    0.7071   -0.7071
 *     SLICE INDEX    -0.7071    0.5000    0.5000
 *
 * If user requests the index of a sagittal view of the data:
 *
 *    int coronalIndex = GetOrientationIndex( svkDcmHeader::CORONAL )
 *
 * This case is of an image rotated 45 degrees in two dimensions. In this scenario
 * the data is so oblique that defining AXIAL, SAGITTAL, and CORONAL is somewhat
 * arbitrary.
 *
 * First we will find the AXIAL direction. This is defined as the greatest absolute value
 * in the SI column. In this case we would take the ROW INDEX and declare that the
 * AXIAL direction. Next we would search for the CORONAL direction. The highest component
 * in the AP direction is also the COLUMN INDEX, but we have already defined that
 * as the AXIAL direction so we take the next highest which is the same for both
 * the COLUMN and SLICE INDEX. In this case it is arbitrary, the method searches in the
 * order ROW, COLUMN, SLICE and searches for values greater than or equal to so it would 
 * take the SLICE INDEX. Since the only INDEX left is the COLUMN INDEX that will be our 
 * SAGITTAL direction.
 *
 *     Result:
 *         AXIAL    index = 1 
 *         CORONAL  index = 2 <-- This is the data's orientation type
 *         SAGITTAL index = 0
 *
 * And the method would return the value 0, for CORONAL 
 *
 *    GetOrientationIndex( svkDcmHeader::CORONAL ) = 0;
 *
 *  \param orientation the view orientation you want to get the index of 
 *  \return the index, 0 is COLUMN, 1 is ROW, 2 is SLICE as defined by the data ordering.
 */
int svkDcmHeader::GetOrientationIndex( svkDcmHeader::Orientation orientation )
{

    // First we need our dcos
    double dcos[3][3] = {{0}};
    this->GetDataDcos(dcos);

    // We need to map each row of the dcos to be AXIAL, SAGITTAL, or CORONAL....
    map<int,int> primaryComponents;

    // Lets initialize with negative values
    primaryComponents[svkDcmHeader::AXIAL]    = -1;
    primaryComponents[svkDcmHeader::CORONAL]  = -1;
    primaryComponents[svkDcmHeader::SAGITTAL] = -1;

    // Lets start by determining the AXIAL direction...
    double maxValue = 0;
    int maxIndex = 0;
    for( int i = 0; i < 3; i++ ) {
        int SIIndex = 2; // 2 is the SI direction which corresponds to AXIAL 
        double value = fabs(dcos[i][SIIndex]); 
        if( value >= maxValue ) {
            maxValue = value;
            maxIndex = i;
        }
    }
    primaryComponents[svkDcmHeader::AXIAL] = maxIndex;

    // Now lets determin the coronal direction...
    maxValue = 0;
    maxIndex = 0;
    for( int i = 0; i < 3; i++ ) {
        if( i == primaryComponents[svkDcmHeader::AXIAL] ) {
            continue; // If this row has already been assigned to AXIAL, do not consider it for coronal
        }
        int PAIndex = 1; // 1 is the PA direction  which corresponds to CORONAL
        double value = fabs(dcos[i][PAIndex]); 
        if( value >= maxValue ) {
            maxValue = value;
            maxIndex = i;
        }
    }
    primaryComponents[svkDcmHeader::CORONAL] = maxIndex;

    // Now lets determin the sagittal direction...
    maxValue = 0;
    maxIndex = 0;
    for( int i = 0; i < 3; i++ ) {
        if( i == primaryComponents[svkDcmHeader::AXIAL] || i == primaryComponents[svkDcmHeader::CORONAL] ) {
            continue; // If this row has already been assigned to AXIAL or for CORONAL do not consider it for SAGITTAL
        }
        int LRIndex = 0; // 0 is the LR direction  which corresponds to CORONAL
        double value = fabs(dcos[i][LRIndex]);  
        if( value >= maxValue ) {
            maxValue = value;
            maxIndex = i;
        }
    }
    primaryComponents[svkDcmHeader::SAGITTAL] = maxIndex;

    // Now they we have defined each index, lets give the user the requested index
    return primaryComponents[orientation];
}


/*!
 * Determine if the data set is coronal/sagital/axial
 */
svkDcmHeader::Orientation svkDcmHeader::GetOrientationType( )
{
    Orientation orientation = UNKNOWN_ORIENTATION;
    int axialIndex =    this->GetOrientationIndex(AXIAL);
    int coronalIndex =  this->GetOrientationIndex(CORONAL);
    int sagittalIndex = this->GetOrientationIndex(SAGITTAL);

    // Orientation type is defined as the SLICE dimension
    int sliceIndex = 2;
    if( axialIndex == sliceIndex ) {
        orientation = AXIAL;
    } else if ( coronalIndex == sliceIndex ) {
        orientation = CORONAL;
    } else if( sagittalIndex == sliceIndex ) {
        orientation = SAGITTAL;
    }

    return orientation;
 
}


/*!
 *  Returns the right handed normal vector to the slice defined by DICOM imageOrientationPatient. 
 *  from the DICOM header. 
 */
void svkDcmHeader::GetNormalVector(double normal[3])
{

    double orientation[2][3]; 
    this->GetOrientation( orientation );

    double colVector[3];
    colVector[0] = orientation[0][0];
    colVector[1] = orientation[0][1];
    colVector[2] = orientation[0][2];

    double rowVector[3];
    rowVector[0] = orientation[1][0];
    rowVector[1] = orientation[1][1];
    rowVector[2] = orientation[1][2];

    vtkMath* math = vtkMath::New();
    math->Cross(colVector, rowVector, normal);
    math->Delete();
}


/*!
 *  Sets the slice order value so that the sense of the normal to the orientation 
 *  plane can be calculated correctly.  
 */
void svkDcmHeader::SetSliceOrder( DcmDataOrderingDirection sliceOrderVal )
{
    this->dataSliceOrder = sliceOrderVal; 
}


/*!
 *  Returns the 3x3 DCOS matrix with slice vector indicating the direction of data 
 *  ordering in the data set.  i.e. in LPS, 0,0,-1 indicates data is ordered by slice 
 *  from S to I. 
 */
void svkDcmHeader::GetDataDcos(double dcos[3][3], DcmDataOrderingDirection sliceOrderVal)
{
    if (sliceOrderVal == SLICE_ORDER_UNDEFINED && 
        this->dataSliceOrder == SLICE_ORDER_UNDEFINED) {
        vtkErrorWithObjectMacro(this, "GetDataDcos slice order is undefined");
    } else if (sliceOrderVal == SLICE_ORDER_UNDEFINED && 
               this->dataSliceOrder != SLICE_ORDER_UNDEFINED) {
        sliceOrderVal = this->dataSliceOrder; 
    }

    double orientation[2][3];
    this->GetOrientation( orientation );

    double normal[3];   
    this->GetNormalVector( normal );
    if (sliceOrderVal == INCREMENT_ALONG_NEG_NORMAL) {
        for (int i = 0; i < 3; i++) {
            normal[i] *= -1; 
        }
    }

    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 3; j++) {
            dcos[i][j] = orientation[i][j];  
        } 
    } 

    for (int j = 0; j < 3; j++) {
        dcos[2][j] = normal[j]; 
    }

    //
    //  change -0.0000 to 0.0000
    //
    for( int i = 0; i <= 2; i++ ) {
        for( int j = 0; j <= 2; j++ ) {
            dcos[i][j] = (dcos[i][j] == 0.0)? 0.0 : dcos[i][j];
        }
    }

}


/*!
 *  Method that copies a DICOM heder to the input (headerCopy).  New instance UIDs are generated
 *  for SeriesInstanceUID, SOPInstanceUID, MediaStorageSOPInstanceUID.  StudyInstanceuUID is  
 *  preserved.  
 */
void svkDcmHeader::MakeDerivedDcmHeader(svkDcmHeader* headerCopy, string seriesDescription)
{
    this->CopyDcmHeader(headerCopy);  
    headerCopy->InsertUniqueUID("SeriesInstanceUID");

    string newUID = this->GenerateUniqueUID(); 
    headerCopy->SetValue("SOPInstanceUID",              newUID.c_str(), false);
    headerCopy->SetValue("MediaStorageSOPInstanceUID",  newUID.c_str(), true);

    headerCopy->SetValue("ImageType", "DERIVED\\SECONDARY");
    headerCopy->SetSliceOrder(this->dataSliceOrder);
    if ( !seriesDescription.empty() ) {
        headerCopy->SetValue("SeriesDescription", seriesDescription);
    }
}


/*!
 *  Method that updates the current DICOM heder. New instance UIDs are generated
 *  for SeriesInstanceUID, SOPInstanceUID, MediaStorageSOPInstanceUID.  StudyInstanceuUID is  
 *  preserved.  
 */
void svkDcmHeader::MakeDerivedDcmHeader(string seriesDescription)
{
    this->InsertUniqueUID("SeriesInstanceUID");

    string newUID = this->GenerateUniqueUID(); 
    this->SetValue("SOPInstanceUID",             newUID.c_str(), false);
    this->SetValue("MediaStorageSOPInstanceUID", newUID.c_str(), true);

    this->SetValue("ImageType", "DERIVED\\SECONDARY");
    this->SetSliceOrder(this->dataSliceOrder);
    if ( !seriesDescription.empty() ) {
        this->SetValue("SeriesDescription", seriesDescription);
    }
}


/*!
 *
 */
bool svkDcmHeader::WasModified()
{
    if (this->GetMTime() > lastUpdateTime) {
        return 1;  
    } else {
       return 0;  
    }
}


/*!
 *  
 */
void svkDcmHeader::UpdateSpatialParams()
{

    //  
    //  Note:
    //      that this is sensitive to the order of operations!!!!!    
    //  
    this->lastUpdateTime = this->GetMTime();
    this->UpdateNumTimePoints();
    this->UpdateNumCoils();
    this->UpdateOrientation();
    this->UpdatePixelSize();
    this->UpdateOrigin0();
    this->UpdatePixelSpacing();
    this->UpdateDimensionIndexVector();
}


/*!
 *
 */
void svkDcmHeader::UpdateOrientation()
{

    istringstream* iss = new istringstream();
    string orientationString;

    //  The 6 elements of the ImageOrientationPatient are the in plane vectors along the
    //  row and column directions:

    //  for multi-frame objects, orientation is in shared or per frame functional group sequence: 
    string parentSequence;
    bool inFunctionalGroup = false;
    if( this->ElementExists( "PlaneOrientationSequence", "PerFrameFunctionalGroupsSequence" ) ) {
        parentSequence = "PerFrameFunctionalGroupsSequence";
        inFunctionalGroup = true;
    } else if( this->ElementExists( "PlaneOrientationSequence", "SharedFunctionalGroupsSequence" ) ) {
        parentSequence = "SharedFunctionalGroupsSequence";
        inFunctionalGroup = true;
    }

    int linearIndex = 0;
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 3; j++) {
            if ( inFunctionalGroup ) {
                orientationString = this->GetStringSequenceItemElement(
                    "PlaneOrientationSequence",
                    0,
                    "ImageOrientationPatient",
                    linearIndex,
                    parentSequence.c_str(), 
                    0
                );
            } else {
                orientationString = this->GetStringValue( "ImageOrientationPatient", linearIndex ); 
            }
            iss->str(orientationString);
            *iss >> this->orientation[i][j];
            iss->clear();
            linearIndex++;
        }
    }

    delete iss;
}


/*!
 *
 */
void svkDcmHeader::UpdatePixelSize()
{

    istringstream* iss = new istringstream();
    string sizeString;
    string parentSequence;

    //  for multi-frame objects, orientation is in shared functional group sequence: 
    bool inFunctionalGroup = false; 
    if( this->ElementExists( "PixelMeasuresSequence", "PerFrameFunctionalGroupsSequence" ) ) {
        parentSequence = "PerFrameFunctionalGroupsSequence";
        inFunctionalGroup = true; 
    } else if( this->ElementExists( "PixelMeasuresSequence", "SharedFunctionalGroupsSequence" ) ) {
        parentSequence = "SharedFunctionalGroupsSequence";
        inFunctionalGroup = true; 
    } 


    for (int i = 0; i < 2; i++ ) {
        if ( inFunctionalGroup ) {
            sizeString = this->GetStringSequenceItemElement(
                "PixelMeasuresSequence",
                0,
                "PixelSpacing",
                i,
                parentSequence.c_str(),
                0
            );
        } else {
            if( this->ElementExists( "PixelSpacing" ) ) {
                sizeString = this->GetStringValue( "PixelSpacing", i ); 
            } else {
                sizeString = "0"; 
            }
        }
        iss->str(sizeString);
        *iss >> this->pixelSize[i];
        iss->clear();
    }

    if( this->ElementExists( "SliceThickness", "PixelMeasuresSequence") ) {
        this->pixelSize[2] = this->GetDoubleSequenceItemElement(
            "PixelMeasuresSequence",
            0,
            "SliceThickness",
            parentSequence.c_str(),
            0
        );
    } else if( this->ElementExists( "SliceThickness") ) {
        //  in case it lives elsewhere
        bool searchInto = true; 
        this->pixelSize[2] = this->GetDoubleValue("SliceThickness", searchInto); 
    } else {
        this->pixelSize[2] = 0;
        
    }

    delete iss;
}


/*!
 *
 */
void svkDcmHeader::UpdatePixelSpacing()
{
    double size[3];
    this->GetPixelSize(size);
    for (int i = 0; i < 3; i++) {
        this->pixelSpacing[i] = size[i];
    }

    if ( this->GetNumberOfSlices() >= 2 ) {

        //  If can't get origins, then return pixelSize as slice spacing
        double origin0[3];
        if ( this->GetOrigin(origin0, 0) != 0 ) { 
            return; 
        }
        double origin1[3];
        if ( this->GetOrigin(origin1, 1) != 0 ) {
            return; 
        }
        double sliceSpacing = 0;
        for (int i = 0; i < 3; i++ ) {
            sliceSpacing += pow(origin1[i] - origin0[i], 2);
        }
        sliceSpacing = pow(sliceSpacing, .5);
        //cout << "SLICE SPACING WITH GAP: " << sliceSpacing << endl;

        this->pixelSpacing[2] = sliceSpacing;
    }
    //cout << "Spacing: " << this->pixelSpacing[0] << " " << this->pixelSpacing[1] << " " << this->pixelSpacing[2] << endl;

}


/*!
 *
 */
void svkDcmHeader::UpdateOrigin0()
{

    if ( this->ElementExists("ImagePositionPatient") == true ) {

        istringstream* iss = new istringstream();
        string originString; 
        int frameNumber = 0; 

        bool inFunctionalGroup = false; 
        if( this->ElementExists( "ImagePositionPatient", "PerFrameFunctionalGroupsSequence") ) {
            inFunctionalGroup = true; 
        } 

        //  Iterate of all 3 positions of the ImagePositionPatient element;
        for (int i = 0; i < 3; i++) {
            if ( inFunctionalGroup ) {
                originString = this->GetStringSequenceItemElement(
                    "PlanePositionSequence",
                    0,
                    "ImagePositionPatient",
                    i, 
                    "PerFrameFunctionalGroupsSequence",
                    frameNumber
                );
            } else {
                originString = this->GetStringValue( "ImagePositionPatient", i ); 
            } 
            iss->str(originString);
            *iss >> this->origin0[i];
            iss->clear();
        }

        delete iss;

    } else {

        if (this->GetDebug()) {
            vtkWarningWithObjectMacro(
                this, 
                "::UpdateOrigin0(): ImagePositionPatient not defined, can't update Origin0." 
            );    
        }
    }
}


/*!
 *  Returns the number of coils the DcmHeader.
 *  Parses FrameContentSequence for the number of 
 *  coils, if specified in the Multi Frame Dimension Module
 */
int svkDcmHeader::GetNumberOfCoils()
{

    if ( this->WasModified() ) {
        this->UpdateSpatialParams();
    }

    return this->numCoils; 

}


/*!
 *
 */
void svkDcmHeader::UpdateNumCoils()
{
    this->numCoils = 1;
    this->numCoils = 1;
    svkDcmHeader::DimensionVector dimensionVector = this->GetDimensionIndexVector(); 
    this->numCoils = this->GetDimensionVectorValue(&dimensionVector, svkDcmHeader::CHANNEL_INDEX) + 1;
}


/*!
 *  Returns the number of time points from the DcmHeader.
 *  Parses FrameContentSequence for the number of 
 *  time points, if specified in the Multi Frame Dimension Module
 */
int svkDcmHeader::GetNumberOfTimePoints()
{

    if ( this->WasModified() ) {
        this->UpdateSpatialParams();
    }

    return this->numTimePts; 
}


/*!
 *
 */
void svkDcmHeader::UpdateNumTimePoints()
{
    svkDcmHeader::DimensionVector dimensionVector = this->GetDimensionIndexVector(); 
    this->numTimePts = this->GetDimensionVectorValue(&dimensionVector, svkDcmHeader::TIME_INDEX) + 1;
}


/*!
 *  Gets the position of the index representing the specified dimension type
 *  from the DimensionIndexSequence.  e.g. given 3 indices "0/1/2", which
 *  one represents slice, coil number, etc.
 *  Returns -1 if dimension can't be determined.   
 */
int svkDcmHeader::GetDimensionIndexPosition(string indexLabel)    
{
    int dimIndexNumber = -1; 

    int numDims = this->GetNumberOfItemsInSequence("DimensionIndexSequence");
    for ( int i = 0; i < numDims; i++ ) {

        string dimensionLabel = this->GetStringSequenceItemElement(
            "DimensionIndexSequence",
            i, 
            "DimensionDescriptionLabel",
            NULL, 
            i
        );
        if ( dimensionLabel.compare( indexLabel ) == 0 ) {
            dimIndexNumber = i; 
        }

    }

    return dimIndexNumber; 
}


/*!
 *  Gets the index labe at the specified DimensionIndex position( start index at 0)
 */
string svkDcmHeader::GetDimensionIndexLabel(int dimensionIndexNumber )    
{
    string dimensionIndexLabel = ""; 

    int numDims = this->GetNumberOfItemsInSequence("DimensionIndexSequence");
    if ( dimensionIndexNumber < numDims ) { 
        
        if ( ! this->ElementExists( "DimensionDescriptionLabel", "DimensionIndexSequence") == true ) {
            //  insert the description label: 
            this->InsertDimensionIndexLabels(); 
        }

        //  Type 3 
        dimensionIndexLabel = this->GetStringSequenceItemElement(
            "DimensionIndexSequence",
            dimensionIndexNumber, 
            "DimensionDescriptionLabel",
            NULL, 
            dimensionIndexNumber 
        );
    }

    return dimensionIndexLabel; 
}


/*!
 *   Fill in type 3 label fields in DimensionIndexSequence if missing:  
 */ 
void svkDcmHeader::InsertDimensionIndexLabels( )
{

    int numDims = this->GetNumberOfItemsInSequence("DimensionIndexSequence");

    for (int dimNum = 0; dimNum < numDims; dimNum++ ) {

        string dimensionIndexPointer= ""; 
        string dimensionFunctionalGroupPointer = ""; 
        string dimensionIndexLabel = ""; 

        //  Type 1
        if ( this->ElementExists( "DimensionIndexPointer", "DimensionIndexSequence") == true ) {
            dimensionIndexPointer = this->GetStringSequenceItemElement(
                "DimensionIndexSequence",
                dimNum, 
                "DimensionIndexPointer",
                NULL, 
                dimNum
            );
            //cout << "dimensionIndexPointer" << dimensionIndexPointer << endl;
            string tagName = this->GetDcmNameFromTag( dimensionIndexPointer ); 
            cout << "TN: " << tagName << endl;
            dimensionIndexLabel = tagName; 
            if (dimNum == 0) {
                dimensionIndexLabel = "TIME"; 
            } else {
                dimensionIndexLabel = "SLICE"; 
            } 
            
        }

        this->AddSequenceItemElement(
            "DimensionIndexSequence",
            dimNum,
            "DimensionDescriptionLabel",
            //this->DimensionIndexLabelToString( svkDcmHeader::TIME_INDEX)
            dimensionIndexLabel
        );
    }

    return;  
}


/*!
 *  Gets the number of frames in the specified dimension. 
 *  Iterates over each frame in the FrameContentSequence and gets the set of unique
 *  values for the specified dimensionIndex.  This is therefore the 
 *  dimension size of the specified index.
 */
int svkDcmHeader::GetNumberOfFramesInDimension( int dimensionIndex )
{

    int numFramesInDimension; 

    bool frameContentExists = false; 
    if( this->ElementExists( "FrameContentSequence", "PerFrameFunctionalGroupsSequence" ) ) {
        frameContentExists = true; 
    }

    if ( (dimensionIndex >= 0) && (frameContentExists == true) ) {

        int numberOfFrames = this->GetNumberOfFrames();
        set <int> frames;
        for (int frameNum = 0; frameNum < numberOfFrames; frameNum++ ) {

            int value = this->GetIntSequenceItemElement(
                "FrameContentSequence",
                0, 
                "DimensionIndexValues",
                "PerFrameFunctionalGroupsSequence",
                frameNum,
                dimensionIndex  // position of the specific index in the DimensionIndexSequence . 
            );
            frames.insert(value);
        }

        numFramesInDimension = frames.size();

    } else {
        numFramesInDimension = 1; 
    }

    return numFramesInDimension; 
}


/*!
 *
 */
int svkDcmHeader::GetNumberOfSlices()
{
    svkDcmHeader::DimensionVector dimensionVector = this->GetDimensionIndexVector(); 
    int numSlices = this->GetDimensionVectorValue(&dimensionVector, svkDcmHeader::SLICE_INDEX) + 1;
    return numSlices; 
}


/*!
 *  Uses the dimension index sequence to determine the slice for the frame.
 */
int svkDcmHeader::GetSliceForFrame( int frame )
{
    svkDcmHeader::DimensionVector dimensionVector = this->GetDimensionIndexVector();
    svkDcmHeader::DimensionVector frameIndexVector = dimensionVector;
    this->GetDimensionVectorIndexFromFrame( &dimensionVector, &frameIndexVector, frame);
    return svkDcmHeader::GetDimensionVectorValue( &frameIndexVector, svkDcmHeader::SLICE_INDEX);
}


/*!
 *  Get the number of Dimension indices required 
 *  for specifying the frame's slice, time, coil. 
 *  See FrameContentSequence DimensionIndexValues
 */
int svkDcmHeader::GetNumberOfDimensionIndices(int numTimePts, int numCoils)
{
    //  retain order slice, time, coil
    int numDimensionIndices = 1;    //Default is slice only 
    if ( numTimePts > 1 ) {
        numDimensionIndices++;
    }
    if ( numCoils > 1 ) {
        numDimensionIndices++;
    }
    return numDimensionIndices;

}


/*!
 *  Get the Dimension Index Values for a set of indices 
 */
void svkDcmHeader::SetDimensionIndices(unsigned int* indexValues, int numFrameIndices, int sliceNum, int timePt, int coilNum, int numTimePts, int numCoils)
{
    indexValues[0] = sliceNum;
    if ( numFrameIndices >= 2 ) {
        if ( numTimePts > 1 ) {
            indexValues[1] = timePt;
        } else if (numTimePts == 1 && numCoils > 1) {
            indexValues[1] = coilNum;
        }
    }
    if ( numFrameIndices == 3 ) {
        if ( numCoils > 1 ) {
            indexValues[2] = coilNum;
        }
    }
}


/*!
 *  Initializes the DICOM Patient Module (Patient IE)
 */
void svkDcmHeader::InitPatientModule(string PatientName, string patientID, string PatientBirthDate, string PatientSex)
{
    if ( !PatientName.empty() ) {
        this->SetValue(
            "PatientName",
            PatientName
        );
    }

    if ( !patientID.empty() ) {
        this->SetValue(
            "PatientID",
            patientID
        );
    }
    if ( !PatientBirthDate.empty() ) {
        this->SetValue(
            "PatientBirthDate",
            PatientBirthDate
        );
    }
    if ( !PatientSex.empty() ) {
        this->SetValue(
            "PatientSex",
            PatientSex
        );
    }
}


/*!
 *  Initializes the DICOM Patient Module (Patient IE)
 */
void svkDcmHeader::InitGeneralStudyModule(string studyDate, string studyTime, string referringPhysiciansName, string studyID, string accessionNumber, string studyInstanceUID)
{

    if ( !studyDate.empty() ) {
        this->SetValue(
            "StudyDate",
            studyDate
        );
    }

    if ( !studyTime.empty() ) {
        this->SetValue(
            "StudyTime",
            studyTime
        );
    }

    if ( !referringPhysiciansName.empty() ) {
        this->SetValue(
            "ReferringPhysicianName",
            referringPhysiciansName
        );
    }

    if ( !studyID.empty() ) {
        this->SetValue(
            "StudyID",
            studyID
        );
    }

    if ( !accessionNumber.empty() ) {
        this->SetValue(
            "AccessionNumber",
            accessionNumber
        );
    }

    if ( !studyInstanceUID.empty() ) {
        this->SetValue(
            "StudyInstanceUID",
            studyInstanceUID
        );
    }
}


/*!
 *  Initializes the DICOM General Series Module (Series IE)
 *  Modality is set on initialztion of IOD (svkIOD).  
 */
void svkDcmHeader::InitGeneralSeriesModule(string seriesNumber, string seriesDescription, string patientPosition)
{

    if ( !seriesNumber.empty() ) {
        this->SetValue(
            "SeriesNumber",
            seriesNumber
        );
    }

    if ( !seriesDescription.empty() ) {
        this->SetValue(
            "SeriesDescription",
            seriesDescription
        );
    }

    if ( !patientPosition.empty() ) {
        this->SetValue(
            "PatientPosition",
            patientPosition
        );
    }
}


/*!
 *  Initializes the DICOM Image Pixel Module
 */
void svkDcmHeader::InitImagePixelModule( int rows, int columns, svkDcmHeader::DcmPixelDataFormat dataType)
{
    this->SetValue( "Rows", rows );
    this->SetValue( "Columns", columns );
    this->SetPixelDataType( dataType );
}


/*!
 *
 */
void svkDcmHeader::InitPlaneOrientationMacro( double dcos[3][3] )
{

    ostringstream ossDcos;
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 3; j++) {
            ossDcos << dcos[i][j];
            if (i * j != 2) {
                ossDcos<< "\\";
            }
        }
    }


    this->InitPlaneOrientationMacro( ossDcos.str() ); 

    //  Set slice order based on the normal to the newly set orientation: 
    double normal[3];
    this->GetNormalVector(normal);

    double dcosSliceOrder[3];
    for (int i = 0; i < 3; i++) {
        dcosSliceOrder[i] = dcos[2][i];
    }

    //  Use the scalar product to determine whether the data in the .cmplx
    //  file is ordered along the slice normal or antiparalle to it.
    vtkMath* math = vtkMath::New();
    if (math->Dot(normal, dcosSliceOrder) > 0 ) {
        this->dataSliceOrder = svkDcmHeader::INCREMENT_ALONG_POS_NORMAL;
    } else {
        this->dataSliceOrder = svkDcmHeader::INCREMENT_ALONG_NEG_NORMAL;
    }
    this->SetSliceOrder( this->dataSliceOrder );
    math->Delete();

}


/*!
 *
 */
void svkDcmHeader::InitPlaneOrientationMacro( string orientationString )
{

    this->ClearSequence( "PlaneOrientationSequence" );

    this->AddSequenceItemElement(
        "SharedFunctionalGroupsSequence",
        0,
        "PlaneOrientationSequence"
    );


    this->AddSequenceItemElement(
        "PlaneOrientationSequence",
        0,
        "ImageOrientationPatient",
        orientationString,
        "SharedFunctionalGroupsSequence",
        0
    );
}


/*!
 *  Initialize the MR Image Module. 
 */
void svkDcmHeader::InitMRImageModule( string repetitionTime, string echoTime)
{
    if ( !repetitionTime.empty() ) {
        this->SetValue( "RepetitionTime", repetitionTime);
    }

    if ( !echoTime.empty() ) {
        this->SetValue( "EchoTime", echoTime);
    }
}


/*!
 *  Initialize the ImagePlane Module. 
 *  For single frame objects this may be called repeatedly changing only the value of the 
 *  current instance ImagePositionPatient value. 
 */
void svkDcmHeader::InitImagePlaneModule( string imagePositionPatient, string pixelSpacing, string imageOrientationPatient, string sliceThickness)
{
    if ( !pixelSpacing.empty() ) {
        this->SetValue( "PixelSpacing", pixelSpacing);
    }

    if ( !imageOrientationPatient.empty() ) {
        this->SetValue( "ImageOrientationPatient", imageOrientationPatient);
    }

    if ( !imagePositionPatient.empty() ) {
        this->SetValue( "ImagePositionPatient", imagePositionPatient);
    }

    if ( !sliceThickness.empty() ) {
        this->SetValue( "SliceThickness", sliceThickness);
    }
}


/*!
 *  Initialize the Pixel Measures Macro. 
 */
void svkDcmHeader::InitPixelMeasuresMacro( string pixelSpacing, string sliceThickness )
{
    this->AddSequenceItemElement(
        "SharedFunctionalGroupsSequence",
        0,
        "PixelMeasuresSequence"
    );


    this->AddSequenceItemElement(
        "PixelMeasuresSequence",
        0,
        "PixelSpacing",
        pixelSpacing,
        "SharedFunctionalGroupsSequence",
        0
    );

    this->AddSequenceItemElement(
        "PixelMeasuresSequence",
        0,
        "SliceThickness",
        sliceThickness,
        "SharedFunctionalGroupsSequence",
        0
    );
}


/*!
 *  Generalized form of method that uses DimensionVector.  
 *  The rows of the dimensionVector contain the max index of that dimension, i.e. size -1. 
 *  Initializes the Per Frame Functional Gruop Sequence and attributes derived from 
 *  method args, eg. dcos -> ImageOrientationPatient.  
 */
void svkDcmHeader::InitPerFrameFunctionalGroupSequence(double toplc[3], double voxelSpacing[3],
                                             double dcos[3][3], svkDcmHeader::DimensionVector* dimensionVector) 
{

    //  =============================================================================
    //  If pixel measures, etc are in the perFrame rather than shared functional 
    //  group, reinitialize them now in the shared func group so they aren't lost:
    //  =============================================================================
    bool reinitPixelMeasures            = false;
    if( this->ElementExists( "PixelMeasuresSequence", "PerFrameFunctionalGroupsSequence" ) ) {
        reinitPixelMeasures = true;
    }
    bool reinitPlaneOrientationMacro    = false;
    if( this->ElementExists( "PlaneOrientationSequence", "PerFrameFunctionalGroupsSequence" ) ) {
        reinitPlaneOrientationMacro = true;
    }
    bool reinitPixelValueTransformation = false;
    double slope;
    double intercept;
    if( this->ElementExists( "PixelValueTransformationSequence", "PerFrameFunctionalGroupsSequence" ) ) {
        reinitPixelValueTransformation = true;
        //get slope and intercept: 
        slope     = this->GetDoubleSequenceItemElement ( 
            "PixelValueTransformationSequence", 0, "RescaleSlope", "PerFrameFunctionalGroupsSequence" );
        intercept = this->GetDoubleSequenceItemElement ( 
            "PixelValueTransformationSequence", 0, "RescaleIntercept", "PerFrameFunctionalGroupsSequence" );
    }
    bool reinitMREchoMacro = false;
    float TE;
    if( this->ElementExists( "MREchoSequence", "PerFrameFunctionalGroupsSequence" ) ) {
        reinitMREchoMacro = true;
        TE = this->GetFloatSequenceItemElement( 
            "MREchoSequence", 0, "EffectiveEchoTime", "PerFrameFunctionalGroupsSequence", 0); 
    }
    bool reinitMRTimingAndRelatedParametersMacro = false; 
    float TR; 
    float flipAngle; 
    int numEchoes = 0; 
    if( this->ElementExists( "MRTimingAndRelatedParametersSequence", "PerFrameFunctionalGroupsSequence" ) ) {
        reinitMRTimingAndRelatedParametersMacro = true; 
        TR = this->GetFloatSequenceItemElement( 
            "MRTimingAndRelatedParametersSequence", 0, "RepetitionTime", "PerFrameFunctionalGroupsSequence", 0); 
        flipAngle = this->GetFloatSequenceItemElement(
            "MRTimingAndRelatedParametersSequence", 0, "FlipAngle", "PerFrameFunctionalGroupsSequence", 0); 
    }
    //  =============================================================================


    this->ClearSequence( "PerFrameFunctionalGroupsSequence" );

    //  obtain the number of frames for each dimension. 
    //  skip the first 2 dimensions: cols, rows: 
    int numFrames = this->GetNumberOfFrames(dimensionVector);
    this->SetValue( "NumberOfFrames", numFrames); 
    this->InitMultiFrameDimensionModule( dimensionVector ); 

    this->InitFrameContentMacro( dimensionVector ); 
    this->InitPlaneOrientationMacro( dcos ); 
    this->InitPlanePositionMacro( toplc, voxelSpacing, dcos, dimensionVector); 


    //  =============================================================================
    if ( reinitPixelMeasures == true ) {
        string pixelSpacingString = svkTypeUtils::DoubleToString(pixelSpacing[0]) + "\\" + svkTypeUtils::DoubleToString(pixelSpacing[1]);
        string sliceThicknessString = svkTypeUtils::DoubleToString(pixelSpacing[2]);
        this->InitPixelMeasuresMacro( pixelSpacingString, sliceThicknessString );
    }
    if ( reinitPlaneOrientationMacro == true ) {

        ostringstream ossDcos;
        for (int i = 0; i < 2; i++) {
            for (int j = 0; j < 3; j++) {
                ossDcos << dcos[i][j];
                if (i * j != 2) {
                    ossDcos<< "\\";
                }
            }
        }
        this->InitPlaneOrientationMacro( ossDcos.str() );
    }
    if ( reinitPixelValueTransformation == true ) {
        this->InitPixelValueTransformationMacro(slope, intercept);
    }
    if ( reinitMREchoMacro == true ) {
        this->InitMREchoMacro(TE); 
    }
    if ( reinitMRTimingAndRelatedParametersMacro == true ) {
        this->InitMRTimingAndRelatedParametersMacro(TR, flipAngle, numEchoes); 
    }

    //  =============================================================================

    //  Finally, to avoid confusion there should only be one ImageOrientationPatient entry in 
    //  the data set, within the Shared functional group
    if( this->ElementExists("ImageOrientationPatient", "top")) {
        //  TODO: this is probably fine, but triggers a lot of changes in test cases that I don't have time to 
        //  look at right now.  
        //this->RemoveElement( "ImageOrientationPatient" ); 
    }

}
 
 
/*!
 *  Generalized version of InitMultiFrameDimensionModule that adds an item to the
 *  sequence for each dimension type represented in the DimensionVector. 
 */
void svkDcmHeader::InitMultiFrameDimensionModule( svkDcmHeader::DimensionVector* dimensionVector) 
{

    this->ClearSequence( "DimensionIndexSequence" );

    int numSequenceDims = dimensionVector->size();     
    int indexCount = 0;
    // start from 2, since cols and rows aren't included 
    for ( int dim = 2; dim < numSequenceDims; dim++) {

        map< svkDcmHeader::DimensionIndexLabel, int > row; 
        map< svkDcmHeader::DimensionIndexLabel, int >::iterator it; 

        row = (*dimensionVector)[dim]; 
        it =   row.begin(); 
        svkDcmHeader::DimensionIndexLabel dimLabel = it->first;  

        this->AddSequenceItemElement(
            "DimensionIndexSequence",
            indexCount,
            "DimensionDescriptionLabel",
            this->DimensionIndexLabelToString( dimLabel ) 
        );

        unsigned short* dimensionIndexPointer = new unsigned short[2]; 
        unsigned short* functionalGroupPointer = new unsigned short[2]; 

        this->GetDimensionIndexPointer( dimLabel, dimensionIndexPointer, functionalGroupPointer); 

        this->AddSequenceItemElement(
            "DimensionIndexSequence",       //seqName
            indexCount,                     //seqItemPosition
            "DimensionIndexPointer",        //elementName
            dimensionIndexPointer,  
            2 
        ); 
        this->AddSequenceItemElement(
            "DimensionIndexSequence",       //seqName
            indexCount,                     //seqItemPosition
            "FunctionalGroupPointer",       //elementName
            functionalGroupPointer,  
            2 
        ); 
        
        delete [] dimensionIndexPointer; 
        delete [] functionalGroupPointer; 

        indexCount++; 
    }
}


/*!
 * 
 */
void svkDcmHeader::GetDimensionIndexPointer( svkDcmHeader::DimensionIndexLabel dimLabel, unsigned short* dimensionIndexPointer, unsigned short* functionalGroupPointer)
{

    string hexstring; 
    if ( dimLabel == svkDcmHeader::SLICE_INDEX ) {
        //  (0020,0032) DS [-119.53100\-159.75101\100.95700]        #  32, 3 ImagePositionPatient
        hexstring = "0x0020";
        dimensionIndexPointer[0] = (unsigned short)strtol(hexstring.c_str(), NULL, 0);
        hexstring = "0x0032";
        dimensionIndexPointer[1] = (unsigned short)strtol(hexstring.c_str(), NULL, 0);
        //  (0020,9113) SQ (Sequence with undefined length #=1)     # u/l, 1 PlanePositionSequence
        hexstring = "0x0020";
        functionalGroupPointer[0] = (unsigned short)strtol(hexstring.c_str(), NULL, 0);
        hexstring = "0x9113";
        functionalGroupPointer[1] = (unsigned short)strtol(hexstring.c_str(), NULL, 0);
    } else if ( dimLabel == svkDcmHeader::TIME_INDEX ) {
        //  (0020,9128) #   2, 1 TemporalPositionIndex
        hexstring = "0x0020";
        dimensionIndexPointer[0] = (unsigned short)strtol(hexstring.c_str(), NULL, 0);
        hexstring = "0x9128";
        dimensionIndexPointer[1] = (unsigned short)strtol(hexstring.c_str(), NULL, 0);
        //  (0020,9111) SQ (Sequence with undefined length #=8)     # u/l, 1 FrameContentSequence
        hexstring = "0x0020";
        functionalGroupPointer[0] = (unsigned short)strtol(hexstring.c_str(), NULL, 0);
        hexstring = "0x9111";
        functionalGroupPointer[1] = (unsigned short)strtol(hexstring.c_str(), NULL, 0);
    } else if ( dimLabel == svkDcmHeader::CHANNEL_INDEX ) {
        //  (0018,9047) SH [1]                                      #   2, 1 MultiCoilElementName
        hexstring = "0x0018";
        dimensionIndexPointer[0] = (unsigned short)strtol(hexstring.c_str(), NULL, 0);
        hexstring = "0x9047";
        dimensionIndexPointer[1] = (unsigned short)strtol(hexstring.c_str(), NULL, 0);
        //  (0018,9045) SQ (Sequence with undefined length #=8)     # u/l, 1 MultiCoilDefinitionSequence
        hexstring = "0x0018";
        functionalGroupPointer[0] = (unsigned short)strtol(hexstring.c_str(), NULL, 0);
        hexstring = "0x9045";
        functionalGroupPointer[1] = (unsigned short)strtol(hexstring.c_str(), NULL, 0);
    }

}


/*
 *  gets the dimension indices for the specified frame number and puts them into the 
 *  loopIndex svkDcmHeader::DimensionVector. 
 */
void svkDcmHeader::GetDimensionVectorIndexFromFrame( svkDcmHeader::DimensionVector* dimensionVector, svkDcmHeader::DimensionVector* loopIndex, int frame) 
{

    //cout << "frame : " << frame << endl;
    int minDimension = 2; 
     
    for ( int dimension = minDimension; dimension < dimensionVector->size(); dimension++ ) {

        //  Init each dim to 0:     
        svkDcmHeader::SetDimensionVectorValue( loopIndex, dimension, 0); 

        int innerSize = 1; 
        int currentSize = 1; 
        for ( int innerDimension = minDimension; innerDimension < dimension; innerDimension++ ) {
            innerSize *= ( svkDcmHeader::GetDimensionVectorValue(dimensionVector, innerDimension) + 1); 
        }
        currentSize = innerSize * ( svkDcmHeader::GetDimensionVectorValue(dimensionVector, dimension) + 1);

        int remainder = frame % currentSize; 
        int indexValue = remainder / innerSize; 
        svkDcmHeader::SetDimensionVectorValue( loopIndex, dimension, indexValue); 

    }
}


/*
 *  gets the dimension indices for the specified cellID number and puts them into the 
 *  loopIndex svkDcmHeader::DimensionVector.  
 */
void svkDcmHeader::GetDimensionVectorIndexFromCellID( svkDcmHeader::DimensionVector* dimensionVector, svkDcmHeader::DimensionVector* loopIndex, int cellID) 
{
    //cout << "cell ID: " << cellID << endl;
     
    for ( int dimension = 0; dimension < dimensionVector->size(); dimension++ ) {

        //  Init each dim to 0:     
        svkDcmHeader::SetDimensionVectorValue( loopIndex, dimension, 0); 

        int innerSize = 1; 
        int currentSize = 1; 
        for ( int innerDimension = 0; innerDimension < dimension; innerDimension++ ) {
            innerSize *= ( svkDcmHeader::GetDimensionVectorValue(dimensionVector, innerDimension) + 1); 
        }
        currentSize = innerSize * ( svkDcmHeader::GetDimensionVectorValue(dimensionVector, dimension) + 1);

        int remainder = cellID % currentSize; 
        int indexValue = remainder / innerSize; 
        svkDcmHeader::SetDimensionVectorValue( loopIndex, dimension, indexValue); 

    }
}


/*
 *  gets the cell data index number corresponding to the DimensionIndex values specified in the 
 *  loopIndex svkDcmHeader::DimensionVector. 
 */
int svkDcmHeader::GetCellIDFromDimensionVectorIndex( svkDcmHeader::DimensionVector* dimensionVector, svkDcmHeader::DimensionVector* loopIndex) 
{
    int cellIndex = 0; 

    for ( int dimension = 0; dimension < dimensionVector->size(); dimension++ ) {

        //  get sum of inner loops relative to current dimension: 
        int innerSize = 1; // default for innermost loop
        for ( int innerDimension = 0; innerDimension < dimension; innerDimension++ ) {
            innerSize *= ( svkDcmHeader::GetDimensionVectorValue(dimensionVector, innerDimension) + 1); 
        }

        cellIndex += innerSize * svkDcmHeader::GetDimensionVectorValue(loopIndex, dimension); 

    }

    return cellIndex; 


}


/*!
 *  Generalized form of method to init PlanePosition Macro using dimensions in DimensionVector
 */
void svkDcmHeader::InitPlanePositionMacro(double toplc[3], double voxelSpacing[3],
                                             double dcos[3][3], svkDcmHeader::DimensionVector* dimensionVector) 
{

    int frame = 0;

    //  make a copy to hold the indices of the current loop
    svkDcmHeader::DimensionVector loopIndex = *dimensionVector; 

    int numFrames = this->GetNumberOfFrames(dimensionVector);
    
    int colsT = 4; 
    int rowsT = 3; 
    float transformIndicesToLPS[4][3]; 

    //  column 0
    transformIndicesToLPS[0][0] = dcos[0][0] * voxelSpacing[0]; 
    transformIndicesToLPS[0][1] = dcos[0][1] * voxelSpacing[0]; 
    transformIndicesToLPS[0][2] = dcos[0][2] * voxelSpacing[0]; 

    //  column 1
    transformIndicesToLPS[1][0] = dcos[1][0] * voxelSpacing[1]; 
    transformIndicesToLPS[1][1] = dcos[1][1] * voxelSpacing[1]; 
    transformIndicesToLPS[1][2] = dcos[1][2] * voxelSpacing[1]; 

    //  column 2
    transformIndicesToLPS[2][0] = dcos[2][0] * voxelSpacing[2]; 
    transformIndicesToLPS[2][1] = dcos[2][1] * voxelSpacing[2]; 
    transformIndicesToLPS[2][2] = dcos[2][2] * voxelSpacing[2]; 

    //  column 3
    transformIndicesToLPS[3][0] = toplc[0]; 
    transformIndicesToLPS[3][1] = toplc[1]; 
    transformIndicesToLPS[3][2] = toplc[2]; 

    if (this->GetDebug()) {
        cout << endl;
        cout << "DCOS:    " << dcos[0][0]  << " " << dcos[0][1]  << " " << dcos[0][2]  << endl; 
        cout << "DCOS:    " << dcos[1][0]  << " " << dcos[1][1]  << " " << dcos[1][2]  << endl; 
        cout << "DCOS:    " << dcos[2][0]  << " " << dcos[2][1]  << " " << dcos[2][2]  << endl; 
        cout << "SPACING: " << voxelSpacing[0] << " " <<  voxelSpacing[1] << " " << voxelSpacing[2] << endl;
        cout << "TOPLC:   " << toplc[0] << " " << toplc[1] << " " << toplc[2] << endl;
        cout << endl;
        cout << "T: " << transformIndicesToLPS[0][0]  << " " << transformIndicesToLPS[1][0]  << " " << transformIndicesToLPS[2][0]  << endl; 
        cout << "T: " << transformIndicesToLPS[0][1]  << " " << transformIndicesToLPS[1][1]  << " " << transformIndicesToLPS[2][1]  << endl; 
        cout << "T: " << transformIndicesToLPS[0][2]  << " " << transformIndicesToLPS[1][2]  << " " << transformIndicesToLPS[2][2]  << endl; 
        cout << endl;
    }

    int rowsIJK = 4; 
    int ijk[4]; 

    int rowsLPS = 3; 
    float lps[3]; 


    for ( int frame = 0; frame < numFrames; frame++) {

        //  Loop over dimensions outter to the slice dimension
        svkDcmHeader::GetDimensionVectorIndexFromFrame(dimensionVector, &loopIndex, frame );

        float displacement[3]={0,0,0};
        float frameLPSPosition[3]={0,0,0};

        int sliceNumber = svkDcmHeader::GetDimensionVectorValue(&loopIndex, svkDcmHeader::SLICE_INDEX); 

        this->AddSequenceItemElement(
            "PerFrameFunctionalGroupsSequence",
            sliceNumber,
            "PlanePositionSequence"
        );
        //cout << "FRAME: " << frame << " -> " ; 
        //  Add displacement along normal vector:
        //  for indices i=0,j=0, k = sliceNumber, i.e. displacement along a vector
        //  throught the TLC points of each slice:
        //      0,0,0 -> 0,0,1 -> 0,0,2 -> etc.
        ijk[0] = 0; 
        ijk[1] = 0; 
        ijk[2] = sliceNumber; 
        ijk[3] = 1; 
        for ( int row = 0; row < 3; row++ ) {
            lps[row] = 0; 
            for (int  col = 0; col < 4; col++ ) {
                //cout << "lps +=  " << transformIndicesToLPS[col][row] << " * " <<  ijk[col] << endl;
                lps[row] += transformIndicesToLPS[col][row] * ijk[col]; 
            }
        }

        for(int j = 0; j < 3; j++) { //L, P, S
            frameLPSPosition[j] = lps[j]; 
        }
        //cout << "lps: " << lps[0] << " " << lps[1] << " " << lps[2] << endl;

        string imagePositionPatient;
        for (int j = 0; j < 3; j++) {
            ostringstream oss;
            oss.setf(ios::fixed);
            oss.precision(5);
            oss << frameLPSPosition[j];
            imagePositionPatient += oss.str();
            if (j < 2) {
                imagePositionPatient += '\\';
            }
        }

        this->AddSequenceItemElement(
            "PlanePositionSequence",
            0,
            "ImagePositionPatient",
            imagePositionPatient,
            "PerFrameFunctionalGroupsSequence",
            frame
        );

    }
}



/*!
 *  Generalized form of Init method using DimensionVector. 
 */
void svkDcmHeader::InitFrameContentMacro( svkDcmHeader::DimensionVector* dimensionVector)
{

    int numFrameIndices = dimensionVector->size() - 2;  // don't include cols and rows 
    unsigned int* indexValues = new unsigned int[numFrameIndices];

    //  set up vector to store indices for a given frame: 
    svkDcmHeader::DimensionVector loopIndex = *dimensionVector; 

    //  convert each frame into a set of dimension index values, by converting the 
    //  frame index into a dimension index. 
    int numFrames = this->GetNumberOfFrames(dimensionVector);

    for ( int frame = 0; frame < numFrames; frame++) {
        
        // get the indices for this frame: 
        svkDcmHeader::GetDimensionVectorIndexFromFrame( dimensionVector, &loopIndex, frame); 
        
        for (int i = 0; i < numFrameIndices; i++) {
            indexValues[i] = svkDcmHeader::GetDimensionVectorValue( &loopIndex, i+2); 
        }

        this->AddSequenceItemElement(
            "PerFrameFunctionalGroupsSequence",
            frame,
            "FrameContentSequence"
        );

        this->AddSequenceItemElement(
            "FrameContentSequence",
            0,
            "DimensionIndexValues",
            indexValues,        //  array of vals
            numFrameIndices,    // num values in array
            "PerFrameFunctionalGroupsSequence",
            frame
        );

        this->AddSequenceItemElement(
            "FrameContentSequence",
            0,
            "FrameAcquisitionDateTime",
            this->GetStringValue( "StudyDate" ),
            "PerFrameFunctionalGroupsSequence",
            frame
        );

        this->AddSequenceItemElement(
            "FrameContentSequence",
            0,
            "FrameReferenceDateTime",
            this->GetStringValue( "StudyDate" ),
            "PerFrameFunctionalGroupsSequence",
            frame
        );

        this->AddSequenceItemElement(
            "FrameContentSequence",
            0,
            "FrameAcquisitionDuration",
            "-1",
            "PerFrameFunctionalGroupsSequence",
            frame
        );

    }

    this->SetValue( "NumberOfFrames", numFrames); 

    delete[] indexValues;
}


/*!
 *  Set the linear scaling factors in the VOI LUT Module.  These define the output range 
 *  of pixel values to be displlayed. 
 */
void svkDcmHeader::InitVOILUTModule(float center, float width)
{

    this->SetValue(
        "WindowCenter",
        center 
    );

    this->SetValue(
        "WindowWidth",
        width 
    );

    this->SetValue(
        "VOILUTFunction",
        "LINEAR" 
    );
}


/*!
 *  Set the linear scaling factors in the Pixel Value Transformation Macro
 */
void svkDcmHeader::InitPixelValueTransformationMacro(double slope, double intercept)
{

    this->AddSequenceItemElement(
        "SharedFunctionalGroupsSequence",
        0,
        "PixelValueTransformationSequence"
    );

    this->AddSequenceItemElement(
        "PixelValueTransformationSequence",
        0,
        "RescaleIntercept",
        intercept,
        "SharedFunctionalGroupsSequence",
        0
    );

    this->AddSequenceItemElement(
        "PixelValueTransformationSequence",
        0,
        "RescaleSlope",
        slope,
        "SharedFunctionalGroupsSequence",
        0
    );

    this->AddSequenceItemElement(
        "PixelValueTransformationSequence",
        0,
        "RescaleType",
        "US",   //enum unspecified required for MR modality
        "SharedFunctionalGroupsSequence",
        0
    );
}


/*!
 *  Initialize MR Imaging Modifier Macro
 */
void svkDcmHeader::InitMRImagingModifierMacro(float transmitFreq, float pixelBandwidth, string magTransfer, string bloodNulling)
{

    this->AddSequenceItemElement(
        "SharedFunctionalGroupsSequence",
        0,
        "MRImagingModifierSequence"
    );

    this->AddSequenceItemElement(
        "MRImagingModifierSequence",
        0,
        "MagnetizationTransfer",
        magTransfer,
        "SharedFunctionalGroupsSequence",
        0
    );

    this->AddSequenceItemElement(
        "MRImagingModifierSequence",
        0,
        "BloodSignalNulling",
        bloodNulling,
        "SharedFunctionalGroupsSequence",
        0
    );

    this->AddSequenceItemElement(
        "MRImagingModifierSequence",
        0,
        "Tagging",
        string("NONE"),
        "SharedFunctionalGroupsSequence",
        0
    );

    this->AddSequenceItemElement(
        "MRImagingModifierSequence",
        0,
        "TransmitterFrequency",
        transmitFreq,
        "SharedFunctionalGroupsSequence",
        0
    );

    this->AddSequenceItemElement(
        "MRImagingModifierSequence",
        0,
        "PixelBandwidth",
        pixelBandwidth, 
        "SharedFunctionalGroupsSequence",
        0
    );
}

/*!
 *  Initializes the Volume Localization Sequence in the MRSpectroscopy
 *  DICOM object for PRESS excitation.
 */
void svkDcmHeader::InitVolumeLocalizationSeq(float size[3], float center[3], float dcos[3][3])
{

    this->InsertEmptyElement( "VolumeLocalizationSequence" );

    string midSlabPosition;
    for (int i = 0; i < 3; i++) {
        ostringstream oss;
        oss << center[i];
        midSlabPosition += oss.str();
        if (i < 2) {
            midSlabPosition += '\\';
        }
    }

    //  Volume Localization (PRESS BOX)
    for (int i = 0; i < 3; i++) {

        this->AddSequenceItemElement(
            "VolumeLocalizationSequence",
            i,
            "SlabThickness",
            size[i]
        );

        this->AddSequenceItemElement(
            "VolumeLocalizationSequence",
            i,
            "MidSlabPosition",
            midSlabPosition
        );

        string slabOrientation;
        for (int j = 0; j < 3; j++) {
            ostringstream oss;
            oss << dcos[i][j];
            slabOrientation += oss.str();
            if (j < 2) {
                slabOrientation += '\\';
            }
        }

        this->AddSequenceItemElement(
            "VolumeLocalizationSequence",
            i,
            "SlabOrientation",
            slabOrientation
        );

    }
}


/*!
 *  Initialize MR Timing and Related Parameters Macro. 
 */
void svkDcmHeader::InitMRTimingAndRelatedParametersMacro(float tr, float flipAngle, int numEchoes)
{

    this->AddSequenceItemElement(
        "SharedFunctionalGroupsSequence",
        0,
        "MRTimingAndRelatedParametersSequence"
    );

    this->AddSequenceItemElement(
        "MRTimingAndRelatedParametersSequence",
        0,
        "RepetitionTime",
        tr,
        "SharedFunctionalGroupsSequence",
        0
    );

    if ( flipAngle == -999 ) {
        this->AddSequenceItemElement(
            "MRTimingAndRelatedParametersSequence",
            0,
            "FlipAngle",
            "-1",
            "SharedFunctionalGroupsSequence",
            0
        );
    } else {
        this->AddSequenceItemElement(
            "MRTimingAndRelatedParametersSequence",
            0,
            "FlipAngle",
            flipAngle,
            "SharedFunctionalGroupsSequence",
            0
        );
    }

    this->AddSequenceItemElement(
        "MRTimingAndRelatedParametersSequence",
        0,
        "EchoTrainLength",
        numEchoes,
        "SharedFunctionalGroupsSequence",
        0
    );

    this->AddSequenceItemElement(
        "MRTimingAndRelatedParametersSequence",
        0,
        "RFEchoTrainLength",
        1,
        "SharedFunctionalGroupsSequence",
        0
    );

    this->AddSequenceItemElement(
        "MRTimingAndRelatedParametersSequence",
        0,
        "GradientEchoTrainLength",
        1,
        "SharedFunctionalGroupsSequence",
        0
    );

    this->AddSequenceItemElement(
        "MRTimingAndRelatedParametersSequence",
        0,
        "GradientEchoTrainLength",
        1,
        "SharedFunctionalGroupsSequence",
        0
    );
}


/*!
 *  Initializes the MR Echo Macro. 
 */
void svkDcmHeader::InitMREchoMacro(float TE)
{
    this->AddSequenceItemElement(
        "SharedFunctionalGroupsSequence",
        0,
        "MREchoSequence"
    );
    this->AddSequenceItemElement(
        "MREchoSequence",
        0,
        "EffectiveEchoTime",
        TE,
        "SharedFunctionalGroupsSequence",
        0
    );
}


/*!
 *  Initialize MR Modifier Macro. 
 */
void svkDcmHeader::InitMRModifierMacro(float inversionTime)
{   
    this->AddSequenceItemElement(
        "SharedFunctionalGroupsSequence",
        0,
        "MRModifierSequence"
    );

    string invRecov = "NO"; 
    if ( inversionTime >= 0 ) {
        invRecov.assign("YES");
        this->AddSequenceItemElement(
            "MRModifierSequence",
            0,
            "InversionTimes",
            inversionTime,
            "SharedFunctionalGroupsSequence",
            0
        );
    }

    this->AddSequenceItemElement(
        "MRModifierSequence",
        0,
        "InversionRecovery",
        invRecov,
        "SharedFunctionalGroupsSequence",
        0
    );

    this->AddSequenceItemElement(
        "MRModifierSequence",
        0,
        "SpatialPresaturation",
        string("SLAB"),
        "SharedFunctionalGroupsSequence",
        0
    );

    this->AddSequenceItemElement(
        "MRModifierSequence",
        0,
        "ParallelAcquisition",
        string("NO"),
        "SharedFunctionalGroupsSequence",
        0
    );
}


/*!
 *  Initialize MR Transmit Coil Macro. 
 */
void svkDcmHeader::InitMRTransmitCoilMacro(string coilMfg, string coilName, string coilType)
{   
    this->AddSequenceItemElement(
        "SharedFunctionalGroupsSequence",
        0,
        "MRTransmitCoilSequence"
    );

    this->AddSequenceItemElement(
        "MRTransmitCoilSequence",
        0,
        "TransmitCoilName",
        coilName,
        "SharedFunctionalGroupsSequence",
        0
    );

    this->AddSequenceItemElement(
        "MRTransmitCoilSequence",
        0,
        "TransmitCoilManufacturerName",
        coilMfg,
        "SharedFunctionalGroupsSequence",
        0
    );

    this->AddSequenceItemElement(
        "MRTransmitCoilSequence",
        0,
        "TransmitCoilType",
        coilType,
        "SharedFunctionalGroupsSequence",
        0
    );
}


/*!
 *  Initialize MR Averages Macro. 
 */
void svkDcmHeader::InitMRAveragesMacro(int numAverages)
{
    this->AddSequenceItemElement(
        "SharedFunctionalGroupsSequence",
        0,
        "MRAveragesSequence"
    );

    this->AddSequenceItemElement(
        "MRAveragesSequence",
        0,
        "NumberOfAverages",
        numAverages,
        "SharedFunctionalGroupsSequence",
        0
    );
}


/*!
 *  Initialize Raw Data Module. 
 */
void svkDcmHeader::InitRawDataModule( string contentDate, string contentTime, void* rawFile ) 
{

    if ( !contentDate.empty() ) {
        this->SetValue(
            "ContentDate",
            contentDate
        );
    }

    if ( !contentTime.empty() ) {
        this->SetValue(
            "ContentTime",
            contentTime
        );
    }
}


/*!
 * 
 */
svkDcmHeader::DcmPixelDataFormat svkDcmHeader::GetVtkDataTypeFromSvkDataType( vtkIdType vtkType)
{
    if ( vtkType == VTK_DOUBLE ) {
        return svkDcmHeader::SIGNED_FLOAT_8;
    } else if ( vtkType == VTK_FLOAT ) {
        return svkDcmHeader::SIGNED_FLOAT_4;
    } else if ( vtkType == VTK_FLOAT ) {
        return svkDcmHeader::SIGNED_FLOAT_4;
    } else if ( vtkType == VTK_UNSIGNED_CHAR) {
        return svkDcmHeader::UNSIGNED_INT_1;
    } else if ( vtkType == VTK_UNSIGNED_SHORT) {
        return svkDcmHeader::UNSIGNED_INT_2;
    } else if ( vtkType == VTK_SHORT) {
        return svkDcmHeader::SIGNED_INT_2;
    } else {
        return svkDcmHeader::UNDEFINED; 
    }
}


/*!
 *  Initializes an MR Image Storage header from an Enhanced  MRI Storage header. 
 */
int svkDcmHeader::ConvertEnhancedMriToMriHeader(svkDcmHeader* mri, vtkIdType dataType )
{

    //
    //  Patient IE requires modification
    //
    mri->InitPatientModule(
        this->GetStringValue( "PatientName" ),
        this->GetStringValue( "PatientID" ),
        this->GetStringValue( "PatientBirthDate" ),
        this->GetStringValue( "PatientSex" )
    );


    //
    //  General Study IE requires modification
    //
    mri->InitGeneralStudyModule(
        this->GetStringValue( "StudyDate" ),
        this->GetStringValue( "StudyTime" ),
        this->GetStringValue( "ReferringPhysicianName" ),
        this->GetStringValue( "StudyID" ),
        this->GetStringValue( "AccessionNumber" ), 
        this->GetStringValue( "StudyInstanceUID" )
    );

    //
    //  General Series Module
    //
    mri->InitGeneralSeriesModule(
        "77",
        this->GetStringValue( "SeriesDescription" ),
        this->GetStringValue( "PatientPosition" )
    );

    //  Note that the initial copy doesn't init the position value. 
    //  Each file will need to do this for the appropriate frame
    mri->InitImagePlaneModule( 
        "", 
        this->GetStringSequenceItemElement (
            "PixelMeasuresSequence",
            0,
            "PixelSpacing",
            "SharedFunctionalGroupsSequence"
        ), 
        this->GetStringSequenceItemElement(
            "PlaneOrientationSequence",
            0,
            "ImageOrientationPatient",
            "SharedFunctionalGroupsSequence"
        ), 
        this->GetStringSequenceItemElement (
            "PixelMeasuresSequence",
            0,
            "SliceThickness",
            "SharedFunctionalGroupsSequence"
        )
    );

    //
    //  Image Pixel Module
    //  Set DCM data type based on vtkImageData Scalar type:
    //
    svkDcmHeader::DcmPixelDataFormat dcmDataType;
    dcmDataType = static_cast< svkDcmHeader::DcmPixelDataFormat > (this->GetPixelDataType( dataType ) ); 

    mri->InitImagePixelModule(
        this->GetIntValue( "Rows"),
        this->GetIntValue( "Columns"),
        dcmDataType
    );

    //  Init MR Image Module
    mri->InitMRImageModule( 
        this->GetStringSequenceItemElement(
            "MRTimingAndRelatedParametersSequence",
            0,
            "RepetitionTime",
            "SharedFunctionalGroupsSequence"
        ), 
        this->GetStringSequenceItemElement(
            "MREchoSequence",
            0,
            "EffectiveEchoTime",
            "SharedFunctionalGroupsSequence"
        )
    ); 

    if( this->ElementExists("SVK_VOXEL_TAGGING_SEQUENCE")) {
		mri->SetValue( "SVK_PRIVATE_TAG",  "SVK_PRIVATE_CREATOR");
		this->CopySequence( mri, "SVK_VOXEL_TAGGING_SEQUENCE");
    }


    return 0; 
}


/*!
 *  Initializes an Enhanced MR Image header from an MR Spectroscopy header. 
 *  This is a utility for extracting metabolite maps from MRS data, for example. 
 *  Takes an initialized svkDcmHeader from an svkMriImageData object. 
 */
int svkDcmHeader::InitDerivedMRIHeader(svkDcmHeader* mri, vtkIdType dataType, string seriesDescription)
{

    //"verify that "this" is an svkMRSpectroscopy Object"

    mri->SetValue("ImageType", "DERIVED\\SECONDARY");

    //
    //  Patient IE requires modification
    //
    mri->InitPatientModule(
        this->GetStringValue( "PatientName" ),
        this->GetStringValue( "PatientID" ),
        this->GetStringValue( "PatientBirthDate" ),
        this->GetStringValue( "PatientSex" )
    );


    //
    //  General Study IE requires modification
    //
    mri->InitGeneralStudyModule(
        this->GetStringValue("StudyDate"),
        this->GetStringValue("StudyTime"),
        this->GetStringValue("ReferringPhysicianName"),
        this->GetStringValue("StudyID"),
        this->GetStringValue("AccessionNumber"), 
        this->GetStringValue("StudyInstanceUID")
    );

    //
    //  General Series Module
    //
    mri->InitGeneralSeriesModule(
        "77",
        seriesDescription,
        this->GetStringValue("PatientPosition")
    );

    //
    //  Image Pixel Module
    //  Set DCM data type based on vtkImageData Scalar type:
    //
    svkDcmHeader::DcmPixelDataFormat dcmDataType; 

    if ( dataType == VTK_DOUBLE ) {
        dcmDataType = svkDcmHeader::SIGNED_FLOAT_8;
    } else if ( dataType == VTK_FLOAT ) {
        dcmDataType = svkDcmHeader::SIGNED_FLOAT_4;
    } else if ( dataType == VTK_SHORT ) {
        dcmDataType = svkDcmHeader::SIGNED_INT_2;
    } else if ( dataType == VTK_UNSIGNED_SHORT ) {
        dcmDataType = svkDcmHeader::UNSIGNED_INT_2;
    } else {
        cout << this->GetClassName() << ": Unsupported ScalarType " << dataType << endl;
        exit(1);
    }

    mri->InitImagePixelModule(
        this->GetIntValue( "Rows"),
        this->GetIntValue( "Columns"),
        dcmDataType
    );

    //
    //  Per Frame Functinal Groups Module
    //
    int numSlices = this->GetNumberOfSlices();
    double dcos[3][3] = {{0}};
    this->GetDataDcos( dcos );

    double pixelSpacing[3];
    this->GetPixelSpacing( pixelSpacing );

    double toplc[3];
    this->GetOrigin( toplc, 0 );


    mri->SetSliceOrder( this->dataSliceOrder );

    // Add Pixel Spacing
    string pixelSizes = this->GetStringSequenceItemElement (
                                        "PixelMeasuresSequence",
                                        0,
                                        "PixelSpacing",
                                        "SharedFunctionalGroupsSequence"
                                    );

    string sliceThickness = this->GetStringSequenceItemElement (
                                        "PixelMeasuresSequence",
                                        0,
                                        "SliceThickness",
                                        "SharedFunctionalGroupsSequence"
                                    );

    mri->InitPixelMeasuresMacro(  pixelSizes, sliceThickness );

    mri->AddSequenceItemElement(
            "SharedFunctionalGroupsSequence",
            0,
            "PlaneOrientationSequence"
        );

    mri->InitPlaneOrientationMacro(
        this->GetStringSequenceItemElement(
            "PlaneOrientationSequence",
            0,
            "ImageOrientationPatient",
            "SharedFunctionalGroupsSequence"
        )
    );

    svkDcmHeader::DimensionVector dimensionVector = this->GetDimensionIndexVector();
    svkDcmHeader::SetDimensionVectorValue(&dimensionVector, svkDcmHeader::SLICE_INDEX, numSlices-1);
    svkDcmHeader::SetDimensionVectorValue(&dimensionVector, svkDcmHeader::TIME_INDEX, 0);
    svkDcmHeader::SetDimensionVectorValue(&dimensionVector, svkDcmHeader::CHANNEL_INDEX, 0);
    mri->InitPerFrameFunctionalGroupSequence( toplc, pixelSpacing, dcos, &dimensionVector );

    return 0; 
}


/*!
 *  Replaces PHI field with "DEIDENTIFIED" string. If phiType is LIMITED, then 
 *  dates are preserved. This method does not look at private
 *  or nested tags.  
 *      0008,0018 SOPInstanceUID=UIDROOT,SOPInstanceUID
 *      0008,0020 StudyDate 
 *      0008,0021 SeriesDate
 *      0008,0022 AcquisitionDate
 *      0008,0023 ContentDate
 *      0008,0050 AccessionNumber
 *      0008,0080 InstitutionName
 *      0008,0090 ReferringPhysicianName
 *      0008,1060 NameOfPhysiciansReadingStudy
 *      0008,1155 RefSOPInstanceUID
 *      0010,0010 PatientName
 *      0010,0020 PatientID
 *      0010,0030 PatientBirthDate
 *      0010,1000 OtherPatientIDs
 *      0010,1001 OtherPatientNames
 *      0010,1040 PatientAddress 
 *      0010,1060 PatientMotherBirthName
 *      0010,2154 PatientTelephoneNumbers
 *      0020,000D StudyInstanceUID
 *      0020,000E SeriesInstanceUID
 *      0020,0010 StudyID
 *      0020,0052 FrameOfReferenceUID
 *      0028,0301 BurnedInAnnotation
 *      0032,1032 RequestingPhysician
 *      0040,1001 RequestedProcedureID
 *      0040,0009 ScheduledProcedureStepID
 *      0040,A124 UID
 *      0088,0140 StorageMediaFileSetUID
 *      3006,0024 ReferencedFrameOfReferenceUID
 *      3006,00C2 RelatedFrameOfReferenceUID
 */
void svkDcmHeader::Deidentify( PHIType phiType )
{    
    // set both patientId and studyId to DEIDENTIFIED:
    this->Deidentify( phiType, "DEIDENTIFIED" );
}


/*!
 *  Replaces PHI field with id. If phiType is LIMITED, then 
 *  dates are preserved. This method does not look at private
 *  or nested tags.  
 *      0008,0018 SOPInstanceUID=UIDROOT,SOPInstanceUID
 *      0008,0020 StudyDate 
 *      0008,0021 SeriesDate
 *      0008,0022 AcquisitionDate
 *      0008,0023 ContentDate
 *      0008,0050 AccessionNumber
 *      0008,0080 InstitutionName
 *      0008,0090 ReferringPhysicianName
 *      0008,1060 NameOfPhysiciansReadingStudy
 *      0008,1155 RefSOPInstanceUID
 *      0010,0010 PatientName
 *      0010,0020 PatientID
 *      0010,0030 PatientBirthDate
 *      0010,1000 OtherPatientIDs
 *      0010,1001 OtherPatientNames
 *      0010,1040 PatientAddress 
 *      0010,1060 PatientMotherBirthName
 *      0010,2154 PatientTelephoneNumbers
 *      0020,000D StudyInstanceUID
 *      0020,000E SeriesInstanceUID
 *      0020,0010 StudyID
 *      0020,0052 FrameOfReferenceUID
 *      0028,0301 BurnedInAnnotation
 *      0032,1032 RequestingPhysician
 *      0040,1001 RequestedProcedureID
 *      0040,0009 ScheduledProcedureStepID
 *      0040,A124 UID
 *      0088,0140 StorageMediaFileSetUID
 *      3006,0024 ReferencedFrameOfReferenceUID
 *      3006,00C2 RelatedFrameOfReferenceUID
 */
void svkDcmHeader::Deidentify( PHIType phiType, string id )
{    
    // set both patientId and studyId to the same value:
    this->Deidentify( phiType, id, id );
}


/*!
 *  Replaces PHI patient fields with patientId, study fields and all other PHI fields 
 *  with studyId.  If phiType is LIMITED, then dates are preserved. This
 *  method does not look at private nested tags. 
 *      0008,0018 SOPInstanceUID=UIDROOT,SOPInstanceUID
 *      0008,0020 StudyDate 
 *      0008,0021 SeriesDate
 *      0008,0022 AcquisitionDate
 *      0008,0023 ContentDate
 *      0008,0050 AccessionNumber
 *      0008,0080 InstitutionName
 *      0008,0090 ReferringPhysicianName
 *      0008,1060 NameOfPhysiciansReadingStudy
 *      0008,1155 RefSOPInstanceUID
 *      0010,0010 PatientName
 *      0010,0020 PatientID
 *      0010,0030 PatientBirthDate
 *      0010,1000 OtherPatientIDs
 *      0010,1001 OtherPatientNames
 *      0010,1040 PatientAddress 
 *      0010,1060 PatientMotherBirthName
 *      0010,2154 PatientTelephoneNumbers
 *      0020,000D StudyInstanceUID
 *      0020,000E SeriesInstanceUID
 *      0020,0010 StudyID
 *      0020,0052 FrameOfReferenceUID
 *      0028,0301 BurnedInAnnotation
 *      0032,1032 RequestingPhysician
 *      0040,1001 RequestedProcedureID
 *      0040,0009 ScheduledProcedureStepID
 *      0040,A124 UID
 *      0088,0140 StorageMediaFileSetUID
 *      3006,0024 ReferencedFrameOfReferenceUID
 *      3006,00C2 RelatedFrameOfReferenceUID
 */
void svkDcmHeader::Deidentify( PHIType phiType, string patientId, string studyId )
{     

    //  These fields are removed from PHI_LIMITED and PHI_DEIDENTIFIED data sets: 
    if ( phiType == svkDcmHeader::PHI_DEIDENTIFIED || phiType == PHI_LIMITED ) {
        string emptyString = "";
        string newUID = "";

        newUID = this->GenerateUniqueUID(); 
        this->ModifyValueRecursive( "MediaStorageSOPInstanceUID",    newUID); 
        this->ModifyValueRecursive( "SOPInstanceUID",                newUID); 

        this->ModifyValueRecursive( "AccessionNumber",               studyId); 
        this->ModifyValueRecursive( "InstitutionName",               studyId); 
        this->ModifyValueRecursive( "ReferringPhysicianName",        studyId); 

        if( this->ElementExists( "NameOfPhysiciansReadingStudy" ) ) {
            this->ModifyValueRecursive( "NameOfPhysiciansReadingStudy", emptyString); 
        }

        newUID = this->GenerateUniqueUID(); 
        this->ModifyValueRecursive( "ReferencedSOPInstanceUID",      newUID); 
        this->ModifyValueRecursive( "PatientName",                   patientId); 
        this->ModifyValueRecursive( "PatientID",                     patientId); 

        if( this->ElementExists( "OtherPatientIDs" ) ) {
            this->ModifyValueRecursive( "OtherPatientIDs", emptyString); 
        }

        if( this->ElementExists( "OtherPatientNames" ) ) {
            this->ModifyValueRecursive( "OtherPatientNames", emptyString); 
        }

        if( this->ElementExists( "PatientAddress" ) ) {
            this->ModifyValueRecursive( "PatientAddress",                emptyString); 
        }

        if( this->ElementExists( "PatientMotherBirthName" ) ) {
            this->ModifyValueRecursive( "PatientMotherBirthName",       emptyString); 
        }

        if( this->ElementExists( "PatientTelephoneNumbers" ) ) {
            this->ModifyValueRecursive( "PatientTelephoneNumbers",       emptyString); 
        }

        newUID = this->GenerateUniqueUID(); 
        this->ModifyValueRecursive( "StudyInstanceUID",              newUID); 

        newUID = this->GenerateUniqueUID(); 
        this->ModifyValueRecursive( "SeriesInstanceUID",             newUID); 

        this->ModifyValueRecursive( "StudyID",                       studyId); 

        newUID = this->GenerateUniqueUID(); 
        this->ModifyValueRecursive( "FrameOfReferenceUID",           newUID); 

        this->ModifyValueRecursive( "BurnedInAnnotation",            studyId); 

        if( this->ElementExists( "RequestingPhysician" ) ) {
            this->ModifyValueRecursive( "RequestingPhysician",          emptyString); 
        }

        if( this->ElementExists( "RequestedProcedureID" ) ) {
            this->ModifyValueRecursive( "RequestedProcedureID",         emptyString); 
        }

        if( this->ElementExists( "ScheduledProcedureStepID" ) ) {
            this->ModifyValueRecursive( "ScheduledProcedureStepID",     emptyString); 
        }

        newUID = this->GenerateUniqueUID(); 
        this->ModifyValueRecursive( "UID",                           newUID); 

        newUID = this->GenerateUniqueUID(); 
        this->ModifyValueRecursive( "StorageMediaFileSetUID",        newUID); 

        newUID = this->GenerateUniqueUID(); 
        this->ModifyValueRecursive( "ReferencedFrameOfReferenceUID", newUID); 

        newUID = this->GenerateUniqueUID(); 
        this->ModifyValueRecursive( "RelatedFrameOfReferenceUID",    newUID); 
    }   

    //  These fields are not removed from PHI_LIMITED data sets 
    if ( phiType == svkDcmHeader::PHI_DEIDENTIFIED ) {
        string emptyString = "";
        this->ModifyValueRecursive( "StudyDate",                       emptyString);
        this->ModifyValueRecursive( "SeriesDate",                      emptyString);
        this->ModifyValueRecursive( "AcquisitionDate",                 emptyString);
        this->ModifyValueRecursive( "ContentDate",                     emptyString);
        this->ModifyValueRecursive( "InstanceCreationDate",            emptyString);
        this->ModifyValueRecursive( "PerformedProcedureStepStartDate", emptyString);
        this->ModifyValueRecursive( "PerformedProcedureStepEndDate",   emptyString);
        this->ModifyValueRecursive( "AttributeModificationDateTime",   emptyString);
        this->SetValue( "PatientBirthDate",                            emptyString);
    }

    if ( phiType == svkDcmHeader::PHI_DEIDENTIFIED ) {
        this->SetValue( "DeidentificationMethod", "DEIDENTIFIED" ); 
    } else if ( phiType == svkDcmHeader::PHI_LIMITED ) {
        this->SetValue( "DeidentificationMethod", "LIMITED" ); 
    }
}


/*!
 *  Check for magic DICM chars at byte 128 to test if the file is a DICOM file. 
 */
bool svkDcmHeader::IsFileDICOM( string fname)
{
    bool isDICOM = false; 

    try {
        ifstream* file = new ifstream();
        file->exceptions( ifstream::eofbit | ifstream::failbit | ifstream::badbit );
        file->open( fname.c_str(), ios::binary ); 

        // get length of file:
        file->seekg (0, ios::end);
        long int fileLength = file->tellg();

        if ( fileLength >= 131 ) {

            file->seekg (0, ios::beg);
            //  try to read if not eof
            file->seekg(128, ios::beg); 

            char magicDICOMChars[5]; 
            file->read( magicDICOMChars, 4); 
            //  terminate
            magicDICOMChars[4] = '\0'; 
   
            string magicString( magicDICOMChars ); 

            if ( magicString.compare("DICM") == 0 ) {
                isDICOM = true;
            }
        }

        file->close(); 

        delete file; 

    } catch (const exception& e) {
        cerr << "ERROR(svkDcmHeader::IsFileDICOM opening or reading file (" << fname << "): " << e.what() << endl;
    }

    return isDICOM;
}


/*!
 *  Gets number of frames.  Since this isn't necessarily present for single frame objects, 
 *  default to 1: 
 */
int svkDcmHeader::GetNumberOfFrames()
{
    int numberOfFrames;
    if ( this->ElementExists("NumberOfFrames") ) {
        numberOfFrames = this->GetIntValue( "NumberOfFrames" );
    } else {
        numberOfFrames = 1;
    }
    return numberOfFrames; 
}


/*  
 *  Gets the number of frames defined by a svkDcmHeader DimensionVector instance
 *  the DimensionVector input should be initialized so that each index value is the
 *  max value for that dimension. Since indexing starts at 0 this is 1 less than the
 *  dimension size. 
 */
int svkDcmHeader::GetNumberOfFrames(svkDcmHeader::DimensionVector* dimensionVector)
{
    int numFrames = 1; 
    //the dimension index sequence starts at index 0, so ignore cols and rows
    for ( int dim = 2; dim < dimensionVector->size(); dim++) {
        int dimSize = svkDcmHeader::GetDimensionVectorValue ( dimensionVector, dim ) + 1; 
        numFrames *= dimSize;  
        if (this->GetDebug()) {
            cout << "NUM FRAMES: DIM SIZE " << dim << " => " << dimSize << endl;
        }
    }
    return numFrames; 

}

/*  
 *  Gets the number of cells defined by a svkDcmHeader DimensionVector instance
 *  the DimensionVector input should be initialized so that each index value is the
 *  max value for that dimension. Since indexing starts at 0 this is 1 less than the
 *  dimension size. 
 */
int svkDcmHeader::GetNumberOfCells(svkDcmHeader::DimensionVector* dimensionVector)
{
    int numCells = 1; 
    //the dimension index sequence starts at index 0, so ignore cols and rows
    for ( int dim = 0; dim < dimensionVector->size(); dim++) {
        int dimSize = svkDcmHeader::GetDimensionVectorValue ( dimensionVector, dim ) + 1; 
        numCells *= dimSize;  
    }
    return numCells; 

}


/*!
 *  Get the value of the specified dimension: 
 *  Default is 1 point (or max index = 0)
 */
int svkDcmHeader::GetDimensionVectorValue(svkDcmHeader::DimensionVector* dimensionVector, svkDcmHeader::DimensionIndexLabel dimensionLabel)
{
    int value = 0; 
    for ( int i = 0; i < dimensionVector->size(); i++ ) {
        svkDcmHeader::DimensionIndexLabel dimLabel = (*(*dimensionVector)[i].begin()).first; 
        if ( dimLabel == dimensionLabel ) {
            value = (*(*dimensionVector)[i].begin()).second; 
        }
    }
    return value;
}


/*!
 *  Gets the Dimension label for the specified numeric index
 */
svkDcmHeader::DimensionIndexLabel svkDcmHeader::GetDimensionLabelFromIndex( svkDcmHeader::DimensionVector* dimensionVector, int index )
{
    return (*(*dimensionVector)[index].begin()).first; 

}

/*!
 *  Get the value of the specified dimension.  The index starts at 2 for slice, the inner most 
 *  loop in any DimensionVector.  
 */
int svkDcmHeader::GetDimensionVectorValue(svkDcmHeader::DimensionVector* dimensionVector, int index)
{
    return (*(*dimensionVector)[index].begin()).second; 
}


/*!
 *  Set the value of the specified dimension.  
 */
void svkDcmHeader::SetDimensionVectorValue(svkDcmHeader::DimensionVector* dimensionVector, int index, int value)
{
    (*(*dimensionVector)[index].begin()).second = value; 
}


/*!
 *  Set the value of the specified dimension.  
 */
void svkDcmHeader::SetDimensionVectorValue(svkDcmHeader::DimensionVector* dimensionVector, svkDcmHeader::DimensionIndexLabel indexType, int value)
{
    for ( int i = 0; i < dimensionVector->size(); i++ ) {
        svkDcmHeader::DimensionIndexLabel dimLabel = (*(*dimensionVector)[i].begin()).first; 
        if ( dimLabel == indexType ) {
            (*(*dimensionVector)[i].begin()).second = value; 
        }
    }
}


/*!
 *  Set the value of the specified dimension.  
 */
bool svkDcmHeader::IsDimensionDefined(svkDcmHeader::DimensionVector* dimensionVector, svkDcmHeader::DimensionIndexLabel indexType)
{
    bool isDefined = false; 

    for ( int i = 0; i < dimensionVector->size(); i++ ) {
        svkDcmHeader::DimensionIndexLabel dimLabel = (*(*dimensionVector)[i].begin()).first; 
        if ( dimLabel == indexType ) {
            isDefined = true; 
            break; 
        }
    }
    return isDefined; 
}


/*!
 *  Returns a data structure representing the current data set dimensions and dimension
 *  labels. This includes the 3 spatial dimensions: cols, rows, and   DimensionIndexSequence.  
 *  which includes at least the slice index, but often additional dimensions. 
 *  Returns a vector or maps.  The vector order is from innermost data looping
 *  in the PerFrameFunctionalGroup sequence, to outter most.  Slice is always
 *  present and represents the first index in thsi sequence. 
 *      vector index:  position in dimension index sequence
 *          - map: string key representing the dimensionIndexLabel
 *          - map: int max index value  representing the size-1 of the dimension in the current data set  
 */
svkDcmHeader::DimensionVector  svkDcmHeader::GetDimensionIndexVector()
{
    if ( this->WasModified() ) {
        this->UpdateSpatialParams();
    }

    // Lets make sure the DimensionIndexVector is Initialized
    if( this->dimensionIndexVector.empty() ) {
        this->UpdateDimensionIndexVector();
    }
    return this->dimensionIndexVector;
}


void  svkDcmHeader::UpdateDimensionIndexVector()
{
    svkDcmHeader::DimensionVector dimensionIndexVector; 
    map < DimensionIndexLabel, int> indexRowMap;  

    //Start with cols and rows:     
    indexRowMap.insert( pair<DimensionIndexLabel, int>( svkDcmHeader::COL_INDEX, this->GetIntValue("Columns")-1) );
    dimensionIndexVector.push_back(indexRowMap);                    
    indexRowMap.clear();

    indexRowMap.insert( pair<DimensionIndexLabel, int>( svkDcmHeader::ROW_INDEX, this->GetIntValue("Rows")-1) );
    dimensionIndexVector.push_back(indexRowMap);                    
    indexRowMap.clear();


    //  Now add the items in the dimensionIndexSequence: 
    int numDims = this->GetNumberOfItemsInSequence("DimensionIndexSequence");
    for (int dimensionIndex = 0; dimensionIndex < numDims; dimensionIndex++) {

        string dimensionLabelString = this->GetDimensionIndexLabel(dimensionIndex); 
        svkDcmHeader::DimensionIndexLabel dimensionLabel = this->StringToDimensionIndexLabel( dimensionLabelString); 
        int dimensionMaxIndex = this->GetNumberOfFramesInDimension( dimensionIndex ) - 1; 

        indexRowMap.insert( pair<DimensionIndexLabel, int>( dimensionLabel, dimensionMaxIndex) );
        dimensionIndexVector.push_back(indexRowMap);                    
        indexRowMap.clear();

    }                    

    //initialize slices to at least 1: 
    if ( ! svkDcmHeader::IsDimensionDefined( &dimensionIndexVector, SLICE_INDEX) ) {
        indexRowMap.insert( pair<DimensionIndexLabel, int>( svkDcmHeader::SLICE_INDEX, 0) );
        dimensionIndexVector.push_back(indexRowMap);                    
        indexRowMap.clear();
    }

    if (this->GetDebug()) {
        svkDcmHeader::PrintDimensionIndexVector( &dimensionIndexVector ); 
    }

    this->dimensionIndexVector = dimensionIndexVector; 
}


void svkDcmHeader::PrintDimensionIndexVector( svkDcmHeader::DimensionVector* dimensionVector )
{
    for (int i = 0; i < dimensionVector->size(); i++) {    
        //  Get the value for this index    
        svkDcmHeader::DimensionIndexLabel label = (*(*dimensionVector)[i].begin()).first ; 
        int dimLabel = static_cast<int>( label ); 
        string dimLabelString = svkDcmHeader::DimensionIndexLabelToString( label ); 
        int dimValue = svkDcmHeader::GetDimensionVectorValue( dimensionVector, i); 
        cout << "DIMENSION INDEX LABEL: " << dimLabel << " => " << dimValue << " (" << dimLabelString << ")" << endl;
    }
}


/*!
 *  Adds a dimension to the existing DimensionVector.  Applies the new dimension to the DICOM 
 *  DimensionIndexSequence. 
 *  By default sets the size of the new dimension to 1 (max value = 0) in the DimensionVector. 
 *  This is typically not a spatial dimension so the origin/toplc do not change
 */
void svkDcmHeader::AddDimensionIndex( svkDcmHeader::DimensionVector* dimensionVector, svkDcmHeader::DimensionIndexLabel indexType, int maxIndex)
{

    if ( maxIndex < 0 ) {
        cerr << "ERROR(svkDcmHeader::AddDimensionIndex) dim size must be >= 1 " << endl;
        exit(1);
    }

    //  If the dimension is already defined, the just return; 
    if ( ! svkDcmHeader::IsDimensionDefined( dimensionVector, indexType) ) {
        map < DimensionIndexLabel, int> indexRowMap;  
        indexRowMap.insert( pair<DimensionIndexLabel, int>( indexType, maxIndex) );
        dimensionVector->push_back(indexRowMap);                    
        this->Redimension( dimensionVector );
    }    
}


/*!
 *  Remove a dimension from Dimension Index Sequence, and reset the header to the new dimension. 
 *  This is typically not a spatial dimension so the origin/toplc do not change
 */
void svkDcmHeader::RemoveDimensionIndex( svkDcmHeader::DimensionIndexLabel indexType )
{
    svkDcmHeader::DimensionVector dimensionVector = this->GetDimensionIndexVector(); 
    //  If the dimension is defined, then remove it; 
    if ( svkDcmHeader::IsDimensionDefined( &dimensionVector, indexType) ) {
        string indexString = svkDcmHeader::DimensionIndexLabelToString( indexType ); 

        //  Add 2 for cols and rows ,which are in dimension vector, but not in DICOM
        // dimension index sequence. 
        svkDcmHeader::DimensionVector::iterator itToRemove = 
                    dimensionVector.begin() + this->GetDimensionIndexPosition(indexString) + 2;
        dimensionVector.erase(itToRemove);                    
        this->Redimension( &dimensionVector );
    }    
}


/*!
 *  Resets the Frame content and image position patient based on the new dimensionality 
 *  in the dimensionVector. 
 */
void svkDcmHeader::Redimension(svkDcmHeader::DimensionVector* newDimensionVector)
{

    double dcos[3][3] = {{0}};
    this->GetDataDcos(dcos); 
       
    double pixelSpacing[3];
    this->GetPixelSpacing( pixelSpacing );

    //int newNumVoxels[3];
    //newNumVoxels[0] = svkDcmHeader::GetDimensionVectorValue( newDimensionVector, svkDcmHeader::COL_INDEX) + 1;
    //newNumVoxels[1] = svkDcmHeader::GetDimensionVectorValue( newDimensionVector, svkDcmHeader::ROW_INDEX) + 1;
    //newNumVoxels[2] = svkDcmHeader::GetDimensionVectorValue( newDimensionVector, svkDcmHeader::SLICE_INDEX) + 1;

    //  Now calcuate the new origin based on the reordered dimensionality: 
    int newNumVoxels[3];
    svkDcmHeader::GetSpatialDimensions( newDimensionVector, newNumVoxels );


    double center[3];
    svkDcmHeader::GetCenterFromOrigin(this, center); 
    //cout << "original center: " << center[0] << " " << center[1] << " " << center[2] << endl;

    double newToplcOrigin[3]; 
    svkDcmHeader::GetOriginFromCenter(center, newNumVoxels, pixelSpacing, dcos, newToplcOrigin); 
    //cout << "new voxels: " << newNumVoxels[0] << " " << newNumVoxels[1] << " " << newNumVoxels[2] << endl;
    //cout << "new toplc: " << newToplcOrigin[0] << " " << newToplcOrigin[1] << " " << newToplcOrigin[2] << endl;
       
    //===========================
    //  If pixel measures, etc are in per frame rather than shared func group, 
    //  reinitialize them now in the shared func group.
    //===========================
    bool reinitPixelMeasures            = false; 
    if( this->ElementExists( "PixelMeasuresSequence", "PerFrameFunctionalGroupsSequence" ) ) {
        reinitPixelMeasures = true; 
    }
    bool reinitPlaneOrientationMacro    = false; 
    if( this->ElementExists( "PlaneOrientationSequence", "PerFrameFunctionalGroupsSequence" ) ) {
        reinitPlaneOrientationMacro = true; 
    }
    bool reinitPixelValueTransformation = false; 
    double slope; 
    double intercept; 
    if( this->ElementExists( "PixelValueTransformationSequence", "PerFrameFunctionalGroupsSequence" ) ) {
        reinitPixelValueTransformation = true; 
        //get slope and intercept: 
        slope     = this->GetDoubleSequenceItemElement ( 
            "PixelValueTransformationSequence", 0, "RescaleSlope", "PerFrameFunctionalGroupsSequence" );
        intercept = this->GetDoubleSequenceItemElement ( 
            "PixelValueTransformationSequence", 0, "RescaleIntercept", "PerFrameFunctionalGroupsSequence" );
    }

    this->InitPerFrameFunctionalGroupSequence(newToplcOrigin, pixelSpacing, dcos, newDimensionVector) ; 

    if ( reinitPixelMeasures == true ) {  
        string pixelSpacingString = svkTypeUtils::DoubleToString(pixelSpacing[0]) + "\\" + svkTypeUtils::DoubleToString(pixelSpacing[1]); 
        string sliceThicknessString = svkTypeUtils::DoubleToString(pixelSpacing[2]); 
        this->InitPixelMeasuresMacro( pixelSpacingString, sliceThicknessString ); 
    }
    if ( reinitPlaneOrientationMacro == true ) {  
        
        ostringstream ossDcos;
        for (int i = 0; i < 2; i++) {
            for (int j = 0; j < 3; j++) {
                ossDcos << dcos[i][j];
                if (i * j != 2) {
                    ossDcos<< "\\";
                }
            }
        }
        cout << "OS: " << ossDcos.str() << endl;
        this->InitPlaneOrientationMacro( ossDcos.str() ); 
    }
    if ( reinitPixelValueTransformation == true ) {  
        this->InitPixelValueTransformationMacro(slope, intercept); 
    }

    this->InitMultiFrameDimensionModule( newDimensionVector ); 

    if ( newNumVoxels[0] >= 0 ) {
        this->SetValue("Columns", newNumVoxels[0] ); 
    }
    if ( newNumVoxels[1] >= 0 ) {
        this->SetValue("Rows", newNumVoxels[1] ); 
    }

}


/*!
 * Calls the Redimension method and then sets the origin and pixel spacing.
 */
void svkDcmHeader::Redimension(svkDcmHeader::DimensionVector* dimensionVector, double* newToplcOrigin, double* newPixelSpacing )
{
    double dcos[3][3] = {{0}};
    this->GetDataDcos(dcos);
    this->Redimension(dimensionVector);
    this->InitPerFrameFunctionalGroupSequence(newToplcOrigin, newPixelSpacing, dcos, dimensionVector) ;
}


/*!
 *  resets a dimension size for existing dimension
 */
void svkDcmHeader::SetDimensionIndexSize( svkDcmHeader::DimensionIndexLabel indexType, int maxIndex)
{

    if ( maxIndex < 0 ) {
        cerr << "ERROR(svkDcmHeader::SetDimensionIndex) dim size must be >= 1 (min index 0)" << endl;
        exit(1);
    }
    svkDcmHeader::DimensionVector dimensionVector = this->GetDimensionIndexVector(); 
    svkDcmHeader::SetDimensionVectorValue( &dimensionVector, static_cast<int>(indexType)+2, maxIndex); 
    if (this->GetDebug()) {
        svkDcmHeader::PrintDimensionIndexVector( &dimensionVector );
    }
    this->Redimension( &dimensionVector );

    svkDcmHeader::DimensionVector TdimensionVector = this->GetDimensionIndexVector(); 
}


/*!
 *  Convert between stirng and svkDcmHeader::DimensionIndexLabel 
 *  These are the string representations of the DICOM DimensionIndexLabel attribute. 
 */
svkDcmHeader::DimensionIndexLabel svkDcmHeader::StringToDimensionIndexLabel( string dimensionIndexLabelString )
{

    DimensionIndexLabel indexLabel;     

    //  case insensitive matching:  
    for (size_t i=0; i<dimensionIndexLabelString.length(); ++i) {
        dimensionIndexLabelString[i] = toupper(dimensionIndexLabelString[i]);
    }    

    if ( dimensionIndexLabelString.compare("SLICE") == 0 ) {
        indexLabel = svkDcmHeader::SLICE_INDEX;        
    } else if ( dimensionIndexLabelString.compare("TIME") == 0 ) {
        indexLabel = svkDcmHeader::TIME_INDEX;        
    } else if ( (dimensionIndexLabelString.compare("CHANNEL") == 0 ) || (dimensionIndexLabelString.find("COIL") != string::npos) ) {
        indexLabel = svkDcmHeader::CHANNEL_INDEX;        
    } else if ( dimensionIndexLabelString.compare("EPSI_ACQ") == 0 ) {
        indexLabel = svkDcmHeader::EPSI_ACQ_INDEX;        
    }

    return indexLabel; 

}


/*!
 *  Convert between strng and svkDcmHeader::DimensionIndexLabel 
 *  These are the string representations of the DICOM DimensionIndexLabel attribute. 
 */
string svkDcmHeader::DimensionIndexLabelToString( svkDcmHeader::DimensionIndexLabel label) 
{

    string indexLabelString = ""; 
    int labelIndex = static_cast<int>(label); 

    if ( labelIndex == 0 ) { 
        indexLabelString = "SLICE";
    } else if ( labelIndex == 1 ) {
        indexLabelString = "TIME"; 
    } else if ( labelIndex == 2 ) {
        indexLabelString = "CHANNEL"; 
    } else if ( labelIndex == 3 ) {
        indexLabelString = "EPSI_ACQ"; 
    }

    return indexLabelString; 

}


/*
 *  Returns offset to PixelData field in DICOM file: 
 */
long int svkDcmHeader::GetPixelDataOffset( string fileName ) 
{
    svkDICOMParser* dcmParser = new svkDICOMParser();

    long int pixelDataOffset = dcmParser->GetPixelDataFileOffset(fileName); 

    delete dcmParser; 

    return pixelDataOffset; 

}


/*
 *  Returns a single pixel value at the specified index. 
 *  input:  
 *      offsetToPixelData is the return from GetPixelDataOffset.  It is the
 *          byte offset from the start fo the DICOM file to the start of the PixelData element. 
 *      pixelIndex
 *      fileName    file to read from 
 */
short svkDcmHeader::GetPixelValueAsShort( long int offsetToPixelData, long int pixelIndex, string fileName) 
{

    if ( offsetToPixelData < 0 ) {
        return VTK_SHORT_MIN; 
    }

    ifstream* dcmFS = new ifstream();
    dcmFS->exceptions( ifstream::eofbit | ifstream::failbit | ifstream::badbit );

    void* pixelValue = new short;

    try {

        dcmFS->open( fileName.c_str(), ios::binary );
        if( dcmFS != NULL && dcmFS->good() ) {
            dcmFS->seekg(0, ios::beg);

            int wordSizeBytes = 2;     
            long int offset = offsetToPixelData + (wordSizeBytes * pixelIndex);

            dcmFS->seekg(offset, ios::beg);

            dcmFS->read( static_cast<char*>(pixelValue), 2);

            dcmFS->close();
        } else {
            return VTK_SHORT_MIN;
        }

    } catch (ifstream::failure e) {

        cout << "ERROR: Exception opening/reading file " << endl;
        return VTK_SHORT_MIN;

    }

    return *( static_cast<short*>(pixelValue) );  
}


/*
 *  Calculates the LPS center from the origin(toplc).
 */
void svkDcmHeader::GetCenterFromOrigin( double origin[3], int numVoxels[3], double voxelSpacing[3], double dcos[3][3], double center[3] )
{
    for( int i = 0; i < 3; i++ ) {
        center[i] = origin[i];
        for( int j = 0; j < 3; j++ ) {
            center[i] += dcos[j][i] * voxelSpacing[j] * ( numVoxels[j] / 2.0 - 0.5 );
        }
    }
}


/*
 *  Calculates the LPS center from the origin(toplc).
 */
void svkDcmHeader::GetCenterFromOrigin( svkDcmHeader* hdr, double center[3] )
{

    double dcos[3][3];
    hdr->GetDataDcos( dcos );

    double origin[3];
    hdr->GetOrigin( origin, 0 );

    double voxelSpacing[3];
    hdr->GetPixelSpacing( voxelSpacing );

    int numVoxels[3];
    numVoxels[0] = hdr->GetIntValue( "Columns" );
    numVoxels[1] = hdr->GetIntValue( "Rows" );
    numVoxels[2] = hdr->GetNumberOfSlices();

    //  Get the current center: 
    svkDcmHeader::GetCenterFromOrigin( origin, numVoxels, voxelSpacing, dcos, center);
}


/*
 *  Calculates the LPS origin (toplc) from the center.
 */
void svkDcmHeader::GetOriginFromCenter( double center[3], int numVoxels[3], double voxelSpacing[3], double dcos[3][3], double origin[3] )
{
    for( int i = 0; i < 3; i++ ) {
        origin[i] = center[i];
        for( int j = 0; j < 3; j++ ) {
            origin[i] -= dcos[j][i] * voxelSpacing[j] * ( numVoxels[j] / 2.0 - 0.5 );
        }
    }
}


/*!
 *  Get the spatial dimensions from the DimensionVector.  This is 1 more than the max value in each index: 
 *  If this is a 2x2x1 data set, the max indices would be 1,1,0, but the spatial dimension would return
 *  2,2,1.        
 */
void svkDcmHeader::GetSpatialDimensions(svkDcmHeader::DimensionVector* dimensionVector, int* numVoxels)
{
    for ( int i = 0; i < dimensionVector->size(); i++ ) {
        svkDcmHeader::DimensionIndexLabel dimLabel = (*(*dimensionVector)[i].begin()).first; 
        if ( dimLabel == svkDcmHeader::COL_INDEX) {
            numVoxels[0] = (*(*dimensionVector)[i].begin()).second + 1; 
        } else if ( dimLabel == svkDcmHeader::ROW_INDEX) {
            numVoxels[1] = (*(*dimensionVector)[i].begin()).second + 1; 
        } else if ( dimLabel == svkDcmHeader::SLICE_INDEX) {
            numVoxels[2] = (*(*dimensionVector)[i].begin()).second + 1; 
        }  
    }  
}


/*!
 *  Get the spatial dimensions from the DimensionVector.  This is 1 more than the max value in each index: 
 *  If this is a 2x2x1 data set, the max indices would be 1,1,0, but the spatial dimension would return
 *  2,2,1.        
 */
int svkDcmHeader::GetNumSpatialVoxels(svkDcmHeader::DimensionVector* dimensionVector)
{
    int numVoxels[3]; 
    svkDcmHeader::GetSpatialDimensions(dimensionVector,  numVoxels); 
    return numVoxels[0] * numVoxels[1] * numVoxels[2]; 
}


/*
 *  Given a loop vector (DimensionIndexVector with values for current index, 
 *  return the "spatial" index assuming the data is only 3D: 
 */
int svkDcmHeader::GetSpatialCellIDFromDimensionVectorIndex( svkDcmHeader::DimensionVector* dimensionVector, svkDcmHeader::DimensionVector* loopIndex) 
{
    int cellIndex = 0; 

    for ( int dimension = 0; dimension < 3;  dimension++ ) {

        //  get sum of inner loops relative to current dimension: 
        int innerSize = 1; // default for innermost loop
        for ( int innerDimension = 0; innerDimension < dimension; innerDimension++ ) {
            innerSize *= ( svkDcmHeader::GetDimensionVectorValue(dimensionVector, innerDimension) + 1); 
        }

        cellIndex += innerSize * svkDcmHeader::GetDimensionVectorValue(loopIndex, dimension); 

    }

    return cellIndex; 


}


/*!
 *  Swaps the specified dimension index labels
 */
void svkDcmHeader::SwapDimensionIndexLabels(svkDcmHeader::DimensionVector* dimensionVector, svkDcmHeader::DimensionIndexLabel label1, svkDcmHeader::DimensionIndexLabel label2)
{
    int index1 = -100; 
    int index2 = -100; 
    int dimSize1; 
    int dimSize2; 

    // Booleans to determine if one of the swap dimensions is missing 
    bool label1IsMissing = false;
    bool label2IsMissing = false;

    for ( int i = 0; i < dimensionVector->size(); i++ ) {

        svkDcmHeader::DimensionIndexLabel dimLabel = (*(*dimensionVector)[i].begin()).first;

        if ( dimLabel == label1 ) {
            index1 = i; 
            dimSize1 = svkDcmHeader::GetDimensionVectorValue( dimensionVector, i); 
        }

        if ( dimLabel == label2 ) {
            index2 = i; 
            dimSize2 = svkDcmHeader::GetDimensionVectorValue( dimensionVector, i); 
        }

    }

    // Check for missing labels 
    if ( index1 == -100 && index2 == -100 ) {
        // If both labels are missing we can't swap them.
        cout << "ERROR, can't find dimension labels" << endl;
        exit(1); 
    } else if ( index1 == -100 ) {
        index1 = dimensionVector->size();
        label1IsMissing = true;
    } else if ( index2 == -100 ) {
        index2 = dimensionVector->size();
        label2IsMissing = true;
    }

    map < svkDcmHeader::DimensionIndexLabel, int> targetRow1; 
    map < svkDcmHeader::DimensionIndexLabel, int> targetRow2; 
    targetRow1.insert( pair<svkDcmHeader::DimensionIndexLabel, int>( label1, dimSize2 ) );
    targetRow2.insert( pair<svkDcmHeader::DimensionIndexLabel, int>( label2, dimSize1 ) );

    // If one of the indecies doesn't exist, add it before swapping.
    if( label1IsMissing ) {
        map < svkDcmHeader::DimensionIndexLabel, int> row1;
        row1.insert( pair<svkDcmHeader::DimensionIndexLabel, int>( label1, 1 ) );
        dimensionVector->push_back(row1);
    } else if( label2IsMissing ) {
        map < svkDcmHeader::DimensionIndexLabel, int> row2;
        row2.insert( pair<svkDcmHeader::DimensionIndexLabel, int>( label2, 1 ) );
        dimensionVector->push_back(row2);
    }
    svkDcmHeader::PrintDimensionIndexVector( dimensionVector );
    (*dimensionVector)[index1].swap( targetRow2 ); 
    (*dimensionVector)[index2].swap( targetRow1 ); 
    
    svkDcmHeader::PrintDimensionIndexVector( dimensionVector );

    // If one label is missing then the last dimension should be removed
    if(label1IsMissing || label2IsMissing) {
        dimensionVector->pop_back();
    }
}
                           



