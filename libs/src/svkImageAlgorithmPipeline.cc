/*
 *  Copyright © 2009-2017 The Regents of the University of California.
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
 *
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
 */


#include <svkImageAlgorithmPipeline.h>

using namespace svk;

//vtkCxxRevisionMacro(svkImageAlgorithmPipeline, "$Rev$");
vtkStandardNewMacro(svkImageAlgorithmPipeline);

//! Constructor
svkImageAlgorithmPipeline::svkImageAlgorithmPipeline()
{
    this->SetNumberOfInputPorts(1);
    this->SetNumberOfOutputPorts(0);
    this->GetPortMapper()->InitializeInputPort( PIPELINE, "pipeline", svkAlgorithmPortMapper::SVK_XML);
    this->GetPortMapper()->SetXMLPortPrefix("svk");
}


//! Destructor
svkImageAlgorithmPipeline::~svkImageAlgorithmPipeline()
{
}


/*!
 * Returns the output for a given unique port id.
 */
vtkAlgorithmOutput*  svkImageAlgorithmPipeline::GetOutputByUniquePortID(string uniquePortID)
{
    return this->idToPortMap[uniquePortID];
}


/*!
 *  Creates the algorithm pipeline and executes it.
 */
int svkImageAlgorithmPipeline::RequestData( vtkInformation* request,
                                    vtkInformationVector** inputVector,
                                    vtkInformationVector* outputVector )
{
    vtkXMLDataElement* pipeline = this->GetPortMapper()->GetXMLInputPortValue(PIPELINE)->GetValue();
    if( pipeline != NULL ) {
        int numberOfFilters = pipeline->GetNumberOfNestedElements();
        //cout << "#################################### INITIALIZING ALGORITHMS...#################################### " << endl;
        // Let's initialize the algorithms...
        for( int i = 0; i < numberOfFilters; i++ ) {
            // Get the first algorithm
            vtkXMLDataElement* algorithm = pipeline->GetNestedElement(i);
            if( algorithm != NULL ) {
                this->InitializeAlgorithmForTag( algorithm );
            }
        }
        //cout << "#################################### Setting input connections...#################################### " << endl;
        this->SetInputConnections( pipeline );
        //cout << "#################################### execute pipeline...#################################### " << endl;
        this->ExecutePipeline( pipeline );
    }
    return 1;
}


/*!
 *  This method goes through all the algorithms in the pipeline and sets any input connections from
 *  the outputs stored in the idToPortMap hash.
 */
void svkImageAlgorithmPipeline::SetInputConnections( vtkXMLDataElement* pipeline )
{
        int numberOfFilters = pipeline->GetNumberOfNestedElements();
        // Second loop sets inputs to the algorithms
        for( int i = 0; i < numberOfFilters; i++ ) {
            vtkXMLDataElement* algorithmXML = pipeline->GetNestedElement(i);
            if( algorithmXML != NULL ) {
                vtkAlgorithm* algorithm = this->xmlToAlgoMap[algorithmXML];
                for( int port = 0; port < algorithm->GetNumberOfInputPorts(); port++ ) {
                    string xmlTag;
                    bool isRepeatable = false;
                    svkAlgorithmPortMapper* portMapper = NULL;
                    if( algorithm->IsA("svkImageReader2")) {
                        xmlTag = "svkArgument:FILENAME";
                    } else if( algorithm->IsA("svkImageMathematics")) {
                        portMapper = svkImageMathematics::SafeDownCast(algorithm)->GetPortMapper();
                    } else if( algorithm->IsA("svkImageAlgorithmWithPortMapper")) {
                        portMapper = svkImageAlgorithmWithPortMapper::SafeDownCast(algorithm)->GetPortMapper();
                    } else if( algorithm->IsA("svkGenericAlgorithmWithPortMapper")) {
                        portMapper = svkGenericAlgorithmWithPortMapper::SafeDownCast(algorithm)->GetPortMapper();
                    } else if( algorithm->IsA("svkImageWriter")) {
                        xmlTag = "svkArgument:INPUT_IMAGE";
                    }
                    if( portMapper != NULL ) {
                        xmlTag = portMapper->GetXMLTagForInputPort(port);
                        isRepeatable = portMapper->GetInputPortRepeatable(port);
                        if( isRepeatable ) {
                            xmlTag.append("_LIST");
                        }
                    }

                    vtkXMLDataElement* inputElement = algorithmXML->FindNestedElementWithName(xmlTag.c_str());
                    if( inputElement!= NULL && inputElement->GetAttribute("input_id") != NULL ) {
                        algorithm->SetInputConnection( port, this->idToPortMap[inputElement->GetAttribute("input_id")]);
                    } else if(inputElement!= NULL &&  isRepeatable ) {
                        int numConnections = inputElement->GetNumberOfNestedElements();
                        for( int connection = 0; connection < numConnections; connection++) {
                            if( inputElement->GetNestedElement(connection)->GetAttribute("input_id")) {
                                if( connection == 0 ) {
                                    algorithm->SetInputConnection( port, this->idToPortMap[inputElement->GetNestedElement(connection)->GetAttribute("input_id")]);
                                } else {
                                    algorithm->AddInputConnection( port, this->idToPortMap[inputElement->GetNestedElement(connection)->GetAttribute("input_id")]);
                                }
                            }
                        }
                    }
                }
            }
        }
}


