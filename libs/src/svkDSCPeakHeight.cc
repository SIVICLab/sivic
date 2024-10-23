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

#include <svkDSCPeakHeight.h>


using namespace svk;


//vtkCxxRevisionMacro(svkDSCPeakHeight, "$Rev$");
vtkStandardNewMacro(svkDSCPeakHeight);


/*!
 *
 */
svkDSCPeakHeight::svkDSCPeakHeight()
{
#if VTK_DEBUG_ON
    this->DebugOn();
#endif

    vtkDebugMacro(<< this->GetClassName() << "::" << this->GetClassName() << "()");

}



/*!
 *
 */
svkDSCPeakHeight::~svkDSCPeakHeight()
{
    vtkDebugMacro(<<this->GetClassName()<<"::~"<<this->GetClassName());
}



/*! 
 *  Integrate real spectra over specified limits. 
 */
void svkDSCPeakHeight::GenerateMaps()
{

    this->ZeroData(); 

    int numVoxels[3]; 
    this->GetOutput()->GetNumberOfVoxels(numVoxels);
    int totalVoxels = numVoxels[0] * numVoxels[1] * numVoxels[2]; 


    //  Get the data array to initialize.  
    vtkDataArray* dscMapArray;
    dscMapArray = this->GetOutput()->GetPointData()->GetArray(0); 

    //  Add the output volume array to the correct array in the svkMriImageData object
    std::string arrayNameString("pixels");

    dscMapArray->SetName( arrayNameString.c_str() );

    double voxelValue;
    for (int i = 0; i < totalVoxels; i++ ) {

        vtkFloatArray* perfusionDynamics = vtkFloatArray::SafeDownCast( 
            svkMriImageData::SafeDownCast(this->GetImageDataInput(0))->GetCellDataRepresentation()->GetArray(i) 
        ); 
        float* imgPtr = perfusionDynamics->GetPointer(0);

        voxelValue = this->GetMapVoxelValue( imgPtr ); 

        dscMapArray->SetTuple1(i, voxelValue);
    }

    if ( this->normalize ) {

        double nawmValue = this->GetNormalizationFactor(); 
        for (int i = 0; i < totalVoxels; i++ ) {

            voxelValue = dscMapArray->GetTuple1( i );
            voxelValue /= nawmValue; 
            dscMapArray->SetTuple1(i, voxelValue);
        }

    }

}


/*!  
 *  For multi-volume data modifies header's per frame functional group sequence:
 */
double svkDSCPeakHeight::GetMapVoxelValue( float* imgPtr )
{

    double voxelValue; 
    voxelValue = this->GetPeakHt( imgPtr ); 
    return voxelValue;
}


/*!  
 *  Gets max peak height of DSC curve (DeltaR2*).  If the S/N
 *  is < 5 returns 0.
 */
double svkDSCPeakHeight::GetPeakHt( float* imgPtr )
{

    //  get total point range to check:    
    int startPt = 0; 
    int endPt = this->GetImageDataInput(0)->GetDcmHeader()->GetNumberOfTimePoints();
    double peakHt = imgPtr[ startPt ];
    for ( int pt = startPt; pt < endPt; pt ++ ) {
        if ( imgPtr[ pt ] > peakHt ) {
            peakHt = imgPtr[ pt ];
        }
    }


    double noise = this->GetNoise( imgPtr );    
    if ( peakHt == 0. || noise == 0. ) {
        peakHt = 0.;
    }

    //cout << " S/N " << peakHt << " / " << noise << endl;
    if ( peakHt/noise < 4. ) {
        peakHt = 0.;        
    }

    return peakHt; 
    
}
