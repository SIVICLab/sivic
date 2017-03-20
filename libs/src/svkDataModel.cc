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


#include <svkDataModel.h>


using namespace svk;


//vtkCxxRevisionMacro(svkDataModel, "$Rev$");
vtkStandardNewMacro(svkDataModel);


//! Constructor
svkDataModel::svkDataModel()
{

#if VTK_DEBUG_ON
    this->DebugOn();
#endif

    reader = NULL;
    this->currentData = NULL;
    this->progressCallback = vtkCallbackCommand::New();
    this->progressCallback->SetCallback( UpdateProgressCallback );
    this->progressCallback->SetClientData( (void*)this );
}


//! Destructor
svkDataModel::~svkDataModel()
{
    for( map<string, svkImageData*>::iterator iter = allDataObjectsByKeyName.begin();
        iter != allDataObjectsByKeyName.end(); ++iter) {
            (iter->second)->Delete();
    }
    allDataObjectsByKeyName.clear(); 
    allDataFileNamesByKeyName.clear(); 
    modelState.clear(); 
    if( reader != NULL ) {
        reader->Delete();
        reader = NULL;
    }
    if( this->progressCallback != NULL ) {
        this->progressCallback->Delete();
        this->progressCallback = NULL;
    }
    if( this->currentData != NULL ) {
        this->currentData->Delete();
        this->currentData = NULL;
    }
}


/*! 
 *  Adds a data object to the data hash. If the object already exists,
 *  it will return 0 and will not modify the hash.
 *
 *  \param objectName the name of the object you wish to add
 *  \param dataObject the pointer to the svkImageData object you wish to add
 *
 *  \return a boolean, 0 if the object name already exists, 1 if it has been added
 */
bool svkDataModel::AddDataObject( string objectName, svkImageData* dataObject )
{
    if( DataExists( objectName ) || dataObject == NULL ) {
        return 0;
    } else {
        allDataObjectsByKeyName[ objectName ] = dataObject; 
        allDataFileNamesByKeyName[ objectName ] = "UNKNOWN"; 
        dataObject->Register(this);
        Modified(); 
        return 1;
    }
}


/*!
 *  Removes a given data object from the data hash. If the object is not
 *  present, method returns 0.
 *
 *  \param objectName the name of the object you wish to remove
 *
 *  \return a boolean, 0 if the object was removed, 1 if the object was not present
 */
bool svkDataModel::RemoveDataObject( string objectName )
{
    if( DataExists( objectName ) ) {
        if( allDataObjectsByKeyName[ objectName ] != NULL ) {
            allDataObjectsByKeyName[ objectName ]->Delete();
            allDataObjectsByKeyName[ objectName ] = NULL;
        }
        allDataObjectsByKeyName.erase( objectName );
        allDataFileNamesByKeyName.erase( objectName );
        Modified(); 
        return 1;
    } else {
        return 0;
    }
}
   
void svkDataModel::RemoveAllDataObjects( )
{
	vector<string> allNames;
    for( map<string, svkImageData*>::iterator iter = allDataObjectsByKeyName.begin();
        iter != allDataObjectsByKeyName.end(); ++iter) {
            allNames.push_back(iter->first);
    }
    for( vector<string>::iterator iter = allNames.begin(); iter != allNames.end(); ++iter) {
    	this->RemoveDataObject( *iter );
    }

}

/*!
 *  Changes a given data object in the hash to be associated with a new data
 *  object. If the object is not found, method returns 0;
 *
 *  \param objectName the name of the object you wish to change
 *  \param dataObject a pointer to the net svkImageData object
 *
 *  \return a boolean, 0 if the object name was not found, 1 if the object was replaced
 */ 
bool svkDataModel::ChangeDataObject( string objectName, svkImageData* dataObject )
{
    if( DataExists( objectName ) || dataObject == NULL ) {
        if( allDataObjectsByKeyName[ objectName ] != NULL ) {
            allDataObjectsByKeyName[ objectName ]->Delete();
            allDataObjectsByKeyName[ objectName ] = NULL;
        }
        dataObject->Register(this);
        allDataObjectsByKeyName[ objectName ] = dataObject;
        allDataFileNamesByKeyName[ objectName ] = "UNKNOWN";
        Modified();
        return 1;
    } else {
        return 0;
    }

}


/*!
 *  Replaces a data object in the data hash, with a new data object loaded 
 *  from the give file.
 *
 *  \param objectName the name of the object you wish to replace
 *  \param fileName the name of the file you wish to load for the give name
 *
 *  \return a boolean, 0 if the object name was not found, 1 if the object was replaced
 */
