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
 *  Utility application for converting between supported file formats. 
 *
 */


#include <vtkSmartPointer.h>

#include <svkImageReaderFactory.h>
#include <svkImageReader2.h>
#include <svkImageWriterFactory.h>
#include <svkImageWriter.h>
#include <svkIdfVolumeWriter.h>
#include <svkIntegratePeak.h>
#include <svkMetaboliteMap.h>
#include <svkQuantifyMetabolites.h>
#ifdef WIN32
extern "C" {
#include <getopt.h>
}
#else
#include <getopt.h>
#endif


#define UNDEFINED_VAL -99999


using namespace svk;


int main (int argc, char** argv)
{

    string usemsg("\n") ; 
    usemsg += "Version " + string(SVK_RELEASE_VERSION) + "\n";   
    usemsg += "svk_quantify -i input_file_name -o output_root_name -t output_data_type              \n";
    usemsg += "             (--peak_center ppm --peak_width ppm --peak_name name | --xml file )     \n";
    usemsg += "             [ --algo type ] [ -b ]                                                  \n"; 
    usemsg += "             [--verbose ] [ -h ]                                                     \n"; 
    usemsg += "                                                                                     \n";  
    usemsg += "   -i input_file_name        name of file to convert.                                \n"; 
    usemsg += "   -o output_root_name       root name of outputfile. This is a directory if using   \n";  
    usemsg += "                             --xml.  All files will get written to the dir.          \n";  
    usemsg += "   -t output_data_type       target data type:                                       \n";  
    usemsg += "                                 3 = UCSF IDF                                        \n";  
    usemsg += "                                 6 = DICOM_MRI                                       \n";  
    usemsg += "   -b                        Only fit inside volume selection                        \n"; 
    usemsg += "   --xml               file  XML quantification config file                          \n"; 
    usemsg += "   --peak_center       ppm   Chemical shift of peak 1 center                         \n";
    usemsg += "   --peak_width        ppm   Width in ppm of peak 1 integration                      \n";
    usemsg += "   --peak_name         name  String label name for peak or ratio                     \n"; 
    usemsg += "   --verbose                 Prints pk ht and integrals for each voxel to stdout.    \n"; 
    usemsg += "   --algo              type  Quantification algorithm :                              \n"; 
    usemsg += "                                 1 = Peak Ht (default)                               \n";  
    usemsg += "                                 2 = Integration                                     \n";  
    usemsg += "                                 3 = Line Width                                      \n";
    usemsg += "                                 4 = Magnitude Peak Ht                               \n";
    usemsg += "                                 5 = Magnitude Integration                           \n";
    usemsg += "                                 6 = Magnitude Line Width                            \n";
    usemsg += "   -h                        print help mesage.                                      \n";  
    usemsg += "                                                                                     \n";  
    usemsg += "Generates metabolite map volume by direct integration of input spectra over          \n"; 
    usemsg += "the specified chemical shift range, or by peak ht determination.                     \n";
    usemsg += "Alternatively, may compute a set of maps based on an input xml configuration file    \n";
    usemsg += "Example:                                                                             \n";
    usemsg += "                                                                                     \n";  
    usemsg += "    Calculate NAA peak ht map                                                        \n";
    usemsg += "    svk_quantify -i mrs.dcm -o naa_pk.dcm -t6 --peak_center 2 --peak_width .4        \n";
    usemsg += "                 --peak_2_width .2 --algo 2 --zscore --peak_name NAA_PK_HT           \n";
    usemsg += "    Calculate all quantities specified in xml file                                   \n";
    usemsg += "    svk_quantify -i mrs.dcm -o test -t3 --xml mrs.xml                                \n";
    usemsg += "\n";  

    string  inputFileName; 
    string  outputFileRoot; 
    string  xmlFileName; 
    svkImageWriterFactory::WriterType dataTypeOut = svkImageWriterFactory::UNDEFINED; 
    float   peakCenterPPM;
    float   peakWidthPPM;
    string  qtyName;
    bool    isVerbose = false;   
    svkMetaboliteMap::algorithm algo = svkMetaboliteMap::PEAK_HT; 
    int     algoInt; 
    bool    onlyQuantifySelectedVolume = false;   


    string cmdLine = svkProvenance::GetCommandLineString( argc, argv );

    enum FLAG_NAME {
        FLAG_PEAK_CENTER = 0,
        FLAG_PEAK_WIDTH, 
        FLAG_QTY_NAME, 
        FLAG_ALGORITHM, 
        FLAG_XML_FILE, 
        FLAG_VERBOSE  
    };


    static struct option long_options[] =
    {
        /* This option sets a flag. */
        {"peak_center",      required_argument, NULL,  FLAG_PEAK_CENTER},
        {"peak_width",       required_argument, NULL,  FLAG_PEAK_WIDTH},
        {"peak_name",        required_argument, NULL,  FLAG_QTY_NAME},
        {"algo",             required_argument, NULL,  FLAG_ALGORITHM},
        {"xml",              required_argument, NULL,  FLAG_XML_FILE},
        {"verbose",          no_argument      , NULL,  FLAG_VERBOSE},
        {0, 0, 0, 0}
    };


    // ===============================================
    //  Process flags and arguments
    // ===============================================
    int i;
    int option_index = 0;
    while ( ( i = getopt_long(argc, argv, "i:o:t:bh", long_options, &option_index) ) != EOF) {
        switch (i) {
            case 'i':
                inputFileName.assign( optarg );
                break;
            case 'o':
                outputFileRoot.assign(optarg);
                break;
            case 't':
                dataTypeOut = static_cast<svkImageWriterFactory::WriterType>( atoi(optarg) );
                break;
            case 'b':
                onlyQuantifySelectedVolume = true;   
                break;
           case FLAG_PEAK_CENTER:
                peakCenterPPM = atof( optarg);
                break;
           case FLAG_PEAK_WIDTH:
                peakWidthPPM = atof( optarg);
                break;
           case FLAG_QTY_NAME:
                qtyName.assign( optarg);
                break;
           case FLAG_VERBOSE:
                isVerbose = true; 
                break;
           case FLAG_XML_FILE:
                xmlFileName.assign(optarg);
                break;
           case FLAG_ALGORITHM:
                algoInt = atoi(optarg); 
                if ( algoInt == 1 ) {
                    algo = svkMetaboliteMap::PEAK_HT; 
                } else if ( algoInt == 2 ) {
                    algo = svkMetaboliteMap::INTEGRATE; 
                } else if ( algoInt == 3 ) {
                    algo = svkMetaboliteMap::LINE_WIDTH;
                } else if ( algoInt == 4 ) {
                    algo = svkMetaboliteMap::MAG_PEAK_HT;
                } else if ( algoInt == 5 ) {
                    algo = svkMetaboliteMap::MAG_INTEGRATE;
                } else if ( algoInt == 6 ) {
                    algo = svkMetaboliteMap::MAG_LINE_WIDTH;
                } else {
                    cout << "ERROR: invalid algorithm: " << algoInt << endl;
                    cout << usemsg << endl;
                    exit(1); 
                }
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


    if ( argc != 0 ) {
        cout << usemsg << endl;
        exit(1); 
    }
  
    
    if ( xmlFileName.length() == 0  && ( inputFileName.length() == 0 || outputFileRoot.length() == 0 ) ) {
        cout << usemsg << endl;
        exit(1); 
    }
  
    //  If no xml file, then validate the other inputs. 
    if ( xmlFileName.length() == 0 ) { 
        if ( peakCenterPPM == UNDEFINED_VAL || peakWidthPPM == UNDEFINED_VAL || qtyName.length() == 0 ) {
            cout << usemsg << endl;
            exit(1); 
        }
    } 

    if ( dataTypeOut < 0 || dataTypeOut >= svkImageWriterFactory::LAST_TYPE ) { 
        cout << usemsg << endl;
        exit(1); 
    }

    //cout << inputFileName << endl;
    //cout << outputFileRoot << endl;
    //cout << dataTypeOut << endl;
    //cout << peakCenterPPM << endl;
    //cout << peakWidthPPM << endl;
    //cout << qtyName<< endl;


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

    // ===============================================
    //  Use the reader to read the data into an
    //  svkMrsImageData object and set any reading
    //  behaviors (e.g. average suppressed data).
    // ===============================================
    reader->SetFileName( inputFileName.c_str() );
    reader->Update(); 


    //  if using XML config, then quantify all mets here, else only quantify the specified peak: 
    if ( xmlFileName.length() != 0 ) {
        svkQuantifyMetabolites* quantMets = svkQuantifyMetabolites::New();
        quantMets->SetXMLFileName( xmlFileName );
        if ( isVerbose ) { 
            quantMets->SetVerbose( isVerbose ); 
        }    
        if ( onlyQuantifySelectedVolume ) { 
            quantMets->LimitToSelectedVolume();
        }    
        quantMets->SetInput( reader->GetOutput() ); 
        quantMets->LimitToSelectedVolume();    
        quantMets->Update();
        vtkstd::vector<svkMriImageData*>* metMapVector = quantMets->GetMetMaps();
    
        vtkSmartPointer< svkImageWriterFactory > writerFactory = vtkSmartPointer< svkImageWriterFactory >::New(); 
        svkImageWriter* writer = static_cast<svkImageWriter*>( writerFactory->CreateImageWriter( dataTypeOut ) ); 
        
        if ( writer == NULL ) {
            cerr << "Can not determine writer of type: " << dataTypeOut << endl;
            exit(1);
        }
        for (int mapId = 0; mapId < metMapVector->size(); mapId++) {
            cout << "mapId: " << mapId << endl; 
            cout << "SD: " << (*metMapVector)[mapId]->GetDcmHeader()->GetStringValue("SeriesDescription") << endl;
            string outputMap = outputFileRoot; 
            outputMap.append("/");    
            outputMap.append( (*metMapVector)[mapId]->GetDcmHeader()->GetStringValue("SeriesDescription") );    
            writer->SetFileName( outputMap.c_str() ); 
            writer->SetInput( (*metMapVector)[mapId] );
            if ( writer->IsA( "svkIdfVolumeWriter" ) ) {
                svkIdfVolumeWriter::SafeDownCast( writer )->SetCastDoubleToFloat( true ); 
            }
    
            //  Set the input command line into the data set provenance:
            reader->GetOutput()->GetProvenance()->SetApplicationCommand( cmdLine );
    
            writer->Write();
        }

    } else {

        svkMetaboliteMap* quant = svkMetaboliteMap::New();
        quant->SetInput( reader->GetOutput() ); 

        if ( isVerbose ) { 
            quant->SetVerbose( isVerbose ); 
        }    

        if ( onlyQuantifySelectedVolume ) { 
            quant->LimitToSelectedVolume();
        }    

        quant->SetPeakPosPPM( peakCenterPPM );
        quant->SetPeakWidthPPM( peakWidthPPM );
    
        if ( algo == svkMetaboliteMap::INTEGRATE ) {
            quant->SetAlgorithmToIntegrate();
            quant->SetSeriesDescription( qtyName + "area metabolite map" ); 
            quant->Update();
        }
    
        if ( algo == svkMetaboliteMap::PEAK_HT) {
            quant->SetAlgorithmToPeakHeight();
            quant->SetSeriesDescription( qtyName + "peak ht metabolite map" ); 
            quant->Update();
        }

        if ( algo == svkMetaboliteMap::LINE_WIDTH) {
            quant->SetAlgorithmToLineWidth();
            quant->SetSeriesDescription( qtyName + "line width metabolite map" );
            quant->Update();
        }

        if ( algo == svkMetaboliteMap::MAG_INTEGRATE ) {
            quant->SetAlgorithmToMagIntegrate();
            quant->SetSeriesDescription( qtyName + "magnitude area metabolite map" );
            quant->Update();
        }

        if ( algo == svkMetaboliteMap::MAG_PEAK_HT) {
            quant->SetAlgorithmToMagPeakHeight();
            quant->SetSeriesDescription( qtyName + "magnitude peak ht metabolite map" );
            quant->Update();
        }

        if ( algo == svkMetaboliteMap::MAG_LINE_WIDTH) {
            quant->SetAlgorithmToMagLineWidth();
            quant->SetSeriesDescription( qtyName + "magnitude line width metabolite map" );
            quant->Update();
        }
    
        //quant->GetOutput()->GetDcmHeader()->PrintDcmHeader();
        //cout << *( quant->GetOutput() ) << endl;
    
        // ===============================================  
        //  Write the data out to the specified file type.  
        //  Use an svkImageWriterFactory to obtain the
        //  correct writer type. 
        // ===============================================  
        if ( isVerbose == false ) {
    
            vtkSmartPointer< svkImageWriterFactory > writerFactory = vtkSmartPointer< svkImageWriterFactory >::New(); 
            svkImageWriter* writer = static_cast<svkImageWriter*>( writerFactory->CreateImageWriter( dataTypeOut ) ); 
        
            if ( writer == NULL ) {
                cerr << "Can not determine writer of type: " << dataTypeOut << endl;
                exit(1);
            }
    
            writer->SetFileName( outputFileRoot.c_str() );
            writer->SetInput( quant->GetOutput() );
    
            //  Set the input command line into the data set provenance:
            reader->GetOutput()->GetProvenance()->SetApplicationCommand( cmdLine );
    
            writer->Write();
        }
    
        quant->Delete(); 
    }
    reader->Delete();

    return 0; 
}



