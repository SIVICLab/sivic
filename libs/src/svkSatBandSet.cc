/*
 *  Copyright © 2009-2010 The Regents of the University of California.
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


#define DEBUG 0


#include <svkSatBandSet.h>
#define CLIP_TOLERANCE 0
#define PROJECTION_MULTIPLIER 4
using namespace svk;


vtkCxxRevisionMacro(svkSatBandSet, "$Rev$");
vtkStandardNewMacro(svkSatBandSet);


//! Constructor
svkSatBandSet::svkSatBandSet()
{
    this->spectra = NULL;
    this->slice = 5;
    this->orientation = svkDcmHeader::AXIAL;
    // Read in from header
    this->satBandOrigins = vtkPoints::New();
    this->satBandNormals = vtkFloatArray::New();

    // Generated 
    this->satBandSurfaceOrigins = vtkPoints::New();
    this->satBandSurfaceNormals = vtkFloatArray::New();
    this->clippingPlanes.push_back( vtkPlane::New() );
    this->clippingPlanes.push_back( vtkPlane::New() );
    this->clippingPlanes.push_back( vtkPlane::New() );
    this->clippingPlanes.push_back( vtkPlane::New() );
    this->clippingPlanes.push_back( vtkPlane::New() );
    this->clippingPlanes.push_back( vtkPlane::New() );
    this->satBandActor= vtkActor::New();
    this->satBandOutlineActor= vtkActor::New();
}


//! Destructor
svkSatBandSet::~svkSatBandSet()
{
    for( int i = 0; i < clippingPlanes.size(); i++ ) {
        clippingPlanes[i]->Delete();
    }
    clippingPlanes.clear();
    if( this->satBandActor!= NULL ) {
        this->satBandActor->Delete();
        this->satBandActor= NULL;
    } 
    if( this->satBandOutlineActor!= NULL ) {
        this->satBandOutlineActor->Delete();
        this->satBandOutlineActor= NULL;
    } 
    if( this->spectra != NULL ) {
        this->spectra->Delete();
        this->spectra = NULL;
    }
    if( this->satBandOrigins != NULL ) {
        this->satBandOrigins->Delete();
        this->satBandOrigins = NULL;
    }
    if( this->satBandNormals != NULL ) {
        this->satBandNormals->Delete();
        this->satBandNormals = NULL;
    }
    if( this->satBandSurfaceOrigins != NULL ) {
        this->satBandSurfaceOrigins->Delete();
        this->satBandSurfaceOrigins = NULL;
    }
    if( this->satBandSurfaceNormals != NULL ) {
        this->satBandSurfaceNormals->Delete();
        this->satBandSurfaceNormals = NULL;
    }

}


/*
 *  Sets the input dataset. Its header and its extent will
 *  be used in generating the sat bands, and clipping them
 *  to a reasonable range.
 *
 *  \param spectra the spectroscopy dataset that contains the desired satBands
 *
 */
void svkSatBandSet::SetInput( svkMrsImageData* spectra ) 
{
    if( this->spectra != NULL ) {
        this->spectra->Delete();
        this->spectra = NULL;
    }
    this->spectra = spectra;

    // if the input dataset is not null, initialize it...
    if( this->spectra != NULL ) {
        this->spectra->Register( this );

        // These member variables are to speed up slicing...
        this->spectraSpacing = this->spectra->GetSpacing();
        this->spectra->GetDcos( spectraDcos );
        this->spectraOrigin = this->spectra->GetOrigin();
        this->spectraExtent = this->spectra->GetExtent();
        double LRNormal[3];
        this->spectra->GetDataBasis(LRNormal, svkImageData::LR );
        double PANormal[3];
        this->spectra->GetDataBasis(PANormal, svkImageData::PA );
        double SINormal[3];
        this->spectra->GetDataBasis(SINormal, svkImageData::SI );
        this->spectraDeltaLR = vtkMath::Dot( this->spectraSpacing, LRNormal );
        this->spectraDeltaAP = vtkMath::Dot( this->spectraSpacing, PANormal );
        this->spectraDeltaSI = vtkMath::Dot( this->spectraSpacing, SINormal );


    
        // Now lets get our clipping planes, and generate the sat bands
        this->GenerateClippingPlanes();
        this->GenerateSatBandsActor();
    }
}


