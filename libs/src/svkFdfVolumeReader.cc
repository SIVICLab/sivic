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


#include <svkFdfVolumeReader.h>
#include </usr/include/vtk/vtkShortArray.h>
#include </usr/include/vtk/vtkUnsignedShortArray.h>
#include </usr/include/vtk/vtkGlobFileNames.h>
#include </usr/include/vtk/vtkSortFileNames.h>
#include </usr/include/vtk/vtkDebugLeaks.h>
#include </usr/include/vtk/vtkByteSwap.h>
#include <svkEnhancedMRIIOD.h>

#include <sys/stat.h>

using namespace svk;


//vtkCxxRevisionMacro(svkFdfVolumeReader, "$Rev$");
vtkStandardNewMacro(svkFdfVolumeReader);


/*!
 *
 */
svkFdfVolumeReader::svkFdfVolumeReader()
{

#if VTK_DEBUG_ON
    this->DebugOn();
    vtkDebugLeaks::ConstructClass("svkFdfVolumeReader");
#endif

    vtkDebugMacro( << this->GetClassName() << "::" << this->GetClassName() << "()" );

    this->pixelData = NULL;
    this->dataArray = NULL; 
    this->fdfFile = NULL;
    this->procparFile = NULL;
    this->fileSize = 0;
    this->ScaleTo16Bit( false, false, false);

    // Set the byte ordering, as little-endian by default.
    this->SetDataByteOrderToLittleEndian(); 
    this->tmpFileNames = NULL; 
}


/*!
 *
 */
svkFdfVolumeReader::~svkFdfVolumeReader()
{

    vtkDebugMacro( << this->GetClassName() << "::~" << this->GetClassName() << "()" );

    if (this->dataArray != NULL) {
        this->dataArray->Delete();
        this->dataArray = NULL; 
    }

    if ( this->fdfFile != NULL )  {
        delete fdfFile; 
        this->fdfFile = NULL; 
    }

    if ( this->procparFile != NULL )  {
        delete procparFile; 
        this->procparFile = NULL; 
    }
}


/*!
 *  Sets options for scaling input floating point data to 16 bit integers.
 */
void svkFdfVolumeReader::ScaleTo16Bit( bool scaleTo16Bit, bool scaleToSignedShort, bool scaleToPositiveRange )
{
    this->scaleTo16Bit = scaleTo16Bit;
    this->scaleToSignedShort = scaleToSignedShort;
    this->scaleToPositiveRange = scaleToPositiveRange;
}






/*!
 *  Check to see if the extension indicates a Varian FDF file.  If so, try 
 *  to open the file for reading.  If that works, then return a success code. 
 *  Return Values: 1 if can read the file, 0 otherwise.
 */
int svkFdfVolumeReader::CanReadFile(const char* fname)
{

    string fileToCheck(fname);

    if( fileToCheck.size() > 4 ) {

        // Also see "vtkImageReader2::GetFileExtensions method" 
        if (  fileToCheck.substr( fileToCheck.size() - 4 ) == ".fdf" ) {
            FILE *fp = fopen(fname, "rb");
            if (fp) {
                fclose(fp);
                vtkDebugMacro(
                    << this->GetClassName() << "::CanReadFile(): It's a Varian FDF File: " << fileToCheck
                );
                return 1;
            }
        } else {
            vtkDebugMacro(
                << this->GetClassName() << "::CanReadFile(): It's NOT a Varian FDF File: " << fileToCheck
            );
            return 0;
        }
    } else {
        vtkDebugMacro(<< this->GetClassName() << "::CanReadFile(): is NOT a valid file: " << fileToCheck);
        return 0;
    }
}


/*!
 *  Reads pixel data from all fdf files. 
 *  For .fdf series, each file contains 1/num_files_in_series worth of pixels. 
 */
void svkFdfVolumeReader::ReadFdfFiles()
{

    vtkDebugMacro( << this->GetClassName() << "::ReadFdfFiles()" );

    for (int fileIndex = 0; fileIndex < this->GetFileNames()->GetNumberOfValues(); fileIndex++) {

        ifstream* volumeDataIn = new ifstream();
        volumeDataIn->exceptions( ifstream::eofbit | ifstream::failbit | ifstream::badbit );

        int numBitsPerByte = 8;
        int pixelWordSize = this->GetHeaderValueAsInt("bits")/numBitsPerByte;

        int numBytesInVol = (this->GetNumPixelsInVol() * pixelWordSize);
        int numFilesInVol = this->GetFileNames()->GetNumberOfValues(); 
        int numBytesInFile = numBytesInVol/numFilesInVol;  
        volumeDataIn->open( this->GetFileNames()->GetValue( fileIndex ), ios::binary );

        /*
        *   Flatten the data volume into one dimension
        */
        if (this->pixelData == NULL) {
            this->pixelData = (void* ) malloc( numBytesInVol); 
        }


        volumeDataIn->seekg(-1 * numBytesInFile, ios::end);
        int offset = (fileIndex * numBytesInFile);
        volumeDataIn->read( (char *)(this->pixelData) + offset, numBytesInFile);
        volumeDataIn->close();
        delete volumeDataIn;
    }

    if ( this->GetHeaderValueAsInt("bigendian") != 0 ) {
        this->SetDataByteOrderToBigEndian();
    }
    
    if ( this->GetSwapBytes() ) {
        if ( this->GetFileType() == svkDcmHeader::UNSIGNED_INT_2) {
            vtkByteSwap::SwapVoidRange(this->pixelData, this->GetNumPixelsInVol(), sizeof(short));
        } else if ( this->GetFileType() == svkDcmHeader::SIGNED_FLOAT_4) {
            vtkByteSwap::SwapVoidRange(this->pixelData, this->GetNumPixelsInVol(), sizeof(float));
        }
    }
  
}


