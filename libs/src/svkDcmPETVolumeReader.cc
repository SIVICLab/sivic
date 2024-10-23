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


#include <svkDcmPETVolumeReader.h>
#include <svkMriImageData.h>
#include <svkTypeUtils.h>
#include </usr/include/vtk/vtkType.h>
#include </usr/include/vtk/vtkDebugLeaks.h>
#include </usr/include/vtk/vtkStringArray.h>
#include </usr/include/vtk/vtkInformation.h>
#include </usr/include/vtk/vtkMath.h>
#include <svkIOD.h>
#include <svkEnhancedMRIIOD.h>
#include <vector>
#include <utility>
#include <sstream>


using namespace svk;


//vtkCxxRevisionMacro(svkDcmPETVolumeReader, "$Rev$");
vtkStandardNewMacro(svkDcmPETVolumeReader);


/*!
 *
 */
svkDcmPETVolumeReader::svkDcmPETVolumeReader()
{
#if VTK_DEBUG_ON
    this->DebugOn();
    vtkDebugLeaks::ConstructClass("svkDcmPETVolumeReader");
#endif
    vtkDebugMacro(<<this->GetClassName() << "::" << this->GetClassName() << "()");
}


/*!
 *
 */
svkDcmPETVolumeReader::~svkDcmPETVolumeReader()
{
    vtkDebugMacro(<<this->GetClassName() << "::~" << this->GetClassName() << "()");
}


/*!
 *  Mandator, must be overridden to get the factory to check for proper
 *  type of vtkImageReader2 to return. 
 */
int svkDcmPETVolumeReader::CanReadFile(const char* fname)
{

    string fileToCheck(fname);

    bool isDcmPET = false; 

    if ( svkDcmHeader::IsFileDICOM( fname ) ) {

        svkImageData* tmp = svkMriImageData::New(); 

        tmp->GetDcmHeader()->ReadDcmFileHeaderOnly( fname );
        string SOPClassUID = tmp->GetDcmHeader()->GetStringValue( "SOPClassUID" ) ;

        //verify that this isn't a proprietary use of DICOM MR ImageStorage: 
        if ( this->ContainsProprietaryContent( tmp ) == svkDcmVolumeReader::DICOM_STD_SOP ) {
                    
            // Check for MR Image Storage (and for now CTImageStorage and PETImageStorage too )
            if ( SOPClassUID == "1.2.840.10008.5.1.4.1.1.128" ) {
                this->SetFileName(fname);
                isDcmPET = true; 
            }
        }
        tmp->Delete();
    }

    if ( isDcmPET ) {
        vtkDebugMacro(<<this->GetClassName() << "::CanReadFile(): It's a DICOM PET File: " <<  fileToCheck );
        return 1;
    }

    vtkDebugMacro(<<this->GetClassName() << "::CanReadFile() It's Not a DICOM PET file " << fileToCheck );

    return 0;
}


/*!
 *  Read the pixels and scale to PET SUV (bw scaled).
 *  Use vendor agnostic approach from QIBA website:
 *  http://qibawiki.rsna.org/index.php/Standardized_Uptake_Value_(SUV)
 *  http://qibawiki.rsna.org/images/6/60/SUV_vendorneutral_pseudocode_happypathonly_20121015_DAC.doc
 *      (0054,1001) CS [BQML]                                   #   4, 1 Units
 *      (0010,1030) DS [90.72]                                  #   6, 1 PatientWeight
 *      (0010,1020) DS [1.88]                                   #   4, 1 PatientSize
 *      (0010,0040) CS [M]                                      #   2, 1 PatientSex
 *      (0018,1074) DS [     373700000]                         #  14, 1 RadionuclideTotalDose
 *      (0018,1072) TM [120300.00]                              #  10, 1 RadiopharmaceuticalStartTime
 *      (0018,1075) DS [6586.2001953125]                        #  16, 1 RadionuclideHalfLife
 *      (0008,0021) DA [20161114]                               #   8, 1 SeriesDate
 *      (0008,0031) TM [131943]                                 #   6, 1 SeriesTime
 *
 *  SUVbw = pixel * bw/actual_activity ;
 */
