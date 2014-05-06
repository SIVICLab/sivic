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
#include <svkMrsImageFFT.h>
#include <svkDdfVolumeWriter.h>
#include <svkDdfVolumeWriter.h>
#include <svkDICOMMRSWriter.h>
#include <svkMRSCombine.h.h>


using namespace svk;

void ExecuteZeroFill( svkMrsZeroFill* zf, string infname, string outfname );
/*
 *  Application for testing data combination algorithms. 
 */
int main (int argc, char** argv)
{

    string fname(argv[1]);
    string fnameOut(argv[2]);

    //  Combine coils using straight addition 
    svkMrsZeroFill* zf = svkMrsZeroFill::New();
    zf->SetNumberOfSpecPointsToDouble( ); 
    zf->SetOutputWholeExtent(0, 12, 0, 10, 0, 11 );
    string bothFilename(fnameOut);
    bothFilename.append("_both");
    ExecuteZeroFill( zf, fname, bothFilename );

    zf->Delete();
    zf = NULL;

    //  Combine coils using straight addition 
    zf = svkMrsZeroFill::New();
    zf->SetNumberOfSpecPointsToNextPower2( ); 

    string spectralFilename(fnameOut);
    spectralFilename.append("_spectral");
    ExecuteZeroFill( zf, fname, spectralFilename );

    zf->Delete();
    zf = NULL;

    //  Combine coils using straight addition 
    zf = svkMrsZeroFill::New();
    zf->SetOutputWholeExtent(0, 20, 0, 20, 0, 20 );

    string spatialFilename(fnameOut);
    spatialFilename.append("_spatial");
    ExecuteZeroFill( zf, fname, spatialFilename );

    zf->Delete();
    zf = NULL;
    return 0; 
}


void ExecuteZeroFill( svkMrsZeroFill* zf, string infname, string outfname )
{
    svkDataModel* model = svkDataModel::New();
    svkImageData* data = model->LoadFile( infname.c_str() );
    data->Register(NULL);
    if( !data->IsA("svkMrsImageData") ) {
        cerr << "INPUT MUST BE SPECTRA!" << endl;
        exit(1);
    }

    //  FFT spectral data: time to frequency domain
    svkMrsImageFFT* spatialFFT = svkMrsImageFFT::New();
    spatialFFT->SetInput( data );
    spatialFFT->SetFFTDomain( svkMrsImageFFT::SPATIAL );
    spatialFFT->SetFFTMode( svkMrsImageFFT::FORWARD );
    spatialFFT->SetPostCorrectCenter( true );
    spatialFFT->Update();
    svkMrsImageData* targetData = svkMrsImageData::New();
    targetData->DeepCopy( spatialFFT->GetOutput() );
    //  Combine coils using straight addition
    zf->SetInput( targetData );
    zf->Update();

    //  Reverse FFT spatial data: kspace to spatial domain
    svkMrsImageFFT* spatialRFFT= svkMrsImageFFT::New();
    spatialRFFT->SetInput( targetData );
    spatialRFFT->SetFFTDomain( svkMrsImageFFT::SPATIAL );
    spatialRFFT->SetFFTMode( svkMrsImageFFT::REVERSE );
    spatialRFFT->SetPreCorrectCenter( true );
    spatialRFFT->Update();

    svkDdfVolumeWriter* writer = svkDdfVolumeWriter::New();
    writer->SetFileName( outfname.c_str() );
    writer->SetInput( spatialRFFT->GetOutput() );
    writer->Write();

    // Clean up
    spatialFFT->Delete();
    spatialRFFT->Delete();
    writer->Delete();
    writer = NULL;
    data->Delete();
    targetData->Delete();
    data = NULL;
    model->Delete();
    model = NULL;
}
