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


#ifndef SVK_PLOT_LINE_GRID_H_INCL
#define SVK_PLOT_LINE_GRID_H_INCL


#include <vector>
#include </usr/include/vtk/vtkRenderer.h>
#include </usr/include/vtk/vtkIdList.h>

#include </usr/include/vtk/vtkImageData.h>
#include </usr/include/vtk/vtkCellData.h>
#include </usr/include/vtk/vtkCamera.h>
#include </usr/include/vtk/vtkProp3DCollection.h>
#include </usr/include/vtk/vtkDataSetCollection.h>
#include </usr/include/vtk/vtkCallbackCommand.h>
#include </usr/include/vtk/vtkAppendPolyData.h>
#include </usr/include/vtk/vtkAreaPicker.h>
#include </usr/include/vtk/vtkCollectionIterator.h>
#include </usr/include/vtk/vtkTransform.h>
#include <svkPlotLine.h>
#include <svkImageData.h>
#include <svkDataView.h>
#include <svk4DImageData.h>
#include <svkMrsImageData.h>
#include </usr/include/vtk/vtkPolyDataCollection.h>
#include </usr/include/vtk/vtkPolyDataMapper.h>
#include </usr/include/vtk/vtkLODActor.h>
#include </usr/include/vtk/vtkProperty.h>


namespace svk {


using namespace std;

/*
 * Representation for 4D Image Data. It creates a vtkPolyData object to represent
 * the spatial location of the data acquired, and uses svkPlotLine objects
 * to modify point data for the individual cells. There are methods for controlling
 * the range, slice, etc.
 *
 */
class svkPlotLineGrid : public vtkObject
{

    public:
        // vtk type revision macro
        vtkTypeMacro( svkPlotLineGrid, vtkObject );

        static svkPlotLineGrid*       New();

        svkPlotLineGrid();
        ~svkPlotLineGrid();


        //  Methods
        void                    SetInput( svk4DImageData* data );
        svk4DImageData*         GetInput( );
        void                    Update(int tlcBrc[2]);
        void                    SetTlcBrc(int tlcBrc[2]);
        int*                    GetTlcBrc();
        void                    SetSlice(int slice);
        void                    SetPlotPoints();
        int                     GetSlice( );
        void                    SetFrequencyWLRange(int lower, int range, int tlcBrc[2]); 
        void                    GetFrequencyWLRange(int &lower, int &range); 
        void                    SetIntensityWLRange(double lower, double range, int tlcBrc[2]); 
        void                    GetIntensityWLRange(double &lower, double &range); 
        void                    AlignCamera( bool invertView = 1 );
        void                    SetComponent( svkPlotLine::PlotComponent component );
        svkPlotLine::PlotComponent GetComponent( );
        std::vector<int>     GetVolumeIndexVector( );
        void                    SetVolumeIndexVector( std::vector<int> );
        void                    SetVolumeIndex( int index, int volumeIndex = 0 );
        int                     GetVolumeIndex( int volumeIndex = 0 );
        void                    UpdateDataArrays(int tlc, int brc);
        void                    SetOrientation( svkDcmHeader::Orientation orientation );
        vtkActor*               GetPlotGridActor();
        void                    CalculateTlcBrcBounds( double bounds[6], int tlcBrc[2]);
        void                    SetColor( double rgb[3]);
        double*                 GetColor( );

    private:

        void                    ClearXYPlots();

        //! The current slice
        int                         slice;
        std::vector< int >      volumeIndexVector;

        //! The ID's of the top left corner, and bottom right corner cells 
        int                         tlcBrc[2];

        //! dcos to be passed as a pointer
        std::vector< std::vector<double> > dcos;

        //! Represents the current slice of plot lines
        std::vector<svkPlotLine*>  xyPlots;

        //! The actor that holds the plot grid
        vtkActor*                plotGridActor;

        vtkPolyData*                polyData;

        vtkPoints*                  points;

        vtkPolyDataMapper*          mapper;

        //! The index of the first point in the plots to be plotted-
        int                         plotRangeX1;     

        //! The index of the last point in the plots to be plotted-
        int                         plotRangeX2;     

        //! The minimum value to be plotted
        double                      plotRangeY1;     

        //! The maximum value to be plotted
        double                      plotRangeY2;     

        //! Array of booleans keeps track of frequency changes
        bool*                       freqUpToDate;

        //! keeps track of whether the frequency is up to date
        int                         freqSelectionUpToDate[2];

        //! Array of booleans keeps track of frequency changes
        bool*                       ampUpToDate;

        int                         ampSelectionUpToDate[2];

        // Defines if the given volume for a given slice is up to date
        std::vector< std::vector<bool> >  volumeUpToDate;

        //! The data
        svk4DImageData*               data;

        //! A callback to update the plot lines when the data changes
        vtkCallbackCommand*         dataModifiedCB;

        //! Component to display (e.g. REAL, IMAG..)
        svkPlotLine::PlotComponent   plotComponent;

        svkDcmHeader::Orientation orientation;

        //  Methods:
        void                        RegeneratePlots();
        static void                 UpdateData(vtkObject* subject, unsigned long eid, void* thisObject, void *calldata);
        void                        AllocateXYPlots();        
        void                        AllocatePolyData();
        void                        UpdatePlotRange(int tlcBrc[2], bool generatePolyData = true);
        void                        UpdateComponent();
        void                        UpdateOrientation();
        void                        GenerateActor();
        void                        HighlightSelectionVoxels();
        bool                        IsSliceUpToDate( int slice );
        void                        SetSliceUpToDate( int slice );
        void                        SetSliceOutOfDate( int slice );
        void                        SetVolumeUpToDate( int volumeIndex );
        void                        SetVolumeOutOfDate( int volumeIndex );
        void                        InitializeVolumeUpToDateVector();
        void						TurnOffAllPlots();

};


}   //svk


#endif //SVK_PLOT_LINE_GRID_H_INCL