/*!
 *  Side effect of Update() method.  Used to load pixel data and initialize vtkImageData
 *  Called after ExecuteInformation()
 */
void svkFdfVolumeReader::ExecuteDataWithInformation(vtkDataObject* output, vtkInformation* outInfo)
{

    this->FileNames = vtkStringArray::New(); 
    if( this->tmpFileNames != NULL ) {
        this->FileNames->DeepCopy(this->tmpFileNames); 
        this->tmpFileNames->Delete(); 
        this->tmpFileNames = NULL; 
    }
    vtkDebugMacro( << this->GetClassName() << "::ExecuteData()" );

    svkImageData* data = svkImageData::SafeDownCast( this->AllocateOutputData(output, outInfo) );

    if ( this->GetFileNames()->GetNumberOfValues() ) {
        vtkDebugMacro( << this->GetClassName() << " FileName: " << FileName );
        struct stat fs;
        if ( stat(this->GetFileNames()->GetValue(0), &fs) ) {
            vtkErrorMacro("Unable to open file " << string(this->GetFileNames()->GetValue(0)) );
            return;
        }

        this->ReadFdfFiles();

        //  If input is float, convert to short int (16 bit depth):     
        if ( this->GetFileType() == svkDcmHeader::SIGNED_FLOAT_4 && this->scaleTo16Bit ) {
            vtkFloatArray* tmpArray = vtkFloatArray::New();
            tmpArray->SetVoidArray( (void*)(this->pixelData), GetNumPixelsInVol(), 0);
            this->dataArray->Delete();     
            if ( this->scaleToSignedShort ) {
                this->dataArray = vtkShortArray::New();
            } else {
                this->dataArray = vtkUnsignedShortArray::New();
            }
            this->dataArray->SetName("pixels");
            this->MapFloatValuesTo16Bit( tmpArray, this->dataArray );
            tmpArray->Delete();
            this->GetOutput()->GetDcmHeader()->SetPixelDataType( svkDcmHeader::SIGNED_INT_2 );
        }  else {
            this->dataArray->SetVoidArray( (void*)(this->pixelData), GetNumPixelsInVol(), 0);
        }

       
        int vtkDataType;  
        if ( this->GetFileType() == svkDcmHeader::UNSIGNED_INT_1) {
            vtkDataType = VTK_UNSIGNED_CHAR; 
        } else if ( this->GetFileType() == svkDcmHeader::UNSIGNED_INT_2 || this->scaleTo16Bit)  {
            vtkDataType = VTK_UNSIGNED_SHORT; 
        } else if ( this->GetFileType() == svkDcmHeader::SIGNED_FLOAT_4 && ! this->scaleTo16Bit) { 
            vtkDataType = VTK_FLOAT; 
        } else if ( this->GetFileType() == svkDcmHeader::SIGNED_FLOAT_4 && this->scaleTo16Bit) { 
            vtkDataType = VTK_UNSIGNED_SHORT; 
        }
        vtkDataObject::SetPointDataActiveScalarInfo(
            this->GetInformation(),
            vtkDataType,
            this->GetNumberOfScalarComponents()
        );



        data->GetPointData()->SetScalars(this->dataArray);

    }

    /* 
     * We need to make a shallow copy of the output, otherwise we would have it
     * registered twice to the same reader which would cause the reader to never delete.
     */
    double dcos[3][3];
    this->GetOutput()->GetDcmHeader()->GetDataDcos( dcos );
    this->GetOutput()->SetDcos(dcos);

    //  SetNumberOfIncrements is supposed to call this, but only works if the data has already
    //  been allocated. but that requires the number of components to be specified.
    this->GetOutput()->GetIncrements();

    if (this->GetDebug()) {
        cout << "FDF READER HEADER " << *data << endl;
        this->GetOutput()->GetDcmHeader()->PrintDcmHeader();
    }

}


/*!
 *  Side effect of Update() method.  Used to initialize the svkDcmHeader member of 
 *  the target svkImageData object and uses the header to set up the Output Informatin.
 *  Called before ExecuteData()
 */
void svkFdfVolumeReader::ExecuteInformation()
{
    vtkDebugMacro( << this->GetClassName() << "::ExecuteInformation()" );

    if (this->FileName == NULL) {
        return;
    }

    if ( this->FileName ) {
        vtkDebugMacro( << this->GetClassName() << " FileName: " << FileName );

        struct stat fs;
        if ( stat(this->FileName, &fs) ) {
            vtkErrorMacro("Unable to open file " << this->FileName );
            return;
        }

        this->InitDcmHeader();
        this->SetupOutputInformation();
    }

    //  This is a workaround required since the vtkImageAlgo executive 
    //  for the reder resets the Extent[5] value to the number of files
    //  which is not correct for 3D multislice volume files. So store
    //  the files in a temporary array until after ExecuteData has been 
    //  called, then reset the array.  
    this->tmpFileNames = vtkStringArray::New(); 
    this->tmpFileNames->DeepCopy(this->FileNames); 
    this->FileNames->Delete(); 
    this->FileNames = NULL; 
}


/*!
 *  Initializes the svkDcmHeader adapter to a specific IOD type
 *  and initizlizes the svkDcmHeader member of the svkImageData
 *  object.
 */
