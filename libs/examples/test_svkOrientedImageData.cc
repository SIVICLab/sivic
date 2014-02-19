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


#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>
#include <svkImageReader2.h>
#include <svkImageReaderFactory.h>
#include <svkImageData.h>
#include <svkMriImageData.h>
#include <svkMrsImageData.h>
#include <vtkCamera.h>
#include <vtkFloatArray.h>
#include <vtkPointData.h>
#include <vtkExtractEdges.h>
#include <vtkImageViewer2.h>
#include <svkImageData.h>
#include <vtkImageDataGeometryFilter.h>
#include <vtkWindowLevelLookupTable.h>
#include <vtkPlane.h>
#include <vtkAxesActor.h>
#include <vtkCubeSource.h>
#include <vtkTransform.h>

using namespace svk;

svkImageData* LoadFile( char* fileName );

int main ( int argc, char** argv )
{
    char* fileNameImage = NULL;
    char* fileNameSpectra = NULL;
    if( argc < 3 ) {
        cerr<<"Not enough input arguments."<<endl;
        return 1;
    } else if( argc == 3 ) {
        fileNameImage = argv[1];
        fileNameSpectra = argv[2];
    } else if( argc > 3) {
        cerr<<"Too many input arguments."<<endl;
        return 1;
    } 
    double* bounds;
    double* range;
    int* extent;
    double* origin;

    // Setup cosine matrix, this should be grabbed from headers
    double dcos[3][3];
    dcos[0][0] = 1;
    dcos[0][1] = 0;
    dcos[0][2] = 0;
    dcos[1][0] = 0;
    dcos[1][1] = 0.99284;
    dcos[1][2] = 0.11948;
    dcos[2][0] = 0;
    dcos[2][1] = 0.11948;
    dcos[2][2] = -0.99284;

    /*
    double dcos[3][3];
    dcos[0][0] = 1;
    dcos[0][1] = 0;
    dcos[0][2] = 0;
    dcos[1][0] = 0;
    dcos[1][1] = 1;
    dcos[1][2] = 0;
    dcos[2][0] = 0;
    dcos[2][1] = 0;
    dcos[2][2] = -1;
    */

    /*
    // Second orientation test
    dcm[0] = 1;
    dcm[1] = 0;
    dcm[2] = -0.00165;
    dcm[3] = 0.00165;
    dcm[4] = 0.11948;
    dcm[5] = 0.99284;
    dcm[6] = 0.00018;
    dcm[7] = -0.99284;
    dcm[8] = 0.11948;
    */

    // Load the data sets
    svkImageData* image = LoadFile( fileNameImage );
    // Create oblique image
    svkImageData* obliqueImage = svkMriImageData::New();
    obliqueImage->CopyVtkImage( image, dcos );
    svkImageData* spectra = LoadFile( fileNameSpectra );
    // Create oblique spectra
    svkImageData* obliqueSpectra = svkMrsImageData::New();
    obliqueSpectra->CopyVtkImage( spectra, dcos );
    
    // initialize window
    vtkRenderer* ren = vtkRenderer::New();
    ren->SetBackground(0.1,0.2,0.4);
    vtkRenderWindow* window = vtkRenderWindow::New();
    vtkRenderWindowInteractor* rwi = vtkRenderWindowInteractor::New();
    window->SetInteractor( rwi );
    window->AddRenderer( ren);  
    window->SetSize(600,600);  


    // Throw in an axes as a reference
    vtkAxesActor* myAxesNormal = vtkAxesActor::New();
    vtkTransform* optimus = vtkTransform::New();
    origin = image->GetOrigin();
    optimus->Translate( origin );
    myAxesNormal->SetTotalLength(100,100,100); 
    myAxesNormal->SetUserTransform(optimus);
    optimus->Delete();
    ren->AddActor( myAxesNormal );
    myAxesNormal->Delete();

    myAxesNormal = vtkAxesActor::New();
    optimus = vtkTransform::New();
    origin = spectra->GetOrigin();
    optimus->Translate( origin );
    myAxesNormal->SetTotalLength(50,50,50); 
    myAxesNormal->SetUserTransform(optimus);
    optimus->Delete();
    ren->AddActor( myAxesNormal );
    myAxesNormal->Delete();
    // Lets create an actor for the normal image 
    extent = image->GetExtent();

    // Lets pipe the image throught a geometry extractor to an actor
    vtkActor* imageActor = vtkActor::New();
    vtkDataSetMapper* mapper = vtkDataSetMapper::New();
    vtkImageDataGeometryFilter* geomFilter = vtkImageDataGeometryFilter::New();
    geomFilter->SetInput( image );
    geomFilter->SetExtent( extent[0], extent[1], extent[2], extent[3], 1, 1 );

    // We have to setup the window levelling correctly
    range = image->GetScalarRange();
    vtkWindowLevelLookupTable* windowLevelTable = vtkWindowLevelLookupTable::New();
    windowLevelTable->SetWindow(range[1] - range[0]);
    windowLevelTable->SetLevel(0.5 * (range[1] + range[0]));
    windowLevelTable->Build();
    mapper->InterpolateScalarsBeforeMappingOff();
    mapper->SetInputConnection( geomFilter->GetOutputPort() );
    geomFilter->Delete();
    mapper->SetScalarRange( image->GetPointData()->GetArray(0)->GetRange() );
    mapper->SetLookupTable( windowLevelTable);
    windowLevelTable->Delete();

    imageActor->SetMapper( mapper );
    mapper->Delete();
    imageActor->GetProperty()->SetSpecular(0);
    imageActor->GetProperty()->SetDiffuse(0);
    imageActor->GetProperty()->SetAmbient(1);
    imageActor->GetProperty()->SetOpacity(0.75);
    imageActor->GetProperty()->SetInterpolationToFlat();
    bounds = image->GetBounds();
    ren->AddActor( imageActor );
    imageActor->Delete();

    // Now lets visualize the bounds of the actor
    vtkActor* boundsActor = vtkActor::New();
    vtkCubeSource* boundsCube = vtkCubeSource::New();
    boundsCube->SetBounds( bounds );
    mapper = vtkDataSetMapper::New();
    mapper->SetInput( boundsCube->GetOutput() );
    boundsCube->Delete();
    boundsActor->SetMapper(mapper); 
    mapper->Delete();
    boundsActor->GetProperty()->SetRepresentationToWireframe(); 
    boundsActor->GetProperty()->SetSpecular(0);
    boundsActor->GetProperty()->SetDiffuse(0);
    boundsActor->GetProperty()->SetAmbient(1);
    boundsActor->GetProperty()->SetColor(0,0,1);
    ren->AddActor( boundsActor );
    boundsActor->Delete();
    

    // Lets create an actor for the oblique object

    vtkActor* obliqueImageActor = vtkActor::New();
    mapper = vtkDataSetMapper::New();
    geomFilter = vtkImageDataGeometryFilter::New();
    geomFilter->SetInput( obliqueImage );
    extent = obliqueImage->GetExtent();
    geomFilter->SetExtent( extent[0], extent[1], extent[2], extent[3], 0, 0 );
    range = image->GetScalarRange();

    windowLevelTable = vtkWindowLevelLookupTable::New();
    windowLevelTable->SetWindow(range[1] - range[0]);
    windowLevelTable->SetLevel(0.5 * (range[1] + range[0]));
    windowLevelTable->Build();

    mapper->InterpolateScalarsBeforeMappingOff();
    mapper->SetInputConnection( geomFilter->GetOutputPort() );
    geomFilter->Delete();
    mapper->SetScalarRange( image->GetPointData()->GetArray(0)->GetRange() );
    mapper->SetLookupTable( windowLevelTable);
    windowLevelTable->Delete();
    obliqueImageActor->SetMapper( mapper );
    mapper->Delete();
    obliqueImageActor->GetProperty()->SetSpecular(0);
    obliqueImageActor->GetProperty()->SetDiffuse(0);
    obliqueImageActor->GetProperty()->SetAmbient(1);
    obliqueImageActor->GetProperty()->SetInterpolationToFlat();
    bounds = obliqueImage->GetBounds();
    ren->AddActor( obliqueImageActor );
    obliqueImageActor->Delete();

    boundsActor = vtkActor::New();
    boundsCube = vtkCubeSource::New();
    boundsCube->SetBounds( bounds );
    mapper = vtkDataSetMapper::New();
    mapper->SetInput( boundsCube->GetOutput() );
    boundsCube->Delete();
    boundsActor->SetMapper(mapper); 
    mapper->Delete();
    boundsActor->GetProperty()->SetRepresentationToWireframe(); 
    boundsActor->GetProperty()->SetSpecular(0);
    boundsActor->GetProperty()->SetDiffuse(0);
    boundsActor->GetProperty()->SetAmbient(1);
    ren->AddActor( boundsActor );
    boundsActor->Delete();

    // Lets render the spectra
    vtkExtractEdges* edgeExtractor = vtkExtractEdges::New();   
    edgeExtractor->SetInput(spectra );
    vtkPolyDataMapper* specMapper = vtkPolyDataMapper::New();
    specMapper->SetInput( edgeExtractor->GetOutput() );
    edgeExtractor->Delete();   
    //vtkPolyData* myEdges = edgeExtractor->GetOutput();
    //myEdges->Update();
    specMapper->ScalarVisibilityOff();
    vtkActor* spactor = vtkActor::New();
    spactor->SetMapper( specMapper );
    specMapper->Delete();
    spactor->GetProperty()->SetDiffuseColor(1,0,0);
    spactor->GetProperty()->SetOpacity(0.75);
    ren->AddActor( spactor );
    spactor->Delete();

    // Now oblique Spectra
    edgeExtractor = vtkExtractEdges::New();   
    edgeExtractor->SetInput(obliqueSpectra );
    specMapper = vtkPolyDataMapper::New();
    specMapper->SetInput( edgeExtractor->GetOutput() );
    edgeExtractor->Delete();   
    //myEdges = edgeExtractor->GetOutput();
    //myEdges->Update();
    specMapper->ScalarVisibilityOff();
    vtkActor* obliqueSpactor = vtkActor::New();
    obliqueSpactor->SetMapper( specMapper );
    specMapper->Delete();
    obliqueSpactor->GetProperty()->SetDiffuseColor(0,1,0);
    obliqueSpactor->GetProperty()->SetOpacity(0.75);
    ren->AddActor( obliqueSpactor );
    obliqueSpactor->Delete();

    bounds = obliqueSpectra->GetBounds();
    boundsActor = vtkActor::New();
    boundsCube = vtkCubeSource::New();
    boundsCube->SetBounds( bounds );
    mapper = vtkDataSetMapper::New();
    mapper->SetInput( boundsCube->GetOutput() );
    boundsCube->Delete();
    boundsActor->SetMapper(mapper); 
    mapper->Delete();
    boundsActor->GetProperty()->SetRepresentationToWireframe(); 
    boundsActor->GetProperty()->SetSpecular(0);
    boundsActor->GetProperty()->SetDiffuse(0);
    boundsActor->GetProperty()->SetAmbient(1);
    ren->AddActor( boundsActor );
    boundsActor->Delete();
 
    // Check results... 
    cout<<"Spectra: "<<*spectra;
    /*
    cout<<"Oblique Spectra: "<<*obliqueSpectra;
    cout<<"Edge Extractor Output: "<<*myEdges;
    cout<<"Oblique Image Bounds:"<<bounds[0]<<" "<<bounds[1]<<" "<<bounds[2]<<" "<<bounds[3]<<" "<<bounds[4]<<" "<<bounds[5]<<endl;
    bounds = image->GetBounds();
    cout<<"Normal Image Bounds:"<<bounds[0]<<" "<<bounds[1]<<" "<<bounds[2]<<" "<<bounds[3]<<" "<<bounds[4]<<" "<<bounds[5]<<endl;
    bounds = obliqueSpectra->GetBounds();
    cout<<"Oblique Spectra Bounds:"<<bounds[0]<<" "<<bounds[1]<<" "<<bounds[2]<<" "<<bounds[3]<<" "<<bounds[4]<<" "<<bounds[5]<<endl;
    bounds = spectra->GetBounds();
    cout<<"Normal Spectra Bounds:"<<bounds[0]<<" "<<bounds[1]<<" "<<bounds[2]<<" "<<bounds[3]<<" "<<bounds[4]<<" "<<bounds[5]<<endl;
*/
    ren->ResetCamera();
    window->Render();  
    rwi->Start();
    
    ren->Delete();
    window->Delete();
    rwi->Delete();
    obliqueImage->Delete();
    obliqueSpectra->Delete();
    fileNameImage = NULL;
    fileNameSpectra = NULL;
    image->Delete();
    spectra->Delete();
    cout<<"DONE################################################################"<<endl;
    return 0;
    
}

svkImageData* LoadFile( char* fileName )
{
    svkImageData* myData;
    svkImageReaderFactory* readerFactory = svkImageReaderFactory::New();
    svkImageReader2* reader = readerFactory->CreateImageReader2(fileName);
    readerFactory->Delete();

    if (reader == NULL) {
        cerr << "Can not determine appropriate reader for: " << fileName << endl;
        exit(1);
    }

    reader->SetFileName( fileName );
    reader->Update();
    myData = reader->GetOutput();
    myData->Register(NULL);
    reader->Delete();
    return myData;
}
