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


#include <svkImageThreshold.h>


using namespace svk;


vtkStandardNewMacro(svkImageThreshold);


/*!
 *
 */
svkImageThreshold::svkImageThreshold()
{
#if VTK_DEBUG_ON
    this->DebugOn();
#endif

    vtkDebugMacro(<< this->GetClassName() << "::" << this->GetClassName() << "()");

    // We have to call this in the constructor.
    this->SetNumberOfInputPorts(15);
    bool required = true;
    this->GetPortMapper()->InitializeInputPort( INPUT_IMAGE, "INPUT_IMAGE", svkAlgorithmPortMapper::SVK_MR_IMAGE_DATA);
    this->GetPortMapper()->InitializeInputPort( INPUT_ROI, "INPUT_ROI", svkAlgorithmPortMapper::SVK_MR_IMAGE_DATA, !required);
    this->GetPortMapper()->InitializeInputPort( OUTPUT_SERIES_DESCRIPTION, "OUTPUT_SERIES_DESCRIPTION", svkAlgorithmPortMapper::SVK_STRING, !required);
    this->GetPortMapper()->InitializeInputPort( MASK_OUTPUT_VALUE, "MASK_OUTPUT_VALUE", svkAlgorithmPortMapper::SVK_INT, !required);
    this->GetPortMapper()->InitializeInputPort( VOLUME, "VOLUME", svkAlgorithmPortMapper::SVK_INT, !required);
    this->GetPortMapper()->InitializeInputPort( THRESHOLD_MIN, "THRESHOLD_MIN", svkAlgorithmPortMapper::SVK_DOUBLE, !required);
    this->GetPortMapper()->InitializeInputPort( THRESHOLD_MAX, "THRESHOLD_MAX", svkAlgorithmPortMapper::SVK_DOUBLE, !required);
    this->GetPortMapper()->InitializeInputPort( THRESHOLD_BY_MODE_IMAGE, "THRESHOLD_BY_MODE_IMAGE", svkAlgorithmPortMapper::SVK_MR_IMAGE_DATA, !required);
    this->GetPortMapper()->InitializeInputPort( THRESHOLD_BY_MODE_FACTOR, "THRESHOLD_BY_MODE_FACTOR", svkAlgorithmPortMapper::SVK_DOUBLE, !required);
    this->GetPortMapper()->InitializeInputPort( THRESHOLD_BY_MODE_START_BIN, "THRESHOLD_BY_MODE_START_BIN", svkAlgorithmPortMapper::SVK_DOUBLE, !required);
    this->GetPortMapper()->InitializeInputPort( THRESHOLD_BY_MODE_SMOOTH_BINS, "THRESHOLD_BY_MODE_SMOOTH_BINS", svkAlgorithmPortMapper::SVK_INT, !required);
    this->GetPortMapper()->InitializeInputPort( THRESHOLD_BY_MODE_BIN_SIZE, "THRESHOLD_BY_MODE_BIN_SIZE", svkAlgorithmPortMapper::SVK_DOUBLE, !required);
    this->GetPortMapper()->InitializeInputPort( THRESHOLD_BY_MODE_NUM_BINS, "THRESHOLD_BY_MODE_NUM_BINS", svkAlgorithmPortMapper::SVK_INT, !required);
    this->GetPortMapper()->InitializeInputPort( EXCLUSIVE_INTEGER_MATCHING, "EXCLUSIVE_INTEGER_MATCHING", svkAlgorithmPortMapper::SVK_BOOL, !required);
    this->GetPortMapper()->InitializeInputPort( OUTPUT_SCALAR_TYPE, "OUTPUT_SCALAR_TYPE", svkAlgorithmPortMapper::SVK_INT, !required);
    this->SetNumberOfOutputPorts(1);
    this->GetPortMapper()->InitializeOutputPort( 0, "THRESHOLDED_OUTPUT", svkAlgorithmPortMapper::SVK_MR_IMAGE_DATA);
}


/*!
 *
 */
svkImageThreshold::~svkImageThreshold()
{
    vtkDebugMacro(<<this->GetClassName()<<"::~svkImageThreshold()");
}


/*!
 * Utility setter for input port .
 */
void svkImageThreshold::SetThresholdMax( double max )
{
    this->GetPortMapper()->SetDoubleInputPortValue( THRESHOLD_MAX, max);
}


/*!
 * Utility getter for input port .
 */
svkDouble* svkImageThreshold::GetThresholdMax( )
{
    return this->GetPortMapper()->GetDoubleInputPortValue( THRESHOLD_MAX );
}


/*!
 * Utility setter for input port .
 */
void svkImageThreshold::SetThresholdMin( double min )
{
    this->GetPortMapper()->SetDoubleInputPortValue( THRESHOLD_MIN, min);
}


