/*
 *  Copyright © 2009-2014 The Regents of the University of California.
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


#ifndef SVK_DATA_ACQUISITION_DESCRIPTION_XML_H
#define SVK_DATA_ACQUISITION_DESCRIPTION_XML_H


#ifdef __cplusplus

#include <vtkObject.h>
#include <vtkXMLDataElement.h>
#include <svkSatBandsXML.h>

#include "svkTypes.h"


namespace svk {


using namespace std;


/*! 
 *  Class to manage IO to/from XML data acquisition description files. 
 * 
 */
class svkDataAcquisitionDescriptionXML: public vtkObject
{

    public:

        static svkDataAcquisitionDescriptionXML* New();
        vtkTypeRevisionMacro( svkDataAcquisitionDescriptionXML, vtkObject);

        int                         SetXMLFileName( vtkstd::string xmlFileName );     
        vtkstd::string              GetXMLFileName( );
        void                        ClearXMLFile( );
        void                        InitializeEmptyXMLFile( );
        void                        SetVerbose( bool isVerbose );     
        int                         GetXMLVersion(); 
        vtkXMLDataElement*          FindNestedElementWithPath( string xmlPath);
        vtkstd::string              GetNestedElementCharacterDataWithPath( string xmlPath );
        void                        SetNestedElementWithPath( string xmlPath, string value );

        vtkXMLDataElement*          GetXMLDataElement(); 

        void                        GetEncodedSpace( int matrixSize[3], float fov[3] );
        svkSatBandsXML*             GetSatBandsXML();

        void                        SetTrajectoryType( vtkstd::string type );
        vtkstd::string              GetTrajectoryType( );

        void                        SetTrajectoryID( vtkstd::string ID );
        vtkstd::string              GetTrajectoryID( );

        void                        SetTrajectoryComment( vtkstd::string comment );
        vtkstd::string              GetTrajectoryComment( );

        void                        SetTrajectoryParameter( vtkstd::string name, long value  );
        long                        GetTrajectoryLongParameter( vtkstd::string name );

        void                        SetTrajectoryParameter( vtkstd::string name, double value  );
        double                      GetTrajectoryDoubleParameter( vtkstd::string name );


        int                         WriteXMLFile( string xmlFileName );


    protected:

        svkDataAcquisitionDescriptionXML();
        ~svkDataAcquisitionDescriptionXML();


    private:

        void                        SetTrajectoryParameter( vtkstd::string type, vtkstd::string name, vtkstd::string value  );
        vtkstd::string              GetTrajectoryParameter( vtkstd::string type, vtkstd::string name );
        
        //  Members:
        float                       versionNumber;
        bool                        isVerbose; 
        vtkstd::string              xmlFileName; 
        vtkXMLDataElement*          dataAcquisitionDescriptionXML;
        vtkXMLDataElement*          versionElement;
        vtkXMLDataElement*          satBandsElement;
        svkSatBandsXML*             satBandsXML;


};


}   //svk
#endif

#endif //SVK_DATA_ACQUISITION_DESCRIPTION_XML_H

#ifdef __cplusplus
extern "C" {
#endif


void*       svkDataAcquisitionDescriptionXML_New();
void*       svkDataAcquisitionDescriptionXML_Delete( void* dataAcquisitionDescriptionXML );
void*       svkDataAcquisitionDescriptionXML_Read(const char* xmlFileName, int* status);
int         svkDataAcquisitionDescriptionXML_WriteXMLFile(const char* filepath, void* xml );

void*       svkDataAcquisitionDescriptionXML_GetSatBandsXML( void* dataAcquisitionDescriptionXML ); 
void        svkDataAcquisitionDescriptionXML_SetTrajectory(const char* type, const char* id, const char* comment, void* xml); 
const char* svkDataAcquisitionDescriptionXML_GetTrajectoryType(void* xml); 
const char* svkDataAcquisitionDescriptionXML_GetTrajectoryID(void* xml); 
const char* svkDataAcquisitionDescriptionXML_GetTrajectoryComment(void* xml); 
void        svkDataAcquisitionDescriptionXML_SetTrajectoryLongParameter(const char* name, long value, void* xml); 
long        svkDataAcquisitionDescriptionXML_GetTrajectoryLongParameter(const char* name, void* xml); 
void        svkDataAcquisitionDescriptionXML_SetTrajectoryDoubleParameter(const char* name, double value, void* xml); 
double      svkDataAcquisitionDescriptionXML_GetTrajectoryDoubleParameter(const char* name, void* xml); 


#ifdef __cplusplus
}
#endif