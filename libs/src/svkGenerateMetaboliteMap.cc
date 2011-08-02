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

#include <svkGenerateMetaboliteMap.h>
#include <svkSpecPoint.h>


using namespace svk;


vtkCxxRevisionMacro(svkGenerateMetaboliteMap, "$Rev$");
vtkStandardNewMacro(svkGenerateMetaboliteMap);


/*!
 *
 */
svkGenerateMetaboliteMap::svkGenerateMetaboliteMap()
{
#if VTK_DEBUG_ON
    this->DebugOn();
#endif

    vtkDebugMacro(<< this->GetClassName() << "::" << this->GetClassName() << "()");

    this->newSeriesDescription = ""; 
    this->quantificationAlgorithm = svkGenerateMetaboliteMap::INTEGRATE; 
    this->isVerbose = false; 
    this->useSelectedVolumeFraction = 0;
    this->quantificationMask = NULL;


}



/*!
 *
 */
svkGenerateMetaboliteMap::~svkGenerateMetaboliteMap()
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
void svkGenerateMetaboliteMap::SetSeriesDescription( vtkstd::string newSeriesDescription )
{
    this->newSeriesDescription = newSeriesDescription;
    this->Modified(); 
}


/*!
 *  Resets the origin and extent for correct initialization of output svkMriImageData object from input 
 *  svkMrsImageData object. 
 */