bool svkDataModel::ReplaceDataFromFile(string objectName, string fileName)
{
    if( DataExists( objectName ) ) {
        if( GetDataObject( objectName ) != NULL ) {
            GetDataObject( objectName )->Delete();
        }
        allDataObjectsByKeyName.erase( objectName );
        allDataFileNamesByKeyName.erase( objectName );
        AddFileToModel( objectName, fileName );
        return 1;
    } else {
        return 0;
    }
}


/*!
 *  Returns data object associated with a given name in the data hash. If the
 *  hash does not contain that key, it returns NULL. 
 *
 *  \param objectName the name of the object you want to get
 *
 *  \return an svkImageData pointer of the given name, or null if the name does not exist
 */
svkImageData* svkDataModel::GetDataObject( string objectName )
{
    if( DataExists(objectName) ) {
        return allDataObjectsByKeyName[ objectName ];
    } else {  
        return NULL;
    }
}

/*!
 * Returns the hash of data objects in the model.
 */
map<string, svkImageData*> svkDataModel::GetAllDataObjects( )
{
	return this->allDataObjectsByKeyName;
}


/*!
 *
 */  
bool svkDataModel::SetDataFileName(string objectName, string fileName)
{
    if( !DataExists( objectName ) ) {
        return 0;
    } else {
        allDataFileNamesByKeyName[objectName] = fileName;
        this->Modified();
        return 1;
    }
}


/*!
 *
 */  
string svkDataModel::GetDataFileName(string objectName)
{
    if( !DataExists( objectName ) ) {
        return "";
    } else {
        return allDataFileNamesByKeyName[objectName];
    }
}


/*!
 *  Loads a file into an svkImageData object and returns a pointer to it.
 *  If the file cannot be loaded, the program exits.
 * 
 *  NOTE!!! The result of this method needs be registered before
 *          another dataset can be loaded, otherwise loading a 
 *          second dataset will DELETE the first!
 *
 *  \param fileName the name of the file to load
 *
 *  \return an svkImageData pointer to the loaded object or NULL if 
 *          file could not be loaded. 
 */
svkImageData* svkDataModel::LoadFile( string fileName, bool onlyOneInputFile )
{
    float* imageOrigin;
    svkImageReaderFactory* readerFactory = svkImageReaderFactory::New();

    /* We keep a reader as a member variable so as to give the caller an opportunity
    * to register the image data before the reader is deleted. If a subsequent 
    * dataset is load, a new reader is created deleting the old reader and any data
    * it loaded that was not registered.
    */ 
    if( reader != NULL ) {
        reader->Delete();
        reader = NULL;
    }
    reader = readerFactory->CreateImageReader2(fileName.c_str());
    readerFactory->Delete();

    if (reader != NULL) {
        if( onlyOneInputFile ) {
            reader->OnlyReadOneInputFile();
        }
        reader->AddObserver(vtkCommand::ProgressEvent, progressCallback);
        reader->SetFileName( fileName.c_str() );
        reader->Update();
        vtkInformation* readerInformation = reader->GetOutputPortInformation(0);
        const char* dataType = readerInformation->Get(vtkDataObject::DATA_TYPE_NAME());

        if( this->currentData != NULL ) {
            this->currentData->Delete();
            this->currentData = NULL;
        }
        // We are going to decouple the data from the reader to avoid downstream update issues
        this->currentData = svkImageDataFactory::CreateInstance( dataType );
        this->currentData->ShallowCopy( reader->GetOutput() );

        reader->RemoveObserver(progressCallback);
    } else {
    
        vtkWarningWithObjectMacro( this, "Can not find appropriate reader for " << fileName );

    }
    
    return this->currentData;
}


/*!
 *  Utility method for getting tags the header of a specific file.
 */
svkDcmHeader* svkDataModel::GetDcmHeader( string fileName )
{
    return this->LoadFile( fileName )->GetDcmHeader();
}


/*!
 *  Adds a file into the data hash.
 *
 *  \param objectName the name you want to give the new object
 *  \param fileName the file from which you want to create the new object 
 *
 *  \return a pointer to the new data object
 */  
svkImageData* svkDataModel::AddFileToModel(string objectName, string fileName, bool onlyOneInputFile)
{
	svkImageData* myData = LoadFile( fileName.c_str(), onlyOneInputFile );
    if( this->DataExists( objectName ) ) {
        ChangeDataObject( objectName, myData );
    } else {
        AddDataObject( objectName, myData );
    }
	SetDataFileName( objectName, fileName );
	return myData;
}


/*!
 *  Not yet implemented, returns 0.
 */ 
bool svkDataModel::WriteToFile( string objectName, string fileName)
{
    return 0;
}

/*!
 *  Not yet implemented, returns 0.
 */ 
