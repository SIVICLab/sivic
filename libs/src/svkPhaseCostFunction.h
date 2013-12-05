/*
 *  Copyright © 2009-2013 The Regents of the University of California.
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


int POWELL_CALLS_TO_GET_VALUE = 0;


#include <vtkMath.h>

#include <svkMRSAutoPhase.h>
#include <svkPhaseSpec.h>

#include <math.h>


using namespace svk;


/*
 *  Cost function for ITK optimizer: 
 */
class svkPhaseCostFunction : public itk::SingleValuedCostFunction 
{

    public:

        typedef svkPhaseCostFunction       Self;
        typedef itk::SingleValuedCostFunction   Superclass;
        typedef itk::SmartPointer<Self>         Pointer;
        typedef itk::SmartPointer<const Self>   ConstPointer;
        itkNewMacro( Self );
        itkTypeMacro( svkPhaseCostFunction, SingleValuedCostFunction );

        typedef Superclass::ParametersType      ParametersType;
        typedef Superclass::DerivativeType      DerivativeType;
        typedef Superclass::MeasureType         MeasureType ;
        void    SetModel( svkMRSAutoPhase::phasingModel model) {
            this->phasingModel = model; 
        }
        
        svkPhaseCostFunction() 
        {
            this->copySpectrum = vtkFloatArray::New();
            this->zeroOrderPhasePeak = 0;  
        }


        void GetDerivative( const ParametersType & ,
                            DerivativeType &  ) const
        {
        }


        /*!
         *  Cost function based on maximizing the intensity of the first FID point. 
         */
        MeasureType  GetFirstPointPhaseValue0( const ParametersType& parameters) const
        {

            ++POWELL_CALLS_TO_GET_VALUE;
        
            double phi0 = parameters[0];
        
            double intensity = FLT_MIN; 
            float cmplxPt[2];
            double tmp; 

            // apply zero order phase to data:  
            this->copySpectrum->GetTupleValue(0, cmplxPt);
            svk::svkPhaseSpec::ZeroOrderPhase(phi0, cmplxPt);

            //  maximize positive peak height (minimize negative peak ht) 
            tmp = -1*cmplxPt[0];
            //if ( tmp >= intensity) {
                intensity= tmp; 
                cout << "new phase: " << phi0 * 180. / vtkMath::Pi() << " " << intensity << endl;
            //}

            MeasureType measure = intensity; 
            return measure;
        }


        /*!
         *  Cost function based on finding the global maximum peak height
         */
        MeasureType  GetValueMaxGlobalPeakHt0( const ParametersType& parameters) const
        {

            ++POWELL_CALLS_TO_GET_VALUE;
        
            double phi0 = parameters[0];
        
            cout << "      GetValue_A( " ;
            cout << phi0 * 180. / vtkMath::Pi() << " ";
            cout << ") = ";
        
            double peakHeight = FLT_MIN; 
            float cmplxPt[2];
            int minPt = 0; 
            int maxPt = this->numFreqPoints; 
            double tmp; 
            for  ( int t = minPt; t < maxPt; t++ ){

                // apply zero order phase to data:  
                this->spectrum->GetTupleValue(t, cmplxPt);
                svk::svkPhaseSpec::ZeroOrderPhase(phi0, cmplxPt);

                //  maximize positive peak height (minimize negative peak ht) 
                tmp = cmplxPt[0];
                if ( tmp >= peakHeight ) {
                    peakHeight = tmp; 
                }

            }
            MeasureType measure = peakHeight; 
            return measure;
        }


        /*!
         *  Cost function based on maximizing autodetected  peak areas.
         *  areas are weighted by their height in the magnitude spectrum.   
         *  Area takes into account the line shape (absorptive vs dispersive)
         *  more than simply using peak height. 
         */
        MeasureType  GetValueMaxPeakHts0( const ParametersType& parameters) const
        {
            ++POWELL_CALLS_TO_GET_VALUE;
        
            double phi0 = parameters[0];

            svkPhaseSpec::ZeroOrderPhase( phi0, this->copySpectrum); 

            cout << "      GetValue_B( " ;
            cout << phi0 * 180. / vtkMath::Pi() << " ";
            cout << ") = ";

            double sumPeakDiffs = 0; 
            int numPeaks = this->peaks->GetNumPeaks();  
            for (int i = 0; i < numPeaks; i++) {

                float targetHeight = this->peaks->GetAvRMSPeakHeight(i); 
                float peakAreaValue = this->peaks->GetPeakArea(this->copySpectrum, i); 

                //  weight high SN peaks more: 
                //  peak area works better than peak height (maximizes
                //  absorption mode more accurately)
                sumPeakDiffs += (500*targetHeight - peakAreaValue )* targetHeight;
            }

            MeasureType measure = sumPeakDiffs; 
            return measure;
        }


