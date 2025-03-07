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



#include <svkMrsZeroFill.h>


using namespace svk;


//vtkCxxRevisionMacro(svkMrsZeroFill, "$Rev$");
vtkStandardNewMacro(svkMrsZeroFill);


svkMrsZeroFill::svkMrsZeroFill()
{

#if VTK_DEBUG_ON
    this->DebugOn();
#endif

    vtkDebugMacro(<<this->GetClassName() << "::" << this->GetClassName() << "()");

    this->numSpecPoints = 0;
    this->outputWholeExtent[0] = -1;
    this->outputWholeExtent[1] = -1;
    this->outputWholeExtent[2] = -1;
    this->outputWholeExtent[3] = -1;
    this->outputWholeExtent[4] = -1;
    this->outputWholeExtent[5] = -1;

    
    // These will store the fill type. This is used in case the user defines the fill
    // type before setting the input. The extent is computed on RequestInformation.
    rowFillType = VALUE;
    columnFillType = VALUE;
    sliceFillType = VALUE;
    specFillType = VALUE;

}


/*!
 *  Destructor. 
 */
svkMrsZeroFill::~svkMrsZeroFill()
{
}


/*!
 * Initializes the output extent to match the input extent for non-initialized indecies.
 */
void svkMrsZeroFill::InitializeOutputWholeExtent() 
{
    svkMrsImageData* data = svkMrsImageData::SafeDownCast(this->GetImageDataInput(0)); 
    int* inExtent = data->GetExtent();
    this->outputWholeExtent[0] = this->outputWholeExtent[0] == -1 ? inExtent[0]:this->outputWholeExtent[0];
    this->outputWholeExtent[1] = this->outputWholeExtent[1] == -1 ? inExtent[1]:this->outputWholeExtent[1];
    this->outputWholeExtent[2] = this->outputWholeExtent[2] == -1 ? inExtent[2]:this->outputWholeExtent[2];
    this->outputWholeExtent[3] = this->outputWholeExtent[3] == -1 ? inExtent[3]:this->outputWholeExtent[3];
    this->outputWholeExtent[4] = this->outputWholeExtent[4] == -1 ? inExtent[4]:this->outputWholeExtent[4];
    this->outputWholeExtent[5] = this->outputWholeExtent[5] == -1 ? inExtent[5]:this->outputWholeExtent[5];

}


/*! 
 *  Sets the total number of spectral points after zero-filling. 
 *  If the number of points is less than the current number of 
 *  spectral points the algorithm will do nothing.    
 */
void svkMrsZeroFill::SetNumberOfSpecPoints(int numSpecPoints)
{
    this->numSpecPoints = numSpecPoints; 
}


void svkMrsZeroFill::SetNumberOfSpecPointsToDouble( )
{
    this->specFillType = DOUBLE;
}

void svkMrsZeroFill::SetNumberOfSpecPointsToNextPower2( )
{
    this->specFillType = POWER2;
}

void svkMrsZeroFill::SetNumberOfRows(int numRows)
{
    this->outputWholeExtent[2] = 0; 
    this->outputWholeExtent[3] = numRows; 
}

void svkMrsZeroFill::SetNumberOfRowsToDouble( )
{
    this->rowFillType = DOUBLE;
}

void svkMrsZeroFill::SetNumberOfRowsToNextPower2( )
{
    this->rowFillType = POWER2;
}

void svkMrsZeroFill::SetNumberOfColumns(int numColumns)
{
    this->outputWholeExtent[0] = 0; 
    this->outputWholeExtent[1] = numColumns; 
}

void svkMrsZeroFill::SetNumberOfColumnsToDouble( )
{
    this->columnFillType = DOUBLE;
}

void svkMrsZeroFill::SetNumberOfColumnsToNextPower2( )
{
    this->columnFillType = POWER2;
}

void svkMrsZeroFill::SetNumberOfSlices(int numSlices)
{
    this->outputWholeExtent[4] = 0; 
    this->outputWholeExtent[5] = numSlices; 
}

void svkMrsZeroFill::SetNumberOfSlicesToDouble( )
{
    this->sliceFillType = DOUBLE;
}

void svkMrsZeroFill::SetNumberOfSlicesToNextPower2( )
{
    this->sliceFillType = POWER2;
}


