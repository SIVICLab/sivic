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


#include <svkImageData.h>
#include <svkMrsImageData.h>
#include <svkMriImageData.h>
#include <svkDataModel.h>

using namespace svk;

int main (int argc, char** argv)
{
    string fname(argv[1]);

    svkDataModel* model = svkDataModel::New();

    // Define a dcos
    double dcos_in[3][3] = { { 10, 10, 10 }, { 10, 10, 10 }, { 10, 10, 10 } };
    svkImageData* mri = model->AddFileToModel( "image", fname );
    mri->Update();
    cout << "Image: " << *mri << endl;
    mri->SetDcos(dcos_in);
    svkImageData* mriCopy = svkMriImageData::New();
    mriCopy->DeepCopy( mri, svkDcmHeader::UNSIGNED_INT_2);
    mriCopy->ComputeBounds( );
    mriCopy->Update( );
    cout << "Image Copy: " << *mriCopy << endl;
    double dcos_out[3][3];
    mriCopy->GetDcos(dcos_out);

    assert( dcos_in[0][0] == dcos_out[0][0] );
    assert( dcos_in[0][1] == dcos_out[0][1] );
    assert( dcos_in[0][2] == dcos_out[0][2] );
    assert( dcos_in[1][0] == dcos_out[1][0] );
    assert( dcos_in[1][1] == dcos_out[1][1] );
    assert( dcos_in[1][2] == dcos_out[1][2] );
    assert( dcos_in[2][0] == dcos_out[2][0] );
    assert( dcos_in[2][1] == dcos_out[2][1] );
    assert( dcos_in[2][2] == dcos_out[2][2] );
    assert( mri->GetCellData()->GetNumberOfArrays() == mriCopy->GetCellData()->GetNumberOfArrays());
    assert( mri->GetPointData()->GetNumberOfArrays() == mriCopy->GetPointData()->GetNumberOfArrays());

    //mriCopy->CastDataFormat( svkDcmHeader::SIGNED_FLOAT_4 );
    mriCopy->CastDataFormat( svkDcmHeader::UNSIGNED_INT_2 );

    double* originalTuple;
    double* copiedTuple;
    cout << "Image Copy Post Cast: " << *mriCopy << endl;
    for( int i = 0; i < mri->GetPointData()->GetNumberOfArrays(); i++ ) {
        for( int j = 0; j < mri->GetPointData()->GetArray(i)->GetNumberOfTuples(); j++ ) {
            originalTuple = mri->GetPointData()->GetArray(i)->GetTuple( j );
            copiedTuple = mriCopy->GetPointData()->GetArray(i)->GetTuple( j );
            for( int k = 0; k < mri->GetPointData()->GetArray(i)->GetNumberOfComponents(); k++) {
                assert( originalTuple[k] == copiedTuple[k] );
            }
        }
    }

    for( int i = 0; i < mri->GetCellData()->GetNumberOfArrays(); i++ ) {
        for( int j = 0; j < mri->GetCellData()->GetArray(i)->GetNumberOfTuples(); j++ ) {
            originalTuple = mri->GetCellData()->GetArray(i)->GetTuple( j );
            copiedTuple = mriCopy->GetCellData()->GetArray(i)->GetTuple( j );
            for( int k = 0; k < mri->GetCellData()->GetArray(i)->GetNumberOfComponents(); k++) {
                assert( originalTuple[k] == copiedTuple[k] );
            }
        }
    }
    svkImageData* zeroCopy = svkMriImageData::New();
    zeroCopy->ZeroCopy( mri, svkDcmHeader::SIGNED_FLOAT_4 );
    zeroCopy->Update();
    cout<< "zero Copy: " << *zeroCopy << endl;

    double* zeroTuple;
    for( int i = 0; i < zeroCopy->GetCellData()->GetNumberOfArrays(); i++ ) {
        for( int j = 0; j < zeroCopy->GetCellData()->GetArray(i)->GetNumberOfTuples(); j++ ) {
            zeroTuple = zeroCopy->GetCellData()->GetArray(i)->GetTuple( j );
            for( int k = 0; k < zeroCopy->GetCellData()->GetArray(i)->GetNumberOfComponents(); k++) {
                assert( zeroTuple[k] == 0 );
            }
        }
    }

    for( int i = 0; i < zeroCopy->GetPointData()->GetNumberOfArrays(); i++ ) {
        for( int j = 0; j < zeroCopy->GetPointData()->GetArray(i)->GetNumberOfTuples(); j++ ) {
            zeroTuple = zeroCopy->GetPointData()->GetArray(i)->GetTuple( j );
            for( int k = 0; k < zeroCopy->GetPointData()->GetArray(i)->GetNumberOfComponents(); k++) {
                assert( zeroTuple[k] == 0 );
            }
        }
    }

    svkImageData* structureCopy = svkMriImageData::New();
    structureCopy->CopyStructure( mri );
    structureCopy->Update();
    cout<< "Structure Copy: " << *structureCopy << endl;

    if( mri->IsA("svkMriImageData") ) {
        double* pixels = static_cast<svkMriImageData*>(mri)->GetImagePixels( 20 );
        vtkDataArray* pixelsArray = static_cast<svkMriImageData*>(mri)->GetImagePixelsArray( 20 );
        for( int i = 0; i < pixelsArray->GetNumberOfTuples(); i++ ) {
            cout<< "double: "<<pixels[i] << " Array: " << pixelsArray->GetTuple1(i) << endl;
            assert( pixelsArray->GetTuple1(i) == pixels[i] );
        }
        cout << "DataArray: " << *pixelsArray << endl; 
        pixelsArray->Delete();
    }
    mri->Delete();
    mriCopy->Delete();

    cout << "COPY TEST PASSED" << endl;
    return 0; 
}


