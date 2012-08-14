/*
 *  Copyright © 2009-2012 The Regents of the University of California.
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

#include <svkDynamicMRIAlgoTemplate.h>



using namespace svk;


vtkCxxRevisionMacro(svkDynamicMRIAlgoTemplate, "$Rev$");
vtkStandardNewMacro(svkDynamicMRIAlgoTemplate);


int fcn_lmder(void *p, int m, int n, const real *x, real *fvec, real *fjac, int ldfjac, int iflag);
int fcn_lmdif(void *p, int m, int n, const real *x, real *fvec, int iflag); 
                                   


/*!
 *
 */
svkDynamicMRIAlgoTemplate::svkDynamicMRIAlgoTemplate()
{
#if VTK_DEBUG_ON
    this->DebugOn();
#endif

    vtkDebugMacro(<< this->GetClassName() << "::" << this->GetClassName() << "()");

    this->newSeriesDescription = ""; 
    //  3 required input ports: 
    this->SetNumberOfInputPorts(3);

}



/*!
 *
 */
svkDynamicMRIAlgoTemplate::~svkDynamicMRIAlgoTemplate()
{
    vtkDebugMacro(<<this->GetClassName()<<"::~"<<this->GetClassName());
}


/*!
 *  Set the series description for the DICOM header of the copy.  
 */
void svkDynamicMRIAlgoTemplate::SetSeriesDescription( vtkstd::string newSeriesDescription )
{
    this->newSeriesDescription = newSeriesDescription;
    this->Modified(); 
}


/*!
 *  Resets the origin and extent for correct initialization of output svkMriImageData object from input 
 *  svkMrsImageData object. 
 */
int svkDynamicMRIAlgoTemplate::RequestInformation( vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector )
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
int svkDynamicMRIAlgoTemplate::RequestData( vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector )
{

    //  Create the template data object by  
    //  extractng an svkMriImageData from the input svImageData object
    //  Use an arbitrary point for initialization of scalars.  Actual data 
    //  will be overwritten by algorithm. 
    int indexArray[1];
    indexArray[0] = 0;
    svkMriImageData::SafeDownCast( this->GetImageDataInput(0) )->GetCellDataRepresentation()->GetImage(
        svkMriImageData::SafeDownCast( this->GetOutput() ), 
        0, 
        this->newSeriesDescription, 
        indexArray, 
        0 
    ); 

    this->GenerateKineticParamMap();

    svkDcmHeader* hdr = this->GetOutput()->GetDcmHeader();
    hdr->InsertUniqueUID("SeriesInstanceUID");
    hdr->InsertUniqueUID("SOPInstanceUID");
    hdr->SetValue("SeriesDescription", this->newSeriesDescription);

    return 1; 
};


/*!
 *  Generate 3D image parameter map from analysis of dynamic data.  
 */
void svkDynamicMRIAlgoTemplate::GenerateKineticParamMap()
{

    this->ZeroData();

    int numVoxels[3];
    this->GetOutput()->GetNumberOfVoxels(numVoxels);
    int totalVoxels = numVoxels[0] * numVoxels[1] * numVoxels[2];


    //  Get the data array to initialize.  
    vtkDataArray* kineticsMapArray;
    kineticsMapArray = this->GetOutput()->GetPointData()->GetArray(0);

    //  Add the output volume array to the correct array in the svkMriImageData object
    vtkstd::string arrayNameString("pixels");

    kineticsMapArray->SetName( arrayNameString.c_str() );

    double voxelValue;
    for (int i = 0; i < totalVoxels; i++ ) {

        cout << "VOXEL NUMBER: " << i << endl;
        vtkFloatArray* kineticTrace0 = vtkFloatArray::SafeDownCast(
            svkMriImageData::SafeDownCast(this->GetImageDataInput(0))->GetCellDataRepresentation()->GetArray(i)
        );
        vtkFloatArray* kineticTrace1 = vtkFloatArray::SafeDownCast(
            svkMriImageData::SafeDownCast(this->GetImageDataInput(1))->GetCellDataRepresentation()->GetArray(i)
        );
        vtkFloatArray* kineticTrace2 = vtkFloatArray::SafeDownCast(
            svkMriImageData::SafeDownCast(this->GetImageDataInput(2))->GetCellDataRepresentation()->GetArray(i)
        );

        //cout << "NUM COMP: " << kineticTrace->GetNumberOfComponents() << endl;
        //cout << "NUM TUPS: " << kineticTrace->GetNumberOfTuples() << endl;

        float* metKinetics0 = kineticTrace0->GetPointer(0);
        float* metKinetics1 = kineticTrace1->GetPointer(0);
        float* metKinetics2 = kineticTrace2->GetPointer(0);

        voxelValue = this->GetKineticsMapVoxelValue( metKinetics0, metKinetics1, metKinetics2  );

        kineticsMapArray->SetTuple1(i, voxelValue);
    }
}


