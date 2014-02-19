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


#ifndef SVK_COIL_COMBINE_H
#define SVK_COIL_COMBINE_H


#include <vtkObject.h>
#include <vtkObjectFactory.h>
#include <vtkInformation.h>
#include <vtkStreamingDemandDrivenPipeline.h>

#include <svkSpecUtils.h>
#include <svkImageInPlaceFilter.h>
#include <svkPhaseSpec.h>


namespace svk {


using namespace std;



/*! 
 *  WARNING:  
 *  This Algorithm is a beta version for RSNA that simply adds the complex values from each coil without weighting. 

 *  Class that combines single coild data into a combined image.  Complex data must be in phase prior to combining for 
 *  constructive addition of complex data (see svkMultiCoilPhase).  Sum of squares outputs magnitude data and is not
 *  sensitive to the phase of the complex input data. The weighting factors used in the linear 
 *  combination may be set from a) spatially dependent coil sensitivity maps (experimentally derived or other), 
 *  b) peak amplitude (e.g. h20 peak), or constant. 
 *  
 *  This is(will be) based on methodes derived and validated in 
 *  the Sarah Nelson lab at UCSF, Department of Radiology and Biomedical Imaging. 
 *
 *  References:
 *      1.  "A Comparison of Two Phase Correction Strategies in Multi-Channel MRSI Reconstruction"
 *           W. Bian, J.C. Crane, W. Sohn, I. Park, E. Ozturk-Isik, S.J. Nelson ISMRM Annual Meeting, 2009 
 *           http://www.ismrm.org/09/Session14.htm
 *
 */
class svkCoilCombine : public svkImageInPlaceFilter
{

    public:

        static svkCoilCombine* New();
        vtkTypeRevisionMacro( svkCoilCombine, svkImageInPlaceFilter);


        typedef enum {
            ADDITION = 0,
            SUM_OF_SQUARES
        } CombinationMethod;

        typedef enum {
            COIL = 0,
            TIME  
        } CombinationDimension;

        void    SetCombinationMethod( CombinationMethod method);
        void    SetCombinationDimension( CombinationDimension dimension);


    protected:

        svkCoilCombine();
        ~svkCoilCombine();

        virtual int     FillInputPortInformation(int port, vtkInformation* info);


        //  Methods:
        virtual int     RequestInformation(
                            vtkInformation* request, 
                            vtkInformationVector** inputVector,
                            vtkInformationVector* outputVector
                        );
        virtual int     RequestData(
                            vtkInformation* request, 
                            vtkInformationVector** inputVector,
                            vtkInformationVector* outputVector
                        );


    private:

        void                    RedimensionData(); 
        void                    RequestAdditionData(); 
        void                    RequestSumOfSquaresData(); 

        CombinationMethod       combinationMethod; 
        CombinationDimension    combinationDimension; 


};


}   //svk


#endif //SVK_COIL_COMBINE_H

