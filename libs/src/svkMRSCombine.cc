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



#include <svkMRSCombine.h>


using namespace svk;


vtkStandardNewMacro(svkMRSCombine);


svkMRSCombine::svkMRSCombine()
{

#if VTK_DEBUG_ON
    this->DebugOn();
#endif

    vtkDebugMacro(<<this->GetClassName() << "::" << this->GetClassName() << "()");

    this->combinationMethod = svkMRSCombine::ADDITION; 
    this->combinationDimension = svkMRSCombine::COIL; 
    this->SetNumberOfInputPorts(2); 

}


svkMRSCombine::~svkMRSCombine()
{
}


/*
 *
 */
void svkMRSCombine::SetCombinationMethod( CombinationMethod method)
{
    this->combinationMethod = method; 
}


/*
 *
 */
void svkMRSCombine::SetCombinationDimension( CombinationDimension dimension )
{
    this->combinationDimension = dimension; 
}

/*! 
 *
 */
int svkMRSCombine::RequestInformation( vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector )
{
    return 1;
}


/*! 
 *
 */
int svkMRSCombine::RequestData( vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector )
{

    if ( this->combinationMethod == svkMRSCombine::ADDITION || 
         this->combinationMethod == svkMRSCombine::WEIGHTED_ADDITION ||
         this->combinationMethod == svkMRSCombine::WEIGHTED_ADDITION_SQRT_WT ||
         this->combinationMethod == svkMRSCombine::SUBTRACTION ) {
        this->RequestLinearCombinationData(); 
    } else if (this->combinationMethod == svkMRSCombine::SUM_OF_SQUARES ) {
        this->RequestSumOfSquaresData(); 
    }

    //  Redimension data set:
    this->RedimensionData(); 

    //  scale weighted images to same intensity as input: 
    if (
        this->combinationMethod == svkMRSCombine::WEIGHTED_ADDITION  //||
        //this->combinationMethod == svkMRSCombine::WEIGHTED_ADDITION_SQRT_WT
    )
    {
        this->maxSignalIntensityOutput = this->GetMaxSignalIntensity();
        //cout << "MAX SIG IN:  " << this->maxSignalIntensityInput << endl;
        //cout << "MAX SIG OUT: " << this->maxSignalIntensityOutput << endl;
        this->ScaleOutputIntensity(); 
    }

    //  Trigger observer update via modified event:
    this->GetOutput()->GetProvenance()->AddAlgorithm( this->GetClassName() );
    this->GetInput()->Modified();
    
    return 1; 

}


/*
 *  Returns the maximum MRS signal intensity over all channels. 
 */
float svkMRSCombine::ScaleOutputIntensity() 
{
    svkMrsImageData* data = svkMrsImageData::SafeDownCast(this->GetImageDataInput(0));
    svkDcmHeader::DimensionVector dimensionVector = data->GetDcmHeader()->GetDimensionIndexVector();
    int numTimePoints = data->GetDcmHeader()->GetIntValue( "DataPointColumns" );
    int numCells = svkDcmHeader::GetNumberOfCells( &dimensionVector );

    float globalScale = this->maxSignalIntensityInput/this->maxSignalIntensityOutput; 
    //globalScale = 100; // to validate against original "nmrsrc" implementation. 

    for ( int cellID = 0; cellID < numCells; cellID++ ) {

        vtkFloatArray* spectrum = static_cast<vtkFloatArray*>( data->GetSpectrum( cellID ) );
        double tuple[2];
        for (int i = 0; i < numTimePoints; i++ ) {
            spectrum->GetTuple(i, tuple);
            tuple[0] = tuple[0] * globalScale;  
            tuple[1] = tuple[1] * globalScale;  
            spectrum->SetTuple(i, tuple); 
        }
    }
}


/*
 *  Returns the maximum MRS signal intensity over all channels. 
 */
float svkMRSCombine::GetMaxSignalIntensity() 
{
    float maxSignalIntensity = VTK_FLOAT_MIN; 

    svkMrsImageData* data = svkMrsImageData::SafeDownCast(this->GetImageDataInput(0));
    svkDcmHeader::DimensionVector dimensionVector = data->GetDcmHeader()->GetDimensionIndexVector();
    int numTimePoints = data->GetDcmHeader()->GetIntValue( "DataPointColumns" );
    int numCells = svkDcmHeader::GetNumberOfCells( &dimensionVector );

    for ( int cellID = 0; cellID < numCells; cellID++ ) {

        vtkFloatArray* spectrum = static_cast<vtkFloatArray*>( data->GetSpectrum( cellID ) );
        double tuple[2];
        for (int i = 0; i < numTimePoints; i++ ) {
            spectrum->GetTuple(i, tuple);
            if ( tuple[0] > maxSignalIntensity ) {
                maxSignalIntensity = tuple[0]; 
            }
        }

    }

    if (maxSignalIntensity == 0. ) {    
        maxSignalIntensity = 1; 
    }
        
    return maxSignalIntensity; 
}


