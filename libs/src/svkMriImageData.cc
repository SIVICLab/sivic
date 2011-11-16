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
#include <svk4DImageData.h>


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
    this->cellDataRepresentation = NULL;

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
            GetPointData()->GetScalars()->GetTuple1( npixels * slice + i ));
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
        this->GetPointData()->GetScalars()->SetTuple( npixels * slice + i, &(pixels[i]) ); 
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
    pixelBufferArray = vtkDataArray::CreateDataArray( this->GetPointData()->GetScalars()->GetDataType());
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
            GetPointData()->GetScalars()->GetTuple1( npixels * slice + i ));
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
    pixel = this->GetPointData()->GetScalars()->GetTuple( id );
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
    this->GetPointData()->GetScalars()->SetTuple1( id, value );
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


/*!
 *  This method will return a representation of the data using
 *  cell data. This representation is used primarily for the
 *  visualization of the image volume as traces.
 *
 * @return
 */
 svk4DImageData* svkMriImageData::GetCellDataRepresentation()
 {
     // Check to see if representation has been initialized
     if( this->cellDataRepresentation == NULL ) {
         this->cellDataRepresentation = svk4DImageData::New();
         this->cellDataRepresentation->SetDcmHeader( this->GetDcmHeader());
         this->cellDataRepresentation->Modified();
         this->cellDataRepresentation->SyncVTKImageDataToDcmHeader();
         this->InitializeCellDataArrays();
         // These calls are commented because we are not sure if we want
         // the image data to update if the 4Drepresentanion is modified
         //this->cellRepresentationModifiedCB = vtkCallbackCommand::New();
         //this->cellRepresentationModifiedCB->SetCallback( UpdatePixelData );
         //this->cellRepresentationModifiedCB->SetClientData( (void*)this );
         this->SyncCellRepresentationToPixelData();
     }
     return this->cellDataRepresentation;
 }


/*!
 *  This method will copy the cell data representation values into
 *  the pixel data array.
 *
 */
void svkMriImageData::SyncPixelDataToCellRepresentation()
{
    // NOT YET IMPLEMENTED
}


/*!
 *  This method will copy the pixel data into the cell representation
 *  of the data.
 */
void svkMriImageData::SyncCellRepresentationToPixelData()
{

    int numChannels  = this->GetDcmHeader()->GetNumberOfCoils();
    int numTimePts = this->GetDcmHeader()->GetNumberOfTimePoints();
    vtkDataArray* oldScalars = this->GetPointData()->GetScalars();
    for( int channel = 0; channel < numChannels; channel++ ) {
        int* channelPtr = &channel;
        for( int timePoint = 0; timePoint < numTimePts; timePoint++ ) {
            this->GetPointData()->SetActiveScalars( this->GetPointData()->GetArray( timePoint )->GetName() );
            this->cellDataRepresentation->SetImage(this, timePoint, channelPtr);
        }
    }
    this->GetPointData()->SetActiveScalars( oldScalars->GetName() );


}


/*!
 *  This method will initialize the data arrays for the cell data representation.
 */
void svkMriImageData::InitializeCellDataArrays()
{
    int numChannels  = this->GetDcmHeader()->GetNumberOfCoils();
    int numTimePts = this->GetDcmHeader()->GetNumberOfTimePoints();
    int* extent = this->GetExtent();
    int* dims = this->GetDimensions();
    vtkstd::string representation = this->GetDcmHeader()->GetStringSequenceItemElement(
                                        "MRImageFrameTypeSequence",
                                        0,
                                        "ComplexImageComponent",
                                        "SharedFunctionalGroupsSequence",
                                        0
                                        );
    vtkCellData* cellData = this->cellDataRepresentation->GetCellData();
    char arrayName[30];
    int numComponents = 1;
    if( representation.compare("MIXED") == 0) {
        numComponents = 2;
    }
    int linearIndex = 0;
    int counter = 0;
    int numVoxels = (dims[0] + 1) * (dims[1] + 1) * (dims[2] + 1);
    cellData->SetNumberOfTuples(  numTimePts );
    cellData->AllocateArrays( numChannels * numVoxels );
	vtkAbstractArray* array = NULL;
	int type = this->GetPointData()->GetArray(0)->GetDataType();
    for( int channel = 0; channel < numChannels; channel++ ) {
        for (int z = 0; z < dims[2]; z++) {
            for (int y = 0; y < dims[1]; y++) {
                for (int x = 0; x < dims[0]; x++) {
                    array = vtkDataArray::CreateArray( type );
                    array->SetNumberOfComponents( numComponents );
                    array->SetNumberOfTuples( numTimePts );
                    sprintf(arrayName, "%d %d %d %d", x, y, z, channel);
                    static_cast<svkFastCellData*>(cellData)->FastAddArray( array );
                    array->SetName(arrayName);
                    array->FastDelete();
                }
            }
        }
    }
    svkFastCellData::SafeDownCast( cellData )->FinishFastAdd();
}


 /*!
  *  Updates the pixel data when the data representation is modified.
  *
  * @param subject
  * @param eid
  * @param thisObject
  * @param calldata
  */
void svkMriImageData::UpdatePixelData(vtkObject* subject, unsigned long eid, void* thisObject, void *calldata)
{
    svkMriImageData* data = static_cast<svkMriImageData*>(thisObject);
    data->SyncPixelDataToCellRepresentation();
}
