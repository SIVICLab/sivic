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

#ifndef SVK_KINETIC_MODEL_COST_FUNCTION_H
#define SVK_KINETIC_MODEL_COST_FUNCTION_H

#include <itkParticleSwarmOptimizer.h>
using namespace svk;


/*
 *  Cost function for ITK optimizer: 
 *  Abstract base class for kinetic modeling cost functions.  
 */
class svkKineticModelCostFunction : public itk::SingleValuedCostFunction 
{

    public:

        typedef svkKineticModelCostFunction             Self;
        typedef itk::SingleValuedCostFunction           Superclass;
        typedef itk::SmartPointer<Self>                 Pointer;
        typedef itk::SmartPointer<const Self>           ConstPointer;
        itkTypeMacro( svkKineticModelCostFunction, SingleValuedCostFunction );

        typedef Superclass::ParametersType              ParametersType;
        typedef Superclass::DerivativeType              DerivativeType;
        typedef Superclass::MeasureType                 MeasureType;


        /*
         *
         */
        svkKineticModelCostFunction()
        {
            this->numTimePoints = 0;
            this->numSignals    = 0;
        }


        /*
         *
         */
        ~svkKineticModelCostFunction()
        {
        }


        /*
         *
         */
        void GetDerivative( const ParametersType & ,
                            DerivativeType &  ) const
        {
        }


        /*!  
         *  Returns the cost function value for the current param values: 
         *  typedef double MeasureType
         */
        MeasureType  GetValue( const ParametersType & parameters ) const
        {
            double cost = GetResidual( parameters );
            MeasureType measure = cost;
            return measure;
        }


        /*!
         *  Set the kinetic signal for metabolite N
         */
        void SetNumberOfSignals( int numSignals )
        {
            this->numSignals = numSignals; 
            this->signalVector.resize(numSignals); 
            this->modelSignalVector.resize(numSignals); 
        }


        /*
         *  Return the number of signals in the concrete model instance
         */
        int GetNumberOfSignals( ) const
        {
            return this->numSignals; 
        }


        /*
         *  Set the kinetic signal for metabolite N
         */
        void SetSignal( float* signal, int signalIndex, string signalName)
        {
            map<string, float*>  signalMap;  
            signalMap[signalName] = signal; 
            if (signalIndex >  this->signalVector.size() ) {
                cout << "ERROR, signal index is out of range" << endl; 
                exit(1); 
            }
            this->signalVector[signalIndex] = signalMap;
        }


        /*
         *  Get pointer to the vector of input kinetic signals
         */
        vector < map < string, float* > >* GetSignals( )
        {
            return &(this->signalVector);
        }


        /*
         *  Get the specific signal at the specified time point.
         */
        float GetSignalAtTime(int signalIndex, int timePt) const
        {
            //map< string, float*>::iterator mapIter;
            return  (this->signalVector[ signalIndex ].begin()->second)[timePt]; 
        }


        /*
         *  Get the specific signal at the specified time point.
         */
        string GetSignalName( int signalIndex ) const
        {
            return  this->signalVector[ signalIndex ].begin()->first; 
        }


        /*
         *  Get the specific signal (pointer to the float array)
         */
        float* GetSignal(int signalIndex) const
        {
            return  (this->signalVector[ signalIndex ]).begin()->second; 
        }


        /*
         *  Initializes the arrays that will hold the fitted data
         *  with the correct number of points. 
         */
        void SetNumTimePoints( int numTimePoints )
        {
            this->numTimePoints = numTimePoints;
            if ( this->numSignals == 0 ) { 
                cout << "ERROR, numSignals not yet initialized" << endl;
                exit(1); 
            }
            for ( int i = 0; i < this->numSignals; i++ ) {   
                float* model = new float [this->numTimePoints];
                string modelSignalName = this->GetSignalName(i); 
                //cout << "NAME: " << modelSignalName << endl;
                this->SetModelSignal( model, i, modelSignalName); 
            }
        }


        /*!
         *  Sets the TR, and as this is tied to the param scale factors, this gets
         *  reinitizlized at this time as well as a side effect. 
         */
        void SetTR( float TR )
        {
            this->TR = TR; 

            int numModelParams = this->GetNumberOfParameters();
            this->paramScaleFactors.resize(numModelParams);
            this->InitParamScaleFactors(); 
        }


        /*
         *  Set the kinetic signal for metabolite N
         */
        void SetModelSignal( float* modelSignal, int modelSignalIndex, string modelSignalName)
        {
            map<string, float*>  modelSignalMap;  
            modelSignalMap[modelSignalName] = modelSignal; 
            if ( modelSignalIndex >  this->modelSignalVector.size() ) {
                cout << "ERROR, signal index is out of range" << endl; 
                exit(1); 
            }
            this->modelSignalVector[modelSignalIndex] = modelSignalMap;
        }


