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


#ifndef SVK_OBLIQUE_RESLICE_H
#define SVK_OBLIQUE_RESLICE_H


#include <vtkObject.h>
#include <vtkObjectFactory.h>
#include <vtkImageReslice.h>

#include <svkImageAlgorithm.h>
#include <svkImageData.h>
#include <svkMriImageData.h>


namespace svk {


using namespace std;


/*! 
 *  Class to reslice an svkMriImageData at the specified orientation, set either explicitly by 
 *  providing the target orientatin dcos, or by providing an svkImageData object. 
 */
class svkObliqueReslice : public svkImageAlgorithm
{

    public:

        static svkObliqueReslice* New();
        vtkTypeRevisionMacro( svkObliqueReslice, svkImageAlgorithm );


        //  Set the target orientation from a target image or the dcos explicitily:
        void                    SetTargetDcosFromImage(svkImageData* image);
        void                    SetTargetDcos(double dcos[3][3]);   //fillinputportinfo


    protected:

        svkObliqueReslice();
        ~svkObliqueReslice();

        virtual int         FillInputPortInformation( int vtkNotUsed(port), vtkInformation* info );
        virtual int         RequestData(
                                vtkInformation* request,
                                vtkInformationVector** inputVector,
                                vtkInformationVector* outputVector
                            );
        virtual int         RequestInformation( 
                                vtkInformation* request, 
                                vtkInformationVector** inputVector, 
                                vtkInformationVector* outputVector 
                            ); 



    private:

        //  Methods:
        virtual void        UpdateProvenance();
        virtual void        UpdateHeader();
        void                SetReslicedHeaderSpacing();
        void                SetReslicedHeaderPerFrameFunctionalGroups();
        void                SetReslicedHeaderOrientation();
        void                SetRotationMatrix( );
        void                RotateAxes(double axesIn[3][3], double rotatedAxes[3][3]);
        void                Print3x3(double matrix[3][3], string name);

        //  Members:
        vtkImageReslice*    reslicer;
        svkImageData*       reslicedImage; 
        double              targetDcos[3][3]; 
        double              rotation[3][3]; 
        double              newSpacing[3]; 
        int                 newNumVoxels[3]; 

};


}   //svk


#endif //SVK_OBLIQUE_RESLICE_H