bool svkDataModel::WriteToFile(svkImageData *data, const char *fileName)
{
    svkImageWriterFactory* writerFactory = svkImageWriterFactory::New();
    svkImageWriter* writer;

    string fileNameString = string( fileName);
    size_t pos = fileNameString.find(".");
    if( strcmp( fileNameString.substr(pos).c_str(), ".ddf" ) == 0 ) {
        writer = static_cast<svkImageWriter*>(writerFactory->CreateImageWriter(svkImageWriterFactory::DDF));
    } else {
        writer = static_cast<svkImageWriter*>(writerFactory->CreateImageWriter(svkImageWriterFactory::DICOM_MRS));
    }
    cout << "FN: " << fileName << endl;
    writer->SetFileName(fileName);
    writerFactory->Delete();

    writer->SetInputData(data);

    writer->Write();
    writer->Delete();
    return 0;
}

/*!
 *  Gets the entire state hash.
 *
 *  \return the state hash
 */ 
map<string, void*> svkDataModel::GetModelState()
{
    return modelState;
}


/*!
 *  Sets the entire state hash.
 *
 *  \param modelState the new state hash
 */
void svkDataModel::SetModelState( map< string, void* > modelState )
{
    this->modelState = modelState;
    Modified();
}


/*!
 *  Adds a state to the hash. If the state is already present the method
 *  returns 0 without modifying the hash.
 *
 *  \param stateName the name of the new state you want to add
 *  \param stateValue a void pointer to the data representing the state
 *
 *  \return boolean, 0 if the state already exists, 1 if it has been added 
 */
bool svkDataModel::AddState( string stateName, void* stateValue )
{
    if( StateExists( stateName ) ) {
        return 0;
    } else {
        modelState[ stateName ] = stateValue; 
        Modified();
        return 1;
    }
}


/*!
 *  Changes a state in the state hash. If the state does not exist, zero is
 *  returned and the hash is not modified.  
 *
 *  \param stateName the name of the state you wish to change
 *  \param stateValue a void pointer to the data representing the state
 *
 *  \return a boolean, 0 if the state does not exist, 1 if the state has been changed
 */
bool svkDataModel::ChangeState( string stateName, void* stateValue )
{
    int* myNewState = static_cast<int*>(stateValue);
    if( StateExists( stateName ) ) {
        modelState[ stateName ] = stateValue;
        Modified();
        return 1;
    } else {
        return 0;
    }
}


/*!
 *  Gets the state value for the given key. Returns NULL if the key is not
 *  found.
 *
 *  \param stateName the name of the state you wish to get
 *
 *  \return a void pointer to the data the state represents
 */
void* svkDataModel::GetState( string stateName )
{
    if( StateExists( stateName ) ) {
        return modelState[ stateName ];
    } else {
        return NULL;
    }
}


/*!
 *  Checks to see if a given data object exists or not.
 *
 *  \param objectName the name of the object you are looking for
 *
 *  \return a boolean, 0 for no object found, 1 for object found
 */
bool svkDataModel::DataExists( string objectName )
{
    if( allDataObjectsByKeyName.find( objectName ) == allDataObjectsByKeyName.end() ) {
        if (this->GetDebug()) {
            cout << "Data Object \'" << objectName << "\' does not exist." << endl;
        }
        return 0;
    } else {
        if (this->GetDebug()) {
            cout << "Data Object \'" << objectName << "\' exists." << endl;
        }
        return 1;
    }
}


/*!
 *  Checks to see if a given state exists or not.
 *
 *  \param stateName the name of the state you are looking for
 *
 *  \return a boolean, 0 for no state found, 1 for state found
 */
bool svkDataModel::StateExists( string stateName )
{
    if( modelState.find( stateName ) == modelState.end() ) {
        if (this->GetDebug()) {
            cout << "State \'" << stateName << "\' does not exist." << endl;
        } 
        return 0;
    } else {
        if (this->GetDebug()) {
            cout << "State \'" << stateName << "\' exists." << endl;
        }
        return 1;
    }
}

void svkDataModel::SetProgressText( string progressText ) 
{
    this->progressText = progressText;
}

string svkDataModel::GetProgressText( ) 
{
    return this->progressText;
}

void svkDataModel::UpdateProgress(double amount)
{
    this->InvokeEvent(vtkCommand::ProgressEvent,static_cast<void *>(&amount));
}   

void svkDataModel::UpdateProgressCallback(vtkObject* subject, unsigned long, void* thisObject, void* callData)
{
    if( vtkAlgorithm::SafeDownCast(subject)->GetProgressText() != NULL ) {
        static_cast<svkDataModel*>(thisObject)->SetProgressText( string( static_cast<vtkAlgorithm*>(subject)->GetProgressText()) );
    }
    static_cast<svkDataModel*>(thisObject)->UpdateProgress(*(double*)(callData)); 

}