        /*
         *  Get the specific model at the specified time point.
         */
        float GetModelSignalAtTime(int modelSignalIndex, int timePt) const
        {
            return  (this->modelSignalVector[ modelSignalIndex ].begin()->second)[timePt]; 
        }


        /*
         *  Get the specific signal (pointer to the float array)
         */
        float* GetModelSignal(int modelSignalIndex) const
        {
            return  (this->modelSignalVector[ modelSignalIndex ]).begin()->second; 
        }

        /*!
         *  Get the number of outputs
         *  defined in sub class, but for example: 
         *  This is fitted signals (first), them parameter maps, e.g.:
         *      Outputports:  0 for fitted pyruvate kinetics
         *      Outputports:  1 for fitted lactate kinetics
         *      Outputports:  2 for fitted urea kinetics
         *      Outputports:  3 for T1all map 
         *      Outputports:  4 for Kpl map 
         *      Outputports:  5 for Ktrans map 
         *      Outputports:  5 for K2 map 
         */
        unsigned int GetNumberOfOutputPorts(void) const 
        {
            int numOutputPorts = this->GetNumberOfSignals() + this->GetNumberOfParameters(); 
            return numOutputPorts; 
        }


        /*!
         *  For a given set of parameter values, compute the model kinetics:
         *
         *  Function to calculate metabolite signal vs time based on parametric model.  
         *      uses the following : 
         *      parameters    = kinetic model parameters to be fit (1/T1all, Kpl).  
         *      kineticModelN = model metabolite signal intensity vs time ( pyr, lac, urea)
         *      signalN       = input signal vs time for each of 3 measured metabolites (pyr, lac, urea) 
         *      numTimePoints = number of observed time points.  
         */
        virtual void GetKineticModel( const ParametersType& parameters ) const = 0; 


        /*!
         *  Get the number of adjustable parameters in the model defined by cost function
         */
        virtual unsigned int GetNumberOfParameters(void) const = 0; 


        /*!
         *  Initialize the number of input signals for the model 
         */
        virtual void InitNumberOfSignals(void) = 0; 


        /*!
         *  Get the vector that contains the string identifier for each output port
         */
        virtual void InitOutputDescriptionVector(vector<string>* outputDescriptionVector ) const = 0; 

        /*!
         *  Initialize the parameter uppler and lower bounds for this model and scale them to be
         *  dimensionless. 
         */
        virtual void InitScaledParamBounds( 
            vector<float>* lowerBounds, 
            vector<float>* upperBounds,
            vector<vtkFloatArray*>* averageSigVector = NULL
        ) {
            this->InitParamBounds( lowerBounds, upperBounds, averageSigVector); 
            this->MakeBoundsDimensionless( lowerBounds, upperBounds);
        }; 


        /*!
         *  Initialize the parameter uppler and lower bounds for this model. 
         */
        virtual void InitParamBounds( 
            vector<float>* lowerBounds, 
            vector<float>* upperBounds,
            vector<vtkFloatArray*>* averageSigVector = NULL
        ) = 0; 


        /*!
         *  Initialize the parameter initial values (dimensionless, scaled by TR)
         *  Initial guesses are the mid point of the param bounds. 
         */
        virtual void InitParamInitialPosition( ParametersType* initialPosition,
            vector<float>* lowerBounds, vector<float>* upperBounds)
        {
            if (this->TR == 0 )  {
                cout << "ERROR: TR Must be set before initializing parameters" << endl;
                exit(1);
            }

            for ( int param = 0; param < this->GetNumberOfParameters(); param++ ) {
                (*initialPosition)[param] =  ( (*upperBounds)[param] + (*lowerBounds)[param]) / 2. ;
            }
        }


       /*!
        *   Get the scaled (with time units) final fitted param values.  This applies 
        *   the revese scaling by TR back to the dimensioned time space. 
        */
        virtual void GetParamFinalScaledPosition( ParametersType* finalPosition )
        {
            if (this->TR == 0 )  {
                cout << "ERROR: TR Must be set before scaling final parameters" << endl;
                exit(1);
            }

            float scaleFactor; 
            for ( int i = 0; i < this->GetNumberOfParameters(); i++ ) { 
                scaleFactor = this->paramScaleFactors[i];
                (*finalPosition)[i] /= scaleFactor;
            }
        }


        /*!
         *  Print param bounds and init point in real time dimension
         */
        void PrintParmBounds( ParametersType* initialPosition, vector<float>* lowerBounds, vector<float>* upperBounds, 
            vector<string>* outputDescriptionVector )
        {
            cout << endl;
            cout << "======================================================" << endl;
            cout << "Model Parameter Bounds => lower: upper (initial value)" << endl;
            cout << "======================================================" << endl;
            float scaleFactor;
            float startGuess;
            float lowerBound;
            float upperBound;
            int numSigs = this->GetNumberOfSignals(); 
            for ( int i = 0; i < this->GetNumberOfParameters(); i++ ) { 
                scaleFactor = this->paramScaleFactors[i];
                startGuess = (*initialPosition)[i] / scaleFactor; 
                lowerBound = (*lowerBounds)[i] / scaleFactor; 
                upperBound = (*upperBounds)[i] / scaleFactor; 
                cout << setw(3) << right << i << ". " << setw(10) << left 
                    <<  (*outputDescriptionVector)[i + numSigs] << " => " << setw(12) << lowerBound << " :  " 
                    << setw(12) << upperBound <<  " (" << startGuess << ")" << endl;
            }   
            cout << "======================================================" << endl;
            cout << endl;
        }