/*!
 * The output whole extent defines the extent to which you want to pad.
 * If it is smaller than the extent of the input data then the input
 * data will not be modified.
 */
void svkMrsZeroFill::SetOutputWholeExtent(int extent[6])
{
    int modified = 0;
    
    for ( int i = 0; i < 6; i++ ) {
        if (this->outputWholeExtent[i] != extent[i]) {
            this->outputWholeExtent[i] = extent[i];
            modified = 1;
        }
    }

    if ( modified ) {
        this->Modified();
    }
}


/*!
 * The output whole extent defines the extent to which you want to pad.
 * If it is smaller than the extent of the input data then the input
 * data will not be modified.
 */
void svkMrsZeroFill::SetOutputWholeExtent(int minX, int maxX, int minY, int maxY, int minZ, int maxZ)
{
    int extent[6];

    extent[0] = minX;  extent[1] = maxX;
    extent[2] = minY;  extent[3] = maxY;
    extent[4] = minZ;  extent[5] = maxZ;
    this->SetOutputWholeExtent(extent);
}


/*! 
 * Defines the output extent.
 */
int svkMrsZeroFill::RequestInformation( vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector )
{
    // get the info objects 
    vtkInformation* outInfo = outputVector->GetInformationObject(0);
    vtkInformation* inInfo = inputVector[0]->GetInformationObject(0);

    svkMrsImageData* data = svkMrsImageData::SafeDownCast(this->GetImageDataInput(0)); 

    this->InitializeOutputWholeExtent();

    // Lets check to see if any of the fill types have been defined
    
    // Check Number of Spec Points
    int numSpecPts  = data->GetDcmHeader()->GetIntValue( "DataPointColumns" );
    if( this->specFillType == DOUBLE ) {
        this->SetNumberOfSpecPoints( numSpecPts * 2 );
    } else if ( this->specFillType == POWER2 ) {
        this->SetNumberOfSpecPoints( svkUtils::GetNextPower2(numSpecPts) );
    }


    // Check Number of Columns
    int cols = data->GetDcmHeader()->GetIntValue( "Columns" );
    if( this->columnFillType == DOUBLE ) {
        this->SetNumberOfColumns( cols * 2 );
    } else if ( this->columnFillType == POWER2 ) {
        this->SetNumberOfColumns( svkUtils::GetNextPower2( cols ) );
    }

    // Check Number of Rows
    int rows = data->GetDcmHeader()->GetIntValue( "Rows" );
    if( this->rowFillType == DOUBLE ) {
        this->SetNumberOfRows( rows * 2 );
    } else if ( this->rowFillType == POWER2 ) {
        this->SetNumberOfRows( svkUtils::GetNextPower2( rows ) );
    }

    // Check Number of Slices
    int slices = data->GetDcmHeader()->GetNumberOfSlices();
    if( this->sliceFillType == DOUBLE ) {
        this->SetNumberOfSlices( slices * 2 );
    } else if ( this->sliceFillType == POWER2 ) {
        this->SetNumberOfSlices( svkUtils::GetNextPower2( slices ) );
    }

    outInfo->Set(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(), this->outputWholeExtent, 6);

    double newSpacing[3] = {0,0,0};
    int oldNumVoxels[3];
    data->GetNumberOfVoxels(oldNumVoxels);
    int newNumVoxels[3];
    newNumVoxels[0] =  this->outputWholeExtent[1] - this->outputWholeExtent[0];
    newNumVoxels[1] =  this->outputWholeExtent[3] - this->outputWholeExtent[2];
    newNumVoxels[2] =  this->outputWholeExtent[5] - this->outputWholeExtent[4];
    double spacing[3];
    data->GetSpacing(spacing);
    newSpacing[0] = spacing[0]*((double)(oldNumVoxels[0] ))/(newNumVoxels[0] );
    newSpacing[1] = spacing[1]*((double)(oldNumVoxels[1] ))/(newNumVoxels[1] );
    newSpacing[2] = spacing[2]*((double)(oldNumVoxels[2] ))/(newNumVoxels[2] );
    outInfo->Set(vtkDataObject::SPACING(), newSpacing, 3);


    return 1;
}


/*!
 * Primary execution method. If the number of target points is greater than the number of points
 * in the dataset then it will be padded in the spectral domain. If the output extent is greater
 * than the extent of the input data then it will pad the spatial domain.
 */
