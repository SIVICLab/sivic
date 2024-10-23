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


#ifndef SVK_DSC_DELTA_R2_H
#define SVK_DSC_DELTA_R2_H


#include </usr/include/vtk/vtkObject.h>
#include </usr/include/vtk/vtkObjectFactory.h>
#include </usr/include/vtk/vtkInformation.h>

#include <svkImageData.h>
#include <svkMriImageData.h>
#include <svkImageInPlaceFilter.h>
#include <svkDcmHeader.h>


namespace svk {


using namespace std;


/*! 
 *  Class to convert from DSC T2* curves to Delta R2* representation. This is an in-place
 *  method that permits toggling between representations.
 *  
 *  Thanks to Emma Essock-Burns and Janine Lupo PhD, UCSF Department of Radiology and 
 *  Biomedical Imaging for help implementing this class.
 * 
 *  References: 
 *  1.  "Dynamic Susceptibility-Weighted Perfusion Imaging of High-Grade Gliomas: Characterization 
 *      of Spatial Heterogeneity", Janine M. Lupo, Soonmee Chaa, Susan M. Chang and Sarah J. Nelson, 
 *      AJNR 26: 1446-1454 (2005).    

 *  2.  "Differentiation of Glioblastoma Multiforme and Single Brain Metastasis by Peak Height and 
 *      Percentage of Signal Intensity Recovery Derived from Dynamic Susceptibility-Weighted 
 *      Contrast-Enhanced Perfusion MR Imaging", 
 *      S. Chaa, J.M. Lupo, M.-H. Chen, K.R. Lamborn, M.W. McDermott, M.S. Berger, S.J. Nelson and
 *      W.P. Dillon,  AJNR 28: 1078-1084 (2007). 
 *
 */
class svkDSCDeltaR2: public svkImageInPlaceFilter
{

    public:

        static svkDSCDeltaR2* New();
        vtkTypeMacro( svkDSCDeltaR2, svkImageInPlaceFilter);

        typedef enum {
            T2 = 0, 
            DR2 = 1 
        }representation;

        void                    SetRepresentation(svkDSCDeltaR2::representation representation);


    protected:

        svkDSCDeltaR2();
        ~svkDSCDeltaR2();

        virtual int             RequestData( 
                                    vtkInformation* request, 
                                    vtkInformationVector** inputVector, 
                                    vtkInformationVector* outputVector 
                                );

        virtual int             FillInputPortInformation( int vtkNotUsed(port), vtkInformation* info );


    private:


        //  Members:
        svkDSCDeltaR2::representation currentRepresentation;  
        svkDSCDeltaR2::representation targetRepresentation;  


};


}   //svk


#endif //SVK_DSC_DELTA_R2_H

