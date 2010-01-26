/*
 *  Copyright © 2009-2010 The Regents of the University of California.
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


#include <svkImageClip.h>


using namespace svk;


vtkCxxRevisionMacro(svkImageClip, "$Rev$");
vtkStandardNewMacro(svkImageClip);


/*!
 *
 */
svkImageClip::svkImageClip()
{
#if VTK_DEBUG_ON
    this->DebugOn();
#endif

    vtkDebugMacro( << this->GetClassName() << "::" << this->GetClassName() << "()");
}


/*!
 *
 */
svkImageClip::~svkImageClip()
{
    vtkDebugMacro( << this->GetClassName() << "::~" << this->GetClassName() << "()");
}


/*!
 *
 */
int svkImageClip::RequestData( vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector )
{

    int* extent = this->GetImageDataInput(0)->GetExtent();
    int newE[6];
    cout << extent [0] << endl;
    cout << extent [1] << endl;
    cout << extent [2] << endl;
    cout << extent [3] << endl;
    cout << extent [4] << endl;
    cout << extent [5] << endl;
    newE[0] = extent[0] + 50; 
    newE[1] = extent[1] - 50; 
    newE[2] = extent[2] + 50; 
    newE[3] = extent[3] - 50; 
    newE[4] = extent[4];
    newE[5] = extent[5];
    cout << extent [0] << endl;
    cout << extent [1] << endl;
    cout << extent [2] << endl;
    cout << extent [3] << endl;
    cout << extent [4] << endl;
    cout << extent [5] << endl;

    // Need to crop to actually see minimum intensity
    vtkImageClip *clip = vtkImageClip::New();
    cout << "clip input image" << this->GetImageDataInput(0) << endl;
    cout << "clip input image" << *( this->GetImageDataInput(0) ) << endl;
    clip->SetInput( this->GetImageDataInput(0) );
    clip->SetOutputWholeExtent(newE[0], newE[1], newE[2], newE[3], newE[4], newE[5]);
    clip->ClipDataOn();
    clip->Update();
    cout << "clip otuput image" << clip->GetOutput() << endl;

    //  Now the header should be updateds since this changes the data 
    //  set dimensionality/size. 
    this->UpdateHeader();

    //Two options: 
    //  1. copy the output of the new algo to the existing svkImageData piece
    //  2. have the svkImageData point at the new output (SetPiece) and 
    //      reinitialize the view. This would require sending a separate signal  
    //      such as NewDataEvent, rather than ModifiedEvent.
    //  Consider how the copy might interefere with a pipeline with inputs and outputs
    //  connected if there is a copy in between.    
    //  Also consider a history buffer in svkImageData to support "undo" operations on 
    //  non in-place filters.    
    this->GetImageDataInput(0)->ShallowCopy( clip->GetOutput() );

    //  Trigger observer update via modified event:
    this->Modified();
    this->GetImageDataInput(0)->InvokeEvent(vtkCommand::ModifiedEvent);

    clip->Delete();

    return 1; 
}


/*!
 *
 */
void svkImageClip::UpdateProvenance()
{
    vtkDebugMacro(<<this->GetClassName()<<"::UpdateProvenance()");
}


/*!
 *
 */
void svkImageClip::UpdateHeader()
{
    vtkDebugMacro(<<this->GetClassName()<<"::UpdateHeader()");
}

