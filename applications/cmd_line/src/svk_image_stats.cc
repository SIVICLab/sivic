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
    usemsg += "   -s            var=value           Substitute variable.                         \n";
    usemsg += "   -v                                Verbose output.                              \n";
    usemsg += "   -h                                Print help message.                          \n";
    usemsg += "                                                                                  \n";
    usemsg += "                                                                                  \n";

    string configFileName;
    string fileRootName;
    string outputFileName;
    //BIOPSY_KLUDGE: This is a temporary fix for validating against existing ucsf programs.
    vector<string> biopsies;
    vector<string> xmlVariables;
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
    while ((i = getopt_long(argc, argv, "xto:r:c:b:s:hv", long_options, &option_index)) != EOF) {
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
            case 's':
                xmlVariables.push_back(optarg);
                break;
            case 'c':
                configFileName.assign(optarg);
                break;
            case 'b':
                //BIOPSY_KLUDGE: This is a temporary fix for validating against existing ucsf programs.
                biopsies.push_back(optarg);
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
        vector<string> defaultMeasures;
        defaultMeasures.push_back("median");
        defaultMeasures.push_back("percent25");
        defaultMeasures.push_back("percent75");
        defaultMeasures.push_back("mean");
        defaultMeasures.push_back("sd");
        defaultMeasures.push_back("min");
        defaultMeasures.push_back("max");
        defaultMeasures.push_back("percent10");
        defaultMeasures.push_back("percent90");
        defaultMeasures.push_back("mode");
        defaultMeasures.push_back("skewness");
        defaultMeasures.push_back("kurtosis");
        defaultMeasures.push_back("sum");

        string roiListPath = "svk:pipeline/svkAlgorithm:svkImageStatistics/svkArgument:INPUT_ROI_LIST";
        vtkXMLDataElement* roiListElem = svkUtils::FindNestedElementWithPath(configXML, roiListPath);
        vector<string> defaultROIs;
        for( int i = 0; i < roiListElem->GetNumberOfNestedElements(); i++ ) {
            defaultROIs.push_back(roiListElem->GetNestedElement(i)->GetAttribute("input_id"));
        }

        int numMaps = 0;
        string imageListPath = "svk:pipeline/svkAlgorithm:svkImageStatistics/svkArgument:INPUT_IMAGE_LIST";
        vtkXMLDataElement* imageListElem = svkUtils::FindNestedElementWithPath(configXML, imageListPath);
        vector<string> defaultImages;
        for( int i = 0; i < imageListElem->GetNumberOfNestedElements(); i++ ) {
            string inputImageName = imageListElem->GetNestedElement(i)->GetAttribute("input_id");
            defaultImages.push_back(inputImageName);
            numMaps++;
            if( inputImageName != "recov" &&  inputImageName != "invrec" &&inputImageName != "rf" ) {
                numMaps++; // For normalized version. recov and rf are not normalized
            }
        }

        // Open file to write the measures into
        string outputTabFile = outputFileName;
        outputTabFile.append(".tab");
        ofstream resultsTab;
        resultsTab.open(outputTabFile.c_str());
        resultsTab << "SUMMARY VALUES FROM ANALYSIS OF IMAGE INTENSITIES: " << outputFileName << endl << endl;
        resultsTab << "Number of rois, biopsies and parameter maps: ";
        resultsTab << "     " << defaultROIs.size()-biopsies.size();
        resultsTab << "     " << biopsies.size();
        resultsTab << "     " << numMaps << endl<< endl;
        resultsTab << "Number of tables:   " << defaultMeasures.size();
        string nbins = "?";
        string smooth = "?";
        // Get smooth value
        // Get nbins value
        string nbinsElemPath = "svk:pipeline/svkAlgorithm:svkImageStatistics/svkArgument:NUM_BINS";
        vtkXMLDataElement* nbinsElem = svkUtils::FindNestedElementWithPath(configXML,nbinsElemPath);
        if( nbinsElem != NULL ) {
            nbins = nbinsElem->GetCharacterData();
        } else {
            cout << "ERROR:no svk:pipeline/svkAlgorithm:svkImageStatistics/svkArgument:NUM_BINS element in config file!" << endl;
        }
        string smoothElemPath = "svk:pipeline/svkAlgorithm:svkImageStatistics/svkArgument:SMOOTH_BINS";
        vtkXMLDataElement* smoothElem = svkUtils::FindNestedElementWithPath(configXML,smoothElemPath);
        if( smoothElem != NULL ) {
            smooth = smoothElem->GetCharacterData();
        } else {
            cout << "ERROR:no svk:pipeline/svkAlgorithm:svkImageStatistics/svkArgument:SMOOTH_BINS element in config file!" << endl;
        }
        resultsTab << "  nbin:   " << nbins;
        resultsTab << "  nsmooth:     " << smooth;
        resultsTab << "  ptmin:     " << smooth;
        resultsTab << endl << endl;
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
                            //TODO: COMPATIBILITY MODE?
                            if( measure == "kurtosis" || measure == "skewness" ) {
                                tables[measure][roi][image][1] = measuresElem->GetNestedElement(j)->GetCharacterData();
                            }
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
        //for(measuresIter measures = tables.begin(); measures != tables.end(); measures++) {
        for(int measureIndex = 0; measureIndex < defaultMeasures.size(); measureIndex++) {
            string measure = defaultMeasures[measureIndex];
            resultsTab << "TABLE of " << measure << " values" << endl << endl;
            for(int roiIndex = 0; roiIndex < defaultROIs.size(); roiIndex++) {
                string roi = defaultROIs[roiIndex];
                // Print column headings
                if( roiIndex == 0) {
                    resultsTab.width(9);
                    resultsTab << left << "roi label";
                        resultsTab.width(11);
                        resultsTab << right << "vol(cc)";
                    // Once for unnormalized
                    for(int imageIndex = 0; imageIndex < defaultImages.size(); imageIndex++) {
                        resultsTab.width(11);
                        resultsTab << right << defaultImages[imageIndex];

                    }
                    // Once for normalized
                    for(int imageIndex = 0; imageIndex < defaultImages.size(); imageIndex++) {
                        if( defaultImages[imageIndex] != "recov" && defaultImages[imageIndex] != "invrec" && defaultImages[imageIndex] != "rf"){
                            resultsTab.width(11);
                            string normalizedName = "n";
                            normalizedName.append(defaultImages[imageIndex]);
                            resultsTab << right << normalizedName;
                        }
                    }
                    resultsTab << endl;
                }
                resultsTab.width(9);
                resultsTab << left << roi;
                // Print values, then normalized values
                resultsTab.width(11);
                resultsTab << right << setprecision(2) << fixed << svkTypeUtils::StringToDouble(volumes[roi]);
                for(int imageIndex = 0; imageIndex < defaultImages.size(); imageIndex++) {
                    string image = defaultImages[imageIndex];
                    resultsTab.width(11);
                    float value = 0.0;
                    if(tables.find(measure) != tables.end() && tables[measure].find(roi) != tables[measure].end() ) {
                        value = svkTypeUtils::StringToFloat(tables[measure][roi][image][0]);
                        if(defaultImages[imageIndex] == "rf" && measure != "kurtosis" && measure != "skewness" ){
                            double tmp = 100.0*svkTypeUtils::StringToDouble(tables[measure][roi][image][0]);
                            value = tmp;
                        }

                    }
                    bool addNegative = false;
                    resultsTab << right << setprecision(2);
                    //TODO: Compatibility Mode
                    if( value > 99999 ) {
                        value =  99999;
                    } else if( value < -99999 ) {
                        value =  -99999;
                    }
                    if( value >= 10000000 || value <= -1000000 ) {
                        resultsTab << scientific;
                    } else {
                        resultsTab << fixed;
                    }
                    char buffer [50];
                    sprintf (buffer, "%10.2f",value);
                    resultsTab << buffer;
                }
                for(int imageIndex = 0; imageIndex < defaultImages.size(); imageIndex++) {
                    if( defaultImages[imageIndex] != "recov" && defaultImages[imageIndex] != "invrec" && defaultImages[imageIndex] != "rf"){
                        string image = defaultImages[imageIndex];
                        float value = 0.0;
                        if(tables.find(measure) != tables.end() && tables[measure].find(roi) != tables[measure].end() ) {
                            value = svkTypeUtils::StringToFloat(tables[measure][roi][image][1]);
                        }
                        char buffer [50];
                        // Make sure there is no negative zero.
                        if( value == 0 ) {
                            value = 0;
                        }
                        sprintf (buffer, "%10.2f",value);
                        resultsTab << buffer;
                    }
                }
                resultsTab << endl;
            }
            resultsTab << endl;
        }
        resultsTab << "TABLE of prescale, scale and normalization factors" << endl;
        string normalMeasure = "";
        string normalROI = "";
        string normMethodPath = "svk:pipeline/svkAlgorithm:svkImageStatistics/svkArgument:NORMALIZATION_METHOD";
        vtkXMLDataElement* normMethodElem = svkUtils::FindNestedElementWithPath(configXML, normMethodPath);
        if( normMethodElem != NULL ) {
            int normMethodEnum = svkTypeUtils::StringToInt(normMethodElem->GetCharacterData());
            if( normMethodEnum == svkImageStatistics::MODE ) {
                normalMeasure = "mode";
            } else if ( normMethodEnum == svkImageStatistics::MEAN ) {
                normalMeasure = "mean";
            }
            string normIndexPath = "svk:pipeline/svkAlgorithm:svkImageStatistics/svkArgument:NORMALIZATION_ROI_INDEX";
            vtkXMLDataElement* normIndexElem = svkUtils::FindNestedElementWithPath(configXML, normIndexPath);
            if( normIndexElem != NULL ) {
                int index = svkTypeUtils::StringToInt( normIndexElem->GetCharacterData() );
                string roiListPath = "svk:pipeline/svkAlgorithm:svkImageStatistics/svkArgument:INPUT_ROI_LIST";
                vtkXMLDataElement* roiListElem = svkUtils::FindNestedElementWithPath(configXML, roiListPath);
                if( roiListElem != NULL ) {
                    vtkXMLDataElement* roiNormElem = roiListElem->GetNestedElement(index);
                    if(roiNormElem != NULL ) {
                        normalROI = roiNormElem->GetAttribute("input_id");
                    } else {
                        cout << "ERROR: No ROI found for index" << index << endl;
                    }
                } else {
                    cout << "ERROR: No svkArgument:INPUT_ROI_LIST found" << endl;
                }
            } else {
                cout << "ERROR: No svkArgument:NORMALIZATION_INDEX found" << endl;
            }
        } else if(verbose) {
            cout << "Warning: No svkArgument:NORMALIZATION_METHOD element found" << endl;
        }
        if( !normalMeasure.empty() && !normalROI.empty()) {
            for(int imageIndex = 0; imageIndex < defaultImages.size(); imageIndex++) {
                string image = defaultImages[imageIndex];
                resultsTab << "  " << image;
                resultsTab.width(11);
                // There is no scaling yet so this is always 1.000
                resultsTab << right << "1.000";
                resultsTab.width(11);
                if (defaultImages[imageIndex] == "rf"){
                    resultsTab << right << "0.1000E-01";
                } else if((defaultImages[imageIndex] == "cbv" || defaultImages[imageIndex] == "ph") &&  configFileName.find("nonlin_cbv") != std::string::npos  ){
                    resultsTab << right << "10.00";
                } else { 
                    resultsTab << right << "1.000";
                }
                resultsTab.width(11);
                // This will depend on the config
                if( defaultImages[imageIndex] != "recov" && defaultImages[imageIndex] != "invrec" &&  defaultImages[imageIndex] != "rf"){
                    resultsTab << right << svkTypeUtils::StringToDouble(tables[normalMeasure][normalROI][image][0]) << endl;
                } else {
                    resultsTab << right << "1.00" << endl;
                }

            }
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

