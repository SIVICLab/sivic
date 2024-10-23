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


#include </usr/include/vtk/vtkInformation.h>
#include </usr/include/vtk/vtkInformationVector.h>
#include </usr/include/vtk/vtkStreamingDemandDrivenPipeline.h>

#include <svkMetaboliteRatioZScores.h>
#include <svkSpecPoint.h>


using namespace svk;


//vtkCxxRevisionMacro(svkMetaboliteRatioZScores, "$Rev$");
vtkStandardNewMacro(svkMetaboliteRatioZScores);


/*!
 *
 */
svkMetaboliteRatioZScores::svkMetaboliteRatioZScores()
{
#if VTK_DEBUG_ON
    this->DebugOn();
#endif

    vtkDebugMacro(<< this->GetClassName() << "::" << this->GetClassName() << "()");

    this->newSeriesDescription  = ""; 
    this->isVerbose             = false; 
    this->useSelectedVolumeFraction = 0;
    this->quantificationMask    = NULL;
    this->iterationMask         = NULL;
    this->zscoreUpperThreshold  = 2.25; 
    this->zscoreLowerThreshold  = 1.25; 
    this->yInterceptZero        = true; 

    //  Requires an numerator and denominator    
    this->SetNumberOfInputPorts(3);
}


/*!
 *
 */
svkMetaboliteRatioZScores::~svkMetaboliteRatioZScores()
{
    vtkDebugMacro(<<this->GetClassName()<<"::~"<<this->GetClassName());

    if ( this->quantificationMask != NULL )  {
        delete[] this->quantificationMask;
        this->quantificationMask = NULL;
    }

    if ( this->iterationMask != NULL )  {
        delete[] this->iterationMask;
        this->iterationMask = NULL;
    }

}


/*!
 *  Set the series description for the DICOM header of the output image.  
 */
void svkMetaboliteRatioZScores::SetSeriesDescription( std::string newSeriesDescription )
{
    this->newSeriesDescription = newSeriesDescription;
    this->Modified(); 
}


/*
 *  The quantification mask is the voxels to use in the calculation.
 *  By default this is all voxels where the numerator and denominator are not zero.
 *  Alternatively this can be limited to the selected volume.     
 */
void svkMetaboliteRatioZScores::InitQuantificationMask()
{
    int totalVoxels = this->GetTotalVoxels(); 

    //  Determines binary mask (quantificationMask) indicating whether a given voxel 
    //  should be quantified (1) or not (0). Usually this is based on whether a 
    //  specified fraction of the voxel inside the selected volume. 
    if ( this->quantificationMask == NULL ) {
        // Generate the mask from the svkMrsImageData input (port 2)
        this->quantificationMask = new short[totalVoxels];
        if ( this->useSelectedVolumeFraction ) {
            svkMrsImageData::SafeDownCast( this->GetImageDataInput(2) )->GetSelectionBoxMask(
                this->quantificationMask,
                this->useSelectedVolumeFraction
            );
        } else {
            //  all voxels are included in calculations, unless both num and denom are zero:
            //  GetNumerator and Denominator pixels:
            vtkDataArray* numeratorArray   = this->GetImageDataInput(0)->GetPointData()->GetScalars(); 
            vtkDataArray* denominatorArray = this->GetImageDataInput(1)->GetPointData()->GetScalars(); 
            for (int j = 0; j < totalVoxels; j++) {
                double   numeratorPixel = numeratorArray->GetTuple1(j); 
                double denominatorPixel = denominatorArray->GetTuple1(j); 
                if ( numeratorPixel != 0 && denominatorPixel != 0 ) {
                    this->quantificationMask[j] = 1;
                } else {
                    this->quantificationMask[j] = 0;
                }
            }    
        }
    }
    
}


/*!
 *  Generate the z-score image. This method sets up the masks and image templates
 */
int svkMetaboliteRatioZScores::RequestData( vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector )
{

    this->InitQuantificationMask(); 

    //  Initialize the iteration mask.  This is a mask showning which voxels to exclude
    //  from the regression analysis (e.g. in GetMean, GetDistanceSD, GetRegression) when determining z-scores.  
    //  The mask may vary from iteration to iteration as voxels get excluded as outliers. 
    //  At the end of the analysis, the remaining voxels in the iterationMask represent 
    //  the "control/normal" population of voxels. 
    if ( this->iterationMask != NULL ) {
        delete[] this->iterationMask; 
        this->iterationMask = NULL; 
    }
    int totalVoxels = this->GetTotalVoxels(); 
    this->iterationMask = new short[totalVoxels];
    memcpy( this->iterationMask, this->quantificationMask, totalVoxels * sizeof(short) ); 

    this->GetOutput()->DeepCopy( this->GetImageDataInput(0) );  
    svkDcmHeader* hdr = this->GetOutput()->GetDcmHeader();
    hdr->InsertUniqueUID("SeriesInstanceUID");
    hdr->InsertUniqueUID("SOPInstanceUID");
    hdr->SetValue("SeriesDescription", this->newSeriesDescription);
    this->ComputeZScore(); 

    return 1; 
}


