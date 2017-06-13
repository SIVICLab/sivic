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



#include <svkMrsApodizationFilter.h>
#include <svkImageMathematics.h>


using namespace svk;


//vtkCxxRevisionMacro(svkMrsApodizationFilter, "$Rev$");
vtkStandardNewMacro(svkMrsApodizationFilter);


svkMrsApodizationFilter::svkMrsApodizationFilter()
{

#if VTK_DEBUG_ON
    this->DebugOn();
#endif
    this->spatialFilter = NULL;
    this->spatialFilterReal = NULL;
    vtkDebugMacro(<<this->GetClassName() << "::" << this->GetClassName() << "()");

}


/*!
 *  Destructor. 
 */
svkMrsApodizationFilter::~svkMrsApodizationFilter()
{
    if ( this->spatialFilter != NULL ) {
        this->spatialFilter->Delete(); 
        this->spatialFilter = NULL; 
    }
    if ( this->spatialFilterReal != NULL ) {
        this->spatialFilterReal->Delete(); 
        this->spatialFilterReal = NULL; 
    }
}


/*!
 * Sets the window to be used for the apodization.
 */
void svkMrsApodizationFilter::SetWindow( vector < vtkFloatArray* >* window )
{
    this->window = window;
    if ( this->window->size() == 1 ) {
        this->filterDomain = svkMrsApodizationFilter::SPECTRAL_WINDOW; 
    } else if ( this->window->size() == 3 ) {
        this->filterDomain = svkMrsApodizationFilter::SPATIAL_WINDOW; 
    } 
}


/*! 
 *
 */
int svkMrsApodizationFilter::RequestInformation( vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector )
{
    return 1;
}


/*! 
 *  Call the spectral or spatial filtering subroutine.
 */
int svkMrsApodizationFilter::RequestData( vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector )
{

    int status = 0; 
    if ( this->filterDomain == svkMrsApodizationFilter::SPECTRAL_WINDOW ) { 
        status = this->RequestDataSpectral(); 
    } else if ( this->filterDomain == svkMrsApodizationFilter::SPATIAL_WINDOW ) { 
        status = this->RequestDataSpatial(); 
    } 
    return status; 
}


/*!
 *  Call the spectral or spatial filtering subroutine: Each data array in the input will be multiplied by the window.
 */
int svkMrsApodizationFilter::RequestDataSpectral()
{

    svkMrsImageData* data = svkMrsImageData::SafeDownCast(this->GetImageDataInput(0)); 
    svkDcmHeader* hdr = data->GetDcmHeader();
    int numPoints    = hdr->GetIntValue( "DataPointColumns" );

    double* windowTuple = NULL;
    double* specTuple   = NULL;

    //  Get the Dimension Index and index values  
    svkDcmHeader::DimensionVector dimensionVector = data->GetDcmHeader()->GetDimensionIndexVector();

    //  GetNumber of cells in the image:
    int numCells = svkDcmHeader::GetNumberOfCells( &dimensionVector );

    vtkFloatArray* spectralWindow = (*this->window)[0]; 

    for (int cellID = 0; cellID < numCells; cellID++ ) {

        vtkFloatArray* spectrum = static_cast<vtkFloatArray*>(
            svkMrsImageData::SafeDownCast(data)->GetSpectrum( cellID )
        );

        //  Iterate over frequency points in spectrum and apply phase the window:
        for ( int i = 0; i < numPoints; i++ ) {
            windowTuple = spectralWindow->GetTuple( i );
            specTuple = spectrum->GetTuple( i );
            spectrum->SetTuple2( i, specTuple[0] * windowTuple[0], specTuple[1] * windowTuple[1] ); 
        }
    }

    return 1; 
} 


/*!
 *  Call the spatial filtering subroutine apply filter to 3D image at each frequency point. 
 */
