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



#include <svkDICOMSCWriter.h>


using namespace svk;


vtkCxxRevisionMacro(svkDICOMSCWriter, "$Rev$");
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
    bool createNewSeries = 1;
    this->isGray = false;

}


/*!
 *
 */
svkDICOMSCWriter::~svkDICOMSCWriter()
{
}



/*!
 *
 */
void svkDICOMSCWriter::Write()
{

    this->SetErrorCode(vtkErrorCode::NoError);

    // Error checking
    if ( this->GetImageDataInput(0) == NULL ) {
        vtkErrorMacro(<<"Write:Please specify an input!");
        return;
    }

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

    // Fill in image information.
    this->GetImageDataInput(0)->UpdateInformation();

    int *wExtent;
    wExtent = this->GetImageDataInput(0)->GetWholeExtent();
    this->FileNumber = this->GetImageDataInput(0)->GetWholeExtent()[4];
    this->MinimumFileNumber = this->MaximumFileNumber = this->FileNumber;
    this->FilesDeleted = 0;
    this->UpdateProgress(0.0);

    // loop over the z axis and write the slices
    for (this->FileNumber = wExtent[4]; this->FileNumber <= wExtent[5]; ++this->FileNumber) {

        cout << "FileNumber: " << this->FileNumber << endl; 

        this->MaximumFileNumber = this->FileNumber;
        this->GetImageDataInput(0)->SetUpdateExtent(
            wExtent[0], wExtent[1],
            wExtent[2], wExtent[3],
            this->FileNumber,
            this->FileNumber
        );

        // determine the name
        if (this->FileName) {
            sprintf(this->InternalFileName,"%s",this->FileName);
        } else {
            if (this->FilePrefix) {
                sprintf(this->InternalFileName, this->FilePattern,
                    this->FilePrefix, this->FileNumber);
            } else {
                sprintf(this->InternalFileName, this->FilePattern,this->FileNumber);
            }
        }
        this->GetImageDataInput(0)->Update();
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
 *  Note:  WARNING:  Probably need a LUT to map the color data to BW for this particular IOD
 *  Currently just taking one component, which is probably reasonable for a BW window.
 *
 *  Maps RGB color input image to gray scale via vtkImageLuminance
 *  for output to DICOM Secondary Capture. 
 */
void svkDICOMSCWriter::WriteSlice()
{

    vtkDebugMacro( <<  this->GetClassName() << "::WriteSlice()" );

    this->InitDcmHeader();

    if ( this->isGray ) {

        //  Get luminance of image for mapping to grey scale for pixel 
        //  extraction.
        vtkImageLuminance* luminance = vtkImageLuminance::New();
        luminance->SetInput( this->GetImageDataInput(0) );
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
                    = static_cast<unsigned short>( luminance->GetOutput()->GetScalarComponentAsFloat(x, y, 0, 0));
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

        unsigned char* pixelsRGB = 
            static_cast<vtkUnsignedCharArray*>(this->GetImageDataInput(0)->GetPointData()->GetScalars())->GetPointer(0);

        int numRGBComponents = 3; 
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
 *  The StudyInstanceUID is copied from the template to attach the images to the 
 *  data from which it was derived.    
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

    this->dcmHeader->SetDcmPatientsName(  
        this->dcmHeaderTemplate->GetStringValue("PatientsName")  
    );

    this->dcmHeader->SetValue(
        "StudyInstanceUID",
        this->dcmHeaderTemplate->GetStringValue("StudyInstanceUID") 
    );

    this->dcmHeader->SetValue(
        "AccessionNumber",
        this->dcmHeaderTemplate->GetStringValue("StudyInstanceUID") 
    );

    if( !this->createNewSeries ) {
        this->dcmHeader->SetValue(
            "SeriesInstanceUID",
            this->dcmHeaderTemplate->GetStringValue("SeriesInstanceUID") 
        );
    }

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
 *
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
        this->SetInputConnection(index, input->GetProducerPort());
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
 *
 */
void svkDICOMSCWriter::SetCreateNewSeries( bool createNewSeries ) 
{
    this->createNewSeries = createNewSeries;
}


/*!
 *
 */
void svkDICOMSCWriter::SetOutputToGrayscale( bool isOutputGray )
{
    this->isGray = isOutputGray;
}