/*
 * Sets the slice used for clipping the sat bands.
 *
 * \param slice the slice value is used to clip the sat bands
 */
void svkSatBandSet::SetClipSlice( int slice )
{
    this->slice = slice;
    if( this->spectra != NULL ) {
        this->GenerateSliceClippingPlanes();
    }
}


/*
 *  Returns the set of actor representing the sat bands.
 *
 *  \return a vtkActorcontaining the sat bands as actors.
 */
vtkActor* svkSatBandSet::GetSatBandsActor( ) 
{
    return this->satBandActor;
}


/*
 *  Returns the set of actor representing the sat bands.
 *
 *  \return a vtkActorcontaining the sat bands as actors.
 */
vtkActor* svkSatBandSet::GetSatBandsOutlineActor( ) 
{
    return this->satBandOutlineActor;
}


/*
 *  Creates a visual representation of the Sat Bands. The way it does this is by first
 *  creating vtkPlaneSource objects with the parallel surfaces of the sat band slab.
 *  Next two points are (somewhat) arbitrarily chosen to generate the initial extent
 *  of the planes. These points should be well outside of the entire extent of the dataset.
 *  The points are then projected onto the surface of the planes, passed into the vtkPlaneSource
 *  object, and used to generate points on the corners of each slab. This points are put into
 *  a vtkHexaheron (vtkCell) and pushed into an vtkUnstructuredGrid. See vtkHexahedron documentation
 *  for a description of point ordering. Next the hexahedron are clipped using planes that parallel
 *  the edges of the spectroscopy dataset, and the current slice. This is actually data clipping
 *  NOT actor clipping.
 */
