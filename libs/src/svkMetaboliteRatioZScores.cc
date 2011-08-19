/*
 *  Copyright © 2009-2011 The Regents of the University of California.
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


#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkStreamingDemandDrivenPipeline.h>

#include <svkMetaboliteRatioZScores.h>
#include <svkSpecPoint.h>


using namespace svk;

#define UNDEFINED_PEAK_PPM VTK_FLOAT_MAX

vtkCxxRevisionMacro(svkMetaboliteRatioZScores, "$Rev$");
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
    this->zscoreThreshold       = 1.96; 

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
void svkMetaboliteRatioZScores::SetSeriesDescription( vtkstd::string newSeriesDescription )
{
    this->newSeriesDescription = newSeriesDescription;
    this->Modified(); 
}


/*!
 *  Generate the z-score image. 
 */
int svkMetaboliteRatioZScores::RequestData( vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector )
{
    int numVoxels[3];
    this->GetImageDataInput(2)->GetNumberOfVoxels(numVoxels);
    int totalVoxels = numVoxels[0] * numVoxels[1] * numVoxels[2];

    //  Determines binary mask indicating whether a given voxel should be quantified (1)
    //  or not (0). Usually this is based on whether a specified fraction of the voxel
    //  inside the selected volume.
    if ( this->quantificationMask == NULL ) {

        // Generate the mask from the svkMrsImageData input (port 2)
        this->quantificationMask = new short[totalVoxels];
        this->iterationMask      = new short[totalVoxels];

        svkMrsImageData::SafeDownCast( this->GetImageDataInput(2) )->GetSelectionBoxMask(
            this->quantificationMask,
            this->useSelectedVolumeFraction
        );
    }

    //  Initialize the iteration mask.  This is a mask showning which voxels to exclude
    //  from the regression analysis (e.g. GetMean, GetSD, GetRegression) when determining z-scores.  
    //  The mask may vary from iteration to iteration as voxels get excluded as outliers. 
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
        
        if ( numOutliersInIteration == 0 ) {
            converged = true;  
        }

    }
    
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

    int numVoxels[3]; 
    this->GetOutput()->GetNumberOfVoxels(numVoxels);
    int totalVoxels = numVoxels[0] * numVoxels[1] * numVoxels[2]; 

    double* zscores = vtkDoubleArray::SafeDownCast(this->GetOutput(0)->GetPointData()->GetScalars())->GetPointer(0);

    for (int i = 0; i < totalVoxels; i++ ) {
       
        //  Only consder voxels currently used in regression: 
        if ( this->quantificationMask[i] && this->iterationMask[i] ) {

            //  if the zscore is an outlier, then remove voxel from 
            //  from iterationMask so it isn't used in the next
            //  regression iteration.     
            if ( zscores[i] >= this->zscoreThreshold ) {
                this->iterationMask[i] = 0; 
                numOutliersInIteration++; 
            }
        }
    }
    return numOutliersInIteration; 
}



/*!
 *  Computes the z-scores of each x,y (numerator, denominator) point from the current 
 *  regression line. Sets the values into the output image. 
 */
void svkMetaboliteRatioZScores::GetZScores(double slope, double intercept) 
{
    this->GetDistanceFromRegression( slope, intercept ); 

    int numVoxels[3]; 
    this->GetOutput()->GetNumberOfVoxels(numVoxels);
    int totalVoxels = numVoxels[0] * numVoxels[1] * numVoxels[2]; 
    double* distancePixels = vtkDoubleArray::SafeDownCast(this->GetOutput(0)->GetPointData()->GetScalars())->GetPointer(0);
    double distanceMean = this->GetMean( distancePixels, totalVoxels );
    double distanceSD = this->GetSD( distancePixels, distanceMean, totalVoxels );

    double zscore; 
    for (int i = 0; i < totalVoxels; i++ ) {
        
        if ( this->quantificationMask[i] ) {
            zscore = (distancePixels[i] - distanceMean) / distanceSD;     
        } else {
            zscore = 0.;      
        }
        this->GetOutput()->GetPointData()->GetScalars()->SetTuple1(i, zscore);
        
    }
}


/*!
 *  Computes the distance of each x,y (numerator, denominator) point from the current 
 *  regression line. Sets the values into the output image. 
 */