/*!
 * Utility getter for input port .
 */
svkDouble* svkImageThreshold::GetThresholdMin( )
{
    return this->GetPortMapper()->GetDoubleInputPortValue( THRESHOLD_MIN );
}


/*!
 * Utility setter for input port .
 */
void svkImageThreshold::SetMaskOutputValue( int value )
{
    this->GetPortMapper()->SetIntInputPortValue( MASK_OUTPUT_VALUE, value);
}


/*!
 * Utility getter for input port .
 */
svkInt* svkImageThreshold::GetMaskOutputValue( )
{
    return this->GetPortMapper()->GetIntInputPortValue( MASK_OUTPUT_VALUE );
}

/*!
 * Utility setter for input port .
 */
void svkImageThreshold::SetVolume( int volume )
{
    this->GetPortMapper()->SetIntInputPortValue( VOLUME, volume);
}


/*!
 * Utility getter for input port .
 */
svkInt* svkImageThreshold::GetVolume( )
{
    return this->GetPortMapper()->GetIntInputPortValue( VOLUME );
}


/*!
 * Utility setter for input port .
 */
void svkImageThreshold::SetMaskSeriesDescription( string description )
{
    this->GetPortMapper()->SetStringInputPortValue( OUTPUT_SERIES_DESCRIPTION, description);
}


/*!
 * Utility getter for input port .
 */
svkString* svkImageThreshold::GetMaskSeriesDescription( )
{
    return this->GetPortMapper()->GetStringInputPortValue( OUTPUT_SERIES_DESCRIPTION );
}


/*!
 * Sets the output scalar type.
 */
void svkImageThreshold::SetOutputScalarType( int outputScalarType )
{
    this->GetPortMapper()->SetIntInputPortValue( OUTPUT_SCALAR_TYPE, outputScalarType);
}


/*!
 * Utility getter for input port .
 */
svkInt* svkImageThreshold::GetOutputScalarType( )
{
    return this->GetPortMapper()->GetIntInputPortValue( OUTPUT_SCALAR_TYPE );
}


/*!
 *  RequestData pass the input through the algorithm, and copies the dcos and header
 *  to the output. 
 */