/*!
 *  Limits regression analysis to voxels within the selected volume. 
 *  Fraction indicates if voxels partially (fraction %) within the 
 *  selected volume are included in regression. 
 *  The default is to include all voxels in the calculation (fraction = 0).
 */
void svkMetaboliteRatioZScores::LimitToSelectedVolume(float fraction)
{
    this->useSelectedVolumeFraction = fraction;
    this->Modified();
} 


/*!
 *  Limits regression analysis to voxels within the given mask. 
 *  Default is to use all voxels in calculation. 
 */
void svkMetaboliteRatioZScores::LimitToSelectedVolume( short* selectedVolumeMask)
{
    if ( this->quantificationMask != NULL ) {
        delete[] this->quantificationMask; 
        this->quantificationMask = NULL;
    }
    int totalVoxels = this->GetTotalVoxels(); 
    this->quantificationMask = new short[totalVoxels];
    memcpy( this->quantificationMask, selectedVolumeMask, totalVoxels * sizeof(short) ); 

    this->LimitToSelectedVolume(1);
}


/*! 
 *  Creates z-score image 
 */
void svkMetaboliteRatioZScores::ComputeZScore()
{

    this->ZeroData(); 

    double slope; 
    double intercept; 
    bool   converged = false;  

    while ( ! converged ) {

        this->GetRegression(slope, intercept); 

        //  The z-score is this image - mean distance / distance SD 
        this->GetZScores(slope, intercept); 
    
        //  Exclude outliers from regression
        int numOutliersInIteration = this->GetOutliers(); 
        cout << "OUTLIERS: " << numOutliersInIteration << endl; 
        if ( numOutliersInIteration == 0 ) {
            converged = true;  
        }
    }
    
}


/*!
 *
 */
int svkMetaboliteRatioZScores::GetTotalVoxels() 
{
    int numVoxels[3]; 
    this->GetImageDataInput(0)->GetNumberOfVoxels(numVoxels);
    int totalVoxels = numVoxels[0] * numVoxels[1] * numVoxels[2]; 
    return totalVoxels;     
}


/*!
 *  Determins which voxels are outliers in the 
 *  current regression interation, according to 
 *  the specified criteria.  These voxels are 
 *  removed from the iterationMask and the number
 *  of outlier voxels is returned as a measure
 *  of convergence.   
 */
int svkMetaboliteRatioZScores::GetOutliers() 
{

    int numOutliersInIteration = 0; 

    int totalVoxels = this->GetTotalVoxels(); 
    vtkDataArray* zscoreArray   = this->GetOutput()->GetPointData()->GetScalars(); 
    double*  zscores = new double[totalVoxels];  
    for (int i = 0; i < totalVoxels; i++ ) { 
        zscores[i] = zscoreArray->GetTuple1(i);      
    }


    for (int i = 0; i < totalVoxels; i++ ) {
       
        //  Only consder voxels currently used in regression: 
        if ( this->quantificationMask[i] && this->iterationMask[i] ) {

            //  if the zscore is an outlier, then remove voxel from 
            //  from iterationMask so it isn't used in the next
            //  regression iteration.     
            //if ( zscores[i] >= this->zscoreUpperThreshold ) {
            if ( zscores[i] >= this->zscoreUpperThreshold || zscores[i] <= -1 * this->zscoreLowerThreshold) {
                this->iterationMask[i] = 0; 
                numOutliersInIteration++; 
            }
        }
    } 

    delete [] zscores; 

    return numOutliersInIteration; 
}



/*!
 *  Computes the z-scores of each x,y (numerator, denominator) point from the current 
 *  regression line. Sets the values into the output image. z-score is the distance of a
 *  point from the regression line, normalized by the standard deviation of the distances. 
 */
