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
#include <svkMriZeroFill.h>
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

    // Create do padding by 1 extra pixel on each side
    svkMriZeroFill* padOne = svkMriZeroFill::New();
    padOne->SetInput( data );

    // Pad + 1 pixel. Remeber normally extent max is dim - 1 
    padOne->SetOutputWholeExtent(0, dims[0], 0, dims[1], 0, dims[2]);
    padOne->Update();

    name = string(dirOut);
    name.append("/paddedOne");
    WriteData( name, padOne->GetOutput() );

    // Create do padding by 1 extra pixel on each side
    svkMriZeroFill* padTwo = svkMriZeroFill::New();
    padTwo->SetInput( data );

    // Pad + 2 pixel. Remeber normally extent max is dim - 1 
    padTwo->SetOutputWholeExtent(0, dims[0] + 1, 0, dims[1] + 1, 0, dims[2] + 1);
    padTwo->Update();

    name = string(dirOut);
    name.append("/paddedTwo");
    WriteData( name, padTwo->GetOutput() );

    padTwo->Delete();
    padOne->Delete();
    model->Delete();
    data->Delete();
    cout << "RETURNING FROM MAIN..." << endl;
    return 0; 
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

