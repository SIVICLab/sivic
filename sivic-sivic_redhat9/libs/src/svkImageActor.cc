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


#include <svkImageActor.h>


using namespace svk;


//vtkCxxRevisionMacro(svkImageActor, "$Rev$");
vtkStandardNewMacro(svkImageActor);


//! Constructor
svkImageActor::svkImageActor()
{

#if VTK_DEBUG_ON
    this->DebugOn();
#endif
    this->IsIdentity = false;
}


//! Destructor
svkImageActor::~svkImageActor()
{
}

void svkImageActor::ComputeMatrix()
{
    if (this->IsIdentity) {
        return;
    }

    // check whether or not need to rebuild the matrix
    if ( this->GetMTime() > this->MatrixMTime || (this->GetInput() && this->GetInput()->GetMTime() > this->MatrixMTime) ) {
        this->GetOrientation();
        this->Transform->Push();
        this->Transform->Identity();
        this->Transform->PostMultiply();

        // shift back to actor's origin
        this->Transform->Translate(-this->Origin[0],
                                  -this->Origin[1],
                                  -this->Origin[2]);

        // scale
        this->Transform->Scale(this->Scale[0],
                              this->Scale[1],
                              this->Scale[2]);

        // rotate
        this->Transform->RotateY(this->Orientation[1]);
        this->Transform->RotateX(this->Orientation[0]);
        this->Transform->RotateZ(this->Orientation[2]);

        // move back from origin and translate
        this->Transform->Translate(this->Origin[0] + this->Position[0],
                                  this->Origin[1] + this->Position[1],
                                  this->Origin[2] + this->Position[2]);

        if ( this->GetInput() ) {
            double dcos[3][3];
            svkImageData::SafeDownCast(this->GetInput())->GetDcos(dcos);

            double* origin = this->GetInput()->GetOrigin();

            // Translations occur around 0,0,0 so to get our matrix we untranslate, apply dcos, and retranslate
            vtkTransform* untranslate = vtkTransform::New();
            untranslate->Identity();
            untranslate->Translate(-origin[0], -origin[1], -origin[2] );

            vtkTransform* rotationTransform = vtkTransform::New();
            rotationTransform->Identity();
            vtkMatrix4x4* rotationMatrix = vtkMatrix4x4::New();
            for( int i = 0; i < 3; i++) {
                for( int j = 0; j < 3; j++) {
                    rotationMatrix->SetElement(i,j,dcos[j][i]);
                }
            }
            rotationTransform->SetMatrix(rotationMatrix);
            rotationMatrix->Delete();

            vtkTransform* retranslate = vtkTransform::New();
            retranslate->Identity();
            retranslate->Translate(origin[0], origin[1], origin[2] );

            this->Transform->PostMultiply();
            this->Transform->Concatenate( untranslate );
            this->Transform->Concatenate( rotationTransform );
            this->Transform->Concatenate( retranslate );


            untranslate->Delete();
            rotationTransform->Delete();
            retranslate->Delete();

        }

        // apply user defined transform last if there is one
        if (this->UserTransform) {
            this->Transform->Concatenate(this->UserTransform->GetMatrix());
        }

        this->Transform->PreMultiply();
        this->Transform->GetMatrix(this->Matrix);
        this->MatrixMTime.Modified();
        this->Transform->Pop();
    }

}