void svkFdfVolumeReader::InitDcmHeader()
{

    vtkDebugMacro( << this->GetClassName() << "::InitDcmHeader()" );

    svkIOD* iod = svkEnhancedMRIIOD::New();
    iod->SetDcmHeader( this->GetOutput()->GetDcmHeader());
    iod->InitDcmHeader();
    iod->Delete();

    //  Read the fdf header into a map of values used to initialize the
    //  DICOM header. 
    this->ParseFdf(); 

/*
    this->InitPatientModule();
    this->InitGeneralStudyModule();
*/
    this->InitGeneralSeriesModule();
    this->InitGeneralEquipmentModule();
    this->InitImagePixelModule();
    this->InitMultiFrameFunctionalGroupsModule();
/*
    this->InitMultiFrameDimensionModule();
    this->InitAcquisitionContextModule();
*/

    if (this->GetDebug()) {
        this->GetOutput()->GetDcmHeader()->PrintDcmHeader();
    }
}


/*!
 *  Returns the file type enum 
 */
svkDcmHeader::DcmPixelDataFormat svkFdfVolumeReader::GetFileType()
{

    int numBitsPerByte = 8;
    int pixelWordSize = this->GetHeaderValueAsInt("bits")/numBitsPerByte;

    string storage = this->GetHeaderValueAsString("storage");

    svkDcmHeader::DcmPixelDataFormat format = svkDcmHeader::UNDEFINED;
    if ( pixelWordSize == 4 && storage == "float" ) {
        format = svkDcmHeader::SIGNED_FLOAT_4;
    } else {
        throw runtime_error("Unsupported data type (min and max intensity values out of range.");
    }

    return format; 
}


/*!
 *
 */
void svkFdfVolumeReader::InitPatientModule()
{
    this->GetOutput()->GetDcmHeader()->InitPatientModule(
        "", 
        this->GetHeaderValueAsString("studyId"), 
        "", 
        "" 
    );
}


/*!
 *
 */
void svkFdfVolumeReader::InitGeneralStudyModule()
{

    this->GetOutput()->GetDcmHeader()->InitGeneralStudyModule(
        this->GetHeaderValueAsString("studyDate"), 
        "",
        "",
        this->GetHeaderValueAsString("studyID"), 
        "", 
        "" 
    );

}


/*!
 *
 */
void svkFdfVolumeReader::InitGeneralSeriesModule()
{
    this->GetOutput()->GetDcmHeader()->InitGeneralSeriesModule(
        "0", 
        "Varian Image", 
        this->GetDcmPatientPositionString()
    );
}


/*!
 *  DDF is historically the UCSF representation of a GE raw file so
 *  initialize to svkFdfVolumeReader::MFG_STRING.
 */
void svkFdfVolumeReader::InitGeneralEquipmentModule()
{
    // No way to know what type of scanner the images were acquired on. 
}


/*!
 *
 */
void svkFdfVolumeReader::InitImagePixelModule()
{
    this->GetOutput()->GetDcmHeader()->SetValue( "Columns", this->GetHeaderValueAsInt("matrix[]", 0) );
    this->GetOutput()->GetDcmHeader()->SetValue( "Rows", this->GetHeaderValueAsInt("matrix[]", 1) );
    this->GetOutput()->GetDcmHeader()->SetPixelDataType( this->GetFileType() );
}



/*! 
 *  
 */
void svkFdfVolumeReader::InitMultiFrameFunctionalGroupsModule()
{
    this->InitSharedFunctionalGroupMacros();
    this->InitPerFrameFunctionalGroupMacros();
}


/*! 
 *  
 */
void svkFdfVolumeReader::InitMultiFrameDimensionModule()
{
}


/*! 
 *  
 */
void svkFdfVolumeReader::InitAcquisitionContextModule()
{
}


/*! 
 *  
 */
void svkFdfVolumeReader::InitSharedFunctionalGroupMacros()
{
    this->InitPixelMeasuresMacro();
    this->InitPlaneOrientationMacro();
    this->InitMRReceiveCoilMacro();
}


/*! 
 *  
 */
void svkFdfVolumeReader::InitPerFrameFunctionalGroupMacros()
{

    double dcos[3][3];
    this->GetOutput()->GetDcmHeader()->SetSliceOrder( this->dataSliceOrder );
    this->GetOutput()->GetDcmHeader()->GetDataDcos( dcos );

    double pixelSpacing[3]; 
    this->GetPixelSize( pixelSpacing ); 

    double toplc[3]; 
    double sliceSpacing; 
    this->GetTLCAndSliceSpacing(toplc, &sliceSpacing ); 
    pixelSpacing[2] = sliceSpacing; 

    svkDcmHeader::DimensionVector dimensionVector = this->GetOutput()->GetDcmHeader()->GetDimensionIndexVector(); 
    this->numSlices = this->GetHeaderValueAsInt("matrix[]", 2);
    svkDcmHeader::SetDimensionVectorValue(&dimensionVector, svkDcmHeader::SLICE_INDEX, this->numSlices-1);

    this->GetOutput()->GetDcmHeader()->InitPerFrameFunctionalGroupSequence(
                toplc,        
                pixelSpacing,  
                dcos,  
                &dimensionVector
    );
}