void svkMetaboliteRatioZScores::GetZScores(double slope, double intercept) 
{

    //  Initialize the output scalars with distance 
    //  measurements
    //cout << "SLOPE INTERCEPT: " << slope << " " << intercept << endl;
    this->GetDistanceFromRegression( slope, intercept ); 

    int totalVoxels = this->GetTotalVoxels(); 
    vtkDataArray* distanceArray   = this->GetOutput()->GetPointData()->GetScalars(); 
    double*   distancePixels = new double[totalVoxels];  
    for (int i = 0; i < totalVoxels; i++ ) { 
        distancePixels[i] = distanceArray->GetTuple1(i);      
    }

    double distanceMean = this->GetMean( distancePixels, totalVoxels );
    double distanceSD = this->GetDistanceSD( distancePixels, totalVoxels );
    //cout << "DISTANCE MEAN , SD: " << distanceMean << " " << distanceSD << endl;

    double zscore; 
    for (int i = 0; i < totalVoxels; i++ ) {
        
        if ( this->quantificationMask[i] ) {
        //if ( this->quantificationMask[i] && this->iterationMask[i] ) {
            zscore = (distancePixels[i] ) / distanceSD;     
            //zscore = (distancePixels[i] - distanceMean) / distanceSD;     
        } else {
            zscore = 0.;      
        }
        //cout << "ZSCOREOUT: " << i << " distance: " << distancePixels[i] << " zs: " << zscore << endl; 
        this->GetOutput()->GetPointData()->GetScalars()->SetTuple1(i, zscore);
        
    }

    delete [] distancePixels; 
}


/*!
 *  Computes the distance of each x,y (numerator, denominator) point from the current 
 *  regression line. Sets the values into the output image. 
 */
void svkMetaboliteRatioZScores::GetDistanceFromRegression( double slope, double intercept ) 
{
  
    //  define the regresion vector passing through y-intercept:    
    //  (numerator, denominator) (x,y)
    double regressionVectorOrigin[2];   
    regressionVectorOrigin[0] = 0; 
    regressionVectorOrigin[1] = intercept; //y value at x = 0
    double regressionVectorEnd[2];   
    regressionVectorEnd[0] = 1; 
    regressionVectorEnd[1] = slope + intercept; // y value at x = 1
    double regressionVector[2];   
    regressionVector[0] = regressionVectorEnd[0] - regressionVectorOrigin[0];   //  x value
    regressionVector[1] = regressionVectorEnd[1] - regressionVectorOrigin[1];   //  y value
    vtkMath::Normalize2D(regressionVector);

    // define the data point vector originating at the regression y-intercept:    
    double dataVector[2];   

    //  GetNumerator and Denominator pixels:
    vtkDataArray* numeratorArray   = this->GetImageDataInput(0)->GetPointData()->GetScalars(); 
    vtkDataArray* denominatorArray = this->GetImageDataInput(1)->GetPointData()->GetScalars(); 

    int totalVoxels = this->GetTotalVoxels(); 
    double*   numeratorPixels = new double[totalVoxels];  
    double* denominatorPixels = new double[totalVoxels];  
    
    for (int i = 0; i < totalVoxels; i++ ) { 
        numeratorPixels[i]   = numeratorArray->GetTuple1(i);      
        denominatorPixels[i] = denominatorArray->GetTuple1(i);      
    }
    //cout << "for xl: " << endl;
    //for (int i = 0; i < totalVoxels; i++ ) {
        //if ( this->quantificationMask[i] && this->iterationMask[i] ) {
            //cout << numeratorPixels[i]  << ", " << denominatorPixels[i] << endl;
        //}
    //}
    //cout << "end for xl: " << endl;

    double projection; 
    double distance; 
    for (int i = 0; i < totalVoxels; i++ ) {

        //cout << "numerator:denominator " << numeratorPixels[i] << " " << denominatorPixels[i] << endl;
        dataVector[0] = denominatorPixels[i] - regressionVectorOrigin[0];   // difference between x values
        dataVector[1] = numeratorPixels[i]   - regressionVectorOrigin[1];   // difference between y values

        //  Now the perpendicular distnance defined by the dot prouduct of 
        //  these two vectors:
        if ( this->quantificationMask[i] ) {
        //if ( this->quantificationMask[i] && this->iterationMask[i] ) {

            if ( this->yInterceptZero == true ) {
                //cout << "    no intercept distance" << numeratorPixels << " " << regressionTmp[1] << endl;
                distance = ( numeratorPixels[i] - slope * denominatorPixels[i] ) / ( pow( (slope*slope + 1), 0.5) ); 
            } else {

                //  Project dataVector onto regression line, and extend
                //  normalized regression vector to the projection point:
                double regressionTmp[2];
                regressionTmp[0] = regressionVector[0]; 
                regressionTmp[1] = regressionVector[1]; 
                projection = vtkMath::Dot2D( dataVector, regressionTmp); 
                regressionTmp[0] = regressionTmp[0] * projection; 
                regressionTmp[1] = regressionTmp[1] * projection; 
    
                //  Get distance from dataVector to this point. 
                distance  =  ( dataVector[0] - regressionTmp[0] ) * ( dataVector[0] - regressionTmp[0] );
                distance +=  ( dataVector[1] - regressionTmp[1] ) * ( dataVector[1] - regressionTmp[1] );
                distance = pow(distance, 0.5); 
                //cout << "    dcr " << regressionTmp[0] << " " << regressionTmp[1] << endl;
                //cout << "    dcp " << dataVector[0] << " " << dataVector[1] << endl;
            }


        } else {
            distance = 0.;
        }
        //cout << "DISTANCE FROM REGRESSION: " << i << " = " << distance << endl;

        this->GetOutput()->GetPointData()->GetScalars()->SetTuple1(i, distance);
    }

    delete [] numeratorPixels;
    delete [] denominatorPixels;
}