void svkSatBandSet::GenerateSatBandsActor( ) 
{
    svkDcmHeader* header= this->spectra->GetDcmHeader();
    double thickness;
    double origin[3];
    double normal[3];
    double point1[3];
    double point2[3];
    double planeOrigin[3];
    vtkPlaneSource* polyDataPlaneSource;
    double* bounds = this->spectra->GetBounds();
    double minPoint[3] = { bounds[0], bounds[2], bounds[4] };
    double maxPoint[3] = { bounds[1], bounds[3], bounds[5] };

    // We need to know how many sat bands there are.
    int numberOfSatBands = header->GetNumberOfItemsInSequence("MRSpatialSaturationSequence");

    // Read in from header
    this->satBandOrigins->SetNumberOfPoints( numberOfSatBands );
    this->satBandNormals->SetNumberOfComponents(3);
    this->satBandNormals->SetNumberOfTuples( numberOfSatBands );

    // Generated 
    this->satBandSurfaceOrigins->SetNumberOfPoints( numberOfSatBands*2 );
    this->satBandSurfaceNormals->SetNumberOfComponents(3);
    this->satBandSurfaceNormals->SetNumberOfTuples( numberOfSatBands*2 );

    // Loop throught all sat bands
    vtkPoints* corners = vtkPoints::New();
    vtkUnstructuredGrid* slabDataSet = vtkUnstructuredGrid::New();
    vtkPolyData* slabPolyData = vtkPolyData::New();
    slabDataSet->Allocate(numberOfSatBands, numberOfSatBands);
    slabPolyData->Allocate(numberOfSatBands*6, numberOfSatBands*6);
    corners->SetNumberOfPoints( 8 * numberOfSatBands );
    slabDataSet->SetPoints( corners );
    slabPolyData->SetPoints( corners );
    corners->Delete();
    for( int i = 0; i < numberOfSatBands; i++ ) {

        // First we acquire the thickness, origin and normals from the header...

        thickness = header->GetFloatSequenceItemElement("MRSpatialSaturationSequence", i, 
                                                        "SlabThickness" , "SharedFunctionalGroupsSequence");

        origin[0] = strtod(header->GetStringSequenceItemElement("MRSpatialSaturationSequence", i, "MidSlabPosition", 
                                                                0, "SharedFunctionalGroupsSequence").c_str(),NULL);
        origin[1] = strtod(header->GetStringSequenceItemElement("MRSpatialSaturationSequence", i, "MidSlabPosition", 
                                                                1, "SharedFunctionalGroupsSequence").c_str(),NULL);
        origin[2] = strtod(header->GetStringSequenceItemElement("MRSpatialSaturationSequence", i, "MidSlabPosition", 
                                                                2, "SharedFunctionalGroupsSequence").c_str(),NULL);

        // Lets save the origin in case we need the values later... 
        this->satBandOrigins->SetPoint(i, origin[0], origin[1], origin[2]);

        normal[0] = strtod(header->GetStringSequenceItemElement("MRSpatialSaturationSequence", i, "SlabOrientation", 
                                                                0, "SharedFunctionalGroupsSequence").c_str(),NULL);
        normal[1] = strtod(header->GetStringSequenceItemElement("MRSpatialSaturationSequence", i, "SlabOrientation", 
                                                                1, "SharedFunctionalGroupsSequence").c_str(),NULL);
        normal[2] = strtod(header->GetStringSequenceItemElement("MRSpatialSaturationSequence", i, "SlabOrientation", 
                                                                2, "SharedFunctionalGroupsSequence").c_str(),NULL);

        // Lets save the normals in case we need the values later... 
        this->satBandNormals->SetTuple3( i, normal[0], normal[1], normal[2] );
       
        // We now need to generate a hexahedron to represent the each sat band, these will be the corners 
        /*
         * Here is the tricky part. We want to create a vtkPlaneSource object to generate the four corners
         * of our plane. To do this we need to define the origin, normal, and two points. The two points 
         * represent the two corners of the plane. The chosen corners are really arbitrary and just define
         * how the plane extends from its origin. We are going to generate large planes, the cut them
         * down later. To get good points we are going to look at the primary component (highest value)
         * of the normal and choose two points ot the 4*bounds of the data set in the other two dimensions.
         * This should guarantee that the planes ore large enough and do not get cut off inside the dataset.
         */

        // length two, the two extreme planes of the sat band
        for( int j = 0; j < 2; j++ ) {
            if( j == 0 ) { 

                // planeOrigin is the origin of one plane on the edge of the satband
                planeOrigin[0] = satBandOrigins->GetPoint(i)[0] + satBandNormals->GetTuple(i)[0]*(thickness/2);
                planeOrigin[1] = satBandOrigins->GetPoint(i)[1] + satBandNormals->GetTuple(i)[1]*(thickness/2);
                planeOrigin[2] = satBandOrigins->GetPoint(i)[2] + satBandNormals->GetTuple(i)[2]*(thickness/2);

        
                // If the largest component is Z
                if(fabs(satBandNormals->GetTuple(i)[2]) >= fabs(satBandNormals->GetTuple(i)[0]) 
                   && fabs(satBandNormals->GetTuple(i)[2]) >= fabs(satBandNormals->GetTuple(i)[1])) {

                    minPoint[0] = PROJECTION_MULTIPLIER *( bounds[1]-bounds[0] );
                    minPoint[1] = 0;
                    minPoint[2] = planeOrigin[2];
                    maxPoint[0] = 0;
                    maxPoint[1] = PROJECTION_MULTIPLIER *(bounds[3]-bounds[2] );
                    maxPoint[2] = planeOrigin[2];

                // If the largest component is Y
                } else if( fabs(satBandNormals->GetTuple(i)[1]) >= fabs(satBandNormals->GetTuple(i)[0]) 
                           && fabs(satBandNormals->GetTuple(i)[1]) >= fabs(satBandNormals->GetTuple(i)[2])) {

                    minPoint[0] = PROJECTION_MULTIPLIER *(bounds[1]-bounds[0] );
                    minPoint[1] = planeOrigin[1];
                    minPoint[2] = 0;
                    maxPoint[0] = 0;
                    maxPoint[1] = planeOrigin[1];
                    maxPoint[2] = PROJECTION_MULTIPLIER *(bounds[5]-bounds[4] );

                // If the largest component is X
                } else if( fabs(satBandNormals->GetTuple(i)[0]) >= fabs(satBandNormals->GetTuple(i)[1]) 
                           && fabs(satBandNormals->GetTuple(i)[0]) >= fabs(satBandNormals->GetTuple(i)[2])) {

                    minPoint[0] = planeOrigin[0];
                    minPoint[1] = PROJECTION_MULTIPLIER *(bounds[1]-bounds[0] );
                    minPoint[2] = 0;
                    maxPoint[0] = planeOrigin[0];
                    maxPoint[1] = 0;
                    maxPoint[2] = PROJECTION_MULTIPLIER *(bounds[5]-bounds[4] );
                } 

                // We need our points projected down onto the plane...
                vtkPlane::GeneralizedProjectPoint( minPoint, planeOrigin, satBandNormals->GetTuple(i), point1); 
                vtkPlane::GeneralizedProjectPoint( maxPoint, planeOrigin, satBandNormals->GetTuple(i), point2); 
            } else {

                // Take the points chosen for the first edge. Guarantees a regular hexahedron
                planeOrigin[0] = satBandOrigins->GetPoint(i)[0] - satBandNormals->GetTuple(i)[0]*(thickness/2);
                planeOrigin[1] = satBandOrigins->GetPoint(i)[1] - satBandNormals->GetTuple(i)[1]*(thickness/2);
                planeOrigin[2] = satBandOrigins->GetPoint(i)[2] - satBandNormals->GetTuple(i)[2]*(thickness/2);
                vtkPlane::GeneralizedProjectPoint( point1, planeOrigin, satBandNormals->GetTuple(i), point1); 
                vtkPlane::GeneralizedProjectPoint( point2, planeOrigin, satBandNormals->GetTuple(i), point2); 
            }
      
            // This are being saved to use in the future... 
            this->satBandSurfaceOrigins->SetPoint( i, planeOrigin[0], planeOrigin[1], planeOrigin[2] ); 
            this->satBandSurfaceNormals->SetTuple3( i, satBandNormals->GetTuple(i)[0], 
                                                       satBandNormals->GetTuple(i)[1], 
                                                       satBandNormals->GetTuple(i)[2]); 

            // Now lets create our plane source
            polyDataPlaneSource = vtkPlaneSource::New(); 
       
            polyDataPlaneSource->SetCenter( planeOrigin[0], planeOrigin[1], planeOrigin[2] );
            polyDataPlaneSource->SetNormal( satBandNormals->GetTuple(i)[0], 
                                            satBandNormals->GetTuple(i)[1], 
                                            satBandNormals->GetTuple(i)[2] );

            polyDataPlaneSource->SetPoint1( point1[0], point1[1], point1[2] );
            polyDataPlaneSource->SetPoint2( point2[0], point2[1], point2[2] );

            // I have no idea why this second SetCenter is need, but without the planes are off...
            polyDataPlaneSource->SetCenter( planeOrigin[0], planeOrigin[1], planeOrigin[2] );
            vtkPolyData* planeData = polyDataPlaneSource->GetOutput();
            planeData->Update();

            // Add the points of the planes to the set of corners for the hexahedron
            corners->InsertPoint( i*8 + 4*j, planeData->GetPoint(0) );
            corners->InsertPoint( i*8 + 4*j + 1, planeData->GetPoint(1) );
            corners->InsertPoint( i*8 + 4*j + 2, planeData->GetPoint(2) );
            corners->InsertPoint( i*8 + 4*j + 3, planeData->GetPoint(3) );
            polyDataPlaneSource->Delete();
        }

        // Now we turn the planes data into a hexahedron
        vtkHexahedron* slab = vtkHexahedron::New();
        slab->GetPointIds()->SetNumberOfIds( 8 );

        // The ordering for the hexahdron is specific, see vtkHexahedron documentation
        slab->GetPointIds()->SetId(0, i*8 + 0);
        slab->GetPointIds()->SetId(1, i*8 + 4);
        slab->GetPointIds()->SetId(2, i*8 + 6);
        slab->GetPointIds()->SetId(3, i*8 + 2);
        slab->GetPointIds()->SetId(4, i*8 + 1);
        slab->GetPointIds()->SetId(5, i*8 + 5);
        slab->GetPointIds()->SetId(6, i*8 + 7);
        slab->GetPointIds()->SetId(7, i*8 + 3);

        // We need an object that can be mapped, so we but the hexahedron into on UnstructuredGrid
        slabDataSet->InsertNextCell(slab->GetCellType(), slab->GetPointIds());
        for( int k=0; k < slab->GetNumberOfFaces(); k++ ) {
            slabPolyData->InsertNextCell(slab->GetFace(k)->GetCellType(), slab->GetFace(k)->GetPointIds());
        }
        slab->Delete();
        
    }

    /*
        // This section is for adding colors to the sat bands

        vtkUnsignedCharArray* colors = vtkUnsignedCharArray::New();
        colors->SetNumberOfTuples(numberOfSatBands);
        colors->SetNumberOfComponents(4);
        srand( time(NULL) );
        double red;
        double green;
        double blue;
        for( int i=0; i < numberOfSatBands ; i++ ) {
            red =  255 - 255*(i/((double)(numberOfSatBands-1)));
            //double blue =  255*(rand()/((double)RAND_MAX));
            green =  255*(i/((double)(numberOfSatBands-1)));
            //double red =  255*(rand()/((double)RAND_MAX));
            if( i < numberOfSatBands/2.0 ) {
                blue =  255-255*(i/((double)(numberOfSatBands-1)/2.0));
            } else {
                blue =  255*((i-numberOfSatBands/2.0)/((double)(numberOfSatBands-1)/2.0));
            }
            //double green =  255*(rand()/((double)RAND_MAX));
            //double green =  0;
            //cout << "color is " << red << " " << green << " " << blue << endl;
            colors->InsertTuple4(i,red ,green,blue, 255 );
            //colors->InsertTuple4(i, 255,0,0, 255 );
        }
        colors->InsertTuple4(0,255,0,0, 255 );
        colors->InsertTuple4(1,255,255,0, 255 );
        colors->InsertTuple4(2,0,0,255, 255 );
        colors->InsertTuple4(3,0,255,0, 255 );
        colors->InsertTuple4(4,0,0,255,255 );
        colors->InsertTuple4(5,255,0,0, 255 );
        colors->InsertTuple4(6,0,0,255, 255 );
        colors->InsertTuple4(7,0,255,255, 255 );
        colors->InsertTuple4(8,255,0,255, 255 );
        colors->InsertTuple4(9,255,0,0, 255 );
        colors->InsertTuple4(10,255,255,0, 255 );
        colors->InsertTuple4(11,255,255,0, 255 );
        colors->InsertTuple4(12,0,255,0, 255 );
        colors->InsertTuple4(13,0,255,255, 255 );
        colors->InsertTuple4(14,0,255,0, 255 );
        colors->InsertTuple4(15,0,0,255, 255 );

        slabDataSet->GetCellData()->Initialize();
        slabDataSet->GetPointData()->Initialize();
        //slabDataSet->GetCellData()->SetScalars( colors );

     */

        /*
         * Now that we have our hexahedron lets clip it by the extent of the dataset, and the current slice.
         * The clipping planes for this have already been generated... 
         * I could not get the data to clip correctly using a single vtkClipDataSet, so 6 are changed, one for each slice              */ 

        // clippers clip the 3D dataset 
        vector<vtkClipDataSet*> clippers;
        clippers.push_back( vtkClipDataSet::New() );
        clippers.push_back( vtkClipDataSet::New() );
        clippers.push_back( vtkClipDataSet::New() );
        clippers.push_back( vtkClipDataSet::New() );
        clippers.push_back( vtkClipDataSet::New() );
        clippers.push_back( vtkClipDataSet::New() );

        clippers[0]->SetInput( slabDataSet );
        clippers[0]->SetClipFunction(clippingPlanes[0]);

        clippers[1]->SetInput( clippers[0]->GetOutput() );
        clippers[1]->SetClipFunction(clippingPlanes[1]);

        clippers[2]->SetInput( clippers[1]->GetOutput() );
        clippers[2]->SetClipFunction(clippingPlanes[2]);

        clippers[3]->SetInput( clippers[2]->GetOutput() );
        clippers[3]->SetClipFunction(clippingPlanes[3]);

        clippers[4]->SetInput( clippers[3]->GetOutput() );
        clippers[4]->SetClipFunction(clippingPlanes[4]);

        clippers[5]->SetInput( clippers[4]->GetOutput() );
        clippers[5]->SetClipFunction(clippingPlanes[5]);


        // cutters cut the faces of the 3D data set for highlighting edges
        vector<vtkClipPolyData*> cutters;
        cutters.push_back( vtkClipPolyData::New() );
        cutters.push_back( vtkClipPolyData::New() );
        cutters.push_back( vtkClipPolyData::New() );
        cutters.push_back( vtkClipPolyData::New() );
        cutters.push_back( vtkClipPolyData::New() );
        cutters.push_back( vtkClipPolyData::New() );

        cutters[0]->SetInput( slabPolyData );
        cutters[0]->SetClipFunction(clippingPlanes[0]);

        cutters[1]->SetInput( cutters[0]->GetOutput() );
        cutters[1]->SetClipFunction(clippingPlanes[1]);

        cutters[2]->SetInput( cutters[1]->GetOutput() );
        cutters[2]->SetClipFunction(clippingPlanes[2]);

        cutters[3]->SetInput( cutters[2]->GetOutput() );
        cutters[3]->SetClipFunction(clippingPlanes[3]);

        cutters[4]->SetInput( cutters[3]->GetOutput() );
        cutters[4]->SetClipFunction(clippingPlanes[4]);

        cutters[5]->SetInput( cutters[4]->GetOutput() );
        cutters[5]->SetClipFunction(clippingPlanes[5]);

        vtkDataSetMapper* satBandMapper = vtkDataSetMapper::New();
        vtkPolyDataMapper* satBandOutlineMapper = vtkPolyDataMapper::New();

        satBandMapper->SetInput(clippers[5]->GetOutput());
        satBandOutlineMapper->SetInput( cutters[5]->GetOutput() );
        clippers[0]->Delete();
        clippers[1]->Delete();
        clippers[2]->Delete();
        clippers[3]->Delete();
        clippers[4]->Delete();
        clippers[5]->Delete();
        clippers.clear();

        cutters[0]->Delete();
        cutters[1]->Delete();
        cutters[2]->Delete();
        cutters[3]->Delete();
        cutters[4]->Delete();
        cutters[5]->Delete();
        cutters.clear();
        // Now we will create the actual actor
        satBandMapper->ScalarVisibilityOff();
        satBandOutlineMapper->ScalarVisibilityOff();
        satBandActor->SetMapper(satBandMapper);
        satBandOutlineActor->SetMapper(satBandOutlineMapper);
        satBandMapper->Delete();
        satBandOutlineMapper->Delete();
        satBandActor->GetProperty()->SetAmbientColor( 0.9, 0.25, 0.0 );
        satBandActor->GetProperty()->SetOpacity(0.4);
        satBandActor->GetProperty()->SetAmbient(1.0);
        satBandActor->GetProperty()->SetLineWidth(3.0);
        satBandActor->GetProperty()->SetDiffuse(0.0);
        satBandActor->GetProperty()->SetSpecular(0.0);

        satBandOutlineActor->GetProperty()->SetAmbientColor( 0.9, 0.25, 0.0 );
        satBandOutlineActor->GetProperty()->SetAmbient(1.0);
        satBandOutlineActor->GetProperty()->SetDiffuse(0.0);
        satBandOutlineActor->GetProperty()->SetLineWidth(2.0);
        satBandOutlineActor->GetProperty()->SetSpecular(0.0);
        satBandOutlineActor->GetProperty()->SetOpacity(1.0);
        satBandOutlineActor->GetProperty()->SetRepresentationToWireframe();
    
        satBandActor->SetPickable(0);
        satBandOutlineActor->SetPickable(0);
        slabDataSet->Delete();
        slabPolyData->Delete();
}


