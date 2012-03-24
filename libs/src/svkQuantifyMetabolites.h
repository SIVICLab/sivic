/*
 *  Copyright © 2009-2012 The Regents of the University of California.
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


#ifndef SVK_QUANTIFY_METABOLITES_H
#define SVK_QUANTIFY_METABOLITES_H


#include <vtkObject.h>
#include <vtkObjectFactory.h>
#include <vtkInformation.h>

#include <svkImageData.h>
#include <svkMriImageData.h>
#include <svkMrsImageData.h>
#include <svkImageAlgorithm.h>
#include <svkDcmHeader.h>
#include <svkEnhancedMRIIOD.h>


namespace svk {


using namespace std;


/*! 
 *  Class to generate a set of metabolite maps representing quantified peak regions, ratios or z-scores.  
 *  This class uses other algorithms to perform the necessary quantification and outputs an array
 *  of metabolilte maps (svkMriImageData objects). 
 *  
 *  Requires an XML input file specifying the regions to quantify.  
 */
class svkQuantifyMetabolites: public svkImageAlgorithm
{

    public:

        static svkQuantifyMetabolites* New();
        vtkTypeRevisionMacro( svkQuantifyMetabolites, svkImageAlgorithm);

        void                                                SetXMLFileName( vtkstd::string xmlFileName );     
        void                                                SetVerbose( bool isVerbose );     
        vtkstd::vector< svkMriImageData* >*                 GetMetMaps();
        void                                                LimitToSelectedVolume(float fraction = 0.5001); 
        vtkstd::vector< vtkstd::vector< vtkstd::string > >  GetRegionNameVector();
        int                                                 GetIntFromString(vtkstd::string stringVal ); 
        float                                               GetFloatFromString(vtkstd::string stringVal ); 
        void                                                ModifyRegion( int regionID, float peakPPM, float widthPPM ); 
        static void                                         WriteDefaultXMLTemplate( string fileName, bool clobber = false ); 
        static string                                       GetDefaultXMLFileName(); 


    protected:

        svkQuantifyMetabolites();
        ~svkQuantifyMetabolites();

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
        void                                GenerateRegionMaps();
        void                                GenerateRatioMaps();
        void                                GenerateZScoreMaps();
        string                              ReplaceSlashesAndWhiteSpace( vtkstd::string inString); 
        void                                GetNumeratorAndDenominatorImages( 
                                                vtkXMLDataElement* ratioXML, 
                                                svkImageData* numeratorImage, 
                                                svkImageData* denominatorImage
                                            ); 


        //  Members:
        bool                                isVerbose; 
        vtkstd::string                      xmlFileName; 
        vtkstd::vector< svkMriImageData* >  metMapVector; 
        float                               useSelectedVolumeFraction;
        short*                              selectedVolumeMask;
        vtkXMLDataElement*                  mrsXML;
        vtkXMLDataElement*                  applicationXML;


        //  map of regions: region name, peak (ppm) and peak width (ppm)
        vtkstd::vector < vtkstd::vector< vtkstd::string > >  regionVector;



};


}   //svk


#endif //SVK_QUANTIFY_METABOLITES_H

