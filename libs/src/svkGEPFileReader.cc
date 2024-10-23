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


#include <svkGEPFileReader.h>
#include <svkIOD.h>
#include <svkMRSIOD.h>
#include <svkUtils.h>
#include <svkTypeUtils.h>
#include </usr/include/vtk/vtkDebugLeaks.h>
#include </usr/include/vtk/vtkByteSwap.h>
#include </usr/include/vtk/vtkStringArray.h>

#include <sys/stat.h>

#define  UNASSIGNED_ID ""
#define  UNASSIGNED_UID ""


using namespace svk;


//vtkCxxRevisionMacro(svkGEPFileReader, "$Rev$");
vtkStandardNewMacro(svkGEPFileReader);


/*!
 *
 */
svkGEPFileReader::svkGEPFileReader()
{

#if VTK_DEBUG_ON
    this->DebugOn();
    vtkDebugLeaks::ConstructClass("svkGEPFileReader");
#endif

    vtkDebugMacro( << this->GetClassName() << "::" << this->GetClassName() << "()" );

    this->gepf = NULL; 
    this->mapper = NULL; 
    this->tmpFileNames = NULL;

    this->progressCallback = vtkCallbackCommand::New();
    this->progressCallback->SetCallback( UpdateProgressCallback );
    this->progressCallback->SetClientData( (void*)this );

    this->onlyReadHeader = false; 
    this->checkSeriesUID  = true; 

    this->deidPatientId   = UNASSIGNED_ID; 
    this->deidStudyId     = UNASSIGNED_ID; 
    this->phiType         = svkDcmHeader::PHI_IDENTIFIED; 
    this->deidStudyUID    = UNASSIGNED_UID; 
    this->deidSeriesUID   = UNASSIGNED_UID; 
    this->deidImageUID    = UNASSIGNED_UID; 
    this->deidLandmarkUID = UNASSIGNED_UID; 
    this->psdLogic        = "";
}



/*!
 *
 */
svkGEPFileReader::~svkGEPFileReader()
{

    vtkDebugMacro( << this->GetClassName() << "::~" << this->GetClassName() << "()" );

    if ( this->gepf != NULL )  {
        delete this->gepf;
        this->gepf = NULL;
    }

    if ( this->mapper != NULL )  {
        mapper->Delete();
        this->mapper = NULL;
    }

    if( this->progressCallback != NULL ) {
        this->progressCallback->Delete();
        this->progressCallback = NULL;
    }

    //  Now delete all the pointers to input args:  
    map< string, void* >::iterator mapIter;
    for ( mapIter = this->inputArgs.begin(); mapIter != this->inputArgs.end(); ++mapIter ) {
         //delete this->inputArgs[ mapIter->first ];  
        //cout << " need to clean up memory" << endl; 
    }
}


/*!
 *  Check to see if this is a GE pfile.  If so, try
 *  to open the file for reading.  If that works, then return a success code.
 *  Return Values: 1 if can read the file, 0 otherwise.
 */
int svkGEPFileReader::CanReadFile(const char* fname)
{

    bool    isGEPFile = false; 
    bool    isKnownPSD = false; 


    try {


        this->gepf = new ifstream();
        this->gepf->exceptions( ifstream::eofbit | ifstream::failbit | ifstream::badbit );
        this->gepf->open( fname, ios::binary);

        this->pfileVersion = 0; 

        if ( this->gepf->is_open() ) {

            this->pfileVersion = this->GetPFileVersion(); 
            this->SetDataByteOrder(); 
      
            if ( this->pfileVersion ) {
                this->InitOffsetsMap(); 

                if ( this->pfileVersion < 12 ) {
                    string geLogo = this->GetFieldAsString( "rhr.rh_logo" );
                    if ( geLogo.find("GE") != string::npos) {
                        isGEPFile = true; 
                    }
                } else {
                    string offset = this->GetFieldAsString( "rhr.rdb_hdr_off_data" );
                    if ( offset.compare("66072") == 0 || 
                         offset.compare("145908") == 0 || 
                         offset.compare("149788") == 0 ||
                         offset.compare("150336") == 0 ||
                         offset.compare("157276") == 0 ||
                         offset.compare("213684") == 0 
                    ) {
                        isGEPFile = true; 
                    }
                }

                if ( isGEPFile ) { 
                    this->SetFileName( fname ); 
                    if ( this->onlyReadHeader ) {
                        //  Just for parsing header, don't care about mapping
                        isKnownPSD = true;
                    } else {
                        this->ReadGEPFile(); 
                        // We may already have a mapper from can read.
                        if( this->mapper != NULL ) {
                            this->mapper->Delete();
                        }
                        this->mapper = this->GetPFileMapper(); 
                        if ( this->mapper != NULL ) {
                            isKnownPSD = true;
                        }
                    }
                }
            }


        }

    } catch ( ifstream::failure e ) {

        cerr << "ERROR(svkGEPFileReader::CanReadFile():  Error opening or reading file (" << fname << "): " << e.what() << endl;
    }

    if ( this->gepf->is_open() ) {
        this->gepf->close();
    }
    if ( this->gepf != NULL ) {
        delete this->gepf;
        this->gepf = NULL; 
    }

    if ( isGEPFile && isKnownPSD ) {
        vtkDebugMacro(
            << this->GetClassName() << "::CanReadFile(): It's a GE and recognized PSD" 
            << this->pfileVersion << " PFile: " << fname 
        );
        return 1;
    } else {
        vtkDebugMacro(<< this->GetClassName() << "::CanReadFile(): It's NOT a GE PFile, or the psd is not recognized: " << fname);
        return 0;
    }

}


/*!
 *  Side effect of Update() method.  Used to load pixel data and initialize vtkImageData
 *  Called before ExecuteData()
 */
void svkGEPFileReader::ExecuteInformation()
{

    vtkDebugMacro( << this->GetClassName() << "::ExecuteInformation()" );

    if (this->FileName == NULL) {
        return;
    }

    if ( this->FileName ) {

        struct stat fs;
        if ( stat(this->FileName, &fs) ) {
            vtkErrorMacro("Unable to open file " << this->FileName );
            return;
        }
        if ( fs.st_size == 0 ) {
            vtkErrorMacro("PFIle has size 0. " << this->FileName );
            exit(1);
        }

        this->InitDcmHeader(); 

        //  Only need to call this if we are using the data, not printing header
        if ( ! this->onlyReadHeader ) {
            this->SetupOutputInformation();
        }

    }

    //  This is a workaround required since the vtkImageAlgo executive
    //  for the reader resets the Extent[5] value to the number of files
    //  which is not correct for 3D multislice volume files. So store
    //  the files in a temporary array until after ExecuteData has been
    //  called, then reset the array.
    if ( this->onlyReadHeader == false ) {
        if (this->tmpFileNames != NULL ) {
            this->tmpFileNames->Delete(); 
        }
        this->tmpFileNames = vtkStringArray::New();
        this->tmpFileNames->DeepCopy(this->FileNames);
        //for ( int fileNumber = 0; fileNumber < this->GetFileNames()->GetNumberOfValues(); fileNumber++ ) {  
            //cout << "INIT TMP: " << this->GetFileNames()->GetValue(fileNumber) << endl;
        //}  	 	 
        this->FileNames->Delete();
        this->FileNames = NULL;
    }

}


/*!
 *  Side effect of Update() method.  Used to load pixel data and initialize vtkImageData
 *  Called after ExecuteInformation()
 */
void svkGEPFileReader::ExecuteDataWithInformation(vtkDataObject* output, vtkInformation* outInfo)
{

    if ( this->onlyReadHeader == false ) {
        this->FileNames = vtkStringArray::New();
        this->FileNames->DeepCopy(this->tmpFileNames);
        this->tmpFileNames->Delete();
        this->tmpFileNames = NULL;

        vtkDebugMacro( << this->GetClassName() << "::ExecuteData()" );

        vtkImageData::SetScalarType(VTK_FLOAT, outInfo);     
        svkImageData* data = svkImageData::SafeDownCast( this->AllocateOutputData(output, outInfo) );
        vtkImageData::SetScalarType(VTK_FLOAT, data->GetInformation());     

        vtkDebugMacro( << this->GetClassName() << " FileName: " << this->FileName );
        this->mapper->AddObserver(vtkCommand::ProgressEvent, progressCallback);

        //  Set the orientation in the svkImageData object, synchronized from the dcm header:
        double dcos[3][3];
        this->GetOutput()->GetDcmHeader()->GetDataDcos( dcos );
        this->GetOutput()->SetDcos(dcos);

        this->mapper->ReadData(this->GetFileNames(), data);

        //  resync any header changes with the svkImageData object's member variables
        this->SetupOutputInformation(); 

        this->mapper->RemoveObserver(progressCallback);

        //  SetNumberOfIncrements is supposed to call this, but only works if the data has already
        //  been allocated. but that requires the number of components to be specified.
        this->GetOutput()->GetIncrements();

        this->SetProvenance(); 
    }

}


/*!
 *  Appends algo info to provenance record.
 */
void svkGEPFileReader::SetProvenance()
{

    //cout << "This call to SetProvenance should be implicit in the executive" << endl; 
    this->GetOutput()->GetProvenance()->AddAlgorithm( this->GetClassName() );

    int argIndex = -1; 

    map < string, void* >::iterator  it;

    it = this->inputArgs.find( "SetMapperBehavior" );
    if ( it != this->inputArgs.end() ) {

        argIndex++; 

        this->GetOutput()->GetProvenance()->AddAlgorithmArg(
            this->GetClassName(),
            argIndex,
            "MapperBehavior",
            *( static_cast<int*>( this->inputArgs[ it->first ] ) )
        );

    }

    it = this->inputArgs.find( "phiType" );
    if ( it != this->inputArgs.end() ) {

        argIndex++; 
        this->GetOutput()->GetProvenance()->AddAlgorithmArg(
            this->GetClassName(),
            argIndex,
            "PHIType",
            *( static_cast<int*>( this->inputArgs[ it->first ] ) )
        );

        it = this->inputArgs.find( "patientDeidentificationId" );
        if ( it != this->inputArgs.end() ) {
            argIndex++; 
            this->GetOutput()->GetProvenance()->AddAlgorithmArg(
                this->GetClassName(),
                argIndex,
                "patientDeidentificationId",
                *( static_cast<string*>( this->inputArgs[ it->first ] ) )
            );
        }

        it = this->inputArgs.find( "studyDeidentificationId" );
        if ( it != this->inputArgs.end() ) {
            argIndex++; 
            this->GetOutput()->GetProvenance()->AddAlgorithmArg(
                this->GetClassName(),
                argIndex,
                "studyDeidentificationId",
                *( static_cast<string*>( this->inputArgs[ it->first ] ) )
            );
        }
    }

    it = this->inputArgs.find( "temperature" );
    if ( it != this->inputArgs.end() ) {
        argIndex++; 
        this->GetOutput()->GetProvenance()->AddAlgorithmArg(
            this->GetClassName(),
            argIndex,
            "temperature",
            *( static_cast< float* >( this->inputArgs[ it->first ] ) )
        );
    }
}



/*!
 *  Initializes the svkDcmHeader adapter to a specific IOD type      
 *  and initizlizes the svkDcmHeader member of the svkImageData 
 *  object.    
 */
void svkGEPFileReader::InitDcmHeader()
{
    vtkDebugMacro( << this->GetClassName() << "::InitDcmHeader()" );

    //  This could be MRI or MRS:  
    svkIOD* iod = svkMRSIOD::New();
    iod->SetDcmHeader( this->GetOutput()->GetDcmHeader());
    iod->InitDcmHeader();
    iod->Delete();

    //  Parse the GE PFile into a string map, the
    //  contents of which will be used to initialize
    //  the svkImageData's DICOM header.
    this->ReadGEPFile(); 

    if ( ! this->onlyReadHeader ) {
        //  Fill in data set specific values using the appropriate mapper type:
        this->mapper = this->GetPFileMapper(); 
    
        //cout << "SWAP BYTES: " << this->GetSwapBytes() << endl;
        //cout << "MAPER FN: " << this->GetFileNames()->GetValue(0) << endl;

        this->mapper->SetPfileName( this->GetFileNames()->GetValue(0) ); 

        //  all the IE initialization modules would be contained within the 
        this->mapper->InitializeDcmHeader( 
            this->pfMap, 
            this->GetOutput()->GetDcmHeader(), 
            this->pfileVersion, 
            this->GetSwapBytes(), 
            this->inputArgs
        ); 

        //  Deidentify data if necessary
        map < string, void* >::iterator  it;
        it = this->inputArgs.find( "phiType" );
        if ( it != this->inputArgs.end() ) {
            this->DeidentifyData(); 
        }


        if (this->GetDebug()) { 
            this->GetOutput()->GetDcmHeader()->PrintDcmHeader();
        }

    }
}


/*!
 *  Sets mapper's data loading behavior.  
 */
void svkGEPFileReader::SetMapperBehavior(svkGEPFileMapper::MapperBehavior type) 
{
    svkGEPFileMapper::MapperBehavior* typeTmp = new svkGEPFileMapper::MapperBehavior(type); 
    this->inputArgs[ "SetMapperBehavior" ] = static_cast<void*>( typeTmp );

    map< string, void* >::iterator mapIter;
    for ( mapIter = this->inputArgs.begin(); mapIter != this->inputArgs.end(); ++mapIter ) {
        //cout << "input args: " << mapIter->first << " = " << *( static_cast<svkGEPFileMapper::MapperBehavior*>( this->inputArgs[ mapIter->first ] ) )<< endl;
    }

}


/*!
 */
void svkGEPFileReader::SetEPSIParams( EPSIType type, svkEPSIReorder::EPSIAxis axis, int first,
                                      int numLobes, int numSkip, int flipLobe )
{

    int* epsiType     = new int (type);
    int* epsiAxis     = new int (axis);
    int* epsiFirst    = new int (first);
    int* epsiNumLobes = new int (numLobes);
    int* epsiNumSkip  = new int (numSkip);
    int* epsiFlipLobe = new int (flipLobe);

    this->inputArgs.insert( pair<string, void*>( "epsiType",     static_cast<void*>( epsiType ) ) );
    this->inputArgs.insert( pair<string, void*>( "epsiAxis",     static_cast<void*>( epsiAxis ) ) );
    this->inputArgs.insert( pair<string, void*>( "epsiFirst",    static_cast<void*>( epsiFirst ) ) );
    this->inputArgs.insert( pair<string, void*>( "epsiNumLobes", static_cast<void*>( epsiNumLobes ) ) );
    this->inputArgs.insert( pair<string, void*>( "epsiNumSkip",  static_cast<void*>( epsiNumSkip ) ) );
    this->inputArgs.insert( pair<string, void*>( "epsiFlipLobe", static_cast<void*>( epsiFlipLobe) ) );

    this->SetMapperBehavior( svkGEPFileMapper::LOAD_EPSI );

}

/*!
 */
void svkGEPFileReader::SetEPSIParams( int flipLobe )
{

    int* epsiFlipLobe = new int (flipLobe);
    this->inputArgs.insert( pair<string, void*>( "epsiFlipLobe", static_cast<void*>( epsiFlipLobe) ) );

}

/*!
 *  Sets mapper's data deidentification behavior.  All PHI fields will be replaced with 
 *  this value in the loaded DCM header. 
 */
void svkGEPFileReader::SetDeidentify(svkDcmHeader::PHIType phiType) 
{
    svkDcmHeader::PHIType* phiTypeTmp = new svkDcmHeader::PHIType(phiType);
    this->inputArgs[ "phiType" ] = static_cast<void*>( phiTypeTmp );
    this->phiType = phiType; 
}


/*!
 *  Sets string to use for identification.  All PHI fields will be replaced with this 
 *  value in the loaded DCM header. See !svkDcmHeader::Deidentify().  
 */
void svkGEPFileReader::SetDeidentify( svkDcmHeader::PHIType phiType, string deidentificationId ) 
{
    svkDcmHeader::PHIType* phiTypeTmp = new svkDcmHeader::PHIType(phiType);
    string* idTmp = new string(deidentificationId);
    this->inputArgs[ "phiType" ] = static_cast<void*>( phiTypeTmp );
    this->inputArgs[ "studyDeidentificationId" ] = static_cast<void*>( idTmp );
    this->phiType = phiType; 
    this->deidPatientId = deidentificationId; 
    this->deidStudyId   = deidentificationId; 
}


/*!
 *  Sets string to use for identification.  All PHI fields will be replaced with this 
 *  value in the loaded DCM header. See !svkDcmHeader::Deidentify().  
 */
void svkGEPFileReader::SetDeidentify( svkDcmHeader::PHIType phiType, string patientId, string studyId ) 
{
    svkDcmHeader::PHIType* phiTypeTmp = new svkDcmHeader::PHIType(phiType);
    string* patIdTmp = new string(patientId);
    string* studyIdTmp = new string(studyId);
    this->inputArgs[ "phiType" ] = static_cast<void*>( phiTypeTmp );
    this->inputArgs[ "patientDeidentificationId" ] = static_cast<void*>( patIdTmp );
    this->inputArgs[ "studyDeidentificationId" ] = static_cast<void*>( studyIdTmp);
    this->phiType = phiType; 
    this->deidPatientId = patientId; 
    this->deidStudyId   = studyId; 
}


/*!
 *  Prints the GE PFile Header to stdout
 */
void svkGEPFileReader::PrintHeader()
{
    cout << "PRINT HEADER" << endl;
    this->DumpHeader(); 
}


/*!
 *  Prints the GE PFile Header to stdout
 */
void svkGEPFileReader::PrintShortHeader()
{
    cout << "PRINT SHORT HEADER" << endl;
    this->DumpShortHeader(); 
}


/*!
 *  Sets the acquisition temperature in degrees celcius.  
 *  Used to set the chemcial shift of water for the PPM reference.  
 */
void svkGEPFileReader::SetTemperature( float temp )
{
    float* flagTemp = new float(temp);
    // possibly enforce use of vtk primitive types, everything is then a vtkObject and we have a pseudo homogeneous container.
    this->inputArgs.insert( pair<string, void*>( "temperature", static_cast<void*>( flagTemp ) ) );

}

/*!
 *  Sets the value of chop for the acquisition. 
 */
void svkGEPFileReader::SetChop( bool chop )
{
    bool* chopTmp = new bool(chop); 
    this->inputArgs.insert( pair<string, void*>( "chop", static_cast<void*>(chopTmp) ) );
    
}


/*!
 *  Sets up deidentification call based on user input. 
 */ 
void svkGEPFileReader::DeidentifyData()
{

    map < string, void* >::iterator itPhi     = this->inputArgs.find( "phiType" );
    map < string, void* >::iterator itPatId   = this->inputArgs.find( "patientDeidentificationId" );
    map < string, void* >::iterator itStudyId = this->inputArgs.find( "studyDeidentificationId" );

    if ( itPatId != this->inputArgs.end() && itStudyId != this->inputArgs.end() ) { 
        this->GetOutput()->GetDcmHeader()->Deidentify( 
                                                *( static_cast< svkDcmHeader::PHIType* >( this->inputArgs[ itPhi->first ] ) ), 
                                                *( static_cast< string* >( this->inputArgs[ itPatId->first ] ) ),
                                                *( static_cast< string* >( this->inputArgs[ itStudyId->first ] ) ) 
                                           ); 
    } else if ( itStudyId != this->inputArgs.end() ) { 
        this->GetOutput()->GetDcmHeader()->Deidentify( 
                                                *( static_cast< svkDcmHeader::PHIType* >( this->inputArgs[ itPhi->first ] ) ), 
                                                *( static_cast< string* >( this->inputArgs[ itStudyId->first ] ) )
                                           ); 
    } else {
        this->GetOutput()->GetDcmHeader()->Deidentify( 
                                                *( static_cast< svkDcmHeader::PHIType* >( this->inputArgs[ itPhi->first ] ) )
                                           ); 
    }
}


/*
 *  Method to reset the "psdname" which is used to select a particular mapper class strategy for 
 *  reading parsing the data. 
 */
void svkGEPFileReader::SetPSDLogic( string psdName )
{
    this->psdLogic = psdName; 
}



/*!
 *  Create an svkGEPFileMapper of the appropriate type for the present pfile 
 */
svkGEPFileMapper* svkGEPFileReader::GetPFileMapper() 
{
    svkGEPFileMapper* aMapper = NULL; 

    string psd = this->pfMap["rhi.psdname"][3];
    //convert to lower case:
    string::iterator it;
    for ( it = psd.begin(); it < psd.end(); it++ ) {
        *it = (char)tolower(*it);
    }

    //  reset the psd to determine the mapper logic strategy if necessary
    if ( this->psdLogic.size() > 0 ) {
        psd.assign( this->psdLogic ); 
    }

  
    if ( ( psd.compare("probe-p") == 0 ) || ( psd.find("presscsi") != string::npos ) ) { 

        // product GE sequence:  
        aMapper = svkGEPFileMapper::New();

    } else if ( psd.find("prose_prostate_ucsf") != string::npos ) {

        //  UCSF Prostate MRSI sequence 
        aMapper = svkGEPFileMapperUCSFProseProstate::New();

    } else if ( psd.find("prose_breast_ucsf") != string::npos ) {

        //  UCSF prototype Breast MRSI sequence 
        aMapper = svkGEPFileMapperUCSFProseBreast::New();

    } else if ( ( psd.find("probe") != string::npos ) || ( psd.find("prose") != string::npos ) ) {

        //  Assume that if it's not an exact match that it is a UCSF research sequence. 
        aMapper = svkGEPFileMapperUCSF::New();

    } else if ( ( psd.find("fidcsi_ucsf_dev0") != string::npos ) || ( psd.find("fidcsi_ucsf_multipulse") != string::npos ) ) {

        //  fidcsi ucsf sequence 
        aMapper = svkGEPFileMapperUCSFfidcsiDev0::New();

    } else if ( psd.find("UCSF_MNS_7T") != string::npos ) {

        //  multi-nuclear spec ucsf sequence 
        aMapper = svkGEPFileMapperUCSFfidcsiDev07t::New();

    } else if ( psd.find("fidcsi") != string::npos ) {

        //  fidcsi ucsf sequence 
        //  should strive to use this one: aMapper = svkGEPFileMapperUCSFfidcsiDev0::New();
        aMapper = svkGEPFileMapperUCSFfidcsi::New();

    } else if ( ( psd.compare("jpress") == 0 ) || ( psd.compare("mbrease_dev") == 0 ) ) {

        aMapper = svkGEPFileMapperMBrease::New();

    } else if ( psd.compare("mpcsiobl") == 0  ) {

        aMapper = svkGEPFileMapperMPCSIOBL::New();

    } else {

        vtkErrorMacro("No PFile mapper available for " << psd );

    }

    cout << "PFile Mapper Type for: " << psd << " = " << aMapper->GetClassName() << endl;

    return aMapper;          
}


/*!
 *  returns the mapper 
 */
svkGEPFileMapper* svkGEPFileReader::GetMapper() 
{
    return this->mapper; 
}


/*!
 *  Read GE Pfile  header fields into a string STL map for use during initialization
 *  of DICOM header by Init*Module methods.
 */
void svkGEPFileReader::ReadGEPFile()
{

    if (this->GetDebug()) { 
        cout << "FN: " << this->GetFileName() << endl;
    }
        
    // temporary workaround becaues pfiles from dynamic series don't share a series UID
    this->checkSeriesUID = false;  
    string refSUID = this->GetSeriesUID( this->GetFileName());
   
    this->GlobFileNames();  	 	 

    vtkStringArray* validatedFileNames = vtkStringArray::New();

    //  Verify that all the pfiles belong to the same series (same 
    //  series instance uid as specified input file): 
    for ( int fileNumber = 0; fileNumber < this->GetFileNames()->GetNumberOfValues(); fileNumber++ ) {  
        if ( this->GetSeriesUID(this->GetFileNames()->GetValue(fileNumber)).compare(refSUID) == 0)  {
            validatedFileNames->InsertNextValue( this->GetFileNames()->GetValue(fileNumber) );               
        } 
    }  	 	 

    this->SetFileNames(validatedFileNames); 
    validatedFileNames->Delete();

    if (this->GetDebug()) { 
        cout << "NUMBER FILES: " <<  this->GetFileNames()->GetNumberOfValues() << endl;
        for ( int fileNumber = 0; fileNumber < this->GetFileNames()->GetNumberOfValues(); fileNumber++ ) {  
               cout << "VALIDATEDFN: " << this << " " << this->GetFileNames()->GetValue(fileNumber) << endl;
        }  	 	 
    }


    try {

        // In case the stream is open from can read
        if ( this->gepf != NULL )  {
            if( this->gepf->is_open() ) {
                this->gepf->close();
            }
            delete this->gepf;
            this->gepf = NULL;
        }

        //  Read in the GE Pfile Header:
        this->gepf = new ifstream();
        this->gepf->exceptions( ifstream::eofbit | ifstream::failbit | ifstream::badbit );

        this->gepf->open( this->GetFileNames()->GetValue(0), ios::binary ); 

        //  Iterate through map, get offset and read the appropriate length at that offset.  fix endianness. 
        this->ParsePFile(); 

        this->gepf->close();     

    } catch (const exception& e) {
        cerr << "ERROR opening or reading GE P-File (" << this->GetFileName() << "): " << e.what() << endl;
    }

}


/*  
 *  Quick parse of header to get seriesInstanceUID
 */