        /*!
         *  Cost function based on finding the difference between peaks in spectrum 
         *  and average RMS spectrum.  Differences are weighted by the ht
         *  of the peak in the magnitude spectrum. 
         */
        MeasureType  GetValueDiffFromMagnitude0( const ParametersType& parameters) const 
        {
            ++POWELL_CALLS_TO_GET_VALUE;
        
            double phi0 = parameters[0];

            svkPhaseSpec::ZeroOrderPhase( phi0, this->copySpectrum); 

            cout << "      GetValue_C( " ;
            cout << phi0 * 180. / vtkMath::Pi() << " ";
            cout << ") = ";

            double sumPeakDiffs = 0; 
            int numPeaks = this->peaks->GetNumPeaks();  
            for (int i = 0; i < numPeaks; i++) {
            //for (int i = 2; i < numPeaks; i++) {

                float targetHeight = this->peaks->GetAvRMSPeakHeight(i); 
                float peakAreaValue = this->peaks->GetPeakArea(this->copySpectrum, i); 
                sumPeakDiffs += (500*targetHeight - peakAreaValue) * targetHeight;

            }

            MeasureType measure = sumPeakDiffs; 
            return measure;
        }


        /*!
         *  Version that only gets phase from one peak (zeroOrderPhasePeak). 
         *  Cost function based on finding the difference between peaks in spectrum 
         *  and average RMS spectrum.  Differences are weighted by the ht
         *  of the peak in the magnitude spectrum. 
         */
        MeasureType  GetValueDiffFromMagnitude0OnePeak( const ParametersType& parameters) const 
        {
            ++POWELL_CALLS_TO_GET_VALUE;
            cout << "   GetValue_D" << endl;
        
            double phi0 = parameters[0];

            svkPhaseSpec::ZeroOrderPhase( phi0, this->copySpectrum); 

            double sumPeakDiffs = 0; 
            int zeroOrderPeak = this->zeroOrderPhasePeak; 
            float targetHeight = this->peaks->GetAvRMSPeakHeight( zeroOrderPeak ); 
            float peakAreaValue = this->peaks->GetPeakArea(this->copySpectrum,  zeroOrderPeak ); 
            sumPeakDiffs += (500*targetHeight - peakAreaValue) * targetHeight;

            MeasureType measure = sumPeakDiffs; 
            return measure;
        }


        /*!
         *  Version that only gets phase from one peak (zeroOrderPhasePeak). 
         *  Cost function based on finding the difference between peaks in spectrum 
         *  and average RMS spectrum.  Differences are weighted by the ht
         *  of the peak in the magnitude spectrum. 
         */
        MeasureType  GetValueMaxPeakHt0OnePeak( const ParametersType& parameters) const 
        {
            ++POWELL_CALLS_TO_GET_VALUE;
        
            double phi0 = parameters[0];
            svkPhaseSpec::ZeroOrderPhase( phi0, this->copySpectrum); 

            cout << "   GetValue_E: " <<  phi0 ;

            double sumPeak = 0; 
            int zeroOrderPeak = this->zeroOrderPhasePeak; 
            float targetHeight = this->peaks->GetAvRMSPeakHeight( zeroOrderPeak ); 
            float peakAreaValue = this->peaks->GetPeakArea(this->copySpectrum,  zeroOrderPeak ); 
            float peakAreaSym = this->peaks->GetPeakSymmetry(this->copySpectrum,  zeroOrderPeak ); 

            // maximize peak integrated area, the higher the area, the more negative the contribution 
            sumPeak =  -1 * peakAreaValue;  

            // maximize symmetry : the more symmetric the smaller the contribution 
            sumPeak += peakAreaSym; 

            MeasureType measure = sumPeak; 
            return measure;
        }


