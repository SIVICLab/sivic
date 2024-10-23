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



#include <svkIntegratePeak.h>
#include <svkSpecPoint.h>

#include </usr/include/vtk/vtkImageWeightedSum.h>
#include </usr/include/vtk/vtkImageMathematics.h>
#include </usr/include/vtk/vtkImageMagnitude.h>



using namespace svk;


//vtkCxxRevisionMacro(svkIntegratePeak, "$Rev$");
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
 *  Generates a metabolite map from the integral over the specified range.  Currently the data struct is an MRS object with 
 *  one frequency pt, but should be a metabolite map. 
 */
int svkIntegratePeak::RequestData( vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector )
{

    svkMrsImageData* data = svkMrsImageData::SafeDownCast( this->GetImageDataInput(0) ); 

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
                            this->peakCenterPPM + (this->peakWidthPPM/2), svkSpecPoint::PPM, svkSpecPoint::PTS )
                      );   
    int endPoint   =  static_cast< int > (
                        point->ConvertPosUnits(
                            this->peakCenterPPM - (this->peakWidthPPM/2), svkSpecPoint::PPM, svkSpecPoint::PTS )
                      );   

    point->Delete();

    // For each voxel, add frequency points.
    float cmplxPt[2];
    float integral[2];

    svkMriImageData* ptImage = svkMriImageData::New();
    svkMriImageData* sumImage = svkMriImageData::New();

    for( int channel = 0; channel < numChannels; channel++ ) {
        for (int timePt = 0; timePt < numTimePts; timePt++) {

            for (int i = startPoint; i <= endPoint; i++) {

                data->GetImage( ptImage, i, timePt, channel, 2, "" );
                //cout << " freq: " << i << " " << ( ptImage->GetPointData()->GetScalars()->GetComponent(0,0) ) << endl;;
                //cout << " freq: " << i << " " << ( ptImage->GetPointData()->GetScalars()->GetComponent(0,1) ) << endl;;
                ptImage->Modified();

                if ( this->magnitudeIntegration ) {
                    vtkImageMagnitude* mag = vtkImageMagnitude::New();
                    mag->SetInputData( ptImage ); 
                    mag->Update(); 

                    double sumTuple[2];  
                    double magTuple[1];  
                    if ( i == startPoint ) {
                        // Initialize it
                        sumImage->DeepCopy( ptImage ); 
                        // Zero it
                        for ( int j=0; j<sumImage->GetPointData()->GetScalars()->GetNumberOfTuples(); j++ ) {
                            sumImage->GetPointData()->GetScalars()->GetTuple(j, sumTuple);
                            sumTuple[0] = 0; 
                            sumTuple[1] = 0; 
                            sumImage->GetPointData()->GetScalars()->SetTuple(j, sumTuple);
                        }
                    } else {
                        for ( int j=0; j<sumImage->GetPointData()->GetScalars()->GetNumberOfTuples(); j++ ) {
                            sumImage->GetPointData()->GetScalars()->GetTuple(j, sumTuple);
                            mag->GetOutput()->GetPointData()->GetScalars()->GetTuple(j, magTuple);
                            sumTuple[0] += magTuple[0];
                            sumTuple[1] = 0; 
                            sumImage->GetPointData()->GetScalars()->SetTuple(j, sumTuple);
                        }      
                    }
                    
                    mag->Delete();
                }

            }

            //Set this image as the first frequency point in the MRS object: 
            data->SetImage( sumImage, 0, timePt, channel );

        }
    }

    //  Redimension data set:
    this->UpdateHeader(data); 

    //  Trigger observer update via modified event:
    this->GetInput()->Modified();
    this->Update();

    ptImage->Delete(); 
    sumImage->Delete(); 

    return 1; 
} 


/*!
 *  Modify the number of frequency points in the data set. 
 *  Also set the ppm value to the peak center. 
 */
void svkIntegratePeak::UpdateHeader(svkImageData* data)
{

    data->GetDcmHeader()->SetValue(
        "DataPointColumns",
        1
    );

    data->GetDcmHeader()->SetValue(
        "ChemicalShiftReference",
        this->peakCenterPPM
    );

    //data->GetDcmHeader()->SetValue(
        //"DataRepresentation",
        //"MAGNITUDE" 
    //);


}


/*!
 *
 */
int svkIntegratePeak::FillInputPortInformation( int vtkNotUsed(port), vtkInformation* info )
{
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "svkMrsImageData"); 
    return 1;
}


