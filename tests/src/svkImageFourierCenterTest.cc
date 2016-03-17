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
#include <svkImageFourierCenter.h>
#include <svkMriImageFFT.h>
#include <svkMrsImageFFT.h>
#include <svkIdfVolumeWriter.h>
using namespace svk;

void WriteData( string name, svkImageData* data );

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


    svkMriImageFFT* fft = svkMriImageFFT::New();
    fft->SetInputData( data );
    fft->Update();

    svkImageFourierCenter* center = svkImageFourierCenter::New();
    center->SetInputData( fft->GetOutput() );
    center->Update();


    // We need an svkMri image data object. The svkImageFourierCenter algorihtm
    // works with vtkObjects only. This is because we are sub-classing
    // a vtkImageAlgorithm instead of an svkImageAlgorithm
    svkMriImageData* centeredData = svkMriImageData::New(); 
    centeredData->DeepCopy(fft->GetOutput());

    centeredData->DeepCopy( center->GetOutput() );


    vtkImageExtractComponents* real = vtkImageExtractComponents::New();
    real->SetComponents( 0 );
    real->SetInputData( centeredData );
    real->Update();

    svkMriImageData* realCenteredData = svkMriImageData::New(); 
    realCenteredData->DeepCopy( centeredData );
    realCenteredData->DeepCopy( real->GetOutput() );

    name = string(dirOut);
    name.append("/realCenteredFFT");
    WriteData( name, realCenteredData );
    realCenteredData->Delete();
    real->Delete();

    // now lets uncenter the data

    svkImageFourierCenter* uncenter = svkImageFourierCenter::New();
    uncenter->SetReverseCenter( true );
    uncenter->SetInputData( centeredData );
    uncenter->Update();
    
    // We need an svkMri image data object. The svkImageFourierCenter algorihtm
    // works with vtkObjects only. This is because we are sub-classing
    // a vtkImageAlgorithm instead of an svkImageAlgorithm
    svkMriImageData* uncenteredData = svkMriImageData::New(); 
    uncenteredData->DeepCopy(fft->GetOutput());

    uncenteredData->DeepCopy( uncenter->GetOutput() );

    real = vtkImageExtractComponents::New();
    real->SetComponents( 0 );
    real->SetInputData( uncenteredData );
    real->Update();

    svkMriImageData* realUncenteredData = svkMriImageData::New(); 
    realUncenteredData->DeepCopy( uncenteredData );
    realUncenteredData->DeepCopy( real->GetOutput() );

    name = string(dirOut);
    name.append("/realUncenteredFFT");
    WriteData( name, realUncenteredData );
    realUncenteredData->Delete();
    real->Delete();

    fft->Delete();
    centeredData->Delete();
    center->Delete();
    uncenteredData->Delete();
    uncenter->Delete();
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
