/*
 *  Copyright © 2009 The Regents of the University of California.
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



#include <svkMriImageData.h>


using namespace svk;


vtkCxxRevisionMacro(svkMriImageData, "$Rev$");
vtkStandardNewMacro(svkMriImageData);


/*!
 *
 */
svkMriImageData::svkMriImageData()
{
    topoGenerator = NULL;
    pixelBuffer = NULL;
#if VTK_DEBUG_ON
    this->DebugOn();
#endif

}


/*!
 *
 */
vtkObject* svkMriImageData::NewObject()
{
    return vtkObject::SafeDownCast( svkMriImageData::New() );
}


/*!
 *
 */
svkMriImageData::~svkMriImageData()
{

    vtkDebugMacro(<<"svkMriImageData::~svkMriImageData");
    if( pixelBuffer != NULL ) {
        delete[] pixelBuffer;
    }

}


/*!
 *  Creates an array of doubles that is the pixels of image at slice "slice.
 *  Currently assumes 1 component data
 */
double* svkMriImageData::GetImagePixels( int slice)
{
    int xpixels;
    int ypixels;
    int npixels;
    xpixels = this->GetExtent()[1] - this->GetExtent()[0] + 1;
    ypixels = this->GetExtent()[3] - this->GetExtent()[2] + 1;
    npixels = xpixels * ypixels;
    if( pixelBuffer == NULL ) {
        pixelBuffer = new double[ npixels ];
    }
    for( int i = 0; i < npixels; i++ ) {
        pixelBuffer[i] = (this->
            GetPointData()->GetArray("pixels")->GetTuple1( npixels * slice + i ));
    }
    return pixelBuffer;
}


/*!
 *  Creates an array of doubles that is the pixels of image at slice "slice.
 *  Output goes into a vtkDataArray that matches the type of the pixel array in
 *  the original dataset. Currently assumes 1 component data.
 */
vtkDataArray* svkMriImageData::GetImagePixelsArray( int slice )
{
    vtkDataArray* pixelBufferArray; 
    pixelBufferArray = vtkDataArray::CreateDataArray( this->GetPointData()->GetArray("pixels")->GetDataType());
    int xpixels;
    int ypixels;
    int npixels;
    xpixels = this->GetExtent()[1] - this->GetExtent()[0] + 1;
    ypixels = this->GetExtent()[3] - this->GetExtent()[2] + 1;
    npixels = xpixels * ypixels;
    pixelBufferArray->SetNumberOfComponents(1);
    pixelBufferArray->SetNumberOfTuples(npixels);
    for( int i = 0; i < npixels; i++ ) {
        pixelBufferArray->SetTuple1(i, this->
            GetPointData()->GetArray("pixels")->GetTuple1( npixels * slice + i ));
    }
    return pixelBufferArray;
}


/*!
 *  Returns a pointer to the data at the specified voxel index. 
 */
double* svkMriImageData::GetImagePixel( int x, int y, int z )
{
    int voxelID = this->GetIDFromIndex(x, y, z);
    return  this->GetImagePixel(voxelID);  
}


/*!
 *  Returns a pointer to the data at the specified voxel ID. 
 */
double* svkMriImageData::GetImagePixel( int id )
{
    double* pixel = NULL; 
    pixel = this->GetPointData()->GetArray("pixels")->GetTuple( id );
    return pixel; 
}


/*!
 *  Returns a pointer to the data at the specified voxel index. 
 */
void svkMriImageData::SetImagePixel( int x, int y, int z, double value )
{
    int voxelID = this->GetIDFromIndex(x, y, z);
    this->SetImagePixel(voxelID, value);  
}


/*!
 *  Returns a pointer to the data at the specified voxel ID. 
 */
void svkMriImageData::SetImagePixel( int id , double value )
{
    double* pixel = NULL; 
    this->GetPointData()->GetArray("pixels")->SetTuple1( id, value );
}


/*!
 *  Return the number of voxels in the data set. 
 */
void svkMriImageData::GetNumberOfVoxels(int numVoxels[3])
{
    // Since images have the same dimension and voxels:  
    numVoxels[0] = (this->GetDimensions())[0];
    numVoxels[1] = (this->GetDimensions())[1];
    numVoxels[2] = (this->GetDimensions())[2];
}