int svkMrsZeroFill::RequestData( vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector )
{
    int returnSpatial = 1;
    int returnSpectral = 1;
    svkMrsImageData* data = svkMrsImageData::SafeDownCast(this->GetImageDataInput(0)); 
    int numPts = data->GetDcmHeader()->GetIntValue("DataPointColumns");
    int* inExtent = data->GetExtent();
    if( this->numSpecPoints > numPts ) {
        returnSpectral = this->RequestDataSpectral( request, inputVector, outputVector );
    } 
    
    if(   this->outputWholeExtent[1] > inExtent[1]   
       || this->outputWholeExtent[3] > inExtent[3] 
       || this->outputWholeExtent[5] > inExtent[5] ) { 
        returnSpatial = this->RequestDataSpatial( request, inputVector, outputVector );
    }
    ostringstream progressStream;
    progressStream <<"Zero Fill Complete";
    this->SetProgressText( progressStream.str().c_str() );
    this->UpdateProgress( 0 );
    return (returnSpatial && returnSpectral);
}


/*!
 * Zero Fills in the spatial domain.
 */
int svkMrsZeroFill::RequestDataSpatial( vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector )
{
    svkMrsImageData* data = svkMrsImageData::SafeDownCast(this->GetImageDataInput(0)); 

    // Lets get the input dataset's parameters
    int numChannels = data->GetDcmHeader()->GetNumberOfCoils();
    int numTimePts = data->GetDcmHeader()->GetNumberOfTimePoints();
    int numberOfPoints = data->GetCellData()->GetArray(0)->GetNumberOfTuples();
    double origin[3];
    int* inExtent = data->GetExtent();
    int oldNumVoxels[3];
    oldNumVoxels[0] =  inExtent[1] - inExtent[0];
    oldNumVoxels[1] =  inExtent[3] - inExtent[2];
    oldNumVoxels[2] =  inExtent[5] - inExtent[4];


    // Now let's define the new spatial extent/number of voxels
    int newNumVoxels[3];
    newNumVoxels[0] =  this->outputWholeExtent[1] - this->outputWholeExtent[0];
    newNumVoxels[1] =  this->outputWholeExtent[3] - this->outputWholeExtent[2];
    newNumVoxels[2] =  this->outputWholeExtent[5] - this->outputWholeExtent[4];

  
    // Lets determine the new number of frames for the dataset 
    int newNumFrames = newNumVoxels[2] * ( numTimePts ) * ( numChannels );

    // We need a temporary dataset to to copy the results to
    svkMrsImageData* outputData = svkMrsImageData::New();
    outputData->ShallowCopy( data ); 
    outputData->GetDcmHeader()->SetValue("Columns", newNumVoxels[0]);
    outputData->GetDcmHeader()->SetValue("Rows", newNumVoxels[1]);
    outputData->GetDcmHeader()->SetValue("NumberOfFrames", newNumFrames);
    data->GetDcmHeader()->GetOrigin(origin);

    // New we need to know how to transate the extent. This is how input voxels are mapped to output voxels.
    int extentTranslation[3] = {0,0,0};

    for( int i = 0; i <3; i++ ) {
        if( newNumVoxels[i] % 2 == 1 && oldNumVoxels[i] % 2 == 0 ) {
            extentTranslation[i] = (newNumVoxels[i] - oldNumVoxels[i])/2;
        } else {
            extentTranslation[i] = vtkMath::Round((newNumVoxels[i] - oldNumVoxels[i])/2.0);
        }
    }

    //Adjust Spacing And Origin
    double* spacing = data->GetSpacing();
    double newSpacing[3] = {0,0,0};
    newSpacing[0] = spacing[0]*((double)(oldNumVoxels[0] ))/(newNumVoxels[0] );
    newSpacing[1] = spacing[1]*((double)(oldNumVoxels[1] ))/(newNumVoxels[1] );
    newSpacing[2] = spacing[2]*((double)(oldNumVoxels[2] ))/(newNumVoxels[2] );
    ostringstream* oss = new ostringstream();
    *oss << newSpacing[0];
    string pixelSpacingString( oss->str() + "\\" );
    delete oss;

    oss = new ostringstream();
    *oss << newSpacing[1];
    pixelSpacingString.append( oss->str() );

    delete oss;
    oss = new ostringstream();
    *oss << newSpacing[2];
    string sliceThicknessString( oss->str() );
    delete oss;

    double dcos[3][3];
    data->GetDcos( dcos );
    double newOrigin[3] = { origin[0], origin[1], origin[2] };

    //  this is in space, not a fractional shift
    double originShift[3] = { 0, 0, 0 };
    originShift[0] = newSpacing[0]/2 - spacing[0]/2;
    originShift[1] = newSpacing[1]/2 - spacing[1]/2;
    originShift[2] = newSpacing[2]/2 - spacing[2]/2;

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            newOrigin[i] += (originShift[j]) * dcos[j][i];
        }
    }

    // And construct the output target's spatial parameters.
    outputData->GetDcmHeader()->InitPixelMeasuresMacro( pixelSpacingString, sliceThicknessString );

    svkDcmHeader::DimensionVector dimensionVector = data->GetDcmHeader()->GetDimensionIndexVector();
    svkDcmHeader::SetDimensionVectorValue(&dimensionVector, svkDcmHeader::SLICE_INDEX, newNumVoxels[2]-1);
    svkDcmHeader::SetDimensionVectorValue(&dimensionVector, svkDcmHeader::TIME_INDEX, numTimePts-1);
    svkDcmHeader::SetDimensionVectorValue(&dimensionVector, svkDcmHeader::CHANNEL_INDEX, numChannels-1);
    outputData->GetDcmHeader()->InitPerFrameFunctionalGroupSequence( newOrigin, newSpacing, dcos, &dimensionVector); 

    outputData->SyncVTKImageDataToDcmHeader();
    outputData->Modified();

    // Lastly let's fill the new dataset with arrays of value zero in all components.
    this->InitializeDataArrays( outputData );

    svkDcmHeader* hdr = data->GetDcmHeader();
    int numSpecPts    = hdr->GetIntValue( "DataPointColumns" );
    int cols          = hdr->GetIntValue( "Columns" );
    int rows          = hdr->GetIntValue( "Rows" );
    int slices        = hdr->GetNumberOfSlices();

    // We need to scale the image by the updated size
    // This is because the FFT will scale up by the number of points
    // and the IFFT divides by it.
    double scale = ((double)(newNumVoxels[0]*newNumVoxels[1]*newNumVoxels[2]))
                   / ((double)(oldNumVoxels[0]*oldNumVoxels[1]*oldNumVoxels[2]));

    char arrayName[30];
    double progress = 0;
    for( int channel = 0; channel < numChannels; channel++ ) { 
        for( int timePt = 0; timePt < numTimePts; timePt++ ) { 
            ostringstream progressStream;
            progressStream <<"Executing Spatial Zero Fill for Time Point " << timePt+1 << "/"
                           << numTimePts << " and Channel: " << channel+1 << "/" << numChannels;
            this->SetProgressText( progressStream.str().c_str() );
            for (int z = 0; z < slices; z++) {
                for (int y = 0; y < rows; y++) {
                    for (int x = 0; x < cols; x++) {
                        progress = ((x+1)*(y+1)*(z+1))/((double)slices*rows*cols);
                        this->UpdateProgress( progress );
                        vtkDataArray* spectrum = data->GetSpectrum(x, y, z, timePt, channel);
                        vtkDataArray* result = outputData->GetSpectrum(x + extentTranslation[0], y + extentTranslation[1], z + extentTranslation[2], timePt, channel);
                        int numPoints = result->GetNumberOfTuples();
                        int numComponents = result->GetNumberOfComponents();

                        // TODO: This math could be changed to pointer math for speed
                        for( int component = 0; component < numComponents; component++ ) {
                            for( int point = 0; point < numPoints; point++ ) {
                                result->SetComponent(point, component, scale*spectrum->GetComponent(point,component));
                            }
                        }

                    }
                }
            }
        }
    }
    this->UpdateProgress( 0 );
    
    // Since we have moved the origin of the data, we need to shift the content.
    double shiftWindow[3] = { 0, 0, 0 };
    shiftWindow[0] = -originShift[0]/newSpacing[0];
    shiftWindow[1] = -originShift[1]/newSpacing[1];
    shiftWindow[2] = -originShift[2]/newSpacing[2];

    string k0Sampled = data->GetDcmHeader()->GetStringValue( "SVK_K0Sampled");
    if (this->GetDebug()) {
        cout << "K0Sampled: " << k0Sampled << endl;
    }
    for ( int i = 0; i < 3; i++ ) {
        shiftWindow[i] = -1 * ( newSpacing[i] - spacing[i] ) / ( 2 * newSpacing[i] ); 
        //  if k0 not sampled, apply the additional phase shift with respect to the original voxel 
        //  size as a fraction of the new smaller voxel size:
        if ( k0Sampled.compare( "NO" ) == 0 ) {
            shiftWindow[i] -= spacing[i] /  (2 * newSpacing[i] ); 
        }
    }

    //  Set k0 sampled to true if necessary:  
    //      The phase correction from the original sampling 
    //      has been accounted for now, so 
    //      change the value of k0 sampled, otherwise
    //      the data will get shifted again by the FFT
    if ( k0Sampled.compare( "NO" ) == 0 ) {
        outputData->GetDcmHeader()->SetValue( "SVK_K0Sampled", "YES" );
    }
    if (this->GetDebug()) {
        cout << " SHIFT WINDOW(MRSZF) :          " << shiftWindow[0] << " " 
            << shiftWindow[1] << " " << shiftWindow[2] << endl;
        cout << " SHIFT WINDOW(MRSZF) (PixSize) :" << newSpacing[0] << " " 
            << newSpacing[1] << " " << newSpacing[2] << endl;
    }
    // Apply a half voxel  difference phase shift. This is because the sampled points of the data has changed.
    svkMrsLinearPhase* linearShift = svkMrsLinearPhase::New();
    linearShift->SetShiftWindow( shiftWindow );
    linearShift->SetInputData( outputData );
    linearShift->Update();


    //outputData->SyncVTKImageDataToDcmHeader();


    // Since this algorithm is in place we need to copy the result back to the source
    data->ShallowCopy(outputData);


    data->SyncVTKImageDataToDcmHeader();
    linearShift->Delete();
    outputData->Delete();
    return 1; 

}