        /*!
         *  Cost function based on maximizing autodetected peak areas.
         *  areas are weighted by their height in the magnitude spectrum.   
         *  Area takes into account the line shape (absorptive vs dispersive)
         *  more than simply using peak height. 
         */
        MeasureType  GetValueMaxPeakHts1( const ParametersType& parameters) const
        {
            ++POWELL_CALLS_TO_GET_VALUE;
        
            double phi1 = parameters[0];
            cout << "   GetValue_F" << phi1 ;

            int startPt; 
            int peakPt; 
            int endPt; 
            this->peaks->GetPeakDefinition( this->zeroOrderPhasePeak, &startPt, &peakPt, &endPt ); 

            svkPhaseSpec::FirstOrderPhase( phi1, peakPt, this->copySpectrum); 

            double sumPeakDiffs = 0; 
            int numPeaks = this->peaks->GetNumPeaks();  
            for (int i = 0; i < numPeaks; i++) {
                // This peak should already be zero order phased.
                if ( i != this->zeroOrderPhasePeak ) {

                    float targetHeight = this->peaks->GetAvRMSPeakHeight(i); 
                    float peakAreaValue = this->peaks->GetPeakArea(this->copySpectrum, i); 
                    float peakAreaSym = this->peaks->GetPeakSymmetry(this->copySpectrum, i); 

                    //  weight high SN peaks more: 
                    //  peak area works better than peak height (maximizes
                    //  absorption mode more accurately)
                    sumPeakDiffs += -1 * peakAreaValue * targetHeight;
                    sumPeakDiffs += peakAreaSym * targetHeight;
                }
            }

            MeasureType measure = sumPeakDiffs; 
            return measure;
        }

        /*!
         *  Cost function based on maximizing integrated area (zero and first order).  
         *  areas are weighted by their height in the magnitude spectrum.   
         *  Area takes into account the line shape (absorptive vs dispersive)
         *  more than simply using peak height. 
         */
        MeasureType  GetValueMaxInt01( const ParametersType& parameters) const
        {
            ++POWELL_CALLS_TO_GET_VALUE;
        
            double phi0 = parameters[0];
            double phi1 = parameters[1] + this->numFirstOrderPhaseValues/2;
            //cout << " phi0, phi1 => " << phi0 << " " << phi1 <<  endl;

#ifndef SWARM
            if ( phi0 > vtkMath::Pi() || phi0 < -1*vtkMath::Pi() || phi1 > this->numFreqPoints || phi1 < 0 ) {
                cout << "OUT OF BOUNDS" << endl;
                return DBL_MIN; 
            }
#endif

            cout << "      GetValue_G( "  << static_cast<int>(phi0 * 180. / vtkMath::Pi()) << ", " << phi1 << ") = ";

            svkPhaseSpec::FirstOrderPhase( phi0, this->linearPhaseArrays[static_cast<int>(phi1)], this->copySpectrum); 

            double sumPeaks = 0; 
            int numPeaks = this->peaks->GetNumPeaks();  
//for (int i = 0; i < numPeaks; i++) {
            for (int i = 2; i < numPeaks; i++) {

                float targetHeight = this->peaks->GetAvRMSPeakHeight(i); 
                float peakAreaValue = this->peaks->GetPeakArea(this->copySpectrum, i); 
                float peakAreaSym = this->peaks->GetPeakSymmetry(this->copySpectrum, i); 

                //  weight high SN peaks more: 
                //  peak area works better than peak height (maximizes
                //  absorption mode more accurately)
                //sumPeaks += (50*targetHeight - peakAreaValue )* targetHeight;
                //sumPeaks += (-1 * peakAreaValue ) / targetHeight;

                //  maximize peak height and symmetry, divide by target height to normalize weights
                //  across spectrum.   
                sumPeaks += (-1 * peakAreaValue ) / (targetHeight * targetHeight);
                sumPeaks += ( peakAreaSym ) / (targetHeight * targetHeight);
            }

            MeasureType measure = sumPeaks; 
            return measure;
        }


