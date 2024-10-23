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
 *      Beck Olson,
 */


#include </usr/include/vtk/vtkInformation.h>
#include </usr/include/vtk/vtkInformationVector.h>
#include </usr/include/vtk/vtkStreamingDemandDrivenPipeline.h>

#include <svkMrsSingleVoxelSincExtraction.h>
#include <math.h>
#include <stdio.h>
#include <string.h>


using namespace svk;


//vtkCxxRevisionMacro(svkMrsSingleVoxelSincExtraction, "$Rev$");
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

    this->SetNumberOfInputPorts(5);
    bool required = true;
    this->GetPortMapper()->InitializeInputPort( INPUT_SPECTRA,   "INPUT_SPECTRA", svkAlgorithmPortMapper::SVK_MRS_IMAGE_DATA);
    this->GetPortMapper()->InitializeInputPort( L_COORDINATE, "L_COORDINATE", svkAlgorithmPortMapper::SVK_DOUBLE);
    this->GetPortMapper()->InitializeInputPort( P_COORDINATE, "P_COORDINATE", svkAlgorithmPortMapper::SVK_DOUBLE);
    this->GetPortMapper()->InitializeInputPort( S_COORDINATE, "S_COORDINATE", svkAlgorithmPortMapper::SVK_DOUBLE);
    this->GetPortMapper()->InitializeInputPort( RETAIN_INPUT_EXTENT, "RETAIN_INPUT_EXTENT", svkAlgorithmPortMapper::SVK_BOOL, !required);
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
 * Utility setter;
 */
void svkMrsSingleVoxelSincExtraction::SetVoxelCenter(double l_coordinate, double p_coordinate, double s_coordinate)
{
    this->GetPortMapper()->SetDoubleInputPortValue( L_COORDINATE, l_coordinate);
    this->GetPortMapper()->SetDoubleInputPortValue( P_COORDINATE, p_coordinate);
    this->GetPortMapper()->SetDoubleInputPortValue( S_COORDINATE, s_coordinate);
}


/*!
 *  Sets to true and the output will have the same extent as the input, but the same data in every
 *  voxel equal to the requpested lps voxel center.
 */
void svkMrsSingleVoxelSincExtraction::SetRetainInputExtent(bool retainInputExtent)
{
    this->GetPortMapper()->SetBoolInputPortValue( RETAIN_INPUT_EXTENT, retainInputExtent);
}


/*!
 *  Always produces single voxel output
 */