string svkGEPFileReader::GetSeriesUID(const char* fname)
{
    string seriesInstanceUID;     

    //  to avoid recursion when getting the seriesUID from just one file: 
    if ( this->checkSeriesUID ) {

        try {

            svkGEPFileReader* tmpReader = svkGEPFileReader::New();
            tmpReader->SetFileName(fname);
            tmpReader->OnlyReadHeader(true); 
            tmpReader->checkSeriesUID = false; 
            tmpReader->OnlyReadOneInputFile(); 
            tmpReader->Update();
            //  this will be the compressed value, but serves the purpose.     
            seriesInstanceUID = ( tmpReader->GetPFMap() )["rhs.series_uid"][3]; 
            tmpReader->Delete();    

        } catch (const exception& e) {
            cerr << "ERROR opening or reading GE P-File (" << fname << "): " << e.what() << endl;
        }
    } else {
        seriesInstanceUID = "0"; 
    }

    return seriesInstanceUID;     
    
}


/*!
 *  Initializes the map of pfile offsets and data types, parses the pfile header
 *  and sets the field value in the map to a string representation of the header field. 
 *
 *  key        pfMap[key][0]    pfMap[key][1] pfMap[key][2] pfMap[key][3]
 *  ----------------------------------------------------------------------
 *  fieldName  nativeType       numElements   offset        stringValue
 */
void svkGEPFileReader::ParsePFile()
{

    this->pfileVersion = this->GetPFileVersion(); 
    this->SetDataByteOrder();
    this->InitOffsetsMap(); 

    map< string, vector<string> >::iterator mapIter;
    string key; 
    for ( mapIter = this->pfMap.begin(); mapIter != this->pfMap.end(); ++mapIter ) {
        key = mapIter->first; 
        this->pfMap[key].push_back( this->GetFieldAsString( key ) );
    }

    this->FillInMissingInfo();

    if ( this->GetDebug() ) { 
        this->PrintKeyValuePairs(); 
    }
}


/*!
 *  Some earlier pfile versions don't have all information explicitly encoded, so add 
 *  the known values to the pfMap here:
 */
void svkGEPFileReader::FillInMissingInfo()
{

    this->pfMap["rhr.rdb_hdr_off_data"].push_back( "INT_4" );  //type
    this->pfMap["rhr.rdb_hdr_off_data"].push_back( "1" );      //num elements 
    this->pfMap["rhr.rdb_hdr_off_data"].push_back( "" );       //offset
    if ( (int)(this->pfileVersion) == 9 ) {
        this->pfMap["rhr.rdb_hdr_off_data"].push_back( "39984" );  
    } else if ( (int)(this->pfileVersion) == 11 ) {
        this->pfMap["rhr.rdb_hdr_off_data"].push_back( "61464" ); 
    }
}


/*!
 *
 */
int svkGEPFileReader::FillOutputPortInformation( int vtkNotUsed(port), vtkInformation* info )
{
    info->Set(vtkDataObject::DATA_TYPE_NAME(), "svkMrsImageData");
    return 1;
}


/*!
 *  Prints the key value pairs parsed from the header.
 */
void svkGEPFileReader::PrintOffsets()
{
    //  Print out key value pairs parsed from header:
    map< string, vector<string> >::iterator mapIter;
    for ( mapIter = this->pfMap.begin(); mapIter != this->pfMap.end(); ++mapIter ) {

        cout << "OFFSETS " << " " << mapIter->first << " = ";
    
        vector<string>::iterator it;
        for ( it = this->pfMap[mapIter->first].begin() ; it < this->pfMap[mapIter->first].end(); it++ ) {
            cout << " " << *it ;           
        }
        cout << endl;
    }
}


/*!
 *  Prints the key value pairs parsed from the header.
 */
void svkGEPFileReader::DumpHeader()
{
    //  Print out key value pairs parsed from header:
    map< string, vector<string> >::iterator mapIter;
    for ( mapIter = this->pfMap.begin(); mapIter != this->pfMap.end(); ++mapIter ) {
        cout << setiosflags(ios::left) << setw(25) << mapIter->first << " = " <<  this->pfMap[mapIter->first][3] << endl;
    }
}


/*!
 *  Prints the key value pairs parsed from the header.
 */
void svkGEPFileReader::DumpShortHeader()
{
    cout << setiosflags(ios::left) << setw(25) << "rhe.patname               = " <<  this->pfMap["rhe.patname"][3] << endl;
    cout << setiosflags(ios::left) << setw(25) << "rhe.patid                 = " <<  this->pfMap["rhe.patid"][3] << endl;
    cout << setiosflags(ios::left) << setw(25) << "rhr.rh_scan_date          = " <<  this->pfMap["rhr.rh_scan_date"][3] << endl;
    cout << setiosflags(ios::left) << setw(25) << "rhr.rh_scan_time          = " <<  this->pfMap["rhr.rh_scan_time"][3] << endl;
    cout << setiosflags(ios::left) << setw(25) << "rhe.ex_no                 = " <<  this->pfMap["rhe.ex_no"][3] << endl;
    cout << setiosflags(ios::left) << setw(25) << "rhs.se_desc               = " <<  this->pfMap["rhs.se_desc"][3] << endl;
}


/*!
 *  Prints the key value pairs parsed from the header.
 */
void svkGEPFileReader::PrintKeyValuePairs()
{

    //  Print out key value pairs parsed from header:
    map< string, vector<string> >::iterator mapIter;
    for ( mapIter = this->pfMap.begin(); mapIter != this->pfMap.end(); ++mapIter ) {

        cout << "KEY:VALUE: " << " " << mapIter->first << " = ";

        vector<string>::iterator it;
        for ( it = this->pfMap[mapIter->first].begin() ; it < this->pfMap[mapIter->first].end(); it++ ) {
            cout << " " << *it ;
        }
        cout << endl;
    }
}


/*!
 *  Sets the byte order of the raw file based on it's version 
 *  needs to be more granular for platform/arch. 
 */
void svkGEPFileReader::SetDataByteOrder()
{
    //  These were acquired on SGI (bigendian)
    if (this->pfileVersion < 11) {

        this->SetDataByteOrderToBigEndian();

    } else if (this->pfileVersion >= 11) {

        this->SetDataByteOrderToLittleEndian();

    }
}


/*!
 *  
 *  Initialize map of field names to dataType, size and offset.  This map is used to parse 
 *  GE Pfile: 
 *
 *  //  fieldName           nativeType  numElements offset stringValue
 *  //  ----------------------------------------------------------
 *  //  rhr.rh_rdbm_rev   , FLOAT_4,    1   ,       0       value
 *  //  rhr.rh_logo       , CHAR   ,    10  ,       34      value
 * 
 */
void svkGEPFileReader::InitOffsetsMap()
{
    string offsets = this->GetOffsetsString();

    size_t delim;

    string fieldName;
    string val;

    //  If pfMap has already been initialized with offsets, 
    //  then just return 
    if ( this->pfMap.size() > 0 ) {
        return; 
    }

    while ( (delim = offsets.find(",")) != string::npos ) {

        fieldName = this->StripWhite( offsets.substr( 0, delim ) );

        offsets = offsets.substr( delim +1 );

        for ( int i = 0; i < 3; i++ ) {
            delim = offsets.find(",");
            val = offsets.substr( 0, delim );
            this->pfMap[fieldName].push_back(val);
            offsets = offsets.substr( delim +1 );
        }
    }

    if (this->GetDebug()) { 
        this->PrintOffsets();
    }
}


/*!
 *  Returns the file root without extension
 */
svkDcmHeader::DcmPixelDataFormat svkGEPFileReader::GetFileType()
{
    return svkDcmHeader::SIGNED_FLOAT_4;
}


/*!
 *
 *  Read from file, data of specified type and offset into a string.  
 */
string svkGEPFileReader::GetFieldAsString( string key )
{

    //  Seek to offset for the specified key:
    int offset;
    istringstream* issOffset = new istringstream();
    issOffset->str( this->pfMap[key][2] );
    *issOffset >> offset;
    delete issOffset; 
    this->gepf->seekg( offset , ios_base::beg );

    ostringstream ossValue;

    string type = this->StripWhite( this->pfMap[key][0] ); 

    int numElements;
    istringstream* iss = new istringstream();
    iss->str( this->pfMap[key][1] );
    *iss >> numElements;

    if ( type.compare("FLOAT_4") == 0) {
        float floatVal; 
        this->gepf->read( (char*)(&floatVal), 4 * numElements );
        if ( this->GetSwapBytes() ) {
            vtkByteSwap::SwapVoidRange((void *)&floatVal, 1, sizeof(float));
        }
        ossValue << floatVal;
    } else if ( type.compare("INT_2") == 0 ) {
        short shortVal; 
        this->gepf->read( (char*)(&shortVal), 2 * numElements );
        if ( this->GetSwapBytes() ) {
            vtkByteSwap::SwapVoidRange((void *)&shortVal, 1, sizeof(short));
        }
        ossValue << shortVal;
    } else if ( type.compare("UINT_2") == 0 ) {
        unsigned short ushortVal; 
        this->gepf->read( (char*)(&ushortVal), 2 * numElements );
        if ( this->GetSwapBytes() ) {
            vtkByteSwap::SwapVoidRange((void *)&ushortVal, 1, sizeof(unsigned short));
        }
        ossValue << ushortVal;
    } else if ( type.compare("UINT_4") == 0 ) {
        unsigned int uintVal; 
        this->gepf->read( (char*)(&uintVal), 4 * numElements );
        if ( this->GetSwapBytes() ) {
            vtkByteSwap::SwapVoidRange((void *)&uintVal, 1, sizeof(unsigned int));
        }
        ossValue << uintVal;
    } else if ( type.compare("INT_4") == 0) {
        int intVal; 
        this->gepf->read( (char*)(&intVal), 4 * numElements );
        if ( this->GetSwapBytes() ) {
            vtkByteSwap::SwapVoidRange((void *)&intVal, 1, sizeof(int));
        }
        ossValue << intVal;
    } else if ( type.compare("LINT_4") == 0) {
        int intVal; 
        this->gepf->read( (char*)(&intVal), 4 * numElements );
        if ( this->GetSwapBytes() ) {
            vtkByteSwap::SwapVoidRange((void *)&intVal, 1, sizeof(int));
        }
        ossValue << intVal;
    } else if ( type.compare("ULINT_4") == 0) {
        unsigned int ulintVal; 
        this->gepf->read( (char*)(&ulintVal), 4 * numElements );
        if ( this->GetSwapBytes() ) {
            vtkByteSwap::SwapVoidRange((void *)&ulintVal, 1, sizeof(unsigned int));
        }
        ossValue << ulintVal;
    } else if ( type.compare("LINT_8") == 0) {
        //  a long int may be 4 or 8 bytes, but a long long should be 
        //  8 bytes as required: 
        long long int intVal; 
        this->gepf->read( (char*)(&intVal), 8 * numElements );
        if ( this->GetSwapBytes() ) {
            vtkByteSwap::SwapVoidRange((void *)&intVal, 1, sizeof(long long int));
        }
        ossValue << intVal;
    } else if ( type.compare("CHAR") == 0) {
        //  null terminate char array: 
        char* charVal = new char[numElements + 1];  
        this->gepf->read( charVal, 1 * numElements );
        charVal[numElements] = '\0';
        ossValue << charVal;
        delete [] charVal; 
    } else if ( type.compare("UID") == 0) {
        char* charVal = new char[numElements+1];  
        this->gepf->read( charVal, 1 * numElements );
        charVal[numElements] = '\0'; 
        ossValue << charVal;
        delete [] charVal; 
        //  Uncompress UIDfields:
        string tmp = this->UncompressUID( ossValue.str().c_str() );  
        ossValue.clear();     
        ossValue.str("");     
        ossValue << tmp ; 
    } else {
        cout << "CAN NOT DETERMINE TYPE " << endl;
    }
    
    delete iss; 

    if (this->GetDebug()) { 
        cout << key << ":" <<  ossValue.str() << endl; 
    }

    return ossValue.str();
}


/*!
 *  The following code for compresses the UIDs as required by GE
 *  input long_uid, and get short_uid out
 */
int svkGEPFileReader::GECompressUID(unsigned char *short_uid, char *long_uid)
{
    const int UID_FAIL = 0;  
    const int UID_OK = 1;
    const int UID_LEN = 64;
    const int RT_FOUR_BITS = 0x0f;
    const int LT_FOUR_BITS = 0xf0;
    const int DB_UID_LEN = 32;

    int i, len;

    //memset(long_uid, '\0', UID_LEN + 1);

    if((len = strlen((char *)long_uid)) > UID_LEN) {
        return(UID_FAIL);
    }

    //  compress the uid so that the first char of the long uid goes into the
    //  left 4 bits of the first short_uid element. 
    //  The 2nd char of the long goes into the right 4 bits of the first short_uid element. 
    //  etc. 
    char leftBits;  
    char rightBits;  
    int  lastIndex = 0;
    for(i = 0; i < len; i++) {
      
        // Shift the hex range for the numeric ASCII chars to hex 0x0: 
        if( 0x30 <= long_uid[i] && long_uid[i] <= 0x39) {
            long_uid[i] -= ('0' - 1);
        } else if (long_uid[i] == '.') {
            long_uid[i] = 0xb; 
        } else if (long_uid[i] == '\0') {
            long_uid[i] = 0x0; 
        } else {
            return(UID_FAIL);
        }

        //  Now encode the even chars in the left 4 bits of the 
        //  short_uid elements, and the odd chars in the right 
        //  4 bits of the same elements of the short_uid.  

        //  get the left 4 bytes of the short_uid element:  
        if(i % 2 == 0) { 
            //  mask out the left 4 bits to 0
            leftBits = long_uid[i] & RT_FOUR_BITS; 
            //  shift the right 4 bits over to the left. 
            leftBits <<= 4; 
            //  mask out the right 4 bits to 0 just to be sure. 
            leftBits &= LT_FOUR_BITS; 
        } else {
            //  mask out the left 4 bits to 0
            rightBits = long_uid[i] & RT_FOUR_BITS;  
            short_uid[i/2] = leftBits | rightBits; 
            lastIndex = i/2;
        }

    }

    //  if there are an odd number of chars in the array, make sure to add the remaining leftBits: 
    if ( i == len && len % 2 != 0 ) {
        short_uid[i/2] = leftBits; 
        lastIndex = i/2;
    }

    for(i = lastIndex + 1 ; i < 32; i++) {
        short_uid[i] = 0x0;
    }

    return(UID_OK);
}


/*!
 *  The following code for uncompressing the UIDs may be directly 
 *  from GE.  Get clearance.  
 */
int svkGEPFileReader::GEUncompressUID(unsigned char *short_uid, char *long_uid)
{
    const int UID_FAIL = 0;  
    const int UID_OK = 1;
    const int UID_LEN = 64;
    const int RT_FOUR_BITS = 0x0f;
    const int LT_FOUR_BITS = 0xf0;
    const int DB_UID_LEN = 32;

    int i, len;

    memset(long_uid, '\0', UID_LEN + 1);

    if((len = strlen((char *)short_uid)) > DB_UID_LEN) {
        return(UID_FAIL);
    }

    /* expanding the uid */
    for(i = 0; i < len * 2; i++) {
        /* get the proper value from the short_uid/compressed string */
        /* an even number */
        if(i % 2 == 0) { 
            long_uid[i] = short_uid[i/2] & LT_FOUR_BITS;
            long_uid[i] >>= 4;
            long_uid[i] &= 0x0f;
        } else {
            long_uid[i] = short_uid[i/2] & RT_FOUR_BITS;
        }

        /* we have the proper value, now expand/decompress it */
        if(0x1 <= long_uid[i] && long_uid[i] <= 0xa) {
            long_uid[i] += '0' - 1;
        } else if (long_uid[i] == 0xb) {
            long_uid[i] = '.';
        } else if (long_uid[i] == 0x0) {
            break;  /* This is the end, no need to look further */
        } else {
            memset(long_uid, '\0', UID_LEN + 1);
            return(UID_FAIL);
        }
    }
    return(UID_OK);
}


/*
 *  Uncompresses the UID from the raw header. 
 */
string svkGEPFileReader::UncompressUID(const char* compressedUID)
{
    char uncompressedUID[BUFSIZ];
    this->GEUncompressUID((unsigned char*)compressedUID, uncompressedUID);
    return string(uncompressedUID);
}


/*
 *  Compresses the UID for raw header. 
 */
string svkGEPFileReader::CompressUID(char* uncompressedUID)
{
    char compressedUID[BUFSIZ];
    if ( ! this->GECompressUID((unsigned char*)compressedUID, uncompressedUID) ) {
        cout << "ERROR! Value does not represent a valid UID: " << uncompressedUID << endl;
        exit(1);
    }
    return string(compressedUID);
}


/*!  
 *  Get the pfile version number from already opened pfile file. 
 *  Returns 0 if version can't be determined. 
 */
float svkGEPFileReader::GetPFileVersion() 
{
    
    float version = 0;
    float rdbmRev = 0.;
    float rdbmRevSwapped;

    //  The byte ordering probably depends on the native platform for that 
    //  revision, so we're guessing here initially.
    if ( this->gepf->is_open() ) {
        gepf->seekg( 0, ios_base::beg );
        gepf->read( (char*)(&rdbmRev), 4);
        rdbmRevSwapped = rdbmRev;
        vtkByteSwap::SwapVoidRange((void *)&rdbmRevSwapped, 1, sizeof(float));
        //cout << "rdbm rev" << setprecision(8) << rdbmRev << " " << rdbmRevSwapped <<  endl;
    }

    version = svkGEPFileReader::LookupRawVersion(rdbmRev, rdbmRevSwapped);

    return version; 
}

/*!
 *  Lookup version from rdbmRev:      
 */ 
float svkGEPFileReader::LookupRawVersion(float rdbmRev, float rdbmRevSwapped) 
{
    float version = 0; 
    //  
    //  Map the rdbm revision to a platform number (e.g. 11.x, 12.x, etc.) 
    //
    ostringstream ossValue;
    ossValue << rdbmRev;
    string rdbmRevString = ossValue.str();

    ostringstream ossValueSwapped;
    ossValueSwapped << rdbmRevSwapped;
    string rdbmRevSwappedString = ossValueSwapped.str();

    if ( (rdbmRev > 6.95 && rdbmRev <= 8.0) ||  (rdbmRevSwapped > 6.95 && rdbmRevSwapped <= 8.0) ) { 
        version = 9.0; 
    } else if ( rdbmRev == 9.0  || rdbmRevSwapped == 9.0  ) { 
        version = 11.0;
    } else if ( rdbmRev == 11.0 || rdbmRevSwapped == 11.0 ) { 
        version = 12.0;
    } else if ( (int)rdbmRev == 14 || (int)rdbmRevSwapped == 14 ) { 
        version = 14.0;
    } else if ( (int)rdbmRev == 15 || (int)rdbmRevSwapped == 15 ) { 
        version = 15.0;
    } else if ( (int)rdbmRev == 16 || (int)rdbmRevSwapped == 16 ) { 
        version = 16.0;
    } else if ( rdbmRevString.compare("20.007") == 0 || rdbmRevSwappedString.compare("20.007") == 0 ) { 
        version = 23;
    } else if ( (int)rdbmRev == 20 || (int)rdbmRevSwapped == 20 ) { 
        version = 20.0;
    } else if ( (int)rdbmRev == 21 || (int)rdbmRevSwapped == 21 ) { 
        version = 21.0;
    } else if ( (int)rdbmRev == 24 || (int)rdbmRevSwapped == 24 ) { 
        version = 24.0;
    } else if ( (int)rdbmRev == 26 || (int)rdbmRevSwapped == 26 ) { 
        version = 26.0;
    } 
    return version; 
}


/*!
 *  Get a string containing a mapping of offsets and datatype for pfile fields. 
 *  for the specified pfile version. 
 */
