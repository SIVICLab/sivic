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


#ifndef SVK_METABOLITE_MAP_H
#define SVK_METABOLITE_MAP_H


#include </usr/include/vtk/vtkObject.h>
#include </usr/include/vtk/vtkObjectFactory.h>
#include </usr/include/vtk/vtkInformation.h>

#include <svkImageData.h>
#include <svkMriImageData.h>
#include <svkMrsImageData.h>
#include <svkImageAlgorithm.h>
#include <svkDcmHeader.h>
#include <svkEnhancedMRIIOD.h>


namespace svk {


using namespace std;


/*! 
 *  Class to extract an svkMriImageData object out of an svkMrsImageData object.  This is 
 *  useful for generating metaboite maps or other maps from spectral, dynamic or other multi-
 *  volumetric data. 
 */
class svkMetaboliteMap: public svkImageAlgorithm
{

    public:

        static svkMetaboliteMap* New();
        vtkTypeMacro( svkMetaboliteMap, svkImageAlgorithm);

        typedef enum {
            INTEGRATE = 0, 
            PEAK_HT = 1, 
            MAG_PEAK_HT = 2, 
            LINE_WIDTH = 3,  
            MAG_LINE_WIDTH = 4,
            MAG_INTEGRATE = 5
        }algorithm;


        void                    SetSeriesDescription(std::string newSeriesDescription);
        void                    SetOutputDataType(svkDcmHeader::DcmPixelDataFormat dataType);
        void                    SetZeroCopy(bool zeroCopy); 
        void                    SetPeakPosPPM( float peak_center_ppm );     
        void                    SetPeakWidthPPM( float peak_width_ppm  );     
        void                    SetAlgorithmToIntegrate();     
        void                    SetAlgorithmToPeakHeight();     
        void                    SetAlgorithmToMagPeakHeight();     
        void                    SetAlgorithmToLineWidth();
        void                    SetAlgorithmToMagLineWidth();
        void                    SetAlgorithmToMagIntegrate();
        void                    SetAlgorithm( std::string algo );     
        void                    SetVerbose( bool isVerbose );     
        void                    LimitToSelectedVolume(float fraction = 0.5001);
        short*                  GetSelectedVolumeMask();


        static double           GetIntegral( float* specPtr, int startPt, int endPt);
        static double           GetMagIntegral( float* specPtr, int startPt, int endPt);
        static double           GetPeakHt( float* specPtr, int startPt, int endPt );
        static double           GetMagPeakHt( float* specPtr, int startPt, int endPt );


    protected:

        svkMetaboliteMap();
        ~svkMetaboliteMap();

        virtual int             RequestInformation(
                                    vtkInformation* request,
                                    vtkInformationVector** inputVector,
                                    vtkInformationVector* outputVector
                                );
        virtual int             RequestData( 
                                    vtkInformation* request, 
                                    vtkInformationVector** inputVector, 
                                    vtkInformationVector* outputVector 
                                );
        void                    ZeroData(); 

        virtual int             FillInputPortInformation( int vtkNotUsed(port), vtkInformation* info );
        virtual int             FillOutputPortInformation( int vtkNotUsed(port), vtkInformation* info ); 


    private:

        //  Methods:
        virtual void            UpdateProvenance();
        void                    RedimensionData(); 
        void                    GetIntegrationPtRange(int& startPt, int& endPt); 
        void                    GenerateMap(); 
        float                   GetWidthInHz( int startPt, int endPt); 
        double                  GetMagLineWidth( float* specPtr, int startPt, int endPt );
        double                  GetLineWidth( float* specPtr, int startPt, int endPt );
        double                  GetMapVoxelValue( float* specPtr, int startPt, int endPt); 


        //  Members:
        std::string              newSeriesDescription; 
        float                       peakCenterPPM;
        float                       peakWidthPPM;
        svkMetaboliteMap::algorithm quantificationAlgorithm;  
        bool                        isVerbose; 
        float                       useSelectedVolumeFraction;
        short*                      quantificationMask;


};


}   //svk


#endif //SVK_METABOLITE_MAP_H

