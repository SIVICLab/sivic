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
    /*
    svkImageData* data = LoadFile( fileName );
    data->Update();
    data->Delete();
    */ 
      
    vtkRenderer* ren = vtkRenderer::New();
    vtkRenderWindow* window = vtkRenderWindow::New();
    vtkRenderWindowInteractor* rwi = vtkRenderWindowInteractor::New();
    vtkCollectionIterator* myIterator = vtkCollectionIterator::New();
    window->SetInteractor( rwi );
    window->AddRenderer( ren );
    svkImageData* data = LoadFile( fileName );
    vtkActorCollection* topology = data->GetTopoActorCollection(0);
   
    myIterator->SetCollection(topology);
    myIterator->InitTraversal();
    while( !myIterator->IsDoneWithTraversal() ) {
        ren->AddActor( vtkActor::SafeDownCast( myIterator->GetCurrentObject() ) );
        myIterator->GoToNextItem();
    }
    
    ren->GetActiveCamera()->SetParallelProjection(1); 
    window->Render();
    vtkActorCollection* selectionBoxCollection = data->GetTopoActorCollection(1);
    selectionBoxCollection->InitTraversal();
    vtkActor* selectionBox = selectionBoxCollection->GetNextActor();
    ren->AddActor( selectionBox );
    ren->ResetCamera(); 
    
    rwi->Start();
    
    ren->Delete();
    window->Delete();
    rwi->Delete();
    
    cout<<"Topology:"<<endl<<*topology<<endl;
    myIterator->Delete();
    cout<<"data:"<<endl<<*data<<endl;
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