/*!
 *  The FDF toplc is the center of the first voxel. 
 *  calculate toplc in user coords, then conver to magnet
 *  frame: 

 *  Convert locations from user frame to magnet frame: 
 *  According to the VNMR User Programming Manual: 
 *  the fdf "orientation field" contains the dcos relating
 *  the user (xyz) to the magnet (XYZ) frame according to: 
 *      X = (dcos00 * x) + (dcos10 * y) + (dcos20 * z)
 *      Y = (dcos01 * x) + (dcos11 * y) + (dcos21 * z)
 *      Z = (dcos02 * x) + (dcos12 * y) + (dcos22 * z)
 *
 *   And conversely:     
 *
 *      x = (dcos00 * X) + (dcos01 * Y) + (dcos02 * Z)
 *      y = (dcos10 * X) + (dcos11 * Y) + (dcos12 * Z)
 *      z = (dcos20 * X) + (dcos21 * Y) + (dcos22 * Z)
 */
void svkFdfVolumeReader::GetTLCAndSliceSpacing(double* toplc, double* sliceSpacing)
{

    /*  
     *  First do all calculations in the user frame (cols, rows, slices)  
     *  Then next convert to Magnet frame.  Finally consider patient orientation
     *  to convert to LPS.  
     */
    double dcos[3][3];
    this->GetOutput()->GetDcmHeader()->SetSliceOrder( this->dataSliceOrder );
    this->GetOutput()->GetDcmHeader()->GetDataDcos( dcos );
    double pixelSpacing[3];
    this->GetOutput()->GetDcmHeader()->GetPixelSize(pixelSpacing); 

    //  Get center coordinate float array from fdfMap and use that to generate 
    //  Displace from that coordinate by 1/2 fov - 1/2voxel to get to the center of the
    //  toplc from which the individual frame locations are calculated

    //  If volumetric 3D, get the center of the TLC voxel in LPS coords: 
    double* volumeTlcLPSFrame = new double[3];  
    if (GetHeaderValueAsInt("rank") == 3) {

        //  Get the volumetric center in magnet frame coords: 
        double volumeCenterUserFrame[3];  
        for (int i = 0; i < 3; i++) {
            volumeCenterUserFrame[i] = this->GetHeaderValueAsFloat("location[]", i); 
        }

        double* volumeTlcUserFrame = new double[3];  
        for (int i = 0; i < 3; i++) {
            volumeTlcUserFrame[i] = volumeCenterUserFrame[i] 
                                 + ( this->GetHeaderValueAsFloat("roi[]", i) - pixelSpacing[i] )/2; 
        }
        this->UserToMagnet(volumeTlcUserFrame, volumeTlcLPSFrame, dcos);  
        delete [] volumeTlcUserFrame;
        
    }

    double displacement[3];
    //  Center of toplc (LPS) pixel in frame:  
    double frameLPSPosition[3];

    /*  
     *  Iterate over slices 
     *  If 3D vol, calculate slice position, otherwise use value encoded 
     *  into slice header
     */
    int slices = this->GetHeaderValueAsInt("matrix[]", 2); 

    //  by default the slice spacing is the same as the pixel size, but 
    //  if there are more than one slices, then compute the value below
    double pixelSize[3];
    this->GetPixelSize(pixelSize); 
    *sliceSpacing = pixelSpacing[2]; 

    for (int i = 0; i < slices; i++) {


        //  Need to displace along normal from tlc of slice: 
        //  add displacement along normal vector to get toplc for each frame:
        for (int j = 0; j < 3; j++) {
            displacement[j] = dcos[2][j] * pixelSpacing[2] * i;
        }

        string imagePositionPatient;

        if (GetHeaderValueAsInt("rank") == 2) {

            //  Location is the center of the image frame in user (acquisition frame). 
            double centerUserFrame[3];  
            for ( int j = 0; j < 3; j++) {
                centerUserFrame[j] = this->GetHeaderValueAsFloat("location[]", i * 3 + j ) ;
            }

            //  Now get the center of the tlc voxel in the magnet frame: 
            double* tlcUserFrame = new double[3];  
            for (int j = 0; j < 2; j++) {
                tlcUserFrame[j] = centerUserFrame[j] 
                    - ( this->GetHeaderValueAsFloat("roi[]", j) - pixelSpacing[j] )/2; 
            }
            tlcUserFrame[2] = centerUserFrame[2]; 

            //  and convert to LPS (magnet) frame: 
            this->UserToMagnet(tlcUserFrame, frameLPSPosition, dcos);  
                        
            delete [] tlcUserFrame; 

        } else {

            for(int j = 0; j < 3; j++) { //L, P, S
                frameLPSPosition[j] = volumeTlcLPSFrame[j] +  displacement[j] ;
            }

        }

        if ( i == 0 ) {
            for (int index = 0; index < 3; index++) {
                toplc[index] = frameLPSPosition[index]; 
            }
        }
    }

    //  If there is one slice the spacing is the same as the pixel size
    if ( slices > 1) {

        *sliceSpacing = 0;
        for (int i = 0; i < 3; i++ ) {
            *sliceSpacing += pow(toplc[i] - frameLPSPosition[i], 2);
        }
        *sliceSpacing = pow(*sliceSpacing, .5)/(slices-1);

    }

    delete[] volumeTlcLPSFrame;

}




/*!
 * Compute the pixel spacing in the slice direction to account for gaps
 */
void svkFdfVolumeReader::GetPixelSize( double* pixelSize)
{
    float fov; 
    float numPixels; 
    for (int i = 0; i < 3; i++) {
        fov        = fabs( GetHeaderValueAsFloat("roi[]", i) );
        numPixels  = GetHeaderValueAsFloat("matrix[]", i);
        pixelSize[i] = fov/numPixels; 
    }
}


/*!
 *  Pixel Spacing:
 */
