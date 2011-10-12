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
#include <vtkXMLDataElement.h>
#include <vtkXMLUtilities.h>

#include <svkMetaboliteMap.h>
#include <svkSpecPoint.h>


using namespace svk;


vtkCxxRevisionMacro(svkMetaboliteMap, "$Rev$");
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
void svkMetaboliteMap::SetSeriesDescription( vtkstd::string newSeriesDescription )
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
        this->newSeriesDescription 
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
    if (this->quantificationAlgorithm == svkMetaboliteMap::INTEGRATE) { 
        this->Integrate(); 
    } else if (this->quantificationAlgorithm == svkMetaboliteMap::PEAK_HT) { 
        this->PeakHt(); 
    } else if (this->quantificationAlgorithm == svkMetaboliteMap::MAG_PEAK_HT) { 
        this->MagPeakHt(); 
    }

    svkDcmHeader* hdr = this->GetOutput()->GetDcmHeader();
    hdr->InsertUniqueUID("SeriesInstanceUID");
    hdr->InsertUniqueUID("SOPInstanceUID");
    hdr->SetValue("SeriesDescription", this->newSeriesDescription);

    return 1; 
};


/*! 
 *  Peak height of real spectra within specified limits. 
 */
void svkMetaboliteMap::PeakHt()
{

    this->ZeroData(); 

    //  Get integration limits:
    int startPt = 0; 
    int endPt = 0; 
    this->GetIntegrationPtRange(startPt, endPt); 

    //  If integration limits are out of range just returned zero'd image.
    int numSpecPoints = this->GetImageDataInput(0)->GetDcmHeader()->GetIntValue("DataPointColumns");
    if ( startPt < 0 || endPt >= numSpecPoints ) {
        vtkWarningWithObjectMacro(this, "Integration limits out of range, returning zero value map");
        return; 
    }

    int numVoxels[3]; 
    this->GetOutput()->GetNumberOfVoxels(numVoxels);
    int totalVoxels = numVoxels[0] * numVoxels[1] * numVoxels[2]; 
    double peakHt; 
    for (int i = 0; i < totalVoxels; i++ ) {

        if ( this->quantificationMask[i] ) {
            vtkFloatArray* spectrum = vtkFloatArray::SafeDownCast( 
                svkMrsImageData::SafeDownCast(this->GetImageDataInput(0))->GetSpectrumFromID(i) 
            ); 
            float* specPtr = spectrum->GetPointer(0);

            peakHt = specPtr[2*startPt];     
            for ( int pt = startPt; pt <= endPt; pt ++ ) {
                if ( specPtr[2*pt] > peakHt ) {
                    peakHt = specPtr[2*pt]; 
                }
            }
        } else {
            peakHt = 0.; 
        }
        if ( this->isVerbose ) {
            cout << "voxel(" << i << ") ppm center: " << this->peakCenterPPM << " peak_ht: " << peakHt << endl; 
        }
        this->GetOutput()->GetPointData()->GetScalars()->SetTuple1(i, peakHt);
    }
}


/*! 
 *  Integrate real spectra over specified limits. 
 */
void svkMetaboliteMap::Integrate()
{

    this->ZeroData(); 

    //  Get integration limits:
    int startPt = 0; 
    int endPt = 0; 
    this->GetIntegrationPtRange(startPt, endPt); 

    //  If integration limits are out of range just returned zero'd image.
    int numSpecPoints = this->GetImageDataInput(0)->GetDcmHeader()->GetIntValue("DataPointColumns");
    if ( startPt < 0 || endPt >= numSpecPoints ) {
        vtkWarningWithObjectMacro(this, "Integration limits out of range, returning zero value map");
        return; 
    }

    int numVoxels[3]; 
    this->GetOutput()->GetNumberOfVoxels(numVoxels);
    int totalVoxels = numVoxels[0] * numVoxels[1] * numVoxels[2]; 
    double integral;
    for (int i = 0; i < totalVoxels; i++ ) {

        if ( this->quantificationMask[i] ) {
            vtkFloatArray* spectrum = vtkFloatArray::SafeDownCast( 
                svkMrsImageData::SafeDownCast(this->GetImageDataInput(0))->GetSpectrumFromID(i) 
            ); 
            float* specPtr = spectrum->GetPointer(0);

            integral = 0;     
            for ( int pt = startPt; pt <= endPt; pt ++ ) {
                integral += specPtr[2*pt]; 
            }
        } else {
            integral = 0.;
        }
        if ( this->isVerbose ) {
            cout << "voxel(" << i << ") ppm center: " << this->peakCenterPPM << " integral: " << integral << endl; 
        }
        this->GetOutput()->GetPointData()->GetScalars()->SetTuple1(i, integral);
    }

}


/*! 
 *  Peak height of magnitude spectra within specified limits. 
 */
void svkMetaboliteMap::MagPeakHt()
{

    this->ZeroData(); 

    //  Get integration limits:
    int startPt = 0; 
    int endPt = 0; 
    this->GetIntegrationPtRange(startPt, endPt); 

    //  If integration limits are out of range just returned zero'd image.
    int numSpecPoints = this->GetImageDataInput(0)->GetDcmHeader()->GetIntValue("DataPointColumns");
    if ( startPt < 0 || endPt >= numSpecPoints ) {
        vtkWarningWithObjectMacro(this, "Integration limits out of range, returning zero value map");
        return; 
    }

    int numVoxels[3]; 
    this->GetOutput()->GetNumberOfVoxels(numVoxels);
    int totalVoxels = numVoxels[0] * numVoxels[1] * numVoxels[2]; 
    double magPeakHt; 
    double magPeakHtTmp; 
    for (int i = 0; i < totalVoxels; i++ ) {

        if ( this->quantificationMask[i] ) {
            vtkFloatArray* spectrum = vtkFloatArray::SafeDownCast( 
                svkMrsImageData::SafeDownCast(this->GetImageDataInput(0))->GetSpectrumFromID(i) 
            ); 
            float* specPtr = spectrum->GetPointer(0);

            magPeakHt  = pow( specPtr[2*startPt], 2);     
            magPeakHt += pow( specPtr[2*startPt + 1], 2);     
            magPeakHt = pow( magPeakHt, 0.5); 

            for ( int pt = startPt; pt <= endPt; pt ++ ) {

                magPeakHtTmp  = pow( specPtr[2*pt], 2); 
                magPeakHtTmp += pow( specPtr[2*pt + 1], 2); 
                magPeakHtTmp  = pow(magPeakHtTmp, 0.5); 

                if ( magPeakHtTmp > magPeakHt ) {
                    magPeakHt = magPeakHtTmp;        
                }
            }
        } else {
            magPeakHt = 0.; 
        }
        if ( this->isVerbose ) {
            cout << "voxel(" << i << ") ppm center: " << this->peakCenterPPM << " peak_ht: " << magPeakHt << endl; 
        }
        this->GetOutput()->GetPointData()->GetScalars()->SetTuple1(i, magPeakHt);
    }
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
 *
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


/*
 *  Set algo type based on string description svkMetaboliteMap::algorithm. 
 */
void svkMetaboliteMap::SetAlgorithm( vtkstd::string algo )
{
    if ( algo.compare("INTEGRATE") == 0 ) {
        this->SetAlgorithmToIntegrate(); 
    } else if ( algo.compare("PEAK_HT") == 0 ) {
        this->SetAlgorithmToPeakHeight(); 
    } else if ( algo.compare("MAG_PEAK_HT") == 0 ) {
        this->SetAlgorithmToMagPeakHeight(); 
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