        /*!
         *  Override the default param bounds with custom settings
         */
        void SetCustomizedScaledParamBounds( 
            vector<float>* lowerBounds, 
            vector<float>* upperBounds, 
            vector<int>    customBoundsParamNumbers,
            vector<float>  customLowerBounds, 
            vector<float>  customUpperBounds  
        ) {
            this->MakeBoundsVectorsDimensionless( &customBoundsParamNumbers, &customLowerBounds, &customUpperBounds);

            for (int i = 0; i < customBoundsParamNumbers.size(); i++ ) {

                int paramNumber = customBoundsParamNumbers[i]; 
                (*lowerBounds)[paramNumber] = customLowerBounds[i]; 
                (*upperBounds)[paramNumber] = customUpperBounds[i];  
            }
        }



    protected:


        /*!
         *  Cost function used to minimizing the residual of fitted and observed dynamics. 
         *  Get the sum of square difference between the input signal and modeled signal at the current
         *  set of parameter values. 
         */
        MeasureType  GetResidual( const ParametersType& parameters) const
        {
            //cout << "GUESS: " << parameters[0] << " " << parameters[1] << endl;;
            this->GetKineticModel( parameters );
            double residual = 0;
            for ( int t = 0; t < this->numTimePoints; t++ ) {
                for  (int sigNumber = 0; sigNumber < this->GetNumberOfSignals(); sigNumber++ ) {
                    residual += ( this->GetSignalAtTime(sigNumber, t) - this->GetModelSignal(sigNumber)[t] )  
                              * ( this->GetSignalAtTime(sigNumber, t) - this->GetModelSignal(sigNumber)[t] );
                }
            }
            MeasureType measure = residual ;
            //cout << "MEASURE" << measure << endl;
            //for ( int t = 0; t < 8; t++ ) {
                //for  (int sigNumber = 0; sigNumber < this->GetNumberOfSignals(); sigNumber++ ) {
                    //cout << "DIFF( " << sigNumber << "): " << this->GetSignalAtTime(sigNumber, t)  << " - " <<  this->GetModelSignal(sigNumber)[t]  << " = " << ( this->GetSignalAtTime(sigNumber, t) - this->GetModelSignal(sigNumber)[t] ) << endl;   
                //}
            //}
            return measure;
        }


        /*
         *  factors to convert time domain params into dimensionless (point) space
         */
        virtual void InitParamScaleFactors() = 0; 

    
        /*!
         *  Wrapper method to add the boundsParamNumbers vector before passing to maek dimensionless. 
         */
        virtual void MakeBoundsDimensionless( vector<float>* lowerBounds, vector<float>* upperBounds)
        {
            vector<int> boundsParamNumbers;
            for ( int param = 0; param < this->GetNumberOfParameters(); param++ ) {
                boundsParamNumbers.push_back(param);
            }

            if ( this->TR == 0 )  {
                cout << "ERROR: TR Must be set before making bounds dimensionless " << endl;
                exit(1);
            }
            this->MakeBoundsVectorsDimensionless( &boundsParamNumbers, lowerBounds, upperBounds);
        }  


        /*!
         *  Make the bounds dimensionless (i.e. convert from tine domain 
         *  to point domain units).  These are the user specified bounds provided on the 
         *  command line
         *      rates are multilied by TR, 
         *      times are divided by TR 
         */
        virtual void MakeBoundsVectorsDimensionless( vector<int>*  boundsParamNumbers,
            vector<float>* lowerBounds, vector<float>* upperBounds)
        {
            //  loop over the bounds the user specified
            float scaleFactor;
            for (int i = 0; i < boundsParamNumbers->size(); i++ ) {

                int paramNumber = (*boundsParamNumbers)[i]; 
                scaleFactor = this->paramScaleFactors[paramNumber];

                (*lowerBounds)[i] *= scaleFactor; 
                (*upperBounds)[i] *= scaleFactor; 
            }
        }


        //  this is the vector of float arrays representing the observed metabolite signals as a function of time
        //  for different number of sites this should be flexibile
        vector< map < string, float* > >  signalVector;

        //  this is the vector of float arrays representing the modeled/computed metabolite signals as a function of time
        //  for different number of sites this should be flexibile based on the model parameters. 
        vector< map < string, float* > >    modelSignalVector;
        int                                 numTimePoints;
        int                                 numSignals;
        float                               TR;
        vector< float >                     paramScaleFactors;


};



#endif// SVK_KINETIC_MODEL_COST_FUNCTION_H
