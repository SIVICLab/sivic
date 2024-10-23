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


#ifndef SVK_DATA_MODEL_H
#define SVK_DATA_MODEL_H


#include <svkImageData.h>
#include <map>
#include </usr/include/vtk/vtkObject.h>
#include <string>
#include </usr/include/vtk/vtkImageData.h>
#include </usr/include/vtk/vtkObjectFactory.h>
#include </usr/include/vtk/vtkImageFlip.h>
#include </usr/include/vtk/vtkCallbackCommand.h>
#include <svkImageReaderFactory.h>
#include <svkImageDataFactory.h>
#include </usr/include/vtk/vtkAlgorithmOutput.h>
#include </usr/include/vtk/vtkTrivialProducer.h>
#include </usr/include/vtk/vtkSmartPointer.h>

#include </usr/include/vtk/vtkAlgorithm.h>
#include </usr/include/vtk/vtkImageReader2.h>

#include <svkImageWriterFactory.h>
#include <svkImageWriter.h>


namespace svk {


using std::string;
using std::map;


/*! 
 *  The purpose of the DataModel class is two fold. The first is to maintain
 *  a hash of svkImageData objects that can be accessed and modified by any
 *  "views". The second is to maintain a hash of states that are to be shared
 *  between "views". 
 *  The DataViewControllers observe the DataModel and can respond if relevant
 *  changes are made. 
 */
class svkDataModel : public vtkObject
{

    public:

        // vtk type revision macro
        vtkTypeMacro( svkDataModel,vtkObject );
   
        static svkDataModel*       New();  
        
        svkDataModel();
        ~svkDataModel();
        
        // DataObject manipulators 
        virtual bool                       AddDataObject( string objectName, svkImageData* dataObject );
        virtual bool                       RemoveDataObject( string objectName );
        virtual void                       RemoveAllDataObjects( );
        virtual bool                       ChangeDataObject(string objectName, svkImageData* dataObject);
        virtual bool                       ReplaceDataFromFile(string objectName, string fileName);
        virtual svkImageData*              GetDataObject( string objectName );
        virtual map<string, svkImageData*> GetAllDataObjects( );

        // For storing filenames...       
        virtual bool                       SetDataFileName( string objectName, string fileName );
        virtual string                     GetDataFileName( string objectName );

        // File loaders 
        virtual svkImageData*              LoadFile( string fileName, bool onlyOneInputFile = false );
        svkDcmHeader*                      GetDcmHeader( string fileName );
        virtual svkImageData*              AddFileToModel(string objectName, string fileName, bool onlyOneInputFile = false);
       
        // File writers 
        virtual bool                       WriteToFile( string objectName, string fileName);

        // File writers 
		virtual bool                       WriteToFile(svkImageData *data, const char *fileName);
       
        // State manipulators 
        virtual map < string, void* >      GetModelState();
        virtual void                       SetModelState( map< string, void* > modelState );
        virtual bool                       AddState( string stateName, void* stateValue );
        virtual bool                       ChangeState( string stateName, void* stateValue );
        virtual void*                      GetState( string stateName );
    
        // Existence checks for objects and states
        virtual bool                       DataExists( string objectName );
        virtual bool                       StateExists( string stateName );
        string                             GetProgressText( );
        void                               SetProgressText( string progressText );
    

    private:
    
        /*! 
         *  This hash associates strings with svkImageData pointers.
         *  It represents the currently loaded data that may need to be shared
         *  between visualizations.
         */
        map<string, svkImageData*> allDataObjectsByKeyName;
        map<string, string       > allDataFileNamesByKeyName;
        static void UpdateProgressCallback(vtkObject* subject, unsigned long, void* thisObject, void* callData);
        void UpdateProgress(double amount);


        /*! 
         *  This hash associates strings with void pointers. It represents
         *  the current state of the visualization, in case allow 
         *  inter-visualization communication.
         */
        map<string, void*> modelState;
        svkImageReader2* reader;

        svkImageData* currentData;
    
        string progressText;
        vtkCallbackCommand* progressCallback;

};


}   //svk


#endif //SVK_DATA_MODEL_H
