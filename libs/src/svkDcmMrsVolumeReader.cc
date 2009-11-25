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

#include <svkDcmMrsVolumeReader.h>


using namespace svk;


vtkCxxRevisionMacro(svkDcmMrsVolumeReader, "$Rev$");
vtkStandardNewMacro(svkDcmMrsVolumeReader);


/*!
 *
 */
svkDcmMrsVolumeReader::svkDcmMrsVolumeReader()
{
#if VTK_DEBUG_ON
    this->DebugOn();
    vtkDebugLeaks::ConstructClass("svkDcmMrsVolumeReader");
#endif
    vtkDebugMacro(<<this->GetClassName() << "::" << this->GetClassName() << "()");
}


/*!
 *
 */
svkDcmMrsVolumeReader::~svkDcmMrsVolumeReader()
{
    vtkDebugMacro(<<this->GetClassName() << "::~" << this->GetClassName() << "()");
}


/*!
 *  Mandator, must be overridden to get the factory to check for proper
 *  type of vtkImageReader2 to return. 
 */
int svkDcmMrsVolumeReader::CanReadFile(const char* fname)
{

    string fileToCheck(fname);

    if( fileToCheck.size() > 4 ) {

        if ( 
            fileToCheck.substr( fileToCheck.size() - 4 ) == ".dcm"  || 
            fileToCheck.substr( fileToCheck.size() - 4 ) == ".DCM" 
        )  {

            this->GetOutput()->GetDcmHeader()->ReadDcmFile( fname ); 
            string SOPClassUID = this->GetOutput()->GetDcmHeader()->GetStringValue( "SOPClassUID" ) ; 

            if ( SOPClassUID == "1.2.840.10008.5.1.4.1.1.4.2" ) {           

                cout << this->GetClassName() << "::CanReadFile(): It's a DICOM MRS File: " <<  fileToCheck << endl;

                SetFileName(fname);

                return 1;
            }

        }

    } 

    vtkDebugMacro(<<this->GetClassName() << "::CanReadFile() It's Not a DICOM MRS file " << fileToCheck );

    return 0;
}


/*! 
 *  
 */
void svkDcmMrsVolumeReader::LoadData( svkImageData* data )
{

    string dataRepresentation = this->GetOutput()->GetDcmHeader()->GetStringValue( "DataRepresentation" ); 
    int numComponents; 
    if ( dataRepresentation == "COMPLEX" ) {
        numComponents = 2; 
    } else {
        numComponents = 1; 
    }

    int numFreqPts = this->GetOutput()->GetDcmHeader()->GetIntValue( "DataPointColumns" ); 

    int numVoxels[3]; 
    data->GetNumberOfVoxels(numVoxels); 

    long unsigned int dataLength = numVoxels[0] * numVoxels[1] * numVoxels[2] * numFreqPts * numComponents;

    float* specData = new float [ dataLength ];
    this->GetOutput()->GetDcmHeader()->GetFloatValue( "SpectroscopyData", specData, dataLength);  

    for (int z = 0; z < (this->GetDataExtent())[5] ; z++) {
        for (int y = 0; y < (this->GetDataExtent())[3]; y++) {
            for (int x = 0; x < (this->GetDataExtent())[1]; x++) {
                SetCellSpectrum( data, x, y, z, numComponents, specData );
            }
        }
    }

    delete [] specData; 

    //this->GetOutput()->GetDcmHeader()->PrintDcmHeader(); 
    this->GetOutput()->GetDcmHeader()->ClearElement( "SpectroscopyData" ); 
}


/*!
 *
 */
void svkDcmMrsVolumeReader::SetCellSpectrum(svkImageData* data, int x, int y, int z, int numComponents, float* specData)
{

    //  Set XY points to plot - 2 components per tuble for complex data sets:
    vtkDataArray* dataArray = vtkDataArray::CreateDataArray(VTK_FLOAT);
    dataArray->SetNumberOfComponents( numComponents );

    int numFreqPts = this->GetOutput()->GetDcmHeader()->GetIntValue( "DataPointColumns" ); 
    dataArray->SetNumberOfTuples(numFreqPts);
    char arrayName[30];
    sprintf(arrayName, "%d %d %d 0 0", x, y, z);
    dataArray->SetName(arrayName);

    int numVoxels[3]; 
    data->GetNumberOfVoxels(numVoxels); 
    int offset = (z * numVoxels[0] * numVoxels[1] 
               + y * numVoxels[0]
               + x) * numFreqPts * numComponents;    


    for (int i = 0; i < numFreqPts; i++) {
        dataArray->SetTuple( i, &(specData[ offset + (i * 2) ]) );
    }

    //  Add the spectrum's dataArray to the CellData:
    //  vtkCellData is a subclass of vtkFieldData
    data->GetCellData()->AddArray(dataArray);

    //  Should these be a member var, deleted in destructor?
    dataArray->Delete();

    return;
}


/*!
 *  Returns the file root without extension
 */
svkDcmHeader::DcmPixelDataFormat svkDcmMrsVolumeReader::GetFileType()
{
    return svkDcmHeader::SIGNED_FLOAT_4;
}


/*!
 *
 */
int svkDcmMrsVolumeReader::FillOutputPortInformation( int vtkNotUsed(port), vtkInformation* info )
{
    info->Set(vtkDataObject::DATA_TYPE_NAME(), "svkMrsImageData");
    return 1;
}

