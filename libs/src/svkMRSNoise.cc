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
 *      Beck Olson
 */



#include <svkMRSNoise.h>


using namespace svk;


//vtkCxxRevisionMacro(svkMRSNoise, "$Rev$");
vtkStandardNewMacro(svkMRSNoise);


svkMRSNoise::svkMRSNoise()
{

#if VTK_DEBUG_ON
    this->DebugOn();
#endif

    vtkDebugMacro(<<this->GetClassName() << "::" << this->GetClassName() << "()");

    this->noiseSD = 0.0;
    this->onlyUseSelectionBox = false;
    this->noiseWindowPercent = .05;
    this->noiseWindowStartPt = -1;
    this->noiseWindowEndPt = -1;

}


svkMRSNoise::~svkMRSNoise()
{
}


/*!
 *
 */
void svkMRSNoise::SetNoiseStartPoint( int startPt )
{
    this->noiseWindowStartPt = startPt;
}


/*!
 *
 */
void svkMRSNoise::SetNoiseEndPoint( int endPt )
{
    this->noiseWindowEndPt = endPt;
}



/*! 
 *
 */
int svkMRSNoise::RequestInformation( vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector )
{
    return 1;
}


/*! 
 *
 */
int svkMRSNoise::RequestData( vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector )
{

    svkMrsImageData* data = svkMrsImageData::SafeDownCast(this->GetImageDataInput(0)); 

    int numTimePoints = data->GetDcmHeader()->GetIntValue( "DataPointColumns" );
    //this->spectralWidth = data->GetDcmHeader()->GetFloatValue( "SpectralWidth" );

    svkDcmHeader::DimensionVector dimensionVector = data->GetDcmHeader()->GetDimensionIndexVector();
    int numCells = svkDcmHeader::GetNumberOfCells( &dimensionVector );


    //  Get Selection box mask: 
    float tolerance = .5;     
    this->selectionBoxMask = new short[numCells];
    data->GetSelectionBoxMask(selectionBoxMask, tolerance); 

    //  steps: 
    //  get average spectrum (in selection box)?
    //  break spectrum up into small sections and find the section with the smallest SD
    //  calculate the average SD in that region from each voxel 
    if ( this->noiseWindowStartPt < 0 || this->noiseWindowEndPt < 0 ) {
        this->InitAverageSpectrum(); 
        this->FindNoiseWindow();
    }
    this->CalculateNoiseSD(); 

    delete [] this->selectionBoxMask; 

    
    //  Trigger observer update via modified event:
    this->GetInput()->Modified();
    this->Update();

    return 1; 
}


/*!
 *  Returns the average value of noise standard deviation.  The noise SD is calculated for each voxel
 *  and the average of the individual SDs is returned. 
 */
void svkMRSNoise::CalculateNoiseSD()
{

    svkMrsImageData* data = svkMrsImageData::SafeDownCast(this->GetImageDataInput(0)); 
    svkDcmHeader::DimensionVector dimensionVector = data->GetDcmHeader()->GetDimensionIndexVector();
    svkDcmHeader::DimensionVector loopVector = dimensionVector;

    int numTimePoints = data->GetDcmHeader()->GetIntValue( "DataPointColumns" );
    int numCells = svkDcmHeader::GetNumberOfCells( &dimensionVector );
    
    int numVoxelsAveraged = 0; 

    //  First calculate the value for a voxel, then the SD for that same voxel
    float mean = 0; 
    float noiseSDTmp = 0; 
    double meanTmp = 0; 

    for ( int cellID = 0; cellID < numCells; cellID++ ) {

        if (this->onlyUseSelectionBox == true ) {
            svkDcmHeader::GetDimensionVectorIndexFromCellID( &dimensionVector, &loopVector, cellID );
            int spatialCellIndex = svkDcmHeader::GetSpatialCellIDFromDimensionVectorIndex( &dimensionVector, &loopVector);
            if ( this->selectionBoxMask[cellID] == 0 ) {
                continue; 
            } 
        }

        // average in this spectrum
        vtkFloatArray* spectrum = static_cast<vtkFloatArray*>( data->GetSpectrum( cellID ) );
        mean = this->CalcWindowMean( spectrum, this->noiseWindowStartPt, this->noiseWindowEndPt ); 
        meanTmp += mean; 
        noiseSDTmp += this->CalcWindowSD( spectrum, mean, this->noiseWindowStartPt, this->noiseWindowEndPt ); 

        numVoxelsAveraged++; 
    }

    this->noiseSD = noiseSDTmp / numVoxelsAveraged; 

    //   init the global mean value (baseline value 
    this->noiseWindowMean = meanTmp / numVoxelsAveraged; 

    return; 
}