/*!
 * Zero Fills in the spectral domain.
 */
int svkMrsZeroFill::RequestDataSpectral( vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector )
{

    svkMrsImageData* data = svkMrsImageData::SafeDownCast(this->GetImageDataInput(0)); 

    svkDcmHeader* hdr = data->GetDcmHeader();
    int numSpecPts    = hdr->GetIntValue( "DataPointColumns" );

    // If the numSpecPoints is < current dimensionality, do nothing:
    if ( this->numSpecPoints <= numSpecPts ) { 
        return 0; // is this the correct return value ?
    }

    float cmplxPt[2];
    cmplxPt[0] = 0.;
    cmplxPt[1] = 0.;
	vtkDataArray* spectrum = NULL;
    double progress = 0;

    //  Get the Dimension Index and index values  
    svkDcmHeader::DimensionVector dimensionVector = data->GetDcmHeader()->GetDimensionIndexVector();

    //  GetNumber of cells in the image:
    int numCells = svkDcmHeader::GetNumberOfCells( &dimensionVector );

    // this adds a LOT of time to the execution if updated for every cell
    ostringstream progressStream;
    progressStream <<"Executing Spectral Zero Fill for " << numCells  ;
    this->SetProgressText( progressStream.str().c_str() );
    this->UpdateProgress( progress );

    for (int cellID = 0; cellID < numCells; cellID++ ) {


        progress = (cellID+1)/((double)numCells);

        spectrum = data->GetSpectrum( cellID ); 
        //  Iterate over frequency points in spectrum and apply phase correction:
        for ( int freq = numSpecPts; freq < this->numSpecPoints; freq++ ) {
            spectrum->InsertTuple(freq, cmplxPt);
        }
    }

    hdr->SetValue( "DataPointColumns", this->numSpecPoints);

    return 1; 
} 