void svkFdfVolumeReader::InitPixelMeasuresMacro()
{

    double pixelSize[3]; 
    this->GetPixelSize( pixelSize );

    //  These are in the user frame: (cols, rows, slice) 
    string pixelSizeString[3]; 
    for (int i = 0; i < 3; i++) {
        ostringstream oss;
        oss << pixelSize[i];
        pixelSizeString[i].assign( oss.str() );
    }

    this->GetOutput()->GetDcmHeader()->InitPixelMeasuresMacro(
        pixelSizeString[0] + "\\" + pixelSizeString[1], 
        pixelSizeString[2]
    );
}


/*!
 *  
 */
void svkFdfVolumeReader::InitPlaneOrientationMacro()
{

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "SharedFunctionalGroupsSequence",
        0,
        "PlaneOrientationSequence"
    );

    string orientationString;
 
    //  varian user frame appears to use LAI coords (rather than LPS), 
    //  so flip the 2nd and 3rd idndex.  Is there an "entry" indicator?
    //  HF vs FF should flip both RL and SI.  Supine/Prone should flip 
    //  RL and AP.  
    //  FF + Prone should flip AP + SI relative to HF + Supine 
    float dcos[9];
    dcos[0] =      GetHeaderValueAsFloat("orientation[]", 0);
    dcos[1] = -1 * GetHeaderValueAsFloat("orientation[]", 1);
    dcos[2] = -1 * GetHeaderValueAsFloat("orientation[]", 2);
    dcos[3] =      GetHeaderValueAsFloat("orientation[]", 3);
    dcos[4] = -1 * GetHeaderValueAsFloat("orientation[]", 4);
    dcos[5] = -1 * GetHeaderValueAsFloat("orientation[]", 5);
    dcos[6] =      GetHeaderValueAsFloat("orientation[]", 6);
    dcos[7] = -1 * GetHeaderValueAsFloat("orientation[]", 7);
    dcos[8] = -1 * GetHeaderValueAsFloat("orientation[]", 8);

    //  If feet first, swap LR, SI
    string position1 = GetHeaderValueAsString("position1", 0);

    //  I believe that for the Varian, the Magnet Z axis points along the inferior 
    //  and y vector points along posterior so, i.e. AS positive.
    //  therefore, dcos6,7,8 =  0,0,1 is actually 00-1
    dcos[6] *= 1; 
    dcos[7] *=-1; 
    dcos[8] *=-1; 

    if( position1.find("feet first") != string::npos ) {
        //  swap L
        dcos[0] *=-1; 
        dcos[3] *=-1; 
        dcos[6] *=-1; 

        //  swap S
        dcos[2] *=-1; 
        dcos[5] *=-1; 
        dcos[8] *=-1; 
    }

    for (int i = 0; i < 6; i++) {
        ostringstream dcosOSS;
        dcosOSS << dcos[i];
        orientationString.append( dcosOSS.str() );
        if (i < 5) {
            orientationString.append( "\\");
        }
    }

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "PlaneOrientationSequence",
        0,
        "ImageOrientationPatient",
        orientationString,
        "SharedFunctionalGroupsSequence",
        0
    );

    //  Determine whether the data is ordered with or against the slice normal direction.
    double normal[3];
    this->GetOutput()->GetDcmHeader()->GetNormalVector(normal);

    double dcosSliceOrder[3];
    dcosSliceOrder[0] = dcos[6];
    dcosSliceOrder[1] = dcos[7];
    dcosSliceOrder[2] = dcos[8];

    //  Use the scalar product to determine whether the data in the .fdf
    //  file is ordered along the slice normal or antiparalle to it.  Note that
    //  I believe that for the Varian, the Magnet Z axis points along the inferior 
    //  therefore, dcos6,7,8 =  0,0,1 is actually 00-1
    vtkMath* math = vtkMath::New();
    if (math->Dot(normal, dcosSliceOrder) > 0 ) {
        this->dataSliceOrder = svkDcmHeader::INCREMENT_ALONG_POS_NORMAL;
    } else {
        this->dataSliceOrder = svkDcmHeader::INCREMENT_ALONG_NEG_NORMAL;
    }
    math->Delete(); 

}


/*!
 *  Receive Coil:
 */
void svkFdfVolumeReader::InitMRReceiveCoilMacro()
{

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "SharedFunctionalGroupsSequence",
        0,
        "MRReceiveCoilSequence"
    );

    this->GetOutput()->GetDcmHeader()->AddSequenceItemElement(
        "MRReceiveCoilSequence",
        0,
        "ReceiveCoilName",
        "Varian Coil",
        "SharedFunctionalGroupsSequence",
        0
    );

}



/*! 
 *  Use the FDF patient position string to set the DCM_PatientPosition data element.
 */
string svkFdfVolumeReader::GetDcmPatientPositionString()
{
    string dcmPatientPosition;

    string position1 = GetHeaderValueAsString("position1", 0);
    if( position1.find("head first") != string::npos ) {
        dcmPatientPosition.assign("HF");
    } else if( position1.find("feet first") != string::npos ) {
        dcmPatientPosition.assign("FF");
    } else {
        dcmPatientPosition.assign("UNKNOWN");
    }

    string position2 = GetHeaderValueAsString("position2", 0);
    if( position2.find("supine") != string::npos ) {
        dcmPatientPosition += "S";
    } else if( position2.find("prone") != string::npos ) {
        dcmPatientPosition += "P";
    } else if( position2.find("decubitus left") != string::npos ) {
        dcmPatientPosition += "DL";
    } else if( position2.find("decubitus right") != string::npos ) {
        dcmPatientPosition += "DR";
    } else {
        dcmPatientPosition += "UNKNOWN";
    }

    return dcmPatientPosition; 
}