int svkMrsSingleVoxelSincExtraction::RequestInformation( vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector )
{
    svkBool* retainInputExtentBool = this->GetPortMapper()->GetBoolInputPortValue(RETAIN_INPUT_EXTENT);
    bool retainInputExtent = false;

    if( this->GetPortMapper()->GetBoolInputPortValue(RETAIN_INPUT_EXTENT) == NULL ) {
        this->SetRetainInputExtent( retainInputExtent);
    } else {
        retainInputExtent = retainInputExtentBool->GetValue();
    }

    int extent[6] = {0,1,0,1,0,1};
    double inSpacing[3]; 
    this->GetImageDataInput(0)->GetSpacing( inSpacing );
    
    double outSpacing[3]; 
    for (int i = 0; i < 3; i++) {
        outSpacing[i] = inSpacing[i];
    }

    svkDouble* l_coordinate =  this->GetPortMapper()->GetDoubleInputPortValue( L_COORDINATE );
    svkDouble* p_coordinate =  this->GetPortMapper()->GetDoubleInputPortValue( P_COORDINATE );
    svkDouble* s_coordinate =  this->GetPortMapper()->GetDoubleInputPortValue( S_COORDINATE );
    double outOrigin[3] = {l_coordinate->GetValue(), p_coordinate->GetValue(), s_coordinate->GetValue()};
    if( retainInputExtent ) {
        this->GetImageDataInput(0)->GetExtent( extent );
        this->GetImageDataInput(0)->GetDcmHeader()->GetOrigin( outOrigin );
    }

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
    int dataType = VTK_FLOAT;
    if( this->GetImageDataInput(0) != NULL &&  this->GetImageDataInput(0)->GetCellData()->GetArray(0) != NULL ) {
        numComponents = this->GetImageDataInput(0)->GetCellData()->GetArray(0)->GetNumberOfComponents();
        dataType = this->GetImageDataInput(0)->GetCellData()->GetArray(0)->GetDataType();
    }
    int* extent = data->GetExtent();
    float pi =vtkMath::Pi();
    svkDouble* l_coordinate =  this->GetPortMapper()->GetDoubleInputPortValue( L_COORDINATE );
    svkDouble* p_coordinate =  this->GetPortMapper()->GetDoubleInputPortValue( P_COORDINATE );
    svkDouble* s_coordinate =  this->GetPortMapper()->GetDoubleInputPortValue( S_COORDINATE );
    double voxelCenter[3] = { l_coordinate->GetValue(), p_coordinate->GetValue(), s_coordinate->GetValue() } ;

    double voxelIndex[3];
    // This method computes from the edge of voxel, we want to count the index from the centers.
    data->GetIndexFromPosition(voxelCenter, voxelIndex);
    voxelIndex[0] -= 0.5;
    voxelIndex[1] -= 0.5;
    voxelIndex[2] -= 0.5;

    // Create a vector to hold the interpolated results
    svkDcmHeader::DimensionVector resultDimVector = this->GetOutput()->GetDcmHeader()->GetDimensionIndexVector();
    svkDcmHeader::SetDimensionVectorValue( &resultDimVector, svkDcmHeader::COL_INDEX, 0);
    svkDcmHeader::SetDimensionVectorValue( &resultDimVector, svkDcmHeader::ROW_INDEX, 0);
    svkDcmHeader::SetDimensionVectorValue( &resultDimVector, svkDcmHeader::SLICE_INDEX, 0);
    int numResultsArrays = svkDcmHeader::GetNumberOfCells( &resultDimVector );
    vector<vtkDataArray*> resultArrays;
    for( int i = 0; i < numResultsArrays; i++ ) {
        vtkDataArray* array = vtkDataArray::CreateDataArray(dataType);
        array->SetNumberOfComponents( numComponents );
        array->SetNumberOfTuples( numFrequencyPoints );
        for( int j = 0; j < numComponents; j++ ) {
            array->FillComponent(j, 0);
        }
        resultArrays.push_back(array);
    }

    svkDcmHeader::DimensionVector inDimVector = this->GetOutput()->GetDcmHeader()->GetDimensionIndexVector();
    svkDcmHeader::DimensionVector loopVector = inDimVector;
    svkDcmHeader::DimensionVector loopResultDimVector;
    int numInputCells = svkDcmHeader::GetNumberOfCells( &inDimVector );
    for (int cellID = 0; cellID < numInputCells; cellID++) {
        svkDcmHeader::GetDimensionVectorIndexFromCellID(&inDimVector, &loopVector, cellID);
        int x = svkDcmHeader::GetDimensionVectorValue( &loopVector, svkDcmHeader::COL_INDEX );
        int y = svkDcmHeader::GetDimensionVectorValue( &loopVector, svkDcmHeader::ROW_INDEX );
        int z = svkDcmHeader::GetDimensionVectorValue( &loopVector, svkDcmHeader::SLICE_INDEX );
        vtkDataArray* spectrum = data->GetSpectrum( cellID );
        loopResultDimVector = loopVector;
        svkDcmHeader::SetDimensionVectorValue( &loopResultDimVector, svkDcmHeader::COL_INDEX, 0);
        svkDcmHeader::SetDimensionVectorValue( &loopResultDimVector, svkDcmHeader::ROW_INDEX, 0);
        svkDcmHeader::SetDimensionVectorValue( &loopResultDimVector, svkDcmHeader::SLICE_INDEX, 0);
        int interpIndex = svkDcmHeader::GetCellIDFromDimensionVectorIndex(&resultDimVector, &loopResultDimVector);
        float piDeltaX = pi*(voxelIndex[0] - x);
        float piDeltaY = pi*(voxelIndex[1] - y);
        float piDeltaZ = pi*(voxelIndex[2] - z);
        for(int freq = 0; freq < numFrequencyPoints; freq++ ) {
            float* value = new float[numComponents];
            for(int comp = 0; comp < numComponents; comp++ ) {
                value[comp]= spectrum->GetTuple(freq)[comp];
                //Add to this frequency the spatial since for the current voxel  the current intensity.
                if( piDeltaX != 0 ) {
                    value[comp] *= sin(static_cast<double>(piDeltaX))/piDeltaX;
                }
                if( piDeltaY != 0 ) {
                    value[comp] *= sin(static_cast<double>(piDeltaY))/piDeltaY;
                }
                if( piDeltaZ != 0 ) {
                    value[comp] *= sin(static_cast<double>(piDeltaZ))/piDeltaZ;
                }
                value[comp] += resultArrays[interpIndex]->GetTuple(freq)[comp];
            }
            resultArrays[interpIndex]->SetTuple( freq, value );
            delete value;
        }
    }
    svkBool* retainInputExtentBool = this->GetPortMapper()->GetBoolInputPortValue(RETAIN_INPUT_EXTENT);
    svkDcmHeader::DimensionVector outDimVector = this->GetOutput()->GetDcmHeader()->GetDimensionIndexVector();
    if( retainInputExtentBool != NULL && retainInputExtentBool->GetValue() == false) {
        svkDcmHeader::SetDimensionVectorValue( &outDimVector, svkDcmHeader::COL_INDEX, 0);
        svkDcmHeader::SetDimensionVectorValue( &outDimVector, svkDcmHeader::ROW_INDEX, 0);
        svkDcmHeader::SetDimensionVectorValue( &outDimVector, svkDcmHeader::SLICE_INDEX, 0);
        svkMrsImageData::SafeDownCast(this->GetOutput())->Redimension( &outDimVector, voxelCenter, data->GetSpacing(), true );
    }

    loopVector = outDimVector;
    int numOutputCells = svkDcmHeader::GetNumberOfCells( &outDimVector );
    for (int cellID = 0; cellID < numOutputCells; cellID++) {
        svkDcmHeader::GetDimensionVectorIndexFromCellID(&outDimVector, &loopVector, cellID);
        loopResultDimVector = loopVector;
        svkDcmHeader::SetDimensionVectorValue( &loopResultDimVector, svkDcmHeader::COL_INDEX, 0);
        svkDcmHeader::SetDimensionVectorValue( &loopResultDimVector, svkDcmHeader::ROW_INDEX, 0);
        svkDcmHeader::SetDimensionVectorValue( &loopResultDimVector, svkDcmHeader::SLICE_INDEX, 0);
        int interpIndex = svkDcmHeader::GetCellIDFromDimensionVectorIndex(&resultDimVector, &loopResultDimVector);
        svkMrsImageData::SafeDownCast(this->GetOutput())->GetSpectrum( cellID )->DeepCopy(resultArrays[interpIndex]);
    }

    for( int i = 0; i < resultArrays.size(); i++ ) {
        resultArrays[i]->Delete();
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

