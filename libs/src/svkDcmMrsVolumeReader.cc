/*
 *  Copyright © 2009-2017 The Regents of the University of California.
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
#include </usr/include/vtk/vtkObjectFactory.h>
#include </usr/include/vtk/vtkDebugLeaks.h>
#include </usr/include/vtk/vtkInformation.h>

#include <svkMrsImageData.h>


using namespace svk;


//vtkCxxRevisionMacro(svkDcmMrsVolumeReader, "$Rev$");
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

    std::string fileToCheck(fname);

    if ( svkDcmHeader::IsFileDICOM( fname ) ) {
 
        svkImageData* tmp = svkMrsImageData::New(); 
        tmp->GetDcmHeader()->ReadDcmFile( fname ); 
        std::string SOPClassUID = tmp->GetDcmHeader()->GetStringValue( "SOPClassUID" ) ; 
        tmp->Delete(); 

        if ( SOPClassUID == "1.2.840.10008.5.1.4.1.1.4.2" ) {           

            vtkDebugMacro(<<this->GetClassName() << "::CanReadFile(): It's a DICOM MRS file " << fileToCheck );

            this->SetFileName(fname);

            return 1;
        }
    } 

    vtkDebugMacro(<<this->GetClassName() << "::CanReadFile() It's Not a DICOM MRS file " << fileToCheck );

    return 0;
}


/*! 
 *  Initializes any private DICOM attributes that are needed internally
 */
void svkDcmMrsVolumeReader::InitPrivateHeader()
{

    if( ! this->GetOutput()->GetDcmHeader()->ElementExists( "SVK_PRIVATE_TAG", "top") ) {
        this->GetOutput()->GetDcmHeader()->SetValue(  "SVK_PRIVATE_TAG", "SVK_PRIVATE_CREATOR"); 
    }

    //  If not present, add the SVK Domain attributes defaulted to spatial domain 
    //  Private Attributes for spatial domain encoding:
    if( ! this->GetOutput()->GetDcmHeader()->ElementExists( "SVK_ColumnsDomain", "top") ) {

        this->GetOutput()->GetDcmHeader()->SetValue( "SVK_ColumnsDomain",  "SPACE"); 
        this->GetOutput()->GetDcmHeader()->SetValue( "SVK_RowsDomain", "SPACE"); 
        this->GetOutput()->GetDcmHeader()->SetValue( "SVK_SliceDomain", "SPACE"); 
    }

}


/*! 
 *  
 */
void svkDcmMrsVolumeReader::LoadData( svkImageData* data )
{

    std::string dataRepresentation = this->GetOutput()->GetDcmHeader()->GetStringValue( "DataRepresentation" ); 
    int numComponents; 
    if ( dataRepresentation == "COMPLEX" ) {
        numComponents = 2; 
    } else {
        numComponents = 1; 
    }

    this->numFreqPts = this->GetOutput()->GetDcmHeader()->GetIntValue( "DataPointColumns" ); 

    //==============================
    svkDcmHeader::DimensionVector dimVector = this->GetOutput()->GetDcmHeader()->GetDimensionIndexVector(); 
    svkDcmHeader::DimensionVector loopVector = dimVector;
    int numCells = svkDcmHeader::GetNumberOfCells( &dimVector );

    long unsigned int dataLength = numCells * this->numFreqPts * numComponents; 
    float* specData = new float [ dataLength ];
    this->GetOutput()->GetDcmHeader()->GetFloatValue( "SpectroscopyData", specData, dataLength);  
    
    for (int cellID = 0; cellID < numCells; cellID++ ) {
        svkDcmHeader::GetDimensionVectorIndexFromCellID( &dimVector, &loopVector, cellID);
        SetCellSpectrum( data, &loopVector, numComponents, specData );
    }
    //==============================


    //==============================
    /*
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
    */
    //==============================


    svkFastCellData::SafeDownCast(data->GetCellData())->FinishFastAdd();

    delete [] specData; 

    //this->GetOutput()->GetDcmHeader()->PrintDcmHeader(); 
    this->GetOutput()->GetDcmHeader()->ClearElement( "SpectroscopyData" ); 
}


/*!
 *
 */
void svkDcmMrsVolumeReader::SetCellSpectrum( svkImageData* data, svkDcmHeader::DimensionVector* loopVector, int numComponents, float* specData )
{

    //  Set XY points to plot - 2 components per tuble for complex data sets:
    vtkDataArray* dataArray = vtkDataArray::CreateDataArray(VTK_FLOAT);
    dataArray->SetNumberOfComponents( numComponents );

    dataArray->SetNumberOfTuples(this->numFreqPts);
    string arrayName = svk4DImageData::GetArrayName( loopVector ); 
    dataArray->SetName( arrayName.c_str() );

    svkDcmHeader::DimensionVector dimVector = this->GetOutput()->GetDcmHeader()->GetDimensionIndexVector(); 
    int cellID = svkDcmHeader::GetCellIDFromDimensionVectorIndex( &dimVector, loopVector);
    int offset = cellID * this->numFreqPts * numComponents;


    //  According to DICOM Part 3 C.8.14.4.1, data should be ordered from low to high frequency, 
    //  Or as the complex conjugate:
    string signalDomain = this->GetOutput()->GetDcmHeader()->GetStringValue( "SignalDomainColumns" );
    bool isTimeDomain = false;
    if ( signalDomain.compare("TIME") == 0 ) {
        isTimeDomain = true;
    }

    for (int i = 0; i < this->numFreqPts; i++) {
        // see previous comment about DICOM convention for time domain data. 
        if ( isTimeDomain ) {
            specData[ offset + (i * 2) + 1 ] *= -1;     //invert the imaginary component
        }
        dataArray->SetTuple( i, &(specData[ offset + (i * 2) ]) );
    }

    //  Add the spectrum's dataArray to the CellData:
    //  vtkCellData is a subclass of vtkFieldData
    svkFastCellData::SafeDownCast(data->GetCellData())->FastAddArray(dataArray); 

    

    //  Should these be a member var, deleted in destructor?
    dataArray->Delete();

    return;
}

        

/*!
 *
 */
void svkDcmMrsVolumeReader::SetCellSpectrum(svkImageData* data, int x, int y, int z, int timePt, int coilNum, int numComponents, float* specData)
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


    //  According to DICOM Part 3 C.8.14.4.1, data should be ordered from low to high frequency, 
    //  Or as the complex conjugate:
    string signalDomain = this->GetOutput()->GetDcmHeader()->GetStringValue( "SignalDomainColumns" );
    bool isTimeDomain = false;
    if ( signalDomain.compare("TIME") == 0 ) {
        isTimeDomain = true;
    }

    for (int i = 0; i < this->numFreqPts; i++) {
        // see previous comment about DICOM convention for time domain data. 
        if ( isTimeDomain ) {
            specData[ offset + (i * 2) + 1 ] *= -1;     //invert the imaginary component
        }
        dataArray->SetTuple( i, &(specData[ offset + (i * 2) ]) );
    }

    //  Add the spectrum's dataArray to the CellData:
    //  vtkCellData is a subclass of vtkFieldData
    svkFastCellData::SafeDownCast(data->GetCellData())->FastAddArray(dataArray); 

    

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

