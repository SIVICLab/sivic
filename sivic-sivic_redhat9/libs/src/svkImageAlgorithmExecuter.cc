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



#include <svkImageAlgorithmExecuter.h>


using namespace svk;


//vtkCxxRevisionMacro(svkImageAlgorithmExecuter, "$Rev$");
vtkStandardNewMacro(svkImageAlgorithmExecuter);


/*!
 *
 */
svkImageAlgorithmExecuter::svkImageAlgorithmExecuter()
{
#if VTK_DEBUG_ON
    this->DebugOn();
#endif

    vtkDebugMacro(<< this->GetClassName() << "::" << this->GetClassName() << "()");
    this->algo = NULL;

}


/*!
 *
 */
svkImageAlgorithmExecuter::~svkImageAlgorithmExecuter()
{
    vtkDebugMacro(<<this->GetClassName()<<"::~svkImageAlgorithmExecuter()");
    if( this->algo != NULL ) {
        this->algo->Delete();
        this->algo = NULL;
    }
}


/*!
 *  RequestData pass the input through the algorithm, and copies the dcos and header
 *  to the output. 
 */
int svkImageAlgorithmExecuter::RequestData( vtkInformation* request, 
                                            vtkInformationVector** inputVector, 
                                            vtkInformationVector* outputVector )
{

    // Do nothing if algo is null
    if( this->algo == NULL ) {
        return 0;
    }
    
    // Set the input of the vtk algorithm to be the input of the executer
    this->algo->SetInputData(this->GetImageDataInput(0));

    // Pass the header through NOTE: We may want to generate a new header here.....
    
    this->GetImageDataInput(0)->GetDcmHeader()->MakeDerivedDcmHeader( this->GetOutput()->GetDcmHeader()
                        , this->GetImageDataInput(0)->GetDcmHeader()->GetStringValue("SeriesDescription") );

    // Update the vtk algorithm
    this->algo->Update();

    // Copy the output of the vtk algorithm
    this->GetOutput()->DeepCopy( this->algo->GetOutput() );
    this->GetOutput()->CopyDcos( this->GetImageDataInput(0) );
    return 1; 
}


/*!
 *  Setter for the vtkImageAlgorithm you want to execute using an svk object.
 */
void svkImageAlgorithmExecuter::SetAlgorithm( vtkImageAlgorithm* algo ) 
{
    this->algo = algo;
    if( this->algo != NULL ) {
        this->algo->Register( this ); 
    }
}
