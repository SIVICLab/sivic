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



#include <svkDcmHeader.h>


using namespace svk;


vtkCxxRevisionMacro(svkDcmHeader, "$Rev$");


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
    this->dataSliceOrder = SLICE_ORDER_UNDEFINED;
}


/*!
 *
 */
svkDcmHeader::~svkDcmHeader()
{
    vtkDebugMacro(<<this->GetClassName()<<"~svkDcmHeader");
}


/*!
 *  Converts a string representation of a patient name to a DICOM VR = PN representation
 *  and sets the value in the DICOM header: 
 *      "[lastName[^firstName[^middleName[^namePrefix[^nameSuffix]]]]"
 *
 *      \param patientsName space delimited patient name string 
 */
void svkDcmHeader::SetDcmPatientsName(string patientsName)
{

    size_t delim;

    const string dcmPNDelim("^");

    // There should be no more than 5 elements to the name (4 delims):
    // replace spaces with ^, or add ^ between elements not present
    for (int i = 0; i < 4; i++) {
        if ( (delim = patientsName.find_first_of(' ') ) != string::npos ) {
            patientsName.replace( delim, 1, dcmPNDelim);
        } else {
            patientsName.push_back('^');
        }
    }

    this->SetValue(
        "PatientsName",
        patientsName
    );
}


/*!
 *  Set the format that specifies the word size and representation in the DICOM PixelData.
 */
void svkDcmHeader::SetPixelDataType(DcmPixelDataFormat dataType)
{

    if ( dataType == UNSIGNED_INT_1 ) {
        this->SetValue( "BitsAllocated", 8 );
        this->SetValue( "BitsStored", 8 );
        this->SetValue( "HighBit", 7 );
        this->SetValue( "PixelRepresentation", 0 ); //unsigned
    } else if ( dataType == UNSIGNED_INT_2 ) {
        this->SetValue( "BitsAllocated", 16 );
        this->SetValue( "BitsStored", 16 );
        this->SetValue( "HighBit", 15 );
        this->SetValue( "PixelRepresentation", 0 ); //unsigned
    } else if ( dataType == SIGNED_INT_2 ) {
        this->SetValue( "BitsAllocated", 16 );
        this->SetValue( "BitsStored", 16 );
        this->SetValue( "HighBit", 15 );
        this->SetValue( "PixelRepresentation", 1 ); //unsigned
    } else if ( dataType == SIGNED_FLOAT_4 ) {
        this->SetValue( "BitsAllocated", 32 );
        this->SetValue( "BitsStored", 32 );
        this->SetValue( "HighBit", 31 );
        this->SetValue( "PixelRepresentation", 1 ); //signed
    } else {
        throw runtime_error("Unsupported data type representation.");
    }
}


/*!
 *  Get the format that specifies the word size and representation in the DICOM PixelData.
 */
