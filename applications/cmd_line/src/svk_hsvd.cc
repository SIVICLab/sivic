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
 *      Bjoern Menze, Ph.D
 *      Jason C. Crane, Ph.D.
 *      Beck Olson
 *
 *  Utility application for applying an HSVD filter to an MRS data set. 
 *
 */

#include <vtkSmartPointer.h>

#include <svkImageReaderFactory.h>
#include <svkImageWriterFactory.h>
#include <svkImageWriter.h>
#include <svkDcmHeader.h>
#include <svkHSVD.h>
#include <svkSpecPoint.h>


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
    usemsg += "svk_hsvd -i input_file_name -o output_file_name [ -t output_data_type ]  \n"; 
    usemsg += "         [ -fb ] [ -m order ] [ -h ]                                      \n"; 
    usemsg += "                                                                         \n";  
    usemsg += "   -i                name   Name of file to convert.                     \n"; 
    usemsg += "   -o                name   Name of outputfile.                          \n";
    usemsg += "   -t                type   Target data type:                            \n";
    usemsg += "                                 2 = UCSF DDF                            \n";
    usemsg += "                                 4 = DICOM_MRS (default)                 \n";
    usemsg += "   -f                       Write out filter image                       \n"; 
    usemsg += "   -b                       only filter spectra in selection box, others \n"; 
    usemsg += "                            are zeroed out.                              \n"; 
    usemsg += "   -m                order  model order (default = 25)                   \n"; 
    usemsg += "   -w                       remove water (downfield of 4.2PPM )          \n"; 
    usemsg += "   -l                       remove lipid (upfield of 1.8PPM )          \n"; 
    usemsg += "   -h                       Print this help mesage.                      \n";  
    usemsg += "                                                                         \n";  
    usemsg += "HSVD filter to remove baseline components from spectra.                  \n";  
    usemsg += "Default is to remove water by filtering all frequencies downfield        \n"; 
    usemsg += "4.2 PPM.                                                                 \n"; 
    usemsg += "\n";  


    string inputFileName; 
    string outputFileName;
    bool writeFilter = false; 
    int modelOrder = 25; 
    bool limitToSelectionBox = false; 
    bool filterWater = false; 
    bool filterLipid = false; 

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
    while ( ( i = getopt_long(argc, argv, "i:o:t:m:fbwlh", long_options, &option_index) ) != EOF) {
        switch (i) {
            case 'i':
                inputFileName.assign( optarg );
                break;
            case 'o':
                outputFileName.assign(optarg);
                break;
            case 't':
                dataTypeOut = static_cast<svkImageWriterFactory::WriterType>( atoi(optarg) );
                break;
            case 'f':
                writeFilter = true; 
                break;
            case 'b':
                limitToSelectionBox = true; 
                break;
            case 'm':
                modelOrder = atoi( optarg ); 
                break;
            case 'w':
                filterWater = true; 
                break;
            case 'l':
                filterLipid = true; 
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
    cout << "file name: " << outputFileName << endl;
    cout << "argc: " << argc << endl;
    if ( argc != 0 ||  inputFileName.length() == 0  
         || outputFileName.length() == 0 
         || ( dataTypeOut != svkImageWriterFactory::DICOM_MRS && dataTypeOut != svkImageWriterFactory::DDF ) 
         
    ) {
        cout << usemsg << endl;
        exit(1); 
    }

    cout << "file name: " << inputFileName << endl;

    //  if no filters specified, then just filter water:
    if ( !filterWater && !filterLipid ) {
        filterWater = true; 
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
    svkSpecPoint* point = svkSpecPoint::New();
    point->SetDcmHeader( hdr ); 
    int numFreqPoints = hdr->GetIntValue( "DataPointColumns" );

    // ===============================================  
    //  HSVD DATA   
    // ===============================================  
    svkHSVD* hsvd = svkHSVD::New();
    if ( writeFilter ) {
        hsvd->ExportFilterImage(); 
    }
    hsvd->SetInput( reader->GetOutput() ); 
    hsvd->SetModelOrder( modelOrder ); 
    if ( limitToSelectionBox ) {
        hsvd->OnlyFitSpectraInVolumeLocalization(); 
    }
    if ( filterWater ) {
        cout << "Filter Water " << endl;
        hsvd->RemoveH20On();
    }
    if ( filterLipid ) {
        cout << "Filter Lipid " << endl;
        hsvd->RemoveLipidOn();
    }
    hsvd->Update();

    // ===============================================  
    //  Write the data out to the specified file type.  
    //  Use an svkImageWriterFactory to obtain the
    //  correct writer type. 
    // ===============================================  
    vtkSmartPointer< svkImageWriterFactory > writerFactory = vtkSmartPointer< svkImageWriterFactory >::New(); 
    svkImageWriter* writer = static_cast<svkImageWriter*>( writerFactory->CreateImageWriter( dataTypeOut ) ); 

    if ( writer == NULL ) {
        cerr << "Can not determine writer of type: " << dataTypeOut << endl;
        exit(1);
    }

    writer->SetFileName( outputFileName.c_str() );
    writer->SetInput( svkMrsImageData::SafeDownCast( reader->GetOutput() ) );

    // ===============================================  
    //  Set the input command line into the data set 
    //  provenance: 
    // ===============================================  
    reader->GetOutput()->GetProvenance()->SetApplicationCommand( cmdLine );

    // ===============================================  
    //  Write data to file: 
    // ===============================================  
    writer->Write();


    //  Write out the filter image if requested.
    if ( writeFilter ) {

        string filterImageName = outputFileName; 
        filterImageName.append("_filter"); 
        
        writer->SetFileName( filterImageName.c_str() );
        writer->SetInput( hsvd->GetFilterImage() ); 

        // ===============================================  
        //  Set the input command line into the data set 
        //  provenance: 
        // ===============================================  
        hsvd->GetFilterImage()->GetProvenance()->SetApplicationCommand( cmdLine );

        // ===============================================  
        //  Write data to file: 
        // ===============================================  
        writer->Write();
        
    }

    // ===============================================  
    //  Clean up: 
    // ===============================================  
    writer->Delete();
    reader->Delete();
    hsvd->Delete();

    return 0; 
}

