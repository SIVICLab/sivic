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
 *      Beck Olson,
 */

#ifndef SVK_MRS_ZERO_ORDER_PHASE_COST_FUNCTION_H
#define SVK_MRS_ZERO_ORDER_PHASE_COST_FUNCTION_H


#include <svkPhaseCostFunction.h>
#include <svkMRSPeakPick.h>
#include <svkMetaboliteMap.h>


using namespace svk;


/*
 *  Cost function for ITK optimizer: 
 */
class svkMRSZeroOrderPhaseCostFunction : public svkPhaseCostFunction
{

    public:

        typedef svkMRSZeroOrderPhaseCostFunction            Self;
        typedef svkPhaseCostFunction                        Superclass;
        typedef itk::SmartPointer<Self>                     Pointer;
        typedef itk::SmartPointer<const Self>               ConstPointer;


        itkTypeMacro( svkMRSZeroOrderPhaseCostFunction, svkPhaseCostFunction);

        itkNewMacro( Self );


        svkMRSZeroOrderPhaseCostFunction() {
            this->zeroOrderPhasePeak = 0;  
        }


        /*!
         *  Cost function based on maximizing the sum of peak intensities 
         *      cost function for svkMRSAutoPhase::PhasingModel =  MAX_PEAK_HTS_0 = 2
         */
        MeasureType  GetZeroOrderPhaseCost_2( const ParametersType& parameters) const
        {

            double phi0 = parameters[0];
        
            double intensity = 0; 
            float cmplxPt[2];
            double tmp; 

            //  Get the sum of the peak hits for picked peaks: 
            //  maximize positive peak height (minimize negative peak ht) 
            for ( int peakNum = 0; peakNum < this->peaks->GetNumPeaks(); peakNum++ ) {

                int startPt;     
                int endPt;     
                int peakPt;     
                this->peaks->GetPeakDefinition( peakNum, &startPt, &peakPt, &endPt ); 

                // apply zero order phase to data in window, then get peak ht:  
                for ( int freq = startPt; freq <= endPt; freq++) {
                    this->copySpectrum->GetTupleValue(freq, cmplxPt);
                    svk::svkPhaseSpec::ZeroOrderPhase(phi0, cmplxPt);
                    this->copySpectrum->SetTuple(freq, cmplxPt); 
                }
                float peakHt = svkMetaboliteMap::GetPeakHt( 
                        static_cast<float*>(this->copySpectrum->GetVoidPointer(4)), 
                        startPt, 
                        endPt 
                ); 
                cout << "PEAK HT: " << peakHt << " @ " << phi0 * 180. / vtkMath::Pi() << endl;
                intensity += (-1 * peakHt); 
            } 

            MeasureType measure = intensity; 
            return measure;
        }


        /*  
         *  returns the cost function for the current param values: 
         *  typedef double MeasureType
         */
        virtual MeasureType  GetValue( const ParametersType & parameters ) const
        { 

            double cost;  

            // make a member variable (copy)
            //cout << "copy spectrum" << endl;
            this->copySpectrum->DeepCopy(this->spectrum); 
            cost = GetZeroOrderPhaseCost_2( parameters ); 

            MeasureType measure = cost; 
            //cout << "                          cost: " << measure << endl; 
            return measure;
        }


        /*
         *
         */  
        virtual unsigned int GetNumberOfParameters(void) const
        {
            return 1; 
        }
        

        void SetPeakPicker( svkMRSPeakPick* peaks )  
        {
            this->peaks = peaks;
        }


        /*
         *
         */
        void SetZeroOrderPhasePeak( int peakNum)
        {
            this->zeroOrderPhasePeak = peakNum;
        }

        
        /*
         *
         */
        int GetZeroOrderPhasePeak( )
        {
            return this->zeroOrderPhasePeak;
        }

        
        


    private: 
        svkMRSPeakPick*                 peaks;
        int                             zeroOrderPhasePeak;

};



#endif// SVK_MRS_ZERO_ORDER_PHASE_COST_FUNCTION_H
