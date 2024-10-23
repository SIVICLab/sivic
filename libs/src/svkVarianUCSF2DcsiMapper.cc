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


#include <svkVarianUCSF2DcsiMapper.h>
#include <svkVarianReader.h>
#include </usr/include/vtk/vtkDebugLeaks.h>
#include </usr/include/vtk/vtkTransform.h>
#include </usr/include/vtk/vtkMatrix4x4.h>
#include </usr/include/vtk/vtkByteSwap.h>


using namespace svk;


//vtkCxxRevisionMacro(svkVarianUCSF2DcsiMapper, "$Rev$");
vtkStandardNewMacro(svkVarianUCSF2DcsiMapper);



/*!
 *
 */
svkVarianUCSF2DcsiMapper::svkVarianUCSF2DcsiMapper()
{

#if VTK_DEBUG_ON
    this->DebugOn();
    vtkDebugLeaks::ConstructClass("svkVarianUCSF2DcsiMapper");
#endif

    vtkDebugMacro( << this->GetClassName() << "::" << this->GetClassName() << "()" );

    this->specData = NULL;

}


/*!
 *
 */
svkVarianUCSF2DcsiMapper::~svkVarianUCSF2DcsiMapper()
{
    vtkDebugMacro( << this->GetClassName() << "::~" << this->GetClassName() << "()" );

    if ( this->specData != NULL )  {
        delete [] specData;
        this->specData = NULL;
    }

}


/*!
 *  Pixel Spacing:
 */
void svkVarianUCSF2DcsiMapper::InitPixelMeasuresMacro()
{

    float numPixels[3];
    numPixels[0] = this->GetHeaderValueAsInt("nv", 0);
    numPixels[1] = this->GetHeaderValueAsInt("nv2", 0);
    numPixels[2] = this->GetHeaderValueAsInt("ns", 0);

    //  lpe (phase encode resolution in cm, already converted in base class)
    float pixelSize[3];
    pixelSize[0] = this->GetHeaderValueAsFloat("lpe", 0) / numPixels[0] ;
    pixelSize[1] = this->GetHeaderValueAsFloat("lpe2", 0) /  numPixels[1];
    pixelSize[2] = this->GetHeaderValueAsFloat("thk", 0) / numPixels[2];

    std::string pixelSizeString[3];

    for (int i = 0; i < 3; i++) {
        ostringstream oss;
        oss << pixelSize[i];
        pixelSizeString[i].assign( oss.str() );
    }

    this->dcmHeader->InitPixelMeasuresMacro(
        pixelSizeString[0] + "\\" + pixelSizeString[1],
        pixelSizeString[2]
    );
}


/*!
 *  The FID toplc is the center of the first voxel.
 */
void svkVarianUCSF2DcsiMapper::InitPerFrameFunctionalGroupMacros()
{

    double dcos[3][3];
    this->dcmHeader->SetSliceOrder( this->dataSliceOrder );
    this->dcmHeader->GetDataDcos( dcos );
    double pixelSpacing[3];
    this->dcmHeader->GetPixelSize(pixelSpacing);

    int numPixels[3];
    numPixels[0] = this->GetHeaderValueAsInt("nv", 0);
    numPixels[1] = this->GetHeaderValueAsInt("nv2", 0);
    numPixels[2] = this->GetHeaderValueAsInt("ns", 0);

    //  Get center coordinate float array from fidMap and use that to generate
    //  Displace from that coordinate by 1/2 fov - 1/2voxel to get to the center of the
    //  toplc from which the individual frame locations are calculated


    //  Center of toplc (LPS) pixel in frame:
    double toplc[3];

    //
    //  If 3D vol, calculate slice position, otherwise use value encoded
    //  into slice header
    //

     //  If 2D (single slice)
     if ( numPixels[2] == 1 ) {

        //  Location is the center of the image frame in user (acquisition frame).
        double centerAcqFrame[3];
        centerAcqFrame[0] = this->GetHeaderValueAsFloat("ppe", 0);
        centerAcqFrame[1] = this->GetHeaderValueAsFloat("ppe2", 0);
        centerAcqFrame[2] = this->GetHeaderValueAsFloat("pro", 0);

        //  Account for slice offset (pss is offset in cm):
        int mmPerCm = 10; 
        centerAcqFrame[2] += this->GetHeaderValueAsFloat("pss", 0) * mmPerCm ;


        //  Now get the center of the tlc voxel in the acq frame:
        double* tlcAcqFrame = new double[3];
        for (int j = 0; j < 2; j++) {
            tlcAcqFrame[j] = centerAcqFrame[j]
                - ( ( numPixels[j] * pixelSpacing[j] ) - pixelSpacing[j] )/2;
        }
        tlcAcqFrame[2] = centerAcqFrame[2];

        //  and convert to LPS (magnet) frame:
        svkVarianReader::UserToMagnet(tlcAcqFrame, toplc, dcos);

        delete [] tlcAcqFrame;

    } 

    svkDcmHeader::DimensionVector dimensionVector = this->dcmHeader->GetDimensionIndexVector();
    svkDcmHeader::SetDimensionVectorValue(&dimensionVector, svkDcmHeader::SLICE_INDEX, numFrames-1);

    this->dcmHeader->InitPerFrameFunctionalGroupSequence(
        toplc, pixelSpacing, dcos, &dimensionVector
    );

}



/*!
 *
 */
void svkVarianUCSF2DcsiMapper::InitMRSpectroscopyPulseSequenceModule()
{
    this->dcmHeader->SetValue(
        "PulseSequenceName",
        this->GetHeaderValueAsString( "seqfil" )
    );

    int numVoxels[3];
    numVoxels[0] = this->GetHeaderValueAsInt("nv", 0);
    numVoxels[1] = this->GetHeaderValueAsInt("nv2", 0);
    numVoxels[2] = this->GetHeaderValueAsInt("ns", 0);

    string acqType = "VOLUME";
    if (numVoxels[0] == 1 && numVoxels[1] == 1 &&  numVoxels[2] == 1) {
        acqType = "SINGLE VOXEL";
    }

    this->dcmHeader->SetValue(
        "MRSpectroscopyAcquisitionType",
        acqType
    );

    this->dcmHeader->SetValue(
        "EchoPulseSequence",
        "SPIN"
    );

    this->dcmHeader->SetValue(
        "MultipleSpinEcho",
        "NO"
    );

    this->dcmHeader->SetValue(
        "MultiPlanarExcitation",
        "NO"
    );

    this->dcmHeader->SetValue(
        "SteadyStatePulseSequence",
        "NONE"
    );


    this->dcmHeader->SetValue(
        "EchoPlanarPulseSequence",
        "NO"
    );

    this->dcmHeader->SetValue(
        "SpectrallySelectedSuppression",
        ""
    );

    this->dcmHeader->SetValue(
        "GeometryOfKSpaceTraversal",
        "RECTILINEAR"
    );

    this->dcmHeader->SetValue(
        "RectilinearPhaseEncodeReordering",
        "LINEAR"
    );

    this->dcmHeader->SetValue(
        "SegmentedKSpaceTraversal",
        "SINGLE"
    );

    this->dcmHeader->SetValue(
        "CoverageOfKSpace",
        "FULL"
    );

    this->dcmHeader->SetValue( "NumberOfKSpaceTrajectories", 1 );

    //  k = 0 is NOT sampled in the acquisition. 
    string k0Sampled = "NO";
    this->dcmHeader->SetValue( "SVK_K0Sampled", k0Sampled);

}