/*
 *  Read FDF header fields into a string STL map for use during initialization 
 *  of DICOM header by Init*Module methods. 
 *  The fdf header consists of a list of "=" delimited key/value pairs. 
 *  If a procpar file is present in the directory, parse that as well. 
 */
void svkFdfVolumeReader::ParseFdf()
{

    string fdfFileName( this->GetFileName() );  
    string fdfFileExtension( this->GetFileExtension( this->GetFileName() ) );  
    string fdfFilePath( this->GetFilePath( this->GetFileName() ) );  

    vtkGlobFileNames* globFileNames = vtkGlobFileNames::New();
    globFileNames->AddFileNames( string( fdfFilePath + "/*." + fdfFileExtension).c_str() );

    vtkSortFileNames* sortFileNames = vtkSortFileNames::New();
    sortFileNames->GroupingOn(); 
    sortFileNames->SetInputFileNames( globFileNames->GetFileNames() );
    sortFileNames->NumericSortOn();
    sortFileNames->Update();

    //  If globed file names are not similar, use only the specified file
    if (sortFileNames->GetNumberOfGroups() > 1 ) {

        vtkWarningWithObjectMacro(this, "Found Multiple fdf file groups, using only specified file ");   

        vtkStringArray* fileNames = vtkStringArray::New(); 
        fileNames->SetNumberOfValues(1);  
        fileNames->SetValue(0, this->GetFileName() ); 
        sortFileNames->SetInputFileNames( fileNames ); 
        fileNames->Delete(); 

    } 

    this->SetFileNames( sortFileNames->GetFileNames() );
    vtkStringArray* fileNames =  sortFileNames->GetFileNames();
    for (int i = 0; i < fileNames->GetNumberOfValues(); i++) {
        cout << "FN: " << fileNames->GetValue(i) << endl; 
    }

    try { 

        /*  Read in the FDF Header:
         *  for image 1 read everything.  
         *  for subsequent images in the series get "slice_no" and "location[]" elements
         *  and append these to the existing map value in the order read.  Also, add a 
         *  file name element to map also in this order. 
         */
        this->fdfFile = new ifstream();
        this->fdfFile->exceptions( ifstream::eofbit | ifstream::failbit | ifstream::badbit );

        for (int fileIndex = 0; fileIndex < this->GetFileNames()->GetNumberOfValues(); fileIndex++) {

            string currentFdfFileName( this->GetFileNames()->GetValue( fileIndex ) ); 

            this->fdfFile->open( currentFdfFileName.c_str(), ios::binary );
            if ( ! this->fdfFile->is_open() ) {
                throw runtime_error( "Could not open fdf file: " + currentFdfFileName );
            } 

            this->ParseAndSetStringElements("FileName", currentFdfFileName);

            // determine how big the data buffer is (num pts * word size).  
            // header key-value pairs use total_bytes_in_file - sizeof_data_buffer
            // read key-value pairs from the top until start of data buffer. 
            this->fileSize = this->GetFileSize( this->fdfFile );

            vtkStringArray* keysToFind = vtkStringArray::New(); 
            SetKeysToSearch(keysToFind, fileIndex);

            while (! this->fdfFile->eof() ) {
                if ( this->GetFdfKeyValuePair(keysToFind) != 0 ) {
                    break; 
                }
            }

            keysToFind->Delete(); 

            this->fdfFile->close();
        }

        if (GetHeaderValueAsInt("rank") == 2) {
            this->AddDimensionTo2DData();
        }

        //  Convert fdf spatial params from cm to mm: 
        this->ConvertCmToMm(); 

        if (this->GetDebug()) {
            this->PrintKeyValuePairs(); 
        }

        this->ParseProcpar(fdfFilePath);

        if (this->GetDebug()) {
            this->PrintProcparKeyValuePairs();
        }

    } catch (const exception& e) {
        cerr << "ERROR opening or reading Varian fdf file (" << fdfFileName << "): " << e.what() << endl;
    }

    globFileNames->Delete();
    sortFileNames->Delete();
}


/*! 
 *  Utility function to read a single line from the fdf file and return 
 *  set the delimited key/value pair into the stl map.  
 *  Returns -1 if reading isn't successful. 
 */
