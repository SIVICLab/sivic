/*
 *  Copyright © 2009-2014 The Regents of the University of California.
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
 *      Beck Olson,
 *      Nonlinear LS fit added by: Christine Leon (Jul 28, 2012)
 */


#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkStreamingDemandDrivenPipeline.h>

#include <svkMrsSingleVoxelSincExtraction.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include <svkKineticModelCostFunction.h>
#include <svk2SiteExchangeCostFunction.h>
#include <svk2SitePerfCostFunction.h>


using namespace svk;


vtkCxxRevisionMacro(svkMrsSingleVoxelSincExtraction, "$Rev$");
vtkStandardNewMacro(svkMrsSingleVoxelSincExtraction);


/*!
 *
 */
svkMrsSingleVoxelSincExtraction::svkMrsSingleVoxelSincExtraction()
{
#if VTK_DEBUG_ON
    this->DebugOn();
#endif

    vtkDebugMacro(<< this->GetClassName() << "::" << this->GetClassName() << "()");

    this->SetNumberOfInputPorts(1);
    this->SetNumberOfOutputPorts(1);
}


/*!
 *
 */
svkMrsSingleVoxelSincExtraction::~svkMrsSingleVoxelSincExtraction()
{
    vtkDebugMacro(<<this->GetClassName()<<"::~"<<this->GetClassName());
}


/*!
 *  Always produces single voxel output
 */
int svkMrsSingleVoxelSincExtraction::RequestInformation( vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector )
{

    int extent[6] = {0,1,0,1,0,1};
    double inSpacing[3]; 
    this->GetImageDataInput(0)->GetSpacing( inSpacing );
    
    double outSpacing[3]; 
    for (int i = 0; i < 3; i++) {
        outSpacing[i] = inSpacing[i];
    }

    double outOrigin[3];
    this->GetImageDataInput(0)->GetDcmHeader()->GetOrigin( outOrigin ); 

    vtkInformation *outInfo = outputVector->GetInformationObject(0);
    outInfo->Set(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(), extent, 6);
    outInfo->Set(vtkStreamingDemandDrivenPipeline::UPDATE_EXTENT(), extent, 6);
    outInfo->Set(vtkDataObject::SPACING(), outSpacing, 3);
    outInfo->Set(vtkDataObject::ORIGIN(), outOrigin, 3);

    return 1;
}


/*!
 *  Copy the Dcm Header and Provenance from the input to the output. 
 */
int svkMrsSingleVoxelSincExtraction::RequestData( vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector )
{

    /*
     * 1. Find the row/column/slice that the given point is within.
     * 2. Iterate through all dimensions summing the since contribution from each frequency point
     */
    this->GetOutput(0)->DeepCopy( this->GetImageDataInput(0));
    svkMrsImageData* data = svkMrsImageData::SafeDownCast(this->GetImageDataInput(0) );
    vtkCellData* cellData = data->GetCellData();
    int numFrequencyPoints = cellData->GetNumberOfTuples();
    int numChannels  = data->GetNumberOfChannels();
    int numTimePts = data->GetDcmHeader()->GetNumberOfTimePoints();
    // Should this be hard-coded?
    int numComponents = 2;
    int* extent = data->GetExtent();
    float pi =vtkMath::Pi();
    //TODO: Base the relative index on an input LPS coordinate, and find what its "index" would be.
    float voxelIndex[] = {5.5, 5.5, 5.5};
    float* specPtr = NULL;
    vtkFloatArray* array = vtkFloatArray::New();
    array->SetNumberOfComponents(2);
    array->SetNumberOfTuples( numFrequencyPoints );
    float* arrayPtr = array->GetPointer(0);
    for( int channel = 0; channel < numChannels; channel++ ) {
        for( int timePt = 0; timePt < numTimePts; timePt++ ) {
            for (int z = extent[4]; z < extent[5]; z++) {
                for (int y = extent[2]; y < extent[3]; y++) {
                    for (int x = extent[0]; x < extent[1]; x++) {
                        vtkFloatArray* spectrum = vtkFloatArray::SafeDownCast(data->GetSpectrum( x, y, z, timePt, channel ));
                        specPtr = spectrum->GetPointer(0);
                        float piDeltaX = vtkMath::Pi()*(voxelIndex[0] - x);
                        float piDeltaY = vtkMath::Pi()*(voxelIndex[1] - y);
                        float piDeltaZ = vtkMath::Pi()*(voxelIndex[2] - z);
                        for(int fc = 0; fc < numComponents*numFrequencyPoints; fc++ ) {
                            float value = specPtr[fc];
                            //Add to this frequencey the spatial since for the current voxel  the current intensity.
                            if( piDeltaX != 0 ) {
                                value *= sin(piDeltaX)/piDeltaX;
                            }
                            if( piDeltaY != 0 ) {
                                value *= sin(piDeltaY)/piDeltaY;
                            }
                            if( piDeltaZ != 0 ) {
                                value *= sin(piDeltaZ)/piDeltaZ;
                            }
                            arrayPtr[fc] += value;
                        }
                    }
                }
            }
        }
    }
    for( int channel = 0; channel < numChannels; channel++ ) {
        for( int timePt = 0; timePt < numTimePts; timePt++ ) {
            for (int z = extent[4]; z < extent[5]; z++) {
                for (int y = extent[2]; y < extent[3]; y++) {
                    for (int x = extent[0]; x < extent[1]; x++) {
                        svkMrsImageData::SafeDownCast(this->GetOutput())->GetSpectrum( x, y, z, timePt, channel )->DeepCopy(array);
                    }
                }
            }
        }
    }

    return 1; 
}


int svkMrsSingleVoxelSincExtraction::FillInputPortInformation( int port, vtkInformation* info )
{
    if ( port == 0 ) {
        info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "svkMrsImageData");
    } 
    return 1;
}



/*!
 *  Output from this algo is an svkMriImageData object. 
 */
int svkMrsSingleVoxelSincExtraction::FillOutputPortInformation( int vtkNotUsed(port), vtkInformation* info )
{
    info->Set( vtkDataObject::DATA_TYPE_NAME(), "svkMrsImageData");
    return 1;
}

