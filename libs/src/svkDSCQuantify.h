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


#ifndef SVK_DSC_QUANTIFY_H
#define SVK_DSC_QUANTIFY_H


#include </usr/include/vtk/vtkObject.h>
#include </usr/include/vtk/vtkObjectFactory.h>
#include </usr/include/vtk/vtkInformation.h>

#include <svkImageData.h>
#include <svkMriImageData.h>
#include <svkMrsImageData.h>
#include <svkImageAlgorithm.h>
#include <svkDcmHeader.h>
#include <svkEnhancedMRIIOD.h>


namespace svk {


using namespace std;


/*! 
 *  Class to generate a set of DSC maps representing for example peak ht and % recovery. 
 *  This class uses other algorithms to perform the necessary quantification and outputs an array
 *  of dsc maps (svkMriImageData objects). 
 */
class svkDSCQuantify: public svkImageAlgorithm
{

    public:

        static svkDSCQuantify* New();
        vtkTypeMacro( svkDSCQuantify, svkImageAlgorithm);

        std::vector< svkMriImageData* >*                 GetDSCMaps();
        std::vector< std::vector< std::string > >  GetRegionNameVector();
        int                                                 GetIntFromString(std::string stringVal ); 
        float                                               GetFloatFromString(std::string stringVal ); 

    protected:

        svkDSCQuantify();
        ~svkDSCQuantify();

        virtual int             RequestInformation(
                                    vtkInformation* request,
                                    vtkInformationVector** inputVector,
                                    vtkInformationVector* outputVector
                                );
        virtual int             RequestData( 
                                    vtkInformation* request, 
                                    vtkInformationVector** inputVector, 
                                    vtkInformationVector* outputVector 
                                );


        virtual int             FillInputPortInformation( int vtkNotUsed(port), vtkInformation* info );
        virtual int             FillOutputPortInformation( int vtkNotUsed(port), vtkInformation* info ); 


    private:

        //  Methods:
        virtual void                        UpdateProvenance();
        void                                Quantify();
        void                                GenerateDSCMaps();
        void                                GenerateNormalizedMaps(); 
        string                              ReplaceSlashesAndWhiteSpace( std::string inString); 
        void                                GetNumeratorAndDenominatorImages( 
                                                vtkXMLDataElement* ratioXML, 
                                                svkImageData* numeratorImage, 
                                                svkImageData* denominatorImage
                                            ); 


        //  Members:
        bool                                isVerbose; 
        std::vector< svkMriImageData* >  dscMapVector; 


        //  map of regions: region name, peak (ppm) and peak width (ppm)
        std::vector < std::vector< std::string > >  regionVector;

};


}   //svk


#endif //SVK_DSC_QUANTIFY_H

