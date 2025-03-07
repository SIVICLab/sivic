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



#include <svkMrsLinearPhase.h>


using namespace svk;


//vtkCxxRevisionMacro(svkMrsLinearPhase, "$Rev$");
vtkStandardNewMacro(svkMrsLinearPhase);


svkMrsLinearPhase::svkMrsLinearPhase()
{

#if VTK_DEBUG_ON
    this->DebugOn();
#endif

    vtkDebugMacro(<<this->GetClassName() << "::" << this->GetClassName() << "()");
    this->shiftWindow[0] = 0;
    this->shiftWindow[1] = 0;
    this->shiftWindow[2] = 0;
}


/*!
 *  Destructor. 
 */
svkMrsLinearPhase::~svkMrsLinearPhase()
{
}


/*!
 *  Set an additional phase shift for example to voxel shift data.
 */
void svkMrsLinearPhase::SetShiftWindow( double shiftWindow[3] )
{
    this->shiftWindow[0] = shiftWindow[0];
    this->shiftWindow[1] = shiftWindow[1];
    this->shiftWindow[2] = shiftWindow[2];
}


/*! 
 * Defines the output extent.
 */
int svkMrsLinearPhase::RequestInformation( vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector )
{
    return 1;
}


/*!
 * Primary execution method. If the number of target points is greater than the number of points
 * in the dataset then it will be padded in the spectral domain. If the output extent is greater
 * than the extent of the input data then it will pad the spatial domain.
 */
int svkMrsLinearPhase::RequestData( vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector )
{
    svkMrsImageData* data = svkMrsImageData::SafeDownCast(this->GetImageDataInput(0));
    int numVoxels[3];

    data->GetNumberOfVoxels(numVoxels);

    int numFrequencyPoints = data->GetCellData()->GetNumberOfTuples();
    int numChannels  = data->GetNumberOfChannels();
    int numTimePts = data->GetDcmHeader()->GetNumberOfTimePoints();
    int numDimensions = 3;
    int tlcVoxel[3] = {0,0,0};
    int brcVoxel[3] = {0,0,0};
    data->Get3DVoxelsInSelectionBox(tlcVoxel, brcVoxel, 0.0);

    for( int channel = 0; channel < numChannels; channel++ ) {
        for( int timePt = 0; timePt < numTimePts; timePt++ ) {

            for( int dim = 0; dim < numDimensions; dim++) { // Apply the phase shift in each dimension

                // Let's create the phase array for this dimension
                vtkImageComplex* phaseArray = new vtkImageComplex[ numVoxels[dim] ];
                svkSpecUtils::CreateLinearPhaseShiftArray(numVoxels[dim], phaseArray, this->shiftWindow[dim]);

                for (int z = 0; z < numVoxels[2]; z++) {
                    for (int y = 0; y < numVoxels[1]; y++) {
                        for (int x = 0; x < numVoxels[0]; x++) {
                            // Lets determine the index for our phase array. This depends on the dimension we are phasing
                            int phaseArrayIndex = -1;
                            if( dim == 0 ) {
                                phaseArrayIndex = x;
                            } else if( dim == 1 ) {
                                phaseArrayIndex = y;
                            } else if( dim == 2 ) {
                                phaseArrayIndex = z;
                            }

                            // Lets get our phase value
                            vtkImageComplex phaseValue = phaseArray[phaseArrayIndex];

                            // Lets get the spectrum for this voxel

                            vtkFloatArray* spectrum = vtkFloatArray::SafeDownCast(data->GetSpectrum( x, y, z, timePt, channel ));
                            float* specPtr = spectrum->GetPointer(0);

                            // THis will hold the current value during the calculation
                            vtkImageComplex currentValue;
                            // THis will hold the new value during the calculation
                            vtkImageComplex newValue;
                            for (int point = 0; point < numFrequencyPoints; point++) {

                                // Let's grab the current values and store them so we don't overwrite
                                currentValue.Real = specPtr[ 2*point ];
                                currentValue.Imag = specPtr[ 2*point + 1 ];

                                // And apply the phase value
                                newValue.Real = ( phaseValue.Real*currentValue.Real - phaseValue.Imag*currentValue.Imag );
                                newValue.Imag = ( phaseValue.Real*currentValue.Imag + phaseValue.Imag*currentValue.Real );




/*
                                if( x > tlcVoxel[0]
                                    && y > tlcVoxel[1]
                                    && z > tlcVoxel[2]
                                    && x < brcVoxel[0]
                                    && y < brcVoxel[1]
                                    && z < brcVoxel[2]
                                                     ){
                                    specPtr[2*point] =  1;
                                    specPtr[2*point + 1] = 1;
                                    if( point == numFrequencyPoints/2 ) {
                                        specPtr[2*point] =  2;
                                        specPtr[2*point + 1] = 2;
                                    }
                                } else {
                                    specPtr[2*point] = 0;
                                    specPtr[2*point + 1] = 0;

                                }
*/






                                // And set the result back
                                specPtr[2*point] = newValue.Real;
                                specPtr[2*point + 1] = newValue.Imag;
                            }
                        }
                    }
                }

                delete[] phaseArray;
            }
        }
    }

    return 1;
}

/*!
 * Define required input data type.
 */
int svkMrsLinearPhase::FillInputPortInformation( int vtkNotUsed(port), vtkInformation* info )
{
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "svkMrsImageData"); 
    return 1;
}
