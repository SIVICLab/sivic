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


#include <svkVariableFlipDatReader.h>
#include <svkImageReaderFactory.h>
#include <svkString.h>
#include <svkTypeUtils.h>
#include </usr/include/vtk/vtkDebugLeaks.h>
#include </usr/include/vtk/vtkByteSwap.h>
#include </usr/include/vtk/vtkSmartPointer.h>
#include </usr/include/vtk/vtkDelimitedTextReader.h>
#include </usr/include/vtk/vtkTable.h>
#include </usr/include/vtk/vtkStringToNumeric.h>

#include <sys/stat.h>


using namespace svk;


vtkStandardNewMacro(svkVariableFlipDatReader);


/*!
 *  
 */
svkVariableFlipDatReader::svkVariableFlipDatReader()
{

#if VTK_DEBUG_ON
    this->DebugOn();
    vtkDebugLeaks::ConstructClass("svkVariableFlipDatReader");
#endif

    vtkDebugMacro( << this->GetClassName() << "::" << this->GetClassName() << "()" );

    //  3 required input ports: 
    this->SetNumberOfInputPorts(0);
    this->datType = svkImageReader2::UNDEFINED; 
    this->datFp   = NULL;
}


/*!
 *
 */
svkVariableFlipDatReader::~svkVariableFlipDatReader()
{
    vtkDebugMacro( << this->GetClassName() << "::~" << this->GetClassName() << "()" );
    if ( this->datFp != NULL )  {
        delete this->datFp;
        this->datFp = NULL;
    }

}


/*!
 *  Check to see if the extension indicates a UCSF IDF file.  If so, try 
 *  to open the file for reading.  If that works, then return a success code. 
 *  Return Values: 1 if can read the file, 0 otherwise.
 */
int svkVariableFlipDatReader::CanReadFile(const char* fname)
{

    string fileToCheck(fname);
    bool canRead = 0; 

    if( fileToCheck.size() > 4 ) {

        // Also see "vtkImageReader2::GetFileExtensions method" 
        if (
            fileToCheck.substr( fileToCheck.size() - 4 ) == ".dat"
        )  {

            if ( this->OpenDatFile(fname) == 0  ) { 

                this->InitDatReader(); 
                if ( this->datType == svkImageReader2::VARIABLE_FLIP_DAT ) { 

                    vtkDebugMacro(<< this->GetClassName() << "::CanReadFile(): It's a Variable Flip Dat File: " << fileToCheck);
                    canRead = 1; 
                }

                this->datFp->close();
            }

            if ( this->datFp != NULL ) {
                delete this->datFp;
                this->datFp= NULL;
            }
            return canRead; 

        } else {
            vtkDebugMacro(<< this->GetClassName() << "::CanReadFile(): It's NOT a Variable Flip Dat File: " << fileToCheck);
            return 0;
        }

    } else {
        vtkDebugMacro(<< this->GetClassName() << "::CanReadFile(): s NOT a valid file: " << fileToCheck);
        return 0;
    }
}


/*
 *  opens dat file
 *      return: 0 = successful
 *      return: 1 = could not open 
 */
int svkVariableFlipDatReader::OpenDatFile(const char* fname)
{
    int status = 1; 
    this->datFp = new ifstream();
    this->datFp->exceptions( ifstream::eofbit | ifstream::failbit | ifstream::badbit );
    this->datFp->open( fname, ios::binary);
    if ( this->datFp->is_open() ) {
        this->datFp->seekg( 0, ios_base::beg );
        status = 0; 
    }
    return status; 
}


/*
 *  Gets the dat type.  If this is a variable flip angle dat file, also parses remainder 
 *  of dat file meta data into this->datMap
 */
svkImageReader2::ReaderType svkVariableFlipDatReader::InitDatReader()
{
    if ( this->datType == svkImageReader2::UNDEFINED ) {

        //  read first line of this->datFp 
        if ( this->datFp->is_open() ) {

            istringstream* iss = new istringstream();

            this->datMap["dat_type"] = this->ReadLineValue( this->datFp, iss, ':');
            if ( this->datMap["dat_type"].compare("variable_flip_angle") == 0 ) {
                if ( this->ParseDatMetaData() == 0 ) { 
                    this->datType = svkImageReader2::VARIABLE_FLIP_DAT; 
                }
            }    

            delete iss; 
        }
    }

    //cout << "svkVariabelFlipDatReader(DAT TYPE): " << this->datType << endl;
    return this->datType; 
}    


/*!
 *   Parses dat file meta data
 *      returns 0 on success, or 1 if the version is not recognized.    
 */
int svkVariableFlipDatReader::ParseDatMetaData()
{
    int status = 1; 
    istringstream* iss = new istringstream();

    this->datMap["dat_version"] = this->ReadLineValue( this->datFp, iss, ':');

    //  Version 1: 
    if ( this->datMap["dat_version"].compare("1") == 0 ) {
        this->datMap["dat_content_name"]    = this->ReadLineValue( this->datFp, iss, ':');
        this->datMap["num_time_pts"]        = this->ReadLineValue( this->datFp, iss, ':');
        this->datMap["profile_bandwidth"]   = this->ReadLineValue( this->datFp, iss, ':');
        this->datMap["profile_num_pts"]     = this->ReadLineValue( this->datFp, iss, ':');
        this->PrintKeyValuePairs(); 
        status = 0; 
    } else {
        cout << "ERROR(svkVariableFllipDatReader::ParseDatMetaData) Unknown Version: " <<  this->datMap["dat_version"] << endl;
        status = 1; 
    }

    delete iss; 
    return status; 
}    


