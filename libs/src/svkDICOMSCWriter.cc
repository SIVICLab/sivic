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



#include <svkDICOMSCWriter.h>
#include </usr/include/vtk/vtkStreamingDemandDrivenPipeline.h>


using namespace svk;


//vtkCxxRevisionMacro(svkDICOMSCWriter, "$Rev$");
vtkStandardNewMacro(svkDICOMSCWriter);


/*!
 *
 */
svkDICOMSCWriter::svkDICOMSCWriter()
{

#if VTK_DEBUG_ON
    this->DebugOn();
#endif

    vtkDebugMacro( <<  this->GetClassName() << "::" << this->GetClassName() << "()");

    this->seriesNumber = 0; 
    this->instanceNumber = 1; 
    this->useInstanceNumber = true; 
    this->isGray = false;

}


/*!
 *
 */
svkDICOMSCWriter::~svkDICOMSCWriter()
{
}



/*!
 *  The main method which triggers the writer to write DICOM SC files.
 */
void svkDICOMSCWriter::Write()
{

    this->SetErrorCode(vtkErrorCode::NoError);

    // Error checking
    if ( this->GetImageDataInput(0) == NULL ) {
        vtkErrorMacro(<<"Write:Please specify an input!");
        return;
    }

    if ( this->FileName ) { 
        string fileNameString( this->FileName );
        size_t pos = fileNameString.rfind(".dcm");
        if( pos == string::npos ) {
            pos = fileNameString.rfind(".DCM");
            if( pos == string::npos ) {
                fileNameString+=".dcm";
                delete this->FileName;
                this->FileName = new char[fileNameString.size() + 1];
                strcpy(this->FileName, fileNameString.c_str());
            }
        }
    } else if ( this->FilePattern ) {
        string filePatternString( this->FilePattern );
        size_t pos = filePatternString.rfind(".dcm");
        if( pos == string::npos ) {
            pos = filePatternString.rfind(".DCM");
            if( pos == string::npos ) {
                filePatternString+=".dcm";
                delete this->FilePattern;
                this->FilePattern = new char[filePatternString.size() + 1];
                strcpy(this->FilePattern, filePatternString.c_str());
            }
        } 
        if( this->FilePrefix ) {
            string filePrefixString( this->FilePrefix );
            size_t pos = filePrefixString.rfind(".dcm");
            if( pos != string::npos ) {
                filePrefixString.replace(pos, pos + 4, "");
                delete this->FilePrefix;
                this->FilePrefix = new char[filePrefixString.size() + 1];
                strcpy(this->FilePrefix, filePrefixString.c_str());
            }
            pos = filePrefixString.rfind(".DCM");
            if( pos != string::npos ) {
                filePrefixString.replace(pos, pos + 4, "");
                delete this->FilePrefix;
                this->FilePrefix = new char[filePrefixString.size() + 1];
                strcpy(this->FilePrefix, filePrefixString.c_str());
            }
        }
    } else {
        vtkErrorMacro(<<"Write:Please specify either a FileName or a file prefix and pattern");
        this->SetErrorCode(vtkErrorCode::NoFileNameError);
        return;
    }
   
     
    // Make sure the file name is allocated
    this->InternalFileName =
        new char[(this->FileName ? strlen(this->FileName) : 1) +
            (this->FilePrefix ? strlen(this->FilePrefix) : 1) +
            (this->FilePattern ? strlen(this->FilePattern) : 1) + 10];
    

    // Fill in image information.
    this->UpdateInformation(); 

    int* wExtent = vtkImageData::SafeDownCast(this->GetInput(0))->GetExtent();
    this->FileNumber = wExtent[4]; 
    this->MaximumFileNumber = this->FileNumber;
    this->MinimumFileNumber = this->FileNumber;
    this->FilesDeleted = 0;
    this->UpdateProgress(0.0);


    this->InitDcmHeader();
    if( useInstanceNumber ) {
        this->instanceNumber = this->dcmHeaderTemplate->GetIntValue("InstanceNumber");
    }
 
    // loop over the z axis and write the slices
    for (this->FileNumber = wExtent[4]; this->FileNumber <= wExtent[5]; ++this->FileNumber) {

        this->dcmHeader->InsertUniqueUID( "SOPInstanceUID" );

        if( useInstanceNumber ) {
            this->dcmHeader->SetValue(
                "InstanceNumber",
                this->instanceNumber + this->FileNumber
            );
        } else {
            this->dcmHeader->SetValue(
                "InstanceNumber",
                this->FileNumber 
            );
        }

        this->MaximumFileNumber = this->FileNumber;
        this->UpdateInformation(); 
        int upExtent[6];  
        upExtent[0] = wExtent[0]; 
        upExtent[1] = wExtent[1]; 
        upExtent[2] = wExtent[2]; 
        upExtent[3] = wExtent[3]; 
        upExtent[4] = this->FileNumber; 
        upExtent[5] = this->FileNumber; 

        this->GetUpdateExtent( upExtent );  
        this->Update();

        // determine the name
        if (this->FileName) {
            sprintf(this->InternalFileName,"%s",this->FileName);
        } else {
            if (this->FilePrefix) {
                if( useInstanceNumber ) {
                    sprintf(this->InternalFileName, this->FilePattern,
                        this->FilePrefix, this->instanceNumber + this->FileNumber);
                } else {
                    sprintf(this->InternalFileName, this->FilePattern,
                        this->FilePrefix, this->FileNumber );
                }
            } else {
                if( useInstanceNumber ) {
                    sprintf(this->InternalFileName, this->FilePattern, this->instanceNumber + this->FileNumber);
                } else {
                    sprintf(this->InternalFileName, this->FilePattern,this->FileNumber);
                }
            }
        }
        this->Update(); 
        this->WriteSlice();
        if (this->ErrorCode == vtkErrorCode::OutOfDiskSpaceError) {
            vtkErrorMacro("Ran out of disk space; deleting file(s) already written");
            this->DeleteFiles();
            return;
        }
        this->UpdateProgress(
            (this->FileNumber - wExtent[4])/(wExtent[5] - wExtent[4] + 1.0)
        );
    }

    delete [] this->InternalFileName;
    this->InternalFileName = NULL;

}