string svkGEPFileReader::GetOffsetsString()
{

    //  fieldName                           nativeType  numElements offset stringValue
    //  ----------------------------------------------------------
    //  rhr.rh_rdbm_rev                    , FLOAT_4,   1   ,       0       value
    //  rhr.rh_logo                        , CHAR   ,   10  ,       34

    string offsets; 

    if ( (int)(this->pfileVersion) == 9 ) {

        offsets.assign (" \
            rhr.rh_rdbm_rev                    , FLOAT_4, 1   , 0,\
            rhr.rh_scan_date                   , CHAR   , 10  , 16,\
            rhr.rh_scan_time                   , CHAR   , 8   , 26,\
            rhr.rh_npasses                     , INT_2  , 1   , 64,\
            rhr.rh_nslices                     , INT_2  , 1   , 68,\
            rhr.csi_dims                       , INT_2  , 1   , 372,\
            rhr.rh_dab[0].start_rcv            , INT_2  , 1   , 200,\
            rhr.rh_dab[0].stop_rcv             , INT_2  , 1   , 202,\
            rhr.rh_dab[1].start_rcv            , INT_2  , 1   , 204,\
            rhr.rh_dab[1].stop_rcv             , INT_2  , 1   , 206,\
            rhr.rh_dab[2].start_rcv            , INT_2  , 1   , 208,\
            rhr.rh_dab[2].stop_rcv             , INT_2  , 1   , 210,\
            rhr.rh_dab[3].start_rcv            , INT_2  , 1   , 212,\
            rhr.rh_dab[3].stop_rcv             , INT_2  , 1   , 214,\
            rhr.rh_data_collect_type           , INT_2  , 1   , 56,\
            rhr.rh_file_contents               , INT_2  , 1   , 44,\
            rhr.rh_frame_size                  , INT_2  , 1   , 80,\
            rhr.rh_point_size                  , INT_2  , 1   , 82,\
            rhr.rh_ps_mps_freq                 , LINT_4 , 1   , 424,\
            rhr.rh_user_usage_tag              , ULINT_4, 1   , 988,\
            rhr.roilenx                        , FLOAT_4, 1   , 380,\
            rhr.roileny                        , FLOAT_4, 1   , 384,\
            rhr.roilenz                        , FLOAT_4, 1   , 388,\
            rhr.spectral_width                 , FLOAT_4, 1   , 368,\
            rhr.xcsi                           , INT_2  , 1   , 374,\
            rhr.ycsi                           , INT_2  , 1   , 376,\
            rhr.zcsi                           , INT_2  , 1   , 378,\
            rhr.rh_logo                        , CHAR   , 10  , 34,\
            rhr.rh_raw_pass_size               , LINT_4 , 1   , 116,\
            rhr.rh_user0                       , FLOAT_4, 1   , 216,\
            rhr.rh_user1                       , FLOAT_4, 1   , 220,\
            rhr.rh_user2                       , FLOAT_4, 1   , 224,\
            rhr.rh_user3                       , FLOAT_4, 1   , 228,\
            rhr.rh_user4                       , FLOAT_4, 1   , 232,\
            rhr.rh_user5                       , FLOAT_4, 1   , 236,\
            rhr.rh_user6                       , FLOAT_4, 1   , 240,\
            rhr.rh_user7                       , FLOAT_4, 1   , 244,\
            rhr.rh_user8                       , FLOAT_4, 1   , 248,\
            rhr.rh_user9                       , FLOAT_4, 1   , 252,\
            rhr.rh_user10                      , FLOAT_4, 1   , 256,\
            rhr.rh_user11                      , FLOAT_4, 1   , 260,\
            rhr.rh_user12                      , FLOAT_4, 1   , 264,\
            rhr.rh_user13                      , FLOAT_4, 1   , 268,\
            rhr.rh_user14                      , FLOAT_4, 1   , 272,\
            rhr.rh_user15                      , FLOAT_4, 1   , 276,\
            rhr.rh_user16                      , FLOAT_4, 1   , 280,\
            rhr.rh_user17                      , FLOAT_4, 1   , 284,\
            rhr.rh_user18                      , FLOAT_4, 1   , 288,\
            rhr.rh_user19                      , FLOAT_4, 1   , 292,\
            rhr.rh_user20                      , FLOAT_4, 1   , 1000,\
            rhr.rh_user21                      , FLOAT_4, 1   , 1004,\
            rhr.rh_user22                      , FLOAT_4, 1   , 1008,\
            rhr.rh_user23                      , FLOAT_4, 1   , 1012,\
            rhr.rh_user24                      , FLOAT_4, 1   , 1016,\
            rhr.rh_user25                      , FLOAT_4, 1   , 1020,\
            rhr.rh_user26                      , FLOAT_4, 1   , 1024,\
            rhr.rh_user27                      , FLOAT_4, 1   , 1028,\
            rhr.rh_user28                      , FLOAT_4, 1   , 1032,\
            rhr.rh_user29                      , FLOAT_4, 1   , 1036,\
            rhr.rh_user30                      , FLOAT_4, 1   , 1040,\
            rhr.rh_user31                      , FLOAT_4, 1   , 1044,\
            rhr.rh_user32                      , FLOAT_4, 1   , 1048,\
            rhr.rh_user33                      , FLOAT_4, 1   , 1052,\
            rhr.rh_user34                      , FLOAT_4, 1   , 1056,\
            rhr.rh_user35                      , FLOAT_4, 1   , 1060,\
            rhr.rh_user36                      , FLOAT_4, 1   , 1064,\
            rhr.rh_user37                      , FLOAT_4, 1   , 1068,\
            rhr.rh_user38                      , FLOAT_4, 1   , 1072,\
            rhr.rh_user39                      , FLOAT_4, 1   , 1076,\
            rhr.rh_user40                      , FLOAT_4, 1   , 1080,\
            rhr.rh_user41                      , FLOAT_4, 1   , 1084,\
            rhr.rh_user42                      , FLOAT_4, 1   , 1088,\
            rhr.rh_user43                      , FLOAT_4, 1   , 1092,\
            rhr.rh_user44                      , FLOAT_4, 1   , 1096,\
            rhr.rh_user45                      , FLOAT_4, 1   , 1100,\
            rhr.rh_user46                      , FLOAT_4, 1   , 1104,\
            rhr.rh_user47                      , FLOAT_4, 1   , 1108,\
            rhr.rh_user48                      , FLOAT_4, 1   , 1112,\
            rhi.psdname                        , CHAR   , 33  , 39260,\
            rhi.scanspacing                    , FLOAT_4, 1   , 39060,\
            rhi.te                             , INT_4  , 1   , 39148,\
            rhi.ti                             , INT_4  , 1   , 39144,\
            rhi.tr                             , INT_4  , 1   , 39140,\
            rhi.tlhc_A                         , FLOAT_4, 1   , 39104,\
            rhi.tlhc_R                         , FLOAT_4, 1   , 39100,\
            rhi.tlhc_S                         , FLOAT_4, 1   , 39108,\
            rhi.t                              , INT_4  , 1   , 39140,\
            rhi.trhc_A                         , FLOAT_4, 1   , 39116,\
            rhi.trhc_R                         , FLOAT_4, 1   , 39112,\
            rhi.trhc_S                         , FLOAT_4, 1   , 39120,\
            rhi.user0                          , FLOAT_4, 1   , 39364,\
            rhi.user1                          , FLOAT_4, 1   , 39368,\
            rhi.user2                          , FLOAT_4, 1   , 39372,\
            rhi.user3                          , FLOAT_4, 1   , 39376,\
            rhi.user4                          , FLOAT_4, 1   , 39380,\
            rhi.user5                          , FLOAT_4, 1   , 39384,\
            rhi.user6                          , FLOAT_4, 1   , 39388,\
            rhi.user7                          , FLOAT_4, 1   , 39392,\
            rhi.user8                          , FLOAT_4, 1   , 39396,\
            rhi.user9                          , FLOAT_4, 1   , 39400,\
            rhi.user10                         , FLOAT_4, 1   , 39404,\
            rhi.user11                         , FLOAT_4, 1   , 39408,\
            rhi.user12                         , FLOAT_4, 1   , 39412,\
            rhi.user13                         , FLOAT_4, 1   , 39416,\
            rhi.user14                         , FLOAT_4, 1   , 39420,\
            rhi.user15                         , FLOAT_4, 1   , 39424,\
            rhi.user16                         , FLOAT_4, 1   , 39428,\
            rhi.user17                         , FLOAT_4, 1   , 39432,\
            rhi.user18                         , FLOAT_4, 1   , 39436,\
            rhi.user19                         , FLOAT_4, 1   , 39440,\
            rhi.user20                         , FLOAT_4, 1   , 39444,\
            rhi.user21                         , FLOAT_4, 1   , 39448,\
            rhi.user22                         , FLOAT_4, 1   , 39452,\
            rhi.user23                         , FLOAT_4, 1   , 39456,\
            rhi.user24                         , FLOAT_4, 1   , 39460,\
            rhi.user25                         , FLOAT_4, 1   , 39840,\
            rhi.user26                         , FLOAT_4, 1   , 39844,\
            rhi.user27                         , FLOAT_4, 1   , 39848,\
            rhi.user28                         , FLOAT_4, 1   , 39852,\
            rhi.user29                         , FLOAT_4, 1   , 39856,\
            rhi.user30                         , FLOAT_4, 1   , 39860,\
            rhi.user31                         , FLOAT_4, 1   , 39864,\
            rhi.user32                         , FLOAT_4, 1   , 39868,\
            rhi.user33                         , FLOAT_4, 1   , 39872,\
            rhi.user34                         , FLOAT_4, 1   , 39876,\
            rhi.user35                         , FLOAT_4, 1   , 39880,\
            rhi.user36                         , FLOAT_4, 1   , 39884,\
            rhi.user37                         , FLOAT_4, 1   , 39888,\
            rhi.user38                         , FLOAT_4, 1   , 39892,\
            rhi.user39                         , FLOAT_4, 1   , 39896,\
            rhi.user40                         , FLOAT_4, 1   , 39900,\
            rhi.user41                         , FLOAT_4, 1   , 39904,\
            rhi.user42                         , FLOAT_4, 1   , 39908,\
            rhi.user43                         , FLOAT_4, 1   , 39912,\
            rhi.user44                         , FLOAT_4, 1   , 39916,\
            rhi.user45                         , FLOAT_4, 1   , 39920,\
            rhi.user46                         , FLOAT_4, 1   , 39924,\
            rhi.user47                         , FLOAT_4, 1   , 39928,\
            rhi.user48                         , FLOAT_4, 1   , 39932,\
            rhi.cname                          , CHAR   , 17  , 39316,\
            rhi.brhc_A                         , FLOAT_4, 1   , 39128,\
            rhi.brhc_R                         , FLOAT_4, 1   , 39124,\
            rhi.brhc_S                         , FLOAT_4, 1   , 39132,\
            rhi.ctr_A                          , FLOAT_4, 1   , 39080,\
            rhi.ctr_R                          , FLOAT_4, 1   , 39076,\
            rhi.ctr_S                          , FLOAT_4, 1   , 39084,\
            rhi.dfov                           , FLOAT_4, 1   , 38976,\
            rhi.freq_dir                       , INT_2  , 1   , 39704,\
            rhi.ctyp                           , INT_2  , 1   , 39314,\
            rhi.loc                            , FLOAT_4, 1   , 39072,\
            rhi.mr_flip                        , INT_2  , 1   , 39200,\
            rhi.nex                            , FLOAT_4, 1   , 39164,\
            rhi.numecho                        , INT_2  , 1   , 39156,\
            rhi.image_uid                      , UID    , 32  , 39708,\
            rhi.rawrunnum                      , INT_4  , 1   , 39340,\
            rhe.ex_datetime                    , INT_4  , 1   , 37084,\
            rhe.ex_no                          , UINT_2 , 1   , 36880,\
            rhe.magstrength                    , INT_4  , 1   , 36956,\
            rhe.patid                          , CHAR   , 13  , 36960,\
            rhe.patidff                        , CHAR   , 65  , 37521,\
            rhe.patname                        , CHAR   , 25  , 36973,\
            rhe.patnameff                      , CHAR   , 65  , 37456,\
            rhe.refphy                         , CHAR   , 33  , 37088,\
            rhe.reqnum                         , CHAR   , 13  , 37071,\
            rhe.reqnumff                       , CHAR   , 17  , 37586,\
            rhe.study_uid                      , UID    , 32  , 37358,\
            rhe.dateofbirth                    , CHAR   , 9   , 37603,\
            rhe.patsex                         , INT_2  , 1   , 37002,\
            rhe.hospname                       , CHAR   , 33  , 36882,\
            rhe.ex_sysid                       , CHAR   , 9   , 37200,\
            rhe.uniq_sys_id                    , CHAR   , 16  , 37322,\
            rhe.ex_verscre                     , CHAR   , 2   , 37236,\
            rhs.se_no                          , INT_2  , 1   , 37922,\
            rhs.se_desc                        , CHAR   , 30  , 37932,\
            rhs.entry                          , INT_4  , 1   , 37992,\
            rhs.position                       , INT_4  , 1   , 37988,\
            rhs.series_uid                     , UID    , 32  , 38256,\
            rhs.landmark_uid                   , UID    , 32  , 38288,\
            rhs.anref                          , CHAR   , 3   , 37996,\
        "); 

    } else if ( (int)(this->pfileVersion) == 11 ) {

        offsets.assign (" \
            rhr.rh_rdbm_rev                    , FLOAT_4, 1   , 0,\
            rhr.rh_scan_date                   , CHAR   , 10  , 16,\
            rhr.rh_scan_time                   , CHAR   , 8   , 26,\
            rhr.rh_npasses                     , INT_2  , 1   , 64,\
            rhr.rh_nslices                     , INT_2  , 1   , 68,\
            rhr.csi_dims                       , INT_2  , 1   , 372,\
            rhr.rh_dab[0].start_rcv            , INT_2  , 1   , 200,\
            rhr.rh_dab[0].stop_rcv             , INT_2  , 1   , 202,\
            rhr.rh_dab[1].start_rcv            , INT_2  , 1   , 204,\
            rhr.rh_dab[1].stop_rcv             , INT_2  , 1   , 206,\
            rhr.rh_dab[2].start_rcv            , INT_2  , 1   , 208,\
            rhr.rh_dab[2].stop_rcv             , INT_2  , 1   , 210,\
            rhr.rh_dab[3].start_rcv            , INT_2  , 1   , 212,\
            rhr.rh_dab[3].stop_rcv             , INT_2  , 1   , 214,\
            rhr.rh_data_collect_type           , INT_2  , 1   , 56,\
            rhr.rh_file_contents               , INT_2  , 1   , 44,\
            rhr.rh_frame_size                  , UINT_2 , 1   , 80,\
            rhr.rh_point_size                  , INT_2  , 1   , 82,\
            rhr.rh_ps_mps_freq                 , LINT_4 , 1   , 424,\
            rhr.rh_user_usage_tag              , ULINT_4, 1   , 988,\
            rhr.roilenx                        , FLOAT_4, 1   , 380,\
            rhr.roileny                        , FLOAT_4, 1   , 384,\
            rhr.roilenz                        , FLOAT_4, 1   , 388,\
            rhr.spectral_width                 , FLOAT_4, 1   , 368,\
            rhr.xcsi                           , INT_2  , 1   , 374,\
            rhr.ycsi                           , INT_2  , 1   , 376,\
            rhr.zcsi                           , INT_2  , 1   , 378,\
            rhr.rh_logo                        , CHAR   , 10  , 34,\
            rhr.rh_raw_pass_size               , LINT_4 , 1   , 116,\
            rhr.rh_user0                       , FLOAT_4, 1   , 216,\
            rhr.rh_user1                       , FLOAT_4, 1   , 220,\
            rhr.rh_user2                       , FLOAT_4, 1   , 224,\
            rhr.rh_user3                       , FLOAT_4, 1   , 228,\
            rhr.rh_user4                       , FLOAT_4, 1   , 232,\
            rhr.rh_user5                       , FLOAT_4, 1   , 236,\
            rhr.rh_user6                       , FLOAT_4, 1   , 240,\
            rhr.rh_user7                       , FLOAT_4, 1   , 244,\
            rhr.rh_user8                       , FLOAT_4, 1   , 248,\
            rhr.rh_user9                       , FLOAT_4, 1   , 252,\
            rhr.rh_user10                      , FLOAT_4, 1   , 256,\
            rhr.rh_user11                      , FLOAT_4, 1   , 260,\
            rhr.rh_user12                      , FLOAT_4, 1   , 264,\
            rhr.rh_user13                      , FLOAT_4, 1   , 268,\
            rhr.rh_user14                      , FLOAT_4, 1   , 272,\
            rhr.rh_user15                      , FLOAT_4, 1   , 276,\
            rhr.rh_user16                      , FLOAT_4, 1   , 280,\
            rhr.rh_user17                      , FLOAT_4, 1   , 284,\
            rhr.rh_user18                      , FLOAT_4, 1   , 288,\
            rhr.rh_user19                      , FLOAT_4, 1   , 292,\
            rhr.rh_user20                      , FLOAT_4, 1   , 1000,\
            rhr.rh_user21                      , FLOAT_4, 1   , 1004,\
            rhr.rh_user22                      , FLOAT_4, 1   , 1008,\
            rhr.rh_user23                      , FLOAT_4, 1   , 1012,\
            rhr.rh_user24                      , FLOAT_4, 1   , 1016,\
            rhr.rh_user25                      , FLOAT_4, 1   , 1020,\
            rhr.rh_user26                      , FLOAT_4, 1   , 1024,\
            rhr.rh_user27                      , FLOAT_4, 1   , 1028,\
            rhr.rh_user28                      , FLOAT_4, 1   , 1032,\
            rhr.rh_user29                      , FLOAT_4, 1   , 1036,\
            rhr.rh_user30                      , FLOAT_4, 1   , 1040,\
            rhr.rh_user31                      , FLOAT_4, 1   , 1044,\
            rhr.rh_user32                      , FLOAT_4, 1   , 1048,\
            rhr.rh_user33                      , FLOAT_4, 1   , 1052,\
            rhr.rh_user34                      , FLOAT_4, 1   , 1056,\
            rhr.rh_user35                      , FLOAT_4, 1   , 1060,\
            rhr.rh_user36                      , FLOAT_4, 1   , 1064,\
            rhr.rh_user37                      , FLOAT_4, 1   , 1068,\
            rhr.rh_user38                      , FLOAT_4, 1   , 1072,\
            rhr.rh_user39                      , FLOAT_4, 1   , 1076,\
            rhr.rh_user40                      , FLOAT_4, 1   , 1080,\
            rhr.rh_user41                      , FLOAT_4, 1   , 1084,\
            rhr.rh_user42                      , FLOAT_4, 1   , 1088,\
            rhr.rh_user43                      , FLOAT_4, 1   , 1092,\
            rhr.rh_user44                      , FLOAT_4, 1   , 1096,\
            rhr.rh_user45                      , FLOAT_4, 1   , 1100,\
            rhr.rh_user46                      , FLOAT_4, 1   , 1104,\
            rhr.rh_user47                      , FLOAT_4, 1   , 1108,\
            rhr.rh_user48                      , FLOAT_4, 1   , 1112,\
            rhi.psdname                        , CHAR   , 33  , 60248,\
            rhi.scanspacing                    , FLOAT_4, 1   , 60048,\
            rhi.te                             , INT_4  , 1   , 60136,\
            rhi.ti                             , INT_4  , 1   , 60132,\
            rhi.tr                             , INT_4  , 1   , 60128,\
            rhi.tlhc_A                         , FLOAT_4, 1   , 60092,\
            rhi.tlhc_R                         , FLOAT_4, 1   , 60088,\
            rhi.tlhc_S                         , FLOAT_4, 1   , 60096,\
            rhi.t                              , INT_4  , 1   , 60128,\
            rhi.trhc_A                         , FLOAT_4, 1   , 60104,\
            rhi.trhc_R                         , FLOAT_4, 1   , 60100,\
            rhi.trhc_S                         , FLOAT_4, 1   , 60108,\
            rhi.user0                          , FLOAT_4, 1   , 60352,\
            rhi.user1                          , FLOAT_4, 1   , 60356,\
            rhi.user2                          , FLOAT_4, 1   , 60360,\
            rhi.user3                          , FLOAT_4, 1   , 60364,\
            rhi.user4                          , FLOAT_4, 1   , 60368,\
            rhi.user5                          , FLOAT_4, 1   , 60372,\
            rhi.user6                          , FLOAT_4, 1   , 60376,\
            rhi.user7                          , FLOAT_4, 1   , 60380,\
            rhi.user8                          , FLOAT_4, 1   , 60384,\
            rhi.user9                          , FLOAT_4, 1   , 60388,\
            rhi.user10                         , FLOAT_4, 1   , 60392,\
            rhi.user11                         , FLOAT_4, 1   , 60396,\
            rhi.user12                         , FLOAT_4, 1   , 60400,\
            rhi.user13                         , FLOAT_4, 1   , 60404,\
            rhi.user14                         , FLOAT_4, 1   , 60408,\
            rhi.user15                         , FLOAT_4, 1   , 60412,\
            rhi.user16                         , FLOAT_4, 1   , 60416,\
            rhi.user17                         , FLOAT_4, 1   , 60420,\
            rhi.user18                         , FLOAT_4, 1   , 60424,\
            rhi.user19                         , FLOAT_4, 1   , 60428,\
            rhi.user20                         , FLOAT_4, 1   , 60432,\
            rhi.user21                         , FLOAT_4, 1   , 60436,\
            rhi.user22                         , FLOAT_4, 1   , 60440,\
            rhi.user23                         , FLOAT_4, 1   , 60536,\
            rhi.user24                         , FLOAT_4, 1   , 60540,\
            rhi.user25                         , FLOAT_4, 1   , 60828,\
            rhi.user26                         , FLOAT_4, 1   , 60832,\
            rhi.user27                         , FLOAT_4, 1   , 60836,\
            rhi.user28                         , FLOAT_4, 1   , 60840,\
            rhi.user29                         , FLOAT_4, 1   , 60844,\
            rhi.user30                         , FLOAT_4, 1   , 60848,\
            rhi.user31                         , FLOAT_4, 1   , 60852,\
            rhi.user32                         , FLOAT_4, 1   , 60856,\
            rhi.user33                         , FLOAT_4, 1   , 60860,\
            rhi.user34                         , FLOAT_4, 1   , 60864,\
            rhi.user35                         , FLOAT_4, 1   , 60868,\
            rhi.user36                         , FLOAT_4, 1   , 60872,\
            rhi.user37                         , FLOAT_4, 1   , 60876,\
            rhi.user38                         , FLOAT_4, 1   , 60880,\
            rhi.user39                         , FLOAT_4, 1   , 60884,\
            rhi.user40                         , FLOAT_4, 1   , 60888,\
            rhi.user41                         , FLOAT_4, 1   , 60892,\
            rhi.user42                         , FLOAT_4, 1   , 60896,\
            rhi.user43                         , FLOAT_4, 1   , 60900,\
            rhi.user44                         , FLOAT_4, 1   , 60904,\
            rhi.user45                         , FLOAT_4, 1   , 60908,\
            rhi.user46                         , FLOAT_4, 1   , 60912,\
            rhi.user47                         , FLOAT_4, 1   , 60916,\
            rhi.user48                         , FLOAT_4, 1   , 60920,\
            rhi.cname                          , CHAR   , 17  , 60304,\
            rhi.brhc_A                         , FLOAT_4, 1   , 60116,\
            rhi.brhc_R                         , FLOAT_4, 1   , 60112,\
            rhi.brhc_S                         , FLOAT_4, 1   , 60120,\
            rhi.ctr_A                          , FLOAT_4, 1   , 60068,\
            rhi.ctr_R                          , FLOAT_4, 1   , 60064,\
            rhi.ctr_S                          , FLOAT_4, 1   , 60072,\
            rhi.dfov                           , FLOAT_4, 1   , 59964,\
            rhi.freq_dir                       , INT_2  , 1   , 60692,\
            rhi.ctyp                           , INT_2  , 1   , 60302,\
            rhi.loc                            , FLOAT_4, 1   , 60060,\
            rhi.mr_flip                        , INT_2  , 1   , 60188,\
            rhi.nex                            , FLOAT_4, 1   , 60152,\
            rhi.numecho                        , INT_2  , 1   , 60144,\
            rhi.image_uid                      , UID    , 32  , 60696,\
            rhi.rawrunnum                      , INT_4  , 1   , 60328,\
            rhe.ex_datetime                    , INT_4  , 1   , 57564,\
            rhe.ex_no                          , UINT_2 , 1   , 57360,\
            rhe.magstrength                    , INT_4  , 1   , 57436,\
            rhe.patid                          , CHAR   , 13  , 57440,\
            rhe.patidff                        , CHAR   , 65  , 58001,\
            rhe.patname                        , CHAR   , 25  , 57453,\
            rhe.patnameff                      , CHAR   , 65  , 57936,\
            rhe.refphy                         , CHAR   , 33  , 57568,\
            rhe.reqnum                         , CHAR   , 13  , 57551,\
            rhe.reqnumff                       , CHAR   , 17  , 58066,\
            rhe.study_uid                      , UID    , 32  , 57838,\
            rhe.dateofbirth                    , CHAR   , 9   , 58083,\
            rhe.patsex                         , INT_2  , 1   , 57482,\
            rhe.hospname                       , CHAR   , 33  , 57362,\
            rhe.ex_sysid                       , CHAR   , 9   , 57680,\
            rhe.uniq_sys_id                    , CHAR   , 16  , 57802,\
            rhe.ex_verscre                     , CHAR   , 2   , 57716,\
            rhs.se_no                          , INT_2  , 1   , 58402,\
            rhs.se_desc                        , CHAR   , 30  , 58412,\
            rhs.entry                          , INT_4  , 1   , 58472,\
            rhs.position                       , INT_4  , 1   , 58468,\
            rhs.series_uid                     , UID    , 32  , 58736,\
            rhs.landmark_uid                   , UID    , 32  , 58768,\
            rhs.anref                          , CHAR   , 3   , 58476,\
        "); 

    } else if ( (int)(this->pfileVersion) == 12 ) {

        offsets.assign (" \
            rhr.rh_rdbm_rev                    , FLOAT_4, 1   , 0,\
            rhr.rh_scan_date                   , CHAR   , 10  , 16,\
            rhr.rh_scan_time                   , CHAR   , 8   , 26,\
            rhr.rh_npasses                     , INT_2  , 1   , 64,\
            rhr.rh_nslices                     , INT_2  , 1   , 68,\
            rhr.csi_dims                       , INT_2  , 1   , 372,\
            rhr.rh_dab[0].start_rcv            , INT_2  , 1   , 200,\
            rhr.rh_dab[0].stop_rcv             , INT_2  , 1   , 202,\
            rhr.rh_dab[1].start_rcv            , INT_2  , 1   , 204,\
            rhr.rh_dab[1].stop_rcv             , INT_2  , 1   , 206,\
            rhr.rh_dab[2].start_rcv            , INT_2  , 1   , 208,\
            rhr.rh_dab[2].stop_rcv             , INT_2  , 1   , 210,\
            rhr.rh_dab[3].start_rcv            , INT_2  , 1   , 212,\
            rhr.rh_dab[3].stop_rcv             , INT_2  , 1   , 214,\
            rhr.rh_data_collect_type           , INT_2  , 1   , 56,\
            rhr.rh_file_contents               , INT_2  , 1   , 44,\
            rhr.rdb_hdr_off_data               , INT_4  , 1   , 1468,\
            rhr.rh_frame_size                  , UINT_2 , 1   , 80,\
            rhr.rh_point_size                  , INT_2  , 1   , 82,\
            rhr.rh_ps_mps_freq                 , LINT_4 , 1   , 424,\
            rhr.rh_user_usage_tag              , ULINT_4, 1   , 988,\
            rhr.roilenx                        , FLOAT_4, 1   , 380,\
            rhr.roileny                        , FLOAT_4, 1   , 384,\
            rhr.roilenz                        , FLOAT_4, 1   , 388,\
            rhr.spectral_width                 , FLOAT_4, 1   , 368,\
            rhr.xcsi                           , INT_2  , 1   , 374,\
            rhr.ycsi                           , INT_2  , 1   , 376,\
            rhr.zcsi                           , INT_2  , 1   , 378,\
            rhr.rh_logo                        , CHAR   , 10  , 34,\
            rhr.rh_raw_pass_size               , LINT_4 , 1   , 116,\
            rhr.rh_user0                       , FLOAT_4, 1   , 216,\
            rhr.rh_user1                       , FLOAT_4, 1   , 220,\
            rhr.rh_user2                       , FLOAT_4, 1   , 224,\
            rhr.rh_user3                       , FLOAT_4, 1   , 228,\
            rhr.rh_user4                       , FLOAT_4, 1   , 232,\
            rhr.rh_user5                       , FLOAT_4, 1   , 236,\
            rhr.rh_user6                       , FLOAT_4, 1   , 240,\
            rhr.rh_user7                       , FLOAT_4, 1   , 244,\
            rhr.rh_user8                       , FLOAT_4, 1   , 248,\
            rhr.rh_user9                       , FLOAT_4, 1   , 252,\
            rhr.rh_user10                      , FLOAT_4, 1   , 256,\
            rhr.rh_user11                      , FLOAT_4, 1   , 260,\
            rhr.rh_user12                      , FLOAT_4, 1   , 264,\
            rhr.rh_user13                      , FLOAT_4, 1   , 268,\
            rhr.rh_user14                      , FLOAT_4, 1   , 272,\
            rhr.rh_user15                      , FLOAT_4, 1   , 276,\
            rhr.rh_user16                      , FLOAT_4, 1   , 280,\
            rhr.rh_user17                      , FLOAT_4, 1   , 284,\
            rhr.rh_user18                      , FLOAT_4, 1   , 288,\
            rhr.rh_user19                      , FLOAT_4, 1   , 292,\
            rhr.rh_user20                      , FLOAT_4, 1   , 1000,\
            rhr.rh_user21                      , FLOAT_4, 1   , 1004,\
            rhr.rh_user22                      , FLOAT_4, 1   , 1008,\
            rhr.rh_user23                      , FLOAT_4, 1   , 1012,\
            rhr.rh_user24                      , FLOAT_4, 1   , 1016,\
            rhr.rh_user25                      , FLOAT_4, 1   , 1020,\
            rhr.rh_user26                      , FLOAT_4, 1   , 1024,\
            rhr.rh_user27                      , FLOAT_4, 1   , 1028,\
            rhr.rh_user28                      , FLOAT_4, 1   , 1032,\
            rhr.rh_user29                      , FLOAT_4, 1   , 1036,\
            rhr.rh_user30                      , FLOAT_4, 1   , 1040,\
            rhr.rh_user31                      , FLOAT_4, 1   , 1044,\
            rhr.rh_user32                      , FLOAT_4, 1   , 1048,\
            rhr.rh_user33                      , FLOAT_4, 1   , 1052,\
            rhr.rh_user34                      , FLOAT_4, 1   , 1056,\
            rhr.rh_user35                      , FLOAT_4, 1   , 1060,\
            rhr.rh_user36                      , FLOAT_4, 1   , 1064,\
            rhr.rh_user37                      , FLOAT_4, 1   , 1068,\
            rhr.rh_user38                      , FLOAT_4, 1   , 1072,\
            rhr.rh_user39                      , FLOAT_4, 1   , 1076,\
            rhr.rh_user40                      , FLOAT_4, 1   , 1080,\
            rhr.rh_user41                      , FLOAT_4, 1   , 1084,\
            rhr.rh_user42                      , FLOAT_4, 1   , 1088,\
            rhr.rh_user43                      , FLOAT_4, 1   , 1092,\
            rhr.rh_user44                      , FLOAT_4, 1   , 1096,\
            rhr.rh_user45                      , FLOAT_4, 1   , 1100,\
            rhr.rh_user46                      , FLOAT_4, 1   , 1104,\
            rhr.rh_user47                      , FLOAT_4, 1   , 1108,\
            rhr.rh_user48                      , FLOAT_4, 1   , 1112,\
            rhi.psdname                        , CHAR   , 33  , 65374,\
            rhi.scanspacing                    , FLOAT_4, 1   , 64552,\
            rhi.te                             , INT_4  , 1   , 65032,\
            rhi.ti                             , INT_4  , 1   , 65028,\
            rhi.tr                             , INT_4  , 1   , 65024,\
            rhi.tlhc_A                         , FLOAT_4, 1   , 64956,\
            rhi.tlhc_R                         , FLOAT_4, 1   , 64952,\
            rhi.tlhc_S                         , FLOAT_4, 1   , 64960,\
            rhi.t                              , INT_4  , 1   , 65024,\
            rhi.trhc_A                         , FLOAT_4, 1   , 64968,\
            rhi.trhc_R                         , FLOAT_4, 1   , 64964,\
            rhi.trhc_S                         , FLOAT_4, 1   , 64972,\
            rhi.user0                          , FLOAT_4, 1   , 64588,\
            rhi.user1                          , FLOAT_4, 1   , 64592,\
            rhi.user2                          , FLOAT_4, 1   , 64596,\
            rhi.user3                          , FLOAT_4, 1   , 64600,\
            rhi.user4                          , FLOAT_4, 1   , 64604,\
            rhi.user5                          , FLOAT_4, 1   , 64608,\
            rhi.user6                          , FLOAT_4, 1   , 64612,\
            rhi.user7                          , FLOAT_4, 1   , 64616,\
            rhi.user8                          , FLOAT_4, 1   , 64620,\
            rhi.user9                          , FLOAT_4, 1   , 64624,\
            rhi.user10                         , FLOAT_4, 1   , 64628,\
            rhi.user11                         , FLOAT_4, 1   , 64632,\
            rhi.user12                         , FLOAT_4, 1   , 64636,\
            rhi.user13                         , FLOAT_4, 1   , 64640,\
            rhi.user14                         , FLOAT_4, 1   , 64644,\
            rhi.user15                         , FLOAT_4, 1   , 64648,\
            rhi.user16                         , FLOAT_4, 1   , 64652,\
            rhi.user17                         , FLOAT_4, 1   , 64656,\
            rhi.user18                         , FLOAT_4, 1   , 64660,\
            rhi.user19                         , FLOAT_4, 1   , 64664,\
            rhi.user20                         , FLOAT_4, 1   , 64668,\
            rhi.user21                         , FLOAT_4, 1   , 64672,\
            rhi.user22                         , FLOAT_4, 1   , 64676,\
            rhi.user23                         , FLOAT_4, 1   , 64688,\
            rhi.user24                         , FLOAT_4, 1   , 64692,\
            rhi.user25                         , FLOAT_4, 1   , 64756,\
            rhi.user26                         , FLOAT_4, 1   , 64760,\
            rhi.user27                         , FLOAT_4, 1   , 64764,\
            rhi.user28                         , FLOAT_4, 1   , 64768,\
            rhi.user29                         , FLOAT_4, 1   , 64772,\
            rhi.user30                         , FLOAT_4, 1   , 64776,\
            rhi.user31                         , FLOAT_4, 1   , 64780,\
            rhi.user32                         , FLOAT_4, 1   , 64784,\
            rhi.user33                         , FLOAT_4, 1   , 64788,\
            rhi.user34                         , FLOAT_4, 1   , 64792,\
            rhi.user35                         , FLOAT_4, 1   , 64796,\
            rhi.user36                         , FLOAT_4, 1   , 64800,\
            rhi.user37                         , FLOAT_4, 1   , 64804,\
            rhi.user38                         , FLOAT_4, 1   , 64808,\
            rhi.user39                         , FLOAT_4, 1   , 64812,\
            rhi.user40                         , FLOAT_4, 1   , 64816,\
            rhi.user41                         , FLOAT_4, 1   , 64820,\
            rhi.user42                         , FLOAT_4, 1   , 64824,\
            rhi.user43                         , FLOAT_4, 1   , 64828,\
            rhi.user44                         , FLOAT_4, 1   , 64832,\
            rhi.user45                         , FLOAT_4, 1   , 64836,\
            rhi.user46                         , FLOAT_4, 1   , 64840,\
            rhi.user47                         , FLOAT_4, 1   , 64844,\
            rhi.user48                         , FLOAT_4, 1   , 64848,\
            rhi.cname                          , CHAR   , 17  , 65491,\
            rhi.brhc_A                         , FLOAT_4, 1   , 64980,\
            rhi.brhc_R                         , FLOAT_4, 1   , 64976,\
            rhi.brhc_S                         , FLOAT_4, 1   , 64984,\
            rhi.ctr_A                          , FLOAT_4, 1   , 64932,\
            rhi.ctr_R                          , FLOAT_4, 1   , 64928,\
            rhi.ctr_S                          , FLOAT_4, 1   , 64936,\
            rhi.dfov                           , FLOAT_4, 1   , 64536,\
            rhi.freq_dir                       , INT_2  , 1   , 65334,\
            rhi.ctyp                           , INT_2  , 1   , 65268,\
            rhi.loc                            , FLOAT_4, 1   , 64556,\
            rhi.mr_flip                        , INT_2  , 1   , 65244,\
            rhi.nex                            , FLOAT_4, 1   , 64564,\
            rhi.numecho                        , INT_2  , 1   , 65206,\
            rhi.image_uid                      , UID    , 32  , 65559,\
            rhi.rawrunnum                      , INT_4  , 1   , 65076,\
            rhe.ex_datetime                    , INT_4  , 1   , 61568,\
            rhe.ex_no                          , UINT_2 , 1   , 61576,\
            rhe.magstrength                    , INT_4  , 1   , 61560,\
            rhe.patid                          , CHAR   , 13  , 61884,\
            rhe.patidff                        , CHAR   , 65  , 62127,\
            rhe.patname                        , CHAR   , 25  , 61897,\
            rhe.patnameff                      , CHAR   , 65  , 62062,\
            rhe.refphy                         , CHAR   , 33  , 61690,\
            rhe.reqnum                         , CHAR   , 13  , 61677,\
            rhe.reqnumff                       , CHAR   , 17  , 62192,\
            rhe.study_uid                      , UID    , 32  , 61966,\
            rhe.dateofbirth                    , CHAR   , 9   , 62209,\
            rhe.patsex                         , INT_2  , 1   , 61600,\
            rhe.hospname                       , CHAR   , 33  , 61851,\
            rhe.ex_sysid                       , CHAR   , 9   , 61828,\
            rhe.uniq_sys_id                    , CHAR   , 16  , 61930,\
            rhe.ex_verscre                     , CHAR   , 2   , 61926,\
            rhs.se_no                          , INT_2  , 1   , 62710,\
            rhs.se_desc                        , CHAR   , 65  , 62786,\
            rhs.entry                          , INT_4  , 1   , 62644,\
            rhs.position                       , INT_4  , 1   , 62640,\
            rhs.series_uid                     , UID    , 32  , 62899,\
            rhs.landmark_uid                   , UID    , 32  , 62931,\
            rhs.anref                          , CHAR   , 3   , 62869,\
        "); 

    } else if ( (int)(this->pfileVersion) == 14 ) {

        offsets.assign (" \
            rhr.rh_rdbm_rev                    , FLOAT_4, 1   , 0,\
            rhr.rh_scan_date                   , CHAR   , 10  , 16,\
            rhr.rh_scan_time                   , CHAR   , 8   , 26,\
            rhr.rh_npasses                     , INT_2  , 1   , 64,\
            rhr.rh_nslices                     , INT_2  , 1   , 68,\
            rhr.csi_dims                       , INT_2  , 1   , 372,\
            rhr.rh_dab[0].start_rcv            , INT_2  , 1   , 200,\
            rhr.rh_dab[0].stop_rcv             , INT_2  , 1   , 202,\
            rhr.rh_dab[1].start_rcv            , INT_2  , 1   , 204,\
            rhr.rh_dab[1].stop_rcv             , INT_2  , 1   , 206,\
            rhr.rh_dab[2].start_rcv            , INT_2  , 1   , 208,\
            rhr.rh_dab[2].stop_rcv             , INT_2  , 1   , 210,\
            rhr.rh_dab[3].start_rcv            , INT_2  , 1   , 212,\
            rhr.rh_dab[3].stop_rcv             , INT_2  , 1   , 214,\
            rhr.rh_data_collect_type           , INT_2  , 1   , 56,\
            rhr.rh_file_contents               , INT_2  , 1   , 44,\
            rhr.rdb_hdr_off_data               , INT_4  , 1   , 1468,\
            rhr.rh_frame_size                  , UINT_2 , 1   , 80,\
            rhr.rh_point_size                  , INT_2  , 1   , 82,\
            rhr.rh_ps_mps_freq                 , LINT_4 , 1   , 424,\
            rhr.rh_user_usage_tag              , ULINT_4, 1   , 988,\
            rhr.roilenx                        , FLOAT_4, 1   , 380,\
            rhr.roileny                        , FLOAT_4, 1   , 384,\
            rhr.roilenz                        , FLOAT_4, 1   , 388,\
            rhr.spectral_width                 , FLOAT_4, 1   , 368,\
            rhr.xcsi                           , INT_2  , 1   , 374,\
            rhr.ycsi                           , INT_2  , 1   , 376,\
            rhr.zcsi                           , INT_2  , 1   , 378,\
            rhr.rh_logo                        , CHAR   , 10  , 34,\
            rhr.rh_raw_pass_size               , ULINT_4, 1   , 116,\
            rhr.rh_user0                       , FLOAT_4, 1   , 216,\
            rhr.rh_user1                       , FLOAT_4, 1   , 220,\
            rhr.rh_user2                       , FLOAT_4, 1   , 224,\
            rhr.rh_user3                       , FLOAT_4, 1   , 228,\
            rhr.rh_user4                       , FLOAT_4, 1   , 232,\
            rhr.rh_user5                       , FLOAT_4, 1   , 236,\
            rhr.rh_user6                       , FLOAT_4, 1   , 240,\
            rhr.rh_user7                       , FLOAT_4, 1   , 244,\
            rhr.rh_user8                       , FLOAT_4, 1   , 248,\
            rhr.rh_user9                       , FLOAT_4, 1   , 252,\
            rhr.rh_user10                      , FLOAT_4, 1   , 256,\
            rhr.rh_user11                      , FLOAT_4, 1   , 260,\
            rhr.rh_user12                      , FLOAT_4, 1   , 264,\
            rhr.rh_user13                      , FLOAT_4, 1   , 268,\
            rhr.rh_user14                      , FLOAT_4, 1   , 272,\
            rhr.rh_user15                      , FLOAT_4, 1   , 276,\
            rhr.rh_user16                      , FLOAT_4, 1   , 280,\
            rhr.rh_user17                      , FLOAT_4, 1   , 284,\
            rhr.rh_user18                      , FLOAT_4, 1   , 288,\
            rhr.rh_user19                      , FLOAT_4, 1   , 292,\
            rhr.rh_user20                      , FLOAT_4, 1   , 1000,\
            rhr.rh_user21                      , FLOAT_4, 1   , 1004,\
            rhr.rh_user22                      , FLOAT_4, 1   , 1008,\
            rhr.rh_user23                      , FLOAT_4, 1   , 1012,\
            rhr.rh_user24                      , FLOAT_4, 1   , 1016,\
            rhr.rh_user25                      , FLOAT_4, 1   , 1020,\
            rhr.rh_user26                      , FLOAT_4, 1   , 1024,\
            rhr.rh_user27                      , FLOAT_4, 1   , 1028,\
            rhr.rh_user28                      , FLOAT_4, 1   , 1032,\
            rhr.rh_user29                      , FLOAT_4, 1   , 1036,\
            rhr.rh_user30                      , FLOAT_4, 1   , 1040,\
            rhr.rh_user31                      , FLOAT_4, 1   , 1044,\
            rhr.rh_user32                      , FLOAT_4, 1   , 1048,\
            rhr.rh_user33                      , FLOAT_4, 1   , 1052,\
            rhr.rh_user34                      , FLOAT_4, 1   , 1056,\
            rhr.rh_user35                      , FLOAT_4, 1   , 1060,\
            rhr.rh_user36                      , FLOAT_4, 1   , 1064,\
            rhr.rh_user37                      , FLOAT_4, 1   , 1068,\
            rhr.rh_user38                      , FLOAT_4, 1   , 1072,\
            rhr.rh_user39                      , FLOAT_4, 1   , 1076,\
            rhr.rh_user40                      , FLOAT_4, 1   , 1080,\
            rhr.rh_user41                      , FLOAT_4, 1   , 1084,\
            rhr.rh_user42                      , FLOAT_4, 1   , 1088,\
            rhr.rh_user43                      , FLOAT_4, 1   , 1092,\
            rhr.rh_user44                      , FLOAT_4, 1   , 1096,\
            rhr.rh_user45                      , FLOAT_4, 1   , 1100,\
            rhr.rh_user46                      , FLOAT_4, 1   , 1104,\
            rhr.rh_user47                      , FLOAT_4, 1   , 1108,\
            rhr.rh_user48                      , FLOAT_4, 1   , 1112,\
            rhi.psdname                        , CHAR   , 33  , 145132,\
            rhi.scanspacing                    , FLOAT_4, 1   , 143940,\
            rhi.te                             , INT_4  , 1   , 144580,\
            rhi.ti                             , INT_4  , 1   , 144576,\
            rhi.tr                             , INT_4  , 1   , 144572,\
            rhi.tlhc_A                         , FLOAT_4, 1   , 144344,\
            rhi.tlhc_R                         , FLOAT_4, 1   , 144340,\
            rhi.tlhc_S                         , FLOAT_4, 1   , 144348,\
            rhi.t                              , INT_4  , 1   , 144572,\
            rhi.trhc_A                         , FLOAT_4, 1   , 144356,\
            rhi.trhc_R                         , FLOAT_4, 1   , 144352,\
            rhi.trhc_S                         , FLOAT_4, 1   , 144360,\
            rhi.user0                          , FLOAT_4, 1   , 143976,\
            rhi.user1                          , FLOAT_4, 1   , 143980,\
            rhi.user2                          , FLOAT_4, 1   , 143984,\
            rhi.user3                          , FLOAT_4, 1   , 143988,\
            rhi.user4                          , FLOAT_4, 1   , 143992,\
            rhi.user5                          , FLOAT_4, 1   , 143996,\
            rhi.user6                          , FLOAT_4, 1   , 144000,\
            rhi.user7                          , FLOAT_4, 1   , 144004,\
            rhi.user8                          , FLOAT_4, 1   , 144008,\
            rhi.user9                          , FLOAT_4, 1   , 144012,\
            rhi.user10                         , FLOAT_4, 1   , 144016,\
            rhi.user11                         , FLOAT_4, 1   , 144020,\
            rhi.user12                         , FLOAT_4, 1   , 144024,\
            rhi.user13                         , FLOAT_4, 1   , 144028,\
            rhi.user14                         , FLOAT_4, 1   , 144032,\
            rhi.user15                         , FLOAT_4, 1   , 144036,\
            rhi.user16                         , FLOAT_4, 1   , 144040,\
            rhi.user17                         , FLOAT_4, 1   , 144044,\
            rhi.user18                         , FLOAT_4, 1   , 144048,\
            rhi.user19                         , FLOAT_4, 1   , 144052,\
            rhi.user20                         , FLOAT_4, 1   , 144056,\
            rhi.user21                         , FLOAT_4, 1   , 144060,\
            rhi.user22                         , FLOAT_4, 1   , 144064,\
            rhi.user23                         , FLOAT_4, 1   , 144076,\
            rhi.user24                         , FLOAT_4, 1   , 144080,\
            rhi.user25                         , FLOAT_4, 1   , 144144,\
            rhi.user26                         , FLOAT_4, 1   , 144148,\
            rhi.user27                         , FLOAT_4, 1   , 144152,\
            rhi.user28                         , FLOAT_4, 1   , 144156,\
            rhi.user29                         , FLOAT_4, 1   , 144160,\
            rhi.user30                         , FLOAT_4, 1   , 144164,\
            rhi.user31                         , FLOAT_4, 1   , 144168,\
            rhi.user32                         , FLOAT_4, 1   , 144172,\
            rhi.user33                         , FLOAT_4, 1   , 144176,\
            rhi.user34                         , FLOAT_4, 1   , 144180,\
            rhi.user35                         , FLOAT_4, 1   , 144184,\
            rhi.user36                         , FLOAT_4, 1   , 144188,\
            rhi.user37                         , FLOAT_4, 1   , 144192,\
            rhi.user38                         , FLOAT_4, 1   , 144196,\
            rhi.user39                         , FLOAT_4, 1   , 144200,\
            rhi.user40                         , FLOAT_4, 1   , 144204,\
            rhi.user41                         , FLOAT_4, 1   , 144208,\
            rhi.user42                         , FLOAT_4, 1   , 144212,\
            rhi.user43                         , FLOAT_4, 1   , 144216,\
            rhi.user44                         , FLOAT_4, 1   , 144220,\
            rhi.user45                         , FLOAT_4, 1   , 144224,\
            rhi.user46                         , FLOAT_4, 1   , 144228,\
            rhi.user47                         , FLOAT_4, 1   , 144232,\
            rhi.user48                         , FLOAT_4, 1   , 144236,\
            rhi.cname                          , CHAR   , 17  , 145249,\
            rhi.brhc_A                         , FLOAT_4, 1   , 144368,\
            rhi.brhc_R                         , FLOAT_4, 1   , 144364,\
            rhi.brhc_S                         , FLOAT_4, 1   , 144372,\
            rhi.ctr_A                          , FLOAT_4, 1   , 144320,\
            rhi.ctr_R                          , FLOAT_4, 1   , 144316,\
            rhi.ctr_S                          , FLOAT_4, 1   , 144324,\
            rhi.dfov                           , FLOAT_4, 1   , 143924,\
            rhi.freq_dir                       , INT_2  , 1   , 145018,\
            rhi.ctyp                           , INT_2  , 1   , 144952,\
            rhi.loc                            , FLOAT_4, 1   , 143944,\
            rhi.mr_flip                        , INT_2  , 1   , 144928,\
            rhi.nex                            , FLOAT_4, 1   , 143952,\
            rhi.numecho                        , INT_2  , 1   , 144890,\
            rhi.image_uid                      , UID    , 32  , 145317,\
            rhi.rawrunnum                      , INT_4  , 1   , 144624,\
            rhe.ex_datetime                    , INT_4  , 1   , 140988,\
            rhe.ex_no                          , UINT_2 , 1   , 141044,\
            rhe.magstrength                    , INT_4  , 1   , 140980,\
            rhe.patid                          , CHAR   , 13  , 141368,\
            rhe.patidff                        , CHAR   , 65  , 141611,\
            rhe.patname                        , CHAR   , 25  , 141381,\
            rhe.patnameff                      , CHAR   , 65  , 141546,\
            rhe.refphy                         , CHAR   , 33  , 141174,\
            rhe.reqnum                         , CHAR   , 13  , 141161,\
            rhe.reqnumff                       , CHAR   , 17  , 141676,\
            rhe.study_uid                      , UID    , 32  , 141450,\
            rhe.dateofbirth                    , CHAR   , 9   , 141693,\
            rhe.patsex                         , INT_2  , 1   , 141068,\
            rhe.hospname                       , CHAR   , 33  , 141335,\
            rhe.ex_sysid                       , CHAR   , 9   , 141312,\
            rhe.uniq_sys_id                    , CHAR   , 16  , 141414,\
            rhe.ex_verscre                     , CHAR   , 2   , 141410,\
            rhs.se_no                          , INT_2  , 1   , 142194,\
            rhs.se_desc                        , CHAR   , 65  , 142318,\
            rhs.entry                          , INT_4  , 1   , 142064,\
            rhs.position                       , INT_4  , 1   , 142060,\
            rhs.series_uid                     , UID    , 32  , 142431,\
            rhs.landmark_uid                   , UID    , 32  , 142463,\
            rhs.anref                          , CHAR   , 3   , 142401,\
        "); 

    } else if ( (int)(this->pfileVersion) == 15 ) {

        offsets.assign (" \
            rhr.rh_rdbm_rev                    , FLOAT_4, 1   , 0,\
            rhr.rh_scan_date                   , CHAR   , 10  , 16,\
            rhr.rh_scan_time                   , CHAR   , 8   , 26,\
            rhr.rh_npasses                     , INT_2  , 1   , 64,\
            rhr.rh_nslices                     , UINT_2 , 1   , 68,\
            rhr.csi_dims                       , INT_2  , 1   , 372,\
            rhr.rh_dab[0].start_rcv            , INT_2  , 1   , 200,\
            rhr.rh_dab[0].stop_rcv             , INT_2  , 1   , 202,\
            rhr.rh_dab[1].start_rcv            , INT_2  , 1   , 204,\
            rhr.rh_dab[1].stop_rcv             , INT_2  , 1   , 206,\
            rhr.rh_dab[2].start_rcv            , INT_2  , 1   , 208,\
            rhr.rh_dab[2].stop_rcv             , INT_2  , 1   , 210,\
            rhr.rh_dab[3].start_rcv            , INT_2  , 1   , 212,\
            rhr.rh_dab[3].stop_rcv             , INT_2  , 1   , 214,\
            rhr.rh_data_collect_type           , INT_2  , 1   , 56,\
            rhr.rh_file_contents               , INT_2  , 1   , 44,\
            rhr.rdb_hdr_off_data               , INT_4  , 1   , 1468,\
            rhr.rh_frame_size                  , UINT_2 , 1   , 80,\
            rhr.rh_point_size                  , INT_2  , 1   , 82,\
            rhr.rh_ps_mps_freq                 , LINT_4 , 1   , 424,\
            rhr.rh_user_usage_tag              , ULINT_4, 1   , 988,\
            rhr.roilenx                        , FLOAT_4, 1   , 380,\
            rhr.roileny                        , FLOAT_4, 1   , 384,\
            rhr.roilenz                        , FLOAT_4, 1   , 388,\
            rhr.spectral_width                 , FLOAT_4, 1   , 368,\
            rhr.xcsi                           , INT_2  , 1   , 374,\
            rhr.ycsi                           , INT_2  , 1   , 376,\
            rhr.zcsi                           , INT_2  , 1   , 378,\
            rhr.rh_logo                        , CHAR   , 10  , 34,\
            rhr.rh_raw_pass_size               , ULINT_4, 1   , 116,\
            rhr.rh_user0                       , FLOAT_4, 1   , 216,\
            rhr.rh_user1                       , FLOAT_4, 1   , 220,\
            rhr.rh_user2                       , FLOAT_4, 1   , 224,\
            rhr.rh_user3                       , FLOAT_4, 1   , 228,\
            rhr.rh_user4                       , FLOAT_4, 1   , 232,\
            rhr.rh_user5                       , FLOAT_4, 1   , 236,\
            rhr.rh_user6                       , FLOAT_4, 1   , 240,\
            rhr.rh_user7                       , FLOAT_4, 1   , 244,\
            rhr.rh_user8                       , FLOAT_4, 1   , 248,\
            rhr.rh_user9                       , FLOAT_4, 1   , 252,\
            rhr.rh_user10                      , FLOAT_4, 1   , 256,\
            rhr.rh_user11                      , FLOAT_4, 1   , 260,\
            rhr.rh_user12                      , FLOAT_4, 1   , 264,\
            rhr.rh_user13                      , FLOAT_4, 1   , 268,\
            rhr.rh_user14                      , FLOAT_4, 1   , 272,\
            rhr.rh_user15                      , FLOAT_4, 1   , 276,\
            rhr.rh_user16                      , FLOAT_4, 1   , 280,\
            rhr.rh_user17                      , FLOAT_4, 1   , 284,\
            rhr.rh_user18                      , FLOAT_4, 1   , 288,\
            rhr.rh_user19                      , FLOAT_4, 1   , 292,\
            rhr.rh_user20                      , FLOAT_4, 1   , 1000,\
            rhr.rh_user21                      , FLOAT_4, 1   , 1004,\
            rhr.rh_user22                      , FLOAT_4, 1   , 1008,\
            rhr.rh_user23                      , FLOAT_4, 1   , 1012,\
            rhr.rh_user24                      , FLOAT_4, 1   , 1016,\
            rhr.rh_user25                      , FLOAT_4, 1   , 1020,\
            rhr.rh_user26                      , FLOAT_4, 1   , 1024,\
            rhr.rh_user27                      , FLOAT_4, 1   , 1028,\
            rhr.rh_user28                      , FLOAT_4, 1   , 1032,\
            rhr.rh_user29                      , FLOAT_4, 1   , 1036,\
            rhr.rh_user30                      , FLOAT_4, 1   , 1040,\
            rhr.rh_user31                      , FLOAT_4, 1   , 1044,\
            rhr.rh_user32                      , FLOAT_4, 1   , 1048,\
            rhr.rh_user33                      , FLOAT_4, 1   , 1052,\
            rhr.rh_user34                      , FLOAT_4, 1   , 1056,\
            rhr.rh_user35                      , FLOAT_4, 1   , 1060,\
            rhr.rh_user36                      , FLOAT_4, 1   , 1064,\
            rhr.rh_user37                      , FLOAT_4, 1   , 1068,\
            rhr.rh_user38                      , FLOAT_4, 1   , 1072,\
            rhr.rh_user39                      , FLOAT_4, 1   , 1076,\
            rhr.rh_user40                      , FLOAT_4, 1   , 1080,\
            rhr.rh_user41                      , FLOAT_4, 1   , 1084,\
            rhr.rh_user42                      , FLOAT_4, 1   , 1088,\
            rhr.rh_user43                      , FLOAT_4, 1   , 1092,\
            rhr.rh_user44                      , FLOAT_4, 1   , 1096,\
            rhr.rh_user45                      , FLOAT_4, 1   , 1100,\
            rhr.rh_user46                      , FLOAT_4, 1   , 1104,\
            rhr.rh_user47                      , FLOAT_4, 1   , 1108,\
            rhr.rh_user48                      , FLOAT_4, 1   , 1112,\
            rhi.psdname                        , CHAR   , 33  , 145132,\
            rhi.scanspacing                    , FLOAT_4, 1   , 143940,\
            rhi.te                             , INT_4  , 1   , 144580,\
            rhi.ti                             , INT_4  , 1   , 144576,\
            rhi.tr                             , INT_4  , 1   , 144572,\
            rhi.tlhc_A                         , FLOAT_4, 1   , 144344,\
            rhi.tlhc_R                         , FLOAT_4, 1   , 144340,\
            rhi.tlhc_S                         , FLOAT_4, 1   , 144348,\
            rhi.t                              , INT_4  , 1   , 144572,\
            rhi.trhc_A                         , FLOAT_4, 1   , 144356,\
            rhi.trhc_R                         , FLOAT_4, 1   , 144352,\
            rhi.trhc_S                         , FLOAT_4, 1   , 144360,\
            rhi.user0                          , FLOAT_4, 1   , 143976,\
            rhi.user1                          , FLOAT_4, 1   , 143980,\
            rhi.user2                          , FLOAT_4, 1   , 143984,\
            rhi.user3                          , FLOAT_4, 1   , 143988,\
            rhi.user4                          , FLOAT_4, 1   , 143992,\
            rhi.user5                          , FLOAT_4, 1   , 143996,\
            rhi.user6                          , FLOAT_4, 1   , 144000,\
            rhi.user7                          , FLOAT_4, 1   , 144004,\
            rhi.user8                          , FLOAT_4, 1   , 144008,\
            rhi.user9                          , FLOAT_4, 1   , 144012,\
            rhi.user10                         , FLOAT_4, 1   , 144016,\
            rhi.user11                         , FLOAT_4, 1   , 144020,\
            rhi.user12                         , FLOAT_4, 1   , 144024,\
            rhi.user13                         , FLOAT_4, 1   , 144028,\
            rhi.user14                         , FLOAT_4, 1   , 144032,\
            rhi.user15                         , FLOAT_4, 1   , 144036,\
            rhi.user16                         , FLOAT_4, 1   , 144040,\
            rhi.user17                         , FLOAT_4, 1   , 144044,\
            rhi.user18                         , FLOAT_4, 1   , 144048,\
            rhi.user19                         , FLOAT_4, 1   , 144052,\
            rhi.user20                         , FLOAT_4, 1   , 144056,\
            rhi.user21                         , FLOAT_4, 1   , 144060,\
            rhi.user22                         , FLOAT_4, 1   , 144064,\
            rhi.user23                         , FLOAT_4, 1   , 144076,\
            rhi.user24                         , FLOAT_4, 1   , 144080,\
            rhi.user25                         , FLOAT_4, 1   , 144144,\
            rhi.user26                         , FLOAT_4, 1   , 144148,\
            rhi.user27                         , FLOAT_4, 1   , 144152,\
            rhi.user28                         , FLOAT_4, 1   , 144156,\
            rhi.user29                         , FLOAT_4, 1   , 144160,\
            rhi.user30                         , FLOAT_4, 1   , 144164,\
            rhi.user31                         , FLOAT_4, 1   , 144168,\
            rhi.user32                         , FLOAT_4, 1   , 144172,\
            rhi.user33                         , FLOAT_4, 1   , 144176,\
            rhi.user34                         , FLOAT_4, 1   , 144180,\
            rhi.user35                         , FLOAT_4, 1   , 144184,\
            rhi.user36                         , FLOAT_4, 1   , 144188,\
            rhi.user37                         , FLOAT_4, 1   , 144192,\
            rhi.user38                         , FLOAT_4, 1   , 144196,\
            rhi.user39                         , FLOAT_4, 1   , 144200,\
            rhi.user40                         , FLOAT_4, 1   , 144204,\
            rhi.user41                         , FLOAT_4, 1   , 144208,\
            rhi.user42                         , FLOAT_4, 1   , 144212,\
            rhi.user43                         , FLOAT_4, 1   , 144216,\
            rhi.user44                         , FLOAT_4, 1   , 144220,\
            rhi.user45                         , FLOAT_4, 1   , 144224,\
            rhi.user46                         , FLOAT_4, 1   , 144228,\
            rhi.user47                         , FLOAT_4, 1   , 144232,\
            rhi.user48                         , FLOAT_4, 1   , 144236,\
            rhi.cname                          , CHAR   , 17  , 145249,\
            rhi.brhc_A                         , FLOAT_4, 1   , 144368,\
            rhi.brhc_R                         , FLOAT_4, 1   , 144364,\
            rhi.brhc_S                         , FLOAT_4, 1   , 144372,\
            rhi.ctr_A                          , FLOAT_4, 1   , 144320,\
            rhi.ctr_R                          , FLOAT_4, 1   , 144316,\
            rhi.ctr_S                          , FLOAT_4, 1   , 144324,\
            rhi.dfov                           , FLOAT_4, 1   , 143924,\
            rhi.freq_dir                       , INT_2  , 1   , 145014,\
            rhi.ctyp                           , INT_2  , 1   , 144948,\
            rhi.loc                            , FLOAT_4, 1   , 143944,\
            rhi.mr_flip                        , INT_2  , 1   , 144924,\
            rhi.nex                            , FLOAT_4, 1   , 143952,\
            rhi.numecho                        , INT_2  , 1   , 144890,\
            rhi.image_uid                      , UID    , 32  , 145317,\
            rhi.rawrunnum                      , INT_4  , 1   , 144624,\
            rhe.ex_datetime                    , INT_4  , 1   , 140988,\
            rhe.ex_no                          , UINT_2 , 1   , 141044,\
            rhe.magstrength                    , INT_4  , 1   , 140980,\
            rhe.patid                          , CHAR   , 13  , 141368,\
            rhe.patidff                        , CHAR   , 65  , 141611,\
            rhe.patname                        , CHAR   , 25  , 141381,\
            rhe.patnameff                      , CHAR   , 65  , 141546,\
            rhe.refphy                         , CHAR   , 33  , 141174,\
            rhe.reqnum                         , CHAR   , 13  , 141161,\
            rhe.reqnumff                       , CHAR   , 17  , 141676,\
            rhe.study_uid                      , UID    , 32  , 141450,\
            rhe.dateofbirth                    , CHAR   , 9   , 141693,\
            rhe.patsex                         , INT_2  , 1   , 141068,\
            rhe.hospname                       , CHAR   , 33  , 141335,\
            rhe.ex_sysid                       , CHAR   , 9   , 141312,\
            rhe.uniq_sys_id                    , CHAR   , 16  , 141414,\
            rhe.ex_verscre                     , CHAR   , 2   , 141410,\
            rhs.se_no                          , INT_2  , 1   , 142194,\
            rhs.se_desc                        , CHAR   , 65  , 142318,\
            rhs.entry                          , INT_4  , 1   , 142064,\
            rhs.position                       , INT_4  , 1   , 142060,\
            rhs.series_uid                     , UID    , 32  , 142431,\
            rhs.landmark_uid                   , UID    , 32  , 142463,\
            rhs.anref                          , CHAR   , 3   , 142401,\
        "); 

    } else if ( (int)(this->pfileVersion) == 16 ) {

        offsets.assign (" \
            rhr.rh_rdbm_rev                    , FLOAT_4, 1   , 0,\
            rhr.rh_scan_date                   , CHAR   , 10  , 16,\
            rhr.rh_scan_time                   , CHAR   , 8   , 26,\
            rhr.rh_npasses                     , INT_2  , 1   , 64,\
            rhr.rh_nslices                     , UINT_2 , 1   , 68,\
            rhr.csi_dims                       , INT_2  , 1   , 372,\
            rhr.rh_dab[0].start_rcv            , INT_2  , 1   , 200,\
            rhr.rh_dab[0].stop_rcv             , INT_2  , 1   , 202,\
            rhr.rh_dab[1].start_rcv            , INT_2  , 1   , 204,\
            rhr.rh_dab[1].stop_rcv             , INT_2  , 1   , 206,\
            rhr.rh_dab[2].start_rcv            , INT_2  , 1   , 208,\
            rhr.rh_dab[2].stop_rcv             , INT_2  , 1   , 210,\
            rhr.rh_dab[3].start_rcv            , INT_2  , 1   , 212,\
            rhr.rh_dab[3].stop_rcv             , INT_2  , 1   , 214,\
            rhr.rh_data_collect_type           , INT_2  , 1   , 56,\
            rhr.rh_file_contents               , INT_2  , 1   , 44,\
            rhr.rdb_hdr_off_data               , INT_4  , 1   , 1468,\
            rhr.rh_frame_size                  , UINT_2 , 1   , 80,\
            rhr.rh_point_size                  , INT_2  , 1   , 82,\
            rhr.rh_ps_mps_freq                 , LINT_4 , 1   , 424,\
            rhr.rh_user_usage_tag              , ULINT_4, 1   , 988,\
            rhr.roilenx                        , FLOAT_4, 1   , 380,\
            rhr.roileny                        , FLOAT_4, 1   , 384,\
            rhr.roilenz                        , FLOAT_4, 1   , 388,\
            rhr.spectral_width                 , FLOAT_4, 1   , 368,\
            rhr.xcsi                           , INT_2  , 1   , 374,\
            rhr.ycsi                           , INT_2  , 1   , 376,\
            rhr.zcsi                           , INT_2  , 1   , 378,\
            rhr.rh_logo                        , CHAR   , 10  , 34,\
            rhr.rh_raw_pass_size               , ULINT_4, 1   , 116,\
            rhr.rh_user0                       , FLOAT_4, 1   , 216,\
            rhr.rh_user1                       , FLOAT_4, 1   , 220,\
            rhr.rh_user2                       , FLOAT_4, 1   , 224,\
            rhr.rh_user3                       , FLOAT_4, 1   , 228,\
            rhr.rh_user4                       , FLOAT_4, 1   , 232,\
            rhr.rh_user5                       , FLOAT_4, 1   , 236,\
            rhr.rh_user6                       , FLOAT_4, 1   , 240,\
            rhr.rh_user7                       , FLOAT_4, 1   , 244,\
            rhr.rh_user8                       , FLOAT_4, 1   , 248,\
            rhr.rh_user9                       , FLOAT_4, 1   , 252,\
            rhr.rh_user10                      , FLOAT_4, 1   , 256,\
            rhr.rh_user11                      , FLOAT_4, 1   , 260,\
            rhr.rh_user12                      , FLOAT_4, 1   , 264,\
            rhr.rh_user13                      , FLOAT_4, 1   , 268,\
            rhr.rh_user14                      , FLOAT_4, 1   , 272,\
            rhr.rh_user15                      , FLOAT_4, 1   , 276,\
            rhr.rh_user16                      , FLOAT_4, 1   , 280,\
            rhr.rh_user17                      , FLOAT_4, 1   , 284,\
            rhr.rh_user18                      , FLOAT_4, 1   , 288,\
            rhr.rh_user19                      , FLOAT_4, 1   , 292,\
            rhr.rh_user20                      , FLOAT_4, 1   , 1000,\
            rhr.rh_user21                      , FLOAT_4, 1   , 1004,\
            rhr.rh_user22                      , FLOAT_4, 1   , 1008,\
            rhr.rh_user23                      , FLOAT_4, 1   , 1012,\
            rhr.rh_user24                      , FLOAT_4, 1   , 1016,\
            rhr.rh_user25                      , FLOAT_4, 1   , 1020,\
            rhr.rh_user26                      , FLOAT_4, 1   , 1024,\
            rhr.rh_user27                      , FLOAT_4, 1   , 1028,\
            rhr.rh_user28                      , FLOAT_4, 1   , 1032,\
            rhr.rh_user29                      , FLOAT_4, 1   , 1036,\
            rhr.rh_user30                      , FLOAT_4, 1   , 1040,\
            rhr.rh_user31                      , FLOAT_4, 1   , 1044,\
            rhr.rh_user32                      , FLOAT_4, 1   , 1048,\
            rhr.rh_user33                      , FLOAT_4, 1   , 1052,\
            rhr.rh_user34                      , FLOAT_4, 1   , 1056,\
            rhr.rh_user35                      , FLOAT_4, 1   , 1060,\
            rhr.rh_user36                      , FLOAT_4, 1   , 1064,\
            rhr.rh_user37                      , FLOAT_4, 1   , 1068,\
            rhr.rh_user38                      , FLOAT_4, 1   , 1072,\
            rhr.rh_user39                      , FLOAT_4, 1   , 1076,\
            rhr.rh_user40                      , FLOAT_4, 1   , 1080,\
            rhr.rh_user41                      , FLOAT_4, 1   , 1084,\
            rhr.rh_user42                      , FLOAT_4, 1   , 1088,\
            rhr.rh_user43                      , FLOAT_4, 1   , 1092,\
            rhr.rh_user44                      , FLOAT_4, 1   , 1096,\
            rhr.rh_user45                      , FLOAT_4, 1   , 1100,\
            rhr.rh_user46                      , FLOAT_4, 1   , 1104,\
            rhr.rh_user47                      , FLOAT_4, 1   , 1108,\
            rhr.rh_user48                      , FLOAT_4, 1   , 1112,\
            rhi.psdname                        , CHAR   , 33  , 145180,\
            rhi.scanspacing                    , FLOAT_4, 1   , 143988,\
            rhi.te                             , INT_4  , 1   , 144628,\
            rhi.ti                             , INT_4  , 1   , 144624,\
            rhi.tr                             , INT_4  , 1   , 144620,\
            rhi.tlhc_A                         , FLOAT_4, 1   , 144392,\
            rhi.tlhc_R                         , FLOAT_4, 1   , 144388,\
            rhi.tlhc_S                         , FLOAT_4, 1   , 144396,\
            rhi.t                              , INT_4  , 1   , 144620,\
            rhi.trhc_A                         , FLOAT_4, 1   , 144404,\
            rhi.trhc_R                         , FLOAT_4, 1   , 144400,\
            rhi.trhc_S                         , FLOAT_4, 1   , 144408,\
            rhi.user0                          , FLOAT_4, 1   , 144024,\
            rhi.user1                          , FLOAT_4, 1   , 144028,\
            rhi.user2                          , FLOAT_4, 1   , 144032,\
            rhi.user3                          , FLOAT_4, 1   , 144036,\
            rhi.user4                          , FLOAT_4, 1   , 144040,\
            rhi.user5                          , FLOAT_4, 1   , 144044,\
            rhi.user6                          , FLOAT_4, 1   , 144048,\
            rhi.user7                          , FLOAT_4, 1   , 144052,\
            rhi.user8                          , FLOAT_4, 1   , 144056,\
            rhi.user9                          , FLOAT_4, 1   , 144060,\
            rhi.user10                         , FLOAT_4, 1   , 144064,\
            rhi.user11                         , FLOAT_4, 1   , 144068,\
            rhi.user12                         , FLOAT_4, 1   , 144072,\
            rhi.user13                         , FLOAT_4, 1   , 144076,\
            rhi.user14                         , FLOAT_4, 1   , 144080,\
            rhi.user15                         , FLOAT_4, 1   , 144084,\
            rhi.user16                         , FLOAT_4, 1   , 144088,\
            rhi.user17                         , FLOAT_4, 1   , 144092,\
            rhi.user18                         , FLOAT_4, 1   , 144096,\
            rhi.user19                         , FLOAT_4, 1   , 144100,\
            rhi.user20                         , FLOAT_4, 1   , 144104,\
            rhi.user21                         , FLOAT_4, 1   , 144108,\
            rhi.user22                         , FLOAT_4, 1   , 144112,\
            rhi.user23                         , FLOAT_4, 1   , 144124,\
            rhi.user24                         , FLOAT_4, 1   , 144128,\
            rhi.user25                         , FLOAT_4, 1   , 144192,\
            rhi.user26                         , FLOAT_4, 1   , 144196,\
            rhi.user27                         , FLOAT_4, 1   , 144200,\
            rhi.user28                         , FLOAT_4, 1   , 144204,\
            rhi.user29                         , FLOAT_4, 1   , 144208,\
            rhi.user30                         , FLOAT_4, 1   , 144212,\
            rhi.user31                         , FLOAT_4, 1   , 144216,\
            rhi.user32                         , FLOAT_4, 1   , 144220,\
            rhi.user33                         , FLOAT_4, 1   , 144224,\
            rhi.user34                         , FLOAT_4, 1   , 144228,\
            rhi.user35                         , FLOAT_4, 1   , 144232,\
            rhi.user36                         , FLOAT_4, 1   , 144236,\
            rhi.user37                         , FLOAT_4, 1   , 144240,\
            rhi.user38                         , FLOAT_4, 1   , 144244,\
            rhi.user39                         , FLOAT_4, 1   , 144248,\
            rhi.user40                         , FLOAT_4, 1   , 144252,\
            rhi.user41                         , FLOAT_4, 1   , 144256,\
            rhi.user42                         , FLOAT_4, 1   , 144260,\
            rhi.user43                         , FLOAT_4, 1   , 144264,\
            rhi.user44                         , FLOAT_4, 1   , 144268,\
            rhi.user45                         , FLOAT_4, 1   , 144272,\
            rhi.user46                         , FLOAT_4, 1   , 144276,\
            rhi.user47                         , FLOAT_4, 1   , 144280,\
            rhi.user48                         , FLOAT_4, 1   , 144284,\
            rhi.cname                          , CHAR   , 17  , 145297,\
            rhi.brhc_A                         , FLOAT_4, 1   , 144416,\
            rhi.brhc_R                         , FLOAT_4, 1   , 144412,\
            rhi.brhc_S                         , FLOAT_4, 1   , 144420,\
            rhi.ctr_A                          , FLOAT_4, 1   , 144368,\
            rhi.ctr_R                          , FLOAT_4, 1   , 144364,\
            rhi.ctr_S                          , FLOAT_4, 1   , 144372,\
            rhi.dfov                           , FLOAT_4, 1   , 143972,\
            rhi.freq_dir                       , INT_2  , 1   , 145062,\
            rhi.ctyp                           , INT_2  , 1   , 144996,\
            rhi.loc                            , FLOAT_4, 1   , 143992,\
            rhi.mr_flip                        , INT_2  , 1   , 144972,\
            rhi.nex                            , FLOAT_4, 1   , 144000,\
            rhi.numecho                        , INT_2  , 1   , 144938,\
            rhi.image_uid                      , UID    , 32  , 145365,\
            rhi.rawrunnum                      , INT_4  , 1   , 144672,\
            rhe.ex_datetime                    , INT_4  , 1   , 140988,\
            rhe.ex_no                          , UINT_2 , 1   , 141044,\
            rhe.magstrength                    , INT_4  , 1   , 140980,\
            rhe.patid                          , CHAR   , 13  , 141368,\
            rhe.patidff                        , CHAR   , 65  , 141611,\
            rhe.patname                        , CHAR   , 25  , 141381,\
            rhe.patnameff                      , CHAR   , 65  , 141546,\
            rhe.refphy                         , CHAR   , 33  , 141174,\
            rhe.reqnum                         , CHAR   , 13  , 141161,\
            rhe.reqnumff                       , CHAR   , 17  , 141676,\
            rhe.study_uid                      , UID    , 32  , 141450,\
            rhe.dateofbirth                    , CHAR   , 9   , 141693,\
            rhe.patsex                         , INT_2  , 1   , 141068,\
            rhe.hospname                       , CHAR   , 33  , 141335,\
            rhe.ex_sysid                       , CHAR   , 9   , 141312,\
            rhe.uniq_sys_id                    , CHAR   , 16  , 141414,\
            rhe.ex_verscre                     , CHAR   , 2   , 141410,\
            rhs.se_no                          , INT_2  , 1   , 142194,\
            rhs.se_desc                        , CHAR   , 65  , 142318,\
            rhs.entry                          , INT_4  , 1   , 142064,\
            rhs.position                       , INT_4  , 1   , 142060,\
            rhs.series_uid                     , UID    , 32  , 142431,\
            rhs.landmark_uid                   , UID    , 32  , 142463,\
            rhs.anref                          , CHAR   , 3   , 142401,\
        "); 


    } else if ( (int)(this->pfileVersion) == 20 ) {

        offsets.assign (" \
            rhr.rh_rdbm_rev                    , FLOAT_4, 1   , 0,\
            rhr.rh_scan_date                   , CHAR   , 10  , 16,\
            rhr.rh_scan_time                   , CHAR   , 8   , 26,\
            rhr.rh_npasses                     , INT_2  , 1   , 64,\
            rhr.rh_nslices                     , UINT_2 , 1   , 68,\
            rhr.csi_dims                       , INT_2  , 1   , 372,\
            rhr.rh_dab[0].start_rcv            , INT_2  , 1   , 200,\
            rhr.rh_dab[0].stop_rcv             , INT_2  , 1   , 202,\
            rhr.rh_dab[1].start_rcv            , INT_2  , 1   , 204,\
            rhr.rh_dab[1].stop_rcv             , INT_2  , 1   , 206,\
            rhr.rh_dab[2].start_rcv            , INT_2  , 1   , 208,\
            rhr.rh_dab[2].stop_rcv             , INT_2  , 1   , 210,\
            rhr.rh_dab[3].start_rcv            , INT_2  , 1   , 212,\
            rhr.rh_dab[3].stop_rcv             , INT_2  , 1   , 214,\
            rhr.rh_data_collect_type           , INT_2  , 1   , 56,\
            rhr.rh_file_contents               , INT_2  , 1   , 44,\
            rhr.rdb_hdr_off_data               , INT_4  , 1   , 1468,\
            rhr.rh_frame_size                  , UINT_2 , 1   , 80,\
            rhr.rh_point_size                  , INT_2  , 1   , 82,\
            rhr.rh_ps_mps_freq                 , UINT_4 , 1   , 424,\
            rhr.rh_user_usage_tag              , UINT_4 , 1   , 988,\
            rhr.roilenx                        , FLOAT_4, 1   , 380,\
            rhr.roileny                        , FLOAT_4, 1   , 384,\
            rhr.roilenz                        , FLOAT_4, 1   , 388,\
            rhr.spectral_width                 , FLOAT_4, 1   , 368,\
            rhr.xcsi                           , INT_2  , 1   , 374,\
            rhr.ycsi                           , INT_2  , 1   , 376,\
            rhr.zcsi                           , INT_2  , 1   , 378,\
            rhr.rh_logo                        , CHAR   , 10  , 34,\
            rhr.rh_raw_pass_size               , LINT_8 , 1   , 1660,\
            rhr.rh_user0                       , FLOAT_4, 1   , 216,\
            rhr.rh_user1                       , FLOAT_4, 1   , 220,\
            rhr.rh_user2                       , FLOAT_4, 1   , 224,\
            rhr.rh_user3                       , FLOAT_4, 1   , 228,\
            rhr.rh_user4                       , FLOAT_4, 1   , 232,\
            rhr.rh_user5                       , FLOAT_4, 1   , 236,\
            rhr.rh_user6                       , FLOAT_4, 1   , 240,\
            rhr.rh_user7                       , FLOAT_4, 1   , 244,\
            rhr.rh_user8                       , FLOAT_4, 1   , 248,\
            rhr.rh_user9                       , FLOAT_4, 1   , 252,\
            rhr.rh_user10                      , FLOAT_4, 1   , 256,\
            rhr.rh_user11                      , FLOAT_4, 1   , 260,\
            rhr.rh_user12                      , FLOAT_4, 1   , 264,\
            rhr.rh_user13                      , FLOAT_4, 1   , 268,\
            rhr.rh_user14                      , FLOAT_4, 1   , 272,\
            rhr.rh_user15                      , FLOAT_4, 1   , 276,\
            rhr.rh_user16                      , FLOAT_4, 1   , 280,\
            rhr.rh_user17                      , FLOAT_4, 1   , 284,\
            rhr.rh_user18                      , FLOAT_4, 1   , 288,\
            rhr.rh_user19                      , FLOAT_4, 1   , 292,\
            rhr.rh_user20                      , FLOAT_4, 1   , 1000,\
            rhr.rh_user21                      , FLOAT_4, 1   , 1004,\
            rhr.rh_user22                      , FLOAT_4, 1   , 1008,\
            rhr.rh_user23                      , FLOAT_4, 1   , 1012,\
            rhr.rh_user24                      , FLOAT_4, 1   , 1016,\
            rhr.rh_user25                      , FLOAT_4, 1   , 1020,\
            rhr.rh_user26                      , FLOAT_4, 1   , 1024,\
            rhr.rh_user27                      , FLOAT_4, 1   , 1028,\
            rhr.rh_user28                      , FLOAT_4, 1   , 1032,\
            rhr.rh_user29                      , FLOAT_4, 1   , 1036,\
            rhr.rh_user30                      , FLOAT_4, 1   , 1040,\
            rhr.rh_user31                      , FLOAT_4, 1   , 1044,\
            rhr.rh_user32                      , FLOAT_4, 1   , 1048,\
            rhr.rh_user33                      , FLOAT_4, 1   , 1052,\
            rhr.rh_user34                      , FLOAT_4, 1   , 1056,\
            rhr.rh_user35                      , FLOAT_4, 1   , 1060,\
            rhr.rh_user36                      , FLOAT_4, 1   , 1064,\
            rhr.rh_user37                      , FLOAT_4, 1   , 1068,\
            rhr.rh_user38                      , FLOAT_4, 1   , 1072,\
            rhr.rh_user39                      , FLOAT_4, 1   , 1076,\
            rhr.rh_user40                      , FLOAT_4, 1   , 1080,\
            rhr.rh_user41                      , FLOAT_4, 1   , 1084,\
            rhr.rh_user42                      , FLOAT_4, 1   , 1088,\
            rhr.rh_user43                      , FLOAT_4, 1   , 1092,\
            rhr.rh_user44                      , FLOAT_4, 1   , 1096,\
            rhr.rh_user45                      , FLOAT_4, 1   , 1100,\
            rhr.rh_user46                      , FLOAT_4, 1   , 1104,\
            rhr.rh_user47                      , FLOAT_4, 1   , 1108,\
            rhr.rh_user48                      , FLOAT_4, 1   , 1112,\
            rhi.psdname                        , CHAR   , 33  , 148972,\
            rhi.scanspacing                    , FLOAT_4, 1   , 147660,\
            rhi.te                             , INT_4  , 1   , 148404,\
            rhi.ti                             , INT_4  , 1   , 148400,\
            rhi.tr                             , INT_4  , 1   , 148396,\
            rhi.tlhc_A                         , FLOAT_4, 1   , 148064,\
            rhi.tlhc_R                         , FLOAT_4, 1   , 148060,\
            rhi.tlhc_S                         , FLOAT_4, 1   , 148068,\
            rhi.t                              , INT_4  , 1   , 148396,\
            rhi.trhc_A                         , FLOAT_4, 1   , 148076,\
            rhi.trhc_R                         , FLOAT_4, 1   , 148072,\
            rhi.trhc_S                         , FLOAT_4, 1   , 148080,\
            rhi.user0                          , FLOAT_4, 1   , 147696,\
            rhi.user1                          , FLOAT_4, 1   , 147700,\
            rhi.user2                          , FLOAT_4, 1   , 147704,\
            rhi.user3                          , FLOAT_4, 1   , 147708,\
            rhi.user4                          , FLOAT_4, 1   , 147712,\
            rhi.user5                          , FLOAT_4, 1   , 147716,\
            rhi.user6                          , FLOAT_4, 1   , 147720,\
            rhi.user7                          , FLOAT_4, 1   , 147724,\
            rhi.user8                          , FLOAT_4, 1   , 147728,\
            rhi.user9                          , FLOAT_4, 1   , 147732,\
            rhi.user10                         , FLOAT_4, 1   , 147736,\
            rhi.user11                         , FLOAT_4, 1   , 147740,\
            rhi.user12                         , FLOAT_4, 1   , 147744,\
            rhi.user13                         , FLOAT_4, 1   , 147748,\
            rhi.user14                         , FLOAT_4, 1   , 147752,\
            rhi.user15                         , FLOAT_4, 1   , 147756,\
            rhi.user16                         , FLOAT_4, 1   , 147760,\
            rhi.user17                         , FLOAT_4, 1   , 147764,\
            rhi.user18                         , FLOAT_4, 1   , 147768,\
            rhi.user19                         , FLOAT_4, 1   , 147772,\
            rhi.user20                         , FLOAT_4, 1   , 147776,\
            rhi.user21                         , FLOAT_4, 1   , 147780,\
            rhi.user22                         , FLOAT_4, 1   , 147784,\
            rhi.user23                         , FLOAT_4, 1   , 147796,\
            rhi.user24                         , FLOAT_4, 1   , 147800,\
            rhi.user25                         , FLOAT_4, 1   , 147864,\
            rhi.user26                         , FLOAT_4, 1   , 147868,\
            rhi.user27                         , FLOAT_4, 1   , 147872,\
            rhi.user28                         , FLOAT_4, 1   , 147876,\
            rhi.user29                         , FLOAT_4, 1   , 147880,\
            rhi.user30                         , FLOAT_4, 1   , 147884,\
            rhi.user31                         , FLOAT_4, 1   , 147888,\
            rhi.user32                         , FLOAT_4, 1   , 147892,\
            rhi.user33                         , FLOAT_4, 1   , 147896,\
            rhi.user34                         , FLOAT_4, 1   , 147900,\
            rhi.user35                         , FLOAT_4, 1   , 147904,\
            rhi.user36                         , FLOAT_4, 1   , 147908,\
            rhi.user37                         , FLOAT_4, 1   , 147912,\
            rhi.user38                         , FLOAT_4, 1   , 147916,\
            rhi.user39                         , FLOAT_4, 1   , 147920,\
            rhi.user40                         , FLOAT_4, 1   , 147924,\
            rhi.user41                         , FLOAT_4, 1   , 147928,\
            rhi.user42                         , FLOAT_4, 1   , 147932,\
            rhi.user43                         , FLOAT_4, 1   , 147936,\
            rhi.user44                         , FLOAT_4, 1   , 147940,\
            rhi.user45                         , FLOAT_4, 1   , 147944,\
            rhi.user46                         , FLOAT_4, 1   , 147948,\
            rhi.user47                         , FLOAT_4, 1   , 147952,\
            rhi.user48                         , FLOAT_4, 1   , 147956,\
            rhi.cname                          , CHAR   , 17  , 149089,\
            rhi.brhc_A                         , FLOAT_4, 1   , 148088,\
            rhi.brhc_R                         , FLOAT_4, 1   , 148084,\
            rhi.brhc_S                         , FLOAT_4, 1   , 148092,\
            rhi.ctr_A                          , FLOAT_4, 1   , 148040,\
            rhi.ctr_R                          , FLOAT_4, 1   , 148036,\
            rhi.ctr_S                          , FLOAT_4, 1   , 148044,\
            rhi.dfov                           , FLOAT_4, 1   , 147644,\
            rhi.freq_dir                       , INT_2  , 1   , 148840,\
            rhi.ctyp                           , INT_2  , 1   , 148774,\
            rhi.loc                            , FLOAT_4, 1   , 147664,\
            rhi.mr_flip                        , INT_2  , 1   , 148752,\
            rhi.nex                            , FLOAT_4, 1   , 147672,\
            rhi.numecho                        , INT_2  , 1   , 148718,\
            rhi.image_uid                      , UID    , 32  , 149157,\
            rhi.rawrunnum                      , INT_4  , 1   , 148448,\
            rhe.ex_datetime                    , INT_4  , 1   , 143400,\
            rhe.ex_no                          , UINT_2 , 1   , 143516,\
            rhe.magstrength                    , INT_4  , 1   , 143392,\
            rhe.patid                          , CHAR   , 65  , 144401,\
            rhe.patname                        , CHAR   , 65  , 144336,\
            rhe.refphy                         , CHAR   , 65  , 143877,\
            rhe.reqnum                         , CHAR   , 17  , 144466,\
            rhe.study_uid                      , UID    , 32  , 144240,\
            rhe.dateofbirth                    , CHAR   , 9   , 144483,\
            rhe.patsex                         , INT_2  , 1   , 143540,\
            rhe.hospname                       , CHAR   , 33  , 144163,\
            rhe.ex_sysid                       , CHAR   , 9   , 144140,\
            rhe.uniq_sys_id                    , CHAR   , 16  , 144204,\
            rhe.ex_verscre                     , CHAR   , 2   , 144200,\
            rhs.se_no                          , INT_2  , 1   , 145622,\
            rhs.se_desc                        , CHAR   , 65  , 145762,\
            rhs.entry                          , INT_4  , 1   , 145424,\
            rhs.position                       , INT_4  , 1   , 145420,\
            rhs.series_uid                     , UID    , 32  , 145875,\
            rhs.landmark_uid                   , UID    , 32  , 145907,\
            rhs.anref                          , CHAR   , 3   , 145845,\
        "); 

    } else if ( (int)(this->pfileVersion) == 21 ) {

        offsets.assign (" \
            rhr.rh_rdbm_rev                    , FLOAT_4, 1   , 0,\
            rhr.rh_scan_date                   , CHAR   , 10  , 16,\
            rhr.rh_scan_time                   , CHAR   , 8   , 26,\
            rhr.rh_npasses                     , INT_2  , 1   , 64,\
            rhr.rh_nslices                     , UINT_2 , 1   , 68,\
            rhr.csi_dims                       , INT_2  , 1   , 372,\
            rhr.rh_dab[0].start_rcv            , INT_2  , 1   , 200,\
            rhr.rh_dab[0].stop_rcv             , INT_2  , 1   , 202,\
            rhr.rh_dab[1].start_rcv            , INT_2  , 1   , 204,\
            rhr.rh_dab[1].stop_rcv             , INT_2  , 1   , 206,\
            rhr.rh_dab[2].start_rcv            , INT_2  , 1   , 208,\
            rhr.rh_dab[2].stop_rcv             , INT_2  , 1   , 210,\
            rhr.rh_dab[3].start_rcv            , INT_2  , 1   , 212,\
            rhr.rh_dab[3].stop_rcv             , INT_2  , 1   , 214,\
            rhr.rh_data_collect_type           , INT_2  , 1   , 56,\
            rhr.rh_file_contents               , INT_2  , 1   , 44,\
            rhr.rdb_hdr_off_data               , INT_4  , 1   , 1468,\
            rhr.rh_frame_size                  , UINT_2 , 1   , 80,\
            rhr.rh_point_size                  , INT_2  , 1   , 82,\
            rhr.rh_ps_mps_freq                 , UINT_4 , 1   , 424,\
            rhr.rh_user_usage_tag              , UINT_4 , 1   , 988,\
            rhr.roilenx                        , FLOAT_4, 1   , 380,\
            rhr.roileny                        , FLOAT_4, 1   , 384,\
            rhr.roilenz                        , FLOAT_4, 1   , 388,\
            rhr.spectral_width                 , FLOAT_4, 1   , 368,\
            rhr.xcsi                           , INT_2  , 1   , 374,\
            rhr.ycsi                           , INT_2  , 1   , 376,\
            rhr.zcsi                           , INT_2  , 1   , 378,\
            rhr.rh_logo                        , CHAR   , 10  , 34,\
            rhr.rh_raw_pass_size               , LINT_8 , 1   , 1660,\
            rhr.rh_user0                       , FLOAT_4, 1   , 216,\
            rhr.rh_user1                       , FLOAT_4, 1   , 220,\
            rhr.rh_user2                       , FLOAT_4, 1   , 224,\
            rhr.rh_user3                       , FLOAT_4, 1   , 228,\
            rhr.rh_user4                       , FLOAT_4, 1   , 232,\
            rhr.rh_user5                       , FLOAT_4, 1   , 236,\
            rhr.rh_user6                       , FLOAT_4, 1   , 240,\
            rhr.rh_user7                       , FLOAT_4, 1   , 244,\
            rhr.rh_user8                       , FLOAT_4, 1   , 248,\
            rhr.rh_user9                       , FLOAT_4, 1   , 252,\
            rhr.rh_user10                      , FLOAT_4, 1   , 256,\
            rhr.rh_user11                      , FLOAT_4, 1   , 260,\
            rhr.rh_user12                      , FLOAT_4, 1   , 264,\
            rhr.rh_user13                      , FLOAT_4, 1   , 268,\
            rhr.rh_user14                      , FLOAT_4, 1   , 272,\
            rhr.rh_user15                      , FLOAT_4, 1   , 276,\
            rhr.rh_user16                      , FLOAT_4, 1   , 280,\
            rhr.rh_user17                      , FLOAT_4, 1   , 284,\
            rhr.rh_user18                      , FLOAT_4, 1   , 288,\
            rhr.rh_user19                      , FLOAT_4, 1   , 292,\
            rhr.rh_user20                      , FLOAT_4, 1   , 1000,\
            rhr.rh_user21                      , FLOAT_4, 1   , 1004,\
            rhr.rh_user22                      , FLOAT_4, 1   , 1008,\
            rhr.rh_user23                      , FLOAT_4, 1   , 1012,\
            rhr.rh_user24                      , FLOAT_4, 1   , 1016,\
            rhr.rh_user25                      , FLOAT_4, 1   , 1020,\
            rhr.rh_user26                      , FLOAT_4, 1   , 1024,\
            rhr.rh_user27                      , FLOAT_4, 1   , 1028,\
            rhr.rh_user28                      , FLOAT_4, 1   , 1032,\
            rhr.rh_user29                      , FLOAT_4, 1   , 1036,\
            rhr.rh_user30                      , FLOAT_4, 1   , 1040,\
            rhr.rh_user31                      , FLOAT_4, 1   , 1044,\
            rhr.rh_user32                      , FLOAT_4, 1   , 1048,\
            rhr.rh_user33                      , FLOAT_4, 1   , 1052,\
            rhr.rh_user34                      , FLOAT_4, 1   , 1056,\
            rhr.rh_user35                      , FLOAT_4, 1   , 1060,\
            rhr.rh_user36                      , FLOAT_4, 1   , 1064,\
            rhr.rh_user37                      , FLOAT_4, 1   , 1068,\
            rhr.rh_user38                      , FLOAT_4, 1   , 1072,\
            rhr.rh_user39                      , FLOAT_4, 1   , 1076,\
            rhr.rh_user40                      , FLOAT_4, 1   , 1080,\
            rhr.rh_user41                      , FLOAT_4, 1   , 1084,\
            rhr.rh_user42                      , FLOAT_4, 1   , 1088,\
            rhr.rh_user43                      , FLOAT_4, 1   , 1092,\
            rhr.rh_user44                      , FLOAT_4, 1   , 1096,\
            rhr.rh_user45                      , FLOAT_4, 1   , 1100,\
            rhr.rh_user46                      , FLOAT_4, 1   , 1104,\
            rhr.rh_user47                      , FLOAT_4, 1   , 1108,\
            rhr.rh_user48                      , FLOAT_4, 1   , 1112,\
            rhi.psdname                        , CHAR   , 33  , 149520,\
            rhi.scanspacing                    , FLOAT_4, 1   , 148208,\
            rhi.te                             , INT_4  , 1   , 148952,\
            rhi.ti                             , INT_4  , 1   , 148948,\
            rhi.tr                             , INT_4  , 1   , 148944,\
            rhi.tlhc_A                         , FLOAT_4, 1   , 148612,\
            rhi.tlhc_R                         , FLOAT_4, 1   , 148608,\
            rhi.tlhc_S                         , FLOAT_4, 1   , 148616,\
            rhi.t                              , INT_4  , 1   , 148944,\
            rhi.trhc_A                         , FLOAT_4, 1   , 148624,\
            rhi.trhc_R                         , FLOAT_4, 1   , 148620,\
            rhi.trhc_S                         , FLOAT_4, 1   , 148628,\
            rhi.user0                          , FLOAT_4, 1   , 148244,\
            rhi.user1                          , FLOAT_4, 1   , 148248,\
            rhi.user2                          , FLOAT_4, 1   , 148252,\
            rhi.user3                          , FLOAT_4, 1   , 148256,\
            rhi.user4                          , FLOAT_4, 1   , 148260,\
            rhi.user5                          , FLOAT_4, 1   , 148264,\
            rhi.user6                          , FLOAT_4, 1   , 148268,\
            rhi.user7                          , FLOAT_4, 1   , 148272,\
            rhi.user8                          , FLOAT_4, 1   , 148276,\
            rhi.user9                          , FLOAT_4, 1   , 148280,\
            rhi.user10                         , FLOAT_4, 1   , 148284,\
            rhi.user11                         , FLOAT_4, 1   , 148288,\
            rhi.user12                         , FLOAT_4, 1   , 148292,\
            rhi.user13                         , FLOAT_4, 1   , 148296,\
            rhi.user14                         , FLOAT_4, 1   , 148300,\
            rhi.user15                         , FLOAT_4, 1   , 148304,\
            rhi.user16                         , FLOAT_4, 1   , 148308,\
            rhi.user17                         , FLOAT_4, 1   , 148312,\
            rhi.user18                         , FLOAT_4, 1   , 148316,\
            rhi.user19                         , FLOAT_4, 1   , 148320,\
            rhi.user20                         , FLOAT_4, 1   , 148324,\
            rhi.user21                         , FLOAT_4, 1   , 148328,\
            rhi.user22                         , FLOAT_4, 1   , 148332,\
            rhi.user23                         , FLOAT_4, 1   , 148344,\
            rhi.user24                         , FLOAT_4, 1   , 148348,\
            rhi.user25                         , FLOAT_4, 1   , 148412,\
            rhi.user26                         , FLOAT_4, 1   , 148416,\
            rhi.user27                         , FLOAT_4, 1   , 148420,\
            rhi.user28                         , FLOAT_4, 1   , 148424,\
            rhi.user29                         , FLOAT_4, 1   , 148428,\
            rhi.user30                         , FLOAT_4, 1   , 148432,\
            rhi.user31                         , FLOAT_4, 1   , 148436,\
            rhi.user32                         , FLOAT_4, 1   , 148440,\
            rhi.user33                         , FLOAT_4, 1   , 148444,\
            rhi.user34                         , FLOAT_4, 1   , 148448,\
            rhi.user35                         , FLOAT_4, 1   , 148452,\
            rhi.user36                         , FLOAT_4, 1   , 148456,\
            rhi.user37                         , FLOAT_4, 1   , 148460,\
            rhi.user38                         , FLOAT_4, 1   , 148464,\
            rhi.user39                         , FLOAT_4, 1   , 148468,\
            rhi.user40                         , FLOAT_4, 1   , 148472,\
            rhi.user41                         , FLOAT_4, 1   , 148476,\
            rhi.user42                         , FLOAT_4, 1   , 148480,\
            rhi.user43                         , FLOAT_4, 1   , 148484,\
            rhi.user44                         , FLOAT_4, 1   , 148488,\
            rhi.user45                         , FLOAT_4, 1   , 148492,\
            rhi.user46                         , FLOAT_4, 1   , 148496,\
            rhi.user47                         , FLOAT_4, 1   , 148500,\
            rhi.user48                         , FLOAT_4, 1   , 148504,\
            rhi.cname                          , CHAR   , 17  , 149637,\
            rhi.brhc_A                         , FLOAT_4, 1   , 148636,\
            rhi.brhc_R                         , FLOAT_4, 1   , 148632,\
            rhi.brhc_S                         , FLOAT_4, 1   , 148640,\
            rhi.ctr_A                          , FLOAT_4, 1   , 148588,\
            rhi.ctr_R                          , FLOAT_4, 1   , 148584,\
            rhi.ctr_S                          , FLOAT_4, 1   , 148592,\
            rhi.dfov                           , FLOAT_4, 1   , 148192,\
            rhi.freq_dir                       , INT_2  , 1   , 149388,\
            rhi.ctyp                           , INT_2  , 1   , 149322,\
            rhi.loc                            , FLOAT_4, 1   , 148212,\
            rhi.mr_flip                        , INT_2  , 1   , 149300,\
            rhi.nex                            , FLOAT_4, 1   , 148220,\
            rhi.numecho                        , INT_2  , 1   , 149266,\
            rhi.image_uid                      , UID    , 32  , 149705,\
            rhi.rawrunnum                      , INT_4  , 1   , 148996,\
            rhe.ex_datetime                    , INT_4  , 1   , 143948,\
            rhe.ex_no                          , UINT_2 , 1   , 144064,\
            rhe.magstrength                    , INT_4  , 1   , 143940,\
            rhe.patid                          , CHAR   , 65  , 144949,\
            rhe.patname                        , CHAR   , 65  , 144884,\
            rhe.refphy                         , CHAR   , 65  , 144425,\
            rhe.reqnum                         , CHAR   , 17  , 145014,\
            rhe.study_uid                      , UID    , 32  , 144788,\
            rhe.dateofbirth                    , CHAR   , 9   , 145031,\
            rhe.patsex                         , INT_2  , 1   , 144088,\
            rhe.hospname                       , CHAR   , 33  , 144711,\
            rhe.ex_sysid                       , CHAR   , 9   , 144688,\
            rhe.uniq_sys_id                    , CHAR   , 16  , 144752,\
            rhe.ex_verscre                     , CHAR   , 2   , 144748,\
            rhs.se_no                          , INT_2  , 1   , 146170,\
            rhs.se_desc                        , CHAR   , 65  , 146310,\
            rhs.entry                          , INT_4  , 1   , 145972,\
            rhs.position                       , INT_4  , 1   , 145968,\
            rhs.series_uid                     , UID    , 32  , 146423,\
            rhs.landmark_uid                   , UID    , 32  , 146455,\
            rhs.anref                          , CHAR   , 3   , 146393,\
        "); 

    } else if ( (int)(this->pfileVersion) == 23 ) {

        offsets.assign (" \
            rhr.rh_rdbm_rev                    , FLOAT_4, 1   , 0,\
            rhr.rh_scan_date                   , CHAR   , 10  , 16,\
            rhr.rh_scan_time                   , CHAR   , 8   , 26,\
            rhr.rh_npasses                     , INT_2  , 1   , 64,\
            rhr.rh_nslices                     , UINT_2 , 1   , 68,\
            rhr.csi_dims                       , INT_2  , 1   , 372,\
            rhr.rh_dab[0].start_rcv            , INT_2  , 1   , 200,\
            rhr.rh_dab[0].stop_rcv             , INT_2  , 1   , 202,\
            rhr.rh_dab[1].start_rcv            , INT_2  , 1   , 204,\
            rhr.rh_dab[1].stop_rcv             , INT_2  , 1   , 206,\
            rhr.rh_dab[2].start_rcv            , INT_2  , 1   , 208,\
            rhr.rh_dab[2].stop_rcv             , INT_2  , 1   , 210,\
            rhr.rh_dab[3].start_rcv            , INT_2  , 1   , 212,\
            rhr.rh_dab[3].stop_rcv             , INT_2  , 1   , 214,\
            rhr.rh_data_collect_type           , INT_2  , 1   , 56,\
            rhr.rh_file_contents               , INT_2  , 1   , 44,\
            rhr.rdb_hdr_off_data               , INT_4  , 1   , 1468,\
            rhr.rh_frame_size                  , UINT_2 , 1   , 80,\
            rhr.rh_point_size                  , INT_2  , 1   , 82,\
            rhr.rh_ps_mps_freq                 , UINT_4 , 1   , 424,\
            rhr.rh_user_usage_tag              , UINT_4 , 1   , 988,\
            rhr.roilenx                        , FLOAT_4, 1   , 380,\
            rhr.roileny                        , FLOAT_4, 1   , 384,\
            rhr.roilenz                        , FLOAT_4, 1   , 388,\
            rhr.spectral_width                 , FLOAT_4, 1   , 368,\
            rhr.xcsi                           , INT_2  , 1   , 374,\
            rhr.ycsi                           , INT_2  , 1   , 376,\
            rhr.zcsi                           , INT_2  , 1   , 378,\
            rhr.rh_logo                        , CHAR   , 10  , 34,\
            rhr.rh_raw_pass_size               , LINT_8 , 1   , 1660,\
            rhr.rh_user0                       , FLOAT_4, 1   , 216,\
            rhr.rh_user1                       , FLOAT_4, 1   , 220,\
            rhr.rh_user2                       , FLOAT_4, 1   , 224,\
            rhr.rh_user3                       , FLOAT_4, 1   , 228,\
            rhr.rh_user4                       , FLOAT_4, 1   , 232,\
            rhr.rh_user5                       , FLOAT_4, 1   , 236,\
            rhr.rh_user6                       , FLOAT_4, 1   , 240,\
            rhr.rh_user7                       , FLOAT_4, 1   , 244,\
            rhr.rh_user8                       , FLOAT_4, 1   , 248,\
            rhr.rh_user9                       , FLOAT_4, 1   , 252,\
            rhr.rh_user10                      , FLOAT_4, 1   , 256,\
            rhr.rh_user11                      , FLOAT_4, 1   , 260,\
            rhr.rh_user12                      , FLOAT_4, 1   , 264,\
            rhr.rh_user13                      , FLOAT_4, 1   , 268,\
            rhr.rh_user14                      , FLOAT_4, 1   , 272,\
            rhr.rh_user15                      , FLOAT_4, 1   , 276,\
            rhr.rh_user16                      , FLOAT_4, 1   , 280,\
            rhr.rh_user17                      , FLOAT_4, 1   , 284,\
            rhr.rh_user18                      , FLOAT_4, 1   , 288,\
            rhr.rh_user19                      , FLOAT_4, 1   , 292,\
            rhr.rh_user20                      , FLOAT_4, 1   , 1000,\
            rhr.rh_user21                      , FLOAT_4, 1   , 1004,\
            rhr.rh_user22                      , FLOAT_4, 1   , 1008,\
            rhr.rh_user23                      , FLOAT_4, 1   , 1012,\
            rhr.rh_user24                      , FLOAT_4, 1   , 1016,\
            rhr.rh_user25                      , FLOAT_4, 1   , 1020,\
            rhr.rh_user26                      , FLOAT_4, 1   , 1024,\
            rhr.rh_user27                      , FLOAT_4, 1   , 1028,\
            rhr.rh_user28                      , FLOAT_4, 1   , 1032,\
            rhr.rh_user29                      , FLOAT_4, 1   , 1036,\
            rhr.rh_user30                      , FLOAT_4, 1   , 1040,\
            rhr.rh_user31                      , FLOAT_4, 1   , 1044,\
            rhr.rh_user32                      , FLOAT_4, 1   , 1048,\
            rhr.rh_user33                      , FLOAT_4, 1   , 1052,\
            rhr.rh_user34                      , FLOAT_4, 1   , 1056,\
            rhr.rh_user35                      , FLOAT_4, 1   , 1060,\
            rhr.rh_user36                      , FLOAT_4, 1   , 1064,\
            rhr.rh_user37                      , FLOAT_4, 1   , 1068,\
            rhr.rh_user38                      , FLOAT_4, 1   , 1072,\
            rhr.rh_user39                      , FLOAT_4, 1   , 1076,\
            rhr.rh_user40                      , FLOAT_4, 1   , 1080,\
            rhr.rh_user41                      , FLOAT_4, 1   , 1084,\
            rhr.rh_user42                      , FLOAT_4, 1   , 1088,\
            rhr.rh_user43                      , FLOAT_4, 1   , 1092,\
            rhr.rh_user44                      , FLOAT_4, 1   , 1096,\
            rhr.rh_user45                      , FLOAT_4, 1   , 1100,\
            rhr.rh_user46                      , FLOAT_4, 1   , 1104,\
            rhr.rh_user47                      , FLOAT_4, 1   , 1108,\
            rhr.rh_user48                      , FLOAT_4, 1   , 1112,\
            rhi.psdname                        , CHAR   , 33  , 148972,\
            rhi.scanspacing                    , FLOAT_4, 1   , 147660,\
            rhi.te                             , INT_4  , 1   , 148404,\
            rhi.ti                             , INT_4  , 1   , 148400,\
            rhi.tr                             , INT_4  , 1   , 148396,\
            rhi.tlhc_A                         , FLOAT_4, 1   , 148064,\
            rhi.tlhc_R                         , FLOAT_4, 1   , 148060,\
            rhi.tlhc_S                         , FLOAT_4, 1   , 148068,\
            rhi.t                              , INT_4  , 1   , 148396,\
            rhi.trhc_A                         , FLOAT_4, 1   , 148076,\
            rhi.trhc_R                         , FLOAT_4, 1   , 148072,\
            rhi.trhc_S                         , FLOAT_4, 1   , 148080,\
            rhi.user0                          , FLOAT_4, 1   , 147696,\
            rhi.user1                          , FLOAT_4, 1   , 147700,\
            rhi.user2                          , FLOAT_4, 1   , 147704,\
            rhi.user3                          , FLOAT_4, 1   , 147708,\
            rhi.user4                          , FLOAT_4, 1   , 147712,\
            rhi.user5                          , FLOAT_4, 1   , 147716,\
            rhi.user6                          , FLOAT_4, 1   , 147720,\
            rhi.user7                          , FLOAT_4, 1   , 147724,\
            rhi.user8                          , FLOAT_4, 1   , 147728,\
            rhi.user9                          , FLOAT_4, 1   , 147732,\
            rhi.user10                         , FLOAT_4, 1   , 147736,\
            rhi.user11                         , FLOAT_4, 1   , 147740,\
            rhi.user12                         , FLOAT_4, 1   , 147744,\
            rhi.user13                         , FLOAT_4, 1   , 147748,\
            rhi.user14                         , FLOAT_4, 1   , 147752,\
            rhi.user15                         , FLOAT_4, 1   , 147756,\
            rhi.user16                         , FLOAT_4, 1   , 147760,\
            rhi.user17                         , FLOAT_4, 1   , 147764,\
            rhi.user18                         , FLOAT_4, 1   , 147768,\
            rhi.user19                         , FLOAT_4, 1   , 147772,\
            rhi.user20                         , FLOAT_4, 1   , 147776,\
            rhi.user21                         , FLOAT_4, 1   , 147780,\
            rhi.user22                         , FLOAT_4, 1   , 147784,\
            rhi.user23                         , FLOAT_4, 1   , 147796,\
            rhi.user24                         , FLOAT_4, 1   , 147800,\
            rhi.user25                         , FLOAT_4, 1   , 147864,\
            rhi.user26                         , FLOAT_4, 1   , 147868,\
            rhi.user27                         , FLOAT_4, 1   , 147872,\
            rhi.user28                         , FLOAT_4, 1   , 147876,\
            rhi.user29                         , FLOAT_4, 1   , 147880,\
            rhi.user30                         , FLOAT_4, 1   , 147884,\
            rhi.user31                         , FLOAT_4, 1   , 147888,\
            rhi.user32                         , FLOAT_4, 1   , 147892,\
            rhi.user33                         , FLOAT_4, 1   , 147896,\
            rhi.user34                         , FLOAT_4, 1   , 147900,\
            rhi.user35                         , FLOAT_4, 1   , 147904,\
            rhi.user36                         , FLOAT_4, 1   , 147908,\
            rhi.user37                         , FLOAT_4, 1   , 147912,\
            rhi.user38                         , FLOAT_4, 1   , 147916,\
            rhi.user39                         , FLOAT_4, 1   , 147920,\
            rhi.user40                         , FLOAT_4, 1   , 147924,\
            rhi.user41                         , FLOAT_4, 1   , 147928,\
            rhi.user42                         , FLOAT_4, 1   , 147932,\
            rhi.user43                         , FLOAT_4, 1   , 147936,\
            rhi.user44                         , FLOAT_4, 1   , 147940,\
            rhi.user45                         , FLOAT_4, 1   , 147944,\
            rhi.user46                         , FLOAT_4, 1   , 147948,\
            rhi.user47                         , FLOAT_4, 1   , 147952,\
            rhi.user48                         , FLOAT_4, 1   , 147956,\
            rhi.cname                          , CHAR   , 17  , 149089,\
            rhi.brhc_A                         , FLOAT_4, 1   , 148088,\
            rhi.brhc_R                         , FLOAT_4, 1   , 148084,\
            rhi.brhc_S                         , FLOAT_4, 1   , 148092,\
            rhi.ctr_A                          , FLOAT_4, 1   , 148040,\
            rhi.ctr_R                          , FLOAT_4, 1   , 148036,\
            rhi.ctr_S                          , FLOAT_4, 1   , 148044,\
            rhi.dfov                           , FLOAT_4, 1   , 147644,\
            rhi.freq_dir                       , INT_2  , 1   , 148840,\
            rhi.ctyp                           , INT_2  , 1   , 148774,\
            rhi.loc                            , FLOAT_4, 1   , 147664,\
            rhi.mr_flip                        , INT_2  , 1   , 148752,\
            rhi.nex                            , FLOAT_4, 1   , 147672,\
            rhi.numecho                        , INT_2  , 1   , 148718,\
            rhi.image_uid                      , UID    , 32  , 149157,\
            rhi.rawrunnum                      , INT_4  , 1   , 148448,\
            rhe.ex_datetime                    , INT_4  , 1   , 143400,\
            rhe.ex_no                          , UINT_2 , 1   , 143516,\
            rhe.magstrength                    , INT_4  , 1   , 143392,\
            rhe.patid                          , CHAR   , 65  , 144409,\
            rhe.patname                        , CHAR   , 65  , 144344,\
            rhe.refphy                         , CHAR   , 65  , 143877,\
            rhe.reqnum                         , CHAR   , 17  , 144474,\
            rhe.study_uid                      , UID    , 32  , 144248,\
            rhe.dateofbirth                    , CHAR   , 9   , 144491,\
            rhe.patsex                         , INT_2  , 1   , 143540,\
            rhe.hospname                       , CHAR   , 33  , 144171,\
            rhe.ex_sysid                       , CHAR   , 9   , 144140,\
            rhe.uniq_sys_id                    , CHAR   , 16  , 144212,\
            rhe.ex_verscre                     , CHAR   , 2   , 144208,\
            rhs.se_no                          , INT_2  , 1   , 145622,\
            rhs.se_desc                        , CHAR   , 65  , 145762,\
            rhs.entry                          , INT_4  , 1   , 145424,\
            rhs.position                       , INT_4  , 1   , 145420,\
            rhs.series_uid                     , UID    , 32  , 145875,\
            rhs.landmark_uid                   , UID    , 32  , 145907,\
            rhs.anref                          , CHAR   , 3   , 145845,\
        "); 

    } else if ( (int)(this->pfileVersion) == 24 ) {

        offsets.assign (" \
            rhr.rh_rdbm_rev                    , FLOAT_4, 1   , 0,\
            rhr.rh_scan_date                   , CHAR   , 10  , 16,\
            rhr.rh_scan_time                   , CHAR   , 8   , 26,\
            rhr.rh_npasses                     , INT_2  , 1   , 64,\
            rhr.rh_nslices                     , UINT_2 , 1   , 68,\
            rhr.csi_dims                       , INT_2  , 1   , 372,\
            rhr.rh_dab[0].start_rcv            , INT_2  , 1   , 200,\
            rhr.rh_dab[0].stop_rcv             , INT_2  , 1   , 202,\
            rhr.rh_dab[1].start_rcv            , INT_2  , 1   , 204,\
            rhr.rh_dab[1].stop_rcv             , INT_2  , 1   , 206,\
            rhr.rh_dab[2].start_rcv            , INT_2  , 1   , 208,\
            rhr.rh_dab[2].stop_rcv             , INT_2  , 1   , 210,\
            rhr.rh_dab[3].start_rcv            , INT_2  , 1   , 212,\
            rhr.rh_dab[3].stop_rcv             , INT_2  , 1   , 214,\
            rhr.rh_data_collect_type           , INT_2  , 1   , 56,\
            rhr.rh_file_contents               , INT_2  , 1   , 44,\
            rhr.rdb_hdr_off_data               , INT_4  , 1   , 1468,\
            rhr.rh_frame_size                  , UINT_2 , 1   , 80,\
            rhr.rh_point_size                  , INT_2  , 1   , 82,\
            rhr.rh_ps_mps_freq                 , UINT_4 , 1   , 424,\
            rhr.rh_user_usage_tag              , UINT_4 , 1   , 988,\
            rhr.roilenx                        , FLOAT_4, 1   , 380,\
            rhr.roileny                        , FLOAT_4, 1   , 384,\
            rhr.roilenz                        , FLOAT_4, 1   , 388,\
            rhr.spectral_width                 , FLOAT_4, 1   , 368,\
            rhr.xcsi                           , INT_2  , 1   , 374,\
            rhr.ycsi                           , INT_2  , 1   , 376,\
            rhr.zcsi                           , INT_2  , 1   , 378,\
            rhr.rh_logo                        , CHAR   , 10  , 34,\
            rhr.rh_raw_pass_size               , LINT_8 , 1   , 1660,\
            rhr.rh_user0                       , FLOAT_4, 1   , 216,\
            rhr.rh_user1                       , FLOAT_4, 1   , 220,\
            rhr.rh_user2                       , FLOAT_4, 1   , 224,\
            rhr.rh_user3                       , FLOAT_4, 1   , 228,\
            rhr.rh_user4                       , FLOAT_4, 1   , 232,\
            rhr.rh_user5                       , FLOAT_4, 1   , 236,\
            rhr.rh_user6                       , FLOAT_4, 1   , 240,\
            rhr.rh_user7                       , FLOAT_4, 1   , 244,\
            rhr.rh_user8                       , FLOAT_4, 1   , 248,\
            rhr.rh_user9                       , FLOAT_4, 1   , 252,\
            rhr.rh_user10                      , FLOAT_4, 1   , 256,\
            rhr.rh_user11                      , FLOAT_4, 1   , 260,\
            rhr.rh_user12                      , FLOAT_4, 1   , 264,\
            rhr.rh_user13                      , FLOAT_4, 1   , 268,\
            rhr.rh_user14                      , FLOAT_4, 1   , 272,\
            rhr.rh_user15                      , FLOAT_4, 1   , 276,\
            rhr.rh_user16                      , FLOAT_4, 1   , 280,\
            rhr.rh_user17                      , FLOAT_4, 1   , 284,\
            rhr.rh_user18                      , FLOAT_4, 1   , 288,\
            rhr.rh_user19                      , FLOAT_4, 1   , 292,\
            rhr.rh_user20                      , FLOAT_4, 1   , 1000,\
            rhr.rh_user21                      , FLOAT_4, 1   , 1004,\
            rhr.rh_user22                      , FLOAT_4, 1   , 1008,\
            rhr.rh_user23                      , FLOAT_4, 1   , 1012,\
            rhr.rh_user24                      , FLOAT_4, 1   , 1016,\
            rhr.rh_user25                      , FLOAT_4, 1   , 1020,\
            rhr.rh_user26                      , FLOAT_4, 1   , 1024,\
            rhr.rh_user27                      , FLOAT_4, 1   , 1028,\
            rhr.rh_user28                      , FLOAT_4, 1   , 1032,\
            rhr.rh_user29                      , FLOAT_4, 1   , 1036,\
            rhr.rh_user30                      , FLOAT_4, 1   , 1040,\
            rhr.rh_user31                      , FLOAT_4, 1   , 1044,\
            rhr.rh_user32                      , FLOAT_4, 1   , 1048,\
            rhr.rh_user33                      , FLOAT_4, 1   , 1052,\
            rhr.rh_user34                      , FLOAT_4, 1   , 1056,\
            rhr.rh_user35                      , FLOAT_4, 1   , 1060,\
            rhr.rh_user36                      , FLOAT_4, 1   , 1064,\
            rhr.rh_user37                      , FLOAT_4, 1   , 1068,\
            rhr.rh_user38                      , FLOAT_4, 1   , 1072,\
            rhr.rh_user39                      , FLOAT_4, 1   , 1076,\
            rhr.rh_user40                      , FLOAT_4, 1   , 1080,\
            rhr.rh_user41                      , FLOAT_4, 1   , 1084,\
            rhr.rh_user42                      , FLOAT_4, 1   , 1088,\
            rhr.rh_user43                      , FLOAT_4, 1   , 1092,\
            rhr.rh_user44                      , FLOAT_4, 1   , 1096,\
            rhr.rh_user45                      , FLOAT_4, 1   , 1100,\
            rhr.rh_user46                      , FLOAT_4, 1   , 1104,\
            rhr.rh_user47                      , FLOAT_4, 1   , 1108,\
            rhr.rh_user48                      , FLOAT_4, 1   , 1112,\
            rhi.psdname                        , CHAR   , 33  , 148972,\
            rhi.scanspacing                    , FLOAT_4, 1   , 147660,\
            rhi.te                             , INT_4  , 1   , 148404,\
            rhi.ti                             , INT_4  , 1   , 148400,\
            rhi.tr                             , INT_4  , 1   , 148396,\
            rhi.tlhc_A                         , FLOAT_4, 1   , 148064,\
            rhi.tlhc_R                         , FLOAT_4, 1   , 148060,\
            rhi.tlhc_S                         , FLOAT_4, 1   , 148068,\
            rhi.t                              , INT_4  , 1   , 148396,\
            rhi.trhc_A                         , FLOAT_4, 1   , 148076,\
            rhi.trhc_R                         , FLOAT_4, 1   , 148072,\
            rhi.trhc_S                         , FLOAT_4, 1   , 148080,\
            rhi.user0                          , FLOAT_4, 1   , 147696,\
            rhi.user1                          , FLOAT_4, 1   , 147700,\
            rhi.user2                          , FLOAT_4, 1   , 147704,\
            rhi.user3                          , FLOAT_4, 1   , 147708,\
            rhi.user4                          , FLOAT_4, 1   , 147712,\
            rhi.user5                          , FLOAT_4, 1   , 147716,\
            rhi.user6                          , FLOAT_4, 1   , 147720,\
            rhi.user7                          , FLOAT_4, 1   , 147724,\
            rhi.user8                          , FLOAT_4, 1   , 147728,\
            rhi.user9                          , FLOAT_4, 1   , 147732,\
            rhi.user10                         , FLOAT_4, 1   , 147736,\
            rhi.user11                         , FLOAT_4, 1   , 147740,\
            rhi.user12                         , FLOAT_4, 1   , 147744,\
            rhi.user13                         , FLOAT_4, 1   , 147748,\
            rhi.user14                         , FLOAT_4, 1   , 147752,\
            rhi.user15                         , FLOAT_4, 1   , 147756,\
            rhi.user16                         , FLOAT_4, 1   , 147760,\
            rhi.user17                         , FLOAT_4, 1   , 147764,\
            rhi.user18                         , FLOAT_4, 1   , 147768,\
            rhi.user19                         , FLOAT_4, 1   , 147772,\
            rhi.user20                         , FLOAT_4, 1   , 147776,\
            rhi.user21                         , FLOAT_4, 1   , 147780,\
            rhi.user22                         , FLOAT_4, 1   , 147784,\
            rhi.user23                         , FLOAT_4, 1   , 147796,\
            rhi.user24                         , FLOAT_4, 1   , 147800,\
            rhi.user25                         , FLOAT_4, 1   , 147864,\
            rhi.user26                         , FLOAT_4, 1   , 147868,\
            rhi.user27                         , FLOAT_4, 1   , 147872,\
            rhi.user28                         , FLOAT_4, 1   , 147876,\
            rhi.user29                         , FLOAT_4, 1   , 147880,\
            rhi.user30                         , FLOAT_4, 1   , 147884,\
            rhi.user31                         , FLOAT_4, 1   , 147888,\
            rhi.user32                         , FLOAT_4, 1   , 147892,\
            rhi.user33                         , FLOAT_4, 1   , 147896,\
            rhi.user34                         , FLOAT_4, 1   , 147900,\
            rhi.user35                         , FLOAT_4, 1   , 147904,\
            rhi.user36                         , FLOAT_4, 1   , 147908,\
            rhi.user37                         , FLOAT_4, 1   , 147912,\
            rhi.user38                         , FLOAT_4, 1   , 147916,\
            rhi.user39                         , FLOAT_4, 1   , 147920,\
            rhi.user40                         , FLOAT_4, 1   , 147924,\
            rhi.user41                         , FLOAT_4, 1   , 147928,\
            rhi.user42                         , FLOAT_4, 1   , 147932,\
            rhi.user43                         , FLOAT_4, 1   , 147936,\
            rhi.user44                         , FLOAT_4, 1   , 147940,\
            rhi.user45                         , FLOAT_4, 1   , 147944,\
            rhi.user46                         , FLOAT_4, 1   , 147948,\
            rhi.user47                         , FLOAT_4, 1   , 147952,\
            rhi.user48                         , FLOAT_4, 1   , 147956,\
            rhi.cname                          , CHAR   , 17  , 149089,\
            rhi.brhc_A                         , FLOAT_4, 1   , 148088,\
            rhi.brhc_R                         , FLOAT_4, 1   , 148084,\
            rhi.brhc_S                         , FLOAT_4, 1   , 148092,\
            rhi.ctr_A                          , FLOAT_4, 1   , 148040,\
            rhi.ctr_R                          , FLOAT_4, 1   , 148036,\
            rhi.ctr_S                          , FLOAT_4, 1   , 148044,\
            rhi.dfov                           , FLOAT_4, 1   , 147644,\
            rhi.freq_dir                       , INT_2  , 1   , 148840,\
            rhi.ctyp                           , INT_2  , 1   , 148774,\
            rhi.loc                            , FLOAT_4, 1   , 147664,\
            rhi.mr_flip                        , INT_2  , 1   , 148752,\
            rhi.nex                            , FLOAT_4, 1   , 147672,\
            rhi.numecho                        , INT_2  , 1   , 148718,\
            rhi.image_uid                      , UID    , 32  , 149157,\
            rhi.rawrunnum                      , INT_4  , 1   , 148448,\
            rhe.ex_datetime                    , INT_4  , 1   , 143400,\
            rhe.ex_no                          , UINT_2 , 1   , 143516,\
            rhe.magstrength                    , INT_4  , 1   , 143392,\
            rhe.patid                          , CHAR   , 65  , 144409,\
            rhe.patname                        , CHAR   , 65  , 144344,\
            rhe.refphy                         , CHAR   , 65  , 143877,\
            rhe.reqnum                         , CHAR   , 17  , 144474,\
            rhe.study_uid                      , UID    , 32  , 144248,\
            rhe.dateofbirth                    , CHAR   , 9   , 144491,\
            rhe.patsex                         , INT_2  , 1   , 143540,\
            rhe.hospname                       , CHAR   , 33  , 144171,\
            rhe.ex_sysid                       , CHAR   , 9   , 144140,\
            rhe.uniq_sys_id                    , CHAR   , 16  , 144212,\
            rhe.ex_verscre                     , CHAR   , 2   , 144208,\
            rhs.se_no                          , INT_2  , 1   , 145622,\
            rhs.se_desc                        , CHAR   , 65  , 145762,\
            rhs.entry                          , INT_4  , 1   , 145424,\
            rhs.position                       , INT_4  , 1   , 145420,\
            rhs.series_uid                     , UID    , 32  , 145875,\
            rhs.landmark_uid                   , UID    , 32  , 145907,\
            rhs.anref                          , CHAR   , 3   , 145845,\
        "); 

    } else if ( (int)(this->pfileVersion) == 26 ) {

        offsets.assign (" \
            rhr.rh_rdbm_rev                    , FLOAT_4, 1   , 0,\
            rhr.rh_scan_date                   , CHAR   , 10  , 92,\
            rhr.rh_scan_time                   , CHAR   , 8   , 102,\
            rhr.rh_npasses                     , INT_2  , 1   , 140,\
            rhr.rh_nslices                     , UINT_2 , 1   , 144,\
            rhr.csi_dims                       , INT_2  , 1   , 436,\
            rhr.rh_dab[0].start_rcv            , INT_2  , 1   , 264,\
            rhr.rh_dab[0].stop_rcv             , INT_2  , 1   , 266,\
            rhr.rh_dab[1].start_rcv            , INT_2  , 1   , 268,\
            rhr.rh_dab[1].stop_rcv             , INT_2  , 1   , 270,\
            rhr.rh_dab[2].start_rcv            , INT_2  , 1   , 272,\
            rhr.rh_dab[2].stop_rcv             , INT_2  , 1   , 274,\
            rhr.rh_dab[3].start_rcv            , INT_2  , 1   , 276,\
            rhr.rh_dab[3].stop_rcv             , INT_2  , 1   , 278,\
            rhr.rh_data_collect_type           , INT_2  , 1   , 132,\
            rhr.rh_file_contents               , INT_2  , 1   , 120,\
            rhr.rdb_hdr_off_data               , INT_4  , 1   , 4,\
            rhr.rh_frame_size                  , UINT_2 , 1   , 156,\
            rhr.rh_point_size                  , INT_2  , 1   , 158,\
            rhr.rh_ps_mps_freq                 , UINT_4 , 1   , 488,\
            rhr.rh_user_usage_tag              , UINT_4 , 1   , 924,\
            rhr.roilenx                        , FLOAT_4, 1   , 444,\
            rhr.roileny                        , FLOAT_4, 1   , 448,\
            rhr.roilenz                        , FLOAT_4, 1   , 452,\
            rhr.spectral_width                 , FLOAT_4, 1   , 432,\
            rhr.xcsi                           , INT_2  , 1   , 438,\
            rhr.ycsi                           , INT_2  , 1   , 440,\
            rhr.zcsi                           , INT_2  , 1   , 442,\
            rhr.rh_logo                        , CHAR   , 10  , 110,\
            rhr.rh_raw_pass_size               , LINT_8 , 1   , 1540,\
            rhr.rh_user0                       , FLOAT_4, 1   , 280,\
            rhr.rh_user1                       , FLOAT_4, 1   , 284,\
            rhr.rh_user2                       , FLOAT_4, 1   , 288,\
            rhr.rh_user3                       , FLOAT_4, 1   , 292,\
            rhr.rh_user4                       , FLOAT_4, 1   , 296,\
            rhr.rh_user5                       , FLOAT_4, 1   , 300,\
            rhr.rh_user6                       , FLOAT_4, 1   , 304,\
            rhr.rh_user7                       , FLOAT_4, 1   , 308,\
            rhr.rh_user8                       , FLOAT_4, 1   , 312,\
            rhr.rh_user9                       , FLOAT_4, 1   , 316,\
            rhr.rh_user10                      , FLOAT_4, 1   , 320,\
            rhr.rh_user11                      , FLOAT_4, 1   , 324,\
            rhr.rh_user12                      , FLOAT_4, 1   , 328,\
            rhr.rh_user13                      , FLOAT_4, 1   , 332,\
            rhr.rh_user14                      , FLOAT_4, 1   , 336,\
            rhr.rh_user15                      , FLOAT_4, 1   , 340,\
            rhr.rh_user16                      , FLOAT_4, 1   , 344,\
            rhr.rh_user17                      , FLOAT_4, 1   , 348,\
            rhr.rh_user18                      , FLOAT_4, 1   , 352,\
            rhr.rh_user19                      , FLOAT_4, 1   , 356,\
            rhr.rh_user20                      , FLOAT_4, 1   , 936,\
            rhr.rh_user21                      , FLOAT_4, 1   , 940,\
            rhr.rh_user22                      , FLOAT_4, 1   , 944,\
            rhr.rh_user23                      , FLOAT_4, 1   , 948,\
            rhr.rh_user24                      , FLOAT_4, 1   , 952,\
            rhr.rh_user25                      , FLOAT_4, 1   , 956,\
            rhr.rh_user26                      , FLOAT_4, 1   , 960,\
            rhr.rh_user27                      , FLOAT_4, 1   , 964,\
            rhr.rh_user28                      , FLOAT_4, 1   , 968,\
            rhr.rh_user29                      , FLOAT_4, 1   , 972,\
            rhr.rh_user30                      , FLOAT_4, 1   , 976,\
            rhr.rh_user31                      , FLOAT_4, 1   , 980,\
            rhr.rh_user32                      , FLOAT_4, 1   , 984,\
            rhr.rh_user33                      , FLOAT_4, 1   , 988,\
            rhr.rh_user34                      , FLOAT_4, 1   , 992,\
            rhr.rh_user35                      , FLOAT_4, 1   , 996,\
            rhr.rh_user36                      , FLOAT_4, 1   , 1000,\
            rhr.rh_user37                      , FLOAT_4, 1   , 1004,\
            rhr.rh_user38                      , FLOAT_4, 1   , 1008,\
            rhr.rh_user39                      , FLOAT_4, 1   , 1012,\
            rhr.rh_user40                      , FLOAT_4, 1   , 1016,\
            rhr.rh_user41                      , FLOAT_4, 1   , 1020,\
            rhr.rh_user42                      , FLOAT_4, 1   , 1024,\
            rhr.rh_user43                      , FLOAT_4, 1   , 1028,\
            rhr.rh_user44                      , FLOAT_4, 1   , 1032,\
            rhr.rh_user45                      , FLOAT_4, 1   , 1036,\
            rhr.rh_user46                      , FLOAT_4, 1   , 1040,\
            rhr.rh_user47                      , FLOAT_4, 1   , 1044,\
            rhr.rh_user48                      , FLOAT_4, 1   , 1048,\
            rhi.psdname                        , CHAR   , 33  , 199812,\
            rhi.scanspacing                    , FLOAT_4, 1   , 198500,\
            rhi.te                             , INT_4  , 1   , 199244,\
            rhi.ti                             , INT_4  , 1   , 199240,\
            rhi.tr                             , INT_4  , 1   , 199236,\
            rhi.tlhc_A                         , FLOAT_4, 1   , 198904,\
            rhi.tlhc_R                         , FLOAT_4, 1   , 198900,\
            rhi.tlhc_S                         , FLOAT_4, 1   , 198908,\
            rhi.t                              , INT_4  , 1   , 199236,\
            rhi.trhc_A                         , FLOAT_4, 1   , 198916,\
            rhi.trhc_R                         , FLOAT_4, 1   , 198912,\
            rhi.trhc_S                         , FLOAT_4, 1   , 198920,\
            rhi.user0                          , FLOAT_4, 1   , 198536,\
            rhi.user1                          , FLOAT_4, 1   , 198540,\
            rhi.user2                          , FLOAT_4, 1   , 198544,\
            rhi.user3                          , FLOAT_4, 1   , 198548,\
            rhi.user4                          , FLOAT_4, 1   , 198552,\
            rhi.user5                          , FLOAT_4, 1   , 198556,\
            rhi.user6                          , FLOAT_4, 1   , 198560,\
            rhi.user7                          , FLOAT_4, 1   , 198564,\
            rhi.user8                          , FLOAT_4, 1   , 198568,\
            rhi.user9                          , FLOAT_4, 1   , 198572,\
            rhi.user10                         , FLOAT_4, 1   , 198576,\
            rhi.user11                         , FLOAT_4, 1   , 198580,\
            rhi.user12                         , FLOAT_4, 1   , 198584,\
            rhi.user13                         , FLOAT_4, 1   , 198588,\
            rhi.user14                         , FLOAT_4, 1   , 198592,\
            rhi.user15                         , FLOAT_4, 1   , 198596,\
            rhi.user16                         , FLOAT_4, 1   , 198600,\
            rhi.user17                         , FLOAT_4, 1   , 198604,\
            rhi.user18                         , FLOAT_4, 1   , 198608,\
            rhi.user19                         , FLOAT_4, 1   , 198612,\
            rhi.user20                         , FLOAT_4, 1   , 198616,\
            rhi.user21                         , FLOAT_4, 1   , 198620,\
            rhi.user22                         , FLOAT_4, 1   , 198624,\
            rhi.user23                         , FLOAT_4, 1   , 198636,\
            rhi.user24                         , FLOAT_4, 1   , 198640,\
            rhi.user25                         , FLOAT_4, 1   , 198704,\
            rhi.user26                         , FLOAT_4, 1   , 198708,\
            rhi.user27                         , FLOAT_4, 1   , 198712,\
            rhi.user28                         , FLOAT_4, 1   , 198716,\
            rhi.user29                         , FLOAT_4, 1   , 198720,\
            rhi.user30                         , FLOAT_4, 1   , 198724,\
            rhi.user31                         , FLOAT_4, 1   , 198728,\
            rhi.user32                         , FLOAT_4, 1   , 198732,\
            rhi.user33                         , FLOAT_4, 1   , 198736,\
            rhi.user34                         , FLOAT_4, 1   , 198740,\
            rhi.user35                         , FLOAT_4, 1   , 198744,\
            rhi.user36                         , FLOAT_4, 1   , 198748,\
            rhi.user37                         , FLOAT_4, 1   , 198752,\
            rhi.user38                         , FLOAT_4, 1   , 198756,\
            rhi.user39                         , FLOAT_4, 1   , 198760,\
            rhi.user40                         , FLOAT_4, 1   , 198764,\
            rhi.user41                         , FLOAT_4, 1   , 198768,\
            rhi.user42                         , FLOAT_4, 1   , 198772,\
            rhi.user43                         , FLOAT_4, 1   , 198776,\
            rhi.user44                         , FLOAT_4, 1   , 198780,\
            rhi.user45                         , FLOAT_4, 1   , 198784,\
            rhi.user46                         , FLOAT_4, 1   , 198788,\
            rhi.user47                         , FLOAT_4, 1   , 198792,\
            rhi.user48                         , FLOAT_4, 1   , 198796,\
            rhi.cname                          , CHAR   , 17  , 199929,\
            rhi.brhc_A                         , FLOAT_4, 1   , 198928,\
            rhi.brhc_R                         , FLOAT_4, 1   , 198924,\
            rhi.brhc_S                         , FLOAT_4, 1   , 198932,\
            rhi.ctr_A                          , FLOAT_4, 1   , 198880,\
            rhi.ctr_R                          , FLOAT_4, 1   , 198876,\
            rhi.ctr_S                          , FLOAT_4, 1   , 198884,\
            rhi.dfov                           , FLOAT_4, 1   , 198484,\
            rhi.freq_dir                       , INT_2  , 1   , 199680,\
            rhi.ctyp                           , INT_2  , 1   , 199614,\
            rhi.loc                            , FLOAT_4, 1   , 198504,\
            rhi.mr_flip                        , INT_2  , 1   , 199592,\
            rhi.nex                            , FLOAT_4, 1   , 198512,\
            rhi.numecho                        , INT_2  , 1   , 199558,\
            rhi.image_uid                      , UID    , 32  , 199997,\
            rhi.rawrunnum                      , INT_4  , 1   , 199288,\
            rhe.ex_datetime                    , INT_4  , 1   , 194240,\
            rhe.ex_no                          , UINT_2 , 1   , 194356,\
            rhe.magstrength                    , INT_4  , 1   , 194232,\
            rhe.patid                          , CHAR   , 65  , 195249,\
            rhe.patname                        , CHAR   , 65  , 195184,\
            rhe.refphy                         , CHAR   , 65  , 194717,\
            rhe.reqnum                         , CHAR   , 17  , 195314,\
            rhe.study_uid                      , UID    , 32  , 195088,\
            rhe.dateofbirth                    , CHAR   , 9   , 195331,\
            rhe.patsex                         , INT_2  , 1   , 194380,\
            rhe.hospname                       , CHAR   , 33  , 195011,\
            rhe.ex_sysid                       , CHAR   , 9   , 194980,\
            rhe.uniq_sys_id                    , CHAR   , 16  , 195052,\
            rhe.ex_verscre                     , CHAR   , 2   , 195048,\
            rhs.se_no                          , INT_4  , 1   , 196356,\
            rhs.se_desc                        , CHAR   , 65  , 196602,\
            rhs.entry                          , INT_4  , 1   , 196264,\
            rhs.position                       , INT_4  , 1   , 196260,\
            rhs.series_uid                     , UID    , 32  , 196765,\
            rhs.landmark_uid                   , UID    , 32  , 196797,\
            rhs.anref                          , CHAR   , 3   , 196685,\
            rhr.rh_da_xres                     , UINT_2 , 1   , 178,\
            rhr.rh_da_yres                     , INT_2  , 1   , 180,\
            rhr.rh_rc_xres                     , INT_2  , 1   , 182,\
            rhr.rh_rc_yres                     , INT_2  , 1   , 184,\
            rhr.rh_nframes                     , INT_2  , 1   , 150,\
            rhr.rh_im_size                     , INT_2  , 1   , 186,\
            rhr.rh_te                          , INT_4  , 1   , 1148,\
            rhr.rh_te2                         , INT_4  , 1   , 1152,\
            rhr.rh_scalei                      , FLOAT_4, 1   , 232,\
            rhr.rh_phase_scale                 , FLOAT_4, 1   , 804,\
            rhr.rh_slblank                     , INT_2  , 1   , 246,\
            rhr.rh_ileaves                     , INT_2  , 1   , 850,\
            rhr.rh_navs                        , INT_2  , 1   , 148,\
            rhr.rh_nechoes                     , INT_2  , 1   , 146,\
            rhr.rh_fov                         , FLOAT_4, 1   , 1144,\
            rhr.rh_scancent                    , FLOAT_4, 1   , 1268,\
            rhr.rh_recon_ctrl                  , UINT_2 , 1   , 126,\
            rhr.rh_exec_ctrl                   , UINT_2 , 1   , 128,\
            rhr.rh_dacq_ctrl                   , INT_2  , 1   , 124,\
            rhr.rh_ovl                         , INT_2  , 1   , 808,\
            rhs.se_exno                        , UINT_2 , 1   , 196428,\
            rhs.start_loc                      , FLOAT_4, 1   , 195892,\
            rhs.end_loc                        , FLOAT_4, 1   , 195896,\
            rhi.imode                          , INT_2  , 1   , 199608,\
            rhi.imatrix_X                      , INT_2  , 1   , 199548,\
            rhi.imatrix_Y                      , INT_2  , 1   , 199550,\
            rhi.fphase                         , INT_4  , 1   , 199348,\
            rhi.plane                          , INT_2  , 1   , 199572,\
            rhi.dim_X                          , FLOAT_4, 1   , 198860,\
            rhi.dim_Y                          , FLOAT_4, 1   , 198864,\
            rhi.slthick                        , FLOAT_4, 1   , 198496,\
            rhi.numslabs                       , INT_4  , 1   , 199328,\
            rhi.locsperslab                    , INT_4  , 1   , 199332,\
            rhi.norm_R                         , FLOAT_4, 1   , 198888,\
            rhi.norm_A                         , FLOAT_4, 1   , 198892,\
            rhi.norm_S                         , FLOAT_4, 1   , 198896,\
            rhi.dfov_rect                      , FLOAT_4, 1   , 198488,\
            rhr.rh_ps_mps_r1                   , INT_4  , 1   , 476,\
            rhr.rh_ps_mps_r2                   , INT_4  , 1   , 480,\
            rhr.rh_ps_mps_tg                   , INT_4  , 1   , 484,\
            rhr.rh_ps_aps_r1                   , INT_4  , 1   , 492,\
            rhr.rh_ps_aps_r2                   , INT_4  , 1   , 496,\
            rhr.rh_ps_aps_tg                   , INT_4  , 1   , 500,\
            rhr.rh_ps_aps_freq                 , UINT_4 , 1   , 504,\
        "); 

    }

    return offsets; 

}


