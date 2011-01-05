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


#include <svkVarianFidReader.h>
#include <svkVarianCSFidMapper.h>
#include <vtkDebugLeaks.h>

#include <sys/stat.h>

using namespace svk;


vtkCxxRevisionMacro(svkVarianFidReader, "$Rev$");
vtkStandardNewMacro(svkVarianFidReader);


/*!
 *
 */
svkVarianFidReader::svkVarianFidReader()
{

#if VTK_DEBUG_ON
    this->DebugOn();
    vtkDebugLeaks::ConstructClass("svkVarianFidReader");
#endif

    vtkDebugMacro( << this->GetClassName() << "::" << this->GetClassName() << "()" );

    this->fidFile = NULL;
    this->procparFile = NULL;
    this->mapper = NULL;

    //  data is written as big endian:
    this->SetDataByteOrderToBigEndian();
}


/*!
 *
 */
svkVarianFidReader::~svkVarianFidReader()
{

    vtkDebugMacro( << this->GetClassName() << "::~" << this->GetClassName() << "()" );

    if ( this->fidFile != NULL )  {
        delete fidFile; 
        this->fidFile = NULL; 
    }

    if ( this->procparFile != NULL )  {
        delete procparFile; 
        this->procparFile = NULL; 
    }

    if ( this->mapper != NULL )  {
        mapper->Delete(); 
        this->mapper = NULL; 
    }
}



/*!
 *  Check to see if the extension indicates a Varian FDF file.  If so, try 
 *  to open the file for reading.  If that works, then return a success code. 
 *  Return Values: 1 if can read the file, 0 otherwise.
 */
int svkVarianFidReader::CanReadFile(const char* fname)
{

    vtkstd::string fileToCheck(fname);

    if( fileToCheck.size() >= 3 ) {

        // Also see "vtkImageReader2::GetFileExtensions method" 
        if (  fileToCheck.substr( fileToCheck.size() - 3 ) == "fid" ) {
            FILE *fp = fopen(fname, "rb");
            if (fp) {
                fclose(fp);
                vtkDebugMacro(
                    << this->GetClassName() << "::CanReadFile(): It's a Varian FID File: " << fileToCheck
                );
                return 1;
            }
        } else {
            vtkDebugMacro(
                << this->GetClassName() << "::CanReadFile(): It's NOT a Varian FID File: " << fileToCheck
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
void svkVarianFidReader::ExecuteData(vtkDataObject* output)
{

    vtkDebugMacro( << this->GetClassName() << "::ExecuteData()" );
    svkImageData* data = svkImageData::SafeDownCast( this->AllocateOutputData(output) );

    if ( this->FileName ) {
        this->mapper->ReadFidFile( this->FileName, data );
    }

    double dcos[3][3];
    this->GetOutput()->GetDcmHeader()->GetDataDcos( dcos );
    this->GetOutput()->SetDcos(dcos);

    //  SetNumberOfIncrements is supposed to call this, but only works if the data has already
    //  been allocated. but that requires the number of components to be specified.
    this->GetOutput()->GetIncrements();

    this->GetOutput()->GetDcmHeader()->PrintDcmHeader();

}


/*!
 *  Side effect of Update() method.  Used to initialize the svkDcmHeader member of 
 *  the target svkImageData object and uses the header to set up the Output Informatin.
 *  Called before ExecuteData()
 */
void svkVarianFidReader::ExecuteInformation()
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
void svkVarianFidReader::InitDcmHeader()
{

    vtkDebugMacro( << this->GetClassName() << "::InitDcmHeader()" );

    svkMRSIOD* iod = svkMRSIOD::New();
    iod->SetDcmHeader( this->GetOutput()->GetDcmHeader());
    iod->InitDcmHeader();

    //  Read the fid header into a map of values used to initialize the
    //  DICOM header. 
    this->ParseFid(); 

    //  should be a mapper factory to get psd specific instance:
    this->mapper = svkVarianCSFidMapper::New();
    //this->mapper = svkVarianFidMapper::New();

    //  all the IE initialization modules would be contained within the mapper
    this->mapper->InitializeDcmHeader(
        procparMap,
        this->GetOutput()->GetDcmHeader(),
        iod,
        this->GetSwapBytes()
    );

    if (this->GetDebug()) {
        this->GetOutput()->GetDcmHeader()->PrintDcmHeader();
    }

    iod->Delete();
}


/*!
 *  Returns the file type enum 
 */
svkDcmHeader::DcmPixelDataFormat svkVarianFidReader::GetFileType()
{
    svkDcmHeader::DcmPixelDataFormat format = svkDcmHeader::SIGNED_FLOAT_4;

    return format; 
}


/*
 *  Read FID header blocks into a string STL map for use during initialization 
 *  of DICOM header by Init*Module methods. 
 *  If a procpar file is present in the directory, parse that as well. 
 */
void svkVarianFidReader::ParseFid()
{

    vtkstd::string fidFileName( this->GetFileName() );  
    vtkstd::string fidFilePath( this->GetFilePath( this->GetFileName() ) );  

    try { 

        this->ParseProcpar(fidFilePath);
        this->PrintProcparKeyValuePairs();

    } catch (const exception& e) {
        cerr << "ERROR opening or reading Varian fid file (" << fidFileName << "): " << e.what() << endl;
    }

}


/*!
 *
 */
int svkVarianFidReader::FillOutputPortInformation( int vtkNotUsed(port), vtkInformation* info )
{
    info->Set(vtkDataObject::DATA_TYPE_NAME(), "svkMrsImageData");
    return 1;
}
