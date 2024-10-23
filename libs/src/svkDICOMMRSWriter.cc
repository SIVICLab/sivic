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


#include <svkDICOMMRSWriter.h>


using namespace svk;


//vtkCxxRevisionMacro(svkDICOMMRSWriter, "$Rev$");
vtkStandardNewMacro(svkDICOMMRSWriter);


/*!
 *
 */
svkDICOMMRSWriter::svkDICOMMRSWriter()
{

#if VTK_DEBUG_ON
    this->DebugOn();
#endif

    vtkDebugMacro(<< "svkDICOMMRSWriter::svkDICOMMRSWriter");

}


/*!
 *
 */
svkDICOMMRSWriter::~svkDICOMMRSWriter()
{
}



/*!
 *  Write the DICOM MR Spectroscopy multi-frame file.   Also initializes the 
 *  DICOM SpectroscopyData element from the svkImageData object. 
 */
void svkDICOMMRSWriter::Write()
{

    //  Make sure the series is unique: 
    this->GetImageDataInput(0)->GetDcmHeader()->MakeDerivedDcmHeader(); 

    this->SetErrorCode(vtkErrorCode::NoError);

    if (! this->FileName ) {
        vtkErrorMacro(<<"Write:Please specify either a FileName or a file prefix and pattern");
        this->SetErrorCode(vtkErrorCode::NoFileNameError);
        return;
    }

    // Make sure the file name is allocated
    this->InternalFileName =
        new char[(this->FileName ? strlen(this->FileName) : 1) +
        (this->FilePrefix ? strlen(this->FilePrefix) : 1) +
        (this->FilePattern ? strlen(this->FilePattern) : 1) + 10];

    this->FileNumber = 0;
    this->MinimumFileNumber = this->FileNumber;
    this->FilesDeleted = 0;
    this->UpdateProgress(0.0);

    this->MaximumFileNumber = this->FileNumber;

    // determine the name
    if (this->FileName) {
        sprintf(this->InternalFileName,"%s.%s",this->FileName, this->FilePrefix);
    } else {
        if (this->FilePrefix) {
            sprintf(this->InternalFileName, this->FilePattern,
                    this->FilePrefix, this->FileNumber);
        } else {
            sprintf(this->InternalFileName, this->FilePattern, this->FileNumber);
        }
    }

    this->InitSpectroscopyData();

    this->GetImageDataInput(0)->GetDcmHeader()->WriteDcmFile(this->InternalFileName); 

    if (this->ErrorCode == vtkErrorCode::OutOfDiskSpaceError) {
        vtkErrorMacro("Ran out of disk space; deleting file(s) already written");
        this->DeleteFiles();
        return;
    }

    delete [] this->InternalFileName;
    this->InternalFileName = NULL;

    //  Clear the SpectroscopyData element: 
    this->GetImageDataInput(0)->GetDcmHeader()->ClearElement( "SpectroscopyData" ); 
}



/*!
 *  Write the spectral data points to the SpectroscopyData DICOM element.       
 */
void svkDICOMMRSWriter::InitSpectroscopyData()
{

    vtkDebugMacro(<<"svkDICOMSCWRiter::WriteSlice()");

    int specPts = this->GetImageDataInput(0)->GetDcmHeader()->GetIntValue( "DataPointColumns" ); 
    string representation = this->GetImageDataInput(0)->GetDcmHeader()->GetStringValue( "DataRepresentation" ); 

    int numComponents = 1; 
    if (representation.compare("COMPLEX") == 0) {
        numComponents = 2; 
    }

    vtkCellData* cellData = this->GetImageDataInput(0)->GetCellData();

    svkDcmHeader::DimensionVector dimensionVector = this->GetImageDataInput(0)->GetDcmHeader()->GetDimensionIndexVector(); 
    int numCells = svkDcmHeader::GetNumberOfCells( &dimensionVector); 

    int dataLength = numCells * specPts * numComponents ;

    float* specData = new float [ dataLength ];
    int index = 0;

    vtkFloatArray* fa; 
    float* dataTuple = new float[numComponents];  
      
    string signalDomain = this->GetImageDataInput(0)->GetDcmHeader()->GetStringValue( "SignalDomainColumns" ); 
    bool isTimeDomain = false; 
    if ( signalDomain.compare("TIME") == 0 ) {
        isTimeDomain = true; 
    }

    for (int cellID = 0; cellID < numCells; cellID++ ) { 

        fa =  vtkFloatArray::SafeDownCast( cellData->GetArray( cellID ) );
        
        for (int i = 0; i < specPts; i++) {
        
            fa->GetTypedTuple(i, dataTuple);  

            //  According to DICOM Part 3 C.8.14.4.1, time domain data should be 
            //  ordered from low to high frequency (or as the complex conjugate):
            if (numComponents == 2 && isTimeDomain ) {
                dataTuple[1] *= -1;     //invert the imaginary component
            }
        
            for (int j = 0; j < numComponents; j++) {
                specData[ (cellID * specPts * numComponents) + (i * numComponents) + j ] = dataTuple[j];
            }
        }
    }
    delete [] dataTuple; 
        
    this->GetImageDataInput(0)->GetDcmHeader()->SetValue(
        "SpectroscopyData",
        specData, 
        dataLength 
    );
   
    delete [] specData; 
}


/*!
 *
 */
int svkDICOMMRSWriter::FillInputPortInformation( int vtkNotUsed(port), vtkInformation* info )
{
    info->Set(svkDICOMMRSWriter::INPUT_REQUIRED_DATA_TYPE(), "svkMrsImageData");
    return 1;
}


