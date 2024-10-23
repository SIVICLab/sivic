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


#ifndef SVK_DATA_ACQUISITION_DESCRIPTION_XML_H
#define SVK_DATA_ACQUISITION_DESCRIPTION_XML_H

/*!\file
 *  This header file contains the declaration of the
 *  svk::svkDataAcquisitionDescriptionXML class. Additionally it contains a set
 *  of C methods for creating, deleting and interacting with the object. This
 *  is to provide access to the functionality of the class in C programs.
 *
 */


enum EPSILobe
{
    EVEN,
    ODD,
    UNDEFINED
};
#if defined(__cplusplus) && !defined(SVK_EPIC)

#include "/usr/include/vtk/vtkObject.h"
#include </usr/include/vtk/vtkXMLDataElement.h>

#include <svkSatBandsXML.h>
#include <svkEPSIReorder.h>
#include <svkXMLUtils.h>
#include <svkInt.h>
#include <svkEPSIReorder.h>
#include <svkUtils.h>

#include "svkTypes.h"


namespace svk {


using namespace std;


/*! 
 *  Class to manage IO for XML data acquisition description files. These
 *  files are intended to be used to describe acquisitions in such a way that
 *  when they are coupled with a raw data file they can be used to determine all
 *  necessary information to reconstruct the data. Our initial development will
 *  be to use an XML format on the ISMRMRD XML header. Details of the ISMRMRD
 *  can be found here: http://ismrmrd.github.io/
 *
 *  This class uses XML path (xpath) style notation for referencing specific
 *  elements. Currently only the most basic syntax is supported which is to refer
 *  to an element by its location which is defined as a '/' separated list of
 *  all parent elements followed by the element itself. For example if you had
 *  the following xml data:
    \code{.xml}
    <encoding>
        <trajectory>cartesian</trajectory>
    </encoding>
    \endcode
 *
 *  The 'trajectory' element would be found at the path 'encoding/trajectory'.
 *
 * 
 */
class svkDataAcquisitionDescriptionXML: public vtkObject
{

    public:

        static svkDataAcquisitionDescriptionXML* New();
        vtkTypeMacro( svkDataAcquisitionDescriptionXML, vtkObject);

        int                         SetXMLFileName( string xmlFileName );     
        string                      GetXMLFileName( );
        void                        ClearXMLFile( );
        void                        InitializeEmptyXMLFile( );
        void                        SetVerbose( bool isVerbose );     
        int                         GetXMLVersion(); 
        vtkXMLDataElement*          FindNestedElementWithPath( string xmlPath);
        vtkXMLDataElement*          FindOrCreateNestedElementWithPath( string parentPath, string elementName);

        string                      GetDataWithPath( const char* xmlPath );
        int                         GetIntByIndexWithParentPath( int index, const char* parentPath );
        int                         GetIntWithPath( const char* elementPath );
        void                        SetIntWithPath( const char* parentPath, const char* elmentName, int value );
        float                       GetFloatWithPath( const char* elementPath );
        void                        SetFloatWithPath( const char* parentPath, const char* elmentName, float value );

        string                      GetDataByIndexWithParentPath( int index, const char* parentPath );
        vtkXMLDataElement*          GetNestedElementByIndexWithParentPath( int index, const char* parentPath );

        int                         SetDataWithPath( const char* xmlPath, const char* value );
        void                        SetDataWithPath( const char* parentPath, const char* elmentName, string value );

        vtkXMLDataElement*          AddElementWithParentPath( const char* xmlPath, const char* name );
        int                         RemoveElementWithParentPath( const char* xmlPath, const char* name );

        vtkXMLDataElement*          GetRootXMLDataElement();

        void                        GetEncodedSpace( int matrixSize[3], float fov[3] );
        svkSatBandsXML*             GetSatBandsXML();

        void                        SetTrajectoryType( string type );
        string                      GetTrajectoryType( );

        void                        SetTrajectoryID( string ID );
        string                      GetTrajectoryID( );

        void                        SetTrajectoryComment( string comment );
        string                      GetTrajectoryComment( );

        int                         GetTrajectoryNumberOfDimensions();
        void                        AddTrajectoryDimension(string id, string logical, string description);
        string                      GetTrajectoryDimensionId(int index);
        string                      GetTrajectoryDimensionLogical(int index);
        string                      GetTrajectoryDimensionDescription(int index);
        void                        SetEPSIType(EPSIType type);

        void                        SetEPSINumberOfInterleaves( int numberOfInterleaves );
        int                         GetEPSINumberOfInterleaves( );