int svkFdfVolumeReader::GetFdfKeyValuePair( vtkStringArray* keySet )    
{

    int status = 0; 

    istringstream* iss = new istringstream();

    string keyString;
    string valueString;

    try {

        this->ReadLine(this->fdfFile, iss); 

        size_t  position; 
        string  tmp; 
        string  dataType; 
        long    headerSize;

        int dataBufferSize = this->GetDataBufferSize();

        //  Read only to the start of the pixel buffer, 
        //  i.e. no more than the header size:     
        headerSize = this->fileSize - dataBufferSize; 
        if ( this->fdfFile->tellg() < headerSize - 1 ) {

            //  find first white space position before "key" string: 
            position = iss->str().find_first_of(' ');
            if (position != string::npos) {
                tmp.assign( iss->str().substr(position) );
                dataType.assign( iss->str().substr(0, position) ) ; 
            } 
    
            //  If necessary, remove pointer indicator: 
            position = tmp.find_first_of('*');
            if (position != string::npos) {
                tmp.assign( tmp.substr(position + 1) );
            } 
    
            //  Extract key and value strings:
            position = tmp.find_first_of('=');
            if (position != string::npos) {
                keyString.assign( tmp.substr(0, position - 1) );
                keyString = StripWhite(keyString); 
                // Check for key match if doing a limited search: 
                int parseValue = 1;
                if (keySet != NULL) { 
                    if (keySet->GetNumberOfValues() > 0) { 
                        parseValue = 0; 
                        for (int i = 0; i < keySet->GetNumberOfValues(); i++) { 
                            if ( keySet->GetValue(i) == keyString ) {
                                parseValue = 1; 
                            } 
                        } 
                    } 
                }
                if ( !parseValue )  {
                    delete iss; 
                    return status; 
                }   

                valueString.assign( tmp.substr(position + 2) );
                // Remove terminating ; 
                position = valueString.find_first_of(';');
                valueString.assign( valueString.substr(0, position) );

                // Remove string quotes
                this->RemoveStringQuotes( &valueString );
                while ( ( position = valueString.find('"') ) != string::npos) {
                    valueString.erase( position, 1 );
                }
    
                //  Parse elements into vector: remove matrix brackets 
                //  and assign elements to vector: 
                position = valueString.find_first_of('{');
                if (position != string::npos) {

                    valueString.assign( valueString.substr(position + 1) );
                    position = valueString.find_first_of('}');

                    if (position != string::npos) {
                        valueString.assign( valueString.substr(0, position) );
                    } 
                } 

                this->ParseAndSetStringElements(keyString, valueString);
            } 

        } else { 
            this->fdfFile->seekg(0, ios::end);     
        }
    } catch (const exception& e) {
        if (this->GetDebug()) {
            cout <<  "ERROR reading line: " << e.what() << endl;
        }
        status = -1; 
    }

    delete iss; 
    return status; 
}


/*!
 *  Attempts to determine the pixel data buffer size from the currently 
 *  available header information.  If the size can be determined return that, 
 *  otherwise return 0. 
 */
int svkFdfVolumeReader::GetDataBufferSize()
{
    int bufferSize = 0; 
    map<string, string>::iterator it;

    if (fdfMap.find("bits") != fdfMap.end() && 
        fdfMap.find("rank") != fdfMap.end() && 
        fdfMap.find("matrix[]") != fdfMap.end() ) {

        int numDims = this->GetHeaderValueAsInt("rank");

        int numBitsPerByte = 8;
        int pixelWordSize = this->GetHeaderValueAsInt("bits")/numBitsPerByte;

        bufferSize = pixelWordSize; 
        for (int i = 0; i < numDims; i++) { 
            bufferSize *= this->GetHeaderValueAsInt("matrix[]", i);
        }
    }

    return bufferSize;
}


/*!
 *  Push key value pairs into the map's value vector: 
 *  mapFor values that are comma separated lists, put each element into the value 
 *  vector. 
 */
void svkFdfVolumeReader::ParseAndSetStringElements(string key, 
    string valueArrayString) 
{
    size_t pos;
    istringstream* iss = new istringstream();
    string tmpString;     

    while ( (pos = valueArrayString.find_first_of(',')) != string::npos) {  

        iss->str( valueArrayString.substr(0, pos) );
        *iss >> tmpString;
        fdfMap[key].push_back(tmpString); 
        iss->clear();

        valueArrayString.assign( valueArrayString.substr(pos + 1) ); 
    }

    fdfMap[key].push_back(valueArrayString); 
    delete iss; 
}


/*!
 *
 */
string svkFdfVolumeReader::GetStringFromFloat(float floatValue) 
{
    ostringstream tmpOss;
    tmpOss << floatValue; 
    return tmpOss.str(); 
}


/*!
 *
 */
int svkFdfVolumeReader::GetHeaderValueAsInt(string keyString, int valueIndex) 
{
    
    istringstream* iss = new istringstream();
    int value;

    iss->str( (fdfMap[keyString])[valueIndex]);
    *iss >> value;

    delete iss; 

    return value; 
}


/*!
 *
 */
float svkFdfVolumeReader::GetHeaderValueAsFloat(string keyString, int valueIndex) 
{
    
    istringstream* iss = new istringstream();
    float value;

    iss->str( (fdfMap[keyString])[valueIndex]);
    *iss >> value;

    delete iss; 

    return value; 
}


/*!
 *  Checks to see if the specified header key is defined. 
 */
bool svkFdfVolumeReader::IsKeyInHeader( string keyString ) 
{

    if ( (fdfMap.find( keyString )) != fdfMap.end() ) {
        return true; 
    } else {
        return false; 
    }
}


/*!
 *
 */
string svkFdfVolumeReader::GetHeaderValueAsString(string keyString, int valueIndex) 
{
    return (fdfMap[keyString])[valueIndex];
}


/*!
 *  If this is a 2D header, add explicit 3rd dimension with numSlices , 
 *  and modify slice roi and span 3rd dimension too: 
 */
void svkFdfVolumeReader::AddDimensionTo2DData() 
{

    ostringstream numSlicesOss;
    numSlicesOss << this->GetFileNames()->GetNumberOfValues();
    fdfMap["matrix[]"].push_back( numSlicesOss.str() );     

    //  Get Min and Max location value in 3rd dimension: 
    float sliceMin = this->GetHeaderValueAsFloat("location[]", 2) ; 
    float sliceMax = sliceMin; 
       
    float val;  
    int numSlices = this->GetHeaderValueAsInt("matrix[]", 2);
    for (int i = 0; i < numSlices; i++) {
        val = this->GetHeaderValueAsFloat("location[]", (i * 3) + 2 ) ;
        if (val > sliceMax ) {
            sliceMax = val; 
        }    
        if (val < sliceMin ) {
            sliceMin = val; 
        }    
    }

    float sliceThickness = (sliceMax - sliceMin)/(numSlices - 1); 
    float sliceFOV = numSlices * sliceThickness; 
    ostringstream sliceFOVoss;
    sliceFOVoss << sliceFOV; 

    fdfMap["span[]"].push_back( GetHeaderValueAsString("roi[]", 2) ) ; 

}