/*!
 *
 */
void svkMrsZeroFill::ComputeInputUpdateExtent (int inExt[6], int outExt[6], int wholeExtent[6])
{
    // Clip
    for (int i = 0; i < 3; i++) {
        inExt[i*2] = outExt[i*2];
        inExt[i*2+1] = outExt[i*2+1];
        if (inExt[i*2] < wholeExtent[i*2]) {
          inExt[i*2] = wholeExtent[i*2];
        }
        if (inExt[i*2] > wholeExtent[i*2 + 1]) {
            inExt[i*2] = wholeExtent[i*2 + 1]; 
        }
        if (inExt[i*2+1] < wholeExtent[i*2]) {
            inExt[i*2+1] = wholeExtent[i*2]; 
        }
        if (inExt[i*2 + 1] > wholeExtent[i*2 + 1]) {
            inExt[i*2 + 1] = wholeExtent[i*2 + 1]; 
        }
    }
}   


/*!
 *
 */
int svkMrsZeroFill::RequestUpdateExtent (vtkInformation* vtkNotUsed(request), vtkInformationVector** inputVector, vtkInformationVector* outputVector)
{
    // get the info objects
    vtkInformation* outInfo = outputVector->GetInformationObject(0);
    vtkInformation* inInfo = inputVector[0]->GetInformationObject(0);
    
    int wholeExtent[6];
    int inExt[6];
    
    // handle XYZ
    inInfo->Get(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(),wholeExtent);
    outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_EXTENT(),inExt);
    
    this->ComputeInputUpdateExtent(inExt, inExt, wholeExtent);

    inInfo->Set(vtkStreamingDemandDrivenPipeline::UPDATE_EXTENT(),inExt,6);

    return 1;
}



