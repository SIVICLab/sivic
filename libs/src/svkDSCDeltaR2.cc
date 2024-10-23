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

#include <svkDSCDeltaR2.h>


using namespace svk;


//vtkCxxRevisionMacro(svkDSCDeltaR2, "$Rev$");
vtkStandardNewMacro(svkDSCDeltaR2);


/*!
 *
 */
svkDSCDeltaR2::svkDSCDeltaR2()
{
#if VTK_DEBUG_ON
    this->DebugOn();
#endif

    vtkDebugMacro(<< this->GetClassName() << "::" << this->GetClassName() << "()");

    this->currentRepresentation = svkDSCDeltaR2::T2; 
    this->targetRepresentation = svkDSCDeltaR2::T2; 
}



/*!
 *
 */
svkDSCDeltaR2::~svkDSCDeltaR2()
{
    vtkDebugMacro(<<this->GetClassName()<<"::~"<<this->GetClassName());
}



/*!
 *  Change the representation between T2* and DR2*
 *  See reference2: 
 *      ΔR2* = −ln(St/S0)/TE
 *  For now, set S0, to be the value at point 5 after magnetization has stabilized.
 */
int svkDSCDeltaR2::RequestData( vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector )
{
    svk4DImageData* dsc = svkMriImageData::SafeDownCast(this->GetImageDataInput(0))
                    ->GetCellDataRepresentation();
    double TE = dsc->GetDcmHeader()->GetDoubleValue("EchoTime"); 

    int numVoxels[3];    
    dsc->GetNumberOfVoxels( numVoxels ); 
    int totalVoxels = numVoxels[0] * numVoxels[1] * numVoxels[2]; 

    //  Loop through each voxel and convert to target representation:     
    if ( this->currentRepresentation != this->targetRepresentation ) {
        vtkDataArray* voxelData;
        voxelData = dsc->GetArray(0);
        int numPts = voxelData->GetNumberOfTuples(); 

        if (this->targetRepresentation == svkDSCDeltaR2::DR2 ) {

            //  Change representation from T2* to delta R2* 

            for (int i = 0; i < totalVoxels; i++ )    {
                voxelData = dsc->GetArray(i);
                double s0 = voxelData->GetTuple1(0); 
                for ( int t = 0; t < numPts; t++ ) {
                    if ( s0 > 0 ) {
                        double s1 = voxelData->GetTuple1(t); 
                        if (s1 > 0 ) {
                            double value = -1 * log( s1 / s0 ); 
                            value = value / TE;
                            voxelData->SetTuple1( t, value ); 
                        } else {
                            voxelData->SetTuple1( t, 0); 
                        }   
                    } else {  
                        voxelData->SetTuple1( t, 0); 
                    }
                }
            }                
            this->currentRepresentation = svkDSCDeltaR2::DR2; 

        } else {

            //  Change representation delta R2* to T2* 
            cout << "revert to T2" << endl;
            svkMriImageData::SafeDownCast(this->GetImageDataInput(0))
                    ->SyncCellRepresentationToPixelData();

            this->currentRepresentation = svkDSCDeltaR2::T2; 
        }
    }  

    dsc->Modified();
    return 1; 
};


/*!
 *
 */
void svkDSCDeltaR2::SetRepresentation( svkDSCDeltaR2::representation representation )
{
    cout << "SET REPRESENTATIN: " << representation << endl;
    this->targetRepresentation = representation; 
    this->Modified(); 
}



/*!
 *  Output from this algo is an svkMriImageData object. 
 */
int svkDSCDeltaR2::FillInputPortInformation( int vtkNotUsed(port), vtkInformation* info )
{
    info->Set( vtkDataObject::DATA_TYPE_NAME(), "svkMriImageData"); 
    return 1;
}