/*
 *  Returns the sum of squares of the weight values for the specified voxel. 
 */
float svkMRSCombine::GetTotalWeight( svkMriImageData* weightImage, int voxelID) 
{
    svkMrsImageData* data = svkMrsImageData::SafeDownCast( this->GetImageDataInput(0) ); 
    svkDcmHeader* hdr     = data->GetDcmHeader();
    int numChannels       = hdr->GetNumberOfCoils();
    float weightSumOfSquares = 0; 
    for( int channel = 0; channel < numChannels; channel++ ) { 
        float wt = weightImage->GetPointData()->GetAbstractArray(channel)->GetVariantValue(voxelID).ToFloat(); 
        weightSumOfSquares += ( wt * wt ); 
    }
    return weightSumOfSquares; 
}


/*!
 *  Combine data from each channel on a voxel by voxel basis. 
 */
void svkMRSCombine::RequestLinearCombinationData( )
{
    //  Iterate through spectral data from all cells.  
    svkMrsImageData* data = svkMrsImageData::SafeDownCast( this->GetImageDataInput(0) ); 
    svkDcmHeader* hdr  = data->GetDcmHeader();

    int numFrequencyPoints  = hdr->GetIntValue( "DataPointColumns" );
    int numChannels         = hdr->GetNumberOfCoils();
    int numTimePts          = hdr->GetNumberOfTimePoints();
    int numComponents       = data->GetCellData()->GetNumberOfComponents();

    int numVoxels[3]; 
    data->GetNumberOfVoxels(numVoxels); 

    // For each voxel, add data from individual voxels:  
    float cmplxPt0[2];
    float cmplxPtN[2];

    //  If this is a weighted sum, then load in the weight values here: 
    svkMriImageData* weightImage;
    if ( 
        this->combinationMethod == svkMRSCombine::WEIGHTED_ADDITION  ||
        this->combinationMethod == svkMRSCombine::WEIGHTED_ADDITION_SQRT_WT
    ) 
    {
        weightImage = svkMriImageData::SafeDownCast(this->GetImageDataInput( svkMRSCombine::WEIGHTS ) );

        this->maxSignalIntensityInput= this->GetMaxSignalIntensity(); 
    } 

    for (int timePt = 0; timePt < numTimePts; timePt++) {
        for (int z = 0; z < numVoxels[2]; z++) {
            for (int y = 0; y < numVoxels[1]; y++) {
                for (int x = 0; x < numVoxels[0]; x++) {

                    // Get the voxelID for this spatial location
                    int voxelID = data->GetIDFromIndex(x, y, z); 

                    //  for each voxel, get the total weight: 
                    float weightTotal;
                    if ( 
                        this->combinationMethod == svkMRSCombine::WEIGHTED_ADDITION ||
                        this->combinationMethod == svkMRSCombine::WEIGHTED_ADDITION_SQRT_WT
                    ) {
                        weightTotal = this->GetTotalWeight( weightImage, voxelID); 
                        if ( this->combinationMethod == svkMRSCombine::WEIGHTED_ADDITION_SQRT_WT ) {
                            weightTotal = pow( (float)weightTotal, (float)0.5); 
                        }
                    }
                    //cout << "WT TOTAL: " << weightTotal << endl;

                    //  for each frequency point, combine channels using specified combination method.
                    for (int freq = 0; freq < numFrequencyPoints; freq++) {

                        //  for this frequency point initialize the value to 0    
                        cmplxPt0[0] = 0.; 
                        cmplxPt0[1] = 0.; 

                        for( int channel = 0; channel < numChannels; channel++ ) { 

                            vtkFloatArray* spectrumN = static_cast<vtkFloatArray*>( data->GetSpectrum( x, y, z, timePt, channel) );
                            spectrumN->GetTuple(freq, cmplxPtN);

                            if ( this->combinationMethod == svkMRSCombine::ADDITION ) {

                                cmplxPt0[0] += ( cmplxPtN[0] ); 
                                cmplxPt0[1] += ( cmplxPtN[1] ); 

                            } else if ( 
                                this->combinationMethod == svkMRSCombine::WEIGHTED_ADDITION || 
                                this->combinationMethod == svkMRSCombine::WEIGHTED_ADDITION_SQRT_WT
                            ) {
                                
                                //  if all weights are zero, the combination in the previous two lines wil
                                //  be zero, but don't then divide by zero.  For WEIGHTED_ADDITION should use
                                //  a wt_threshold based on  wt_threshold = .2 * wt_median * numChannels. 
                                if ( weightTotal == 0 ) {    
                                    weightTotal = 1; 
                                }
                                float wt = weightImage->GetPointData()->GetAbstractArray(channel)->GetVariantValue(voxelID).ToFloat(); 
                                //cout <<  "weights[" << voxelID << "] = " << wt << endl;
                                cmplxPt0[0] += wt * ( cmplxPtN[0] ) / weightTotal; 
                                cmplxPt0[1] += wt * ( cmplxPtN[1] ) / weightTotal; 

                            } else if ( this->combinationMethod == svkMRSCombine::SUBTRACTION ) {

                                float subtractionWeight; 
                                if ( channel == 0 ) {
                                    subtractionWeight = 1; 
                                } else if ( channel == 1 ) {
                                    subtractionWeight = -1; 
                                } else {
                                    cerr << "ERROR, can not run subtraction algorithm on more than 2 channels." << endl;
                                    exit(1); 
                                }
                                cmplxPt0[0] += subtractionWeight * ( cmplxPtN[0] ); 
                                cmplxPt0[1] += subtractionWeight * ( cmplxPtN[1] ); 

                            }
    
                        }
                        //  write output into the first channel. 
                        int channelOut = 0; 
                        vtkFloatArray* spectrum0 = static_cast<vtkFloatArray*>( data->GetSpectrum( x, y, z, timePt, channelOut) );
                        spectrum0->SetTuple( freq, cmplxPt0);
                    }

                }
            }
        }
    }

} 


