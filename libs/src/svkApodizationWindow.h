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


#ifndef SVK_APODIZATION_WINDOW_H
#define SVK_APODIZATION_WINDOW_H


#include </usr/include/vtk/vtkObjectFactory.h>
#include </usr/include/vtk/vtkObject.h>
#include </usr/include/vtk/vtkFloatArray.h>
#include </usr/include/vtk/vtkMath.h>
#include <svkImageData.h>

namespace svk {


using namespace std;

/*! 
 *  Apodization window.
 */
class svkApodizationWindow : public vtkObject
{

    public:

        typedef enum {
            UNDEFINED = 0,
            LORENTZIAN,
            GAUSSIAN, 
            HAMMING, 
            LAST 
        }WindowType;

        typedef enum {
            COL = 0,
            ROW, 
            SLICE, 
            THREE_D
        }Dimension;

        // vtk type revision macro
        vtkTypeMacro( svkApodizationWindow, vtkObject );
  
        // vtk initialization 
        static svkApodizationWindow* New();  

        static void  GetLorentzianWindow( vector < vtkFloatArray* >* window, svkImageData* data, float fwhh );
        static void  GetGaussianWindow(   vector < vtkFloatArray* >* window, svkImageData* data, float fwhh, float center = 0 );
        static void  GetHammingWindow(    vector < vtkFloatArray* >* window, 
                                          svkImageData* data, 
                                          svkApodizationWindow::Dimension dimension = svkApodizationWindow::THREE_D);

	protected:

       svkApodizationWindow();
       ~svkApodizationWindow();


    private:
        static void  GetLorentzianWindow( vector < vtkFloatArray* >* window, float fwhh, float dt );
        static void  GetGaussianWindow(   vector < vtkFloatArray* >* window, float fwhh, float dt, float center = 0 );
        static void  GetHammingWindowData(    vector < vtkFloatArray* >* window, svkImageData* data, svkApodizationWindow::Dimension dimension );
        static void  InitializeWindowSpectral( vector < vtkFloatArray* >*  window, svkImageData* data );
        static void  InitializeWindowSpatial(  vector < vtkFloatArray* >*  window, svkImageData* data );
        static float GetWindowResolution( svkImageData* data );
        static float GetWindowExpansion( svkImageData* data, int numVoxels );
        
};


}   //svk


#endif //SVK_APODIZATION_WINDOW_H
