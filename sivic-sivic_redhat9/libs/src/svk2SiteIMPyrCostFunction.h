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

#ifndef SVK_2_SITE_IM_PYR_COST_COST_FUNCTION_H
#define SVK_2_SITE_IM_PYR_COST_COST_FUNCTION_H

#include <svkKineticModelCostFunction.h>


using namespace svk;


/*
 *  Cost function for ITK optimizer: 
 *  This represents a 2 site exchange model for conversion of pyr->lactate
 *  Implementation of model from:
 *      Kinetic modeling of hyperpolarized 13C1-pyruvate metabolism in normal rats and TRAMP mice   
 *      Zierhut, Matthew L, Yen, Yi-Fen,  Chen, Albert P,  Bok, Robert,   Albers, Mark J,   Zhang, Vickie
 *      Tropp, Jim,  Park, Ilwoo, Vigneron, Daniel B,  Kurhanewicz, John,  Hurd, Ralph E,  Nelson, Sarah J
 *  
 *     Only equation 1 of the above reference is implemented for initial fitting of Pyr signal to determine
 *          Rinj:       the injection rate 
 *          Kpyr:       the pyruvate signal decay rate 
 *          Tarrival:   the arrival time 
 */
class svk2SiteIMPyrCostFunction : public svkKineticModelCostFunction
{

    public:

        typedef svk2SiteIMPyrCostFunction            Self;
        itkNewMacro( Self );


        /*!
         *
         */
        svk2SiteIMPyrCostFunction() 
        {
            this->InitNumberOfSignals(); 
            this->TR = 0; 
        }


        /*!
         *  For a given set of parameter values, compute the model kinetics
         */   
        virtual void GetKineticModel( const ParametersType& parameters ) const
        {

            float Rinj     = parameters[0];         //  injection rate
            float Kpyr     = parameters[1];         //  Kpyr, signal decay from T1 and excitation
            float Tarrival = parameters[2];         //  arrival time in dimensionless time-point units   

            float injectionDuration = 14/3;         //  X seconds normalized by TR into time point space

            //cout << "TR: " << this->TR << endl;
            //cout << "ID: " << injectionDuration << endl;
            int Tend = static_cast<int>( vtkMath::Round(Tarrival + injectionDuration) ); 
            Tarrival = static_cast<int>( vtkMath::Round(Tarrival) ); 


            //  ==============================================================
            //  DEFINE COST FUNCTION 
            //  ==============================================================
            int PYR = 0; 
            //cout << "Tarrival: " << Tarrival << endl;
            //cout << "TEND:     " << Tend << endl;
            //cout << "SIGTEND:  " << this->GetSignalAtTime(PYR, Tend) << endl; 
            for ( int t = 0; t < this->numTimePoints; t++ ) {

                if ( t < Tarrival ) {
                    cout << "PRE T ARRIVAL" << endl;
                    this->GetModelSignal(PYR)[t] = 0.; //this->GetSignalAtTime(PYR, t);
                }

                if ( Tarrival <= t < Tend) {
                    // PYRUVATE 
                    this->GetModelSignal(PYR)[t] = (Rinj/Kpyr) * (1 - exp( -1 * Kpyr * (t - Tarrival)) ) ; 
                }

                if (t >= Tend) {      
                    // PYRUVATE 
                    this->GetModelSignal(PYR)[t] = this->GetSignalAtTime(PYR, Tend) * (exp( -1 * Kpyr * ( t - Tend) ) ); 
                }

            }

        }


        /*!
         *  Rinj 
         *  Kpyr 
         *  Tarrival
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
            //  pyruvate 
            this->SetNumberOfSignals(1);
        } 


        /*!
         *  Get the vector that contains the string identifier for each output port
         */
        virtual void InitOutputDescriptionVector(vector<string>* outputDescriptionVector ) const
        {
            outputDescriptionVector->resize( this->GetNumberOfOutputPorts() );
            //  Input Signal
            (*outputDescriptionVector)[0] = "pyr";

            //  These are the params from equation 1 of Zierhut:
            (*outputDescriptionVector)[1] = "Rinj";
            (*outputDescriptionVector)[2] = "Kpyr";
            (*outputDescriptionVector)[3] = "Tarrival";
        }


        /*!
         *  Initialize the parameter upper and lower bounds for this model. 
         */
        virtual void InitParamBounds( vector<float>* lowerBounds, vector<float>* upperBounds,
            vector<vtkFloatArray*>* averageSigVector )
        {

            //  These are the params from equation 1 of Zierhut:
            (*upperBounds)[0] =  100000000;     //  Rinj (arbitrary unit signal rise)
            (*lowerBounds)[0] =  10000;         //  Rinj
        
            (*upperBounds)[1] = 0.20;           //  Kpyr
            (*lowerBounds)[1] = 0.0001;         //  Kpyr

            (*upperBounds)[2] =  0;             //  Tarrival
            (*lowerBounds)[2] = -4.00;          //  Tarrival

            //  ================================
            //  Make dimensionless: Do not edit/remove this
            //  ================================
            this->MakeBoundsDimensionless( lowerBounds, upperBounds);
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

            //  Tarrival (time ), divide by TR
            this->paramScaleFactors[2] = 1./this->TR;
        }


    private: 

};


#endif// SVK_2_SITE_IM_PYR_COST_COST_FUNCTION_H
