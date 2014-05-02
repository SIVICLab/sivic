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
 *      Beck Olson
 */



#include <svkMrsImageFlip.h>
#include <vtkImageFlip.h>


using namespace svk;


vtkCxxRevisionMacro(svkMrsImageFlip, "$Rev$");
vtkStandardNewMacro(svkMrsImageFlip);


/*!
 *  Constructor.  Initialize any member variables. 
 */
svkMrsImageFlip::svkMrsImageFlip()
{

#if VTK_DEBUG_ON
    this->DebugOn();
#endif

    vtkDebugMacro(<<this->GetClassName() << "::" << this->GetClassName() << "()");

    //  Initialize any member variables
    this->filteredAxis = 0; 
    this->filteredChannel = -1; 
}


/*!
 *  Clean up any allocated member variables. 
 */
svkMrsImageFlip::~svkMrsImageFlip()
{
}


/*
 *  Filter only the specified channel.  By default filters all 
 *  channels if this method isn't called to set one.
 */
void svkMrsImageFlip::SetFilteredChannel( int channel)
{
    this->filteredChannel = channel; 
}


/*
 *  Sets the axis to reverse: 0 = cols(x), 1 = rows(y), 2 = slice(z)
 *  Data is reversed along this axis, e.g. if axis is set to 0, the 
 *  voxel columns are revsed (voxel in col 0 becomes voxel n col N-1, 
 *  and voxels in col N-1 becomes voxels in col 0, etc.  This can also 
 *  be thought of as reverse the data in each row or along the x direction. 
 */
void svkMrsImageFlip::SetFilteredAxis( int axis )
{
    this->filteredAxis = axis; 
}


/*! 
 *  This method is called during pipeline execution.  This is where you should implement your algorithm. 
 */
int svkMrsImageFlip::RequestData( vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector )
{

    //  Get pointer to input data set. 
    svkMrsImageData* mrsData = svkMrsImageData::SafeDownCast(this->GetImageDataInput(0)); 

    //  Get pointer to data's meta-data header (DICOM object). 
    svkDcmHeader* hdr = mrsData->GetDcmHeader();  

    int numSpecPts = hdr->GetIntValue( "DataPointColumns" );
    int cols       = hdr->GetIntValue( "Columns" );
    int rows       = hdr->GetIntValue( "Rows" );
    int slices     = hdr->GetNumberOfSlices();
    int numTimePts = hdr->GetNumberOfTimePoints();
    int numCoils   = hdr->GetNumberOfCoils();  

    vtkImageData* tmpData = NULL;
    svkMriImageData* singleFreqImage = svkMriImageData::New();

    int lowerChannel = 0; 
    int upperChannel = numCoils; 
    if ( this->filteredChannel != -1 ) {
        lowerChannel = this->filteredChannel; 
        upperChannel = this->filteredChannel + 1; 
    }
        

    for( int timePt = 0 ; timePt < numTimePts; timePt++ ) {
        for( int coil = lowerChannel; coil < upperChannel; coil++ ) {
            for( int freq = 0; freq < numSpecPts; freq++ ) {

                mrsData->GetImage( singleFreqImage, freq, timePt, coil, 2, "");

                singleFreqImage->Modified();

                tmpData = singleFreqImage;

                vtkImageFlip* flip = vtkImageFlip::New();
                flip->SetFilteredAxis( this->filteredAxis ); 

                flip->SetInput( tmpData ); 
                tmpData = flip->GetOutput();
                tmpData->Update();


                mrsData->SetImage( tmpData, freq, timePt, coil);

                flip->Delete(); 

            }
        }
    }


    //  Trigger observer update via modified event:
    this->GetInput()->Modified();
    this->GetInput()->Update();

    return 1; 
} 


/*!
 *  Set the input data type to svkMrsImageData.
 */
int svkMrsImageFlip::FillInputPortInformation( int vtkNotUsed(port), vtkInformation* info )
{
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "svkMrsImageData"); 
    return 1;
}


