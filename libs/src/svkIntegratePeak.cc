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



#include <svkIntegratePeak.h>
#include <svkSpecPoint.h>


using namespace svk;


vtkCxxRevisionMacro(svkIntegratePeak, "$Rev$");
vtkStandardNewMacro(svkIntegratePeak);


svkIntegratePeak::svkIntegratePeak()
{

#if VTK_DEBUG_ON
    this->DebugOn();
#endif

    vtkDebugMacro(<<this->GetClassName() << "::" << this->GetClassName() << "()");
    
    this->magnitudeIntegration = false; 
}


/*!
 *
 */
svkIntegratePeak::~svkIntegratePeak()
{
}


/*!
 *  Set the chemical shift of the peak position to integrate over. 
 */
void svkIntegratePeak::SetPeakPosPPM( float centerPPM ) 
{
    this->peakCenterPPM = centerPPM; 
}


/*!
 *  Set the chemical shift range to integrate over.  Integration will be +/- 1/2 this 
 *  width about the peak position. 
 */
void svkIntegratePeak::SetPeakWidthPPM( float widthPPM ) 
{
    this->peakWidthPPM = widthPPM; 
}


/*!
 *  Use magnitude integration, rather than complex. 
 */
void svkIntegratePeak::SetMagnitudeIntegration() 
{
    this->magnitudeIntegration = true; 
}


/*! 
 *
 */
int svkIntegratePeak::RequestData( vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector )
{

    svkImageData* data = this->GetImageDataInput(0); 

    int numFrequencyPoints = data->GetCellData()->GetNumberOfTuples();
    int numComponents = data->GetCellData()->GetNumberOfComponents();
    int numChannels  = data->GetDcmHeader()->GetNumberOfCoils();
    int numTimePts = data->GetDcmHeader()->GetNumberOfTimePoints();

    int numVoxels[3]; 
    data->GetNumberOfVoxels(numVoxels); 

    //  Get the integration range in points:
    svkSpecPoint* point = svkSpecPoint::New();
    point->SetDcmHeader( data->GetDcmHeader() ); 

    int startPoint =  static_cast< int > (
                        point->ConvertPosUnits(
                            this->peakCenterPPM - (this->peakWidthPPM/2), svkSpecPoint::PPM, svkSpecPoint::PTS )
                      );   
    int endPoint   =  static_cast< int > (
                        point->ConvertPosUnits(
                            this->peakCenterPPM + (this->peakWidthPPM/2), svkSpecPoint::PPM, svkSpecPoint::PTS )
                      );   

    // For each voxel, add frequency points.
    float cmplxPt[2];
    float integral[2];

    for( int channel = 0; channel < numChannels; channel++ ) { 
        for (int timePt = 0; timePt < numTimePts; timePt++) {
            cout << "channel_number time" << channel << " " << timePt << endl; 
            for (int z = 0; z < numVoxels[2]; z++) {
                for (int y = 0; y < numVoxels[1]; y++) {
                    for (int x = 0; x < numVoxels[0]; x++) {
            
               
                        integral[0] = 0;
                        integral[1] = 0; 
          
                        vtkFloatArray* spectrum = static_cast<vtkFloatArray*>(
                            svkMrsImageData::SafeDownCast(data)->GetSpectrum( x, y, z, timePt, channel) );

                        for (int i = startPoint; i <= endPoint; i++) {

                            spectrum->GetTupleValue(i, cmplxPt);

                            if ( this->magnitudeIntegration ) {
                                integral[0] += pow( 
                                                ( ( cmplxPt[0] * cmplxPt[0] ) + ( cmplxPt[1] * cmplxPt[1] ) ), 0.5 ) ; 
                                integral[1] += 0;  
                            } else {
                                integral[0] += cmplxPt[0]; 
                                integral[1] += cmplxPt[1]; 
                            }

                        }

                        //  Shrink the array to one point:
                        if ( spectrum->Resize(1) ) { 
                            spectrum->SetTuple(0, cmplxPt);
                        } else {
                            cout << "svkIntegratePeak: ERROR resizing array " << endl;
                        }

                    }
                }
            }
        }
    }

    //  Redimension data set:
    this->RedimensionData( data ); 

    //  Trigger observer update via modified event:
    this->GetInput()->Modified();
    this->GetInput()->Update();
    return 1; 
} 


/*!
 *  Modify the number of frequency points in the data set. 
 *  Also set the ppm value to the peak center. 
 */
void svkIntegratePeak::RedimensionData( svkImageData* data )
{

    this->GetOutput()->GetDcmHeader()->SetValue(
        "DataPointColumns",
        1
    );

    data->GetDcmHeader()->PrintDcmHeader( ); 
}


/*!
 *
 */
int svkIntegratePeak::FillInputPortInformation( int vtkNotUsed(port), vtkInformation* info )
{
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "svkMrsImageData"); 
    return 1;
}