/*
 *
 */
map <string, vector< string > > 
svkGEPFileReader::GetPFMap() 
{
    return this->pfMap; 
}


/*
 *  Get the number of elements in the header field for the specified key:
 */
int svkGEPFileReader::GetNumElementsInField( string key )
{
    int numElements =svkTypeUtils::StringToInt( this->pfMap[key][1] ); 
    return numElements; 
}


/*
 *  Get the number of bytes in the header field for the specified key:
 */
int svkGEPFileReader::GetNumBytesInField( string key )
{

    int numElements = this->GetNumElementsInField( key );
    string type = this->StripWhite( this->pfMap[ key ][0] );

    int numBytes = 0;     

    if ( type.compare("FLOAT_4") == 0) {
        numBytes = numElements * 4; 
    } else if ( type.compare("INT_2") == 0 ) {
        numBytes = numElements * 2; 
    } else if ( type.compare("UINT_2") == 0 ) {
        numBytes = numElements * 2; 
    } else if ( type.compare("UINT_4") == 0 ) {
        numBytes = numElements * 4; 
    } else if ( type.compare("INT_4") == 0) {
        numBytes = numElements * 4; 
    } else if ( type.compare("LINT_4") == 0) {
        numBytes = numElements * 4; 
    } else if ( type.compare("ULINT_4") == 0) {
        numBytes = numElements * 4; 
    } else if ( type.compare("LINT_8") == 0) {
        numBytes = numElements * 8; 
    } else if ( type.compare("CHAR") == 0) {
        numBytes = numElements * 1; 
    } else if ( type.compare("UID") == 0) {
        numBytes = numElements * 1; 
    }

    return numBytes; 
}


