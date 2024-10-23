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


#include <svkImageDilateErode.h>
#include </usr/include/vtk/vtkImageDilateErode3D.h>

using namespace svk;


vtkStandardNewMacro(svkImageDilateErode);


/*!
 *
 */
svkImageDilateErode::svkImageDilateErode()
{
#if VTK_DEBUG_ON
    this->DebugOn();
#endif

    vtkDebugMacro(<< this->GetClassName() << "::" << this->GetClassName() << "()");

    // We have to call this in the constructor.
    this->SetNumberOfInputPorts(6);
    bool required = true;
    this->GetPortMapper()->InitializeInputPort( INPUT_IMAGE, "INPUT_IMAGE", svkAlgorithmPortMapper::SVK_MR_IMAGE_DATA);
    this->GetPortMapper()->InitializeInputPort( OUTPUT_SERIES_DESCRIPTION, "OUTPUT_SERIES_DESCRIPTION", svkAlgorithmPortMapper::SVK_STRING, !required);
    this->GetPortMapper()->InitializeInputPort( VOLUME, "VOLUME", svkAlgorithmPortMapper::SVK_INT, !required);
    this->GetPortMapper()->InitializeInputPort( DILATE_VALUE, "DILATE_VALUE", svkAlgorithmPortMapper::SVK_INT, required);
    this->GetPortMapper()->InitializeInputPort( ERODE_VALUE, "ERODE_VALUE", svkAlgorithmPortMapper::SVK_INT, required);
    this->GetPortMapper()->InitializeInputPort( KERNEL_SIZE, "KERNEL_SIZE", svkAlgorithmPortMapper::SVK_INT, required);
    this->SetNumberOfOutputPorts(1);
    this->GetPortMapper()->InitializeOutputPort( 0, "THRESHOLDED_OUTPUT", svkAlgorithmPortMapper::SVK_MR_IMAGE_DATA);
}


/*!
 *
 */
svkImageDilateErode::~svkImageDilateErode()
{
    vtkDebugMacro(<<this->GetClassName()<<"::~svkImageDilateErode()");
}


/*!
 * Utility setter for input port .
 */
void svkImageDilateErode::SetDilateValue( int value )
{
    this->GetPortMapper()->SetIntInputPortValue( DILATE_VALUE, value);
}


/*!
 * Utility getter for input port .
 */
svkInt* svkImageDilateErode::GetDilateValue( )
{
    return this->GetPortMapper()->GetIntInputPortValue( DILATE_VALUE );
}


/*!
 * Utility setter for input port .
 */
void svkImageDilateErode::SetErodeValue( int value )
{
    this->GetPortMapper()->SetIntInputPortValue( ERODE_VALUE, value);
}


/*!
 * Utility getter for input port .
 */
svkInt* svkImageDilateErode::GetErodeValue( )
{
    return this->GetPortMapper()->GetIntInputPortValue( ERODE_VALUE );
}

/*!
 * Utility setter for input port .
 */
void svkImageDilateErode::SetKernelSize( int size )
{
    this->GetPortMapper()->SetIntInputPortValue( KERNEL_SIZE, size);
}


/*!
 * Utility getter for input port .
 */
svkInt* svkImageDilateErode::GetKernelSize( )
{
    return this->GetPortMapper()->GetIntInputPortValue( KERNEL_SIZE );
}

/*!
 * Utility setter for input port .
 */
void svkImageDilateErode::SetVolume( int volume )
{
    this->GetPortMapper()->SetIntInputPortValue( VOLUME, volume);
}


/*!
 * Utility getter for input port .
 */
svkInt* svkImageDilateErode::GetVolume( )
{
    return this->GetPortMapper()->GetIntInputPortValue( VOLUME );
}


/*!
 * Utility setter for input port .
 */
void svkImageDilateErode::SetSeriesDescription( string description )
{
    this->GetPortMapper()->SetStringInputPortValue( OUTPUT_SERIES_DESCRIPTION, description);
}


/*!
 * Utility getter for input port .
 */
svkString* svkImageDilateErode::GetSeriesDescription( )
{
    return this->GetPortMapper()->GetStringInputPortValue( OUTPUT_SERIES_DESCRIPTION );
}


/*!
 *  RequestData pass the input through the algorithm, and copies the dcos and header
 *  to the output. 
 */
int svkImageDilateErode::RequestData( vtkInformation* request,
                                    vtkInformationVector** inputVector,
                                    vtkInformationVector* outputVector )
{

    string description = this->GetImageDataInput(0)->GetDcmHeader()->GetStringValue("SeriesDescription");
    description.append(" DilateErode");

    if( this->GetSeriesDescription() != NULL ) {
        description = this->GetSeriesDescription()->GetValue();
    }


    svkMriImageData* image = this->GetPortMapper()->GetMRImageInputPortValue(INPUT_IMAGE, 0 );

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

        vtkImageDilateErode3D* dilateErode = vtkImageDilateErode3D::New(); 
        dilateErode->SetDilateValue( this->GetDilateValue()->GetValue() ); 
        dilateErode->SetErodeValue( this->GetErodeValue()->GetValue() ); 
        dilateErode->SetKernelSize(
            this->GetKernelSize()->GetValue(), 
            this->GetKernelSize()->GetValue(), 
            this->GetKernelSize()->GetValue() 
        ); 

        string arrayName = this->GetImageDataInput(0)->GetPointData()->GetArray(vol)->GetName();
        this->GetImageDataInput(0)->GetPointData()->SetActiveScalars( arrayName.c_str());

        svkImageAlgorithmExecuter *executer = svkImageAlgorithmExecuter::New();
        executer->SetAlgorithm( dilateErode );

        // Set the input of the vtk algorithm to be the input of the executer
        executer->SetInputData(this->GetImageDataInput(0));

        // Update the vtk algorithm
        executer->Update();

        // Copy the output of the vtk algorithm
        this->GetOutput()->GetPointData()->AddArray( executer->GetOutput()->GetPointData()->GetScalars() );
        executer->Delete();
        dilateErode->Delete();

    }
    this->GetOutput()->GetPointData()->SetActiveScalars(activeScalarName.c_str());
    this->GetOutput()->GetDcmHeader()->SetValue("SeriesDescription", description );

    return 1; 
}
