/*
 *  Copyright © 2009-2010 The Regents of the University of California.
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



#include <svkImageReader2.h>


using namespace svk;


vtkCxxRevisionMacro(svkImageReader2, "$Rev$");


/*!
 *
 */
svkImageReader2::svkImageReader2()
{

#if VTK_DEBUG_ON
    this->DebugOn();
#endif

    vtkDebugMacro(<<"svkImageReader2::svkImageReader2()");

    vtkInstantiator::RegisterInstantiator("svkMriImageData", svkMriImageData::NewObject);
    vtkInstantiator::RegisterInstantiator("svkMrsImageData", svkMrsImageData::NewObject);

    this->dataArray = NULL;

}


/*!
 *
 */
svkImageReader2::~svkImageReader2()
{
    vtkDebugMacro(<<"svkImageReader2::~svkImageReader2()");
    if ( this->GetDebug() ) {
        cout << "+++++++++++++++++++++++++++++++++++++++++" << endl;
        cout << "delete image reader 2" << endl;
        cout << "+++++++++++++++++++++++++++++++++++++++++" << endl;
    }

    if (this->dataArray != NULL) {
        this->dataArray->Delete();
        this->dataArray = NULL;
    }

}


/*!
 *  Returns the file root without extension (will include any path elements)
 */
string svkImageReader2::GetFileRoot(const char* fname)
{
    string volumeFileName(fname);
    size_t position;
    position = volumeFileName.find_last_of( "." );
    string fileRoot( volumeFileName.substr(0, position) );
    return fileRoot;
}


/*!
 *  Returns the file root without extension
 */
string svkImageReader2::GetFileExtension(const char* fname)
{
    string volumeFileName(fname);
    size_t position;
    position = volumeFileName.find_last_of( "." );
    string fileExtension( volumeFileName.substr(position + 1) );
    return fileExtension;
}


/*!
 *  Returns the file path:  everything before the last "/".   
 */
string svkImageReader2::GetFilePath(const char* fname)
{
    string volumeFileName(fname);
    size_t position;
    position = volumeFileName.find_last_of( "/" );
    string filePath; 
    if ( position != string::npos ) {
        filePath.assign( volumeFileName.substr(0, position) );
    } else {
        filePath.assign( "." );
    }
    return filePath;
}


/*
 *  Strips leading and trailing white space from string
 */
string svkImageReader2::StripWhite(string in)
{
    string stripped;
    string stripped_leading(in);
    size_t firstNonWhite;
    size_t lastWhite;

    //  Remove leading spaces:
    firstNonWhite = in.find_first_not_of(' ');
    if (firstNonWhite != string::npos) {
        stripped_leading.assign( in.substr(firstNonWhite) );
    }

    //  Remove trailing spaces:
    while (lastWhite = stripped_leading.find_last_of(' ') != string::npos && (lastWhite = stripped_leading.find_last_of(' ')) == stripped.length() ) {
        stripped_leading.assign( stripped_leading.substr(0, lastWhite) );
    }
    stripped = stripped_leading;

    return stripped;
}


/*!
 *  Use svkDcmHeader content to set up Output Information about the
 *  target svkImageData object.
 */
void svkImageReader2::SetupOutputInformation()
{

    svkDcmHeader* dcmHdr = this->GetOutput()->GetDcmHeader();

    //  ============================
    //  Set Dimensionality
    //  ============================
    this->SetFileDimensionality(3);

    //  ============================
    //  Set Extent
    //  ============================
    this->SetupOutputExtent(); 

    //  ============================
    //  Set Voxel Spacing (size)
    //  ============================
    double spacing[3];
    dcmHdr->GetPixelSpacing(spacing);
    this->SetDataSpacing(spacing);

    //  ============================
    //  Set Origin
    //  ============================
    double origin[3];
    dcmHdr->GetOrigin(origin);

    if ( strcmp( this->GetOutput()->GetClassName(), "svkMrsImageData") == 0 ) {
        double pixelSize[3];
        dcmHdr->GetPixelSize(pixelSize);

        double dcos[3][3];
        dcmHdr->GetDataDcos(dcos); 

        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                origin[i] -= (pixelSize[j]/2) * dcos[j][i];
            }
        }
    }

    this->SetDataOrigin( origin );

    this->SetupOutputScalarData(); 

    this->vtkImageReader2::ExecuteInformation();
}


/*!
 *  
 */