/*!
 *  Calculates the linear regresion for the data points.  May exclude volumes outside the selected volume 
 *  (!LimitToSelectedVolume), and outlier voxels excluded by the iterationMask (SetZScoreThreshold). 
 *  The regression by default is constrained to go the the origin (yInterceptZero).  The regression for zero
 *  intercept is defined in ref 1, the general solution with arbitrary y-inertcept is given in ref 2:
 *
 *  Reference2:
 *  1.  Joseph G. Eisenhauer "Regression through the Origin",  
 *      Teaching Statistics Volume 25, Issue 3, pages 76–80, September 2003. 
 *  2. "FORTRAN 77 an introduction to structured problem solving",  V.A.Dyck, J.D.Lawson, J.A.Smith
 *      page381, equation 14.20.
 *  
 *  May compute the regression with out without the intercept term (i.e. y-intercept of 0)    
 */
void svkMetaboliteRatioZScores::GetRegression(double& slope, double& intercept) 
{

    //  GetNumerator and Denominator pixels as doubles, regardless of input type:
    vtkDataArray* numeratorArray   = this->GetImageDataInput(0)->GetPointData()->GetScalars(); 
    vtkDataArray* denominatorArray = this->GetImageDataInput(1)->GetPointData()->GetScalars(); 

    int totalVoxels = this->GetTotalVoxels(); 
    double*   numeratorPixels = new double[totalVoxels];  
    double* denominatorPixels = new double[totalVoxels];  
    
    for (int i = 0; i < totalVoxels; i++ ) { 
        numeratorPixels[i]   = numeratorArray->GetTuple1(i);      
        denominatorPixels[i] = denominatorArray->GetTuple1(i);      
    }


    if (this->yInterceptZero == true ) {
        slope  = this->GetRegressionSlopeZeroIntercept( 
                                numeratorPixels, 
                                denominatorPixels, 
                                totalVoxels 
                             ); 
        intercept = 0.0; 
    } else {
        double numeratorMean   = this->GetMean( numeratorPixels, totalVoxels );
        double denominatorMean = this->GetMean( denominatorPixels, totalVoxels );
        slope                  = this->GetRegressionSlope( 
                                numeratorPixels, 
                                denominatorPixels, 
                                numeratorMean, 
                                denominatorMean, 
                                totalVoxels 
                             ); 
        intercept              = this->GetRegressionIntercept(slope, numeratorMean, denominatorMean); 
        //cout << "MEAN:      " << numeratorMean << endl;
        //cout << "MEAN:      " << denominatorMean << endl;
    }

    delete [] numeratorPixels;
    delete [] denominatorPixels;

    //cout << "slope:     " << slope << endl;
    //cout << "intercept: " << intercept << endl;
}


/*!
 *  Gets the average value of the pixels (limited by the selected volume if
 *  specified). 
 */
double svkMetaboliteRatioZScores::GetMean( double* pixels, int numVoxels )
{
    double sum = 0.;         
    int numPixInCalc = 0; 

    for ( int i = 0; i < numVoxels; i++) { 
        if ( this->quantificationMask[i] && this->iterationMask[i] ) {
            sum += pixels[i]; 
            numPixInCalc++; 
        }
    }
    return sum/numPixInCalc;  
}


/*! 
 *  Gets the standard deviation of the pixel residuals (distance of points from regression line) 
 *  limited by the selected volume and interation mask if specified. 
 */
double svkMetaboliteRatioZScores::GetDistanceSD( double* pixels, int numVoxels)
{

    double sum = 0; 
    int numPixInCalc = 0; 
    for ( int i = 0; i < numVoxels; i++) { 
        if ( this->quantificationMask[i] && this->iterationMask[i] ) {
            sum += (pixels[i] * pixels[i] ); 
            //sum += (pixels[i] - mean) * (pixels[i] - mean); 
            numPixInCalc++; 
        }
    }
   
    double sd;
    if (numPixInCalc > 0 ) {
        sd = sum/(numPixInCalc); 
        sd = pow(sd, 0.5); 
    } else {
        sd = 0; 
    }

    return sd;  
}


