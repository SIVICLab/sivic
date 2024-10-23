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


#include </usr/include/vtk/vtkInformationVector.h>
#include </usr/include/vtk/vtkStreamingDemandDrivenPipeline.h>
#include </usr/include/vtk/vtkMultiThreader.h>

#include <svkThreadedImageAlgorithm.h>


using namespace svk;


//vtkCxxRevisionMacro(svkThreadedImageAlgorithm, "$Rev$");


/*!
 *
 */
svkThreadedImageAlgorithm::svkThreadedImageAlgorithm()
{

#if VTK_DEBUG_ON
    this->DebugOn();
#endif

    vtkDebugMacro(<<this->GetClassName() << "::" << this->GetClassName() << "()");

    vtkInstantiator::RegisterInstantiator("svkMriImageData", svkMriImageData::NewObject);
    vtkInstantiator::RegisterInstantiator("svkMrsImageData", svkMrsImageData::NewObject);
    vtkInstantiator::RegisterInstantiator("svk4DImageData",  svk4DImageData::NewObject);

    vtkMultiThreader::SetGlobalDefaultNumberOfThreads(0);

    // override in sub-class if necessary, but also reimplent virtual GetOutput methods.
    this->SetNumberOfInputPorts(1);
    this->SetNumberOfOutputPorts(1); 

}


/*!
 *
 */
svkThreadedImageAlgorithm::~svkThreadedImageAlgorithm()
{
}


/*!
 *  Modification of default vtk SplitExtent.  The default version splits starting on 
 *  the 3rd axis, but here for better efficiency the split is done on the axis with the
 *  largest nuber of cells.
 *
 *  For streaming and threads.  Splits output update extent into num pieces.
 *  This method needs to be called num times.  Results must not overlap for
 *  consistent starting extent.  Subclass can override this method.
 *  This method returns the number of peices resulting from a successful split.
 *  This can be from 1 to "total".  
 *  If 1 is returned, the extent cannot be split.
 */
int svkThreadedImageAlgorithm::SplitExtent(int splitExt[6],int startExt[6],int num, int total)
{
    int splitAxis;
    int min, max;

    vtkDebugMacro("SplitExtent: ( " << startExt[0] << ", " << startExt[1] << ", "
                << startExt[2] << ", " << startExt[3] << ", "
                << startExt[4] << ", " << startExt[5] << "), "
                << num << " of " << total);

    // start with same extent
    memcpy(splitExt, startExt, 6 * sizeof(int));

    splitAxis = 0;
    int maxAxisSize = 0;     
    for ( int axis = 0; axis < 3; axis++ ) {
        int axisSize = startExt[axis*2 + 1] - startExt[axis*2]; 
        if ( axisSize >= maxAxisSize ) {
            maxAxisSize = axisSize; 
            splitAxis = axis; 
            min = startExt[axis*2]; 
            max = startExt[axis*2 + 1]; 
        }
    }

    while (min >= max)
    {
        // empty extent so cannot split
        if (min > max)
        {
            return 1;
        }
        --splitAxis;
        if (splitAxis < 0)
        { // cannot split
            vtkDebugMacro("  Cannot Split");
            return 1;
        }
        min = startExt[splitAxis*2];
        max = startExt[splitAxis*2+1];
    }

    // determine the actual number of pieces that will be generated
    int range = max - min + 1;
    int valuesPerThread = static_cast<int>(ceil(range/static_cast<double>(total)));
    int maxThreadIdUsed = static_cast<int>(ceil(range/static_cast<double>(valuesPerThread))) - 1;
    if (num < maxThreadIdUsed)
    {
        splitExt[splitAxis*2] = splitExt[splitAxis*2] + num*valuesPerThread;
        splitExt[splitAxis*2+1] = splitExt[splitAxis*2] + valuesPerThread - 1;
    }
    if (num == maxThreadIdUsed)
    {
        splitExt[splitAxis*2] = splitExt[splitAxis*2] + num*valuesPerThread;
    }

    vtkDebugMacro("  Split Piece: ( " <<splitExt[0]<< ", " <<splitExt[1]<< ", "
                << splitExt[2] << ", " << splitExt[3] << ", "
                << splitExt[4] << ", " << splitExt[5] << ")");

    return maxThreadIdUsed + 1;
}


/*!
 *  Wire input to output for in place filters
 */
svkImageData* svkThreadedImageAlgorithm::GetOutput()
{
    return this->GetImageDataInput(0); 
}


/*!
 *  Wire input to output for in place filters 
 */
svkImageData* svkThreadedImageAlgorithm::GetOutput(int port)
{
    return this->GetImageDataInput(port); 
}


/*!
 *
 */
svkImageData* svkThreadedImageAlgorithm::GetImageDataInput(int port)
{
    return svkImageData::SafeDownCast( this->GetInput(port) );
}


/*!
 *  Default output type is same concrete sub class type as the input data.  Override with
 *  specific concrete type in sub-class if necessary.
 */
int svkThreadedImageAlgorithm::FillOutputPortInformation( int vtkNotUsed(port), vtkInformation* info )
{
    info->Set( vtkDataObject::DATA_TYPE_NAME(), this->GetImageDataInput(0)->GetClassName() );
    return 1;
}


/*!
 *  Default input type is svkImageData base class. Override with a specific concrete type in
 *  sub class if necessary.
 */
int svkThreadedImageAlgorithm::FillInputPortInformation( int vtkNotUsed(port), vtkInformation* info )
{
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "svkImageData");
    return 1;
}

