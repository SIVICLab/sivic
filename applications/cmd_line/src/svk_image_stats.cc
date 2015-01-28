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
#include <svkTypeUtils.h>
#include <vtkXMLUtilities.h>
#include <svkImageStatistics.h>
#include <svkDouble.h>
#include <svkInt.h>
#include <svkString.h>
#include <vtkAlgorithmOutput.h>
#include <vector>
#include <map>


//typedef map<string, string>::iterator measuresIter;
using namespace std;
typedef map<string, map<string, map< string, vector<string> > > >::iterator measuresIter;
typedef map<string, map< string, vector<string> > >::iterator roisIter;
typedef map< string, vector<string> >::iterator imagesIter;
using namespace svk;


int main (int argc, char** argv)
{

    string usemsg("\n") ; 
    usemsg += "Version " + string(SVK_RELEASE_VERSION) + "\n";   
    usemsg += "svk_image_stats -r file_root_name -c config_file_name -o output_file_name         \n";
    usemsg += "                [-v] [-h]                                                         \n";
    usemsg += "                                                                                  \n";
    usemsg += "   -r            file_root_name      The rootname of the input files.             \n";
    usemsg += "   -c            config_file_name    Name of the config file.                     \n";
    usemsg += "   -o            output_file_name    Name of the output file.                     \n";
    usemsg += "   -x                                Output results in XML instead of default CSV.\n";
    usemsg += "   -t                                Output results as tab files.                 \n";
    usemsg += "   -v                                Verbose output.                              \n";
    usemsg += "   -h                                Print help message.                          \n";
    usemsg += "                                                                                  \n";
    usemsg += "                                                                                  \n";

    string configFileName;
    string fileRootName;
    string outputFileName;
    bool outputXML = false;
    bool outputTab = false;
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
    while ((i = getopt_long(argc, argv, "xto:r:c:hv", long_options, &option_index)) != EOF) {
        switch (i) {
            case 'r':
                fileRootName.assign(optarg);
                break;
            case 'o':
                outputFileName.assign(optarg);
                break;
            case 'x':
                outputXML = true;
                break;
            case 't':
                outputTab = true;
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
    vtkIndent indent;
    if( outputXML ) {
        outputFileName.append(".xml");
        vtkXMLUtilities::WriteElementToFile( results, outputFileName.c_str(), &indent);
    } else if(outputTab) {

        // Open file to write the measures into
        string outputTabFile = outputFileName;
        outputTabFile.append(".tab");
        ofstream resultsTab;
        resultsTab.open(outputTabFile.c_str());
        resultsTab << "SUMMARY VALUES FROM ANALYSIS OF IMAGE INTENSITIES:" << fileRootName << endl << endl << endl;
        resultsTab << "Number of rois, biopsies and parameter maps:" << endl;
        //map< map< string, map< string, string > > > tables;
        // First let's get a list of all the statistics computed
        int numberOfTables = 0;
        int numberOfMeasures = 0;
        map< string, map< string, map< string, vector<string> > > > tables;
        map< string, string > volumes;

        // Loops over all results
        for( int i = 0; i < results->GetNumberOfNestedElements(); i++) {
            // For the results loops through the tags in that result
            string roi;
            string image;
            vtkXMLDataElement* measuresElem = NULL;
            for( int j = 0; j < results->GetNestedElement(i)->GetNumberOfNestedElements(); j++) {
                // If we have a measure
                if( string(results->GetNestedElement(i)->GetNestedElement(j)->GetName()) == "measures"){
                    measuresElem = results->GetNestedElement(i)->GetNestedElement(j);
                } else { // else if its not a measures tag
                    string elementName = results->GetNestedElement(i)->GetNestedElement(j)->GetName();
                    if( elementName == "IMAGE" ) {
                        image = results->GetNestedElement(i)->GetNestedElement(j)->GetCharacterData();
                    } else if( elementName == "ROI" ) {
                        roi = results->GetNestedElement(i)->GetNestedElement(j)->GetCharacterData();
                    }
                }
            }
            if( !roi.empty() && !image.empty() && measuresElem != NULL ) {
                for (int j = 0; j < measuresElem->GetNumberOfNestedElements(); j++ ) {
                    string measure = measuresElem->GetNestedElement(j)->GetName();
                    if( measure == "volume") {
                        volumes[roi] = measuresElem->GetNestedElement(j)->GetCharacterData();
                    } else {
                        if( tables[measure].empty() ) { // First time measures has been encounter

                            // CREATE VECTOR
                            vector<string> values;
                            values.push_back("0"); // Non-normalized value
                            values.push_back("0"); // Normalized value
                            // CREATE VECTOR

                            // CREATE IMAGE MAP
                            map< string, vector<string> > imageMap;
                            imageMap[image] = values;
                            // CREATE IMAGE MAP

                            // CREATE ROI MAP
                            map< string, map< string, vector<string> > > roiMap;
                            roiMap[roi] = imageMap;
                            // CREATE ROI MAP

                            // SET INTO TABLES
                            tables[measure] = roiMap;
                            // SET INTO TABLES
                        } else if ( tables[measure][roi].empty() ) { // first time ROI has been encountered

                            // CREATE VECTOR
                            vector<string> values;
                            values.push_back("0"); // Non-normalized value
                            values.push_back("0"); // Normalized value
                            // CREATE VECTOR

                            // CREATE IMAGE MAP
                            map< string, vector<string> > imageMap;
                            imageMap[image] = values;
                            // CREATE IMAGE MAP

                            // SET INTO TABLES
                            tables[measure][roi] = imageMap;
                            // SET INTO TABLES
                        } else if ( tables[measure][roi][image].empty() ) { // first time image has been encounter.

                            // CREATE VECTOR
                            vector<string> values;
                            values.push_back("0"); // Non-normalized value
                            values.push_back("0"); // Normalized value
                            // CREATE VECTOR

                            // SET INTO TABLES
                            tables[measure][roi][image] = values;
                            // SET INTO TABLES
                        }
                        if( measuresElem->GetNestedElement(j)->GetAttribute("normalization") == NULL ) {
                            tables[measure][roi][image][0] = measuresElem->GetNestedElement(j)->GetCharacterData();
                            //cout << measure << " within " << roi << " of " << image << " =" << measuresElem->GetNestedElement(j)->GetCharacterData() << endl;
                        } else {
                            tables[measure][roi][image][1] = measuresElem->GetNestedElement(j)->GetCharacterData();
                            //cout << measure << " within " << roi << " of " << image << " normalized =" << measuresElem->GetNestedElement(j)->GetCharacterData() << endl;
                        }
                    }
                }
            }
        }
        //resultsTab << "Number of tables:   " << numberOfTables << endl;
        //resultsTab << "Number of measures: " << numberOfMeasures << endl;
        for(measuresIter measures = tables.begin(); measures != tables.end(); measures++) {
            resultsTab << "TABLE of " << measures->first << " values" << endl << endl;
            for(roisIter rois = measures->second.begin(); rois != measures->second.end(); rois++) {
                // Print column headings
                if( rois == measures->second.begin()) {
                    resultsTab.width(9);
                    resultsTab << left << "roi label";
                        resultsTab.width(11);
                        resultsTab << right << "vol(cc)";
                    for(imagesIter images = rois->second.begin(); images != rois->second.end(); images++) {
                        resultsTab.width(11);
                        resultsTab << right << images->first;
                    }
                    for(imagesIter images = rois->second.begin(); images != rois->second.end(); images++) {
                        resultsTab.width(11);
                        resultsTab << right << images->first;
                    }
                    resultsTab << endl;
                }
                resultsTab.width(9);
                resultsTab << left << rois->first;
                // Print values, then normalized values
                resultsTab.width(11);
                resultsTab << right << setprecision(2) << fixed << svkTypeUtils::StringToDouble(volumes[rois->first]);
                for(imagesIter images = rois->second.begin(); images != rois->second.end(); images++) {
                    resultsTab.width(11);
                    double value = svkTypeUtils::StringToDouble(images->second[0]);
                    resultsTab << right << setprecision(2);
                    if( value >= 10000000 || value <= -1000000 ) {
                        resultsTab << scientific;
                    } else {
                        resultsTab << fixed;
                    }
                    resultsTab << value;
                    //resultsTab << " " << images->second[0];
                }
                for(imagesIter images = rois->second.begin(); images != rois->second.end(); images++) {
                    resultsTab.width(11);
                    resultsTab << right << setprecision(2) << fixed << svkTypeUtils::StringToDouble(images->second[1]);
                    //resultsTab << " " << images->second[1];
                }
                resultsTab << endl;
            }
            resultsTab << endl << endl;
        }

        resultsTab.close();
    } else {

        // Open file to write the measures into
        string outputCSV = outputFileName;
        outputCSV.append(".csv");
        ofstream resultsCSV;
        resultsCSV.open(outputCSV.c_str());

        // Open file to write the histograms into
        string histogramFilename = outputFileName;
        histogramFilename.append("_hist");
        histogramFilename.append(".csv");
        ofstream histogramCSV;
        histogramCSV.open(histogramFilename.c_str());


        for( int i = 0; i < results->GetNestedElement(0)->GetNumberOfNestedElements(); i++) {
            if( string(results->GetNestedElement(0)->GetNestedElement(i)->GetName()) == "measures"){
                for( int j = 0; j < results->GetNestedElement(0)->GetNestedElement(i)->GetNumberOfNestedElements(); j++) {
                    if( string(results->GetNestedElement(0)->GetNestedElement(i)->GetNestedElement(j)->GetName()) != "histogram") {
                        resultsCSV << results->GetNestedElement(0)->GetNestedElement(i)->GetNestedElement(j)->GetName();
                        if( results->GetNestedElement(0)->GetNestedElement(i)->GetNestedElement(j)->GetAttribute("normalization") != NULL ) {
                            resultsCSV << "Normalized";
                        } else {
                            cout << "Attribute is NULL!" << endl;
                        }
                        resultsCSV << ",";
                    }
                }
            } else {
                resultsCSV << results->GetNestedElement(0)->GetNestedElement(i)->GetName() << ",";
            }
        }
        resultsCSV << endl;
        histogramCSV << "IMAGE,ROI,MIN,MAX,CENTER,COUNT" << endl;
        for( int i = 0; i < results->GetNumberOfNestedElements(); i++) {
            string image = results->GetNestedElement(i)->FindNestedElementWithName("IMAGE")->GetCharacterData();
            string roi = results->GetNestedElement(i)->FindNestedElementWithName("ROI")->GetCharacterData();
            resultsCSV << image << ",";
            resultsCSV << roi << ",";
            for( int j = 0; j < results->GetNestedElement(i)->FindNestedElementWithName("measures")->GetNumberOfNestedElements(); j++) {
                if( string(results->GetNestedElement(i)->FindNestedElementWithName("measures")->GetNestedElement(j)->GetName()) != "histogram" ) {
                    resultsCSV << results->GetNestedElement(i)->FindNestedElementWithName("measures")->GetNestedElement(j)->GetCharacterData() << ",";
                } else {
                    for( int k = 0; k < results->GetNestedElement(i)->FindNestedElementWithName("measures")->GetNestedElement(j)->GetNumberOfNestedElements(); k++) {
                        histogramCSV << image << ",";
                        histogramCSV << roi << ",";
                        double min = svkTypeUtils::StringToDouble(results->GetNestedElement(i)->FindNestedElementWithName("measures")->GetNestedElement(j)->GetNestedElement(k)->GetAttribute("min"));
                        double max = svkTypeUtils::StringToDouble(results->GetNestedElement(i)->FindNestedElementWithName("measures")->GetNestedElement(j)->GetNestedElement(k)->GetAttribute("max"));
                        double center = min + (max-min)/2.0;
                        histogramCSV << min << "," << max << "," << center << ",";
                        histogramCSV << results->GetNestedElement(i)->FindNestedElementWithName("measures")->GetNestedElement(j)->GetNestedElement(k)->GetCharacterData() << endl;
                    }
                }
            }
            resultsCSV << endl;
            histogramCSV << endl;
        }
        resultsCSV.close();
        histogramCSV.close();
    }

    pipeline->Delete();
    return 0; 
}