        void                        SetEPSIGradientAmplitude( float gradientAmplitude, EPSILobe lobe);
        float                       GetEPSIGradientAmplitude( EPSILobe lobe );
        
        void                        SetEPSIRampDuration( float rampDuration, EPSILobe lobe);
        float                       GetEPSIRampDuration( EPSILobe lobe );

        void                        SetEPSIPlateauDuration( float plateauDuration, EPSILobe lobe);
        float                       GetEPSIPlateauDuration( EPSILobe lobe );

        void                        SetEPSINumberOfLobes( int numberOfLobes, EPSILobe lobe);
        int                         GetEPSINumberOfLobes( EPSILobe lobe );

        void                        SetEPSISampleSpacing( float sampleSpacing );
        float                       GetEPSISampleSpacing( );

        void                        SetEPSIAcquisitionDelay( float acquisitionDelay );
        float                       GetEPSIAcquisitionDelay( );
        
        void                        SetEPSIEchoDelay( float echoDelay );
        float                       GetEPSIEchoDelay( );
        
        void                        SetEPSIGradientAxis( int dimensionIndex );
        string                      GetEPSIGradientAxisId( );
        int                         GetEPSIGradientAxisIndex( );

        void                        SetTrajectoryParameter( string name, long value  );
        long                        GetTrajectoryLongParameter( string name );

        void                        SetTrajectoryParameter( string name, double value  );
        double                      GetTrajectoryDoubleParameter( string name );

        EPSIType                    GetEPSIType();
        string                      GetEPSITypeString();
        void                        AddEncodedMatrixSizeDimension( string name, int value);
        int                         GetEncodedMatrixSizeNumberOfDimensions();
        string                      GetEncodedMatrixSizeDimensionName(int index);
        int                         GetEncodedMatrixSizeDimensionValue(int index);

        void                        GetSamplingIndicies( int* indicies );
        void                        GetSamplingMask( int* samplingMask );
        void                        GetBlips( int index, string blipDimension, int* blips);

        int                         WriteXMLFile( string xmlFileName );


    protected:

        svkDataAcquisitionDescriptionXML();
        ~svkDataAcquisitionDescriptionXML();


    private:
        vtkXMLDataElement*          GetEPSIEncodingElement();
        void                        SetTrajectoryParameter( string type, string name, string value  );
        string                      GetTrajectoryParameter( string type, string name );
        vtkXMLDataElement*          GetTrajectoryParameterElement( string type, string name );
        
        //  Members:
        float                       versionNumber;
        bool                        isVerbose; 
        string                      xmlFileName; 
        vtkXMLDataElement*          dataAcquisitionDescriptionXML;
        vtkXMLDataElement*          versionElement;
        vtkXMLDataElement*          satBandsElement;
        svkSatBandsXML*             satBandsXML;


};


}   //svk
#endif

