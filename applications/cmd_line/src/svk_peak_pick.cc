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
 *
 *  Utility application to detect peaks in spectra. 
 *
 */

#include <vtkSmartPointer.h>

#include <svkImageReaderFactory.h>
#include <svkImageWriterFactory.h>
#include <svkImageWriter.h>
#include <svkDcmHeader.h>
#include <svkMRSPeakPick.h>
#include <svkMRSNoise.h>


#ifdef WIN32
extern "C" {
#include <getopt.h>
}
#else
#include <getopt.h>
#endif


using namespace svk;


int main (int argc, char** argv)
{

    string usemsg("\n") ; 
    usemsg += "Version " + string(SVK_RELEASE_VERSION) +                               "\n";   
    usemsg += "svk_peak_pick -i input_file_name                                         \n"; 
    usemsg += "         [ -s S/N] [ -b ] [ -h ]                                         \n"; 
    usemsg += "                                                                         \n";  
    usemsg += "   -i    name    Name of file to convert.                                \n"; 
    usemsg += "   -b            only include spectra in selection box.                  \n"; 
    usemsg += "   -s    S/N     Find peaks over this S/N value (defalut = 3).           \n"; 
    usemsg += "   -h            Print this help mesage.                                 \n";  
    usemsg += "                                                                         \n";  
    usemsg += "Locates peaks in spectra.                                                \n";  
    usemsg += "\n";  


    string inputFileName; 
    bool limitToSelectionBox = false; 
    float  signalToNoise = 3;  

    svkImageWriterFactory::WriterType dataTypeOut = svkImageWriterFactory::DICOM_MRS;

    string cmdLine = svkProvenance::GetCommandLineString( argc, argv ); 

    enum FLAG_NAME {
    }; 


    static struct option long_options[] =
    {
        /* This option sets a flag. */
        {0, 0, 0, 0}
    };



    // ===============================================  
    //  Process flags and arguments
    // ===============================================  
    int i;
    int option_index = 0; 
    while ( ( i = getopt_long(argc, argv, "i:bs:h", long_options, &option_index) ) != EOF) {
        switch (i) {
            case 'i':
                inputFileName.assign( optarg );
                break;
            case 'b':
                limitToSelectionBox = true; 
                break;
            case 's':
                signalToNoise =  atof(optarg);
                break;
            case 'h':
                cout << usemsg << endl;
                exit(1);  
                break;
            default:
                ;
        }
    }

    argc -= optind;
    argv += optind;

    // ===============================================  
    //  validate that: 
    //      an output name was supplied
    //      that not suppresses and unsuppressed were both specified 
    //      that only the supported output types was requested. 
    //      
    // ===============================================  
    cout << "file name: " << inputFileName << endl;
    cout << "argc: " << argc << endl;
    if ( argc != 0 ||  inputFileName.length() == 0  ) {
        cout << usemsg << endl;
        exit(1); 
    }

    // ===============================================  
    //  Use a reader factory to get the correct reader  
    //  type . 
    // ===============================================  
    vtkSmartPointer< svkImageReaderFactory > readerFactory = vtkSmartPointer< svkImageReaderFactory >::New(); 
    svkImageReader2* reader = readerFactory->CreateImageReader2(inputFileName.c_str());

    if (reader == NULL) {
        cerr << "Can not determine appropriate reader for: " << inputFileName << endl;
        exit(1);
    }

    reader->SetFileName( inputFileName.c_str() );
    reader->Update(); 

    svkDcmHeader* hdr = reader->GetOutput()->GetDcmHeader(); 

    // ===============================================  
    //  Pick Peaks 
    // ===============================================  
    svkMRSPeakPick* peaks = svkMRSPeakPick::New();
    peaks->SetInputData( reader->GetOutput() ); 
    if ( limitToSelectionBox ) {
        peaks->OnlyUseSelectionBox(); 
    }
    peaks->SetSNLimit( signalToNoise );
    peaks->Update();


////////////////////////////////////////////////////
// temporary test: put average mag data in cell0
svkMRSNoise* noise = svkMRSNoise::New();
noise->SetInputData( reader->GetOutput() );
if ( limitToSelectionBox ) {
    noise->OnlyUseSelectionBox();
}
noise->Update();

svkMrsImageData* data = svkMrsImageData::SafeDownCast(reader->GetOutput()); 
int numTimePoints = data->GetDcmHeader()->GetIntValue( "DataPointColumns" );
vtkFloatArray* cell0 = static_cast<vtkFloatArray*>( data->GetSpectrum( 0 ) );
float tupleAv[2];
float tuple[2];

vtkFloatArray* avSpec = noise->GetAverageMagnitudeSpectrum();
for (int i = 0; i < numTimePoints; i++ ) { 
    avSpec->GetTupleValue(i, tupleAv);
    cell0->GetTupleValue(i, tuple);
    tuple[0] = tupleAv[0];
    tuple[1] = tupleAv[1];
    cell0->SetTuple(i, tuple);
}   
noise->Delete(); 
vtkSmartPointer< svkImageWriterFactory > writerFactory = vtkSmartPointer< svkImageWriterFactory >::New();
svkImageWriter* writer = static_cast<svkImageWriter*>(writerFactory->CreateImageWriter( svkImageWriterFactory::DDF));
writer->SetFileName( "magcheck" ); 
writer->SetInputData( reader->GetOutput() );
writer->Write();
writer->Delete();

////////////////////////////////////////////////////

    //float = noise->GetNoiseSD(); 
    //float mean = noise->GetMeanBaseline(); 
    //cout << "NOISE SD: " << noiseSD << endl;
    //cout << "Mean Baseline: " << mean << endl;
    //float mean = noise->GetMean(); 

    // ===============================================  
    //  Clean up: 
    // ===============================================  
    reader->Delete();
    peaks->Delete();

    return 0; 
}

