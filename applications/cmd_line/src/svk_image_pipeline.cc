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
#include <svkImageAlgorithmPipeline.h>


using namespace svk;


int main (int argc, char** argv)
{

    string usemsg("\n") ; 
    usemsg += "Version " + string(SVK_RELEASE_VERSION) + "\n";   
    usemsg += "svk_xml_image_pipeline -i image_file_name -o output_file_name -c config_file_name \n";
    usemsg += "                       [-v] [-h]                                                  \n";
    usemsg += "                                                                                  \n";
    usemsg += "   -i            image_file_name     The image to be processed                    \n";
    usemsg += "   -o            output_file_name    The name of the output image                 \n";
    usemsg += "   -c            config_file_name    Name of the config file.                     \n";
    usemsg += "   -v                                Verbose output.                              \n";
    usemsg += "   -h                                Print help message.                          \n";
    usemsg += "                                                                                  \n";
    usemsg += "                                                                                  \n";

    string configFileName;
    vector<string> setStrings;
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
    while ((i = getopt_long(argc, argv, "s:c:hv", long_options, &option_index)) != EOF) {
        switch (i) {
            case 's':
                setStrings.push_back(optarg);
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

    cout << "config: <" << configFileName <<  ">" << endl;
    cout << argc << endl;

    /*
     * Check for extra arguments or an empty input file name.
     */
    if ( argc != 0 || configFileName.length() == 0 ) {
        cout << usemsg << endl;
        exit(1);
    }

    if( !svkUtils::FilePathExists( configFileName.c_str() ) ) {
        cerr << "Config file can not be loaded (may not exist) <" << configFileName << ">" << endl;
        exit(1);
    }

    // Replace any variables in the configuration file here..
    string line;
    string xmlFileString;
    ifstream xmlFile (configFileName.c_str());

    // Replace variables in XML
    if (xmlFile.is_open()) {
        while ( getline (xmlFile,line) ) {
            for( int i = 0; i< setStrings.size(); i++ ) {
                std::size_t pos = setStrings[i].find("=");
                string variable = "$";
                variable.append( setStrings[i].substr(0,pos) );
                string value = setStrings[i].substr(pos+1);
                pos = line.find(variable);
                if( pos != string::npos ) {
                    line.replace(pos, variable.size(), value);
                }
            }

            xmlFileString.append( line.c_str() );
        }
        xmlFile.close();
    }
    // Lets start by reading the configuration file
    vtkXMLDataElement* configXML = vtkXMLUtilities::ReadElementFromString( xmlFileString.c_str()  );

    vtkIndent indent;
    configXML->PrintXML(cout, indent);
    svkImageAlgorithmPipeline* pipeline = svkImageAlgorithmPipeline::New();
    pipeline->SetInputPortsFromXML(configXML);
    cout << "Pipeline:" << *pipeline << endl;
    pipeline->Update();
    pipeline->Delete();

    return 0; 
}

