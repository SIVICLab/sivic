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
 *      
 *     
 */


#ifndef SVK_EPSI_REORDER_H
#define SVK_EPSI_REORDER_H

#include </usr/include/vtk/vtkObject.h>
#include </usr/include/vtk/vtkObjectFactory.h>
#include <svkTypes.h>

#include <svkMrsImageFFT.h>
#include <svkImageInPlaceFilter.h>


namespace svk {


using namespace std;


/*! 
 *  This class reorders the EPSI dimension of an EPSI MRS acquisition.
 */
class svkEPSIReorder : public svkImageInPlaceFilter
{

    public:

        static svkEPSIReorder* New();
        vtkTypeMacro( svkEPSIReorder, svkImageInPlaceFilter);

       typedef enum {
            UNDEFINED_EPSI_AXIS = -1,
            COLS,
            ROWS, 
            SLICES 
        } EPSIAxis;

        void                        SetEPSIType( EPSIType epsiType );
        void                        SetNumEPSILobes( int numLobes );  
        void                        SetNumSamplesPerLobe( int numSamples ); 
        int                         GetNumSamplesPerLobe(); 
        void                        SetFirstSample( int firstSample );
        void                        SetNumSamplesToSkip( int numSamplesToSkip );
        void                        SetEPSIAxis( svkEPSIReorder::EPSIAxis epsiAxis ); 
        svkEPSIReorder::EPSIAxis    GetEPSIAxis(); 
        virtual int                 GetNumEPSIAcquisitions();
        virtual int                 GetNumEPSIAcquisitionsPerFID();
        virtual int                 GetNumEPSIFrequencyPoints();
        static void                 CombineLobes(svkImageData* data, bool sumOfSquares = true ); 



    protected:

        svkEPSIReorder();
        ~svkEPSIReorder();

        virtual int     FillInputPortInformation(int port, vtkInformation* info);

        //  Methods:
        virtual int     RequestData(
                            vtkInformation* request, 
                            vtkInformationVector** inputVector,
                            vtkInformationVector* outputVector
                        );


    private: 

        EPSIType        epsiType; 
        int             numLobes;
        int             numSamplesPerLobe; 
        int             firstSample; 
        int             numSamplesToSkip; 
        EPSIAxis        epsiAxis;

        void            ReorderEPSIData( svkImageData* data ); 
        void            UpdateReorderedParams( svkImageData* data, int numVoxels[3] ); 

        int             numVoxelsReordered[3];


};


}   //svk


#endif //SVK_EPSI_REORDER_H