/*!
 * Define required input data type.
 */
int svkMrsZeroFill::FillInputPortInformation( int vtkNotUsed(port), vtkInformation* info )
{
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "svkMrsImageData"); 
    return 1;
}


/*!
 * Prepares the output data for the new size of the data set. This will
 * kill any arrays present and replace them with zero-valued arrays.
 */
void svkMrsZeroFill::InitializeDataArrays( svkMrsImageData* outputData ) 
{
    int cols          = outputData->GetDcmHeader()->GetIntValue( "Columns" );
    int rows          = outputData->GetDcmHeader()->GetIntValue( "Rows" );
    int slices        = outputData->GetDcmHeader()->GetNumberOfSlices();
    int numChannels   = outputData->GetDcmHeader()->GetNumberOfCoils();
    int numTimePts    = outputData->GetDcmHeader()->GetNumberOfTimePoints();

    int numPts = outputData->GetDcmHeader()->GetIntValue("DataPointColumns");

    int numComponents = 1;
    std::string representation =  outputData->GetDcmHeader()->GetStringValue( "DataRepresentation" );
    if (representation.compare( "COMPLEX" ) == 0 ) {
        numComponents = 2;
    }

    // The output data will initially be a copy if the input. Let's remove all if it's arrays.
    char arrayName[30];
    int numArrays = outputData->GetCellData()->GetNumberOfArrays();
    for( int i = 0; i < numArrays; i++ ) {
        // Pops the first array off, then the next array becomes array(0)
        outputData->GetCellData()->RemoveArray( outputData->GetCellData()->GetArray(0)->GetName() );
    }

    vtkCellData* cellData = outputData->GetCellData();
    vtkDataArray* dataArray = NULL;
    double progress = 0;
    for( int channel = 0; channel < numChannels; channel++ ) { 
        for( int timePt = 0; timePt < numTimePts; timePt++ ) { 
            ostringstream progressStream;
            progressStream <<"Allocating Arrays for Time Point " << timePt+1 << "/"
                           << numTimePts << " and Channel: " << channel+1 << "/" << numChannels;
            this->SetProgressText( progressStream.str().c_str() );
            for (int z = 0; z < slices; z++) {
                this->UpdateProgress( progress );
                for (int y = 0; y < rows; y++) {
                    for (int x = 0; x < cols; x++) {
                        // Create a new array and set it's components to zero.
                        dataArray = vtkDataArray::CreateDataArray(VTK_FLOAT);
                        dataArray->SetNumberOfComponents( numComponents );
                        dataArray->SetNumberOfTuples(numPts);
                        sprintf(arrayName, "%d %d %d %d %d", x, y, z, timePt, channel);
                        dataArray->FillComponent( 0, 0 );
                        if( numComponents == 2 ) {
                            dataArray->FillComponent( 1, 0 );
                        }
                        static_cast<svkFastCellData*>(cellData)->FastAddArray( dataArray );
                        dataArray->SetName( arrayName );
                        dataArray->FastDelete();
                    }
                }
                progress = ((z+1))/((double)slices);
            }
        }
    }
    svkFastCellData::SafeDownCast( cellData )->FinishFastAdd();
}