void svkDcmPETVolumeReader::LoadData( svkImageData* data )
{

    int rows               = this->GetOutput()->GetDcmHeader()->GetIntValue( "Rows" );
    int columns            = this->GetOutput()->GetDcmHeader()->GetIntValue( "Columns" );
    int numSlices          = this->GetFileNames()->GetNumberOfValues() / this->numVolumes;
    int numPixelsInSlice   = rows * columns;
    int dataWordSize       = 2;

    int numBytesPerVol     = numPixelsInSlice * numSlices * dataWordSize;
    string voiLUTFunction  = this->GetOutput()->GetDcmHeader()->GetStringValue("VOILUTFunction" );


    /*
     *  Iterate over slices (frames) and copy ImagePositions
     */
    //  need to create an array to store the RescaleSlope for each slice since it differes and is required for SUV mapping. 
    vector < float > rescaleSlopeVector; 
    for (int vol = 0; vol < this->numVolumes; vol++) { 

        void* imageData = (void* ) malloc( numBytesPerVol );
        vtkDataArray* array = NULL;
        array = vtkFloatArray::New();

        ostringstream volNumber;
        volNumber << vol;
        string arrayNameString("pixels");
        arrayNameString.append(volNumber.str());
        array->SetName( arrayNameString.c_str() );

        for (int slice = 0; slice < numSlices; slice++ ) {
            svkImageData* tmpImage = svkMriImageData::New(); 
            tmpImage->GetDcmHeader()->ReadDcmFile( this->GetFileNames()->GetValue( vol + slice * this->numVolumes ) ); 
            tmpImage->GetDcmHeader()->GetShortValue( "PixelData", ((short *)imageData) + (slice * numPixelsInSlice), numPixelsInSlice );
            float rescaleSlope  = tmpImage->GetDcmHeader()->GetFloatValue("RescaleSlope"); 
            rescaleSlopeVector.push_back(rescaleSlope); 
            tmpImage->Delete(); 
        }

        int floatWordSize = 4;
        void* imageFloatData = (void* ) malloc( numPixelsInSlice * numSlices * floatWordSize );
        svkImageData* tmpImage = svkMriImageData::New(); 
        tmpImage->GetDcmHeader()->ReadDcmFileHeaderOnly( this->GetFileNames()->GetValue( vol ) );
        this->GetSUVScaledPixels((float*)imageFloatData,
                                    (unsigned short*)imageData ,
                                    numSlices * numPixelsInSlice,
                                    rescaleSlopeVector);
        delete (unsigned short*)imageData;
        imageData = imageFloatData;
        tmpImage->Delete();

        array->SetVoidArray( (void*)(imageData), numPixelsInSlice * numSlices, 0);
        if (vol == 0 ) {
            data->GetPointData()->SetScalars(array);
        } else {
            data->GetPointData()->AddArray(array);
        }

        // This array has been attached to the point data so lets release our local reference.
        array->Delete();

        if( vol % 2 == 0 ) { // update progress every other volume
			ostringstream progressStream;
			progressStream <<"Reading Volume " << vol << " of " << this->numVolumes;
			this->SetProgressText( progressStream.str().c_str() );
			this->UpdateProgress( vol/((double)this->numVolumes) );
        }

    }
}


/*!
 *  Scales an array of unsigned short pixel values into to a floating point
 *  array according to the given center and window. The minimum value of the
 *  output will be the center - window/2 and the max will be center + window/2.
 */
void svkDcmPETVolumeReader::GetSUVScaledPixels( float* floatPixels, unsigned short* shortPixels, int numberOfValues, vector <float> rescaleSlopeVector)
{
    float bw            = this->GetOutput()->GetDcmHeader()->GetFloatValue("PatientWeight");
    float injectedDose  = this->GetOutput()->GetDcmHeader()->GetFloatSequenceItemElement(
                                "RadiopharmaceuticalInformationSequence", 0,  "RadionuclideTotalDose");
    //  Start time: 
    string seriesTime   = this->GetOutput()->GetDcmHeader()->GetStringValue("SeriesTime");
    //  break int HHMMSS
    int endHH = svkTypeUtils::StringToInt( seriesTime.substr(0, 2) ); 
    int endMM = svkTypeUtils::StringToInt( seriesTime.substr(2, 2) ); 
    int endSS = svkTypeUtils::StringToInt( seriesTime.substr(4, 2) ); 
   

    string startTime = this->GetOutput()->GetDcmHeader()->GetStringSequenceItemElement(
                                "RadiopharmaceuticalInformationSequence", 0, "RadiopharmaceuticalStartTime");
    //  break int HHMMSS
    int startHH = svkTypeUtils::StringToInt( startTime.substr(0, 2) ); 
    int startMM = svkTypeUtils::StringToInt( startTime.substr(2, 2) ); 
    int startSS = svkTypeUtils::StringToInt( startTime.substr(4, 2) ); 

    //  Compute delta time in seconds: 
    int scanTime = (endHH - startHH) * 60 * 60 + (endMM - startMM) * 60 + (endSS - startSS); 
    
    float halfLife      = this->GetOutput()->GetDcmHeader()->GetFloatSequenceItemElement(
                                "RadiopharmaceuticalInformationSequence", 0, "RadionuclideHalfLife");

    float decayedDose   = injectedDose * pow(2, -1*( scanTime ) / halfLife);

    float pixelScaleFactor = (bw * 1000 / decayedDose);

    //  BW SUV scaling
    int x; 
    int y; 
    int z; 
    for( int i = 0; i < numberOfValues; i++ ) {
        //  Get slice number for this pixel and scale it: 
        this->GetOutput()->GetIndexFromID(i, &x, &y, &z);
        floatPixels[i] = pixelScaleFactor * rescaleSlopeVector[z] * shortPixels[i];
    }
}


/*!
 *  Returns the pixel type 
 */
svkDcmHeader::DcmPixelDataFormat svkDcmPETVolumeReader::GetFileType()
{
    return svkDcmHeader::SIGNED_FLOAT_4;
}

