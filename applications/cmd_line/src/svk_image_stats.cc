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
#include <svkImageThreshold.h>
#include <svkUtils.h>
#include <vtkXMLUtilities.h>
#include <svkImageStatistics.h>
#include <svkDouble.h>
#include <svkInt.h>
#include <svkString.h>
#include <vtkAlgorithmOutput.h>


using namespace svk;


int main (int argc, char** argv)
{

    string usemsg("\n") ; 
    usemsg += "Version " + string(SVK_RELEASE_VERSION) + "\n";   
    usemsg += "svk_image_stats -r file_root_name -c config_file_name -o output_file_name    \n";
    usemsg += "                [-v] [-h]                                                    \n";
    usemsg += "                                                                             \n";  
    usemsg += "   -r            file_root_name      The rootname of the input files.        \n";
    usemsg += "   -c            config_file_name    Name of the config file.                \n";
    usemsg += "   -o            output_file_name    Name of the output XML file.            \n";
    usemsg += "   -v                                Verbose output.                         \n";
    usemsg += "   -h                                Print help message.                     \n";
    usemsg += "                                                                             \n";  
    usemsg += "                                                                             \n";  

    string configFileName;
    string fileRootName;
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
    while ((i = getopt_long(argc, argv, "o:r:c:hv", long_options, &option_index)) != EOF) {
        switch (i) {
            case 'r':
                fileRootName.assign(optarg);
                break;
            case 'o':
                outputFileName.assign(optarg);
                break;
            case 'c':
                configFileName.assign(optarg);
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


    /*
     * Check for extra arguments or an empty input file name.
     */
    if ( argc != 0 || configFileName.length() == 0 || fileRootName.length() == 0 || outputFileName.length() == 0) {
        cout << usemsg << endl;
        exit(1); 
    }

    if( ! svkUtils::FilePathExists( configFileName.c_str() ) ) {
        cerr << "Config file can not be loaded (may not exist) " << configFileName << endl;
        exit(1); 
    }

    if( verbose ) {
        cout << "Root name:  " << fileRootName << endl;
        cout << "Config   : "  << configFileName << endl;
        cout << "Output   : "  << outputFileName << endl;
    }


    string rootNameVariable = "ROOTNAME=";
    rootNameVariable.append(fileRootName);
    vector<string> xmlVariables;
    xmlVariables.push_back( rootNameVariable );

    // Lets start by reading the configuration file
    vtkXMLDataElement* configXML = svkUtils::ReadXMLAndSubstituteVariables( configFileName, xmlVariables);


    if( verbose ) {
        vtkIndent indent;
        configXML->PrintXML(cout, indent);
    }
    svkImageAlgorithmPipeline* pipeline = svkImageAlgorithmPipeline::New();
    pipeline->SetInputPortsFromXML(configXML);
    pipeline->Update();
    vtkAlgorithmOutput* xmlResultOutput = pipeline->GetOutputByUniquePortID("xml_results");
    vtkXMLDataElement* results = svkImageStatistics::SafeDownCast(xmlResultOutput->GetProducer())->GetOutput();
    vtkXMLUtilities::WriteElementToFile( results, outputFileName.c_str());
    pipeline->Delete();

    return 0; 
}

