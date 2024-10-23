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

#include <svkDSCRecovery.h>


using namespace svk;


//vtkCxxRevisionMacro(svkDSCRecovery, "$Rev$");
vtkStandardNewMacro(svkDSCRecovery);


/*!
 *
 */
svkDSCRecovery::svkDSCRecovery()
{
#if VTK_DEBUG_ON
    this->DebugOn();
#endif

    vtkDebugMacro(<< this->GetClassName() << "::" << this->GetClassName() << "()");

}



/*!
 *
 */
svkDSCRecovery::~svkDSCRecovery()
{
    vtkDebugMacro(<<this->GetClassName()<<"::~"<<this->GetClassName());
}


/*! 
 *  Integrate real spectra over specified limits. 
 */
void svkDSCRecovery::GenerateMaps()
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

        voxelValue = this->GetRecovery( imgPtr, i ); 

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
 *  Gets % Recovery from the DSC curve (DeltaR2*).  If the S/N
 *  is < 5 returns 0.
 */
double svkDSCRecovery::GetRecovery( float* imgPtr, int voxelID )
{

    //  get total point range to check:    
    int numPts = this->GetImageDataInput(0)->GetDcmHeader()->GetNumberOfTimePoints();

    int voxelIndex[3];
    this->GetImageDataInput(0)->GetIndexFromID(voxelID, voxelIndex);
    int numVoxels[3];
    this->GetImageDataInput(0)->GetNumberOfVoxels(numVoxels);

    int xMin = 0;
    int delta = 1;
    if ( voxelIndex[0] - delta >= 0 ) {
        xMin = voxelIndex[0] - delta;
    }
    int xMax = numVoxels[0];
    if ( voxelIndex[0] + delta < xMax ) {
        xMax = voxelIndex[0] + delta;
    }

    int yMin = 0;
    if ( voxelIndex[1] - delta >= 0 ) {
        yMin = voxelIndex[1] - delta;
    }
    int yMax = numVoxels[1];
    if ( voxelIndex[1] + delta < yMax ) {
        yMax = voxelIndex[1] + delta;
    }

    int zMin = 0;
    if ( voxelIndex[2] - 1 >= 0 ) {
        zMin = voxelIndex[2];
    }
    int zMax = numVoxels[2];
    if ( voxelIndex[2] + 1 < zMax ) {
        zMax = voxelIndex[2];
    }

    

    double averageRecovery = 0;
    int numPtsToAverage = 0;  
    for ( int x = xMin; x <= xMax; x++  ) {
        for ( int y = yMin; y <= yMax; y++  ) {
           for ( int z = zMin; z <= zMax; z++  ) {

                double slope0;    
                double intercept0;    
                int startPt0 = 5; 
                int endPt0 = 21; 
                this->GetRegression(voxelID, startPt0, endPt0, slope0, intercept0); 

                //  Peak Ht: 
                int startPt = 0; 
                int endPt = this->GetImageDataInput(0)->GetDcmHeader()->GetNumberOfTimePoints();
                double peakHt = imgPtr[ startPt ];
                for ( int pt = startPt; pt < endPt; pt ++ ) {
                    if ( imgPtr[ pt ] > peakHt ) {
                        peakHt = imgPtr[ pt ];
                    }
                }
                peakHt = peakHt - intercept0;
               
               
                //  Post bolus baseline ht:
                double slope1;    
                double intercept1;    
                int startPt1 = numPts - 40; 
                int endPt1 = numPts - 1; 
                this->GetRegression(voxelID, startPt1, endPt1, slope1, intercept1); 
               
                if( peakHt == 0 && intercept0 == 0 && intercept1 == 0 ) {
                	continue;
                }
                double percentRecov = 100.;
                if ( peakHt > 0) {
                    percentRecov = 100 - 100 * (peakHt - (intercept1 - intercept0 ))/ peakHt;
                }
                //if( percentRecov > 100 ) {
                //	percentRecov = 100;
                //}
                //if( percentRecov < 0 ) {
               // 	percentRecov = 0;
                //}
                if( percentRecov > 0 ) {
					averageRecovery += percentRecov;
					numPtsToAverage++;
                }
               
               
           }
        }
    }
    if( numPtsToAverage > 0 ) {
		averageRecovery /= numPtsToAverage;
    }
    return averageRecovery;
}


