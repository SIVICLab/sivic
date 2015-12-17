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
 *   
 *  
 *
 *  Utility application for converting between supported file formats. 
 *
 */



#include <svkImageReaderFactory.h>
#include <svkImageReader2.h>
#include <svkImageWriterFactory.h>
#include <svkImageWriter.h>
#include <svkImageCopy.h>

//  Insert your algorithm here in place of "AlgoTemplate":
//#include <svkDynamicMRIAlgoTemplate.h>
#include <svkDCEQuantify.h>

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
    usemsg += "Version " + string(SVK_RELEASE_VERSION) + "\n";
    usemsg += "svk_dce_quantify -i input_file_root -o outputfile_root                   \n";
    usemsg += "                 [--end end_time_point]                                  \n";
    usemsg += "                 [ -t output_data_type ] [ -h ]                          \n";
    usemsg += "\n";
    usemsg += "   -i            input_file_root     Root name of DCE file to quantify.  \n";
    // usemsg += "   --start       start_time_point    Optional timepoint to start DCE     \n"; 
    // usemsg += "                                     analysis (defaults to first)        \n";
    usemsg += "   --end         end_time_point      Optional timepoint to end DCE       \n"; 
    usemsg += "                                     analysis (defaults to last)         \n";
    usemsg += "   -o            output_file_root    Name of outputfiles.                \n";
    usemsg += "   -t            output_data_type    Target data type:                   \n";
    usemsg += "                                         3 = UCSF IDF                    \n";
    usemsg += "                                         5 = DICOM_MRI                   \n";
    usemsg += "                                         6 = DICOM_Enhanced MRI          \n";
    usemsg += "   -h                                Print this help mesage.             \n";
    usemsg += "\n";
    usemsg += "Generate DCE maps.                                                       \n";
    usemsg += "\n";


    string inputFileName; 
    string maskFileName;
    string outputFileName = "";
    int    startTimePt    = 1;
    int    endTimePt      = 1;

    svkImageWriterFactory::WriterType dataTypeOut = svkImageWriterFactory::DICOM_ENHANCED_MRI;

    string cmdLine = svkProvenance::GetCommandLineString( argc, argv );

    enum FLAG_NAME {
        // FLAG_START_TP = 0,
        FLAG_END_TP = 0
    };


    static struct option long_options[] =
    {
        /* This option sets a flag. */
        // {"start", required_argument, NULL, FLAG_START_TP},
        {"end", required_argument, NULL, FLAG_END_TP},
        {0, 0, 0, 0}
    };


    // ===============================================  
    //  Process flags and arguments
    // ===============================================  
    int i;
    int option_index = 0;
    while ((i = getopt_long(argc, argv, "i:o:t:h", long_options, &option_index)) != EOF) {
        switch (i) {
            case 'i':
                inputFileName.assign( optarg );
                break;
            // case FLAG_START_TP:
            //     startTimePt = atof(optarg);
            //     break;
            case FLAG_END_TP:
                endTimePt = atof(optarg);
                break;
            case 'o':
                outputFileName.assign(optarg);
                break;
            case 't':
                dataTypeOut = static_cast<svkImageWriterFactory::WriterType>( atoi(optarg) );
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

    //  Validate input: required inputFN, outputFN and appropriate output image data type 
    if (
        argc != 0 ||  inputFileName.length() == 0
            || outputFileName.length() == 0
            || ( 
                   dataTypeOut != svkImageWriterFactory::DICOM_MRI 
                && dataTypeOut != svkImageWriterFactory::IDF 
                && dataTypeOut != svkImageWriterFactory::DICOM_ENHANCED_MRI 
               )
    ) {
        cout << usemsg << endl;
        exit(1); 
    }


    cout << "Input root:  " << inputFileName << endl;
    cout << "Output root: " << outputFileName << endl;


    // ===============================================
    //  Use a reader factory to create a data reader 
    //  of the correct type for the input file format.
    // ===============================================
    svkImageReaderFactory* readerFactory = svkImageReaderFactory::New();
    svkImageReader2* reader     = readerFactory->CreateImageReader2(inputFileName.c_str());
    readerFactory->Delete();

    if (reader == NULL) { 
        cerr << "Can not determine appropriate reader for: " << inputFileName << endl; 
        exit(1);
    }

    //  Read the data to initialize an svkImageData object
    //  If volume files are being read, interpret them as a time series.  This is automatic for DICOM, but must be specified
    //  for IDF where each vol file may represent channel or time. 
    if (reader->IsA("svkIdfVolumeReader") == true) {
        svkIdfVolumeReader::SafeDownCast( reader )->SetMultiVolumeType(svkIdfVolumeReader::TIME_SERIES_DATA);
    }
    reader->SetFileName(inputFileName.c_str());
    reader->Update();


    // Validate start/end timepoints
    int numVols = reader->GetOutput()->GetDcmHeader()->GetNumberOfTimePoints();
    // int numVols = reader->GetOutput()->GetPointData()->GetNumberOfArrays();
    if(startTimePt < 1 || endTimePt < 1) {
        cerr << "Timepoints must be greater than 1" << endl;
        // cerr << "Start timepoint input: " << startTimePt << endl;
        cerr << "End timepoint input:   " << endTimePt << endl;
        exit(1);
    }
    if(startTimePt > endTimePt || endTimePt > numVols) {
        cerr << "Please double check your timepoints" << endl;
        // cerr << "Start timepoint input: " << startTimePt << endl;
        cerr << "End timepoint input:   " << endTimePt << endl;
        cerr << "Number of timepoints in image: " << numVols << endl;
        exit(1);
    }

    // ===============================================  
    //  Initialize DCE Quantify algorithm 
    // ===============================================  
    svkDCEQuantify* dceQuant = svkDCEQuantify::New();
    dceQuant->SetInputConnection(0, reader->GetOutputPort());
    dceQuant->SetTimepointStart(startTimePt);
    if (endTimePt != 1)
    {
        dceQuant->SetTimepointEnd(endTimePt);
    } else {
        dceQuant->SetTimepointEnd(numVols);
    }
    dceQuant->Update();

    // ===============================================  
    //  Use writer factory to create writer for specified
    //  output file format. 
    // ===============================================  
    svkImageWriterFactory* writerFactory = svkImageWriterFactory::New();
    svkImageWriter* baseHtWriter     = static_cast<svkImageWriter*>( writerFactory->CreateImageWriter(dataTypeOut));
    svkImageWriter* peakHtWriter     = static_cast<svkImageWriter*>( writerFactory->CreateImageWriter(dataTypeOut)); 
    svkImageWriter* peakTimeWriter   = static_cast<svkImageWriter*>( writerFactory->CreateImageWriter(dataTypeOut)); 
    svkImageWriter* maxSlopeWriter   = static_cast<svkImageWriter*>( writerFactory->CreateImageWriter(dataTypeOut)); 
    svkImageWriter* washoutWriter    = static_cast<svkImageWriter*>( writerFactory->CreateImageWriter(dataTypeOut));
    svkImageWriter* washoutPosWriter = static_cast<svkImageWriter*>( writerFactory->CreateImageWriter(dataTypeOut));
    writerFactory->Delete();
    
    if (peakHtWriter == NULL || peakTimeWriter == NULL || maxSlopeWriter == NULL) { 
        cerr << "Can not create writer of type: " << dataTypeOut << endl;
        exit(1);
    }
    
    string baseHtFile     = outputFileName;
    string peakHtFile     = outputFileName;  
    string peakTimeFile   = outputFileName;  
    string maxSlopeFile   = outputFileName;  
    string washoutFile    = outputFileName;
    string washoutPosFile = outputFileName;

    baseHtFile.append("_dce_base_ht");
    peakHtFile.append("_dce_peak_ht");  
    peakTimeFile.append("_dce_peak_time");  
    maxSlopeFile.append("_dce_up_slope");  
    washoutFile.append("_dce_washout");
    washoutPosFile.append("_dce_washout_pos");

    baseHtWriter->SetFileName(baseHtFile.c_str());
    peakHtWriter->SetFileName(peakHtFile.c_str());
    peakTimeWriter->SetFileName(peakTimeFile.c_str());
    maxSlopeWriter->SetFileName(maxSlopeFile.c_str());
    washoutWriter->SetFileName(washoutFile.c_str());
    washoutPosWriter->SetFileName(washoutPosFile.c_str());

    // Hack to use ImageCopy to write out integer value dicoms
    svkImageCopy* baseHtCopier = svkImageCopy::New();
    baseHtCopier->SetInput(dceQuant->GetOutput(0));  // port 0 is baseline ma
    baseHtCopier->SetSeriesDescription("DCE Baseline");
    baseHtCopier->SetOutputDataType(svkDcmHeader::UNSIGNED_INT_2);
    baseHtCopier->Update();

    svkImageCopy* peakHtCopier = svkImageCopy::New();
    peakHtCopier->SetInput(dceQuant->GetOutput(1));  // port 1 is peakHt map
    peakHtCopier->SetSeriesDescription("DCE Peak Value");
    peakHtCopier->SetOutputDataType(svkDcmHeader::UNSIGNED_INT_2);
    peakHtCopier->Update();

    svkImageCopy* peakTimeCopier = svkImageCopy::New();
    peakTimeCopier->SetInput(dceQuant->GetOutput(2));  // port 2 is peakTimet map
    peakTimeCopier->SetSeriesDescription("DCE Peak Time");
    peakTimeCopier->SetOutputDataType(svkDcmHeader::UNSIGNED_INT_2);
    peakTimeCopier->Update();

    svkImageCopy* maxSlopeCopier = svkImageCopy::New();
    maxSlopeCopier->SetInput(dceQuant->GetOutput(3));  // port 3 is max slope map 
    maxSlopeCopier->SetSeriesDescription("DCE Slope");
    maxSlopeCopier->SetOutputDataType(svkDcmHeader::UNSIGNED_INT_2);
    maxSlopeCopier->Update();

    svkImageCopy* washoutPosCopier = svkImageCopy::New();
    washoutPosCopier->SetInput(dceQuant->GetOutput(5));  // port 5 is positive washout map
    washoutPosCopier->SetSeriesDescription("DCE Washout");
    washoutPosCopier->SetOutputDataType(svkDcmHeader::UNSIGNED_INT_2);
    washoutPosCopier->Update();


    baseHtWriter->SetInput(baseHtCopier->GetOutput());
    peakHtWriter->SetInput(peakHtCopier->GetOutput());
    peakTimeWriter->SetInput(peakTimeCopier->GetOutput());
    maxSlopeWriter->SetInput(maxSlopeCopier->GetOutput());
    washoutPosWriter->SetInput(washoutPosCopier->GetOutput());
    washoutWriter->SetInput(dceQuant->GetOutput(4));  // port 4 is washout map

    baseHtWriter->Write();
    peakHtWriter->Write();
    peakTimeWriter->Write();
    maxSlopeWriter->Write();
    washoutWriter->Write();
    washoutPosWriter->Write();

    // ===============================================  


    //  clean up:
    dceQuant->Delete(); 
    baseHtWriter->Delete();
    peakHtWriter->Delete();
    peakTimeWriter->Delete();
    maxSlopeWriter->Delete();
    washoutWriter->Delete();
    washoutPosWriter->Delete();
    baseHtCopier->Delete();
    peakHtCopier->Delete();
    peakTimeCopier->Delete();
    maxSlopeCopier->Delete();
    washoutPosCopier->Delete();

    reader->Delete();

    return 0; 
}


