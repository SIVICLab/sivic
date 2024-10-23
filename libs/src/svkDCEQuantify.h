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


#ifndef SVK_DCE_QUANTIFY_H
#define SVK_DCE_QUANTIFY_H


#include </usr/include/vtk/vtkObject.h>
#include </usr/include/vtk/vtkObjectFactory.h>
#include </usr/include/vtk/vtkInformation.h>

#include <svkImageData.h>
#include <svkMriImageData.h>
#include <svkMrsImageData.h>
#include <svkImageAlgorithm.h>
#include <svkImageAlgorithmWithPortMapper.h>
#include <svkDcmHeader.h>
#include <svkEnhancedMRIIOD.h>


namespace svk {


using namespace std;


/*! 
 *  Class to generate a set of DCE maps representing for example peak_ht, peak_time, slope, etc..  
 *  This class uses other algorithms to perform the necessary quantification and outputs an array
 *  of DCE maps (svkMriImageData objects). 
 */
class svkDCEQuantify: public svkImageAlgorithmWithPortMapper
{

    public:

        enum {
            INPUT_IMAGE = 0,
            START_TIME_PT,
            END_TIME_PT
        } svkDCEQuantifyInput;
        //  Outputports:  0 for base ht map
        //  Outputports:  1 for peak ht map
        //  Outputports:  2 for peak time map
        //  Outputports:  3 for up slope map
        //  Outputports:  4 for washout slope map
        enum {
            BASE_HT_MAP = 0,
            PEAK_HT_MAP,
            PEAK_TIME_MAP,
            UP_SLOPE_MAP,
            WASHOUT_SLOPE_MAP,
            WASHOUT_SLOPE_POS_MAP
        } svkDCEQuantifyOutput;

        static svkDCEQuantify* New();
        vtkTypeMacro( svkDCEQuantify, svkImageAlgorithmWithPortMapper);
        std::vector< svkMriImageData* >* GetDCEMaps();

        void    SetTimepointStart(int startPt);
        svkInt* GetTimepointStart( );

        void    SetTimepointEnd(int endPt);
        svkInt* GetTimepointEnd( );

    protected:

        svkDCEQuantify();
        ~svkDCEQuantify();

        virtual int RequestInformation(
                        vtkInformation* request,
                        vtkInformationVector** inputVector,
                        vtkInformationVector* outputVector
                    );
        virtual int RequestData( 
                        vtkInformation* request, 
                        vtkInformationVector** inputVector, 
                        vtkInformationVector* outputVector 
                    );

    private:

        //  Methods:
        virtual void UpdateProvenance();
        void         Quantify();
        void         GenerateDCEMaps();
        void         GenerateNormalizedMaps(); 

        //  Members:
        std::vector< svkMriImageData* >  dceMapVector; 

};


}   //svk


#endif //SVK_DCE_QUANTIFY_H