int svkMrsApodizationFilter::RequestDataSpatial()
{
    svkMrsImageData* data = svkMrsImageData::SafeDownCast(this->GetImageDataInput(0)); 
    svkDcmHeader* hdr = data->GetDcmHeader();
    
    //  Create a 3D spatial filter from the 3 arrays in the window vector: 
    //  Initialze the dim vector to all 0.  The non-spatial indices determine which volume is used to 
    //  initializet the output image. 
    svkDcmHeader::DimensionVector dimVector = hdr->GetDimensionIndexVector();
    int frequencyPt = 0; 
    for ( int i = 0; i < dimVector.size(); i++ ) {
        svkDcmHeader::SetDimensionVectorValue(&dimVector, i, 0);
    }

    // create a new blank image and initialize it to be a single 3D vol from the 4D input data. 
    this->spatialFilter = svkMriImageData::New();
    this->spatialFilterReal = svkMriImageData::New();
    //  component 2 = complex data: 
    data->GetImage( this->spatialFilter,     frequencyPt, &dimVector, 2, "", VTK_FLOAT);
    //  component 0 = real only data: 
    data->GetImage( this->spatialFilterReal, frequencyPt, &dimVector, 0, "", VTK_FLOAT);
        
    // loop over the spatial indices (xyz) and assign the filter valuesi at each 3D location: 
    int numVoxels[3];
    data->GetNumberOfVoxels( numVoxels );
    double filterTupleX[2]; 
    double filterTupleY[2]; 
    double filterTupleZ[2]; 
    double filterTuple3D[2]; 
    for ( int z = 0; z < numVoxels[2]; z++ ) {
        for ( int y = 0; y < numVoxels[1]; y++ ) {
            for ( int x = 0; x < numVoxels[0]; x++ ) {

                (*this->window)[0]->GetTuple(x, filterTupleX);     
                (*this->window)[1]->GetTuple(y, filterTupleY);     
                (*this->window)[2]->GetTuple(z, filterTupleZ);     
                filterTuple3D[0] = filterTupleX[0] * filterTupleY[0] * filterTupleZ[0]; // real 
                filterTuple3D[1] = filterTupleX[1] * filterTupleY[1] * filterTupleZ[1]; // imaginary
                //cout << "FILTER( " << x << ", " << y << ", " << z << ") = " 
                    //<< filterTupleX[0] << " " << filterTupleY[0] << " " << filterTupleZ[0] << endl;
                //cout << "FILTERVAL( " << x << ", " << y << ", " << z << ") = " << filterTuple3D[0] << endl;
                this->spatialFilter->SetImagePixelTuple(x, y, z, filterTuple3D ); 
                this->spatialFilterReal->SetImagePixel( x, y, z, filterTuple3D[0] ); 
            }
        }
    }
    

    //  Finally, multiply the input k-space image by the 3D filter.  This needs to be applied at each 
    //  frequency and also to other non-spatial indices, e.g. channels, time pionts, etc. 
    //  Loop over each frequency point apply the 3D spatial filter: 
    cout << "===============================================" << endl;
    cout << "TODO:  implement loop to apply filter to all non-spatial dimensions." << endl;
    cout << "===============================================" << endl;

    //  should have outter loop over other non-spatial cell dimensions (channel, etc)
    int numSpectralPoints = hdr->GetIntValue( "DataPointColumns" );
    svkMriImageData* tmp3DImage = svkMriImageData::New();
    svkImageMathematics* math = svkImageMathematics::New(); 
    math->SetOperationToMultiply();
    math->SetInput1Data( this->spatialFilter );  
    for (int freq = 0; freq < numSpectralPoints; freq++ ) {
        data->GetImage( tmp3DImage, freq, &dimVector, 2, "", VTK_FLOAT);
        math->SetInput2Data( tmp3DImage ); 
        math->Update(); 
        //  Set the filter 3D image back into the input data set: 
        data->SetImage(math->GetOutput(), freq, &dimVector); 
    }

    math->Delete(); 
    tmp3DImage->Delete(); 

    return 1; 
} 


/*!
 *  Gets the spatial filter as a 3D image: 
 */
svkMriImageData* svkMrsApodizationFilter::GetSpatialFilter()
{
    return this->spatialFilterReal; 
}



/*!
 *
 */
int svkMrsApodizationFilter::FillInputPortInformation( int vtkNotUsed(port), vtkInformation* info )
{
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "svkMrsImageData"); 
    return 1;
}


