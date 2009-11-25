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
 *  Test driver for readers and writers.
 *  
 *  The following classes are utilized in this driver. 
 *      svkImageReaderFactory
 *      svkIdfVolumeReader
 *      svkMultiWindowToImageFilter
 *      svkImageWriterFactory
 *      svkDICOMSCWriter
 *
 */


#include <svkImageReaderFactory.h>
#include <svkImageData.h>
using namespace svk;


int main (int argc, char** argv)
{
    string fname(argv[1]);
    double magRange[2] = { VTK_DOUBLE_MAX, VTK_DOUBLE_MIN };
    //  Turn debuggin on (1) or of (0):
    vtkObject::SetGlobalWarningDisplay(0);
    svkImageReaderFactory* readerFactory = svkImageReaderFactory::New();
    svkImageReader2* reader = readerFactory->CreateImageReader2( fname.c_str() );
    readerFactory->Delete();
    reader->SetFileName( fname.c_str() );
    reader->Update();
    svkImageData* id = reader->GetOutput(); 
    id->Register(NULL);
    //vtkImageData* tmpImage = reader->vtkImageAlgorithm::GetOutput();
    vtkImageData* tmpImage = id;
    tmpImage->Update();

    reader->Delete();
    cout<<"id reference count = "<<id->GetReferenceCount()<<endl;
    reader = NULL;
    cout<<"tmpImage: "<<*tmpImage<<endl;
    id->Delete(); 
    id = NULL;

    cout<<"GOODBYE TEST !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"<<endl;
    cout << "MAGNITUDE RANGE IS: " << magRange[0] << " " << magRange[1] << endl;
    
    return 0; 
}