/*
 *  Return true if the raw field is of type char. 
 */
bool svkGEPFileReader::IsFieldChar( string key )
{
    string type = this->StripWhite( this->pfMap[ key ][0] );

    if ( type.compare("CHAR") == 0) {
        return true; 
    } else {
        return false; 
    }
}


/*
 *  Return true if the raw field is of type char. 
 */
bool svkGEPFileReader::IsFieldUID( string key )
{
    string type = this->StripWhite( this->pfMap[ key ][0] );

    if ( type.compare("UID") == 0) {
        return true; 
    } else {
        return false; 
    }
}


/*
 *  Return true if the raw field is of type float 4.
 */
bool svkGEPFileReader::IsFieldFloat4( string key )
{
    string type = this->StripWhite( this->pfMap[ key ][0] );

    if ( type.compare("FLOAT_4") == 0) {
        return true;
    } else {
        return false;
    }
}


/*
 *  Return true if the raw field is of type int 2.
 */
bool svkGEPFileReader::IsFieldInt2( string key )
{
    string type = this->StripWhite( this->pfMap[ key ][0] );

    if ( type.compare("INT_2") == 0) {
        return true;
    } else {
        return false;
    }
}


/*
 *  Return true if the raw field is of type int 4.
 */
bool svkGEPFileReader::IsFieldInt4( string key )
{
    string type = this->StripWhite( this->pfMap[ key ][0] );

    if ( type.compare("INT_4") == 0) {
        return true;
    } else {
        return false;
    }
}