int svkImageThreshold::RequestData( vtkInformation* request,
                                    vtkInformationVector** inputVector,
                                    vtkInformationVector* outputVector )
{

    double min = VTK_DOUBLE_MIN;
    double max = VTK_DOUBLE_MAX;
    if( this->GetThresholdMax() != NULL  ) {
        max = this->GetThresholdMax()->GetValue();
    }
    if( this->GetThresholdMin() != NULL  ) {
        min = this->GetThresholdMin()->GetValue();
    }

    string description = this->GetImageDataInput(0)->GetDcmHeader()->GetStringValue("SeriesDescription");
    description.append(" Mask");

    if( this->GetMaskSeriesDescription() != NULL ) {
        description = this->GetMaskSeriesDescription()->GetValue();
    }



    // The MASK_OUTPUT_VALUE is optional so check to see if its set

    svkMriImageData* image = this->GetPortMapper()->GetMRImageInputPortValue(INPUT_IMAGE, 0 );
    if( this->GetPortMapper()->GetDoubleInputPortValue( THRESHOLD_BY_MODE_FACTOR ) != NULL ) {
        double modeFactor = this->GetPortMapper()->GetDoubleInputPortValue( THRESHOLD_BY_MODE_FACTOR )->GetValue();
        double smoothBins = 21;
        int numBins = 1000;
        double binSize = 10;
        double startBin = 0;
        svkMriImageData* roi = this->GetPortMapper()->GetMRImageInputPortValue(INPUT_ROI, 0 );
        if( this->GetPortMapper()->GetMRImageInputPortValue( THRESHOLD_BY_MODE_IMAGE ) != NULL ) {
            image = this->GetPortMapper()->GetMRImageInputPortValue( THRESHOLD_BY_MODE_IMAGE );
        }
        if( this->GetPortMapper()->GetDoubleInputPortValue( THRESHOLD_BY_MODE_START_BIN ) != NULL ) {
            startBin = this->GetPortMapper()->GetDoubleInputPortValue( THRESHOLD_BY_MODE_START_BIN )->GetValue();
        }
        if( this->GetPortMapper()->GetIntInputPortValue( THRESHOLD_BY_MODE_SMOOTH_BINS ) != NULL ) {
            smoothBins = this->GetPortMapper()->GetIntInputPortValue( THRESHOLD_BY_MODE_SMOOTH_BINS )->GetValue();
        }
        if( this->GetPortMapper()->GetDoubleInputPortValue( THRESHOLD_BY_MODE_BIN_SIZE ) != NULL ) {
            binSize = this->GetPortMapper()->GetDoubleInputPortValue( THRESHOLD_BY_MODE_BIN_SIZE )->GetValue();
        }
        if( this->GetPortMapper()->GetIntInputPortValue( THRESHOLD_BY_MODE_NUM_BINS ) != NULL ) {
            numBins = this->GetPortMapper()->GetIntInputPortValue( THRESHOLD_BY_MODE_NUM_BINS )->GetValue();
        }
        binSize = svkStatistics::GetAutoAdjustedBinSize( image, binSize, startBin, numBins );
        vtkDataArray* maskedPixels = svkStatistics::GetMaskedPixels(image,roi);
        vtkDataArray* histogram = svkStatistics::GetHistogram( maskedPixels , binSize, startBin, numBins, smoothBins );
        double mode = svkStatistics::ComputeModeFromHistogram( histogram, binSize, startBin, smoothBins );
        if( modeFactor < 0 ) {
            max = mode*fabs(modeFactor);
        } else {
            min = mode*modeFactor;
        }
        maskedPixels->Delete();
        histogram->Delete();
    }

    // vtkImageThreshold does INCLUSIVE matching, but the UCSF convention is EXCLUSIVE
    if( this->GetPortMapper()->GetBoolInputPortValue( EXCLUSIVE_INTEGER_MATCHING ) != NULL
        && this->GetPortMapper()->GetBoolInputPortValue( EXCLUSIVE_INTEGER_MATCHING )->GetValue()
        && image->GetPointData()->GetScalars()->GetDataType() != VTK_DOUBLE
        && image->GetPointData()->GetScalars()->GetDataType() != VTK_FLOAT ) {
        if( max == floor(max)) {
            max--;
        } else {
            max = floor(max);
        }
        if( min == ceil(min)) {
            min++;
        } else {
            min = ceil(min);
        }
    }

    this->GetOutput()->ZeroCopy(this->GetImageDataInput(0));
    int numVolumes = this->GetImageDataInput(0)->GetPointData()->GetNumberOfArrays();
    for( int vol = 0; vol < numVolumes; vol++) {
        this->GetOutput()->GetPointData()->RemoveArray(0);
    }
    string activeScalarName = this->GetImageDataInput(0)->GetPointData()->GetScalars()->GetName();
    int startVolume = 0;
    int finalVolume = numVolumes - 1;
    if( this->GetVolume() != NULL) {
        startVolume = this->GetVolume()->GetValue();
        finalVolume = this->GetVolume()->GetValue();
        svkDcmHeader::DimensionVector dimensionVector = this->GetOutput()->GetDcmHeader()->GetDimensionIndexVector();
        svkDcmHeader::SetDimensionVectorValue(&dimensionVector, svkDcmHeader::TIME_INDEX, 0);
        svkDcmHeader::SetDimensionVectorValue(&dimensionVector, svkDcmHeader::CHANNEL_INDEX, 0);
        this->GetOutput()->GetDcmHeader()->Redimension(&dimensionVector);
    }
    for( int vol = startVolume; vol <= finalVolume; vol++) {
        vtkImageThreshold* threshold = vtkImageThreshold::New();
        if( this->GetOutputScalarType() != NULL ) {
            threshold->SetOutputScalarType( this->GetOutputScalarType()->GetValue() );
        }
        if(this->GetMaskOutputValue() != NULL ) {
            threshold->SetInValue(this->GetMaskOutputValue()->GetValue());
        }
        threshold->ThresholdBetween( min, max );
        threshold->SetOutValue(0);
        string arrayName = this->GetImageDataInput(0)->GetPointData()->GetArray(vol)->GetName();
        this->GetImageDataInput(0)->GetPointData()->SetActiveScalars( arrayName.c_str());

        svkImageAlgorithmExecuter *executer = svkImageAlgorithmExecuter::New();
        executer->SetAlgorithm(threshold);

        // Set the input of the vtk algorithm to be the input of the executer
        executer->SetInputData(this->GetImageDataInput(0));

        // Update the vtk algorithm
        executer->Update();

        // Copy the output of the vtk algorithm
        this->GetOutput()->GetPointData()->AddArray( executer->GetOutput()->GetPointData()->GetScalars() );
        executer->Delete();
        threshold->Delete();

    }
    this->GetOutput()->GetPointData()->SetActiveScalars(activeScalarName.c_str());
    this->GetOutput()->GetDcmHeader()->SetValue("SeriesDescription", description );
    if( this->GetOutputScalarType() != NULL ) {
        this->GetOutput()->GetDcmHeader()->SetPixelDataType( svkDcmHeader::GetVtkDataTypeFromSvkDataType( this->GetOutputScalarType()->GetValue() ));
    }

    return 1; 
}
