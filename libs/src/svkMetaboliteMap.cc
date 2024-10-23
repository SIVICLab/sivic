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
#include </usr/include/vtk/vtkXMLDataElement.h>
#include </usr/include/vtk/vtkXMLUtilities.h>

#include <svkMetaboliteMap.h>
#include <svkSpecPoint.h>


using namespace svk;


//vtkCxxRevisionMacro(svkMetaboliteMap, "$Rev$");
vtkStandardNewMacro(svkMetaboliteMap);


/*!
 *
 */
svkMetaboliteMap::svkMetaboliteMap()
{
#if VTK_DEBUG_ON
    this->DebugOn();
#endif

    vtkDebugMacro(<< this->GetClassName() << "::" << this->GetClassName() << "()");

    this->newSeriesDescription = ""; 
    this->quantificationAlgorithm = svkMetaboliteMap::INTEGRATE; 
    this->isVerbose = false; 
    this->useSelectedVolumeFraction = 0;
    this->quantificationMask = NULL;


}



/*!
 *
 */
svkMetaboliteMap::~svkMetaboliteMap()
{
    vtkDebugMacro(<<this->GetClassName()<<"::~"<<this->GetClassName());

    if ( this->quantificationMask != NULL )  {
        delete[] this->quantificationMask;
        this->quantificationMask = NULL;
    }

}


/*!
 *  Set the series description for the DICOM header of the copy.  
 */
void svkMetaboliteMap::SetSeriesDescription( std::string newSeriesDescription )
{
    this->newSeriesDescription = newSeriesDescription;
    this->Modified(); 
}


/*!
 *  Resets the origin and extent for correct initialization of output svkMriImageData object from input 
 *  svkMrsImageData object. 
 */
int svkMetaboliteMap::RequestInformation( vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector )
{

    vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
    vtkInformation *outInfo = outputVector->GetInformationObject(0);

    int inWholeExt[6];
    inInfo->Get(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(), inWholeExt);
    double inSpacing[3]; 
    this->GetImageDataInput(0)->GetSpacing( inSpacing );
    

    //  MRI image data has a smaller extent than the input MRS 
    //  image data (points vs cells):
    int outUpExt[6];
    int outWholeExt[6];
    double outSpacing[3]; 
    for (int i = 0; i < 3; i++) {
        outUpExt[2*i]      = inWholeExt[2*i];
        outUpExt[2*i+1]    = inWholeExt[2*i+1] - 1;
        outWholeExt[2*i]   = inWholeExt[2*i];
        outWholeExt[2*i+1] = inWholeExt[2*i+1] - 1;

        outSpacing[i] = inSpacing[i];
    }

    //  MRS Input data has origin at first point (voxel corner).  Whereas output MRI image has origin at
    //  center of a point (point data).  In both cases this is the DICOM origin, but needs to be represented
    //  differently in VTK and DCM: 
    double outOrigin[3];
    svkMrsImageData::SafeDownCast( this->GetImageDataInput(0) )->GetDcmHeader()->GetOrigin( outOrigin ); 

    outInfo->Set(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(), outWholeExt, 6);
    outInfo->Set(vtkStreamingDemandDrivenPipeline::UPDATE_EXTENT(), outUpExt, 6);
    outInfo->Set(vtkDataObject::SPACING(), outSpacing, 3);
    outInfo->Set(vtkDataObject::ORIGIN(), outOrigin, 3);

    return 1;
}


/*!
 *  Copy the Dcm Header and Provenance from the input to the output. 
 */