/*!
 *  Calculate SD of intensities within specified pt range of given spectrum, using the specified mean value for 
 *  the same range.  Inclusive of end points. 
 */
float svkMRSNoise::CalcWindowSD( vtkFloatArray* spectrum, float mean, int startPt, int endPt )
{

    double noise = 0;  
    double tuple[2];

    for (int i = startPt; i <= endPt; i++ ) {
        spectrum->GetTuple(i, tuple); 
        double value = (tuple[0] - mean) * (tuple[0] - mean ); 
        noise += value; 
    }

    noise = noise / (endPt - startPt + 1); 
    float noiseSD = pow(static_cast<double>(noise), static_cast<double>(0.5)); 

    return noiseSD; 
}


/*!
 *  Calculate mean value within specified pt range of given spectrum (inclusive of end 
 *  points. 
 */
float svkMRSNoise::CalcWindowMean( vtkFloatArray* spectrum, int startPt, int endPt )
{
    float mean = 0; 
    double tuple[2];
    for (int i = startPt; i <= endPt; i++ ) {   //inclusive
        spectrum->GetTuple(i, tuple); 
        mean += tuple[0]; 
        //cout << "MEAN: " << i << " " << tuple[0] << endl;
    }

    mean = mean / (endPt - startPt + 1);    // inclusive
    //cout << "MEAN av: " << mean << endl;

    return mean; 
}


/*!
 * 
 */
void svkMRSNoise::FindNoiseWindow()
{
    svkMrsImageData* data = svkMrsImageData::SafeDownCast(this->GetImageDataInput(0)); 
    int numTimePoints = data->GetDcmHeader()->GetIntValue( "DataPointColumns" );

    //  Now analyze this spectrum for the region of smallest noise that is at least 5 percent of data points. 
    int noiseWindow = static_cast<int>(numTimePoints * this->noiseWindowPercent); 

    double tuple[2];
    int startPoint = 0; 
    double noise = FLT_MAX; 
    double mean; 
    //cout << "window size: " << noiseWindow << endl;
    this->noiseWindowStartPt = 0; 
    this->noiseWindowEndPt = 0; 
    for ( int i = startPoint; i < numTimePoints - noiseWindow; i++ ) {

        double noiseTmp = 0; 
        double meanTmp = 0; 

        //cout << "calc noise in window:  " << i << " " << i + noiseWindow << endl;
        meanTmp = this->CalcWindowMean( this->averageSpectrum, i, i + noiseWindow ); 
        noiseTmp = this->CalcWindowSD( this->averageSpectrum, meanTmp, i, i + noiseWindow ); 

        if ( noiseTmp < noise ) {
            noise = noiseTmp; 
            mean  = meanTmp; 
            this->noiseWindowStartPt = i; 
            this->noiseWindowEndPt = i + noiseWindow; 
            //cout << "current: " <<  this->noiseWindowStartPt <<  " " << noise << endl;
        }
    }

    //  Preserver the stat values from the average magnitude spectrum: 
    this->magnitudeNoiseWindowMean = mean;
    this->magnitudeNoiseSD = noise;

    cout << "WINDOW: " << this->noiseWindowStartPt << " -> " << this->noiseWindowEndPt << endl;
    cout << "Av Mag Spec noisd sd: " << this->magnitudeNoiseSD << endl;
    cout << "Av Mag Spec baseline value: " << this->magnitudeNoiseWindowMean << endl;
}


/*!
 *  Get the start point for the noise window. 
 */
int svkMRSNoise::GetNoiseStartPoint()
{
    return this->noiseWindowStartPt; 
}


/*!
 *  Get the start point for the noise window. 
 */
int svkMRSNoise::GetNoiseEndPoint()
{
    return this->noiseWindowEndPt; 
}


/*!
 *  Returns the average magnitude spectrum in component 1. 
 *  Look at magnitude spectra at this point in case the
 *  input is multi-channel, to maximize the identification of noise
 *  vs signal peaks
 */