/*!
 *  Sets RGB DICOM PixelData (default), or if SetOutputToGrayscale was called, maps RGB 
 *  from input image to gray scale via vtkImageLuminance for PixelData. 
 */
void svkDICOMSCWriter::WriteSlice()
{

    vtkDebugMacro( <<  this->GetClassName() << "::WriteSlice()" );

    if ( this->isGray ) {

        //  Get luminance of image for mapping to grey scale for pixel 
        //  extraction.
        vtkImageLuminance* luminance = vtkImageLuminance::New();
        luminance->SetInputData( this->GetImageDataInput(0) );
        luminance->Update();
    
        /*  
        *  Get Scalar data, scale to short and insert into DICOM object : 
        *  Note that it's already unsigned short in this case
        */
        int sizeX = (this->GetImageDataInput(0)->GetDimensions())[0]; 
        int sizeY = (this->GetImageDataInput(0)->GetDimensions())[1]; 
        unsigned short* pixelsBW = new unsigned short[ sizeX * sizeY ];
        int index = 0;
        for (int y = 0; y < sizeY; y++) {
            for (int x = 0; x < sizeX; x++) {
                pixelsBW[index++] 
                    = static_cast<unsigned short>( luminance->GetOutput()->GetScalarComponentAsFloat(x, y, this->FileNumber, 0));
            }
        }
        
        this->dcmHeader->SetValue(
            "PixelData",
            pixelsBW, 
            (this->GetImageDataInput(0)->GetDimensions())[0] * (this->GetImageDataInput(0)->GetDimensions())[1]  
        );

        this->dcmHeader->WriteDcmFile(this->InternalFileName); 

        delete [] pixelsBW; 
        luminance->Delete();

    } else {

        int sizeX = (this->GetImageDataInput(0)->GetDimensions())[0]; 
        int sizeY = (this->GetImageDataInput(0)->GetDimensions())[1]; 
        int numRGBComponents = 3; 
        unsigned char* pixelsRGB = 
            static_cast<vtkUnsignedCharArray*>(this->GetImageDataInput(0)->GetPointData()->GetScalars())->GetPointer( this->FileNumber*sizeX*sizeY*numRGBComponents );

        this->dcmHeader->SetValue(
            "PixelData",
            pixelsRGB, 
            numRGBComponents 
            * (this->GetImageDataInput(0)->GetDimensions())[0] * (this->GetImageDataInput(0)->GetDimensions())[1]  
        );

        this->dcmHeader->WriteDcmFile(this->InternalFileName); 

    }

}



/*!
 *  Generates a new svkDcmHeader initialized to a default DICOM Secondary
 *  Capture IOD, then fills in instance specific values.       
 *  The StudyInstanceUID and other patient/exam specific attributes are copied from the 
 *  template to attach the images to the data from which it was derived.    
 */
