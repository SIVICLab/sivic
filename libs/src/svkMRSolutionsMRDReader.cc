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


#include <svkMRSolutionsMRDReader.h>
#include <svkMRSolutionsMRDMapper.h>
#include <svkTypeUtils.h>
#include <vtkDebugLeaks.h>

#include <sys/stat.h>

using namespace svk;


//vtkCxxRevisionMacro(svkMRSolutionsMRDReader, "$Rev$");
vtkStandardNewMacro(svkMRSolutionsMRDReader);


/*!
 *
 */
svkMRSolutionsMRDReader::svkMRSolutionsMRDReader()
{

#if VTK_DEBUG_ON
    this->DebugOn();
    vtkDebugLeaks::ConstructClass("svkMRSolutionsMRDReader");
#endif

    vtkDebugMacro( << this->GetClassName() << "::" << this->GetClassName() << "()" );

    this->mrdFile = NULL;
    this->mapper = NULL;

    //  data is written as big endian:
    this->SetDataByteOrderToBigEndian();

    this->progressCallback = vtkCallbackCommand::New();
    this->progressCallback->SetCallback( UpdateProgressCallback );
    this->progressCallback->SetClientData( (void*)this );
}


/*!
 *
 */
svkMRSolutionsMRDReader::~svkMRSolutionsMRDReader()
{

    vtkDebugMacro( << this->GetClassName() << "::~" << this->GetClassName() << "()" );

    if ( this->mrdFile != NULL )  {
        delete mrdFile; 
        this->mrdFile = NULL; 
    }

    if ( this->mapper != NULL )  {
        mapper->Delete(); 
        this->mapper = NULL; 
    }
    if( this->progressCallback != NULL ) {
        this->progressCallback->Delete();
        this->progressCallback = NULL;
    }
}



/*!
 *  Check to see if the extension indicates an MRD FDF file.  If so, try 
 *  to open the file for reading.  If that works, then return a success code. 
 *  Return Values: 1 if can read the file, 0 otherwise.
 */
int svkMRSolutionsMRDReader::CanReadFile(const char* fname)
{

    string fileToCheck(fname);

    //  If file has an extension then check it:
    string fileExtension = this->GetFileExtension( fname );
    if( ! fileExtension.empty() ) {

        // Also see "vtkImageReader2::GetFileExtensions method" 

        if (
            fileExtension.compare( "MRD" ) == 0 ||
            fileExtension.compare( "mrd" ) == 0
        )  {

            FILE *fp = fopen(fname, "rb");
            if (fp) {
                fclose(fp);
                vtkDebugMacro(
                    << this->GetClassName() << "::CanReadFile(): It's a MR Solutions MRD File: " << fileToCheck
                );

                //  Now check to see if it's a supported sequence: 
                this->SetFileName( fname ); 
                this->ParseMRD(); 
                if ( this->mapper != NULL )  {
                    mapper->Delete(); 
                    this->mapper = NULL; 
                }
                //  should be a mapper factory to get psd specific instance:
                this->mapper = this->GetMRDMapper();
                if ( this->mapper == NULL ) {
                    cout  << " Not a know FID sequnce.  Can not read file."   << endl;
                    return 0;
                }

                return 1;
            }
        } else {
            vtkDebugMacro(
                << this->GetClassName() << "::CanReadFile(): It's NOT a MR Solutions MRD File: " << fileToCheck
            );
            return 0;
        }
    } else {
        vtkDebugMacro(<< this->GetClassName() << "::CanReadFile(): is NOT a valid file: " << fileToCheck);
        return 0;
    }
}


/*!
 *  Side effect of Update() method.  Used to load pixel data and initialize vtkImageData
 *  Called after ExecuteInformation()
 */