/*!
 *  Convert FDF spatial values from cm to mm roi, span, locations gap:  
 */
void svkFdfVolumeReader::ConvertCmToMm() 
{
    float cmToMm = 10.; 
    float tmp; 

    //  ROI
    for (int i = 0; i < 3; i++ ) {
        tmp = cmToMm * this->GetHeaderValueAsFloat("roi[]", i); 
        (fdfMap["roi[]"])[i] = this->GetStringFromFloat( tmp ); 
    }

    //  SPAN 
    for (int i = 0; i < 3; i++ ) {
        tmp = cmToMm * this->GetHeaderValueAsFloat("span[]", i); 
        (fdfMap["span[]"])[i] = this->GetStringFromFloat( tmp ); 
    }

    //  LOCATIONS
    int numberOfLocationCoords = (fdfMap["location[]"]).size(); 
    for (int i = 0; i < numberOfLocationCoords; i++ ) {
        tmp = cmToMm * this->GetHeaderValueAsFloat("location[]", i); 
        (fdfMap["location[]"])[i] = this->GetStringFromFloat( tmp ); 
    }

    //  GAP 
    if ( this->IsKeyInHeader( "gap" ) ) {
        tmp = cmToMm * this->GetHeaderValueAsFloat("gap", 0); 
        (fdfMap["gap"])[0] = this->GetStringFromFloat( tmp ); 
    } else {
        fdfMap["gap"].push_back("0");
    }
}


/*!
 *  Prints the key value pairs parsed from the header. 
 */
void svkFdfVolumeReader::PrintKeyValuePairs()
{

    //  Print out key value pairs parsed from header:
    map< string, vector<string> >::iterator mapIter;
    for ( mapIter = fdfMap.begin(); mapIter != fdfMap.end(); ++mapIter ) {
     
        cout << this->GetClassName() << " " << mapIter->first << " = ";

        vector<string>::iterator it;
        for ( it = fdfMap[mapIter->first].begin() ; it < fdfMap[mapIter->first].end(); it++ ) {
            cout << " " << *it ;
        }
        cout << endl;
    }
}


/*!
 *  Performs a linear mapping of floating point image values to 16 bit integer dynamic range, either
 *  signed or unsigned and using the full or positive only dynamic range. 
 */
void svkFdfVolumeReader::MapFloatValuesTo16Bit(vtkFloatArray* fltArray, vtkDataArray* dataArray)
{

    //  Get the input range for scaling: 
    double inputRange[2];
    fltArray->GetRange(inputRange);
    double deltaRangeIn = inputRange[1] - inputRange[0]; 

    //  Map to full dynamic range of target type:
    //vtkShortArray* usArray = vtkShortArray::New();
    //int maxShort =  static_cast<int>( usArray->GetDataTypeMax() );
    //int minShort =  static_cast<int>( usArray->GetDataTypeMin() );
    int maxShort =  static_cast<int>( dataArray->GetDataTypeMax() );
    int minShort =  static_cast<int>( dataArray->GetDataTypeMin() );
    if ( this->scaleToPositiveRange ) {
        minShort =  0;  // for now map to short, but only use positive dynamic range:
    }
    double deltaRangeOut = maxShort - minShort;  
    //usArray->Delete(); 
    
    static_cast<vtkShortArray*>( dataArray)->SetNumberOfValues( fltArray->GetNumberOfTuples() ); 


    //  apply linear mapping from float range to short range using positive 
    //  values only. 
    //  maxShort = inputRangeMax * m + b , minShort = inputRangeMin * m + b

    double scaledPixelValue; 

    for (int i = 0; i < fltArray->GetNumberOfTuples(); i++) {

        scaledPixelValue =  (deltaRangeOut/deltaRangeIn) * fltArray->GetValue(i) 
                            +  minShort - ( deltaRangeOut/deltaRangeIn ) * inputRange[0]; 

        if ( this->scaleToSignedShort ) {
            static_cast<vtkShortArray*>(dataArray)->SetValue( 
                i, 
                static_cast<short> ( scaledPixelValue )
            ); 
        } else {
            static_cast<vtkShortArray*>(dataArray)->SetValue( 
                i, 
                static_cast<unsigned short> ( scaledPixelValue )
            ); 
        }
    } 

}


/*!
 *  Sets the keys to search for in the file header.  The first file all values are parsed, but
 *  only certain key values are parsed for subsequent files. 
 */
void svkFdfVolumeReader::SetKeysToSearch(vtkStringArray* fltArray, int fileIndex)
{
    if (fileIndex > 0) {  
        fltArray->InsertNextValue("location[]"); 
        fltArray->InsertNextValue("slice_no"); 
        fltArray->InsertNextValue("display_order"); 
    }
}


/*!
 *
 */
int svkFdfVolumeReader::FillOutputPortInformation( int vtkNotUsed(port), vtkInformation* info )
{
    info->Set(vtkDataObject::DATA_TYPE_NAME(), "svkMriImageData");
    return 1;
}