void svkImageReader2::SetupOutputExtent()
{
    int numVoxels[3];
    numVoxels[0] = this->GetOutput()->GetDcmHeader()->GetIntValue("Columns");
    numVoxels[1] = this->GetOutput()->GetDcmHeader()->GetIntValue("Rows");
    int numberOfFrames = this->GetOutput()->GetDcmHeader()->GetIntValue("NumberOfFrames");
    numVoxels[2] = numberOfFrames; 

    int numCoils =  this->GetOutput()->GetDcmHeader()->GetNumberOfCoils();
    numVoxels[2] /= numCoils;  

    if ( strcmp( this->GetOutput()->GetClassName(), "svkMrsImageData") == 0 ) {
        this->SetDataExtent(
            0,
            numVoxels[0],
            0,
            numVoxels[1],
            0,
            numVoxels[2]
        );
    } else if ( strcmp(this->GetOutput()->GetClassName(), "svkMriImageData") == 0 ) {
        this->SetDataExtent(
            0,
            numVoxels[0] - 1,
            0,
            numVoxels[1] - 1,
            0,
            numVoxels[2] - 1
        );
    }
}


/*!
 *  Set scalar data information for MRI data: 
 */
void svkImageReader2::SetupOutputScalarData()
{

    //  only MRI data has scalar data
    if ( strcmp(this->GetOutput()->GetClassName(), "svkMriImageData") == 0 ) {

        //  ============================
        //  Set data type:
        //  ============================
        if (  this->GetFileType() == svkDcmHeader::UNSIGNED_INT_1 ) {
            this->SetDataScalarTypeToUnsignedChar();
            this->dataArray = vtkUnsignedCharArray::New();
            this->GetOutput()->GetDcmHeader()->SetPixelDataType( svkDcmHeader::UNSIGNED_INT_1 );
        } else if (  this->GetFileType() == svkDcmHeader::UNSIGNED_INT_2 ) {
            this->SetDataScalarTypeToUnsignedShort();
            this->dataArray = vtkUnsignedShortArray::New();
            this->GetOutput()->GetDcmHeader()->SetPixelDataType( svkDcmHeader::UNSIGNED_INT_2 );
        } else if (  this->GetFileType() == svkDcmHeader::SIGNED_FLOAT_4 ) {
            this->SetDataScalarTypeToFloat();
            this->dataArray = vtkFloatArray::New();
            this->GetOutput()->GetDcmHeader()->SetPixelDataType( svkDcmHeader::SIGNED_FLOAT_4 );
        } else {
            vtkErrorWithObjectMacro( this, "Unsupported data type: " << this->GetFileType() );
        }

        this->SetNumberOfScalarComponents(1);

    } else if ( strcmp(this->GetOutput()->GetClassName(), "svkMrsImageData") == 0 ) {

        if (  this->GetFileType() == svkDcmHeader::SIGNED_FLOAT_4 ) {
            this->GetOutput()->GetDcmHeader()->SetPixelDataType( svkDcmHeader::SIGNED_FLOAT_4 );
        } else {
            vtkErrorWithObjectMacro( this, "Unsupported MRS data type: " << this->GetFileType() );
        }

    }

}



/*!
 *
 */
int svkImageReader2::FillOutputPortInformation( int vtkNotUsed(port), vtkInformation* info )
{
    info->Set( vtkDataObject::DATA_TYPE_NAME(), this->GetImageDataInput(0)->GetClassName() );
    return 1;
}


/*!
 *
 */
svkImageData* svkImageReader2::GetOutput()
{
    return this->GetOutput(0);
}


/*!
 *
 */
svkImageData* svkImageReader2::GetOutput(int port)
{
    return svkImageData::SafeDownCast(this->GetOutputDataObject(port));
}



/*!
 *  Remove slashes from idf date and reorder for DICOM compliance:
 *  07/25/2007 -> 20070725
 */
string svkImageReader2::RemoveSlashesFromDate(string* volumeDate)
{
        size_t delim;
        delim = volumeDate->find_first_of('/');
        string month = volumeDate->substr(0, delim);
        month = StripWhite(month);
        if (month.size() != 2) {
            month = "0" + month;
        }

        string dateSub;
        dateSub = volumeDate->substr(delim + 1);
        delim = dateSub.find_first_of('/');
        string day = dateSub.substr(0, delim);
        day = StripWhite(day);
        if (day.size() != 2) {
            day = "0" + day;
        }

        dateSub = dateSub.substr(delim + 1);
        delim = dateSub.find_first_of('/');
        string year = dateSub.substr(0, delim);
        year = StripWhite(year);

        return year+month+day;
}


/*!
 *
 */
svkDcmHeader* svkImageReader2::GetDcmHeader( const char* fileName)
{
    this->SetFileName( fileName );
    return this->GetOutput()->GetDcmHeader();
}
