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

#ifndef SVK_2_SITE_EXCHANGE_COST_COST_FUNCTION_H
#define SVK_2_SITE_EXCHANGE_COST_COST_FUNCTION_H

#include <svkKineticModelCostFunction.h>


using namespace svk;


/*
 *  Cost function for ITK optimizer: 
 *  This represents a 2 site exchange model for conversion of pyr->lactate
 */
class svk2SiteExchangeCostFunction : public svkKineticModelCostFunction
{

    public:

        typedef svk2SiteExchangeCostFunction            Self;
        itkNewMacro( Self );


        /*!
         *
         */
        svk2SiteExchangeCostFunction() 
        {
            //  this model has 2 signals, Pyr and Lac 
            this->InitNumberOfSignals(); 
            this->TR = 0; 
        }


        /*!
         *  For a given set of parameter values, compute the model kinetics
         */   
        virtual void GetKineticModel( const ParametersType& parameters ) const
        {

            double T1all  = parameters[0];
            double Kpl    = parameters[1];
            float  dc     = parameters[2];    //  dc baseline offset                                


            //  cout << "GUESSES: " << T1all << " " << Kpl  << endl;
            //  use model params and initial signal intensity to calculate the metabolite signals vs time 
            //  solved met(t) = met(0)*invlaplace(phi(t)), where phi(t) = sI - x. x is the matrix of parameters.

            //  Find time to peak pyrvaute/urea
            int arrivalTime = GetArrivalTime( this->GetSignal(0) );
  
            //  Use fitted model params and initial concentration/intensity to calculate the lactacte intensity at 
            //  each time point

            //  ==============================================================
            //  DEFINE COST FUNCTION 
            //  Pre arrival time and post arrival time are separate functions
            //      - Before the arrival time the pyr and lac models are just the 
            //        observed input signals. 
            //      - At and after the arrival time the model is: 
            //          pyr(t) = pyr(arrivalTime) * e^(-kt)  
            //              where k is the sum of contributions from T1 decay and convsion to lactate at rate Kpl
            //          lac(t) = lac(arrivalTime) * e^(-kt)      
            //  ==============================================================
            for ( int t = 0; t < this->numTimePoints; t++ ) {

                if (t < arrivalTime ) {
                    this->GetModelSignal(0)[t] = this->GetSignalAtTime(0, t); 
                    this->GetModelSignal(1)[t] = this->GetSignalAtTime(1, t); 
                }

                if (t >= arrivalTime ) {      

                    // PYRUVATE 
                    this->GetModelSignal(0)[t] = this->GetSignalAtTime(0, arrivalTime) 
                        * exp( -((1/T1all) + Kpl) * ( t - arrivalTime) ) + dc;

                    // LACTATE 
                    this->GetModelSignal(1)[t] = this->GetSignalAtTime(1, arrivalTime)         // T1 decay of lac signal
                        * exp( -( t - arrivalTime )/T1all) 
                        - this->GetSignalAtTime(0, arrivalTime )                    
                            * exp( -( t - arrivalTime )/T1all)
                            * ( exp( -Kpl * ( t - arrivalTime )) - 1 ) + dc;

                }

            }

        }


        /*!
         *  T1all
         *  Kpl
         *  DC offset
         */   
        virtual unsigned int GetNumberOfParameters(void) const
        {
            int numParameters = 3;
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
            (*outputDescriptionVector)[2] = "T1all";
            (*outputDescriptionVector)[3] = "Kpl";
            (*outputDescriptionVector)[4] = "dcoffset";
        }


        /*!
         *  Initialize the parameter uppler and lower bounds for this model. 
         *  All params are dimensionless and scaled by TR
         */
        virtual void InitParamBounds( vector<float>* lowerBounds, vector<float>* upperBounds,
            vector<vtkFloatArray*>* averageSigVector )
        {
            (*upperBounds)[0] = 50.;       //  T1all
            (*lowerBounds)[0] = 1. ;       //  T1all
        
            (*upperBounds)[1] = 0.20 ;     //  Kpl
            (*lowerBounds)[1] = 0.00 ;     //  Kpl

            (*upperBounds)[2] =  100000;   //  Baseline
            (*lowerBounds)[2] = -100000;   //  Baseline

        }   


        /*!
         *  Define the scale factors required to make the params dimensionless
         *  (scaled for point rather than time domain)   
         */
        virtual void InitParamScaleFactors()
        {
            //  T1 all ( time ), divide by TR
            this->paramScaleFactors[0] = 1./this->TR;

            //  Kpl (rate), mult by TR
            this->paramScaleFactors[1] = this->TR;

            //  baseline (dimensionless), do not scale
            this->paramScaleFactors[2] = 1.;
        }



    private: 

        /*!
         *  
         */
        int GetArrivalTime( float* firstSignal) const
        {
            int arrivalTime = 0;
            float maxValue0 = firstSignal[0];
            int t;
                    
            for(t = arrivalTime;  t < this->numTimePoints; t++ ) {
                if( firstSignal[t] > maxValue0) {
                    maxValue0 = firstSignal[t];
                    arrivalTime = t;
                }
            }
            //cout << "t: " << arrivalTime << " " << firstSignal[arrivalTime] << " " << this->numTimePoints<< endl;
            return arrivalTime;
        }



};


#endif// SVK_2_SITE_EXCHANGE_COST_COST_FUNCTION_H