/*!  
 *  Fit the kinetics for a single voxel. 
 */
double svkDynamicMRIAlgoTemplate::GetKineticsMapVoxelValue( float* metKinetics0, float* metKinetics1, float* metKinetics2)
{

    double voxelValue;

    //  get num points in kinetic trace: 
    int numPts = this->GetImageDataInput(0)->GetDcmHeader()->GetNumberOfTimePoints();

    //  For testing, just get max intensity:
    float maxValue = metKinetics0[0]; 
    for ( int i = 0; i < numPts; i++ ) {
        cout << "   val: " << i << " " << metKinetics0[i] << " " << metKinetics1[i] << " " <<  metKinetics2[i] << endl;
        if ( metKinetics0[i] > maxValue) {
            maxValue = metKinetics0[ i ]; 
        }
    }

    //  Try to call cminpack function:
    /* the following struct defines the data points */
    typedef struct  {
        int m;
        real *y;
    } fcndata_t;
    fcndata_t data;
    const int m = 15;
    const int n = 3;
    int i, j, ldfjac, maxfev, mode, nprint, info, nfev, njev;
    int ipvt[3];
    real ftol, xtol, gtol, factor, fnorm, epsfcn;
    real x[3], fvec[15], fjac[15*3], diag[3], qtf[3],
        wa1[3], wa2[3], wa3[3], wa4[15];
    int k;

    //  lmder:
    //info = __cminpack_func__(lmder)(fcn_lmder, &data, m, n, x, fvec, fjac, ldfjac, ftol, xtol, gtol,
    //maxfev, diag, mode, factor, nprint, &nfev, &njev,
    //ipvt, qtf, wa1, wa2, wa3, wa4);

    //  lmdif:
    epsfcn = 0.; 
    info = __cminpack_func__(lmdif)(fcn_lmdif, &data, m, n, x, fvec, ftol, xtol, gtol, maxfev, epsfcn,
    diag, mode, factor, nprint, &nfev, fjac, ldfjac,
    ipvt, qtf, wa1, wa2, wa3, wa4);
 

    cout << "   MAX VAL: " << maxValue << endl;
    return maxValue;

}


/*
 *  lmder stub
 */
int fcn_lmder(void *p, int m, int n, const real *x, real *fvec, real *fjac, int ldfjac, int iflag)
{
    return 0; 
}


/*
 *  lmdif stub
 */
int fcn_lmdif(void *p, int m, int n, const real *x, real *fvec, int iflag)
{
    return 0; 
}



/*! 
 *  Zero data
 */
void svkDynamicMRIAlgoTemplate::ZeroData()
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
void svkDynamicMRIAlgoTemplate::UpdateProvenance()
{
    vtkDebugMacro(<<this->GetClassName()<<"::UpdateProvenance()");
}


/*!
 *  input ports 0 - 2 are required. All input ports are for dynamic MRI data. 
 */
int svkDynamicMRIAlgoTemplate::FillInputPortInformation( int port, vtkInformation* info )
{
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "svkMriImageData");
    return 1;
}



/*!
 *  Output from this algo is an svkMriImageData object. 
 */
int svkDynamicMRIAlgoTemplate::FillOutputPortInformation( int vtkNotUsed(port), vtkInformation* info )
{
    info->Set( vtkDataObject::DATA_TYPE_NAME(), "svkMriImageData"); 
    return 1;
}

