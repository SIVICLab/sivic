/*
 *  Copyright © 2009 The Regents of the University of California.
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

    int numSlices = this->GetIntValue( "NumberOfFrames" ) / this->GetNumberOfCoils(); 
    if (numSlices >= 2) {

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
 *
 */
int svkDcmHeader::GetNumberOfCoils()
{
    int numCoils = 1;
    int numberOfFrames = this->GetIntValue("NumberOfFrames");
    int numDims = this->GetNumberOfItemsInSequence("DimensionIndexSequence");

    if (numDims > 1) {
        set <int> coils;
        for (int i = 0; i < numberOfFrames; i++ ) {

            //get number of coils and divide numberof frames by it to get number of slices
            int value = this->GetIntSequenceItemElement(
                "FrameContentSequence",
                0, //frame number
                "DimensionIndexValues",
                "PerFrameFunctionalGroupsSequence",
                i,
                1
            );
            coils.insert(value);
        }
        numCoils = coils.size();
    }
    return numCoils;
}


/*!
 *
 */
int svkDcmHeader::GetNumberOfSlices()
{
    int numberOfFrames = this->GetIntValue( "NumberOfFrames" );
    int numberOfChannels = this->GetNumberOfCoils();
    return numberOfFrames/numberOfChannels;
}
