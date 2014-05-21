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

#ifdef WIN32
extern "C" {
#include <getopt.h>
}
#else
#include <getopt.h>
#include <unistd.h>
#endif
#include <svkImageReaderFactory.h>
#include <svkImageReader2.h>
#include <svkImageStatistics.h>
#include <svkImageThreshold.h>
#include <svkUtils.h>
#include <vtkXMLUtilities.h>
#include <svkXMLImagePipeline.h>


using namespace svk;


int main (int argc, char** argv)
{

    string usemsg("\n") ; 
    usemsg += "Version " + string(SVK_RELEASE_VERSION) + "\n";   
    usemsg += "svk_image_threshold -i image_file_name -o output_file_name -c config_file_name \n";
    usemsg += "                [-v] [-h]                                                      \n";
    usemsg += "                                                                               \n";
    usemsg += "   -i            image_file_name     The image to be thresholded               \n";
    usemsg += "   -o            output_file_name    The name of the output image              \n";
    usemsg += "   -c            config_file_name    Name of the config file.                  \n";
    usemsg += "   -v                                Verbose output.                           \n";
    usemsg += "   -h                                Print help message.                       \n";
    usemsg += "                                                                               \n";
    usemsg += "                                                                               \n";

    string configFileName;
    string inputFileName;
    string outputFileName;
    bool   verbose = false;
    static struct option long_options[] =
    {
        {0, 0, 0, 0}
    };

    /*
    *   Process flags and arguments
    */
    int i;
    int option_index = 0; 
    while ((i = getopt_long(argc, argv, "i:c:o:hv", long_options, &option_index)) != EOF) {
        switch (i) {
            case 'i':
                inputFileName.assign(optarg);
                break;
            case 'c':
                configFileName.assign(optarg);
                break;
            case 'o':
                outputFileName.assign(optarg);
                break;
            case 'v':
                verbose = true;
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

    cout << configFileName << endl;
    cout << inputFileName << endl;
    cout << outputFileName << endl;
    cout << argc << endl;

    /*
     * Check for extra arguments or an empty input file name.
     */
    if ( argc != 0 || configFileName.length() == 0 || inputFileName.length() == 0 ||  outputFileName.length() == 0 ) {
        cout << "HERE" << endl;
        cout << usemsg << endl;
        exit(1); 
    }

    if( !svkUtils::FilePathExists( configFileName.c_str() ) ) {
        cerr << "Config file can not be loaded (may not exist) " << configFileName << endl;
        exit(1); 
    }

    // Lets start by reading the configuration file
    vtkIndent indent;
    vtkXMLDataElement* configXML = vtkXMLUtilities::ReadElementFromFile( configFileName.c_str() );
    vtkXMLDataElement* xmlPipelineInputPorts = vtkXMLDataElement::New();
    xmlPipelineInputPorts->SetName("svkXMLImagePipeline");
    svkUtils::CreateNestedXMLDataElement( xmlPipelineInputPorts, "INPUT_IMAGE", inputFileName);

    svkXMLImagePipeline* pipeline = svkXMLImagePipeline::New();
    pipeline->SetInputPortsFromXML(xmlPipelineInputPorts);
    pipeline->SetXMLPipeline(configXML);
    pipeline->Update();

    /*
    svkXMLImageAlgorithm* thresholder = svkImageThreshold::New();
    cout << "Setting input to thresholder..." << endl;
    thresholder->SetInputPortsFromXML( configXML );
    cout << "Thresholder: " << *thresholder << endl;
    thresholder->Update();
    */
    svkIdfVolumeWriter* idfWriter = svkIdfVolumeWriter::New();
    cout << "Setting filename for idfWriter" << endl;
    idfWriter->SetFileName( outputFileName.c_str());
    cout << "Setting input for idfWriter" << endl;
    idfWriter->SetInput( pipeline->GetOutput() );
    cout << "Writting..." << endl;
    idfWriter->Write();
    cout << "Cleanup..." << endl;
    idfWriter->Delete();
    //thresholder->Delete();
    pipeline->Delete();

    return 0; 
}

