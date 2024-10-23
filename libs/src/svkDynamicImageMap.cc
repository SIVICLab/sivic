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

#include <svkDynamicImageMap.h>


using namespace svk;


//vtkCxxRevisionMacro(svkDynamicImageMap, "$Rev$");


/*!
 *
 */
svkDynamicImageMap::svkDynamicImageMap()
{
#if VTK_DEBUG_ON
    this->DebugOn();
#endif

    vtkDebugMacro(<< this->GetClassName() << "::" << this->GetClassName() << "()");

    this->newSeriesDescription = ""; 
    this->normalize = false; 
    this->SetNumberOfInputPorts(1); 
    this->SetNumberOfOutputPorts(1); 
}



/*!
 *
 */
svkDynamicImageMap::~svkDynamicImageMap()
{
    vtkDebugMacro(<<this->GetClassName()<<"::~"<<this->GetClassName());
}


/*!
 *  Set the series description for the DICOM header of the copy.  
 */
void svkDynamicImageMap::SetSeriesDescription( std::string newSeriesDescription )
{
    this->newSeriesDescription = newSeriesDescription;
    this->Modified(); 
}


/*!
 *  Resets the origin and extent for correct initialization of output svkMriImageData object from input 
 *  svkMrsImageData object. 
 */
int svkDynamicImageMap::RequestInformation( vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector )
{

    vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
    vtkInformation *outInfo = outputVector->GetInformationObject(0);

    int inWholeExt[6];
    inInfo->Get(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(), inWholeExt);
    double inSpacing[3]; 
    this->GetImageDataInput(0)->GetSpacing( inSpacing );
    

    //  MRI image data output map has the same extent as the input MRI 
    //  image data (points):
    int outUpExt[6];
    int outWholeExt[6];
    double outSpacing[3]; 
    for (int i = 0; i < 3; i++) {
        outUpExt[2*i]      = inWholeExt[2*i];
        outUpExt[2*i+1]    = inWholeExt[2*i+1];
        outWholeExt[2*i]   = inWholeExt[2*i];
        outWholeExt[2*i+1] = inWholeExt[2*i+1];

        outSpacing[i] = inSpacing[i];
    }

    //  MRS Input data has origin at first point (voxel corner).  Whereas output MRI image has origin at
    //  center of a point (point data).  In both cases this is the DICOM origin, but needs to be represented
    //  differently in VTK and DCM: 
    double outOrigin[3];
    this->GetImageDataInput(0)->GetDcmHeader()->GetOrigin( outOrigin ); 

    outInfo->Set(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(), outWholeExt, 6);
    outInfo->Set(vtkStreamingDemandDrivenPipeline::UPDATE_EXTENT(), outUpExt, 6);
    outInfo->Set(vtkDataObject::SPACING(), outSpacing, 3);
    outInfo->Set(vtkDataObject::ORIGIN(), outOrigin, 3);

    return 1;
}


/*!
 *  Copy the Dcm Header and Provenance from the input to the output. 
 */
int svkDynamicImageMap::RequestData( vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector )
{

    //  Create the template data object by  
    //  extractng an svkMriImageData from the input svkMrsImageData object
    //  Use an arbitrary point for initialization of scalars.  Actual data 
    //  will be overwritten by algorithm. 
    int numOutputMaps = this->GetNumberOfOutputPorts(); 

    for (int mapPort = 0; mapPort < numOutputMaps; mapPort++ ) { 
        svkMriImageData::SafeDownCast( this->GetImageDataInput(0) )->GetCellDataRepresentation()->GetZeroImage(
            svkMriImageData::SafeDownCast( this->GetOutput(mapPort) ) );  
    }

    this->GenerateMaps(); 

    for (int mapPort = 0; mapPort < numOutputMaps; mapPort++ ) { 
        svkDcmHeader* hdr = this->GetOutput( mapPort )->GetDcmHeader();
        hdr->InsertUniqueUID("SeriesInstanceUID");
        hdr->InsertUniqueUID("SOPInstanceUID");
        hdr->SetValue("SeriesDescription", this->newSeriesDescription);
    }

    return 1; 
};



/*!  
 *  Gets quick noise estimate from points 1-15 (rms)
 */
double svkDynamicImageMap::GetNoise( float* imgPtr )
{

    double noise = 0.;
    for ( int pt = 1; pt <= 15 ; pt ++ ) {
        noise += pow( ((double)imgPtr[ pt ]), (double)2. );
    }
    noise = pow( (noise/15.), 0.5);    
    return noise; 

}