void svkDICOMSCWriter::InitDcmHeader()
{

    if ( svkDcmHeader::adapter_type == svkDcmtkAdapter::DCMTK_API ) {
        this->dcmHeader =  svkDcmtkAdapter::New() ;
    }

    /*  
     *  Get Default populated SOP Instance:
     */
    svkIOD* iod = svkSCIOD::New();
    iod->SetDcmHeader( this->dcmHeader );
    iod->InitDcmHeader();
    iod->Delete();

    if (this->GetDebug()) {
        this->dcmHeader->PrintDcmHeader();
    }

    /*  
     *  Now populate it with values from the data set or template header
     */
    this->dcmHeaderTemplate = this->GetImageDataInput(0)->GetDcmHeader();

    this->dcmHeader->SetValue(
        "SeriesNumber",
        this->seriesNumber 
    );

    if (this->seriesDescription.length() > 0) {
        this->dcmHeader->SetValue(
            "SeriesDescription",
            this->seriesDescription
        );
    }

    this->dcmHeader->SetValue(
        "InstanceNumber",
        this->instanceNumber 
    );

    this->dcmHeader->SetValue(
        "PatientID",
        this->dcmHeaderTemplate->GetStringValue("PatientID") 
    );

    this->dcmHeader->SetValue(
        "StudyDate",
        this->dcmHeaderTemplate->GetStringValue("StudyDate") 
    );

    this->dcmHeader->SetValue(
        "StudyTime",
        "000000"
    );

    this->dcmHeader->SetValue(
        "StudyID",
        this->dcmHeaderTemplate->GetStringValue("StudyID") 
    );

    this->dcmHeader->SetDcmPatientName(  
        this->dcmHeaderTemplate->GetStringValue("PatientName")  
    );

    this->dcmHeader->SetValue(  
        "PatientBirthDate", 
        this->dcmHeaderTemplate->GetStringValue("PatientBirthDate")  
    );

    this->dcmHeader->SetValue(  
        "PatientSex", 
        this->dcmHeaderTemplate->GetStringValue("PatientSex")  
    );

    this->dcmHeader->SetValue(  
        "ReferringPhysicianName", 
        this->dcmHeaderTemplate->GetStringValue("ReferringPhysicianName")  
    );

    if( this->dcmHeader->ElementExists( "StationName" ) ) {
        this->dcmHeader->SetValue(  
            "StationName", 
            this->dcmHeaderTemplate->GetStringValue("StationName")  
        );
    }

    this->dcmHeader->SetValue(  
        "SoftwareVersions", 
        "SIVIC" + string(SVK_RELEASE_VERSION)
    );

    this->dcmHeader->SetValue(
        "StudyInstanceUID",
        this->dcmHeaderTemplate->GetStringValue("StudyInstanceUID") 
    );

    this->dcmHeader->SetValue(
        "AccessionNumber",
        this->dcmHeaderTemplate->GetStringValue("AccessionNumber") 
    );

    this->dcmHeader->SetValue(
        "SeriesInstanceUID",
        this->dcmHeaderTemplate->GetStringValue("SeriesInstanceUID") 
    );

    this->dcmHeader->SetValue(
        "Columns",
        (this->GetImageDataInput(0)->GetDimensions())[0]
    );

    this->dcmHeader->SetValue(
        "Rows",
        (this->GetImageDataInput(0)->GetDimensions())[1]
    );


    /*  Set the ImagePixelModule Attributes depending on whether 
     *  it's RGB or Grayscale. 
     */
    if ( ! this->isGray ) {

        this->dcmHeader->SetValue(
            "SamplesPerPixel",
            3
        );

        this->dcmHeader->SetValue(
            "PhotometricInterpretation",
            "RGB"
        );

        //  Send color by pixel, i.e. r1,g1,b1, r2,g2,b2, ...
        this->dcmHeader->SetValue(
            "PlanarConfiguration",
            0 
        );

        this->dcmHeader->SetValue( "BitsAllocated", 8 );
        this->dcmHeader->SetValue( "BitsStored", 8 );
        this->dcmHeader->SetValue( "HighBit", 7 );
    }
      
    /*  
     *  Hardcode for unsigned integer representation:
     */
    this->dcmHeader->SetValue(
        "PixelRepresentation",
        0
    );

}


/*!
 *
 */
int svkDICOMSCWriter::FillInputPortInformation( int vtkNotUsed(port), vtkInformation* info )
{
    info->Set(svkDICOMSCWriter::INPUT_REQUIRED_DATA_TYPE(), "vtkImageData");
    return 1;
}


/*!
 *  Set the input svkImageDataObject, which will be used to set the DICOM PixelData
 *  and also populate Exam/Patient specific DICOM attributes for the output data. 
 */
void svkDICOMSCWriter::SetInput( vtkDataObject* input )
{
    this->SetInput(0, input);
}


/*!
 *
 */
void svkDICOMSCWriter::SetInput(int index, vtkDataObject* input)
{
    if(input) {
        this->SetInputData(index, input); 
    } else {
        // Setting a NULL input removes the connection.
        this->SetInputConnection(index, 0);
    }
}


/*!
 *
 */
vtkDataObject* svkDICOMSCWriter::GetInput(int port)
{
    return this->GetExecutive()->GetInputData(port, 0);
}


/*!
 *
 */
svkImageData* svkDICOMSCWriter::GetImageDataInput(int port)
{
    return svkImageData::SafeDownCast( this->GetInput(port) );
}


/*!
 *  Not yet implemented. Intended to force new SeriesInstanceUID 
 *  generation of for use when writer instance will create multiple 
 *  DICOM SC output series. 
 */
void svkDICOMSCWriter::CreateNewSeries( ) 
{
}


/*!
 *  If set to true, results in conversion of input pixels to grayscale 
 *  for DICOM PixelDataSets, and also sets ImagePixelModule attribures 
 *  correctly for grayscale output. The default output is RGB.   
 */
void svkDICOMSCWriter::SetOutputToGrayscale( bool isOutputGray )
{
    this->isGray = isOutputGray;
}
/*!
 *  Set to true if you want to use the instance number as the file number.
 */
void svkDICOMSCWriter::SetUseInstanceNumber( bool useInstanceNumber )
{
    this->useInstanceNumber = useInstanceNumber;
}
