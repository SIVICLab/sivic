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
#include <svkMriImageFFT.h>
#include <vtkImageFourierCenter.h>
#include <vtkImageMagnitude.h>
#include <svkIdfVolumeWriter.h>
#include <svkDICOMMRIWriter.h>
#include <svkImageFourierCenter.h>
using namespace svk;

void WriteData( string name, svkImageData* data );

int main (int argc, char** argv)
{
    string fname(argv[1]);
    string dirOut(argv[2]);

    /************************************************
     *      READ IN THE IMAGE                       *
     ***********************************************/

    svkDataModel* model = svkDataModel::New();
    svkImageData* data = model->LoadFile( fname.c_str() );
    data->Register(NULL);
    if( !data->IsA("svkMriImageData") ) {
        cerr << "INPUT MUST BE AN IMAGE!" << endl;
        exit(1);
    }

    /************************************************
     *      WRITE THE IMAGE OUT                     *
     ***********************************************/

    // Write out the original image
    string name  = string(dirOut);
    name.append("/orig");
    WriteData( name, data );

    /************************************************
     *      Run the forward FFT                     *
     ***********************************************/
    
    //Reverse FFT spatial data: kspace to spatial domain
    svkMriImageFFT* fft = svkMriImageFFT::New();
    fft->SetInputData( data );
    fft->SetFFTMode( svkMriImageFFT::FORWARD );
    fft->Update();

    // Write out the ft of the image

    /************************************************
     *      Extract the real and write              *
     ***********************************************/

    // First we need a new object to hold the magnitude of the image
    svkMriImageData* realFT = svkMriImageData::New();

    vtkImageExtractComponents* real = vtkImageExtractComponents::New();
    real->SetComponents( 0 );
    real->SetInputData( fft->GetOutput() );
    real->Update();


    // Lets get the basic structure from the svkImageData
    realFT->DeepCopy( fft->GetOutput() );
    realFT->DeepCopy( real->GetOutput() );

    // Write out the real fft
    name  = string(dirOut);
    name.append("/realFT");
    WriteData( name, realFT );
    realFT->Delete();
    real->Delete();

    /************************************************
     *      Extract the imag and write              *
     ***********************************************/

    // First we need a new object to hold the magnitude of the image
    svkMriImageData* imagFT = svkMriImageData::New();

    vtkImageExtractComponents* imag = vtkImageExtractComponents::New();
    imag->SetComponents( 1 );
    imag->SetInputData( fft->GetOutput() );
    imag->Update();


    // Lets get the basic structure from the svkImageData
    imagFT->DeepCopy( fft->GetOutput() );
    imagFT->DeepCopy( imag->GetOutput() );

    // Write out the imag fft
    name  = string(dirOut);
    name.append("/imagFT");
    WriteData( name, imagFT );
    imagFT->Delete();
    imag->Delete();

    /************************************************
     *      Reverse the fourier transform           *
     ***********************************************/

    svkMriImageFFT* rfft = svkMriImageFFT::New();
    rfft->SetInputData( fft->GetOutput() );
    rfft->SetFFTMode( svkMriImageFFT::REVERSE );
    rfft->Update();

    /************************************************
     *      Extract the real and write              *
     ***********************************************/

    // First we need a new object to hold the magnitude of the image
    svkMriImageData* realRFT = svkMriImageData::New();

    real = vtkImageExtractComponents::New();
    real->SetComponents( 0 );
    real->SetInputData( rfft->GetOutput() );
    real->Update();


    // Lets get the basic structure from the svkImageData
    realRFT->DeepCopy( rfft->GetOutput() );
    realRFT->DeepCopy( real->GetOutput() );

    // Write out the real fft
    name  = string(dirOut);
    name.append("/realRFT");
    WriteData( name, realRFT );
    realRFT->Delete();
    real->Delete();

    /************************************************
     *      Extract the imag and write              *
     ***********************************************/

    // First we need a new object to hold the magnitude of the image
    svkMriImageData* imagRFT = svkMriImageData::New();

    imag = vtkImageExtractComponents::New();
    imag->SetComponents( 1 );
    imag->SetInputData( rfft->GetOutput() );
    imag->Update();


    // Lets get the basic structure from the svkImageData
    imagRFT->DeepCopy( rfft->GetOutput() );
    imagRFT->DeepCopy( imag->GetOutput() );

    // Write out the imag fft
    name  = string(dirOut);
    name.append("/imagRFT");
    WriteData( name, imagRFT );
    imagRFT->Delete();
    imag->Delete();

    fft->Delete();
    rfft->Delete();
    model->Delete();
    data->Delete();
    return 0; 
}


void WriteData( string name, svkImageData* data ) 
{
    data->GetDcmHeader()->SetValue("SeriesDescription", "Test");
    svkIdfVolumeWriter* writer = svkIdfVolumeWriter::New();
    writer->SetCastDoubleToFloat( true );
    writer->SetFileName( name.c_str() );    
    writer->SetInputData( data );
    writer->Write();
    writer->Delete();
}
