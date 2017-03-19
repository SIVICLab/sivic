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
 *      Christine Swisher, Ph.D.
 *      Sarah J. Nelson, Ph.D.
 *      Cornelius Von Morze, Ph.D. 
 *      Ilwoo Park, Ph.D. 
 *      Daniel B. Vigneron, Ph.D. 
 */

#ifndef SVK_2_SITE_IM_COST_COST_FUNCTION_H
#define SVK_2_SITE_IM_COST_COST_FUNCTION_H

#include <svkKineticModelCostFunction.h>


using namespace svk;


/*
 *  Cost function for ITK optimizer: 
 *  This represents a piecewise 2-site (I)ntegrated exchange (M)odel (IM) for conversion of pyr->lactate
 *  Implementation of model from:
 *      Zierhut, M. L. et al. 
 *      Kinetic modeling of hyperpolarized 13C1-pyruvate metabolism in normal rats and TRAMP mice. 
 *      J. Magn. Reson. 202, 85–92 (2010).
 */
class svk2SiteIMCostFunction : public svkKineticModelCostFunction
{

    public:

        typedef svk2SiteIMCostFunction            Self;
        itkNewMacro( Self );


        /*!
         *
         */
        svk2SiteIMCostFunction() 
        {
            this->InitNumberOfSignals(); 
            this->TR = 0; 
        }


        /*!
         *  For a given set of parameter values, compute the model kinetics
         */   
        virtual void GetKineticModel( const ParametersType& parameters ) const
        {

            float  Rinj     = parameters[0];    //  injection rate
            float  Kpyr     = parameters[1];    //  Kpyr, signal decay from T1 and excitation
            float  Tarrival = parameters[2];    //  arrival time    
            float  Kpl      = parameters[3];    //  Kpl conversion rate
            float  Klac     = parameters[4];    //  Klac, signal decay from T1 and excitation
            float  dc       = parameters[5];    //  dc baseline offset                                
            float  injDur   = parameters[6];    //  dc baseline offset                                

            //cout << "FINAL: Rinj     " << Rinj << endl;    
            //cout << "FINAL: Kpyr     " << Kpyr << endl;    
            //cout << "FINAL: Tarrival " << Tarrival << endl;    
            //cout << "FINAL: Kpl      " << Kpl << endl;    
            //cout << "FINAL: Klac     " << Klac << endl;    
            //cout << "FINAL: dc       " << dc << endl;    
            //cout << "FINAL: injDur   " << injDur << endl;    

            //float injectionDuration = 10/3;         //  X seconds normalized by TR into time point space
            float injectionDuration = injDur;         //  X seconds normalized by TR into time point space
            int Tend = static_cast<int>( vtkMath::Round(Tarrival + injectionDuration) );
            Tarrival = static_cast<int>( vtkMath::Round(Tarrival) );

            //cout << "Tarrival: " << Tarrival << endl;
            //cout << "Tend:     " << Tend << endl;
            //cout << "NTP:      " << this->numTimePoints << endl;

            //  ==============================================================
            //  DEFINE COST FUNCTION 
            //  ==============================================================
            int PYR = 0; 
            int LAC = 1; 
            for ( int t = 0; t < this->numTimePoints; t++ ) {

                if ( t < Tarrival ) {
                    this->GetModelSignal(PYR)[t] = 0; //this->GetSignalAtTime(PYR, t);
                    this->GetModelSignal(LAC)[t] = 0; //this->GetSignalAtTime(LAC, t);
                    //cout << " sig1: " << t << " " << this->GetModelSignal(PYR)[t] << endl;
                }

                if ( (Tarrival <= t) && (t < Tend) ) {

                    // PYRUVATE 
                    this->GetModelSignal(PYR)[t] = (Rinj/Kpyr) * (1 - exp( -1 * Kpyr * (t - Tarrival)) ) + dc; 

                    // LACTATE  
                    this->GetModelSignal(LAC)[t] = ( (Kpl * Rinj)/(Kpyr - Klac) ) 
                            * (
                                ( ( 1 - exp( -1 * Klac * ( t - Tarrival)) )/Klac ) 
                              - ( ( 1 - exp( -1 * Kpyr * ( t - Tarrival)) )/Kpyr )
                              ) + dc;    
                    //cout << " sig2: " << t << " " << this->GetModelSignal(PYR)[t] << endl;
                }

                if (t >= Tend) {      

                    // PYRUVATE 
                    this->GetModelSignal(PYR)[t] = this->GetSignalAtTime(PYR, Tend) * (exp( -1 * Kpyr * ( t - Tend) ) ) + dc; 

                    // LACTATE 
                    this->GetModelSignal(LAC)[t] = ( ( this->GetSignalAtTime(LAC, Tend) * Kpl ) / ( Kpyr - Klac ) ) 
                            * ( exp( -1 * Klac * (t-Tend)) - exp( -1 * Kpyr * (t-Tend)) ) 
                            + this->GetSignalAtTime(LAC, Tend) *  exp ( -1 * Klac * ( t - Tend)) + dc; 
                    //cout << " sig3: " << t << " " << this->GetModelSignal(PYR)[t] << endl;

                }

            }

        }


        /*!
         *  Get the number of parameters in the model. 
         */   
        virtual unsigned int GetNumberOfParameters(void) const
        {
            int numParameters = 7;
            return numParameters;
        }


