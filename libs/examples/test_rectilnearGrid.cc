/*
 *  $URL$ 
 *  $Rev$
 *  $Author$
 *  $Date$
 *
 *  Authors:
 *      Jason C. Crane, Ph.D.
 *      Beck Olson
 *
 *  License: TBD
 *
 *
 *
 *  Test driver for the svkPlotGridView/Controller pair.
 *
 *  The following classes are utilized in this driver.
 *      svkPlotGridView
 *      svkPlotGridViewController
 *
 */

#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkActorCollection.h>
#include <svkImageReader2.h>
#include <svkImageReaderFactory.h>
#include <svkImageData.h>
#include <svkImageTopologyGenerator.h>
#include <vtkCollectionIterator.h>
#include <vtkCamera.h>
#include <vtkDataSetMapper.h>
#include <vtkPolyDataMapper.h>
#include <vtkImageToPolyDataFilter.h>
#include <vtkFloatArray.h>
#include <vtkUnsignedCharArray.h>
#include <vtkPointData.h>
#include <vtkDataSetToStructuredGridFilter.h>
#include <vtkRectilinearGrid.h>
#include <vtkRectilinearGridGeometryFilter.h>
#include <vtkGeometryFilter.h>
#include <vtkExtractEdges.h>
#include <vtkInteractorStyleRubberBand2D.h>
#include <vtkAlgorithmOutput.h>
#include <vtkDataSetMapper.h>
#include <vtkVolume.h>
#include <vtkVolumeRayCastMapper.h>

using namespace svk;

svkImageData* LoadFile( char* fileName );

int main ( int argc, char** argv )
{
    char* fileName = NULL;
    if( argc == 1 ) {
        cerr<<"Not enough input arguments."<<endl;
        return 1;
    } else if( argc == 2 ) {
        fileName = argv[1];
    } else if( argc > 2) {
        cerr<<"Too many input arguments."<<endl;
        return 1;
    } 
    vtkRenderer* ren = vtkRenderer::New();
    vtkRenderWindow* window = vtkRenderWindow::New();
    vtkRenderWindowInteractor* rwi = vtkRenderWindowInteractor::New();
    window->SetInteractor( rwi );
    window->AddRenderer( ren );

    vtkUnsignedCharArray* myArray = vtkUnsignedCharArray::New();

    svkImageData* data = LoadFile( fileName );
    vtkImageData* vtkData = data;

    myArray->SetNumberOfTuples(vtkData->GetNumberOfPoints());
    myArray->SetNumberOfComponents(4);
    for( int i=0; i < 1521 ; i++ ) {
        if( i < 1521/2 ) {
            myArray->InsertTuple4(i, i/2,i/3,i/4, 0 );
        } else {
            myArray->InsertTuple4(i, i/2,i/3,i/4, 255 );
        }
    }

    vtkData->GetCellData()->Initialize();
    vtkData->GetPointData()->Initialize();
    vtkData->GetPointData()->SetScalars( myArray );
    cout<<"ImageData object:"<<*vtkData<<endl;

    vtkActor* actor = vtkActor::New();   

    vtkPolyDataMapper* myPdMapper = vtkPolyDataMapper::New(); 
    vtkExtractEdges* myggfilter = vtkExtractEdges::New();
    myggfilter->SetInput( vtkData );
    myPdMapper->SetInputConnection( myggfilter->GetOutputPort() );

    actor->SetMapper( myPdMapper );

    ren->AddActor( actor );
    ren->SetBackground(0.1,0.2,0.4 );

    ren->ResetCamera(); 
    
    window->Render();
    rwi->Start();
   
    actor->Delete(); 
    myggfilter->Delete();
    myPdMapper->Delete();
    ren->Delete();
    window->Delete();
    rwi->Delete();
    data->Delete();
    
    
    fileName = NULL;
   
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
    myData->Register( NULL );
    reader->Delete();
    return myData;
}
