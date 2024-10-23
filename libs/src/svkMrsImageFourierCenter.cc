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



#include <svkMrsImageFourierCenter.h>
#include <svkImageFourierCenter.h>
#include </usr/include/vtk/vtkImageFFT.h>
#include <svkMrsImageFFT.h>

using namespace svk;


//vtkCxxRevisionMacro(svkMrsImageFourierCenter, "$Rev$");
vtkStandardNewMacro(svkMrsImageFourierCenter);


/*!
 *  Constructor.  Initialize any member variables. 
 */
svkMrsImageFourierCenter::svkMrsImageFourierCenter()
{

#if VTK_DEBUG_ON
    this->DebugOn();
#endif

    vtkDebugMacro(<<this->GetClassName() << "::" << this->GetClassName() << "()");

    //  Initialize any member variables
    this->shiftDomain = SPATIAL; 
    this->shiftDirection = FORWARD; 
}


/*!
 *  Clean up any allocated member variables. 
 */
svkMrsImageFourierCenter::~svkMrsImageFourierCenter()
{
}


/*
 *  Sets the domain to shift, spectral or spatial. 
 */
void svkMrsImageFourierCenter::SetShiftDomain( ShiftDomain domain)
{
    this->shiftDomain = domain; 
}


/*
 *  Sets the domain to shift, spectral or spatial. 
 *  FORWARD is used to move 0 frequency from the origin to the image center. 
 *  REVERSE is used to move 0 frequency from the image center to the origin.
 *  image origin.
 */
void svkMrsImageFourierCenter::SetShiftDirection( ShiftDirection direction)
{
    this->shiftDirection = direction; 
}


/*! 
 *  This method is called during pipeline execution.  This is where you should implement your algorithm. 
 */
int svkMrsImageFourierCenter::RequestData( vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector )
{

    // if shifing in spatial domain:
    if ( this->shiftDomain == svkMrsImageFourierCenter::SPATIAL ) {
        this->ApplySpatialShift(); 
    } else if ( this->shiftDomain == svkMrsImageFourierCenter::SPECTRAL ) {
        this->ApplySpectralShift(); 
    }
    
    return 1; 
}


/*
 *  Apply shift to spatial dimensions
 */
void svkMrsImageFourierCenter::ApplySpatialShift() 
{

    //  Get pointer to input data set. 
    svkMrsImageData* mrsData = svkMrsImageData::SafeDownCast(this->GetImageDataInput(0)); 

    //  Get pointer to data's meta-data header (DICOM object). 
    svkDcmHeader* hdr = mrsData->GetDcmHeader();  

    int numSpecPts = hdr->GetIntValue( "DataPointColumns" );
    int cols       = hdr->GetIntValue( "Columns" );
    int rows       = hdr->GetIntValue( "Rows" );
    int slices     = hdr->GetNumberOfSlices();
    int numTimePts = hdr->GetNumberOfTimePoints();
    int numCoils   = hdr->GetNumberOfCoils();  

    svkMriImageData* singleFreqImage = svkMriImageData::New();
    vtkImageData* tmpData = NULL;

    for( int timePt = 0; timePt < numTimePts; timePt++ ) {
        for( int coil = 0; coil < numCoils; coil++ ) {
            for( int freq = 0; freq < numSpecPts; freq++ ) {

                //  ImageFourierCenter requires VTK_DOUBLE
                mrsData->GetImage( singleFreqImage, freq, timePt, coil, 2, "", VTK_DOUBLE);
                singleFreqImage->Modified();

                tmpData = singleFreqImage;

                svkImageFourierCenter* imageShift = svkImageFourierCenter::New();

                //  Reverse is used to move the 0 frequency component from the 
                //  image center back to the image origin as required for FFT algo. 
                if ( this->shiftDirection == svkMrsImageFourierCenter::REVERSE ) {
                    imageShift->SetReverseCenter( true );
                }

                imageShift->SetInputData( tmpData ); 
                tmpData = imageShift->GetOutput();

                imageShift->Update();

                mrsData->SetImage( tmpData, freq, timePt, coil);

                imageShift->Delete(); 
            }
        }
    }


    //  Trigger observer update via modified event:
    this->GetInput()->Modified();
    this->Update();

} 


/*
 *  Apply shift to spectral data 
 */
void svkMrsImageFourierCenter::ApplySpectralShift() 
{

    //  Get pointer to input data set. 
    svkMrsImageData* mrsData = svkMrsImageData::SafeDownCast(this->GetImageDataInput(0)); 

    //  Get pointer to data's meta-data header (DICOM object). 
    svkDcmHeader* hdr = mrsData->GetDcmHeader();  

    int numSpecPts = hdr->GetIntValue( "DataPointColumns" );
    int cols       = hdr->GetIntValue( "Columns" );
    int rows       = hdr->GetIntValue( "Rows" );
    int slices     = hdr->GetNumberOfSlices();
    int numTimePts = hdr->GetNumberOfTimePoints();
    int numCoils   = hdr->GetNumberOfCoils();  

    //  Loop over all spectra and apply shift in appropriate direction 
    //  to each one
    for( int timePt = 0; timePt < numTimePts; timePt++ ) {
        for( int coil = 0; coil < numCoils; coil++ ) {
            for( int z = 0; z < slices; z++ ) {
                for( int y = 0; y < rows; y++ ) {
                    for( int x = 0; x < cols; x++ ) {


                        vtkFloatArray* spectrum = static_cast<vtkFloatArray*>(
                            mrsData->GetSpectrum( x, y, z, timePt, coil) 
                        );

                        vtkImageComplex* complexSpectrum = new vtkImageComplex[ numSpecPts ];
                        svkMrsImageFFT::ConvertArrayToImageComplex( spectrum, complexSpectrum );


                        //  FORWARD: MOVES 0 to from initial point to Center
                        //  REVERSE: Moves 0 from center to initial point 
                        if ( this->shiftDirection == svkMrsImageFourierCenter::FORWARD) {

                            svkMrsImageFFT::FFTShift( complexSpectrum, numSpecPts); 

                        } else if ( this->shiftDirection == svkMrsImageFourierCenter::REVERSE ) {

                            // Move t=0 from center to origin 
                            svkMrsImageFFT::IFFTShift( complexSpectrum, numSpecPts); 

                        } 

                        //  Now set the shifted data back into the MRS data object:
                        for (int i = 0; i < numSpecPts; i++) {
                            spectrum->SetTuple2( i, complexSpectrum[i].Real, complexSpectrum[i].Imag ); 
                        }
           
                    }
                }
            }
        }
    }


    //  Trigger observer update via modified event:
    this->GetInput()->Modified();
    this->Update();
}


/*!
 *  Set the input data type to svkMrsImageData.
 */
int svkMrsImageFourierCenter::FillInputPortInformation( int vtkNotUsed(port), vtkInformation* info )
{
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "svkMrsImageData"); 
    return 1;
}