/*!
 *  Gets the regresion slope with y intercept set to zero:
 *  Ref: 
 *      Joseph G. Eisenhauer "Regression through the Origin",  
 *      Teaching Statistics Volume 25, Issue 3, pages 76–80, September 2003. 
 *     
 */
double svkMetaboliteRatioZScores::GetRegressionSlopeZeroIntercept(double* numeratorPixels, double* denominatorPixels, int numVoxels) 
{
    double slope; 
    double numerator = 0.; 
    double denominator = 0.; 
    for ( int i = 0; i < numVoxels; i++) { 
        if ( this->quantificationMask[i] && this->iterationMask[i] ) {
            numerator   += denominatorPixels[i] *  numeratorPixels[i]; 
            denominator += denominatorPixels[i] *  denominatorPixels[i]; 
        }
    }

    slope = numerator/denominator; 
    return slope; 
    
}


/*! 
 *  Gets the regresion slope 
 *  reference:
 *  "FORTRAN 77 an introduction to structured problem solving",  V.A.Dyck, J.D.Lawson, J.A.Smith
 *  page381, equation 14.20.
 */
double svkMetaboliteRatioZScores::GetRegressionSlope( double* numeratorPixels, double* denominatorPixels, double numeratorMean, double denominatorMean, int numVoxels ) 
{
    double slope; 
    double numerator = 0.; 
    double denominator = 0.; 

    for ( int i = 0; i < numVoxels; i++) { 
        if ( this->quantificationMask[i] && this->iterationMask[i] ) {
            numerator += ( denominatorPixels[i] - denominatorMean) * ( numeratorPixels[i] - numeratorMean); 
            denominator +=  (denominatorPixels[i] - denominatorMean) * ( denominatorPixels[i] - denominatorMean); 
        }
    }

    slope = numerator/denominator; 
    return slope; 
}


/*!
 *  Gets the regresion intercept 
 *  reference:
 *  "FORTRAN 77 an introduction to structured problem solving",  V.A.Dyck, J.D.Lawson, J.A.Smith
 *  page381, equation 14.20.
 */
double svkMetaboliteRatioZScores::GetRegressionIntercept(double slope, double numeratorMean, double denominatorMean)
{
    double intercept = numeratorMean - slope * denominatorMean; 
    return intercept; 
}


/*! 
 *  Zero data
 */
void svkMetaboliteRatioZScores::ZeroData()
{

    int totalVoxels = this->GetTotalVoxels(); 
    double zeroValue = 0.;     
    for (int i = 0; i < totalVoxels; i++ ) {
        this->GetOutput()->GetPointData()->GetScalars()->SetTuple1(i, zeroValue);
    }

}


/*!
 *
 */
void svkMetaboliteRatioZScores::UpdateProvenance()
{
    vtkDebugMacro(<<this->GetClassName()<<"::UpdateProvenance()");
}


/*!
 *  Write the integrals for each voxel to stdout. Default is false.  
 */
void svkMetaboliteRatioZScores::SetVerbose( bool isVerbose )
{
    this->isVerbose = isVerbose; 
}


/*!
 *  Set the ZScore threshold for voxels to be included in the 
 *  regression analysis.  Voxels with Zscores ouside this z-score
 *  threshold (+/-) will be removed from regression interations 
 *  until all remaining voxels are within this range. 
 */
void svkMetaboliteRatioZScores::SetZScoreThresholds( double lowerThreshold, double upperThreshold )
{
    this->zscoreLowerThreshold = lowerThreshold; 
    this->zscoreUpperThreshold = upperThreshold; 
}


/*!
 *  input ports:
 *      0 & 1 are required (numerator and denominator met maps)
 *      2 is optional MRS data used for masking by selection box
 */
int svkMetaboliteRatioZScores::FillInputPortInformation( int port, vtkInformation* info )
{
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "svkMriImageData");
    if (port == 2) {
        info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "svkMrsImageData");
        info->Set(vtkAlgorithm::INPUT_IS_OPTIONAL(), 2);
    }

    return 1;
}


/*!
 *  Output from this algo is an svkMriImageData object. 
 */
int svkMetaboliteRatioZScores::FillOutputPortInformation( int vtkNotUsed(port), vtkInformation* info )
{
    info->Set( vtkDataObject::DATA_TYPE_NAME(), "svkMriImageData"); 
    return 1;
}