        /*!
         *  Cost function based on maximizing autodetected peak areas.
         *  areas are weighted by their height in the magnitude spectrum.   
         *  Area takes into account the line shape (absorptive vs dispersive)
         *  more than simply using peak height. 
         */
        MeasureType  GetValueDiffFromMagnitude1( const ParametersType& parameters) const
        {
            ++POWELL_CALLS_TO_GET_VALUE;
            cout << "   GetValue_H" << endl;
        
            double phi1 = parameters[0];

            int startPt; 
            int peakPt; 
            int endPt; 
            this->peaks->GetPeakDefinition( this->zeroOrderPhasePeak, &startPt, &peakPt, &endPt ); 

            svkPhaseSpec::FirstOrderPhase( phi1, peakPt, this->copySpectrum); 

            double sumPeakDiffs = 0; 
            int numPeaks = this->peaks->GetNumPeaks();  
            for (int i = 0; i < numPeaks; i++) {
                // This peak should already be zero order phased.
                if ( i != this->zeroOrderPhasePeak ) {

                    float targetHeight = this->peaks->GetAvRMSPeakHeight(i); 
                    float peakAreaValue = this->peaks->GetPeakArea(this->copySpectrum, i); 

                    //  weight high SN peaks more: 
                    //  peak area works better than peak height (maximizes
                    //  absorption mode more accurately)
                    sumPeakDiffs += (500*targetHeight - peakAreaValue) * targetHeight;
                }
            }

            MeasureType measure = sumPeakDiffs; 
            return measure;
        }


        /*  
         *  returns the cost function for the current param values: 
         *  typedef double MeasureType
         */
        MeasureType  GetValue( const ParametersType & parameters ) const
        { 

            double cost;  

            // make a member variable (copy)
            //cout << "copy spectrum" << endl;
            this->copySpectrum->DeepCopy(this->spectrum); 

            if ( this->phasingModel == svkMRSAutoPhase::FIRST_POINT_0 ) {
                cost = GetFirstPointPhaseValue0( parameters ); 
            } else if ( this->phasingModel == svkMRSAutoPhase::MAX_GLOBAL_PEAK_HT_0 ) {
                cost = GetValueMaxGlobalPeakHt0( parameters ); 
            } else if ( this->phasingModel == svkMRSAutoPhase::MAX_PEAK_HTS_0 ) {
                cost = GetValueMaxPeakHts0( parameters ); 
            } else if ( this->phasingModel == svkMRSAutoPhase::MIN_DIFF_FROM_MAG_0 ) {
                cost = GetValueDiffFromMagnitude0( parameters ); 
            } else if ( this->phasingModel == svkMRSAutoPhase::MAX_PEAK_HTS_1) {
                cost = GetValueMaxPeakHts1( parameters ); 
            } else if ( this->phasingModel == svkMRSAutoPhase::MAX_PEAK_HT_0_ONE_PEAK ) {
                cost = GetValueMaxPeakHt0OnePeak( parameters ); 
            } else if ( this->phasingModel == svkMRSAutoPhase::MIN_DIFF_FROM_MAG_0_ONE_PEAK ) {
                cost = GetValueDiffFromMagnitude0OnePeak( parameters ); 
            } else if ( this->phasingModel == svkMRSAutoPhase::MIN_DIFF_FROM_MAG_1 ) {
                cost = GetValueDiffFromMagnitude1( parameters ); 
            } else if ( this->phasingModel == svkMRSAutoPhase::MAX_PEAK_HTS_01 ) {
                cost = GetValueMaxInt01( parameters ); 
            }

            MeasureType measure = cost; 
            cout << "                          cost: " << measure << endl; 
            return measure;
        }


        /*
         *
         */  
        unsigned int GetNumberOfParameters(void) const
        {
            int numParameters = 1; 

            if ( this->phasingModel == svkMRSAutoPhase::MAX_PEAK_HTS_01 ) {
                numParameters = 2; 
            }

            return numParameters;
        }


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


        /*
         *
         */  
        void SetPeakPicker( svkMRSPeakPick* peaks )  
        {
            this->peaks = peaks;
        }

        /*
         *
         */  
        void SetLinearPhaseArrays( vtkImageComplex** linearPhaseArrays)  
        {
            this->linearPhaseArrays = linearPhaseArrays;
        }


        /*
         *
         */  
        void SetNumFirstOrderPhaseValues( int numPhaseValues )  
        {
            this->numFirstOrderPhaseValues = numPhaseValues;
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

            vtkFloatArray*                  spectrum;
            vtkFloatArray*                  copySpectrum;
            int                             numFreqPoints; 
            svkMRSAutoPhase::phasingModel   phasingModel; 
            svkMRSPeakPick*                 peaks;
            int                             zeroOrderPhasePeak;  
            vtkImageComplex**               linearPhaseArrays;
            int                             numFirstOrderPhaseValues; 


};



#endif// SVK_PHASE_COST_FUNCTION_H