int svkMetaboliteMap::RequestData( vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector )
{

    //  Create the template data object by  
    //  extractng an svkMriImageData from the input svkMrsImageData object
    //  Use an arbitrary point for initialization of scalars.  Actual data 
    //  will be overwritten by algorithm. 
    svkMrsImageData::SafeDownCast( this->GetImageDataInput(0) )->GetImage(
        svkMriImageData::SafeDownCast( this->GetOutput() ), 
        0, 
        0, 
        0, 
        0, 
        this->newSeriesDescription, 
        VTK_DOUBLE
    );

    //  Determines binary mask (quantificationMask) indicating whether a given voxel 
    //  should be quantified (1) or not (0). Usually this is based on whether a specified 
    //  fraction of the voxel inside the selected volume. 
    if ( this->quantificationMask == NULL ) {
        int numVoxels[3];
        this->GetImageDataInput(0)->GetNumberOfVoxels(numVoxels);
        int totalVoxels = numVoxels[0] * numVoxels[1] * numVoxels[2];
        this->quantificationMask = new short[totalVoxels];

        svkMrsImageData::SafeDownCast( this->GetImageDataInput(0) )->GetSelectionBoxMask( 
            this->quantificationMask, 
            this->useSelectedVolumeFraction
        ); 
            
    }

    cout << "ALGO: " << this->quantificationAlgorithm  << endl;
    this->GenerateMap(); 

    svkDcmHeader* hdr = this->GetOutput()->GetDcmHeader();
    hdr->InsertUniqueUID("SeriesInstanceUID");
    hdr->InsertUniqueUID("SOPInstanceUID");
    hdr->SetValue("SeriesDescription", this->newSeriesDescription);

    return 1; 
};


/*! 
 *  Quantify spectra using specified algorith (e.g. Ingegrate, PkHt, Magnitude). 
 */
void svkMetaboliteMap::GenerateMap()
{

    this->ZeroData(); 

    //  Get integration limits:
    int startPt = 0; 
    int endPt = 0; 
    this->GetIntegrationPtRange(startPt, endPt); 

    //  If integration limits are out of range just returned zero'd image.
    int numSpecPoints = this->GetImageDataInput(0)->GetDcmHeader()->GetIntValue("DataPointColumns");
    if ( startPt < 0 ) { 
        startPt = 0;   
        vtkWarningWithObjectMacro(this, "Integration limits out of range, returning zero value map");
    }
    if ( endPt >= numSpecPoints ) {
        endPt = numSpecPoints - 1;     
        vtkWarningWithObjectMacro(this, "Integration limits out of range, returning zero value map");
    }  

    int numVoxels[3]; 
    this->GetOutput()->GetNumberOfVoxels(numVoxels);
    int totalVoxels = numVoxels[0] * numVoxels[1] * numVoxels[2]; 

    //  Loop over volumes (time points, coils):

    int numCoils      = svkMrsImageData::SafeDownCast( this->GetImageDataInput(0) )->GetDcmHeader()->GetNumberOfCoils();
    int numTimePoints = svkMrsImageData::SafeDownCast( this->GetImageDataInput(0) )->GetDcmHeader()->GetNumberOfTimePoints();

    int volumeIndex = 0; 
    for (int coilIndex = 0; coilIndex < numCoils; coilIndex++) {   
        for (int timeIndex = 0; timeIndex < numTimePoints; timeIndex++) {   

            //  Get the data array to initialize.  For vol0, this already exists, 
            //  for other volumes, need to create a new vtkDataArray to insert
            vtkDataArray* metMapArray;
            if ( volumeIndex == 0 ) {
                metMapArray = this->GetOutput()->GetPointData()->GetArray(0); 
            } else {
                //  allocate a new array, copy volume0 into it and add it to the 
                //  vtkImageData point data:

                //  allocate correct array type 
                int vtkDataType = this->GetOutput()->GetPointData()->GetArray(0)->GetDataType(); 
                if ( vtkDataType == VTK_VOID || vtkDataType == VTK_DOUBLE ) {
                    metMapArray = vtkDoubleArray::New();     
                } else if ( vtkDataType == VTK_FLOAT ) {
                    metMapArray = vtkFloatArray::New();     
                }
                metMapArray->DeepCopy( this->GetOutput()->GetPointData()->GetArray(0) ); 
                this->GetOutput()->GetPointData()->AddArray( metMapArray );
            }

            //  Add the output volume array to the correct array in the svkMriImageData object
            ostringstream number;
            number << volumeIndex;
            std::string arrayNameString("pixels");
            arrayNameString.append(number.str());

            metMapArray->SetName( arrayNameString.c_str() );

            double voxelValue;
            for (int i = 0; i < totalVoxels; i++ ) {

                if ( this->quantificationMask[i] ) {
                    vtkFloatArray* spectrum = vtkFloatArray::SafeDownCast( 
                        svkMrsImageData::SafeDownCast(this->GetImageDataInput(0))->GetSpectrumFromID(i, timeIndex, coilIndex ) 
                    ); 
                    float* specPtr = spectrum->GetPointer(0);

                    voxelValue = this->GetMapVoxelValue( specPtr, startPt, endPt); 

                } else {
                    voxelValue = 0.;
                }
                if ( this->isVerbose ) {
                    cout << "voxel(" << i << ") ppm center: " << this->peakCenterPPM << " value: " << voxelValue << endl; 
                }

                metMapArray->SetTuple1(i, voxelValue);
            }

            volumeIndex++; 

        }
    }

    //  rewrite the header per-frame functional groups:
    if (volumeIndex > 1 ) {    
        this->RedimensionData();
    }
}


