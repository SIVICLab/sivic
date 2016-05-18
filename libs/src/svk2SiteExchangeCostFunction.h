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
        svk2SiteExchangeCostFunction() {
        }


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


        /*!
         *
         */
        virtual MeasureType  GetResidual( const ParametersType& parameters) const
        {
            //cout << "GUESS: " << parameters[0] << " " << parameters[1] << endl;;

            this->GetKineticModel( parameters );

            double residual = 0;

            // Find time to peak pyrvaute/urea
            int arrivalTime = GetArrivalTime( this->GetSignal(0) );

             
            for ( int t = arrivalTime; t < this->numTimePoints; t++ ) { 
                residual += ( this->GetSignalAtTime(0, t) - this->GetModelSignal(0)[t] )  * ( this->GetSignalAtTime(0, t) - this->GetModelSignal(0)[t] ); 
                residual += ( this->GetSignalAtTime(1, t) - this->GetModelSignal(1)[t] )  * ( this->GetSignalAtTime(1, t) - this->GetModelSignal(1)[t] );
            }

            // for now ignore the urea residual 
            //for ( int t = 0; t < this->numTimePoints-arrivalTime; t++ ) { 
                //residual += ( this->signal2[t] - this->kineticModel2[t] )  * ( this->signal2[t] - this->kineticModel2[t] );
            //}
            //cout << "RESIDUAL: " << residual << endl;

            MeasureType measure = residual ;

            return measure;
        }


        /*!
         *  For a given set of parameter values, compute the model kinetics
         */   
        virtual void GetKineticModel( const ParametersType& parameters ) const
        {

            double T1all  = parameters[0];
            double Kpl    = parameters[1];
            double Ktrans = parameters[2];
            double K2     = parameters[3];

            //  use model params and initial signal intensity to calculate the metabolite signals vs time 
            //  solved met(t) = met(0)*invlaplace(phi(t)), where phi(t) = sI - x. x is the matrix of parameters.

            //  Find time to peak pyrvaute/urea
            //arrivalTime = ARRIVAL_TIME;
            int arrivalTime = GetArrivalTime( this->GetSignal(0) );

            //set up Arterial Input function
            float  Ao    = 5000;
            float  alpha = .2;
            float  beta  = 4.0;
            int    TR    = 3; //sec
    
            float* inputFunction   = new float [this->numTimePoints];
            for(int t = 0;  t < this->numTimePoints; t++ ) {
                inputFunction [t] = Ao * powf((t),alpha) * exp(-(t)/beta);
            }
             
            
            //cout << "GUESSES: " << T1all << " " << Kpl  << endl;
  
            //  use fitted model params and initial concentration/intensity to calculate the lactacte intensity at 
            //  each time point
            //  solved met(t) = met(0)*invlaplace(phi(t)), where phi(t) = sI - x. x is the matrix of parameters.

            // DEFINE COST FUNCTION 
            for ( int t = 0; t < this->numTimePoints; t++ ) {

                if (t < arrivalTime ) {
                    //kineticModel0[t] = this->GetSignalAtTime(0, t); 
                    //kineticModel1[t] = this->GetSignalAtTime(1, t); 
                    this->GetModelSignal(0)[t] = this->GetSignalAtTime(0, t); 
                    this->GetModelSignal(1)[t] = this->GetSignalAtTime(1, t); 
                }

                if (t >= arrivalTime ) {      

                    // PYRUVATE 
                    this->GetModelSignal(0)[t] = this->GetSignalAtTime(0, arrivalTime) 
                        * exp( -((1/T1all) + Kpl) * ( t - arrivalTime) );

                    // LACTATE 
                    this->GetModelSignal(1)[t] = this->GetSignalAtTime(1, arrivalTime)         // T1 decay of lac signal
                        * exp( -( t - arrivalTime )/T1all) 
                        - this->GetSignalAtTime(0, arrivalTime )                    
                            * exp( -( t - arrivalTime )/T1all)
                            * ( exp( -Kpl * ( t - arrivalTime )) - 1 );

                }

                // UREA
                //determine convolution with arterial input function
                float* convolutionMat  = new float [this->numTimePoints];
                convolutionMat[0] = 0;
                for (int tau = -(this->numTimePoints); tau < (this->numTimePoints); tau ++){      
                    convolutionMat[t] = inputFunction[tau] * exp(-Ktrans * (t-tau)/K2) + convolutionMat[t-1]; 
                }

                this->GetModelSignal(2)[t] = Ktrans * TR * convolutionMat[t];
                  
                //cout << "Estimated Pyruvate(" << t << "): " <<  kineticModel0[t] << endl; 
    
            }

        }


        /*!
         *
         */   
        virtual unsigned int GetNumberOfParameters(void) const
        {
            int numParameters = 4;
            return numParameters;
        }



};


#endif// SVK_2_SITE_EXCHANGE_COST_COST_FUNCTION_H
