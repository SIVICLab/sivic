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

#ifndef SVK_PHASE_COST_FUNCTION_H
#define SVK_PHASE_COST_FUNCTION_H




#include </usr/include/vtk/vtkMath.h>

#include <svkMRSAutoPhase.h>
#include <svkPhaseSpec.h>

#include <math.h>


using namespace svk;

//int POWELL_CALLS_TO_GET_VALUE = 0;


/*
 *  Cost function for ITK optimizer: 
 */
class svkPhaseCostFunction : public itk::SingleValuedCostFunction 
{

    public:


        typedef svkPhaseCostFunction            Self;
        typedef itk::SingleValuedCostFunction   Superclass;
        typedef itk::SmartPointer<Self>         Pointer;
        typedef itk::SmartPointer<const Self>   ConstPointer;
        //itkNewMacro( Self );
        itkTypeMacro( svkPhaseCostFunction, SingleValuedCostFunction );

        typedef Superclass::ParametersType      ParametersType;
        typedef Superclass::DerivativeType      DerivativeType;
        typedef Superclass::MeasureType         MeasureType ;
        
        svkPhaseCostFunction() 
        {
            this->copySpectrum = vtkFloatArray::New();
        }


        void GetDerivative( const ParametersType & ,
                            DerivativeType &  ) const
        {
        }


        /*  
         *  returns the cost function for the current param values: 
         *  typedef double MeasureType
         */
        virtual MeasureType  GetValue( const ParametersType & parameters ) const = 0; 

        /*
         *
         */  
        virtual unsigned int GetNumberOfParameters(void) const = 0; 

        /*
         *
         */  
        void SetSpectrum( vtkFloatArray* spectrum )  
        {
            this->spectrum = spectrum;
        }


        /*
         *
         */  
        void SetNumFreqPoints( int numFreqPoints )  
        {
            this->numFreqPoints = numFreqPoints;
        }


    protected:

            vtkFloatArray*                  spectrum;
            vtkFloatArray*                  copySpectrum;
            int                             numFreqPoints; 


};



#endif// SVK_PHASE_COST_FUNCTION_H