/*!  
 *  For multi-volume data modifies header's per frame functional group sequence:
 */
double svkMetaboliteMap::GetMapVoxelValue( float* specPtr, int startPt, int endPt)
{

    double voxelValue; 

    if (this->quantificationAlgorithm == svkMetaboliteMap::INTEGRATE) { 
        voxelValue = this->GetIntegral( specPtr, startPt, endPt); 
    } else if (this->quantificationAlgorithm == svkMetaboliteMap::PEAK_HT) { 
        voxelValue = this->GetPeakHt( specPtr, startPt, endPt ); 
    } else if (this->quantificationAlgorithm == svkMetaboliteMap::MAG_PEAK_HT) { 
        voxelValue = this->GetMagPeakHt( specPtr, startPt, endPt ); 
    } else if (this->quantificationAlgorithm == svkMetaboliteMap::LINE_WIDTH) { 
        voxelValue = this->GetLineWidth( specPtr, startPt, endPt ); 
    } else if (this->quantificationAlgorithm == svkMetaboliteMap::MAG_LINE_WIDTH) { 
        voxelValue = this->GetMagLineWidth( specPtr, startPt, endPt ); 
    } else if (this->quantificationAlgorithm == svkMetaboliteMap::MAG_INTEGRATE) {
        voxelValue = this->GetMagIntegral( specPtr, startPt, endPt );
    }
    return voxelValue;
}


/*!  
 *  Gets integral of real component over the specified range from startPt to endPt.
 */
double svkMetaboliteMap::GetIntegral( float* specPtr, int startPt, int endPt)
{

    double integral = 0;

    for ( int pt = startPt; pt <= endPt; pt ++ ) {
        integral += specPtr[2*pt];
    }
    return integral; 
}

/*!
 *  Gets integral of real component over the specified range from startPt to endPt.
 */
double svkMetaboliteMap::GetMagIntegral( float* specPtr, int startPt, int endPt)
{

    double integral = 0;

    double mag= 0;

    for ( int pt = startPt; pt <= endPt; pt ++ ) {
		mag =  pow( specPtr[ 2 * pt], 2);
		mag += pow( specPtr[ 2 * pt + 1], 2);
		mag = pow(mag, 0.5);
        integral += mag;
    }
    return integral;
}


/*!  
 *  Gets max peak ht of real component over the specified range from startPt to endPt.
 */
double svkMetaboliteMap::GetPeakHt( float* specPtr, int startPt, int endPt)
{

    double peakHt = specPtr[ 2 * startPt ];
    for ( int pt = startPt; pt <= endPt; pt ++ ) {
        if ( specPtr[2*pt] > peakHt ) {
            peakHt = specPtr[2*pt];
        }
    }
    return peakHt; 
    
}


/*
 *  Gets the line width of the peak (in Hz)
 */