/*!
 * Generates the clipping planes used to clip the bands to the extent of the dataset, and the slice.
 */
void svkSatBandSet::GenerateClippingPlanes( )
{
    if( this->spectra != NULL ) {

        int uIndexRange[2];
        int vIndexRange[2];
        int wIndexRange[2];
        uIndexRange[0] = spectraExtent[0];
        uIndexRange[1] = spectraExtent[1];
        vIndexRange[0] = spectraExtent[2];
        vIndexRange[1] = spectraExtent[3];
        wIndexRange[0] = spectraExtent[4];
        wIndexRange[1] = spectraExtent[5];
        double rowNormal[3];
        this->spectra->GetDataBasis(rowNormal, svkImageData::ROW );
        double columnNormal[3];
        this->spectra->GetDataBasis(columnNormal, svkImageData::COLUMN );
        double sliceNormal[3];
        this->spectra->GetDataBasis(sliceNormal, svkImageData::SLICE );

        switch (orientation) {
            case svkDcmHeader::AXIAL: 
                clippingPlanes[0]->SetNormal(  rowNormal[0],  rowNormal[1],  rowNormal[2] );
                clippingPlanes[1]->SetNormal( -rowNormal[0], -rowNormal[1], -rowNormal[2] );
                clippingPlanes[2]->SetNormal(  columnNormal[0],  columnNormal[1],  columnNormal[2] );
                clippingPlanes[3]->SetNormal( -columnNormal[0], -columnNormal[1], -columnNormal[2] );
                clippingPlanes[4]->SetNormal(  sliceNormal[0],  sliceNormal[1],  sliceNormal[2] );
                clippingPlanes[5]->SetNormal( -sliceNormal[0], -sliceNormal[1], -sliceNormal[2] );

                clippingPlanes[0]->SetOrigin( spectraOrigin[0] + this->spectraDeltaLR*(uIndexRange[0] - CLIP_TOLERANCE),
                                              spectraOrigin[1] + this->spectraDeltaAP*(uIndexRange[0] - CLIP_TOLERANCE), 
                                              spectraOrigin[2] + this->spectraDeltaSI*(uIndexRange[0] - CLIP_TOLERANCE) );

                clippingPlanes[1]->SetOrigin( spectraOrigin[0] + this->spectraDeltaLR * (uIndexRange[1] + CLIP_TOLERANCE),
                                              spectraOrigin[1] + this->spectraDeltaAP * (uIndexRange[1] + CLIP_TOLERANCE), 
                                              spectraOrigin[2] + this->spectraDeltaSI * (uIndexRange[1] + CLIP_TOLERANCE) );


                clippingPlanes[2]->SetOrigin( spectraOrigin[0] + this->spectraDeltaLR*(vIndexRange[0] - CLIP_TOLERANCE),
                                              spectraOrigin[1] + this->spectraDeltaAP*(vIndexRange[0] - CLIP_TOLERANCE), 
                                              spectraOrigin[2] + this->spectraDeltaSI*(vIndexRange[0] - CLIP_TOLERANCE) );

                clippingPlanes[3]->SetOrigin( spectraOrigin[0] + this->spectraDeltaLR*(vIndexRange[1] + CLIP_TOLERANCE),
                                              spectraOrigin[1] + this->spectraDeltaAP*(vIndexRange[1] + CLIP_TOLERANCE), 
                                              spectraOrigin[2] + this->spectraDeltaSI*(vIndexRange[1] + CLIP_TOLERANCE) );
                break;
            case svkDcmHeader::CORONAL:
                clippingPlanes[0]->SetNormal(  rowNormal[0],  rowNormal[1],  rowNormal[2] );
                clippingPlanes[1]->SetNormal( -rowNormal[0], -rowNormal[1], -rowNormal[2] );
                clippingPlanes[2]->SetNormal(  sliceNormal[0],  sliceNormal[1],  sliceNormal[2] );
                clippingPlanes[3]->SetNormal( -sliceNormal[0], -sliceNormal[1], -sliceNormal[2] );
                clippingPlanes[4]->SetNormal(  columnNormal[0],  columnNormal[1],  columnNormal[2] );
                clippingPlanes[5]->SetNormal( -columnNormal[0], -columnNormal[1], -columnNormal[2] );

                clippingPlanes[0]->SetOrigin( spectraOrigin[0] + this->spectraDeltaLR*(uIndexRange[0] - CLIP_TOLERANCE),
                                              spectraOrigin[1] + this->spectraDeltaAP*(uIndexRange[0] - CLIP_TOLERANCE), 
                                              spectraOrigin[2] + this->spectraDeltaSI*(uIndexRange[0] - CLIP_TOLERANCE) );

                clippingPlanes[1]->SetOrigin( spectraOrigin[0] + this->spectraDeltaLR * (uIndexRange[1] + CLIP_TOLERANCE),
                                              spectraOrigin[1] + this->spectraDeltaAP * (uIndexRange[1] + CLIP_TOLERANCE), 
                                              spectraOrigin[2] + this->spectraDeltaSI * (uIndexRange[1] + CLIP_TOLERANCE) );


                clippingPlanes[2]->SetOrigin( spectraOrigin[0] + this->spectraDeltaLR*(wIndexRange[0] - CLIP_TOLERANCE),
                                              spectraOrigin[1] + this->spectraDeltaAP*(wIndexRange[0] - CLIP_TOLERANCE), 
                                              spectraOrigin[2] + this->spectraDeltaSI*(wIndexRange[0] - CLIP_TOLERANCE) );

                clippingPlanes[3]->SetOrigin( spectraOrigin[0] + this->spectraDeltaLR*(wIndexRange[1] + CLIP_TOLERANCE),
                                              spectraOrigin[1] + this->spectraDeltaAP*(wIndexRange[1] + CLIP_TOLERANCE), 
                                              spectraOrigin[2] + this->spectraDeltaSI*(wIndexRange[1] + CLIP_TOLERANCE) );
                break;
            case svkDcmHeader::SAGITTAL:
                clippingPlanes[0]->SetNormal(  columnNormal[0],  columnNormal[1],  columnNormal[2] );
                clippingPlanes[1]->SetNormal( -columnNormal[0], -columnNormal[1], -columnNormal[2] );
                clippingPlanes[2]->SetNormal(  sliceNormal[0],  sliceNormal[1],  sliceNormal[2] );
                clippingPlanes[3]->SetNormal( -sliceNormal[0], -sliceNormal[1], -sliceNormal[2] );
                clippingPlanes[4]->SetNormal(  rowNormal[0],  rowNormal[1],  rowNormal[2] );
                clippingPlanes[5]->SetNormal( -rowNormal[0], -rowNormal[1], -rowNormal[2] );

                clippingPlanes[0]->SetOrigin( spectraOrigin[0] + this->spectraDeltaLR*(vIndexRange[0] - CLIP_TOLERANCE),
                                              spectraOrigin[1] + this->spectraDeltaAP*(vIndexRange[0] - CLIP_TOLERANCE), 
                                              spectraOrigin[2] + this->spectraDeltaSI*(vIndexRange[0] - CLIP_TOLERANCE) );

                clippingPlanes[1]->SetOrigin( spectraOrigin[0] + this->spectraDeltaLR * (vIndexRange[1] + CLIP_TOLERANCE),
                                              spectraOrigin[1] + this->spectraDeltaAP * (vIndexRange[1] + CLIP_TOLERANCE), 
                                              spectraOrigin[2] + this->spectraDeltaSI * (vIndexRange[1] + CLIP_TOLERANCE) );


                clippingPlanes[2]->SetOrigin( spectraOrigin[0] + this->spectraDeltaLR*(wIndexRange[0] - CLIP_TOLERANCE),
                                              spectraOrigin[1] + this->spectraDeltaAP*(wIndexRange[0] - CLIP_TOLERANCE), 
                                              spectraOrigin[2] + this->spectraDeltaSI*(wIndexRange[0] - CLIP_TOLERANCE) );

                clippingPlanes[3]->SetOrigin( spectraOrigin[0] + this->spectraDeltaLR*(wIndexRange[1] + CLIP_TOLERANCE),
                                              spectraOrigin[1] + this->spectraDeltaAP*(wIndexRange[1] + CLIP_TOLERANCE), 
                                              spectraOrigin[2] + this->spectraDeltaSI*(wIndexRange[1] + CLIP_TOLERANCE) );
                break;

        } 
        this->GenerateSliceClippingPlanes();


    } else {
        if( DEBUG ) {
            cout<<"INPUT HAS NOT BEEN SET!!"<<endl;
        }
    }
}


