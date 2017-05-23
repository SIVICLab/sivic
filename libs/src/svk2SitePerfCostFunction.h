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


        /*!
         *
         */   
        svk2SitePerfCostFunction() 
        {
            this->InitNumberOfSignals(); 
            this->TR = 0;
        }


        /*!
         *  For a given set of parameter values, compute the model kinetics
         *  The params are unitless. 
         */   
        virtual void GetKineticModel( const ParametersType& parameters ) const
        {

            double T1all  = parameters[0];
            double Kpl    = parameters[1];
            double Ktrans = parameters[2];

            //  use model params and initial signal intensity to calculate the metabolite signals vs time 
            //  solved met(t) = met(0)*invlaplace(phi(t)), where phi(t) = sI - x. x is the matrix of parameters.

            //  Find arrival time time to peak pyrvaute/urea
			//arrivalTime = ARRIVAL_TIME;
            int arrivalTime = GetArrivalTime( this->GetSignal(0) ); 

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
                    this->GetModelSignal(0)[t] = this->GetSignalAtTime(0, t);
                    this->GetModelSignal(1)[t] = this->GetSignalAtTime(1, t);
                }

			    if (t >= arrivalTime ) {      

                    // PYRUVATE
					//	convolutionMat[t] = inputFunction[t]+convolutionMat[t];
                    this->GetModelSignal(0)[t] = this->GetSignalAtTime(0, arrivalTime) 
					    * exp( -((T1all) + Kpl) * ( t - arrivalTime) ) +  (1-exp(-Ktrans*t))*inputFunction[t];

                    // LACTATE  (same in both cost functions): 
                    //kineticModel1[t] = signal1[arrivalTime] 
                    this->GetModelSignal(1)[t] = this->GetSignalAtTime(1, arrivalTime) 
                        * exp( -( t - arrivalTime )*T1all) 
                        - this->GetSignalAtTime(0, arrivalTime) 
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
				this->GetModelSignal(2)[t] =   inputFunction[t]; //convolutionMat[t]* kineticModel2[t];
				// }
				//cout << "Estimated AIF(" << t << "): " <<  kineticModel2[t] << endl;
    
            }

        }


        /*!
         *  T1all
         *  Kpl
         *  Ktrans
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
            //  pyruvate,lactate and urea
            this->SetNumberOfSignals(3);
        }



        /*!
         *  Get the vector that contains the string identifier for each output port
         */
        virtual void InitOutputDescriptionVector(vector<string>* outputDescriptionVector ) const 
        {
            outputDescriptionVector->resize( this->GetNumberOfOutputPorts() );
            (*outputDescriptionVector)[0] = "pyr";
            (*outputDescriptionVector)[1] = "lac";
            (*outputDescriptionVector)[2] = "urea";
            (*outputDescriptionVector)[3] = "T1all";
            (*outputDescriptionVector)[4] = "Kpl";
            (*outputDescriptionVector)[5] = "Ktrans";
        }


        /*!
         *  Initialize the parameter upper and lower bounds for this model. 
         */
        virtual void InitParamBounds( vector<float>* lowerBounds, vector<float>* upperBounds,
            vector<vtkFloatArray*>* averageSigVector )
        {

            (*upperBounds)[0] = 28;     //  T1all
            (*lowerBounds)[0] =  8;     //  T1all

            (*upperBounds)[1] = .05;    //  Kpl
            (*lowerBounds)[1] = 0.000;  //  Kpl

            (*upperBounds)[2] = 0;      //  ktrans 
            (*lowerBounds)[2] = 0;      //  ktrans 

            (*upperBounds)[3] = 1;      //  k2 
            (*lowerBounds)[3] = 0;      //  k2

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

            //  Ktrans (rate), mult by TR
            this->paramScaleFactors[2] = this->TR;

            //  K2 (rate), mult by TR
            this->paramScaleFactors[3] = this->TR;
        }


    private: 

        /*!
         *
         */   
        int GetArrivalTime( float* firstSignal ) const
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
            //cout << "t: " << arrivalTime << " " << firstSignal[arrivalTime] << " " << numPts << endl;
	        return arrivalTime; 
        } 



};



#endif// SVK_2_SITE_PERF_COST_COST_FUNCTION_H