double svkMetaboliteMap::GetMagLineWidth( float* specPtr, int startPt, int endPt )
{
    //  Calculate Peak Max Ht and position.  then get position when intensity is 
    //  1/2 that.  

    int peakPosPt = startPt; 
    double magPeakHt = 0;
    double magPeakHtTmp = 0;
    magPeakHt = pow( specPtr[ 2 * startPt], 2);
    magPeakHt += pow( specPtr[ 2 * startPt + 1], 2);
    magPeakHt = pow(magPeakHt, 0.5);

    for ( int pt = startPt; pt <= endPt; pt++ ) {
        magPeakHtTmp = pow( specPtr[ 2 * pt], 2);
        magPeakHtTmp += pow( specPtr[ 2 * pt + 1], 2);
        magPeakHtTmp = pow(magPeakHtTmp, 0.5);
        if ( magPeakHtTmp > magPeakHt ) {
            magPeakHt = magPeakHtTmp;
            peakPosPt = pt; 
        }
    }


    double halfHeight = magPeakHt/2.;

    //  Get fist 1/2 height point
    int fwhmPt1 = startPt; 

    for ( int pt = startPt; pt <= peakPosPt; pt++ ) {
        magPeakHtTmp = pow( specPtr[ 2 * pt], 2);
        magPeakHtTmp += pow( specPtr[ 2 * pt + 1], 2);
        magPeakHtTmp = pow(magPeakHtTmp, 0.5);
        if ( magPeakHtTmp > halfHeight ) {
            fwhmPt1 = pt; 
            break; 
        }
    }

    int fwhmPt2 = endPt; 
    for ( int pt = peakPosPt; pt <= endPt; pt++ ) {
        magPeakHtTmp = pow( specPtr[ 2 * pt], 2);
        magPeakHtTmp += pow( specPtr[ 2 * pt + 1], 2);
        magPeakHtTmp = pow(magPeakHtTmp, 0.5);
        if ( magPeakHtTmp < halfHeight ) {
            fwhmPt2 = pt; 
            break; 
        }
    }
    
    // could interpolate to get the actual delta frequency for better accuracy, but this is a quick start  
    return this->GetWidthInHz( fwhmPt1, fwhmPt2 ); 
    
}


/*
 *  Gets the line width of the peak (in Hz)
 */
double svkMetaboliteMap::GetLineWidth( float* specPtr, int startPt, int endPt )
{
    //  Calculate Peak Max Ht and position.  then get position when intensity is 
    //  1/2 that.  

    int peakPosPt = startPt; 
    double peakHt = specPtr[ 2 * startPt ];
    for ( int pt = startPt; pt <= endPt; pt ++ ) {
        if ( specPtr[2*pt] > peakHt ) {
            peakHt = specPtr[2*pt];
            peakPosPt = pt; 
        }
    }

    double halfHeight = peakHt/2.;

    //  Get fist 1/2 height point
    int fwhmPt1 = startPt; 
    for ( int pt = startPt; pt <= peakPosPt; pt ++ ) {
        if ( specPtr[2*pt] > halfHeight ) {
            fwhmPt1 = pt; 
            break; 
        }
    }

    int fwhmPt2 = endPt; 
    for ( int pt = peakPosPt; pt <= endPt; pt ++ ) {
        if ( specPtr[2*pt] < halfHeight ) {
            fwhmPt2 = pt; 
            break; 
        }
    }
    
    // could interpolate to get the actual delta frequency for better accuracy, but this is a quick start  
    return this->GetWidthInHz( fwhmPt1, fwhmPt2 ); 
    
}


/*
 *  Convert with in points to hz. 
 */
float svkMetaboliteMap::GetWidthInHz( int startPt, int endPt)
{
    //  Get the integration range in points:
    svkSpecPoint* point = svkSpecPoint::New();
    point->SetDcmHeader( this->GetImageDataInput(0)->GetDcmHeader() );

    float startPtHz = point->ConvertPosUnits( startPt, svkSpecPoint::PTS, svkSpecPoint::Hz ); 
    float endPtHz   = point->ConvertPosUnits( endPt,   svkSpecPoint::PTS, svkSpecPoint::Hz ); 
    float deltaHz = fabs(endPtHz - startPtHz);
    point->Delete();
    return deltaHz; 
}


