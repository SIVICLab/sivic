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
 */

#include <svkGEPostageStampReader.h>
#include <vtkObjectFactory.h>
#include <vtkDebugLeaks.h>
#include <vtkInformation.h>

#include <svkMrsImageData.h>


using namespace svk;


vtkCxxRevisionMacro(svkGEPostageStampReader, "$Rev$");
vtkStandardNewMacro(svkGEPostageStampReader);


/*!
 *
 */
svkGEPostageStampReader::svkGEPostageStampReader()
{
#if VTK_DEBUG_ON
    this->DebugOn();
    vtkDebugLeaks::ConstructClass("svkGEPostageStampReader");
#endif
    vtkDebugMacro(<<this->GetClassName() << "::" << this->GetClassName() << "()");
}


/*!
 *
 */
svkGEPostageStampReader::~svkGEPostageStampReader()
{
    vtkDebugMacro(<<this->GetClassName() << "::~" << this->GetClassName() << "()");
}


/*!
 *  Mandatory, must be overridden to get the factory to check for proper
 *  type of vtkImageReader2 to return. 
 */
int svkGEPostageStampReader::CanReadFile(const char* fname)
{

    vtkstd::string fileToCheck(fname);
    bool isGEPostage = false; 

    if ( svkDcmHeader::IsFileDICOM( fname ) ) {
 
        svkImageData* tmp = svkMrsImageData::New(); 
        tmp->GetDcmHeader()->ReadDcmFile( fname ); 
        vtkstd::string SOPClassUID = tmp->GetDcmHeader()->GetStringValue( "SOPClassUID" ) ; 

        // Check for MR Image Storage
        if ( SOPClassUID == "1.2.840.10008.5.1.4.1.1.4" ) {

            //  Check for proprietary use of DICOM MR ImageStorage:
            if ( this->ContainsProprietaryContent( tmp ) == true ) {
                this->SetFileName(fname);
                isGEPostage = true;
            }

        }

        tmp->Delete(); 
    } 

    if ( isGEPostage ) { 
            cout << this->GetClassName() << "::CanReadFile(): It's a GE Postage Stamp File: " <<  fileToCheck << endl;
            this->SetFileName(fname);
            return 1;

    } else {

        vtkDebugMacro(<<this->GetClassName() << "::CanReadFile() It's Not a GE Postage Stamp file " << fileToCheck );

    }

    return 0;

}


/*!
 *  Initializes any private DICOM attributes that are needed internally
 */
void svkGEPostageStampReader::InitPrivateHeader()
{
}


/*! 
 *  
 */
void svkGEPostageStampReader::LoadData( svkImageData* data )
{

    vtkstd::string dataRepresentation = this->GetOutput()->GetDcmHeader()->GetStringValue( "DataRepresentation" ); 
    int numComponents; 
    if ( dataRepresentation == "COMPLEX" ) {
        numComponents = 2; 
    } else {
        numComponents = 1; 
    }

    this->numFreqPts = this->GetOutput()->GetDcmHeader()->GetIntValue( "DataPointColumns" ); 
    this->numTimePts = this->GetOutput()->GetDcmHeader()->GetNumberOfTimePoints();
    int numCoils = this->GetOutput()->GetDcmHeader()->GetNumberOfCoils();

    int numVoxels[3]; 
    data->GetNumberOfVoxels(numVoxels); 

    long unsigned int dataLength = 
        numVoxels[0] * numVoxels[1] * numVoxels[2] * this->numFreqPts * numComponents * numCoils * this->numTimePts;

    float* specData = new float [ dataLength ];
    this->GetOutput()->GetDcmHeader()->GetFloatValue( "SpectroscopyData", specData, dataLength);  
        
    for (int coilNum = 0; coilNum < numCoils; coilNum ++) {
        for (int timePt = 0; timePt < this->numTimePts; timePt ++) {
            for (int z = 0; z < (this->GetDataExtent())[5] ; z++) {
                for (int y = 0; y < (this->GetDataExtent())[3]; y++) {
                    for (int x = 0; x < (this->GetDataExtent())[1]; x++) {
                        SetCellSpectrum( data, x, y, z, timePt, coilNum, numComponents, specData );
                    }
                }
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
void svkGEPostageStampReader::SetCellSpectrum(svkImageData* data, int x, int y, int z, int timePt, int coilNum, int numComponents, float* specData)
{

    //  Set XY points to plot - 2 components per tuble for complex data sets:
    vtkDataArray* dataArray = vtkDataArray::CreateDataArray(VTK_FLOAT);
    dataArray->SetNumberOfComponents( numComponents );

    dataArray->SetNumberOfTuples(this->numFreqPts);
    char arrayName[30];
    sprintf(arrayName, "%d %d %d %d %d", x, y, z, timePt, coilNum);
    dataArray->SetName(arrayName);

    int numVoxels[3]; 
    data->GetNumberOfVoxels(numVoxels); 
    int cols = numVoxels[0];
    int rows = numVoxels[1];
    int slices = numVoxels[2];
    int coilOffset = cols * rows * slices * this->numTimePts;     //number of spectra per coil
    int timePtOffset = cols * rows * slices;

    int offset = ((( cols * rows * z ) + ( cols * y ) + x )  
             + ( timePt * timePtOffset ) + ( coilNum * coilOffset ) ) * this->numFreqPts * numComponents;


    for (int i = 0; i < this->numFreqPts; i++) {
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
svkDcmHeader::DcmPixelDataFormat svkGEPostageStampReader::GetFileType()
{
    return svkDcmHeader::SIGNED_FLOAT_4;
}


/*!
 *
 */
int svkGEPostageStampReader::FillOutputPortInformation( int vtkNotUsed(port), vtkInformation* info )
{
    info->Set(vtkDataObject::DATA_TYPE_NAME(), "svkMrsImageData");
    return 1;
}