/*!
 *  This method actually goes through and executes the components of the pipeline.
 */
void svkImageAlgorithmPipeline::ExecutePipeline( vtkXMLDataElement* pipeline )
{
        int numberOfFilters = pipeline->GetNumberOfNestedElements();
        // Second loop is to set the inputs and to run the algorithms.
        for( int i = 0; i < numberOfFilters; i++ ) {
            vtkXMLDataElement* algorithmXML = pipeline->GetNestedElement(i);
            if( algorithmXML != NULL ) {
                if(string(algorithmXML->GetName()) == "svkAlgorithm:svkImageWriter") {

                    vtkImageWriter::SafeDownCast(xmlToAlgoMap[algorithmXML])->GetInputConnection(0,0)->GetProducer()->Update();
                    vtkImageWriter::SafeDownCast(xmlToAlgoMap[algorithmXML])->Write();
                } else {
                    xmlToAlgoMap[algorithmXML]->Update();
                }
            }
        }
}


/*!
 * Factory method for getting the algorithms by class name.
 */
void svkImageAlgorithmPipeline::InitializeAlgorithmForTag( vtkXMLDataElement* tag )
{
    //TODO: Create algorithm factory class like svkImageReaderFactory
    svkAlgorithmPortMapper* portMapper = NULL;
    if(string(tag->GetName()) == "svkAlgorithm:svkImageReader") {
        svkImageReaderFactory* readerFactory = svkImageReaderFactory::New();
        vtkXMLDataElement* filenameElement = tag->FindNestedElementWithName("svkArgument:FILENAME");
        vtkXMLDataElement* readIntAsSignedElement = tag->FindNestedElementWithName("svkArgument:READ_INT_AS_SIGNED");
        bool readIntAsSigned = false;
        if( readIntAsSignedElement != NULL ) {
            string readIntAsSignedString = readIntAsSignedElement->GetCharacterData();
            if( readIntAsSignedString == "true"){
                readIntAsSigned = true;
            }
        }
        string filename = filenameElement->GetCharacterData();
        bool filePathExists = svkUtils::FilePathExists(filename.c_str());
        if(!filePathExists) {
            cout << "ERROR: File " << filename.c_str() << " does not exist!" << endl;
        } else {
            svkImageReader2* reader = svkImageReader2::SafeDownCast(readerFactory->CreateImageReader2(filename.c_str()));
            if( reader != NULL ) {
                reader->OnlyReadOneInputFile();
                reader->SetFileName( filename.c_str() );
                if(reader->IsA("svkIdfVolumeReader")) {
                    svkIdfVolumeReader::SafeDownCast(reader)->SetReadIntAsSigned( readIntAsSigned );
                }
                this->xmlToAlgoMap[tag] = reader;
                reader->Register(this);
                vtkXMLDataElement* outputElement = tag->FindNestedElementWithName("svkArgument:OUTPUT");
                // Let's save the output into the idToPortMap hash
                reader->Update();
                reader->GetOutput()->GetDcmHeader()->SetValue("SeriesDescription",outputElement->GetAttribute("output_id"));
                this->idToPortMap[outputElement->GetAttribute("output_id")] = reader->GetOutputPort(0);
                this->idToPortMap[outputElement->GetAttribute("output_id")]->Register(this);
                this->idToPortMap[outputElement->GetAttribute("output_id")];
            }
        }
    } else if (string(tag->GetName()) == "svkAlgorithm:svkImageWriter") {
        vtkXMLDataElement* filenameElement = tag->FindNestedElementWithName("svkArgument:FILENAME");
        string filename = filenameElement->GetCharacterData();
        svkImageWriterFactory* writerFactory = svkImageWriterFactory::New();
        vtkImageWriter* writer = writerFactory->CreateImageWriter(svkImageWriterFactory::GetDefaultWriterForFilePattern(filename));
        writer->SetFileName(filename.c_str());
        this->xmlToAlgoMap[tag] = writer;
        writer->Register(this);
        writerFactory->Delete();
    } else {
        vtkAlgorithm* algorithm = NULL;
        svkAlgorithmPortMapper* portMapper = NULL;
        if( string(tag->GetName()) == "svkAlgorithm:svkImageThreshold") {
            algorithm = svkImageThreshold::New();
            portMapper = svkImageThreshold::SafeDownCast( algorithm )->GetPortMapper();
        } else if( string(tag->GetName()) == "svkAlgorithm:svkImageStatistics") {
            algorithm = svkImageStatistics::New();
            portMapper = svkImageStatistics::SafeDownCast( algorithm )->GetPortMapper();
        } else if( string(tag->GetName()) == "svkAlgorithm:svkImageCopy") {
            algorithm = svkImageCopy::New();
            portMapper = svkImageCopy::SafeDownCast( algorithm )->GetPortMapper();
        } else if( string(tag->GetName()) == "svkAlgorithm:svkImageMathematics") {
            algorithm = svkImageMathematics::New();
            portMapper = svkImageMathematics::SafeDownCast( algorithm )->GetPortMapper();
        } else if( string(tag->GetName()) == "svkAlgorithm:svkDCEQuantify") {
            algorithm = svkDCEQuantify::New();
            portMapper = svkDCEQuantify::SafeDownCast( algorithm )->GetPortMapper();
        } else {
            cout << "ERROR! Filter: " << tag->GetName() << " is not yet supported!" << endl;
        }
        if( algorithm != NULL ) {
            portMapper->SetInputPortsFromXML( tag );
            //cout << portMapper->GetXSD() <<endl;
            // Let's save a pointer to the algorithm initialized for this xml element
            this->xmlToAlgoMap[tag] = algorithm;
            algorithm->Register(this);
            for( int port = 0; port < portMapper->GetNumberOfOutputPorts(); port++ ) {
                string xmlTag = portMapper->GetXMLTagForOutputPort(port);
                vtkXMLDataElement* outputElement = tag->FindNestedElementWithName(xmlTag.c_str());
                if( outputElement != NULL && outputElement->GetAttribute("output_id")!= NULL ) {
                    // Let's save the output into the idToPortMap hash
                    this->idToPortMap[outputElement->GetAttribute("output_id")] = portMapper->GetOutputPort(port);
                    this->idToPortMap[outputElement->GetAttribute("output_id")]->Register(this);
                    if( portMapper->GetInputPortType(port) == svkAlgorithmPortMapper::SVK_MR_IMAGE_DATA ) {
                        svkMriImageData* imageData = svkMriImageData::SafeDownCast(algorithm->GetOutputDataObject(port));
                        if( imageData != NULL ) {
                            svkMriImageData::SafeDownCast(algorithm->GetOutputDataObject(port))->GetDcmHeader()->SetValue("SeriesDescription",outputElement->GetAttribute("output_id"));
                        }
                    }
                }
            }
        }
    }
}