/*!  
 *  Gets max peak ht of magnitude spectrum over the specified range from startPt to endPt.
 */
double svkMetaboliteMap::GetMagPeakHt( float* specPtr, int startPt, int endPt)
{

    float magPeakHt  = pow( specPtr[2 * startPt], 2);
    magPeakHt += pow( specPtr[2 * startPt + 1], 2);
    magPeakHt = pow( magPeakHt, 0.5f);

    float magPeakHtTmp = magPeakHt;
    
    for ( int pt = startPt; pt <= endPt; pt++ ) {

        magPeakHtTmp  = pow( specPtr[ 2 * pt], 2);
        magPeakHtTmp += pow( specPtr[ 2 * pt + 1], 2);
        magPeakHtTmp  = pow(magPeakHtTmp, 0.5f);

        if ( magPeakHtTmp > magPeakHt ) {
            magPeakHt = magPeakHtTmp;
        }
    }
    return magPeakHt;
}


/*!  
 *  For multi-volume data modifies header's per frame functional group sequence:
 */
void svkMetaboliteMap::RedimensionData()
{
    svkDcmHeader* hdr = this->GetOutput()->GetDcmHeader();

    double origin[3];
    hdr->GetOrigin( origin, 0 );

    double voxelSpacing[3];
    hdr->GetPixelSpacing( voxelSpacing );

    double dcos[3][3];
    hdr->GetDataDcos( dcos );

    int numCoils      = svkMrsImageData::SafeDownCast( this->GetImageDataInput(0) )->GetDcmHeader()->GetNumberOfCoils();
    int numTimePoints = svkMrsImageData::SafeDownCast( this->GetImageDataInput(0) )->GetDcmHeader()->GetNumberOfTimePoints();

    svkDcmHeader::DimensionVector dimensionVector = this->GetImageDataInput(0)->GetDcmHeader()->GetDimensionIndexVector();
    svkDcmHeader::SetDimensionVectorValue(&dimensionVector, svkDcmHeader::SLICE_INDEX, hdr->GetNumberOfSlices()-1);
    if ( numTimePoints > 1 ) {
        svkDcmHeader::SetDimensionVectorValue(&dimensionVector, svkDcmHeader::TIME_INDEX, numTimePoints-1);
    } 

    if ( numCoils > 1 ) {
        svkDcmHeader::SetDimensionVectorValue(&dimensionVector, svkDcmHeader::CHANNEL_INDEX, numCoils-1);
    } 

    hdr->InitPerFrameFunctionalGroupSequence(
        origin,
        voxelSpacing,
        dcos,
        &dimensionVector
    );

}



/*! 
 *  Zero data
 */
void svkMetaboliteMap::ZeroData()
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
 *  Set the chemical shift of the peak position to integrate over.
 */
void svkMetaboliteMap::SetPeakPosPPM( float centerPPM )
{
    this->peakCenterPPM = centerPPM;
    this->Modified(); 
}


/*!
 *  Set the chemical shift range to integrate over.  Integration will be +/- 1/2 this
 *  width about the peak position.
 */
void svkMetaboliteMap::SetPeakWidthPPM( float widthPPM )
{
    this->peakWidthPPM = widthPPM;
    this->Modified(); 
}


/*!
 *  Converts an integration range from PPM to points
 */
void svkMetaboliteMap::GetIntegrationPtRange(int& startPt, int& endPt) 
{

    //  Get the integration range in points:
    svkSpecPoint* point = svkSpecPoint::New();
    point->SetDcmHeader( this->GetImageDataInput(0)->GetDcmHeader() );
    startPt =  static_cast< int > (
                        point->ConvertPosUnits(
                            this->peakCenterPPM + (this->peakWidthPPM/2), svkSpecPoint::PPM, svkSpecPoint::PTS )
                      );
    endPt   =  static_cast< int > (
                        point->ConvertPosUnits(
                            this->peakCenterPPM - (this->peakWidthPPM/2), svkSpecPoint::PPM, svkSpecPoint::PTS )
                      );  

    point->Delete();
}


