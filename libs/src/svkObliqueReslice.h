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


#ifndef SVK_OBLIQUE_RESLICE_H
#define SVK_OBLIQUE_RESLICE_H


#include </usr/include/vtk/vtkObject.h>
#include </usr/include/vtk/vtkObjectFactory.h>
#include </usr/include/vtk/vtkImageReslice.h>
#include </usr/include/vtk/vtkMatrix4x4.h>
#include </usr/include/vtk/vtkTransform.h>

#include <svkImageAlgorithm.h>
#include <svkImageAlgorithmWithPortMapper.h>
#include <svkImageData.h>
#include <svkMriImageData.h>


namespace svk {


using namespace std;


/*! 
 *  Class to reslice an svkMriImageData at the specified orientation, set either explicitly by 
 *  providing the target orientatin dcos, or by providing an svkImageData object. 
 */
class svkObliqueReslice : public svkImageAlgorithmWithPortMapper
{

    public:

        enum {
            INPUT_IMAGE = 0,
            TARGET_IMAGE,
            MATCH_SPACING_AND_FOV,
            INTERPOLATION_MODE
        } svkObliqueResliceInput;

        enum {
            RESLICED_IMAGE = 0
        } svkObliqueResliceOutput;

        static svkObliqueReslice* New();
        vtkTypeMacro( svkObliqueReslice, svkImageAlgorithm );


        //  Set the target orientation from a target image or the dcos explicitily:
        void                    SetTarget(svkImageData* image);
        void                    SetTargetDcos(double dcos[3][3]);   //fillinputportinfo
        void                    SetMagnificationFactors( float x, float y, float z); 
        void                    SetMatchSpacingAndFovOn( );
        void                    SetInterpolationMode(int interpolationMode);
        svkInt*                 GetInterpolationMode();

    protected:

        svkObliqueReslice();
        ~svkObliqueReslice();

        // virtual int         FillInputPortInformation( int vtkNotUsed(port), vtkInformation* info );
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
        bool                GetMatchSpacingAndFovOn( );
        void                SetReslicedHeaderSpacing();
        void                SetReslicedHeaderPerFrameFunctionalGroups();
        void                SetReslicedHeaderOrientation();
        void                SetRotationMatrix( );
        void                Print3x3(double matrix[3][3], string name);
        bool                IsDcosInitialized(); 
        bool                Magnify();
        void                ComputeTopLeftCorner(double newTlc[3]);



        //  Members:
        vtkImageReslice*    reslicer;
        svkImageData*       reslicedImage; 
        double              targetDcos[3][3]; 
        double              newSpacing[3]; 
        int                 newNumVoxels[3]; 
        float               magnification[3]; 

};


}   //svk


#endif //SVK_OBLIQUE_RESLICE_H

