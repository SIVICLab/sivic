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
 *
 */


#include <svkDataModel.h>
#include <svkImageData.h>
#include <svkMriImageData.h>
#include <svkImageLinearPhase.h>
#include <svkImageFourierCenter.h>
#include <svkMriImageFFT.h>
#include <svkIdfVolumeWriter.h>
using namespace svk;

void WriteData( string name, svkImageData* data );
void TestPhase( string name, double shiftWindow[3], svkImageData* data );

int main (int argc, char** argv)
{
    string fname(argv[1]);
    string dirOut(argv[2]);

    svkDataModel* model = svkDataModel::New();
    svkImageData* data = model->LoadFile( fname.c_str() );
    data->Register(NULL); 
   
    int* dims = data->GetDimensions();
     
    if( !data->IsA("svkMriImageData") ) {
        cerr << "INPUT MUST BE AN IMAGE!" << endl;
        exit(1);
    }

    // Write out the original image
    string name  = string(dirOut);
    name.append("/orig");
    WriteData( name, data );

    name = string(dirOut);
    name.append("/fullPixelShift");
    double fullShiftWindow[3] = {1, 0, 0};
    TestPhase( name, fullShiftWindow, data );

    name = string(dirOut);
    name.append("/halfPixelShift");
    double halfShiftWindow[3] = {0.5, 0.0, 0.0};
    TestPhase( name, halfShiftWindow, data );
    model->Delete();
    data->Delete();
    
    return 0; 
}

void TestPhase( string name, double shiftWindow[3], svkImageData* data )
{

    // First lets fft the data
    svkMriImageFFT* fft = svkMriImageFFT::New();
    fft->SetInput( data );
    fft->Update();

    // Next we center it
    svkImageFourierCenter* center = svkImageFourierCenter::New();
    center->SetInput( fft->GetOutput() );
    center->Update();

    // Next we apply our phase shift
    svkImageLinearPhase* phase = svkImageLinearPhase::New();
    phase->SetShiftWindow( shiftWindow );
    phase->SetInput( center->GetOutput() );
    phase->Update();
  
    // Now we return k = 0 to the origin 
    svkImageFourierCenter* uncenter = svkImageFourierCenter::New();
    uncenter->SetReverseCenter( true );
    uncenter->SetInput( phase->GetOutput() );
    uncenter->Update();

    // Now we copy the result into an svkMriImageData object 
    svkMriImageData* fftPhased = svkMriImageData::New(); 
    fftPhased->DeepCopy(fft->GetOutput());
    fftPhased->DeepCopy( uncenter->GetOutput() );
    fftPhased->Update();
    fftPhased->SyncVTKImageDataToDcmHeader();

    //Now we reverse the center
    svkMriImageFFT* rfft = svkMriImageFFT::New();
    rfft->SetFFTMode( svkMriImageFFT::REVERSE );
    rfft->SetInput( fftPhased  );
    rfft->Update();
    
    vtkImageExtractComponents* real = vtkImageExtractComponents::New();
    real->SetComponents( 0 );
    real->SetInput( rfft->GetOutput() );
    real->Update();

    // We need an svkMri image data object. The svkImageFourierCenter algorihtm
    // works with vtkObjects only. This is because we are sub-classing
    // a vtkImageAlgorithm instead of an svkImageAlgorithm
    svkMriImageData* realPhasedData = svkMriImageData::New(); 
    realPhasedData->DeepCopy(rfft->GetOutput());

    realPhasedData->ShallowCopy( real->GetOutput() );
    realPhasedData->Update();
    realPhasedData->SyncVTKImageDataToDcmHeader();

    string nameReal = string(name);
    WriteData( nameReal.append("Real"), realPhasedData );
    realPhasedData->Delete();
    real->Delete();

    vtkImageExtractComponents* imag = vtkImageExtractComponents::New();
    imag->SetComponents( 1 );
    imag->SetInput( rfft->GetOutput() );
    imag->Update();

    // We need an svkMri image data object. The svkImageFourierCenter algorihtm
    // works with vtkObjects only. This is because we are sub-classing
    // a vtkImageAlgorithm instead of an svkImageAlgorithm
    svkMriImageData* imagPhasedData = svkMriImageData::New(); 
    imagPhasedData->DeepCopy(rfft->GetOutput());

    imagPhasedData->DeepCopy( imag->GetOutput() );
    imagPhasedData->Update();

    string nameImag = string(name);
    WriteData( nameImag.append("Imag"), imagPhasedData );
    imagPhasedData->Delete();
    imag->Delete();

    phase->Delete();
    fft->Delete();
    center->Delete();
    uncenter->Delete();
}

void WriteData( string name, svkImageData* data )
{
    data->GetDcmHeader()->SetValue("SeriesDescription", "Test");
    svkIdfVolumeWriter* writer = svkIdfVolumeWriter::New();
    writer->SetCastDoubleToFloat( true );
    writer->SetFileName( name.c_str() );
    writer->SetInput( data );
    writer->Write();
    writer->Delete();
}