/*
 *  Return true if the raw field is of type long int .
 */
bool svkGEPFileReader::IsFieldLInt4( string key )
{
    string type = this->StripWhite( this->pfMap[ key ][0] );

    if ( type.compare("LINT_4") == 0) {
        return true;
    } else {
        return false;
    }
}


/*
 *  Return true if the raw field is of type long int .
 */
bool svkGEPFileReader::IsFieldLInt8( string key )
{
    string type = this->StripWhite( this->pfMap[ key ][0] );

    if ( type.compare("LINT_8") == 0) {
        return true;
    } else {
        return false;
    }
}


/*
 *  In place deidentification of field
 */
void svkGEPFileReader::DeidentifyField( fstream* fs, string key, string deidString)
{
    int offset;
    int numBytes;

    //  Check if key exists.  If not, just return; 
    if ( this->pfMap.find(key) == pfMap.end() ) {
        cout <<"Can't find element: " << key << endl; 
        return; 
    }

    numBytes    = this->GetNumBytesInField( key ); 
    offset      = svkTypeUtils::StringToInt( this->pfMap[key][2] ); 

    //  Replace numBytes at offset position with the deidString
    //  set the put pointer to the correct offset
    if ( fs->is_open() ) {

        if ( this->IsFieldChar( key ) ) {

            //  =======================================    
            //  CHAR
            //  =======================================    
            cout << "replace char bytes with " << key << " -> " << deidString << " " << numBytes << endl; 
            fs->seekp( offset, ios_base::beg );
            fs->write( deidString.c_str(), numBytes);

        } else if ( this->IsFieldFloat4( key ) ) {

            //  =======================================
            //  Float 4
            //  =======================================
            float value = svkTypeUtils::StringToFloat(deidString);
            if( this->GetSwapBytes() ) {
				vtkByteSwap::SwapVoidRange((void *)&value, 1, sizeof(float));
            }
            cout << "replace float bytes with " << key << " -> " << deidString << " " << numBytes << endl; 
            fs->seekp( offset, ios_base::beg );
            fs->write( (char*)(&value), numBytes);

        } else if ( this->IsFieldInt2( key ) ) {

            //  =======================================
            //  Int 2
            //  =======================================
            int value = svkTypeUtils::StringToInt(deidString);
            if( this->GetSwapBytes() ) {
				vtkByteSwap::SwapVoidRange((void *)&value, 1, numBytes); 
            }
            cout << "replace short int bytes with " << key << " -> " << deidString << " " << numBytes << endl; 
            fs->seekp( offset, ios_base::beg );
            fs->write( (char*)(&value), numBytes);

        } else if ( this->IsFieldInt4( key ) ) {

            //  =======================================
            //  Int 4
            //  =======================================
            int value = svkTypeUtils::StringToInt(deidString);
            if( this->GetSwapBytes() ) {
				vtkByteSwap::SwapVoidRange((void *)&value, 1, sizeof(int));
            }
            cout << "replace int bytes with " << key << " -> " << deidString << " " << numBytes << endl; 
            fs->seekp( offset, ios_base::beg );
            fs->write( (char*)(&value), numBytes);

        } else if ( this->IsFieldLInt4( key ) ) {

            //  =======================================
            //  Long Int 4
            //  =======================================
            int value = svkTypeUtils::StringToInt(deidString);
            if( this->GetSwapBytes() ) {
				vtkByteSwap::SwapVoidRange((void *)&value, 1, numBytes); 
            }
            cout << "replace int bytes with " << key << " -> " << deidString << " " << numBytes << endl; 
            fs->seekp( offset, ios_base::beg );
            fs->write( (char*)(&value), numBytes);

        } else if ( this->IsFieldLInt8( key ) ) {

            //  =======================================
            //  Long Int 8
            //  =======================================
            long int value = svkTypeUtils::StringToLInt(deidString);
            if( this->GetSwapBytes() ) {
				vtkByteSwap::SwapVoidRange((void *)&value, 1, numBytes);
            }
            cout << "replace int bytes with " << key << " -> " << deidString << " " << numBytes << endl; 
            fs->seekp( offset, ios_base::beg );
            fs->write( (char*)(&value), numBytes);

        } else if ( this->IsFieldUID( key ) ) {
            
            //  =======================================    
            //  UID 
            //  UID fields are DICOM VR=UI and type 1, 
            //  so they must have values and the values must be valid.  
            //  therefore, if the deidString isn't a valid UID, we should
            //  generate one here. 
            //  =======================================    
            cout << "replace UID with " << key << " -> " << deidString <<  " nb: " << numBytes << endl; 
            string compressedUID = this->CompressUID( (char*)deidString.c_str() );  
            //cout << "CHECK UID: " <<  this->UncompressUID( compressedUID.c_str() ) << endl;;  
            fs->seekp( offset, ios_base::beg );
            fs->write( compressedUID.c_str(), numBytes);

        } else {

            // other data types need to be replace with the appropriate data type:  
            cout << "WARNING: Unsupported field change: " << key  << endl; 

        }
    }


}