void svkMetaboliteRatioZScores::GetDistanceFromRegression( double slope, double intercept ) 
{
  
    // define the regresion vector passing through y-intercept:    
    double regressionVectorOrigin[2];   
    regressionVectorOrigin[0] = 0; 
    regressionVectorOrigin[1] = intercept; //y value at x = 0
    double regressionVectorEnd[2];   
    regressionVectorEnd[0] = 1; 
    regressionVectorEnd[1] = slope + intercept; // y value at x = 1
    double regressionVector[2];   
    regressionVector[0] = regressionVectorEnd[0] - regressionVectorOrigin[0]; 
    regressionVector[1] = regressionVectorEnd[1] - regressionVectorOrigin[1]; 


    // define the data point vector originating at the regression y-intercept:    
    double dataVector[2];   

    //  GetNumerator and Denominator pixels:
    double* numeratorPixels = vtkDoubleArray::SafeDownCast(this->GetImageDataInput(0)->GetPointData()->GetScalars())->GetPointer(0);
    double* denominatorPixels = vtkDoubleArray::SafeDownCast(this->GetImageDataInput(1)->GetPointData()->GetScalars())->GetPointer(0);

    int numVoxels[3]; 
    this->GetOutput()->GetNumberOfVoxels(numVoxels);
    int totalVoxels = numVoxels[0] * numVoxels[1] * numVoxels[2]; 
    double distance; 
    for (int i = 0; i < totalVoxels; i++ ) {

        dataVector[0] = numeratorPixels[i]   - regressionVectorOrigin[0];
        dataVector[1] = denominatorPixels[i] - regressionVectorOrigin[1];

        //  Now the perpendicular distnance defined by the dot prouduct of 
        //  these two vectors:
        if ( this->quantificationMask[i] ) {
            distance = vtkMath::Dot2D( dataVector, regressionVector); 
        } else {
            distance = 0;
        }

        this->GetOutput()->GetPointData()->GetScalars()->SetTuple1(i, distance);
    }

}


/*!
 *  Calculates the linear regresion for the data points.  May exclude volumes outside the selected volume 
 *  ( see LimitToSelectedVolume). 
 *  reference:
 *  "FORTRAN 77 an introduction to structured problem solving",  V.A.Dyck, J.D.Lawson, J.A.Smith
 *  page381, equation 14.20.
 */
void svkMetaboliteRatioZScores::GetRegression(double& slope, double& intercept) 
{

    //  GetNumerator and Denominator pixels:
    double* numeratorPixels = vtkDoubleArray::SafeDownCast(this->GetImageDataInput(0)->GetPointData()->GetScalars())->GetPointer(0);
    double* denominatorPixels = vtkDoubleArray::SafeDownCast(this->GetImageDataInput(1)->GetPointData()->GetScalars())->GetPointer(0);

    int numVoxels[3]; 
    this->GetOutput()->GetNumberOfVoxels(numVoxels);
    int totalVoxels = numVoxels[0] * numVoxels[1] * numVoxels[2]; 
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
 *  Gets the standard deviation the pixels (limited by the selected volume if
 *  specified). 
 */
double svkMetaboliteRatioZScores::GetSD( double* pixels, double mean, int numVoxels)
{

    double sum = 0; 
    int numPixInCalc = 0; 
    for ( int i = 0; i < numVoxels; i++) { 
        if ( this->quantificationMask[i] && this->iterationMask[i] ) {
            sum += (pixels[i] - mean) * (pixels[i] - mean); 
            numPixInCalc++; 
        }
    }
   
    double sd = sum/(numPixInCalc - 1); 
    sd = pow(sd, 0.5); 

    return sd;  
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

    int numVoxels[3]; 
    this->GetOutput()->GetNumberOfVoxels(numVoxels);
    int totalVoxels = numVoxels[0] * numVoxels[1] * numVoxels[2]; 
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
 *  treshold (+/-) will be removed from regression interations 
 *  until all remaining voxels are within this range. 
 */
void svkMetaboliteRatioZScores::SetZScoreThreshold( double threshold )
{
    this->zscoreThreshold = threshold; 
}


/*!
 *  input ports 0 - 2 are required. Port 3 is for MRS data to 
 *  specify the selection box to limit the compuation spatially. 
 */
int svkMetaboliteRatioZScores::FillInputPortInformation( int port, vtkInformation* info )
{
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "svkMriImageData");
    if (port == 2) {
        info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "svkMrsImageData");
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

