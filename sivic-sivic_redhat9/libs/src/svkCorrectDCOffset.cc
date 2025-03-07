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


#include <svkCorrectDCOffset.h>


using namespace svk;


//vtkCxxRevisionMacro(svkCorrectDCOffset, "$Rev$");
vtkStandardNewMacro(svkCorrectDCOffset);


/*!
 *
 */
svkCorrectDCOffset::svkCorrectDCOffset()
{

#if VTK_DEBUG_ON
    this->DebugOn();
#endif

    vtkDebugMacro( << this->GetClassName() << "::" << this->GetClassName() << "()" );

}


/*!
 *
 */
svkCorrectDCOffset::~svkCorrectDCOffset()
{
    vtkDebugMacro( << this->GetClassName() << "::~" << this->GetClassName() << "()" );
}


/*!
 *
 */ 
int svkCorrectDCOffset::RequestData( vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector )
{
    this->CorrectDCOffset(); 
    this->SetProvenance(); 

    //  Trigger observer update via modified event:
    this->GetInput()->Modified();

    return 1; 
}


/*!
 *  Correct any DC offset present in the sampled FIDs.  This is done on an 
 *  channel by channel basis using the average offset from sampled FIDs in 
 *  that channel. This implementation assumes nothing aobut the sampling 
 *  trajectory and assumes a voxel was sample if it has non-zero data values
 *  in the FID.  Only sampled FIDs are included in the determination of the 
 *  DC offset.  The complex offset is the average of the last 1/8 of the FID 
 *  for all sampled FIDs. 
 */