#if defined(__cplusplus) && !defined(SVK_EPIC)
extern "C" {
#endif
#include <svkTypes.h>
struct svkCString
{
    char c_str[256];
};



void*             svkDataAcquisitionDescriptionXML_New();
void*             svkDataAcquisitionDescriptionXML_Delete( void* dataAcquisitionDescriptionXML );
void*             svkDataAcquisitionDescriptionXML_Read(const char* xmlFileName, int* status);
int               svkDataAcquisitionDescriptionXML_WriteXMLFile(const char* filepath, void* xml );

struct svkCString svkDataAcquisitionDescriptionXML_GetDataWithPath( void* xml, const char* path );
int               svkDataAcquisitionDescriptionXML_SetDataWithPath( void* xml, const char* path, const char* data );
int               svkDataAcquisitionDescriptionXML_AddElementWithParentPath( void* xml, const char* path,
                                                                             const char* name );
int               svkDataAcquisitionDescriptionXML_RemoveElementWithParentPath( void* xml,
                                                                                const char* path, const char* name );

void*             svkDataAcquisitionDescriptionXML_GetSatBandsXML( void* dataAcquisitionDescriptionXML );
void              svkDataAcquisitionDescriptionXML_SetTrajectory(const char* type, const char* id,
                                                                 const char* comment, void* xml);
struct svkCString svkDataAcquisitionDescriptionXML_GetTrajectoryType(void* xml);
struct svkCString svkDataAcquisitionDescriptionXML_GetTrajectoryID(void* xml);
struct svkCString svkDataAcquisitionDescriptionXML_GetTrajectoryComment(void* xml);

void              svkDataAcquisitionDescriptionXML_AddTrajectoryDimension(const char* id, const char* logical,
                                                                          const char* description, void *xml);
int               svkDataAcquisitionDescriptionXML_GetTrajectoryNumberOfDimensions(void *xml);                   
struct svkCString svkDataAcquisitionDescriptionXML_GetTrajectoryDimensionId(int index, void *xml);              
struct svkCString svkDataAcquisitionDescriptionXML_GetTrajectoryDimensionLogical(int index, void *xml);        
struct svkCString svkDataAcquisitionDescriptionXML_GetTrajectoryDimensionDescription(int index, void *xml);   

void              svkDataAcquisitionDescriptionXML_SetEPSITypeToFlyback(void* xml);
void              svkDataAcquisitionDescriptionXML_SetEPSITypeToSymmetric(void* xml);
void              svkDataAcquisitionDescriptionXML_SetEPSIType( EPSIType type, void* xml);
EPSIType          svkDataAcquisitionDescriptionXML_GetEPSIType(void* xml);

void              svkDataAcquisitionDescriptionXML_SetEPSINumberOfInterleaves( int numberOfInterleaves, void* xml);
int               svkDataAcquisitionDescriptionXML_GetEPSINumberOfInterleaves( void* xml);

void              svkDataAcquisitionDescriptionXML_SetEPSIGradientAmplitude(float amplitude, enum EPSILobe lobe, void* xml);
float             svkDataAcquisitionDescriptionXML_GetEPSIGradientAmplitude(enum EPSILobe lobe, void* xml);

void              svkDataAcquisitionDescriptionXML_SetEPSIRampDuration(float duration, enum EPSILobe lobe, void* xml);
float             svkDataAcquisitionDescriptionXML_GetEPSIRampDuration(enum EPSILobe lobe, void* xml);

void              svkDataAcquisitionDescriptionXML_SetEPSIPlateauDuration(float duration, enum EPSILobe lobe, void* xml);
float             svkDataAcquisitionDescriptionXML_GetEPSIPlateauDuration(enum EPSILobe lobe, void* xml);

void              svkDataAcquisitionDescriptionXML_SetEPSINumberOfLobes(int numberOfLobes, enum EPSILobe lobe, void* xml);
float             svkDataAcquisitionDescriptionXML_GetEPSINumberOfLobes(enum EPSILobe lobe, void* xml);

void              svkDataAcquisitionDescriptionXML_SetEPSISampleSpacing(float sampleSpacing, void* xml);
float             svkDataAcquisitionDescriptionXML_GetEPSISampleSpacing(void* xml);

void              svkDataAcquisitionDescriptionXML_SetEPSIAcquisitionDelay(float acquisitionDelay, void* xml);
float             svkDataAcquisitionDescriptionXML_GetEPSIAcquisitionDelay(void* xml);

void              svkDataAcquisitionDescriptionXML_SetEPSIEchoDelay(float echoDelay, void* xml);
float             svkDataAcquisitionDescriptionXML_GetEPSIEchoDelay(void* xml);

void              svkDataAcquisitionDescriptionXML_SetEPSIGradientAxis(int dimensionIndex, void* xml);
struct svkCString svkDataAcquisitionDescriptionXML_GetEPSIGradientAxisId(void* xml);
int               svkDataAcquisitionDescriptionXML_GetEPSIGradientAxisIndex(void* xml);

void              svkDataAcquisitionDescriptionXML_SetTrajectoryLongParameter(const char* name,
                                                                              long value, void* xml);
long              svkDataAcquisitionDescriptionXML_GetTrajectoryLongParameter(const char* name, void* xml);
void              svkDataAcquisitionDescriptionXML_SetTrajectoryDoubleParameter(const char* name,
                                                                                double value, void* xml);
double            svkDataAcquisitionDescriptionXML_GetTrajectoryDoubleParameter(const char* name, void* xml);
void              svkDataAcquisitionDescriptionXML_AddEncodedMatrixSizeDimension(const char* name, int value,
                                                                                 void* xml);
int               svkDataAcquisitionDescriptionXML_GetEncodedMatrixSizeDimensionValue(int index, void* xml);
struct svkCString svkDataAcquisitionDescriptionXML_GetEncodedMatrixSizeDimensionName(int index, void* xml);
int               svkDataAcquisitionDescriptionXML_GetEncodedMatrixSizeNumberOfDimensions(void* xml);

#if defined(__cplusplus) && !defined(SVK_EPIC)
}
#endif
#endif
