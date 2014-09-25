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


#ifndef SVK_SAT_BANDS_XML_H
#define SVK_SAT_BANDS_XML_H


#ifdef __cplusplus

#include <vtkObject.h>
#include <vtkXMLDataElement.h>

#include "svkTypes.h"


namespace svk {


using namespace std;


/*! 
 *  Class to manage IO to/from XML representation of sat bands. 
 */
class svkSatBandsXML: public vtkObject
{

    public:

        static svkSatBandsXML* New();
        vtkTypeRevisionMacro( svkSatBandsXML, vtkObject);

        void                        SetXMLFileName( vtkstd::string xmlFileName );     
        vtkstd::string              GetXMLFileName( );
        void                        ClearXMLFile( );
        void                        SetVerbose( bool isVerbose );     
        int                         GetNumberOfPressBoxSats(); 
        int                         GetNumberOfAutoSats(); 
        void                        GetPressBoxSat(
                                        int     satNumber, 
                                        string* label, 
                                        float*  normalX, 
                                        float*  normalY, 
                                        float*  normalZ, 
                                        float*  thickness,  
                                        float*  distance  
                                    ); 
        void                        GetAutoSat(
                                        int     satNumber, 
                                        string* label, 
                                        float*  normalX, 
                                        float*  normalY, 
                                        float*  normalZ, 
                                        float*  thickness,  
                                        float*  distance  
                                    ); 

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

        
        //  Members:
        bool                        isVerbose; 
        vtkstd::string              xmlFileName; 
        vtkXMLDataElement*          satBandsXML;
        vtkXMLDataElement*          versionElement;
        vtkXMLDataElement*          pressBoxElement;
        vtkXMLDataElement*          autoSatsElement;

        //  map of regions: region name, peak (ppm) and peak width (ppm)
        //vtkstd::vector < vtkstd::vector< vtkstd::string > >  regionVector;

};


}   //svk
#endif

#endif //SVK_SAT_BANDS_XML_H

#ifdef __cplusplus
extern "C" {
#endif


void* svkSatBandsXML_New(char* xmlFileName); 
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

void  svkSatBandsXML_GetAutoSat(
                                    void*   xml, 
                                    int     satNumber, 
                                    float*  normalX, 
                                    float*  normalY, 
                                    float*  normalZ, 
                                    float*  thickness,  
                                    float*  distance  
        ); 


#ifdef __cplusplus
}
#endif
