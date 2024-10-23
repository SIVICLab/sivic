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


#ifndef SVK_MRS_KINETICS_H
#define SVK_MRS_KINETICS_H

#define SWARM

#include <itkParticleSwarmOptimizer.h>

#include </usr/include/vtk/vtkObject.h>
#include </usr/include/vtk/vtkObjectFactory.h>
#include </usr/include/vtk/vtkInformation.h>

#include <svkImageData.h>
#include <svkMriImageData.h>
#include <svkImageAlgorithm.h>
#include <svkDcmHeader.h>
#include <svkKineticModelCostFunction.h>

#include <math.h>
#include <stdio.h>
#include <string.h>



namespace svk {


using namespace std;


/*! 
 *  Kinetic fitting 
 *      Christine Leon Swisher, PhD
 *      Cornelius Von Morze, PhD
 *      Jason C. Crane, PhD
 */
class svkMRSKinetics: public svkImageAlgorithm
{

    public:

        vtkTypeMacro( svkMRSKinetics, svkImageAlgorithm);
        static                  svkMRSKinetics* New();

        typedef enum {
            UNDEFINED = 0, 
            FIRST_MODEL = 1, 
            TWO_SITE_EXCHANGE = FIRST_MODEL,
            TWO_SITE_EXCHANGE_PERF, 
            TWO_SITE_IM, 
            TWO_SITE_IM_PYR, 
            LAST_MODEL = TWO_SITE_IM_PYR
        } MODEL_TYPE;


        void                    SetSeriesDescription(std::string newSeriesDescription);
        void                    SetOutputDataType(svkDcmHeader::DcmPixelDataFormat dataType);
        void                    SetZeroCopy(bool zeroCopy); 
        void                    SetModelType( svkMRSKinetics::MODEL_TYPE modelType ); 
        void                    SetTR( float TR ); 
        float                   GetTR( ); 
        int                     GetNumberOfModelOutputPorts(); 
        int                     GetNumberOfModelSignals(); 
        string                  GetModelOutputDescription( int outputIndex ); 
        void                    SetCustomParamSearchBounds(
                                    vector<int>* customBoundsParamNumbers, 
                                    vector<float>* customLowerBounds, 
                                    vector<float>* customUpperBounds);


    protected:

        svkMRSKinetics();
        ~svkMRSKinetics();

        virtual int             RequestInformation(
                                    vtkInformation* request,
                                    vtkInformationVector** inputVector,
                                    vtkInformationVector* outputVector
                                );
        virtual int             RequestData( 
                                    vtkInformation* request, 
                                    vtkInformationVector** inputVector, 
                                    vtkInformationVector* outputVector 
                                );

        void                    ZeroData(); 
        void                    SyncPointsFromCells(); 

        virtual int             FillInputPortInformation( int vtkNotUsed(port), vtkInformation* info );
        virtual int             FillOutputPortInformation( int vtkNotUsed(port), vtkInformation* info );         


        virtual void            UpdateProvenance();


        //  Members:
        std::string          newSeriesDescription; 


    private:
        void                    GenerateKineticParamMap();
        void                    InitAverageDynamics(bool hasMask, int totalVoxels, unsigned short* mask); 

        void                    FitVoxelKinetics( int voxelID ); 

        void                    InitOptimizer(  itk::ParticleSwarmOptimizer::Pointer itkOptimizer, int voxelID ); 
        void                    InitCostFunction( 
                                    svkKineticModelCostFunction::Pointer& costFunction, 
                                    int voxelID 
                                ); 
        void                    GetCostFunction( svkKineticModelCostFunction::Pointer& costFunction); 
        int                     GetNumberOfModelParameters(); 
        void                    InitModelOutputDescriptionVector(); 
        void                    PrintInitialParamBounds(); 


        float*                      metKinetics0;
        float*                      metKinetics1;
        float*                      metKinetics2;
        int                         currentTimePoint; 
        int                         numTimePoints; 
        int                         num3DOutputMaps;
        vtkDataArray*               mapArrayKpl; 
        vtkDataArray*               mapArrayT1all; 
        vtkDataArray*               mapArrayKtrans; 
        svkMRSKinetics::MODEL_TYPE  modelType; 
        vector<string>              modelOutputDescriptionVector;
        vector<vtkFloatArray*>      averageSignalVector;
        float                       TR; 
        vector<int>*                customBoundsParamNumbers; 
        vector<float>*              customLowerBounds; 
        vector<float>*              customUpperBounds; 

};


}   //svk


#endif //SVK_MRS_KINETICS_H

