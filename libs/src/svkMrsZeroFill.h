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


#ifndef SVK_MRS_ZERO_FILL_H
#define SVK_MRS_ZERO_FILL_H


#include </usr/include/vtk/vtkObject.h>
#include </usr/include/vtk/vtkObjectFactory.h>
#include </usr/include/vtk/vtkInformation.h>
#include </usr/include/vtk/vtkInformationVector.h>
#include </usr/include/vtk/vtkStreamingDemandDrivenPipeline.h>

#include <svkUtils.h>
#include <svkMriZeroFill.h>
#include <svkImageInPlaceFilter.h>
#include <svkMrsLinearPhase.h>


namespace svk {


using namespace std;



/*! 
 *  Class to apply zero and (eventually) first order phase to spectra
 */
class svkMrsZeroFill : public svkImageInPlaceFilter
{

    public:

        static svkMrsZeroFill* New();
        vtkTypeMacro( svkMrsZeroFill, svkImageInPlaceFilter);

        typedef enum {
            VALUE = 0,
            DOUBLE,
            POWER2
        } FillType;



        void            SetNumberOfRows( int numRows );
        void            SetNumberOfRowsToDouble( );
        void            SetNumberOfRowsToNextPower2( );

        void            SetNumberOfColumns( int numColumns );
        void            SetNumberOfColumnsToDouble( );
        void            SetNumberOfColumnsToNextPower2( );

        void            SetNumberOfSlices( int numSlices );
        void            SetNumberOfSlicesToDouble( );
        void            SetNumberOfSlicesToNextPower2( );

        void            SetNumberOfSpecPoints( int numSpecPoints );
        void            SetNumberOfSpecPointsToDouble( );
        void            SetNumberOfSpecPointsToNextPower2( );
        
        void            SetOutputDimensions( int numRows, int numColumns, int numSlices, int numSpecPoints );

        void            SetOutputWholeExtent( int extent[6] );
        void            SetOutputWholeExtent(int minX, int maxX, int minY, int maxY, int minZ, int maxZ);



    protected:

        svkMrsZeroFill();
        ~svkMrsZeroFill();

        virtual int     FillInputPortInformation(int port, vtkInformation* info);


        //  Methods:
        virtual int     RequestInformation(
                            vtkInformation* request, 
                            vtkInformationVector** inputVector,
                            vtkInformationVector* outputVector
                        );

        virtual int     RequestData(
                            vtkInformation* request, 
                            vtkInformationVector** inputVector,
                            vtkInformationVector* outputVector
                        );

        virtual int     RequestDataSpatial(
                            vtkInformation* request, 
                            vtkInformationVector** inputVector,
                            vtkInformationVector* outputVector
                        );

        virtual int     RequestDataSpectral(
                            vtkInformation* request, 
                            vtkInformationVector** inputVector,
                            vtkInformationVector* outputVector
                        );

        virtual int     RequestUpdateExtent(vtkInformation*,
                             vtkInformationVector**,
                             vtkInformationVector*);

        virtual void    ComputeInputUpdateExtent (int inExt[6], int outExt[6], int wExt[6]);

        virtual void    InitializeOutputWholeExtent();

    private:
        
        //  Members:

        // These will store the fill type. This is used in case the user defines the fill
        // type before setting the input. The extent is computed on RequestInformation.
        FillType        rowFillType;
        FillType        columnFillType;
        FillType        sliceFillType;
        FillType        specFillType;

        int             numSpecPoints; 
        //ZeroFillDomain  domain;
        int             outputWholeExtent[6];

        virtual void    InitializeDataArrays( svkMrsImageData* outputData );


};


}   //svk


#endif //SVK_MRS_ZERO_FILL_H