void svkCorrectDCOffset::CorrectDCOffset()
{

    svkMrsImageData* mrsData = svkMrsImageData::SafeDownCast( this->GetImageDataInput(0) );

    svkDcmHeader::DimensionVector fullDimensionVector = mrsData->GetDcmHeader()->GetDimensionIndexVector();
    //cout << "FULL" << endl;
    //svkDcmHeader::PrintDimensionIndexVector(&fullDimensionVector);

    svkDcmHeader::DimensionVector channelDimensionVector = fullDimensionVector;  
    //  analyze one channel at a time: 
    svkDcmHeader::SetDimensionVectorValue(&channelDimensionVector, svkDcmHeader::CHANNEL_INDEX, 0);
    //cout << "SUBSET FULL" << endl;
    //svkDcmHeader::PrintDimensionIndexVector(&channelDimensionVector);
    svkDcmHeader::DimensionVector indexVector = fullDimensionVector; 

    int numVoxelsPerChannel = svkDcmHeader::GetNumberOfCells( &channelDimensionVector ); 
    int numSpecPts          = mrsData->GetDcmHeader()->GetIntValue( "DataPointColumns" );
    int numCoils            = svkDcmHeader::GetDimensionVectorValue(&fullDimensionVector, svkDcmHeader::CHANNEL_INDEX) + 1; 

    string representation = mrsData->GetDcmHeader()->GetStringValue( "DataRepresentation" );
    int numComponents = 1;
    if (representation.compare("COMPLEX") == 0) {
        numComponents = 2;
    }

    //  average last 1/8 of FID from all spectra:
    int fractionOfSpectrum = static_cast<int>( numSpecPts / 8 );
    vtkFloatArray* spectrum;
    float offset[2]; 
    double cmplxPt[2]; 
    int numSampledFIDs = 0; 

    int channelVoxelIndex0 = 0;

    //  correct each coil separately
    for ( int coil = 0; coil < numCoils; coil++ ) {

        offset[0] = 0.0;    // DC offset real
        offset[1] = 0.0;    // DC offset imaginary
        numSampledFIDs = 0; 

        for( int cellID = 0; cellID < numVoxelsPerChannel; cellID++ ) {

            //  Get the dimensions for the single channel.  reset the channel index and get the 
            //  actual cellID for this channel 
            svkDcmHeader::GetDimensionVectorIndexFromCellID(&channelDimensionVector, &indexVector, cellID); 
            svkDcmHeader::SetDimensionVectorValue(&indexVector, svkDcmHeader::CHANNEL_INDEX, coil);
            int absoluteCellID = svkDcmHeader::GetCellIDFromDimensionVectorIndex(&fullDimensionVector, &indexVector); 
            spectrum = static_cast< vtkFloatArray* >( mrsData->GetSpectrum( absoluteCellID) );
            //svkDcmHeader::PrintDimensionIndexVector(&indexVector);

            if ( this->WasKSpacePtSampled( spectrum, numSpecPts * numComponents  ) ) {

                numSampledFIDs++; 

                for (int freq = numSpecPts - fractionOfSpectrum; freq < numSpecPts; freq++ ) {
                    spectrum->GetTuple(freq, cmplxPt);
                    offset[0] += cmplxPt[0];
                    offset[1] += cmplxPt[1];
                }
            }
        }

        offset[0] = offset[0] / ( numSampledFIDs * fractionOfSpectrum );
        offset[1] = offset[1] / ( numSampledFIDs * fractionOfSpectrum );

        //  correct the spectra from this coil; 
        for( int cellID = 0; cellID < numVoxelsPerChannel; cellID++ ) {

            //  Get the dimensions for the single channel.  reset the channel index and get the 
            //  actual cellID for this channel 
            svkDcmHeader::GetDimensionVectorIndexFromCellID(&channelDimensionVector, &indexVector, cellID); 
            svkDcmHeader::SetDimensionVectorValue(&indexVector, svkDcmHeader::CHANNEL_INDEX, coil);
            int absoluteCellID = svkDcmHeader::GetCellIDFromDimensionVectorIndex(&fullDimensionVector, &indexVector); 

            spectrum = static_cast< vtkFloatArray* >( mrsData->GetSpectrum( absoluteCellID ) );

            if ( this->WasKSpacePtSampled( spectrum, numSpecPts * numComponents  ) ) {

                for (int freq = 0; freq < numSpecPts; freq++ ) {

                    spectrum->GetTuple( freq, cmplxPt );

                    cmplxPt[0] -= offset[0];
                    cmplxPt[1] -= offset[1];
                    spectrum->SetTuple( freq, cmplxPt);

                }
            }
        }
    }

    this->GetInput()->Modified();
}


/*!
 *  Heuristic method used by !CorrectDCOffset() for quickly determining whether 
 *  a k-space point was sampled or not.  If the cmplx points are not zero 
 *  the data was sampled experimentally.  If a "reasonable" number of 
 *  points are identically zero then assume the data was zero padded and 
 *  should not be included in the DC offset determination.
 */
bool svkCorrectDCOffset::WasKSpacePtSampled( vtkFloatArray* spectrum, int numPts )
{

    bool wasSampled = false; 

    float* specPtr = spectrum->GetPointer(0);

    int increment = 2;  

    for (int i = 1; i < numPts; i+=increment) {
        if ( specPtr[i] != 0 || specPtr[i-1] != 0 ) {
            wasSampled = true; 
            break; 
        }
        increment *= 2;
    }

    return wasSampled; 

} 


/*!
 *
 */
void svkCorrectDCOffset::SetProvenance()
{
    this->GetImageDataInput(0)->GetProvenance()->AddAlgorithm( this->GetClassName() );
}


/*!
 *
 */
int svkCorrectDCOffset::FillInputPortInformation( int vtkNotUsed(port), vtkInformation* info )
{
    info->Set(vtkDataObject::DATA_TYPE_NAME(), "svkMrsImageData");
    return 1;
}


/*!
 *
 */
int svkCorrectDCOffset::FillOutputPortInformation( int vtkNotUsed(port), vtkInformation* info )
{
    info->Set(vtkDataObject::DATA_TYPE_NAME(), "svkMrsImageData");
    return 1;
}