void svkMRSNoise::InitAverageSpectrum()    
{
    svkMrsImageData* data = svkMrsImageData::SafeDownCast(this->GetImageDataInput(0)); 
    svkDcmHeader::DimensionVector dimensionVector = data->GetDcmHeader()->GetDimensionIndexVector();
    svkDcmHeader::DimensionVector loopVector = dimensionVector;

    int numTimePoints = data->GetDcmHeader()->GetIntValue( "DataPointColumns" );
    int numCells = svkDcmHeader::GetNumberOfCells( &dimensionVector );

    //  Create 0 valued vtkFloatArray for average spectrum 
    vtkFloatArray* spectrum0 = static_cast<vtkFloatArray*>( data->GetSpectrum( 0 ) );
    this->averageSpectrum = vtkFloatArray::New(); 
    this->averageSpectrum->DeepCopy(spectrum0 ); 
    double tuple[2];
    for (int i = 0; i < numTimePoints; i++ ) {
        this->averageSpectrum->GetTuple(i, tuple); 
        tuple[0] = 0.; 
        tuple[1] = 0.; 
        this->averageSpectrum->SetTuple(i, tuple); 
    }
    
    int numVoxelsAveraged = 0; 
    float avTuple[2];
    for ( int cellID = 0; cellID < numCells; cellID++ ) {

        if (this->onlyUseSelectionBox == true ) {
               
            svkDcmHeader::GetDimensionVectorIndexFromCellID( &dimensionVector, &loopVector, cellID );
            int spatialCellIndex = svkDcmHeader::GetSpatialCellIDFromDimensionVectorIndex( &dimensionVector, &loopVector);

            if ( this->selectionBoxMask[spatialCellIndex] == 0 ) {
                continue; 
            } 
        }
        // average in this spectrum
        vtkFloatArray* spectrum = static_cast<vtkFloatArray*>( data->GetSpectrum( cellID ) );
        double tuple[2];
        for (int i = 0; i < numTimePoints; i++ ) {
            spectrum->GetTuple(i, tuple); 
            this->averageSpectrum->GetTuple(i, avTuple); 
            float rms =  tuple[0]*tuple[0] + tuple[1]*tuple[1];  
            rms =  pow( static_cast<double>(rms), static_cast<double>(0.5)); 
            avTuple[0] = avTuple[0] + rms; 
            //avTuple[1] = avTuple[1] + tuple[1];  
            this->averageSpectrum->SetTuple(i, avTuple); 
        }
        numVoxelsAveraged++; 
    }

    for (int i = 0; i < numTimePoints; i++ ) {
        this->averageSpectrum->GetTuple(i, avTuple); 
        avTuple[0] = avTuple[0]/numVoxelsAveraged; 
        this->averageSpectrum->SetTuple(i, avTuple); 
    }

    return; 
} 


/*! 
 *  Get the noise SD
 */
float svkMRSNoise::GetNoiseSD()
{
    return this->noiseSD;
}


/*! 
 *  Get the noise SD from average magnitude spectrum
 */
float svkMRSNoise::GetMagnitudeNoiseSD()
{
    return this->magnitudeNoiseSD;
}


/*! 
 *  Get the mean value of the baseline in the window used for 
 *  SD calc. 
 */
float svkMRSNoise::GetMeanBaseline()
{
    return this->noiseWindowMean;
}


/*! 
 *  Get the mean value of the baseline in the window used for 
 *  SD calc. 
 */
float svkMRSNoise::GetMagnitudeMeanBaseline()
{
    return this->magnitudeNoiseWindowMean;
}


/*!
 *  Returns the average (RMS) magnitude spectrum
 */
vtkFloatArray*  svkMRSNoise::GetAverageMagnitudeSpectrum()
{
    return this->averageSpectrum;
}


/*! 
 *  Sets the linear phase to apply to spectra.
 */
void svkMRSNoise::OnlyUseSelectionBox()
{
    this->onlyUseSelectionBox = true;
}

/*! 
 *  Sets percent of spectrum to use for noise calculation. 
 */
void svkMRSNoise::SetNoiseWindowPercent(float percent)
{
    if ( percent > 1 ) {
        percent = 1.0; 
    } 
    if ( percent < 0 ) {
        percent = 0; 
    } 
    this->noiseWindowPercent = percent;
}


/*!
 *
 */
int svkMRSNoise::FillInputPortInformation( int vtkNotUsed(port), vtkInformation* info )
{
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "svkMrsImageData"); 
    return 1;
}


/*!
 *  PrintSelf method calls parent class PrintSelf, then prints the dcos.
 *
 */
void svkMRSNoise::PrintSelf( ostream &os, vtkIndent indent )
{
    Superclass::PrintSelf( os, indent );
    os << "only use selection box:" << this->onlyUseSelectionBox<< endl;
    os << "noiseSD               :" << this->noiseSD<< endl;
}



