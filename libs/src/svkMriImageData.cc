/*
 *  Copyright © 2009-2011 The Regents of the University of California.
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
 *  Method attempts to determine the best default window/level values for the dataset. It does this by:
 *    1. Create a histogram with the number of bins specified at input.
 *    2. Determine the mode, excluding zero, and use that as the level.
 *    3. Calculate the standard deviation excluding zero.
 *    4. Use the input number of standard deviations to determine the window.
 *    5. If the window extends outside the range, clip the window such that it is within range.
 *
 *  \param window output reference
 *  \param level output reference 
 *  \param numBins the number of bins used to make the histogram 
 *  \param excludeFactor the number of standard deviations used to define the window 
 *  
 */
void svkMriImageData::GetAutoWindowLevel( double& window, double& level, int numBins, double excludeFactor )
{
    // Here are our parameters for the auto window level
    double *range = this->GetScalarRange();
    double fullWindow = range[1]-range[0];
    double binSize = fullWindow/numBins; 
    vtkImageAccumulate* accumulator = vtkImageAccumulate::New();
    accumulator->SetInput( this );

    // Component origin is the position of the first bins. 3 Dimension correspond to up to 3 components
    accumulator->SetComponentOrigin(range[0],0,0);
   
    // The bins we want to include in the output--- in this case all the bins 
    accumulator->SetComponentExtent(0, numBins-1,0,0,0,0);

    // Component Spacing is the binSize
    accumulator->SetComponentSpacing(binSize,0,0);

    // This ignores zero for staticstics (deviation and mean) but STILL PLACES ZERO IN A BIN!!
    accumulator->IgnoreZeroOn();
    accumulator->Update();

    // The output of image accumulate is an image.
    vtkImageData* histogramImage = accumulator->GetOutput();
    histogramImage->Update();

    // The "pixels" of the output are the values in the histogram
    vtkDataArray* histogram = histogramImage->GetPointData()->GetScalars();
    int numPixels = accumulator->GetVoxelCount();
    double lowSum  = 0; 
    double highSum = 0; 
    double excludeSum = 0;
    double minSum = 0;
    double maxSum = 0;
    int minIndex = 0;
    int maxIndex = numBins-1;
    for( int i = 0; i < numBins; i++ ) {
        int lowIndex = i;
        double lowValue = histogram->GetTuple(lowIndex)[0];
        int highIndex = numBins - i - 1;
        double highValue = histogram->GetTuple(highIndex)[0];
        double binRangeLow[2] = { range[0] + lowIndex*binSize, range[0] + (lowIndex+1)*binSize};
        double binRangeHigh[2] = { range[0] + highIndex*binSize, range[0] + (highIndex+1)*binSize};
        excludeSum = highSum+lowSum;
        highSum += highValue;
        lowSum += lowValue;
        // Lets exclude the bin closests to zero....
        if( (highSum+minSum)/numPixels < excludeFactor && !(binRangeHigh[0] <= 0 && binRangeHigh[1] >= 0 ) ) {
            maxIndex = highIndex;
            maxSum = highSum; 
        }
        if( (lowSum+maxSum)/numPixels < excludeFactor && !(binRangeLow[0] <= 0 && binRangeLow[1] >= 0 ) ) {
            minIndex = lowIndex;
            minSum = lowSum;
        }
    } 

    double min = range[0] + (minIndex)*binSize; 
    double max = range[0] + (maxIndex+1)*(binSize); 
    window = max-min;
    level = min + window/2.0;
    accumulator->Delete();

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
 *  Sets a slice of pixels from a double array. 
 *  Currently assumes 1 component data
 */
void svkMriImageData::SetImagePixels( double* pixels, int slice)
{
    int xpixels;
    int ypixels;
    int npixels;
    xpixels = this->GetExtent()[1] - this->GetExtent()[0] + 1;
    ypixels = this->GetExtent()[3] - this->GetExtent()[2] + 1;
    npixels = xpixels * ypixels;
    for( int i = 0; i < npixels; i++ ) {
        this->GetPointData()->GetArray("pixels")->SetTuple( npixels * slice + i, &(pixels[i]) ); 
    }

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