/*!
 *  Prints the key value pairs parsed from the header.
 */
void svkVariableFlipDatReader::PrintKeyValuePairs()
{
    if (this->GetDebug()) {
        map< string, string >::iterator mapIter;
        for ( mapIter = this->datMap.begin(); mapIter != this->datMap.end(); ++mapIter ) {
            cout << this->GetClassName() << " " << mapIter->first << " = ";
            cout << this->datMap[mapIter->first] << endl;
        }
    }
}


/*!
 *
 */ 
int svkVariableFlipDatReader::GetNumTimePoints()
{
    return svkTypeUtils::StringToInt( this->datMap["num_time_pts"] ); 
}


/*!
 *  
 */ 
float svkVariableFlipDatReader::GetProfileBandwidth()
{
    return svkTypeUtils::StringToFloat( this->datMap["profile_bandwidth"] ); 
}


/*!
 *
 */ 
int  svkVariableFlipDatReader::GetProfileNumPoints()
{
    return svkTypeUtils::StringToInt( this->datMap["profile_num_pts"] ); 
}


/*!
 *  This is an array of scaling factors to be applied to the reconstructed data in 
 *  frequency domain.  The bandwidth is defined (GetProfileBandwidth) with 0 hz at 
 *  the center point ( GetNumTimePoints()/2 , e.g. 128 for 256 point profile.
 *
 *  The time index starts at 1 
 *  returns status 0 on success 
 */
int svkVariableFlipDatReader::GetSignalScaling( int timePt, vtkFloatArray* signalScale )
{
    int status = this->GetScaling( "signal", timePt, signalScale ); 
    return status; 
}


/*!
 *
 *  The time index starts at 1 
 *  returns status 0 on success 
 */ 
int svkVariableFlipDatReader::GetMzScaling( int timePt, vtkFloatArray* mzScale )
{
    int status = this->GetScaling( "mz", timePt, mzScale ); 
    return status; 
}


/*
 *  gets either "mz" or "signal" scaling set by the type argument: 
 */
int svkVariableFlipDatReader::GetScaling( string type, int timePt, vtkFloatArray* scale )
{
    int status = 1; 

    scale->SetNumberOfComponents(1); 
    scale->SetNumberOfTuples( this->GetProfileNumPoints() );
    
    if ( (timePt >= 1 ) && (timePt <= this->GetNumTimePoints()) ) {  

        //  read until reached "time_point: timePt, 
        //  then read ProfileNumPoints float values form signal-scaling: vals
        bool continueReading = true;     
        istringstream* iss = new istringstream();
        if ( ! this->datFp->is_open() ) {
            cout << "ERROR DAT FILE NOT OPEN" << endl;
        }
        this->datFp->clear();
        this->datFp->seekg( 0, ios_base::beg );

        this->SetReadLength( 5000 ); 
        while ( continueReading ) {

            //  line may or may not be a timepoint
            string key; 
            string value; 
            this->ReadLineKeyValue( this->datFp, iss, ':', &key, &value);
            if ( key.compare("time_point") == 0 )  {
                int timeIndex = svkTypeUtils::StringToInt( value ); 
                if ( timeIndex == timePt ) {
                    //cout << "timeindex: " << timeIndex << endl;  
                    continueReading = false;     

                    //  the next line contains the scaling values.  Read them into a string
                    //  then use the values to initialize a vtkFloatArray: 

                    //  for type = signal read the next line
                    this->ReadLineKeyValue( this->datFp, iss, ':', &key, &value);

                    //  for mz, skip over the signal scaling line and read the mz_scaling line
                    if ( type.compare("mz") == 0 ) {
                        this->ReadLineKeyValue( this->datFp, iss, ':', &key, &value);
                    }
                    
                    // skip leading space:     
                    size_t posStart = value.find_first_not_of(" "); 
                    value = value.substr(posStart);  
                    for ( int i = 0; i < this->GetProfileNumPoints(); i++ ) {
                        size_t posEnd = value.find_first_of(" ") ; 
                        float scaleVal = svkTypeUtils::StringToFloat( value.substr(0, posEnd) ); 
                        scale->SetTuple(i, &scaleVal);
                        //cout << "VALUE: " << i << " : " << scaleVal << endl; 
                        value = value.substr(posEnd + 1);  
                    
                    }
                    status = 0; 
                }
            }
        }
        delete iss; 
    }
    return status; 
}




/*!
 *  Side effect of Update() method.  Used to initialize the svkDcmHeader member of 
 *  the target svkImageData object and uses the header to set up the Output Informatin.
 *  Called before ExecuteData()
 */
void svkVariableFlipDatReader::ExecuteInformation()
{
    vtkDebugMacro( << this->GetClassName() << "::ExecuteInformation()" );

    if (this->FileName == NULL) {
        return;
    }

    if ( this->OpenDatFile(this->FileName) == 0  ) { 
        this->InitDatReader(); 
    }
}


/*!
 *  returns the data size. 
 */
svkDcmHeader::DcmPixelDataFormat svkVariableFlipDatReader::GetFileType()
{
    return svkDcmHeader::SIGNED_FLOAT_4;
}


/*!
 *
 */
void svkVariableFlipDatReader::InitDcmHeader( )
{
}


/*!
 *  *  input ports 0 is required. All input ports are MRI data. 
 *      0:  Input File Name(required)
 */
//int svkVariableFlipDatReader::FillInputPortInformation( int port, vtkInformation* info )
//{
    //return 1;
//}


/*!
 *
 */
int svkVariableFlipDatReader::FillOutputPortInformation( int vtkNotUsed(port), vtkInformation* info )
{
    info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkImageData");
    return 1;
}