int svkGenerateMetaboliteMap::RequestInformation( vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector )
{

    vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
    vtkInformation *outInfo = outputVector->GetInformationObject(0);

    int inWholeExt[6];
    inInfo->Get(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(), inWholeExt);
    double inSpacing[3]; 
    inInfo->Get(vtkDataObject::SPACING(), inSpacing);

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
int svkGenerateMetaboliteMap::RequestData( vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector )
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

    this->GenerateQuantificationMask(); 

    if (this->quantificationAlgorithm == svkGenerateMetaboliteMap::INTEGRATE) { 
        this->Integrate(); 
    } else if (this->quantificationAlgorithm == svkGenerateMetaboliteMap::PEAK_HT) { 
        this->PeakHt(); 
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
void svkGenerateMetaboliteMap::PeakHt()
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

            double peakHt = specPtr[2*startPt];     
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
void svkGenerateMetaboliteMap::Integrate()
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


/*
 *  Determines binary mask indicating whether a given voxel should be quantified (1)  
 *  or not (0). Usually this is based on whether a specified fraction of the voxel 
 *  inside the selected volume. 
 */
void svkGenerateMetaboliteMap::GenerateQuantificationMask() 
{

    if ( this->quantificationMask == NULL ) {
        //  min/max represent the xyz inidices representing
        //  the rectanglular volume of voxels to quantify.  
        //  Default is for the min/max to include all voxels
        int extent[6]; 
        this->GetImageDataInput(0)->GetExtent( extent ); 
        
        int min[3] = {extent[0], extent[2], extent[4]};
        int max[3] = {extent[1], extent[3], extent[5]};

        //  Initialize the mask to 1 (quantify all voxels): 
        int numVoxels[3]; 
        this->GetOutput()->GetNumberOfVoxels(numVoxels);
        int totalVoxels = numVoxels[0] * numVoxels[1] * numVoxels[2]; 
        this->quantificationMask = new short[totalVoxels]; 

        int voxelIndex[3]; 
        if ( this->useSelectedVolumeFraction > 0 ) { 

            for (int voxelID = 0; voxelID < totalVoxels; voxelID++ ) {

                this->quantificationMask[voxelID] = 0; 

                //  Get the slice number for this voxelID: 
                this->GetImageDataInput(0)->GetIndexFromID( voxelID, voxelIndex ); 
            
                //  Get the ID of the voxels that define the TLC and BRC of the 
                //  the selection box for this slice:
                int tlcBrcID[2];
                svkMrsImageData::SafeDownCast(this->GetImageDataInput(0))->Get2DProjectedTlcBrcInSelectionBox( 
                    tlcBrcID, 
                    svkDcmHeader::UNKNOWN_ORIENTATION, 
                    voxelIndex[2], 
                    this->useSelectedVolumeFraction
                );

                this->GetImageDataInput(0)->GetIndexFromID( tlcBrcID[0], min );
                this->GetImageDataInput(0)->GetIndexFromID( tlcBrcID[1], max );

                //  compare the current voxel's indices to the tlcBrcID range: 
                if ( 
                       ( (min[0] <= voxelIndex[0]) && (voxelIndex[0] <= max[0]) ) 
                    && ( (min[1] <= voxelIndex[1]) && (voxelIndex[1] <= max[1]) ) 
                    && ( (min[2] <= voxelIndex[2]) && (voxelIndex[2] <= max[2]) ) 
                ) {
                    this->quantificationMask[voxelID] = 1; 
                }
                cout << "VOXID: " << voxelID << " = " << this->quantificationMask[voxelID] << endl;
                cout << "   voxelINDEX: " << voxelIndex[0] << " " << voxelIndex[1] << " " << voxelIndex[2] << endl;
                cout << "   min: " << min[0] << " " << min[1] << " " << min[2] << endl;
                cout << "   max: " << max[0] << " " << max[1] << " " << max[2] << endl;
            }


        } else {
            for (int voxelID = 0; voxelID < totalVoxels; voxelID++ ) {
                this->quantificationMask[voxelID] = 1; 
            }
        }
    }
}


/*! 
 *  Zero data
 */
void svkGenerateMetaboliteMap::ZeroData()
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
void svkGenerateMetaboliteMap::SetPeakPosPPM( float centerPPM )
{
    this->peakCenterPPM = centerPPM;
    this->Modified(); 
}


/*!
 *  Set the chemical shift range to integrate over.  Integration will be +/- 1/2 this
 *  width about the peak position.
 */
void svkGenerateMetaboliteMap::SetPeakWidthPPM( float widthPPM )
{
    this->peakWidthPPM = widthPPM;
    this->Modified(); 
}


/*!
 *
 */
void svkGenerateMetaboliteMap::GetIntegrationPtRange(int& startPt, int& endPt) 
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
void svkGenerateMetaboliteMap::SetAlgorithmToIntegrate()
{
    this->quantificationAlgorithm = svkGenerateMetaboliteMap::INTEGRATE; 
    this->Modified(); 
}


/*!
 *
 */
void svkGenerateMetaboliteMap::SetAlgorithmToPeakHeight()
{
    this->quantificationAlgorithm = svkGenerateMetaboliteMap::PEAK_HT; 
    this->Modified(); 
}


/*
 *  Set algo type based on string description svkGenerateMetaboliteMap::algorithm. 
 */
void svkGenerateMetaboliteMap::SetAlgorithm( vtkstd::string algo )
{
    if ( algo.compare("INTEGRATE") == 0 ) {
        this->SetAlgorithmToIntegrate(); 
    } else if ( algo.compare("PEAK_HT") == 0 ) {
        this->SetAlgorithmToPeakHeight(); 
    } else {
        vtkWarningWithObjectMacro(this, "SetAlgorithm(): Not a valid algorithm " + algo );
    }
}


/*!
 *
 */
void svkGenerateMetaboliteMap::UpdateProvenance()
{
    vtkDebugMacro(<<this->GetClassName()<<"::UpdateProvenance()");
}


/*!
 *  Write the integrals for each voxel to stdout. Default is false.  
 */
void svkGenerateMetaboliteMap::SetVerbose( bool isVerbose )
{
    this->isVerbose = isVerbose; 
}


/*
 *  Limits the calculation to voxels that have at least the specified fractional
 *  volume within the selected MRS volume. The default is to include all voxels
 *  in the calculation (fraction = 0).
 */
void svkGenerateMetaboliteMap::LimitToSelectedVolume(float fraction)
{
    this->useSelectedVolumeFraction = fraction;
    this->Modified();
}


/*!
 *
 */
int svkGenerateMetaboliteMap::FillInputPortInformation( int vtkNotUsed(port), vtkInformation* info )
{
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "svkMrsImageData");
    return 1;
}


/*!
 *  Output from this algo is an svkMriImageData object. 
 */
int svkGenerateMetaboliteMap::FillOutputPortInformation( int vtkNotUsed(port), vtkInformation* info )
{
    info->Set( vtkDataObject::DATA_TYPE_NAME(), "svkMriImageData"); 
    return 1;
}