void svkMRSolutionsMRDReader::ExecuteDataWithInformation(vtkDataObject* output, vtkInformation* outInfo)
{

    vtkDebugMacro( << this->GetClassName() << "::ExecuteData()" );
    svkImageData* data = svkImageData::SafeDownCast( this->AllocateOutputData(output, outInfo) );

    if ( this->FileName ) {
        this->mapper->ReadFidFile( this->FileName, data );
    }

    double dcos[3][3];
    this->GetOutput()->GetDcmHeader()->GetDataDcos( dcos );
    this->GetOutput()->SetDcos(dcos);

    //  SetNumberOfIncrements is supposed to call this, but only works if the data has already
    //  been allocated. but that requires the number of components to be specified.
    this->GetOutput()->GetIncrements();

    if (this->GetDebug()) {
        this->GetOutput()->GetDcmHeader()->PrintDcmHeader();
    }

}


/*!
 *  Side effect of Update() method.  Used to initialize the svkDcmHeader member of 
 *  the target svkImageData object and uses the header to set up the Output Informatin.
 *  Called before ExecuteData()
 */
void svkMRSolutionsMRDReader::ExecuteInformation()
{
    vtkDebugMacro( << this->GetClassName() << "::ExecuteInformation()" );

    if (this->FileName == NULL) {
        return;
    }

    if ( this->FileName ) {
        vtkDebugMacro( << this->GetClassName() << " FileName: " << FileName );

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
 *  Initializes the svkDcmHeader adapter to a specific IOD type
 *  and initizlizes the svkDcmHeader member of the svkImageData
 *  object.
 */
void svkMRSolutionsMRDReader::InitDcmHeader()
{

    vtkDebugMacro( << this->GetClassName() << "::InitDcmHeader()" );

    svkMRSIOD* iod = svkMRSIOD::New();
    iod->SetDcmHeader( this->GetOutput()->GetDcmHeader());
    iod->InitDcmHeader();

    //  Read the fid header into a map of values used to initialize the
    //  DICOM header. 
    this->ParseMRD(); 

    //  should be a mapper factory to get psd specific instance:
    if ( this->mapper != NULL )  {
        mapper->Delete(); 
        this->mapper = NULL; 
    }
    this->mapper = this->GetMRDMapper();
    this->mapper->AddObserver(vtkCommand::ProgressEvent, progressCallback);

    //  all the IE initialization modules would be contained within the mapper
    this->mapper->InitializeDcmHeader(
        mrdMap,
        this->GetOutput()->GetDcmHeader(),
        iod,
        this->GetSwapBytes()
    );

    this->RemoveObserver(progressCallback);


    if (this->GetDebug()) {
        this->GetOutput()->GetDcmHeader()->PrintDcmHeader();
    }

    iod->Delete();
}


/*!
 *  Create an svkGEPFileMapper of the appropriate type for the present pfile
 */
svkMRSolutionsMRDMapper* svkMRSolutionsMRDReader::GetMRDMapper()
{
    svkMRSolutionsMRDMapper* aMapper = NULL;
    aMapper = svkMRSolutionsMRDMapper::New(); 
    return aMapper; 
}


/*!
 *  Returns the file type enum 
 */
svkDcmHeader::DcmPixelDataFormat svkMRSolutionsMRDReader::GetFileType()
{
    svkDcmHeader::DcmPixelDataFormat format = svkDcmHeader::SIGNED_FLOAT_4;

    return format; 
}


/*
 *  Read MRD header blocks into a string STL map for use during initialization 
 *  of DICOM header by Init*Module methods. 
 */
void svkMRSolutionsMRDReader::ParseMRD()
{

    string mrdFileName( this->GetFileName() );  
    string mrdFilePath( this->GetFilePath( this->GetFileName() ) );  

    try {

        this->mrdFile = new ifstream();
        this->mrdFile->exceptions( ifstream::eofbit | ifstream::failbit | ifstream::badbit );

        this->mrdFile->open( mrdFileName.c_str(), ifstream::in );
        if ( ! this->mrdFile->is_open() ) {
            throw runtime_error( "Could not open mrd file: " + mrdFileName);
        }

        int int32Size   = 4; 
        int float32Size = 4; 
        int int16Size   = 2; 
        int charSize    = 1; 
        int                 intVal; 
        float               floatVal; 
        unsigned short int  usIntVal; 
        unsigned char       usCharVal; 

        //  xDim (num samples)
        this->mrdFile->read( (char*)(&intVal), int32Size);
        cout << "xDim: " << intVal << endl;
        int numSample = intVal; 
        this->mrdMap["xDim"] = svkTypeUtils::IntToString( intVal ); 

        //  xDim (num views)
        this->mrdFile->read( (char*)(&intVal), int32Size);
        cout << "yDim: " << intVal << endl;
        int numViews = intVal; 
        this->mrdMap["yDim"] = svkTypeUtils::IntToString( intVal ); 

        //  zDim (num views 2)
        this->mrdFile->read( (char*)(&intVal), int32Size);
        cout << "zDim: " << intVal << endl;
        int numViews_2 = intVal;
        this->mrdMap["zDim"] = svkTypeUtils::IntToString( intVal ); 

        //  dim 4
        this->mrdFile->read( (char*)(&intVal), int32Size);
        cout << "dim4: " << intVal << endl;
        int numSlices = intVal;
        this->mrdMap["dim4"] = svkTypeUtils::IntToString( intVal ); 

        //  data type
        this->mrdFile->seekg( 18, ios_base::beg );
        this->mrdFile->read( (char*)(&usIntVal), int16Size);
        //datatype = dec2hex(datatype);
        cout << "dataType: " << usIntVal<< endl;
        this->mrdMap["dataType"] = svkTypeUtils::IntToString( usIntVal ); 

        this->mrdFile->seekg( 48, ios_base::beg );
        this->mrdFile->read( (char*)(&floatVal), float32Size);
        cout << "scaling: " << floatVal<< endl;
        this->mrdMap["scaling"] = svkTypeUtils::DoubleToString( floatVal ); 

        this->mrdFile->read( (char*)(&usCharVal), charSize);
        cout << "bitsRepPixel: " << usCharVal << endl;
        this->mrdMap["bitsRepPixel"] = svkTypeUtils::IntToString( usCharVal ); 

        this->mrdFile->seekg( 512, ios_base::beg );
        this->mrdFile->read( (char*)(&intVal), int32Size);
        cout << "dim5: " << intVal<< endl;
        int numEchoes = intVal;
        this->mrdMap["dim5"] = svkTypeUtils::IntToString( intVal ); 

        this->mrdFile->read( (char*)(&intVal), int32Size);
        cout << "dim6: " << intVal<< endl;
        int numExpts = intVal;
        this->mrdMap["dim6"] = svkTypeUtils::IntToString( intVal ); 

        this->mrdFile->seekg( 256, ios_base::beg );
        char text[256]; 
        this->mrdFile->read( (char*)(text), 256 * charSize);
        string textString(text); 
        cout << "text: " << text << endl;
        cout << "text: " << textString << endl;
        this->mrdMap["text"] = text; 

        this->mrdFile->close();

    } catch (const exception& e) {
        cerr << "ERROR opening or reading MR Solutions MRD fid file (" << mrdFileName << "): " << e.what() << endl;
    }

}


/*!
 *
 */
int svkMRSolutionsMRDReader::FillOutputPortInformation( int vtkNotUsed(port), vtkInformation* info )
{
    info->Set(vtkDataObject::DATA_TYPE_NAME(), "svkMrsImageData");
    return 1;
}


/*!
 * 
 */
void svkMRSolutionsMRDReader::SetProgressText( string progressText ) 
{
    this->progressText = progressText;
}


/*!
 * 
 */
void svkMRSolutionsMRDReader::UpdateProgress(double amount)
{
    this->InvokeEvent(vtkCommand::ProgressEvent,static_cast<void *>(&amount));
}   


/*!
 * 
 */
void svkMRSolutionsMRDReader::UpdateProgressCallback(vtkObject* subject, unsigned long, void* thisObject, void* callData)
{
    static_cast<svkMRSolutionsMRDReader*>(thisObject)->UpdateProgress(*(double*)(callData)); 
}
