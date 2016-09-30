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
#include <svkMrsImageFFT.h>
#include <vtkImageFourierCenter.h>
#include <svkDdfVolumeWriter.h>

using namespace svk;

int main (int argc, char** argv)
{

    string fname(argv[1]);
    string fnameOut(argv[2]);

    svkDataModel* model = svkDataModel::New();
    svkImageData* data = model->LoadFile( fname.c_str() );
    data->Register(NULL); 

    if( !data->IsA("svkMrsImageData") ) {
        cerr << "INPUT MUST BE SPECTRA!" << endl;
        exit(1);
    }


    //  Reverse FFT spatial data: kspace to spatial domain
    svkMrsImageFFT* spatialRFFT= svkMrsImageFFT::New();
    spatialRFFT->SetInputData( data );
    spatialRFFT->SetFFTDomain( svkMrsImageFFT::SPATIAL );
    spatialRFFT->SetFFTMode( svkMrsImageFFT::REVERSE );
    spatialRFFT->SetPreCorrectCenter( true );
    spatialRFFT->SetPostCorrectCenter( true );
    spatialRFFT->Update();
    
    //  FFT spectral data: time to frequency domain
    svkMrsImageFFT* spectralFFT = svkMrsImageFFT::New();
    spectralFFT->SetInputData( spatialRFFT->GetOutput() );
    spectralFFT->SetFFTDomain( svkMrsImageFFT::SPECTRAL );
    spectralFFT->SetFFTMode( svkMrsImageFFT::FORWARD );
    spectralFFT->Update();
    
    svkDdfVolumeWriter* writer = svkDdfVolumeWriter::New();
    writer->SetFileName( fnameOut.c_str() );    
    writer->SetInputData( spectralFFT->GetOutput() );
    writer->Write();
    writer->Delete();
    spatialRFFT->Delete();
    spectralFFT->Delete();
    model->Delete();
    data->Delete();

    return 0; 
}

