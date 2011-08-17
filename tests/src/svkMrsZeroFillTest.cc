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
#include <svkMrsZeroFill.h>
#include <svkDdfVolumeWriter.h>
#include <svkDdfVolumeWriter.h>
#include <svkDICOMMRSWriter.h>
#include <svkCoilCombine.h>


using namespace svk;


/*
 *  Application for testing data combination algorithms. 
 */
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
    svkCoilCombine* combine = svkCoilCombine::New();
    combine->SetInput( data );
    combine->SetCombinationMethod( svkCoilCombine::ADDITION );
    combine->SetCombinationDimension( svkCoilCombine::COIL );
    combine->Update(); 
    //  Combine coils using straight addition 
    svkMrsZeroFill* zf = svkMrsZeroFill::New();
    zf->SetInput( data );
    zf->SetNumberOfSpecPointsToDouble( ); 
    zf->SetOutputWholeExtent(0, 12, 0, 10, 0, 11 ); 
    zf->Update();
    svkDdfVolumeWriter* writer = svkDdfVolumeWriter::New();
    string bothFilename(fnameOut);
    bothFilename.append("_both");
    writer->SetFileName( bothFilename.c_str() );    
    writer->SetInput( data );
    writer->Write();

    // Clean up
    combine->Delete();
    combine = NULL;
    writer->Delete();
    writer = NULL;
    zf->Delete();
    zf = NULL;
    data->Delete();
    data = NULL;
    // Filter is in place so we need to reload the data
    data = model->LoadFile( fname.c_str() );
    data->Register(NULL); 

    combine = svkCoilCombine::New();
    combine->SetInput( data );
    combine->SetCombinationMethod( svkCoilCombine::ADDITION );
    combine->SetCombinationDimension( svkCoilCombine::COIL );
    combine->Update(); 

    //  Combine coils using straight addition 
    zf = svkMrsZeroFill::New();
    zf->SetInput( combine->GetOutput() );
    zf->SetNumberOfSpecPointsToNextPower2( ); 
    zf->Update();

    writer = svkDdfVolumeWriter::New();
    string spectralFilename(fnameOut);
    spectralFilename.append("_spectral");
    writer->SetFileName( spectralFilename.c_str() );    
    writer->SetInput( zf->GetOutput() );
    writer->Write();

    // Clean up
    combine->Delete();
    combine = NULL;
    writer->Delete();
    writer = NULL;
    zf->Delete();
    zf = NULL;
    data->Delete();
    data = NULL;

    // Filter is in place so we need to reload the data
    data = model->LoadFile( fname.c_str() );
    data->Register(NULL); 
    combine = svkCoilCombine::New();
    combine->SetInput( data );
    combine->SetCombinationMethod( svkCoilCombine::ADDITION );
    combine->SetCombinationDimension( svkCoilCombine::COIL );
    combine->Update(); 

    //  Combine coils using straight addition 
    zf = svkMrsZeroFill::New();
    zf->SetInput( combine->GetOutput() );
    zf->SetOutputWholeExtent(0, 9, 0, 10, 0, 11 ); 
    zf->Update();

    writer = svkDdfVolumeWriter::New();
    string spatialFilename(fnameOut);
    spatialFilename.append("_spatial");
    writer->SetFileName( spatialFilename.c_str() );    
    writer->SetInput( zf->GetOutput() );
    writer->Write();

    // Clean up
    zf->Delete();
    zf = NULL;
    combine->Delete();
    combine = NULL;
    writer->Delete();
    writer = NULL;
    data->Delete();
    data = NULL;
    model->Delete();
    model = NULL;
    return 0; 
}