/* 
 *  In place modification of specified raw file field.  
 */
void svkGEPFileReader::ModifyRawField( string rawField, string value)
{

    this->OnlyReadHeader(true); 
    this->ReadGEPFile(); 

    //  Now the pfMap has been initialized as follows.  Use the 
    //  Offsets and field sizes to replace the appropriate bytes with 
    //  the deidentification ID:
    //  key        pfMap[key][0]    pfMap[key][1] pfMap[key][2] pfMap[key][3]
    //  ----------------------------------------------------------------------
    //  fieldName  nativeType       numElements   offset        stringValue

    for ( int fileNumber = 0; fileNumber < this->GetFileNames()->GetNumberOfValues(); fileNumber++ ) {      	 	 
        fstream* fs = new fstream();
        fs->exceptions( fstream::eofbit | fstream::failbit | fstream::badbit );
        fs->open( this->GetFileNames()->GetValue(fileNumber), ios::binary | ios::in | ios::out );

        if ( fs->is_open() ) {
            this->DeidentifyField( fs, rawField, value);
            fs->close();
        } else {
            cout << "ERROR: Could not open raw file for modification: " << this->GetFileNames()->GetValue(0) << endl;
            exit(1);
        }

        delete fs; 
    }

}    


/*!
 *  Set the StudyInstanceUID to be used for deidentification:  
 */