/*!
 *
 */
void svkMetaboliteMap::SetAlgorithmToIntegrate()
{
    this->quantificationAlgorithm = svkMetaboliteMap::INTEGRATE; 
    this->Modified(); 
}


/*!
 *
 */
void svkMetaboliteMap::SetAlgorithmToPeakHeight()
{
    this->quantificationAlgorithm = svkMetaboliteMap::PEAK_HT; 
    this->Modified(); 
}


/*!
 *
 */
void svkMetaboliteMap::SetAlgorithmToMagPeakHeight()
{
    this->quantificationAlgorithm = svkMetaboliteMap::MAG_PEAK_HT; 
    this->Modified(); 
}


/*!
 *
 */
void svkMetaboliteMap::SetAlgorithmToLineWidth()
{
    this->quantificationAlgorithm = svkMetaboliteMap::LINE_WIDTH; 
    this->Modified(); 
}


/*!
 *
 */
void svkMetaboliteMap::SetAlgorithmToMagLineWidth()
{
    this->quantificationAlgorithm = svkMetaboliteMap::MAG_LINE_WIDTH; 
    this->Modified(); 
}

void  svkMetaboliteMap::SetAlgorithmToMagIntegrate()
{
    this->quantificationAlgorithm = svkMetaboliteMap::MAG_INTEGRATE;
    this->Modified();
}


/*
 *  Set algo type based on string description svkMetaboliteMap::algorithm. 
 */
void svkMetaboliteMap::SetAlgorithm( std::string algo )
{
    if ( algo.compare("INTEGRATE") == 0 ) {
        this->SetAlgorithmToIntegrate(); 
    } else if ( algo.compare("PEAK_HT") == 0 ) {
        this->SetAlgorithmToPeakHeight(); 
    } else if ( algo.compare("MAG_PEAK_HT") == 0 ) {
        this->SetAlgorithmToMagPeakHeight(); 
    } else if ( algo.compare("LINE_WIDTH") == 0 ) {
        this->SetAlgorithmToLineWidth(); 
    } else if ( algo.compare("MAG_LINE_WIDTH") == 0 ) {
        this->SetAlgorithmToMagLineWidth(); 
    } else if ( algo.compare("MAG_INTEGRATE") == 0 ) {
        this->SetAlgorithmToMagIntegrate();
    } else {
        vtkWarningWithObjectMacro(this, "SetAlgorithm(): Not a valid algorithm " + algo );
    }
}


/*!
 *
 */
void svkMetaboliteMap::UpdateProvenance()
{
    vtkDebugMacro(<<this->GetClassName()<<"::UpdateProvenance()");
}


/*
 *  Returns a pointer to the selected volume mask
 */
short* svkMetaboliteMap::GetSelectedVolumeMask()             
{
    return this->quantificationMask;     
}


/*!
 *  Write the integrals for each voxel to stdout. Default is false.  
 */
void svkMetaboliteMap::SetVerbose( bool isVerbose )
{
    this->isVerbose = isVerbose; 
}


/*
 *  Limits the calculation to voxels that have at least the specified fractional
 *  volume within the selected MRS volume. The default is to include all voxels
 *  in the calculation (fraction = 0).
 */
void svkMetaboliteMap::LimitToSelectedVolume(float fraction)
{
    this->useSelectedVolumeFraction = fraction;
    this->Modified();
}


/*!
 *
 */
int svkMetaboliteMap::FillInputPortInformation( int vtkNotUsed(port), vtkInformation* info )
{
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "svkMrsImageData");
    return 1;
}


/*!
 *  Output from this algo is an svkMriImageData object. 
 */
int svkMetaboliteMap::FillOutputPortInformation( int vtkNotUsed(port), vtkInformation* info )
{
    info->Set( vtkDataObject::DATA_TYPE_NAME(), "svkMriImageData"); 
    return 1;
}