/*!
 *  Combine data by sum of squares.  Input data is complex, output data is magnitude (one component). 
 */
void svkMRSCombine::RequestSumOfSquaresData()
{

    //  Iterate through spectral data from all cells.  Eventually for performance I should do this by visible
    svkMrsImageData* data = svkMrsImageData::SafeDownCast( this->GetImageDataInput(0) ); 
    svkDcmHeader* hdr = data->GetDcmHeader();

    int numFrequencyPoints  = hdr->GetIntValue( "DataPointColumns" );
    int numChannels         = hdr->GetNumberOfCoils();
    int numTimePts          = hdr->GetNumberOfTimePoints();

    int numVoxels[3]; 
    data->GetNumberOfVoxels(numVoxels); 

    //  Magnitude Output 
    int numComponents = 1; 


    // For each voxel, add data from individual voxels:  
    float cmplxPtN[2];
    float cmplxPtNNew[2];
    float magnigutdValue; 


    int outterLoopLimits[2];
    int innerLoopLimits[2];

    if ( this->combinationDimension == COIL ) {
        outterLoopLimits[0] = 0; 
        outterLoopLimits[1] = numTimePts; 
        innerLoopLimits[0] = 0; 
        innerLoopLimits[1] = numChannels; 
    } else if ( this->combinationDimension == TIME ) {
        outterLoopLimits[0] = 0; 
        outterLoopLimits[1] = numChannels; 
        innerLoopLimits[0] = 0; 
        innerLoopLimits[1] = numTimePts; 
    }


    int timePt; 
    int channel; 
    for (int outterLoopIndex = 0; outterLoopIndex < outterLoopLimits[1]; outterLoopIndex++) {

        for (int z = 0; z < numVoxels[2]; z++) {
            for (int y = 0; y < numVoxels[1]; y++) {
                for (int x = 0; x < numVoxels[0]; x++) {

                    //float* magnitudeData = new float[numFrequencyPoints];
                    float* magnitudeData = new float[numFrequencyPoints * 2];

                    for (int freq = 0; freq < numFrequencyPoints; freq++) {

                        cmplxPtNNew[ 0 ] = 0.; 
                        cmplxPtNNew[ 1 ] = 0.; 

                        //  foreach freq point, get sum of squares over all channels
                        for( int innerLoopIndex = 0; innerLoopIndex < innerLoopLimits[1]; innerLoopIndex++ ) { 

                            if ( this->combinationDimension == COIL ) {
                                timePt  = outterLoopIndex; 
                                channel = innerLoopIndex; 
                            } else if ( this->combinationDimension == TIME ) {
                                channel = outterLoopIndex; 
                                timePt  = innerLoopIndex; 
                            }

                            vtkFloatArray* spectrumN = static_cast<vtkFloatArray*>(
                                            svkMrsImageData::SafeDownCast(data)->GetSpectrum( x, y, z, timePt, channel ) );
    
                            spectrumN->GetTuple(freq, cmplxPtN);
    
                            cmplxPtNNew[0] += ( cmplxPtN[0] * cmplxPtN[0] + cmplxPtN[1] * cmplxPtN[1] ); 
                            cmplxPtNNew[1] += 0; 

                            if ( innerLoopIndex == innerLoopLimits[1] - 1 ) {

                                cmplxPtNNew[0] /= innerLoopLimits[1]; 
                                cmplxPtNNew[0] = pow( (double)cmplxPtNNew[0], (double)0.5 );
                                cmplxPtNNew[1] = 0; 

                                int targetTimeIndex; 
                                int targetChannelIndex; 
                                if ( this->combinationDimension == COIL ) {
                                    targetChannelIndex = 0;
                                    targetTimeIndex = timePt; 
                                } else if ( this->combinationDimension == TIME ) {
                                    targetChannelIndex = channel; 
                                    targetTimeIndex = 0; 
                                }

                                vtkFloatArray* spectrumN = static_cast<vtkFloatArray*>(
                                            svkMrsImageData::SafeDownCast(data)->GetSpectrum( 
                                                    x, y, z, targetTimeIndex, targetChannelIndex) 
                                        );
                                spectrumN->SetTuple(freq, cmplxPtNNew);
                            }

                        }

                    }

                }
            }
        }
    }

}


