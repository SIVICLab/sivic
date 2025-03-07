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
 *
 */


#include <svkDataModel.h>
#include <svkImageData.h>
#include <svkMrsImageFFT.h>
#include <svkPlotGridView.h>
#include <vtkRenderWindow.h>
#include <vtkImageViewer2.h>
#include <vtkBMPReader.h>
#include <vtkPNMReader.h>
#include <vtkImageExtractComponents.h>
#include <vtkImageFourierCenter.h>
#include <vtkImageData.h>
#include <vtkImageMagnitude.h>
#define PI 3.1415926535

using namespace svk;

void Initialize( );
void PrintMatlabFormat( vtkImageData* image, string variable );
void ViewImage( vtkImageData* image );
void FFTImage(  vtkImageData* input, vtkImageData* output );
void RFFTImage( vtkImageData* input, vtkImageData* output );
void CreateImage( vtkImageData* input, int size );
void CreateGradient( vtkImageData* input, int size );
void CreateBox( vtkImageData* input, int size );

vtkImageViewer2* viewer;
vtkRenderWindowInteractor* iren;

int main (int argc, char** argv)
{

    Initialize();

    // Lets Create a sine wave
    vtkImageData* sineWaveImage = vtkImageData::New();
    //CreateImage( sineWaveImage, 12 );
    //CreateGradient( sineWaveImage, 12 );
    CreateBox( sineWaveImage, 12 );
    PrintMatlabFormat( sineWaveImage, string("sineWave") );
    ViewImage( sineWaveImage );

    vtkImageData* sineWaveImageFT = vtkImageData::New();
    FFTImage( sineWaveImage, sineWaveImageFT );
    sineWaveImageFT->Update();
    PrintMatlabFormat( sineWaveImageFT, string("sineWaveFT") );
    ViewImage( sineWaveImageFT );

    vtkImageData* sineWaveImageRFT = vtkImageData::New();
    RFFTImage( sineWaveImageFT, sineWaveImageRFT );
    PrintMatlabFormat( sineWaveImageRFT, string("sineWaveRFT") );
    ViewImage( sineWaveImageRFT );

    // Lets load some spectra....
    svkDataModel* model = svkDataModel::New();

    svkImageData* data = model->LoadFile( "/home/bolson/sivic_data/t3148_1_cor.ddf" );
    data->Register(NULL);
    data->Update();
    cout << "Checking to make sure its an MrsImageData....." << endl;
    if( !data->IsA("svkMrsImageData") ) {
        cerr << "INPUT MUST BE SPECTRA!" << endl;
        exit(1);
    }

    // Lets look at the data...
    svkMriImageData* pointImage =  svkMriImageData::New();
    svkMrsImageData::SafeDownCast( data )->GetImage( pointImage, 187, 0, 0, 2, "" );
    //pointImage->GetPointData()->SetScalars( sineWaveImage->GetPointData()->GetScalars() );
    //pointImage->GetPointData()->Modified();
    //pointImage->Update();
    cout << "Point Image: " << *pointImage << endl;
    cout << "Sine Image: " << *sineWaveImage << endl;
    PrintMatlabFormat( pointImage, string("pointImage") );
    ViewImage( pointImage );

    svkMriImageData* pointImageFT = svkMriImageData::New();
    FFTImage( pointImage, pointImageFT );
    PrintMatlabFormat( pointImageFT, string("pointImageFT") );
    ViewImage( pointImageFT );

    svkMriImageData* pointImageRFT = svkMriImageData::New();
    RFFTImage( pointImageFT, pointImageRFT );
    PrintMatlabFormat( pointImageFT, string("pointImageRFT") );
    ViewImage( pointImageRFT );

    return 0; 
}

void Initialize( )
{
    viewer = vtkImageViewer2::New();
    viewer->GetImageActor()->InterpolateOff();
    iren = vtkRenderWindowInteractor::New();
    viewer->SetupInteractor( iren );
}

void PrintMatlabFormat( vtkImageData* image, string variable )
{
    double* tuple;
    int linearIndex;
    cout << variable.c_str() << "=[ " << endl;
    for( int i = 0; i <= image->GetExtent()[1]; i++ ) {
        for( int j = 0; j <= image->GetExtent()[3]; j++ ) {
            linearIndex = j + i*(image->GetExtent()[1]+1);
            tuple =  image->GetPointData()->GetScalars()->GetTuple(linearIndex);
            cout << "(" << tuple[0] << ") + (" << tuple[1] << "i) ";
        }
        cout << ";" << endl;
    }
    cout << "]" << endl;

}