/*! 
 *  Zero data
 */
void svkDynamicImageMap::ZeroData()
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
void svkDynamicImageMap::SetNormalize()
{
    this->normalize = true; 
};


/*!
 *  Normalize peak height values by NAWM peak ht determined from 
 *  mode of peak height map. 
 */
double svkDynamicImageMap::GetNormalizationFactor()
{

    //  Get min/max peak ht values for setting up histogram 
    vtkImageAccumulate* acc = vtkImageAccumulate::New();
    svkMriImageData* tmp = svkMriImageData::New();
    tmp->DeepCopy( this->GetOutput() );

    acc->SetInputData( tmp ); 
    acc->Update();
    double min = (acc->GetMin())[0]; 
    double max = (acc->GetMax())[0]; 
    double binSize = ( max - min ) / 100;
    acc->Delete();

    std::map< double, int> histo; 
    double bin = min;     
    //  initialize the histogram bins
    while ( bin < max ) {
        histo[bin] = 0; 
        bin += binSize; 
    }

    int numVoxels[3]; 
    this->GetOutput()->GetNumberOfVoxels(numVoxels);
    int totalVoxels = numVoxels[0] * numVoxels[1] * numVoxels[2]; 

    float* mapVals = vtkFloatArray::SafeDownCast(this->GetOutput()->GetPointData()->GetScalars())->GetPointer(0);

    for (int i = 0; i < totalVoxels; i++ ) {
        double val = mapVals[i]; 
        bin = min; 
        while ( bin < max ) {
            if ( (val - bin) <= binSize) {
                histo[bin] = histo[bin] + 1;  
                //  found bin, break out    
                bin = max; 
            }
            bin += binSize; 
        }
    }

    // determine mode of histogram 
    bin = min;     
    double modeValue = bin;
    int modeHt = 0;
    while ( bin < max ) {
        //cout << " histo( " << bin << ") = " << histo[bin] << endl;
        if ( bin != 0 && histo[bin] > modeHt ) { 
            modeValue = bin; 
            modeHt = histo[bin]; 
        }
        bin += binSize; 
    }
        
    //cout << "histo mode " << modeValue << " " << modeHt << endl;
    return modeValue; 

}


/*!
 *  Calculates ordinary linear regression )for specified point range
 *      Ref: "FORTRAN 77 an introduction to structured problem solving",  V.A.Dyck, J.D.Lawson, J.A.Smith
 *      page381, equation 14.20.
 */
void svkDynamicImageMap::GetRegression(int voxelID, int startPt, int endPt, double& slope, double& intercept) 
{

    vtkFloatArray* perfusionDynamics = vtkFloatArray::SafeDownCast( 
        svkMriImageData::SafeDownCast(this->GetImageDataInput(0))->GetCellDataRepresentation()->GetArray(voxelID) 
    ); 
    float* imgPtr = perfusionDynamics->GetPointer(0);


    //  Calculate Mean data values:
    double dr2Mean = 0; 
    double timeMean = 0; 
    for ( int i = startPt; i < endPt; i++ ) {
        dr2Mean += imgPtr[i]; 
        timeMean += i; 
    }
    dr2Mean  /= endPt - startPt; 
    timeMean /= endPt - startPt; 



    //  Calculate slope 
    double numerator = 0.; 
    double denominator = 0.; 
    for ( int i = startPt; i < endPt; i++) { 
        numerator   += ( i - timeMean) * ( imgPtr[i] - dr2Mean ); 
        denominator += ( i - timeMean ) * ( i - timeMean ); 
    }
    slope = numerator/denominator; 


    //  Calculate intercept 
    intercept = dr2Mean - slope * timeMean; 
}


/*!
 *
 */
void svkDynamicImageMap::UpdateProvenance()
{
    vtkDebugMacro(<<this->GetClassName()<<"::UpdateProvenance()");
}


/*!
 *
 */
int svkDynamicImageMap::FillInputPortInformation( int vtkNotUsed(port), vtkInformation* info )
{
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "svkMriImageData");
    return 1;
}


/*!
 *  Output from this algo is an svkMriImageData object. 
 */
int svkDynamicImageMap::FillOutputPortInformation( int vtkNotUsed(port), vtkInformation* info )
{
    info->Set( vtkDataObject::DATA_TYPE_NAME(), "svkMriImageData"); 
    return 1;
}

