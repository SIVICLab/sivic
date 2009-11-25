/*
 *  Copyright © 2009 The Regents of the University of California.
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
 */


#include <svkDcmVolumeReader.h>


using namespace svk;


vtkCxxRevisionMacro(svkDcmVolumeReader, "$Rev$");


/*!
 *
 */
svkDcmVolumeReader::svkDcmVolumeReader()
{
#if VTK_DEBUG_ON
    this->DebugOn();
    vtkDebugLeaks::ConstructClass("svkDcmVolumeReader");
#endif
    vtkDebugMacro(<<this->GetClassName() << "::" << this->GetClassName() << "()");
}


/*!
 *
 */
svkDcmVolumeReader::~svkDcmVolumeReader()
{
    vtkDebugMacro(<<this->GetClassName() << "::~" << this->GetClassName() << "()");
}



/*!
 *  Side effect of Update() method.  Used to load pixel data and initialize vtkImageData
 *  Called before ExecuteData()
 */
void svkDcmVolumeReader::ExecuteInformation()
{

    vtkDebugMacro( << this->GetClassName() << "::ExecuteInformation()" );

    if (this->FileName == NULL) {
        return;
    }

    if ( this->FileName ) {

        struct stat fs;
        if ( stat(this->FileName, &fs) ) {
            vtkErrorMacro("Unable to open file " << this->FileName );
            return;
        }

        this->InitDcmHeader();
        this->SetupOutputInformation();

    }

}


/*!
 *  Side effect of Update() method.  Used to load pixel data and initialize vtkImageData
 *  Called after ExecuteInformation()
 */
void svkDcmVolumeReader::ExecuteData(vtkDataObject* output)
{

    vtkDebugMacro( << this->GetClassName() << "::ExecuteData()" );

    svkImageData* data = svkImageData::SafeDownCast( this->AllocateOutputData(output) );

    if ( this->FileName ) {

        struct stat fs;
        if ( stat(this->FileName, &fs) ) {
            vtkErrorMacro("Unable to open file " << this->FileName );
            return;
        }

    }

    this->LoadData(data);

    this->numFrames = data->GetDcmHeader()->GetIntValue( "NumberOfFrames");
    if (this->numFrames > 1) {
        double origin0[3];
        this->GetOutput()->GetDcmHeader()->GetOrigin(origin0, 0);
        double origin1[3];
        this->GetOutput()->GetDcmHeader()->GetOrigin(origin0, this->numFrames);

        //  Determine whether the data is ordered with or against the slice normal direction.
        double normal[3];
        this->GetOutput()->GetDcmHeader()->GetNormalVector(normal);
   
        //  Get vector from first to last image and get the dot product of that vector with the normal:
        double dcosSliceOrder[3];
        for (int j = 0; j < 3; j++) {
            dcosSliceOrder[j] =  origin1[j] - origin0[j];
        }
   
        //  Use the scalar product to determine whether the data in the .cmplx
        //  file is ordered along the slice normal or antiparalle to it.
        vtkMath* math = vtkMath::New();
        if (math->Dot(normal, dcosSliceOrder) > 0 ) {
            this->dataSliceOrder = svkDcmHeader::INCREMENT_ALONG_POS_NORMAL;
        } else {
            this->dataSliceOrder = svkDcmHeader::INCREMENT_ALONG_NEG_NORMAL;
        }
    } else {
        this->dataSliceOrder = svkDcmHeader::INCREMENT_ALONG_POS_NORMAL;
    }

    double dcos[3][3];
    this->GetOutput()->GetDcmHeader()->SetSliceOrder( this->dataSliceOrder );

    this->GetOutput()->GetDcmHeader()->GetDataDcos( dcos );
    this->GetOutput()->SetDcos(dcos);

    //  SetNumberOfIncrements is supposed to call this, but only works if the data has already
    //  been allocated. but that requires the number of components to be specified.
    this->GetOutput()->GetIncrements();
}


/*!
 *
 */
void svkDcmVolumeReader::InitDcmHeader()
{
    this->GetOutput()->GetDcmHeader()->ReadDcmFile( this->FileName, 100000000 ); 
}