void ViewImage( vtkImageData* image )
{
    viewer->SetInput( image );
    viewer->GetRenderer()->ResetCamera( );
    viewer->GetRenderWindow()->SetSize( 800, 800 );
    viewer->Render(); 
    iren->Start();
}

void FFTImage(  vtkImageData* input, vtkImageData* output )
{
    vtkImageFFT* imageFFT = vtkImageFFT::New();
    imageFFT->SetInput( input );
    imageFFT->SetDimensionality(3);
    imageFFT->Update();
    vtkImageData* tmpData = imageFFT->GetOutput();
    tmpData->Update();
    output->DeepCopy( tmpData );
    output->Update();
    //imageFFT->Delete();

}

void RFFTImage( vtkImageData* input, vtkImageData* output )
{
    vtkImageRFFT* imageRFFT = vtkImageRFFT::New();
    imageRFFT->SetInput( input );
    imageRFFT->SetDimensionality(3);
    imageRFFT->Update();
    output->DeepCopy( imageRFFT->GetOutput() );
    output->Update();
    imageRFFT->Delete();

}

void CenterImage( vtkImageData* input, vtkImageData* output )
{
    vtkImageFourierCenter* ifc = vtkImageFourierCenter::New();
    ifc->SetInput( input );
    ifc->SetDimensionality(2);
    ifc->Update();
    output->DeepCopy( ifc->GetOutput() );
    output->Update();
    //ifc->Delete();

}

void CreateImage( vtkImageData* input, int size )
{
    vtkFloatArray* sinWave = vtkFloatArray::New();
    int rows = size;
    int cols = size;
    sinWave->SetNumberOfComponents(2);
    sinWave->SetNumberOfTuples(rows*cols);
    int linearIndex;
    int constant = 1000000000;
    for( int i = 0; i < rows; i++ ) {
        for( int j = 0; j < cols; j++ ) {
            linearIndex = j+i*cols;
            sinWave->SetComponent(j+i*cols,0, constant*sin((2*PI*j)/(cols-1)) );
            sinWave->SetComponent(j+i*cols,1, constant*cos((2*PI*j)/(cols-1)) );
        }
    }
    input->SetExtent(0, cols-1, 0, rows-1, 0, 0);
    input->SetNumberOfScalarComponents( 2 ); 
    input->GetPointData()->SetScalars( sinWave ); 
    input->SetScalarTypeToFloat( ); 
}

void CreateGradient( vtkImageData* input, int size )
{   
    vtkDoubleArray* sinWave = vtkDoubleArray::New();
    int rows = size;
    int cols = size;
    sinWave->SetNumberOfComponents(2);
    sinWave->SetNumberOfTuples(rows*cols);
    int linearIndex;
    int constant = 1000000;
    for( int i = 0; i < rows; i++ ) { 
        for( int j = 0; j < cols; j++ ) {
            linearIndex = j+i*cols;
            //sinWave->SetComponent(j+i*cols,0, constant*linearIndex);
            //sinWave->SetComponent(j+i*cols,1, constant*linearIndex );
            sinWave->SetComponent(j+i*cols,0, constant);
            sinWave->SetComponent(j+i*cols,1, 0 );
        }
    }
    input->SetExtent(0, cols-1, 0, rows-1, 0, 0);
    input->SetDimensions(cols, rows, 1);
    input->SetNumberOfScalarComponents( 2 );
    input->GetPointData()->SetScalars( sinWave );
    input->SetScalarTypeToDouble( );
    void* tmp = input->GetIncrements();
    input->Modified();
    input->Update();

}

void CreateBox( vtkImageData* input, int size )
{   
    vtkDoubleArray* box = vtkDoubleArray::New();
    int rows = size;
    int cols = size;
    box->SetNumberOfComponents(2);
    box->SetNumberOfTuples(rows*cols);
    int linearIndex;
    int constant = 1000000;
    for( int i = 0; i < rows; i++ ) { 
        for( int j = 0; j < cols; j++ ) {
            linearIndex = j+i*cols;
            if( i > rows/4-1 && i < 3*rows/4 && j > cols/4-1 && j < 3*cols/4 ) {
                box->SetComponent(j+i*cols,0, constant);
                box->SetComponent(j+i*cols,1, constant );
            } else {
                box->SetComponent(j+i*cols,0, 0 );
                box->SetComponent(j+i*cols,1, 0 );
            }
        }
    }
    input->SetExtent(0, cols-1, 0, rows-1, 0, 0);
    input->SetDimensions(cols, rows, 1);
    input->SetNumberOfScalarComponents( 2 );
    input->GetPointData()->SetScalars( box );
    input->SetScalarTypeToDouble( );
    void* tmp = input->GetIncrements();
    input->Modified();
    input->Update();

}


