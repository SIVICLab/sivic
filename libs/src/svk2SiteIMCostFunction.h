/*
 *  Copyright © 2009-2016 The Regents of the University of California.
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

#ifndef SVK_2_SITE_IM_COST_COST_FUNCTION_H
#define SVK_2_SITE_IM_COST_COST_FUNCTION_H

#include <svkKineticModelCostFunction.h>


using namespace svk;


/*
 *  Cost function for ITK optimizer: 
 *  This represents a 2 site exchange model for conversion of pyr->lactate
 *  Implementation of model from:
 *      Kinetic modeling of hyperpolarized 13C1-pyruvate metabolism in normal rats and TRAMP mice   
 *   Zierhut, Matthew L, Yen, Yi-Fen,  Chen, Albert P,  Bok, Robert,   Albers, Mark J,   Zhang, Vickie
 *  Tropp, Jim,  Park, Ilwoo, Vigneron, Daniel B,  Kurhanewicz, John,  Hurd, Ralph E,  Nelson, Sarah J
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

            float injectionDuration = 14/3;         //  X seconds normalized by TR into time point space
            int Tend = static_cast<int>( roundf(Tarrival + injectionDuration) );
            Tarrival = static_cast<int>( roundf(Tarrival) );

            //cout << "Tarrival: " << Tarrival << endl;
            //cout << "TEND:     " << Tend << endl;

            //  ==============================================================
            //  DEFINE COST FUNCTION 
            //  ==============================================================
            int PYR = 0; 
            int LAC = 1; 
            for ( int t = 0; t < this->numTimePoints; t++ ) {

                if ( t < Tarrival ) {
                    this->GetModelSignal(PYR)[t] = 0; //this->GetSignalAtTime(PYR, t);
                    this->GetModelSignal(LAC)[t] = 0; //this->GetSignalAtTime(LAC, t);
                }

                if ( Tarrival <= t < Tend) {

                    // PYRUVATE 
                    this->GetModelSignal(PYR)[t] = (Rinj/Kpyr) * (1 - exp( -1 * Kpyr * (t - Tarrival)) ) + dc; 

                    // LACTATE  
                    this->GetModelSignal(LAC)[t] = ( (Kpl * Rinj)/(Kpyr - Klac) ) 
                            * (
                                ( ( 1 - exp( -1 * Klac * ( t - Tarrival)) )/Klac ) 
                              - ( ( 1 - exp( -1 * Kpyr * ( t - Tarrival)) )/Kpyr )
                              ) + dc;    
                }

                if (t >= Tend) {      

                    // PYRUVATE 
                    this->GetModelSignal(PYR)[t] = this->GetSignalAtTime(PYR, Tend) * (exp( -1 * Kpyr * ( t - Tend) ) ) + dc; 

                    // LACTATE 
                    this->GetModelSignal(LAC)[t] = ( ( this->GetSignalAtTime(LAC, Tend) * Kpl ) / ( Kpyr - Klac ) ) 
                            * ( exp( -1 * Klac * (t-Tend)) - exp( -1 * Kpyr * (t-Tend)) ) 
                            + this->GetSignalAtTime(LAC, Tend) *  exp ( -1 * Klac * ( t - Tend)) + dc; 

                }

            }

        }


        /*!
         *  T1all
         *  Kpl
         */   
        virtual unsigned int GetNumberOfParameters(void) const
        {
            int numParameters = 6;
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
        }


        /*!
         *  Initialize the parameter uppler and lower bounds for this model. 
         *  All params are dimensionless and scaled by TR
         */     
        virtual void InitParamBounds( float* lowerBounds, float* upperBounds ) 
        {

            //  These are the params from equation 1 of Zierhut:
            upperBounds[0] =  100000000     * this->TR;     //  Rinj (arbitrary unit signal rise)
            lowerBounds[0] =  1000          * this->TR;     //  Rinj
        
            upperBounds[1] = 0.20           * this->TR;     //  Kpyr
            lowerBounds[1] = 0.0001         * this->TR;     //  Kpyr

            upperBounds[2] =  0             / this->TR;     //  Tarrival
            lowerBounds[2] = -4.00          / this->TR;     //  Tarrival

            //  These are the params from equation 2 of Zierhut:
            upperBounds[3] = 0.08           * this->TR;     //  Kpl
            lowerBounds[3] = 0.0001         * this->TR;     //  Kpl

            upperBounds[4] = 0.20           * this->TR;     //  Klac
            lowerBounds[4] = 0.0001         * this->TR;     //  Klac

            //  baseline
            upperBounds[5] =  100000;                       //  Baseline
            lowerBounds[5] = -100000;                       //  Baseline
        }   


       /*!
        *   Initialize the parameter initial values (dimensionless, scaled by TR)
        */
        virtual void InitParamInitialPosition( ParametersType* initialPosition )
        {
            if (this->TR == 0 )  {
                cout << "ERROR: TR Must be set before initializing parameters" << endl;
                exit(1); 
            }
            //  These are the params from equation 1 of Zierhut:
            (*initialPosition)[0] =  50000      * this->TR;    // Rinj    (1/s)
            (*initialPosition)[1] =  0.15       * this->TR;    // Kpyr    (1/s)  
            (*initialPosition)[2] =   -3        / this->TR;    // Tarrival (s)  

            //  These are the params from equation 2 of Zierhut:
            (*initialPosition)[3] =  0.01       * this->TR;    // Kpl     (1/s)  
            (*initialPosition)[4] =  0.05       * this->TR;    // Klac    (1/s)  
            (*initialPosition)[5] =  70000;                    // Baseilne (a.u.)  
        } 


       /*!
        *   Get the scaled (with time units) final fitted param values. 
        */
        virtual void GetParamFinalScaledPosition( ParametersType* finalPosition )
        {
            if (this->TR == 0 )  {
                cout << "ERROR: TR Must be set before scaling final parameters" << endl;
                exit(1); 
            }

            //  These are the params from equation 1 of Zierhut:
            (*finalPosition)[0] /= this->TR;    // Rinj     (1/s)
            (*finalPosition)[1] /= this->TR;    // Kpyr     (1/s)  
            (*finalPosition)[2] *= this->TR;    // Tarrival (s)  

            //  These are the params from equation 2 of Zierhut:
            (*finalPosition)[3] /= this->TR;    // Kpl      (1/s)  
            (*finalPosition)[4] /= this->TR;    // Klac     (1/s)  
        } 


    private: 

};


#endif// SVK_2_SITE_IM_COST_COST_FUNCTION_H
