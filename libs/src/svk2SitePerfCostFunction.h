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

#ifndef SVK_2_SITE_PERF_COST_COST_FUNCTION_H
#define SVK_2_SITE_PERF_COST_COST_FUNCTION_H

#include <svkKineticModelCostFunction.h>


using namespace svk;

#define ARRIVAL_TIME 1

/*
 *  Cost function for ITK optimizer: 
 */
class svk2SitePerfCostFunction : public svkKineticModelCostFunction
{

    public:

        typedef svk2SitePerfCostFunction            Self;
        itkNewMacro( Self );


        svk2SitePerfCostFunction() {
        }

        int GetArrivalTime( float* firstSignal, int numPts ) const
        {
            int arrivalTime = 0;
            float maxValue0 = firstSignal[0];
            int t; 
            for(t = arrivalTime;  t < numPts; t++ ) {
                if( firstSignal[t] > maxValue0) {
                    maxValue0 = firstSignal[t];
                    arrivalTime = t;
                }
            }
            //cout << "t: " << arrivalTime << " " << firstSignal[arrivalTime] << " " << numPts << endl;
	        return arrivalTime; 
        } 


        virtual MeasureType  GetResidual( const ParametersType& parameters) const
        {
            //cout << "GUESS: " << parameters[0] << " " << parameters[1] << endl;;

            this->GetKineticModel( parameters,
                                    this->kineticModel0, 
                                    this->kineticModel1,
                                    this->kineticModel2,
                                    this->signal0,
                                    this->signal1,
                                    this->signal2,
                                    this->numTimePoints );

            double residual = 0;

            int arrivalTime = GetArrivalTime( this->signal0, this->numTimePoints ); 


            for ( int t = arrivalTime; t < this->numTimePoints; t++ ) {
			    //for ( int t = 0; t < this->numTimePoints; t++ ) { 
                residual += ( this->signal0[t] - this->kineticModel0[t] )  * ( this->signal0[t] - this->kineticModel0[t] ); 
                residual += ( this->signal1[t] - this->kineticModel1[t] )  * ( this->signal1[t] - this->kineticModel1[t] );
            }

            // for now ignore the urea residual 
			// for ( int t = 0; t < this->numTimePoints-arrivalTime; t++ ) { 
			//     residual += ( this->signal2[t] - this->kineticModel2[t] )  * ( this->signal2[t] - this->kineticModel2[t] );
			// }

            //cout << "RESIDUAL: " << residual << endl;

            MeasureType measure = residual ;

            return measure;
        }



        virtual void GetKineticModel( const ParametersType& parameters,
                    float* kineticModel0,
                    float* kineticModel1,
                    float* kineticModel2,
                    float* signal0,
                    float* signal1,
                    float* signal2,
                    int numTimePoints
        ) const 
        {

            double T1all  = parameters[0];
            double Kpl    = parameters[1];
            double Ktrans = parameters[2];
            double K2     = parameters[3];

            //  use model params and initial signal intensity to calculate the metabolite signals vs time 
            //  solved met(t) = met(0)*invlaplace(phi(t)), where phi(t) = sI - x. x is the matrix of parameters.

            //  Find arrival time time to peak pyrvaute/urea
			//arrivalTime = ARRIVAL_TIME;
            int arrivalTime = GetArrivalTime( signal0, numTimePoints ); 

            //set up Arterial Input function ( from 2siteex
            //float  Ao    = 5000;
            //float  alpha = .2;
            //float  beta  = 4.0;

            //set up Arterial Input function
            float  Ao    = 1e10;
            float  alpha = .3;
            float  beta  = 2.0;
   
            float* inputFunction   = new float [numTimePoints];
            for(int t = 0;  t < numTimePoints; t++ ) {
                inputFunction [t] = Ao * powf((t),alpha) * exp(-(t)/beta);
            }
             
            //float* convolutionMat  = new float [numTimePoints];
            //convolutionMat[0] = 0;
            //cout << "GUESSES: " << T1all << " " << Kpl  << endl;
  
            //  use fitted model params and initial concentration/intensity to calculate the lactacte intensity at 
            //  each time point
            //  solved met(t) = met(0)*invlaplace(phi(t)), where phi(t) = sI - x. x is the matrix of parameters.

            // DEFINE COST FUNCTION 
            for ( int t = 0; t < numTimePoints; t++ ) {
			  
                if (t < arrivalTime ){
                    kineticModel0[t] = signal0[t]; 
                    kineticModel1[t] = signal1[t]; 
                }

			    if (t >= arrivalTime ) {      

                    // PYRUVATE
					//	convolutionMat[t] = inputFunction[t]+convolutionMat[t];
                    kineticModel0[t] = signal0[arrivalTime] 
					    * exp( -((T1all) + Kpl) * ( t - arrivalTime) ) +  (1-exp(-Ktrans*t))*inputFunction[t];

                    // LACTATE  (same in both cost functions): 
                    kineticModel1[t] = signal1[arrivalTime] 
                        * exp( -( t - arrivalTime )*T1all) 
                        - signal0[ arrivalTime ] 
                        * exp( -( t - arrivalTime )*T1all)
                        * ( exp( -Kpl * ( t - arrivalTime )) - 1 );

                }



                //  UREA
                //  determine convolution with arterial input function
				//  convolutionMat[0] = 0;
				//  for (int tau = -(numTimePoints); tau < (numTimePoints); tau ++){      
				//     convolutionMat[t] = inputFunction[tau] * exp(-Ktrans * (t-tau)/K2) + convolutionMat[t-1]; 
				//  }
				//  kineticModel2[t] =  Ktrans * TR * convolutionMat[t]; 

			    //  convolutionMat[t] = inputFuntion[t]*(1-exp(Ktrans*t));
			    //  kineticModel2[t] = 0;
													   
                //for (int tau = 0; tau < t; tau ++){						
				kineticModel2[t] =   inputFunction[t]; //convolutionMat[t]* kineticModel2[t];
				// }
				//cout << "Estimated AIF(" << t << "): " <<  kineticModel2[t] << endl;
    
            }

        }


        virtual unsigned int GetNumberOfParameters(void) const
        {
            int numParameters = 4;
            return numParameters;
        }



};



#endif// SVK_2_SITE_PERF_COST_COST_FUNCTION_H