/*
 *  Generates planes used to clip the sat bands at the edges of the current slice.
 */
void svkSatBandSet::GenerateSliceClippingPlanes( )
{
    if( spectra != NULL ) {
        if( slice >= 0 ) {
            clippingPlanes[4]->SetOrigin( spectraOrigin[0] + this->spectraDeltaLR * ( slice - CLIP_TOLERANCE),
                                          spectraOrigin[1] + this->spectraDeltaAP * ( slice - CLIP_TOLERANCE),
                                          spectraOrigin[2] + this->spectraDeltaSI * ( slice - CLIP_TOLERANCE) );

            clippingPlanes[5]->SetOrigin( spectraOrigin[0] + this->spectraDeltaLR * ( slice + 1 + CLIP_TOLERANCE ),
                                          spectraOrigin[1] + this->spectraDeltaAP * ( slice + 1 + CLIP_TOLERANCE ),
                                          spectraOrigin[2] + this->spectraDeltaSI * ( slice + 1 + CLIP_TOLERANCE ) );
        }
    }
}


/*
 * Sets the orientation in which you wish to render the sat bands.
 */
void svkSatBandSet::SetOrientation( svkDcmHeader::Orientation orientation )
{
    this->orientation = orientation;
    this->GenerateClippingPlanes();
}