/*!
 *  Remove coil dimension from data set:
 */
void svkMRSCombine::RedimensionData()
{ 

    svkMrsImageData* data = svkMrsImageData::SafeDownCast( this->GetImageDataInput(0) );

    int numCoils = data->GetDcmHeader()->GetNumberOfCoils();
    int numTimePts = data->GetDcmHeader()->GetNumberOfTimePoints();

    int numVoxels[3]; 
    data->GetNumberOfVoxels(numVoxels); 

    int minTimePt; 
    int minChannel; 
    int newNumTimePts = numTimePts; 
    int newNumChannels = numCoils; 

    if ( this->combinationDimension == COIL ) {
        minTimePt  = 0; 
        minChannel = 1; 
        newNumChannels = 1; 
    } else if ( this->combinationDimension == TIME ) {
        minTimePt  = 1; 
        minChannel = 0; 
        newNumTimePts = 1; 
    }

    //  Remove all arrays with coil number > 0
    for (int coilNum = minChannel; coilNum < numCoils; coilNum++) {
        for (int timePt = minTimePt; timePt < numTimePts; timePt++) {
            for (int z = 0; z < numVoxels[2]; z++) {
                for (int y = 0; y < numVoxels[1]; y++) {
                    for (int x = 0; x < numVoxels[0]; x++) {
                        char arrayName[30];
                        sprintf(arrayName, "%d %d %d %d %d", x, y, z, timePt, coilNum);
                        data->GetCellData()->RemoveArray( arrayName );
                    }     
                }
            }
        }
    }

    double origin[3]; 
    data->GetDcmHeader()->GetOrigin( origin, 0 ); 

    double voxelSpacing[3];  
    data->GetDcmHeader()->GetPixelSpacing( voxelSpacing );

    double dcos[3][3]; 
    data->GetDcmHeader()->GetDataDcos( dcos ); 


    svkDcmHeader::DimensionVector dimensionVector = data->GetDcmHeader()->GetDimensionIndexVector();
    svkDcmHeader::SetDimensionVectorValue(&dimensionVector, svkDcmHeader::SLICE_INDEX, data->GetDcmHeader()->GetNumberOfSlices()-1);
    svkDcmHeader::SetDimensionVectorValue(&dimensionVector, svkDcmHeader::TIME_INDEX, newNumTimePts-1);
    svkDcmHeader::SetDimensionVectorValue(&dimensionVector, svkDcmHeader::CHANNEL_INDEX, newNumChannels-1);

    data->GetDcmHeader()->InitPerFrameFunctionalGroupSequence( 
        origin,
        voxelSpacing,
        dcos, 
        &dimensionVector
    );

    if (this->GetDebug()) { 
        data->GetDcmHeader()->PrintDcmHeader( ); 
    }
    
}




/*!
 *  Inputs: 
 *      - requires an MRS object to combine
 *      - optional MRI object containing weights for each point being combined. 
 */
int svkMRSCombine::FillInputPortInformation( int port, vtkInformation* info )
{
    if ( port == 0 ) {
        info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "svkMrsImageData"); 
    }
    if ( port == 1 ) {
        info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "svkMriImageData");
        info->Set(vtkAlgorithm::INPUT_IS_OPTIONAL(), 1);
    }
    return 1;
}


