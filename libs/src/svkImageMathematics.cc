/*
 *  Copyright © 2009-2014 The Regents of the University of California.
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


#include <vtkInstantiator.h>
#include <vtkInformation.h>
#include <vtkDataSetAttributes.h>

#include <svkImageMathematics.h>
#include <svkMriImageData.h>


using namespace svk;


vtkCxxRevisionMacro(svkImageMathematics, "$Rev$");
vtkStandardNewMacro(svkImageMathematics);


/*!
 *
 */
svkImageMathematics::svkImageMathematics()
{

#if VTK_DEBUG_ON
    this->DebugOn();
#endif

    vtkDebugMacro(<<this->GetClassName() << "::" << this->GetClassName() << "()");

    vtkInstantiator::RegisterInstantiator("svkMriImageData", svkMriImageData::NewObject);

    this->SetNumberOfInputPorts(3); 
}


/*!
 *
 */
svkImageMathematics::~svkImageMathematics()
{
}


/*!
 *  This method loops over all volumes and calls the VTK super class update method on each to 
 *  perform the specified calculation.  If necessary the output will be masked by an input mask.
 */
void svkImageMathematics::Update()
{
    //  Determine how many vtkPointData arrays are in the input data
    int numVolumes = this->GetImageDataInput(0)->GetPointData()->GetNumberOfArrays();
  
    //  Set the active scalars for each relevant input and output arrays:    
    //  Determine number of input ports for this particular operation (1 or 2).    
    svkImageData* tmp = svkMriImageData::New();
    tmp->DeepCopy( this->GetImageDataInput(0) ); 

    for ( int vol = 0; vol < numVolumes; vol++ ) {

        if (this->GetInput(0)) {
            this->GetImageDataInput(0)->GetPointData()->SetActiveAttribute (vol, vtkDataSetAttributes::SCALARS); 
        }    

        if (this->GetInput(1)) {
            this->GetImageDataInput(1)->GetPointData()->SetActiveAttribute (vol, vtkDataSetAttributes::SCALARS); 
            
        }    
    
        if (this->GetOutput()) {
            this->GetOutput()->GetPointData()->SetActiveAttribute(vtkDataSetAttributes::SCALARS, vol);
        }    
        
        this->Superclass::Update(); 


        //  Mask the data if a mask was provided.  Preferably the masking would be applied by 
        //  limiting the extent fo the vtkImagetMathematics operation, but I'll just zero out the 
        //  results here for now. 
        if (this->GetInput( svkImageMathematics::MASK )) {
            
            vtkDataArray* outArray = this->GetOutput()->GetPointData()->GetScalars();    // returns a vtkDataArray

            svkMriImageData* maskImage = svkMriImageData::SafeDownCast(this->GetImageDataInput(svkImageMathematics::MASK) );
            unsigned short* mask = static_cast<vtkUnsignedShortArray*>( 
                        maskImage->GetPointData()->GetArray(0) )->GetPointer(0) ; 

            int numVoxels[3];
            svkMriImageData::SafeDownCast(this->GetOutput(0))->GetNumberOfVoxels(numVoxels);
            int totalVoxels = numVoxels[0] * numVoxels[1] * numVoxels[2];

            for ( int i = 0; i < totalVoxels; i++ ) {
                if ( mask[i] == 0 ) { 
                    outArray->SetTuple1( i, 0 );    // set value to 0 outside mask: 
                }
            }
        }


        //  Push results into correct array in tmp data
        tmp->GetPointData()->GetArray(vol)->DeepCopy( this->GetOutput()->GetPointData()->GetScalars() ); 
    }

    //  Now copy the multi-volume output results back into the  algorithm's output object. 
    svkMriImageData::SafeDownCast(this->GetOutput())->DeepCopy( tmp ); 
    tmp->Delete();
        
}


/*!
 *  Default output type is same concrete sub class type as the input data.  Override with 
 *  specific concrete type in sub-class if necessary.  
 */
int svkImageMathematics::FillOutputPortInformation( int vtkNotUsed(port), vtkInformation* info )
{
    info->Set( vtkDataObject::DATA_TYPE_NAME(), this->GetImageDataInput(0)->GetClassName() );
    return 1;
}


/*!
 *  Default input type is svkImageData base class. Override with a specific concrete type in 
 *  sub class if necessary. 
 *  Port 0 -> input 1 image to operate on 
 *       1 -> input 2 image to operate on (optional)
 *       2 -> mask (optional)
 */
int svkImageMathematics::FillInputPortInformation( int port, vtkInformation* info )
{
    if ( port == 0 ) {
        info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "svkMriImageData");
    } 
    if ( port == 1 ) {
        info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "svkMriImageData");
        info->Set(vtkAlgorithm::INPUT_IS_OPTIONAL(), 1);
    } 
    if ( port == 2 ) {
        info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "svkMriImageData");
        info->Set(vtkAlgorithm::INPUT_IS_OPTIONAL(), 2);
    } 
    return 1;
}


