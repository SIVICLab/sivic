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


#ifndef SVK_SAT_BANDS_XML_H
#define SVK_SAT_BANDS_XML_H


#ifdef __cplusplus

#include </usr/include/vtk/vtkObject.h>
#include </usr/include/vtk/vtkXMLDataElement.h>

#include "svkTypes.h"


namespace svk {


using namespace std;


/*! 
 *  Class to manage IO to/from XML representation of sat bands. 
 * 
 *  The sat band file contains 2 elements, 1 for PRESS, one for auto-sats. Each of these
 *  is comprised of a sat band definition: 
 * 
 *  normal_x
 *  normal_y
 *  normal_z
 *  thickness
 *  distance_from_origin
 * 
 *  - the normal is defined as the normal from the origin to the center of the band 
 *  - the thickness is the thicness of the band 
 *  - the distance from the origin defined how far along the normal vector the center of the
 *    band is. 
 */
class svkSatBandsXML: public vtkObject
{

    public:

        static svkSatBandsXML* New();
        vtkTypeMacro( svkSatBandsXML, vtkObject);

        int                         SetXMLFileName( std::string xmlFileName );     
        int                         ParseXML( vtkXMLDataElement* satBandsElement );
        std::string              GetXMLFileName( );
        void                        ClearXMLFile( );
        void                        SetVerbose( bool isVerbose );     
        int                         GetNumberOfPressBoxSats(); 
        int                         GetNumberOfAutoSats();
        int                         GetXMLVersion(); 
        void                        GetPressBoxSat(
                                        int     satNumber, 
                                        string* label, 
                                        float*  normalX, 
                                        float*  normalY, 
                                        float*  normalZ, 
                                        float*  thickness,  
                                        float*  distance  
                                    ); 
        int                         GetAutoSat(
                                        int satNumber, 
                                        string* label, 
                                        float normal[3], 
                                        float* thickness, 
                                        float* distance
                                    ); 
        int                         GetAutoSat(
                                        int     satNumber, 
                                        string* label, 
                                        float*  normalX, 
                                        float*  normalY, 
                                        float*  normalZ, 
                                        float*  thickness,  
                                        float*  distance  
                                    ); 
        void                        GetPRESSBoxParameters( 
                                        float pressOrigin[3], 
                                        float pressThickness[3], 
                                        float pressAngles[3] 
                                    ); 

        
        int                         GetAutoSatParameters( 
                                        int    satNumber, 
                                        float  normal[3], 
                                        float* thickness, 
                                        float* distance
                                    ); 

        int                         ConvertDatToXML( string rootName ); 
        int                         ConvertDatToXML2( string rootName ); 

        vtkXMLDataElement*          GetXMLDataElement(); 

        void                        WriteXMLFile( string xmlFileName);



    protected:

        svkSatBandsXML();
        ~svkSatBandsXML();


    private:

        float                       GetFloatElementData( vtkXMLDataElement* element ); 
        void                        InitSatBandInfo( 
                                        vtkXMLDataElement*  satBandElement, 
                                        string*             label, 
                                        float*              normalX, 
                                        float*              normalY, 
                                        float*              normalZ, 
                                        float*              thickness, 
                                        float*              distance
                                    ); 
        bool                        IsNormalUnique( float normal[3], float normals[3][3] );
        bool                        IsConventionalNormal( float normalIn[3] ); 
        void                        InitPressBoxNormals( float normals[3][3] ); 
        void                        MakePRESSOrthonormal( float normals[3][3] ); 
        void                        InitPressDistances(float normals[3][3], float distances[3][2]); 
        void                        RotationMatrixToEulerAngles( float normals[3][3], float eulerAngles[3] ); 
        void                        LPSToRAS( float normals[3][3] ); 
        void                        TransposeNormals( float normals[3][3] );
        void                        PSDAutSatAnglesToNormal( float angle1, float angle2, float normal[3] ); 
        int                         InitPressBoxFromDat( string rootName ); 
        int                         InitPressBoxFromDat2( string rootName );
        int                         InitSatsFromDat( string rootName ); 
        void                        SortNormalArrayRLAPSI( float  normals[3][3] ); 
        // out of 6 plane representation
        void                        GetPRESSBoxParametersVer10( 
                                        float pressOrigin[3], 
                                        float pressThickness[3], 
                                        float pressAngles[3] 
                                    ); 
        // out of 3 plane representation
        void                        GetPRESSBoxParametersVer20( 
                                        float pressOrigin[3], 
                                        float pressThickness[3], 
                                        float pressAngles[3] 
                                    ); 
        
        //  Members:
        float                       versionNumber;
        bool                        isVerbose; 
        std::string              xmlFileName; 
        vtkXMLDataElement*          satBandsXML;
        vtkXMLDataElement*          versionElement;
        vtkXMLDataElement*          pressBoxElement;
        vtkXMLDataElement*          autoSatsElement;


};


}   //svk
#endif

#endif //SVK_SAT_BANDS_XML_H

#ifdef __cplusplus
extern "C" {
#endif


void* svkSatBandsXML_New(const char* xmlFileName, int* status); 
void* svkSatBandsXML_Delete( void* xml ); 
int   svkSatBandsXML_GetNumberOfPressBoxSats( void* xml ); 
int   svkSatBandsXML_GetNumberOfAutoSats( void* xml ); 
void  svkSatBandsXML_GetPressBoxSat(
                                    void*   xml, 
                                    int     satNumber, 
                                    float*  normalX, 
                                    float*  normalY, 
                                    float*  normalZ, 
                                    float*  thickness,  
                                    float*  distance  
        ); 
void  svkSatBandsXML_GetPRESSBoxParameters(
                                    void*   xml, 
                                    float*  pressOrigin,  
                                    float*  pressThickness,  
                                    float*  pressAngles  
        ); 

int   svkSatBandsXML_GetAutoSat(
                                    void*   xml, 
                                    int     satNumber, 
                                    float*  normalX, 
                                    float*  normalY, 
                                    float*  normalZ, 
                                    float*  thickness,  
                                    float*  distance  
        ); 

int  svkSatBandsXML_GetAutoSatParameters(
                                    void* xml, 
                                    int satNumber, 
                                    float* normal, 
                                    float* thickness, 
                                    float* distance
        ); 

#ifdef __cplusplus
}
#endif
