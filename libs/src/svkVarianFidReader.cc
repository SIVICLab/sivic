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


#include <svkVarianFidReader.h>
#include <svkVarianCSFidMapper.h>
#include <svkVarianUCSF2DcsiMapper.h>
#include <svkVarianUCSFEPSI2DMapper.h>
#include </usr/include/vtk/vtkDebugLeaks.h>

#include <sys/stat.h>

using namespace svk;


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

    this->progressCallback = vtkCallbackCommand::New();
    this->progressCallback->SetCallback( UpdateProgressCallback );
    this->progressCallback->SetClientData( (void*)this );
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
    if( this->progressCallback != NULL ) {
        this->progressCallback->Delete();
        this->progressCallback = NULL;
    }
}



/*!
 *  Check to see if the extension indicates a Varian FDF file.  If so, try 
 *  to open the file for reading.  If that works, then return a success code. 
 *  Return Values: 1 if can read the file, 0 otherwise.
 */
int svkVarianFidReader::CanReadFile(const char* fname)
{

    string fileToCheck(fname);

    if( fileToCheck.size() >= 3 ) {

        // Also see "vtkImageReader2::GetFileExtensions method" 
        if (  fileToCheck.substr( fileToCheck.size() - 3 ) == "fid" ) {
            FILE *fp = fopen(fname, "rb");
            if (fp) {
                fclose(fp);
                vtkDebugMacro(
                    << this->GetClassName() << "::CanReadFile(): It's a Varian FID File: " << fileToCheck
                );

                //  Now check to see if it's a supported sequence: 
                this->SetFileName( fname ); 
                this->ParseFid(); 
                if ( this->mapper != NULL )  {
                    mapper->Delete(); 
                    this->mapper = NULL; 
                }
                //  should be a mapper factory to get psd specific instance:
                this->mapper = this->GetFidMapper();
                if ( this->mapper == NULL ) {
                    cout  << "Not a known Varian FID sequnce.  Can not read file."   << endl;
                    return 0;
                }

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
void svkVarianFidReader::ExecuteDataWithInformation(vtkDataObject* output, vtkInformation* outInfo)
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
    if ( this->mapper != NULL )  {
        mapper->Delete(); 
        this->mapper = NULL; 
    }
    this->mapper = this->GetFidMapper();
    this->mapper->AddObserver(vtkCommand::ProgressEvent, progressCallback);

    //  all the IE initialization modules would be contained within the mapper
    this->mapper->InitializeDcmHeader(
        procparMap,
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
svkVarianFidMapper* svkVarianFidReader::GetFidMapper()
{
    svkVarianFidMapper* aMapper = NULL;

    string seqfil;

    map <string, vector < vector<string> > >::iterator mapIt; 
    mapIt = this->procparMap.find("seqfil");
    if ( mapIt != this->procparMap.end() ) {
        seqfil = this->procparMap["seqfil"][0][0];
    } else {
        seqfil = "";
    }


    //convert to lower case:
    string::iterator it;
    for ( it = seqfil.begin(); it < seqfil.end(); it++ ) {
        *it = (char)tolower(*it);
    }

    if ( ( seqfil.compare("csi2d") == 0 ) || ( seqfil.compare("c13_csi2d") == 0 ) || ( seqfil.compare("c13_csi2d_nlr") == 0 ) ) {

        // UCSF 2DCSI :  
        //  TODO:  this may be generalizable for product csi2d.  It was tested against the 
        //  product sequence csi2d from an agilent scanner and appears to be correct. 
        aMapper = svkVarianUCSF2DcsiMapper::New();

    } else if ( seqfil.compare("epsi2d") == 0) {

        // UCSF 2DCSI :
        aMapper =  svkVarianUCSFEPSI2DMapper::New();

    } else if ( seqfil.compare("episense_ce") == 0) {

        aMapper = svkVarianCSFidMapper::New();

    } else { 
        cout << "Not a supported Varian sequence" << endl;
    }

    return aMapper; 
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

    string fidFileName( this->GetFileName() );  
    string fidFilePath( this->GetFilePath( this->GetFileName() ) );  

    try { 

        this->ParseProcpar(fidFilePath);
        if (this->GetDebug()) {
            this->PrintProcparKeyValuePairs();
        }

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


/*!
 * 
 */
void svkVarianFidReader::SetProgressText( string progressText ) 
{
    this->progressText = progressText;
}


/*!
 * 
 */
void svkVarianFidReader::UpdateProgress(double amount)
{
    this->InvokeEvent(vtkCommand::ProgressEvent,static_cast<void *>(&amount));
}   


/*!
 * 
 */
void svkVarianFidReader::UpdateProgressCallback(vtkObject* subject, unsigned long, void* thisObject, void* callData)
{
    static_cast<svkVarianFidReader*>(thisObject)->UpdateProgress(*(double*)(callData)); 
}