void svkGEPFileReader::SetDeidentificationStudyUID(string deidStudyUID) 
{
    this->deidStudyUID.assign(deidStudyUID); 
}

/*!
 *  Set the SeriesInstanceUID to be used for deidentification:  
 */
void svkGEPFileReader::SetDeidentificationSeriesUID(string deidSeriesUID) 
{
    this->deidSeriesUID.assign(deidSeriesUID); 
}

/*!
 *  Set the Image InstanceUID to be used for deidentification:  
 */
void svkGEPFileReader::SetDeidentificationInstanceUID(string deidImageUID) 
{
    this->deidImageUID.assign(deidSeriesUID); 
}

/*!
 *  Set the Landmark UID to be used for deidentification:  
 */
void svkGEPFileReader::SetDeidentificationLandmarkUID(string deidLandmarkUID) 
{
    this->deidLandmarkUID.assign(deidLandmarkUID); 
}


/* 
 *  In place deidentification of raw file with study ID:
 *  By default this will generate new unique UIDs for the object, unless
 *  values have been specified in SetDeidentification*UID methods.
 */ 
void svkGEPFileReader::Deidentify()
{

    this->OnlyReadHeader(true); 
    this->ReadGEPFile(); 

    //  If ID isn't specified, then use default here: 
    if ( this->deidPatientId.compare(UNASSIGNED_ID) == 0 ) {
        this->deidPatientId = "DEIDENTIFIED"; 
    } 
    if ( this->deidStudyId.compare(UNASSIGNED_ID) == 0 ) {
        this->deidStudyId = "DEIDENTIFIED"; 
    } 

    //  If UIDs aren't specified, then generate them here: 
    if ( this->deidStudyUID.compare(UNASSIGNED_UID) == 0 ) {
        this->deidStudyUID = this->GetOutput()->GetDcmHeader()->GenerateUniqueUID();
    } 

    if ( this->deidSeriesUID.compare(UNASSIGNED_UID) == 0 ) {
        this->deidSeriesUID = this->GetOutput()->GetDcmHeader()->GenerateUniqueUID();
    } 

    if ( this->deidImageUID.compare(UNASSIGNED_UID) == 0 ) {
        this->deidImageUID = this->GetOutput()->GetDcmHeader()->GenerateUniqueUID();
    } 

    if ( this->deidLandmarkUID.compare(UNASSIGNED_UID) == 0 ) {
        this->deidLandmarkUID = this->GetOutput()->GetDcmHeader()->GenerateUniqueUID();
    } 

    //  Now the pfMap has been initialized as follows.  Use the 
    //  Offsets and field sizes to replace the appropriate bytes with 
    //  the deidentification ID:
    //  key        pfMap[key][0]    pfMap[key][1] pfMap[key][2] pfMap[key][3]
    //  ----------------------------------------------------------------------
    //  fieldName  nativeType       numElements   offset        stringValue

    for ( int fileNumber = 0; fileNumber < this->GetFileNames()->GetNumberOfValues(); fileNumber++ ) {      	 	 
        fstream* fs = new fstream();
        fs->exceptions( fstream::eofbit | fstream::failbit | fstream::badbit );
        fs->open( this->GetFileNames()->GetValue(fileNumber), ios::binary | ios::in | ios::out );

        if ( fs->is_open() ) {
            //  These fields are removed from PHI_LIMITED and PHI_DEIDENTIFIED data sets:
            this->DeidentifyField( fs, "rhe.refphy",        this->deidStudyId);
            this->DeidentifyField( fs, "rhe.ex_no",         this->deidStudyId);
            this->DeidentifyField( fs, "rhe.reqnum",        this->deidStudyId);
            this->DeidentifyField( fs, "rhe.reqnumff",      this->deidStudyId);
            this->DeidentifyField( fs, "rhe.study_uid",     this->deidStudyUID);
            this->DeidentifyField( fs, "rhe.patid",         this->deidPatientId);
            this->DeidentifyField( fs, "rhe.patidff",       this->deidPatientId);
            this->DeidentifyField( fs, "rhe.patname",       this->deidPatientId);
            this->DeidentifyField( fs, "rhe.patnameff",     this->deidPatientId);
            this->DeidentifyField( fs, "rhe.hospname",      this->deidStudyId);
            this->DeidentifyField( fs, "rhs.landmark_uid",  this->deidLandmarkUID);
            this->DeidentifyField( fs, "rhs.series_uid",    this->deidSeriesUID);
            this->DeidentifyField( fs, "rhi.image_uid",     this->deidImageUID);

            //  These fields are not removed from PHI_LIMITED data sets
            if (this->phiType == svkDcmHeader::PHI_DEIDENTIFIED ) {
                string deidDate = svkTypeUtils::IntToString(VTK_INT_MIN);
                this->DeidentifyField( fs, "rhr.rh_scan_time",  "00:00");
                this->DeidentifyField( fs, "rhe.dateofbirth",   "");
                this->DeidentifyField( fs, "rhr.rh_scan_date",  "01/01/070");
                this->DeidentifyField( fs, "rhe.ex_datetime",  deidDate );
            }

            fs->close();
        } else {
            cout << "ERROR: Could not open raw file for deidentification: " << this->GetFileNames()->GetValue(0) << endl;
            exit(1);
        }

        delete fs; 
    }

}

 
/*!
 *
 */
void svkGEPFileReader::UpdateProgressCallback(vtkObject* subject, unsigned long, void* thisObject, void* callData)
{
    static_cast<svkGEPFileReader*>(thisObject)->SetProgressText( ( static_cast<svkGEPFileMapper*>(subject)->GetProgressText()).c_str() );
    static_cast<svkGEPFileReader*>(thisObject)->UpdateProgress(*(double*)(callData));

}


/*!
 *  get the series description for this file for globbing purposes: 
 */
string svkGEPFileReader::GetFileSeriesDescription( string fileName )
{
    svkGEPFileReader* readerLocal = svkGEPFileReader::New( );
    readerLocal->SetFileName( fileName.c_str() );
    readerLocal->OnlyReadHeader( true );
    readerLocal->checkSeriesUID = false; 
    readerLocal->OnlyReadOneInputFile();
    readerLocal->Update();
    string seriesDescription = readerLocal->GetPFMap()["rhs.se_desc"][3]; 
    readerLocal->Delete();
    return seriesDescription;
}

