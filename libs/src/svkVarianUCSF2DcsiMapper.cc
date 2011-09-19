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


#include <svkVarianUCSF2DcsiMapper.h>
#include <svkVarianReader.h>
#include <vtkDebugLeaks.h>
#include <vtkTransform.h>
#include <vtkMatrix4x4.h>
#include <vtkByteSwap.h>


using namespace svk;


vtkCxxRevisionMacro(svkVarianUCSF2DcsiMapper, "$Rev$");
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

    //  Assume EVEN sampling about k=0 (k0 not sampled):
    string k0Sampled = "NO";
    this->dcmHeader->SetValue( "SVK_K0Sampled", k0Sampled);

}