        /*!
         *  Initialize the number of input signals for the model 
         */
        virtual void InitNumberOfSignals(void) 
        {
            //  pyruvate and lactate
            this->SetNumberOfSignals(2);
        } 


        /*!
         *  Get the vector that contains the string identifier for each output port
         */
        virtual void InitOutputDescriptionVector(vector<string>* outputDescriptionVector ) const
        {
            outputDescriptionVector->resize( this->GetNumberOfOutputPorts() );
            (*outputDescriptionVector)[0] = "pyr";
            (*outputDescriptionVector)[1] = "lac";
            //  These are the params from equation 1 of Zierhut:
            (*outputDescriptionVector)[2] = "Rinj";
            (*outputDescriptionVector)[3] = "Kpyr";
            (*outputDescriptionVector)[4] = "Tarrival";
            //  These are the params from equation 2 of Zierhut:
            (*outputDescriptionVector)[5] = "Kpl";
            (*outputDescriptionVector)[6] = "Klac";
            (*outputDescriptionVector)[7] = "dcoffset";
            (*outputDescriptionVector)[8] = "inj_dur";
        }


        /*!
         *  Initialize the parameter upper and lower bounds for this model. 
         *  All params are dimensionless and scaled by TR
         */     
        virtual void InitParamBounds( vector<float>* lowerBounds, vector<float>* upperBounds, 
            vector<vtkFloatArray*>* averageSigVector ) 
        {

            //  Rinj: 
            //      try to estimate a range for Rinj as the change in 
            //      intensity on the rising curve of hte pyr signal: 
            //  Tarrival: 
            //      shold be less than the maximum point in Pyr curve. 
            //  inj_dur: 
            //      estimate from the rise time
            int PYR = 0; 
            int vecLength        = (*averageSigVector)[PYR]->GetNumberOfTuples(); 
            double maxValue      = VTK_FLOAT_MIN;  
            int    maxValuePt    = 0;  
            int    maxValueTime  = 0;  
            double startValue    = (*averageSigVector)[PYR]->GetTuple1( 0 );
            for ( int i = 0; i < vecLength; i++ ) {
                double value = (*averageSigVector)[PYR]->GetTuple1( i ); 
                if ( value > maxValue ) {
                    maxValue = value; 
                    maxValuePt = i; 
                    maxValueTime = maxValuePt * this->TR; 
                } 
            }
            
            double rinjEstimate = maxValue - startValue;   
            if ( maxValuePt > 0 ) {
                rinjEstimate /= maxValuePt; 
            }  
                
            //  Rinj (s-1) 
            //  These are the params from equation 1 of Zierhut:
            (*upperBounds)[0] =  2    * rinjEstimate;   //  Rinj (units of signal rise per second )
            (*lowerBounds)[0] =  0.1  * rinjEstimate;   //  Rinj
        
            //  Kpyr (s-1)  
            //  These are the params from equation 2 of Zierhut:
            (*upperBounds)[1] = 0.10;         //  Kpyr
            (*lowerBounds)[1] = 0.04;         //  Kpyr

            //  Arrival time of bolus ( seconds )
            (*upperBounds)[2] =  maxValueTime;                          
            (*lowerBounds)[2] =  (maxValueTime - maxValue/(.5 * rinjEstimate)); 

            //  Kpl (s-1)
            //  These are the params from equation 2 of Zierhut:
            (*upperBounds)[3] = 0.08;     //  Kpl
            (*lowerBounds)[3] = 0.0001;   //  Kpl

            //  Klac (s-1).  
            //  These are the params from equation 2 of Zierhut:
            (*upperBounds)[4] = 0.10;     //  Klac
            (*lowerBounds)[4] = 0.04;     //  Klac

            //  baseline offset (unitless)
            double baselineValue = (*averageSigVector)[0]->GetTuple1( vecLength - 1 ); 
            (*upperBounds)[5] =  4 * baselineValue;            //  Baseline DC Offset
            (*lowerBounds)[5] =  0;                            //  Baseline DC Offset

            //  injection duration ( seconds ) 
            (*upperBounds)[6] = (maxValue/(.5*rinjEstimate));     //  injection duration 
            (*lowerBounds)[6] = (maxValue/(2 *rinjEstimate));     //  injection duration 
        }


        /*!
         *  Define the scale factors required to make the params dimensionless
         *  (scaled for point rather than time domain)   
         */
        virtual void InitParamScaleFactors()
        {
            //  Rinj (rate), mult by TR
            this->paramScaleFactors[0] = this->TR;   

            //  Kpyr (rate), mult by TR
            this->paramScaleFactors[1] = this->TR;   

            //  arrival_time (time), divide by TR
            this->paramScaleFactors[2] = 1./this->TR;   

            //  Kpl (rate), mult by TR 
            this->paramScaleFactors[3] = this->TR;   

            //  Klac (rate), mult by TR 
            this->paramScaleFactors[4] = this->TR;   

            //  baseline (dimensionless), do not scale
            this->paramScaleFactors[5] = 1.; 

            //  injection duration (time), divie by TR 
            this->paramScaleFactors[6] = 1./this->TR;   
        }

            

    private: 

};


#endif// SVK_2_SITE_IM_COST_COST_FUNCTION_H