int svkDcmHeader::GetPixelDataType()
{
    int bitsPerWord = this->GetIntValue( "BitsAllocated" );
    int pixRep = this->GetIntValue( "PixelRepresentation");
    if (bitsPerWord == 8 && pixRep == 0) {
        return UNSIGNED_INT_1;
    } else if (bitsPerWord == 16 && pixRep == 0) {
        return UNSIGNED_INT_2;
    } else if (bitsPerWord == 16 && pixRep == 1) {
        return SIGNED_INT_2;
    } else if (bitsPerWord == 32 && pixRep == 1) {
        return SIGNED_FLOAT_4;
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

        if ( this->ElementExists("ImagePositionPatient") == true ) {

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
 *  Get the spacing of the voxels.  This accounts for any gaps between samples.
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
 * Determine if the data set is coronal/sagital/axial
 */
svkDcmHeader::Orientation svkDcmHeader::GetOrientationType( )
{
    double dcos[3][3];
    Orientation orientation = AXIAL;
    this->GetDataDcos( dcos );
    double wVec[3];
    wVec[0] = dcos[2][0];
    wVec[1] = dcos[2][1];
    wVec[2] = dcos[2][2];

   if( pow( wVec[2], 2) >= pow( wVec[1], 2 ) && pow( wVec[2], 2) >= pow( wVec[0], 2 ) ) {
        orientation = AXIAL;
    } else if( pow( wVec[1], 2) >= pow( wVec[0], 2 ) && pow( wVec[1], 2) >= pow( wVec[2], 2 ) ) {
        orientation = CORONAL;
    } else {
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
}


/*!
 *  
 */
void svkDcmHeader::MakeDerivedDcmHeader(svkDcmHeader* headerCopy, string seriesDescription)
{
    this->CopyDcmHeader(headerCopy);  
    headerCopy->InsertUniqueUID("SeriesInstanceUID");
    headerCopy->InsertUniqueUID("SOPInstanceUID");
    headerCopy->InsertUniqueUID("MediaStorageSOPInstanceUID");
    headerCopy->SetValue("ImageType", "DERIVED\\SECONDARY");
    headerCopy->SetValue("SeriesDescription", seriesDescription);
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
    this->lastUpdateTime = this->GetMTime();
    this->UpdateOrientation();
    this->UpdatePixelSize();
    this->UpdateOrigin0();
    this->UpdatePixelSpacing();
}


/*!
 *
 */
void svkDcmHeader::UpdateOrientation()
{

    istringstream* iss = new istringstream();
    string orientationString;

    //  The 6 elements of the ImageOrientationPatient are the in plance vectors along the
    //  row and column directions:
    int linearIndex = 0;
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 3; j++) {
            orientationString = this->GetStringSequenceItemElement(
                "PlaneOrientationSequence",
                0,
                "ImageOrientationPatient",
                linearIndex,
                "SharedFunctionalGroupsSequence",
                0
            );
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
    string parentSequence("SharedFunctionalGroupsSequence");
    if( !this->ElementExists( "PixelSpacing", parentSequence.c_str()) ) {
        parentSequence = "PerFrameFunctionalGroupsSequence";
    }

    for (int i = 0; i < 2; i++ ) {
        sizeString = this->GetStringSequenceItemElement(
            "PixelMeasuresSequence",
            0,
            "PixelSpacing",
            i,
            parentSequence.c_str(),
            0
        );
        iss->str(sizeString);
        *iss >> pixelSize[i];
        iss->clear();
    }

    if( this->ElementExists( "SliceThickness", parentSequence.c_str()) ) {
        pixelSize[2] = this->GetDoubleSequenceItemElement(
            "PixelMeasuresSequence",
            0,
            "SliceThickness",
            parentSequence.c_str(),
            0
        );
    } else {
        pixelSize[2] = 0;
        
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

        //  Iterate of all 3 positions of the ImagePositionPatient element;
        for (int i = 0; i < 3; i++) {
            originString = this->GetStringSequenceItemElement(
                "PlanePositionSequence",
                0,
                "ImagePositionPatient",
                i, 
                "PerFrameFunctionalGroupsSequence",
                frameNumber
            );
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
 *  Returns the number of time points from the DcmHeader.
 *  Parses FrameContentSequence for the number of 
 *  coils, if specified in the Multi Frame Dimension Module
 */
int svkDcmHeader::GetNumberOfCoils()
{
    int numCoils = 1;
    int numberOfFrames = this->GetIntValue("NumberOfFrames");

    //  Determine which index in the DimensionIndexValues attribute represents
    //  the coil number index.  Should use "DimensionIndexPointer" (to do).
    int coilIndexNumber = this->GetDimensionIndexPosition( "Coil Number" ); 
    numCoils = this->GetNumberOfFramesInDimension( coilIndexNumber ); 
    return numCoils;
}


/*!
 *  Returns the number of time points from the DcmHeader.
 *  Parses FrameContentSequence for the number of 
 *  time points, if specified in the Multi Frame Dimension Module
 */
int svkDcmHeader::GetNumberOfTimePoints()
{
    int numTimePts = 1;
    int numberOfFrames = this->GetIntValue("NumberOfFrames");

    //  Determine which index in the DimensionIndexValues attribute represents
    //  the coil number index.  Should use "DimensionIndexPointer" (to do).
    int timeIndexNumber = GetDimensionIndexPosition( "Time Point" ); 
    numTimePts = GetNumberOfFramesInDimension( timeIndexNumber ); 

    return numTimePts;
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
 *  Gets the number of frames in the specified dimension. 
 */
int svkDcmHeader::GetNumberOfFramesInDimension( int dimensionIndex )
{
    int numberOfFrames = this->GetIntValue("NumberOfFrames");

    int numFramesInDimension; 

    if ( dimensionIndex >= 0 ) {
        set <int> frames;
        for (int i = 0; i < numberOfFrames; i++ ) {

            //get number of coils and divide numberof frames by it to get number of slices
            int value = this->GetIntSequenceItemElement(
                "FrameContentSequence",
                0, //frame number
                "DimensionIndexValues",
                "PerFrameFunctionalGroupsSequence",
                i,
                dimensionIndex  // position of the coil index in the index array. 
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
    int numberOfFrames = this->GetIntValue( "NumberOfFrames" );
    int numberOfCoils = this->GetNumberOfCoils();
    int numberOfTimePts = this->GetNumberOfTimePoints();
    return numberOfFrames/(numberOfCoils * numberOfTimePts ) ;
}


/*!
 *  Get the number of Dimension indices requred 
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
 *  Get the Dimension Indices for a given slice, timePt and Coil
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
 *
 */
void svkDcmHeader::InitPerFrameFunctionalGroupSequence(double toplc[3], double voxelSpacing[3],
                                             double dcos[3][3], int numSlices, int numTimePts, int numCoils)
{

    this->ClearSequence( "PerFrameFunctionalGroupsSequence" );
    this->SetValue( "NumberOfFrames", numSlices * numTimePts * numCoils );  
    this->InitMultiFrameDimensionModule( numSlices, numTimePts, numCoils ); 
    this->InitFrameContentMacro( numSlices, numTimePts, numCoils ); 
    this->InitPlanePositionMacro( toplc, voxelSpacing, dcos, numSlices, numTimePts, numCoils); 
}
 

/*!
 *
 */
void svkDcmHeader::InitMultiFrameDimensionModule( int numSlices, int numTimePts, int numCoils )
{

    this->ClearSequence( "DimensionIndexSequence" );

    int indexCount = 0;

    this->AddSequenceItemElement(
        "DimensionIndexSequence",
        indexCount,
        "DimensionDescriptionLabel",
        "Slice"
    );

    if ( numTimePts > 1 ) {
        indexCount++;
        this->AddSequenceItemElement(
            "DimensionIndexSequence",
            indexCount,
            "DimensionDescriptionLabel",
            "Time Point"
        );
    }

    if ( numCoils > 1 ) {
        indexCount++;
        this->AddSequenceItemElement(
            "DimensionIndexSequence",
            indexCount,
            "DimensionDescriptionLabel",
            "Coil Number"
        );
    }
}


/*!
 *
 */
void svkDcmHeader::InitPlanePositionMacro(double toplc[3], double voxelSpacing[3],
                                             double dcos[3][3], int numSlices, int numTimePts, int numCoils)
{

    int frame = 0;

    for (int coilNum = 0; coilNum < numCoils; coilNum++) {

        float displacement[3];
        float frameLPSPosition[3];

        for (int i = 0; i < numSlices; i++) {

            this->AddSequenceItemElement(
                "PerFrameFunctionalGroupsSequence",
                i,
                "PlanePositionSequence"
            );

            //add displacement along normal vector:
            for (int j = 0; j < 3; j++) {
                displacement[j] = dcos[2][j] * voxelSpacing[2] * i;
            }
            for(int j = 0; j < 3; j++) { //L, P, S
                frameLPSPosition[j] = toplc[j] +  displacement[j] ;
            }

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

            frame++;
        }
    }
}


/*!
 *  Mandatory, Must be a per-frame functional group
 */
void svkDcmHeader::InitFrameContentMacro( int numSlices, int numTimePts, int numCoils )
{

    if ( numSlices < 0 ) {
        numSlices = this->GetNumberOfSlices(); 
    }

    if ( numTimePts < 0 ) {
        numTimePts = this->GetNumberOfTimePoints(); 
    }

    if ( numCoils < 0 ) {
        numCoils = this->GetNumberOfCoils(); 
    }
    
    int numFrameIndices = svkDcmHeader::GetNumberOfDimensionIndices( numTimePts, numCoils ) ;

    unsigned int* indexValues = new unsigned int[numFrameIndices];

    int frame = 0;

    for (int coilNum = 0; coilNum < numCoils; coilNum++) {

        for (int timePt = 0; timePt < numTimePts; timePt++) {

            for (int sliceNum = 0; sliceNum < numSlices; sliceNum++) {

                svkDcmHeader::SetDimensionIndices(
                    indexValues, numFrameIndices, sliceNum, timePt, coilNum, numTimePts, numCoils
                );

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
                    "EMPTY_ELEMENT",
                    "PerFrameFunctionalGroupsSequence",
                    frame
                );

                this->AddSequenceItemElement(
                    "FrameContentSequence",
                    0,
                    "FrameReferenceDateTime",
                    "EMPTY_ELEMENT",
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

                frame++;
            }
        }
    }

    delete[] indexValues;

}

