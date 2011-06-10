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
#include <svkSincInterpolationFilter.h>
#include <svkIdfVolumeWriter.h>
#include <svkDICOMMRIWriter.h>

using namespace svk;

void TestSinc( string name, int newDims[3], svkImageData* data );

int main (int argc, char** argv)
{
    string fname(argv[1]);
    string dirOut(argv[2]);

    svkDataModel* model = svkDataModel::New();
    svkImageData* data = model->LoadFile( fname.c_str() );
    data->Register(NULL); 
    
    if( !data->IsA("svkMriImageData") ) {
        cerr << "INPUT MUST BE AN IMAGE!" << endl;
        exit(1);
    }
    string origOut  = string(dirOut);
    origOut.append("/orig");
    svkIdfVolumeWriter* writer = svkIdfVolumeWriter::New();
    writer->SetFileName( origOut.c_str() );    
    writer->SetInput( data );
    writer->Write();
    writer->Delete();

    int newDimsEven[3] = { 16, 16, 16 };
    string sincEvenName = string(dirOut);
    sincEvenName.append("/sincEven");
    TestSinc( sincEvenName, newDimsEven, data ); 

    int newDimsOdd[3] = { 15, 15, 15 };
    string sincOddName = string(dirOut);
    sincOddName.append("/sincOdd");
    TestSinc( sincOddName, newDimsOdd, data ); 
    model->Delete();
    data->Delete();
    return 0; 

}

void TestSinc( string name, int newDims[3], svkImageData* data ) 
{
    svkSincInterpolationFilter* sinc = svkSincInterpolationFilter::New();
    sinc->SetOutputWholeExtent(0, newDims[0]-1, 0, newDims[1]-1, 0, newDims[2]-1);
    sinc->SetInput( data );
    sinc->Update();

    vtkImageExtractComponents* real = vtkImageExtractComponents::New();
    real->SetComponents( 0 );
    real->SetInput( sinc->GetOutput() );
    real->Update();

    svkMriImageData* realSinc = svkMriImageData::New();
    realSinc->DeepCopy( sinc->GetOutput());
    realSinc->DeepCopy( real->GetOutput());

    svkIdfVolumeWriter* writer = svkIdfVolumeWriter::New();
    string realSincName = string(name);
    realSincName.append("Real");
    
    writer->SetFileName( realSincName.c_str() );    
    writer->SetInput( realSinc );
    writer->Write();
    writer->Delete();

    vtkImageExtractComponents* imag = vtkImageExtractComponents::New();
    imag->SetComponents( 1 );
    imag->SetInput( sinc->GetOutput() );
    imag->Update();

    svkMriImageData* imagSinc = svkMriImageData::New();
    imagSinc->DeepCopy( sinc->GetOutput());
    imagSinc->DeepCopy( imag->GetOutput());

    writer = svkIdfVolumeWriter::New();
    string imagSincName = string(name);
    imagSincName.append("Imag");
    
    writer->SetFileName( imagSincName.c_str() );    
    writer->SetInput( imagSinc );
    writer->Write();
    writer->Delete();
    sinc->Delete();
}
