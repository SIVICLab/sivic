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


#include </usr/include/vtk/vtkInformation.h>
#include </usr/include/vtk/vtkInformationVector.h>
#include </usr/include/vtk/vtkStreamingDemandDrivenPipeline.h>
#include </usr/include/vtk/vtkXMLDataElement.h>
#include </usr/include/vtk/vtkXMLUtilities.h>

#include <svkImageMathematics.h>
#include <svkQuantifyMetabolites.h>
#include <svkMetaboliteMap.h>
#include <svkImageCopy.h>
#include <svkMetaboliteRatioZScores.h>
#include <svkUtils.h>
#include <svkTypeUtils.h>

#include <time.h>
#include <sys/stat.h>



using namespace svk;


//vtkCxxRevisionMacro(svkQuantifyMetabolites, "$Rev$");
vtkStandardNewMacro(svkQuantifyMetabolites);


/*!
 *
 */
svkQuantifyMetabolites::svkQuantifyMetabolites()
{
#if VTK_DEBUG_ON
    this->DebugOn();
#endif

    vtkDebugMacro(<< this->GetClassName() << "::" << this->GetClassName() << "()");

    this->isVerbose = false; 
    this->xmlFileName = svkQuantifyMetabolites::GetDefaultXMLFileName(); 
    this->mrsXML = NULL;
    this->useSelectedVolumeFraction = 0;
    this->selectedVolumeMask = NULL;
    this->anatomyType = svkTypes::ANATOMY_BRAIN; 

}


/*!
 *
 */
svkQuantifyMetabolites::~svkQuantifyMetabolites()
{
    vtkDebugMacro(<<this->GetClassName()<<"::~"<<this->GetClassName());
}


/* 
 *  Parses the XML quantification parameters and generate set of metabolite maps.  
 */
void svkQuantifyMetabolites::Quantify()
{

    this->GenerateRegionMaps();
    this->GenerateRatioMaps(); 
    this->GenerateZScoreMaps(); 
    //this->mrsXML->PrintXML(cout, vtkIndent()); 

}


/*
 *  Retruns a pointer to the metabolite map vector
 */
std::vector< svkMriImageData* >* svkQuantifyMetabolites::GetMetMaps()
{
    return &(this->metMapVector);        
}


/*
 *
 */
void svkQuantifyMetabolites::GenerateRegionMaps()
{

    this->GetRegionNameVector(); 

    //  
    //  Loop over the regions to quantify:
    //  
    bool parseQuants = true; 
    int quantIndex = 0; 
    vtkXMLDataElement* quantXML;
    vtkXMLDataElement* algoXML;
    svkMetaboliteMap* mapGen = svkMetaboliteMap::New();
    mapGen->SetInputData( this->GetImageDataInput(0) ); 
    if ( isVerbose ) {
        mapGen->SetVerbose(isVerbose); 
    }

    while ( parseQuants ) {
        ostringstream ossIndex;
        ossIndex << quantIndex;
        std::string quantIndexString(ossIndex.str());

        quantXML = this->mrsXML->FindNestedElementWithNameAndId("QUANT", quantIndexString.c_str());

        if (quantXML != NULL ) {

            //cout << "quant id = " << quantIndexString << endl;
        	if( this->GetDebug() ) {
				quantXML->PrintXML( cout, vtkIndent() );
        	}

            //  Get region and algo to quantify.  peak range is defied already in regionVector;  
            //  image to store in a std::vector of svkMriImageData objects. 
            int regionID; 
            quantXML->GetScalarAttribute("region", regionID); 
            algoXML = quantXML->FindNestedElementWithName("ALGO");
            std::string algoName( algoXML->GetAttributeValue( 0 ) );
            algoName = this->ReplaceSlashesAndWhiteSpace( algoName );

            std::string regionName = this->regionVector[regionID][0];  
            float peakPPM  =  GetFloatFromString( this->regionVector[regionID][1] ); 
            float widthPPM =  GetFloatFromString( this->regionVector[regionID][2] ); 

            cout << "NAME      : " << regionName << endl;
            cout << "PEAK POS  : " << peakPPM << endl; 
            cout << "PEAK WIDTH: " << widthPPM << endl; 
            cout << "ALGO      : " << algoName << endl;

            mapGen->SetPeakPosPPM( peakPPM );
            mapGen->SetPeakWidthPPM( widthPPM ); 
            mapGen->SetSeriesDescription( regionName + "_" + algoName );
            mapGen->SetAlgorithm( algoName );

            if ( this->useSelectedVolumeFraction ) { 
                mapGen->LimitToSelectedVolume(); 
            }
            mapGen->Update();

            svkMriImageData* image = svkMriImageData::New();
            image->DeepCopy( mapGen->GetOutput() );

            this->metMapVector.push_back( image );     

        } else {

            parseQuants = false; 

        }

        quantIndex++; 
    }

    //  If masking, grab a copy to use for z-score computation
    if ( this->useSelectedVolumeFraction ) { 
        
        int numVoxels = this->metMapVector[0]->GetDcmHeader()->GetIntValue("Columns"); 
        numVoxels    *= this->metMapVector[0]->GetDcmHeader()->GetIntValue("Rows"); 
        numVoxels    *= this->metMapVector[0]->GetDcmHeader()->GetNumberOfSlices(); 
        this->selectedVolumeMask = new short[numVoxels];
        memcpy( this->selectedVolumeMask, mapGen->GetSelectedVolumeMask(), numVoxels * sizeof(short) );    

    }    
    mapGen->Delete();
 
}


/*
 *
 */
int svkQuantifyMetabolites::GetIntFromString(std::string stringVal ) 
{
 
    istringstream* iss = new istringstream();
    int intVal;
    iss->str( stringVal ); 
    *iss >> intVal;
    delete iss;
    return intVal; 
}


/*
 *
 */
float svkQuantifyMetabolites::GetFloatFromString(std::string stringVal ) 
{
 
    istringstream* iss = new istringstream();
    float floatVal;
    iss->str( stringVal ); 
    *iss >> floatVal;
    delete iss;
    return floatVal; 
}


/*
 *  Generates the ratio maps for the quantities specified in the input xml 
 *  configuration file. 
 *
 *  If the computation is masked, e.g. by selection box, 
 *  then set division by zero to 
 *  zero, otherwise use actual computed value    
 */
void svkQuantifyMetabolites::GenerateRatioMaps()
{

    //  
    //  Loop over the ratios to quantify:
    //  
    bool parseRatios = true; 
    int ratioIndex = 0; 
    vtkXMLDataElement* ratioXML;

    while ( parseRatios ) {

        ostringstream ossIndex;
        ossIndex << ratioIndex;
        std::string ratioIndexString(ossIndex.str());

        ratioXML = this->mrsXML->FindNestedElementWithNameAndId("RATIO", ratioIndexString.c_str());

        if (ratioXML != NULL ) {

            //  Get images contributing to numerator and  denominator for this ratio
             
            //cout << "RXML: " << ratioXML->GetAttributeValue( 1 ) << endl;; 
            std::string ratioName( ratioXML->GetAttributeValue( 1 ) ); 
            ratioName = this->ReplaceSlashesAndWhiteSpace( ratioName ); 
            //cout << "RATIO NAME : " << ratioName << endl;
            
            //  Initialize the ratio with zeros:
            svkImageData* numeratorImage   = svkMriImageData::New();
            svkImageData* denominatorImage = svkMriImageData::New();
            numeratorImage->ZeroCopy( this->metMapVector[0] ); 
            denominatorImage->ZeroCopy( this->metMapVector[0] ); 

            //  Get the numerator and denominator values for this ratio
            //cout << "NI0: " << *numeratorImage << endl;
            this->GetNumeratorAndDenominatorImages( ratioXML, numeratorImage, denominatorImage); 

            svkImageMathematics* mathR = svkImageMathematics::New(); 
            mathR->SetOperationToDivide();

            //  If the computation is masked, e.g. by selection box, 
            //  then set division by zero to 
            //  zero, otherwise use actual computed value    
            if ( this->useSelectedVolumeFraction ) { 
                mathR->SetConstantC(0);
                mathR->DivideByZeroToCOn();
            }
            //cout << "NI: " << *numeratorImage << endl;
            mathR->SetInput1Data( numeratorImage ); 
            mathR->SetInput2Data( denominatorImage ); 
            cout << "============================" << endl;
            cout << "divide images" << endl;
            cout << "============================" << endl;
            mathR->Update();

            //  Create a new image a copy from a correctly 
            //  instantiated object, then copy over the new pixels
            //  from the vtkImageData object
            svkMriImageData* ratioImage = svkMriImageData::New();
            ratioImage->DeepCopy( this->metMapVector[0] );
            ratioImage->DeepCopy( mathR->GetOutput() );

            //  set the header fields
            std::string seriesDescription( ratioName );
            svkDcmHeader* hdr = ratioImage->GetDcmHeader();
            hdr->SetValue("SeriesDescription", ratioName );

            this->metMapVector.push_back( ratioImage ); 

        } else {

            parseRatios = false; 

        }

        ratioIndex++; 
    }

}

/*!
 *
 */
void svkQuantifyMetabolites::SetAnatomyType(svkTypes::AnatomyType anatomyType)
{
    this->anatomyType = anatomyType; 
}


/*
 *  Sums metabolite imgages contributing to numerator and denominator of image ratios or z-scores.   
 */
void svkQuantifyMetabolites::GetNumeratorAndDenominatorImages( vtkXMLDataElement* ratioXML, svkImageData* numeratorImage, svkImageData* denominatorImage)
{

    svkImageMathematics* mathN = svkImageMathematics::New(); 
    svkImageMathematics* mathD = svkImageMathematics::New(); 
    mathN->SetOperationToAdd();
    mathD->SetOperationToAdd();
    mathN->SetInput1Data( numeratorImage );     
    mathD->SetInput1Data( denominatorImage );     

    svkImageData* inputImage = svkMrsImageData::New();
    inputImage->ZeroCopy( this->metMapVector[0] );

    bool elementsExist = true; 
    int elementID = 0; 
    bool denominatorFound = false;
    bool numeratorFound = false;
    while ( elementsExist ) {

        vtkXMLDataElement* nestedXML = ratioXML->GetNestedElement(elementID); 
        if ( nestedXML != NULL ) {

            //  Is it part of numerator or denominator?
            std::string quantType (nestedXML->GetName() ); 
            std::string quantNumString = nestedXML->GetAttributeValue(0);
            istringstream* iss = new istringstream();
            int quantNum;
            iss->str( quantNumString ); 
            *iss >> quantNum;
            delete iss;
            elementID++;    
            if ( quantType.compare("NUMERATOR") == 0 ) {
                mathN->SetInput2Data( this->metMapVector[quantNum] ); 
                mathN->Update();
                numeratorImage->DeepCopy( mathN->GetOutput() ); 
                mathN->SetInput1Data( numeratorImage ); 
                numeratorFound = true;
            } else if ( quantType.compare("DENOMINATOR") == 0 ) {
                mathD->SetInput2Data( this->metMapVector[quantNum] ); 
                mathD->Update();
                denominatorImage->DeepCopy( mathD->GetOutput() ); 
                mathD->SetInput1Data( denominatorImage ); 
                denominatorFound = true;
            }
            
        } else {

            elementsExist = false; 
        }
    }

    // If the numerator or denominator was not found, set that image to have value 1
    if( !denominatorFound && svkMriImageData::SafeDownCast(denominatorImage) ) {
    	svkMriImageData* imageOnes = svkMriImageData::New();
    	imageOnes->CopyAndFillComponents(denominatorImage, 1);
    	denominatorImage->ShallowCopy(imageOnes);
    	imageOnes->Delete();
    	imageOnes = NULL;
    }
    if( !numeratorFound && svkMriImageData::SafeDownCast(numeratorImage) ) {
    	svkMriImageData* imageOnes = svkMriImageData::New();
    	imageOnes->CopyAndFillComponents(numeratorImage, 1);
    	numeratorImage->ShallowCopy(imageOnes);
    	imageOnes->Delete();
    	imageOnes = NULL;
    }

    mathN->Delete();
    mathD->Delete();
}


/*
 *  Generates z-score maps (metabolite indices) for the quantities specified in 
 *  the input xml configuration file.  
 */
void svkQuantifyMetabolites::GenerateZScoreMaps()
{

    //  
    //  Loop over the zscores to quantify (while loop):
    //  
    bool parseZScores= true; 
    int zscoreIndex = 0; 
    vtkXMLDataElement* zscoreXML;

    svkMetaboliteRatioZScores* zscore = svkMetaboliteRatioZScores::New(); 

    while ( parseZScores) {

        ostringstream ossIndex;
        ossIndex << zscoreIndex;
        std::string zscoreIndexString(ossIndex.str());

        zscoreXML = this->mrsXML->FindNestedElementWithNameAndId("ZSCORE", zscoreIndexString.c_str());

        if (zscoreXML != NULL ) {

            //  Get images contributing to numerator and  denominator for this index 
             
            //cout << "ZSXML: " << zscoreXML->GetAttributeValue( 1 ) << endl;; 
            std::string zscoreName( zscoreXML->GetAttributeValue( 1 ) ); 
            zscoreName = this->ReplaceSlashesAndWhiteSpace( zscoreName ); 
            //cout << "ZSCORE NAME : " << zscoreName << endl;
            
            //  Initialize the z-score maps with zeros:
            svkImageData* numeratorImage   = svkMriImageData::New();
            svkImageData* denominatorImage = svkMriImageData::New();
            numeratorImage->ZeroCopy( this->metMapVector[0] ); 
            denominatorImage->ZeroCopy( this->metMapVector[0] ); 

            //  Get the numerator and denominator images for 
            //  this ratio:
            this->GetNumeratorAndDenominatorImages( zscoreXML, numeratorImage, denominatorImage); 

            //  Now generate the z-score form the 
            //  numerator/denominator images
            zscore->SetInputNumerator( numeratorImage ); 
            zscore->SetInputDenominator( denominatorImage ); 
            zscore->SetInputMrsData( this->GetImageDataInput(0) ); 
            if ( this->useSelectedVolumeFraction ) { 
                zscore->LimitToSelectedVolume( this->selectedVolumeMask ); 
            }
            zscore->Update();

            //  Create a new image a copy from a correctly 
            //  instantiated object, then copy over the new pixels
            //  from the vtkImageData object
            svkMriImageData* zscoreImage = svkMriImageData::New();
            zscoreImage->DeepCopy( zscore->GetOutput() );

            //  set the header fields
            std::string seriesDescription( zscoreName );
            svkDcmHeader* hdr = zscoreImage->GetDcmHeader();
            hdr->SetValue("SeriesDescription", zscoreName );

            this->metMapVector.push_back( zscoreImage ); 

        } else {

            parseZScores = false; 

        }

        zscoreIndex++; 
    }

    zscore->Delete(); 

}


/*
 *  Limits the calculation to voxels that have at least the specified fractional
 *  volume within the selected MRS volume. The default is to include all voxels
 *  in the calculation (fraction = 0).
 */
void svkQuantifyMetabolites::LimitToSelectedVolume(float fraction)
{
    this->useSelectedVolumeFraction = fraction;
    this->Modified();
}


/*!
 *  Resets the origin and extent for correct initialization of output svkMriImageData object from input 
 *  svkMrsImageData object. 
 */
int svkQuantifyMetabolites::RequestInformation( vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector )
{
    return 1;
}


/*!
 *  Copy the Dcm Header and Provenance from the input to the output. 
 */
int svkQuantifyMetabolites::RequestData( vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector )
{

    this->metMapVector.clear();
    this->Quantify();

    return 1; 
};


/*
 *  Replace ('/') slash character and white space in name attributes with "_div_" 
 *  and "_" respectively so we end up with valid file names. 
 */
string svkQuantifyMetabolites::ReplaceSlashesAndWhiteSpace( std::string inString)
{
   
    std::string outString( inString ) ;

    size_t slashPos;
    slashPos = outString.find_first_of("/");

    while ( slashPos != std::string::npos ) {
        outString.assign( outString.replace(slashPos, 1, "_div_") );
        slashPos = outString.find_first_of("/");
    }

    
    size_t whitePos;
    whitePos = outString.find_first_of(" ");

    while ( whitePos != std::string::npos ) {
        outString.assign( outString.replace(whitePos, 1, "_") );
        whitePos = outString.find_first_of(" ");
    }

    return outString; 

}


/*!
 *  Returns an stl map containing the region name, with a vector of 2 floats 
 *  that store the region peak ppm and width in ppm.  All info parsed from the 
 *  xml config file.
 */
std::vector< std::vector< std::string > >  svkQuantifyMetabolites::GetRegionNameVector()
{

    this->regionVector.clear();

    //
    //  Loop over the regions to quantify:
    //
    bool parseRegions = true;
    int regionIndex = 0;

    if ( this->mrsXML == NULL ) {

        this->mrsXML = vtkXMLUtilities::ReadElementFromFile( this->xmlFileName.c_str() );
		if( this->GetDebug() ) {
			this->mrsXML->PrintXML(cout, vtkIndent());
		}

        //  =========================================
        //  Search for the application specific sub-element 
        //  (i.e. 1H Brain, 13C prostate, etc.)
        //  =========================================
        int numApplicationElements = this->mrsXML->GetNumberOfNestedElements();     
        vtkXMLDataElement* applicationElement = NULL; 
        vtkXMLDataElement* bestApplication = NULL;

        //  =========================================
        //  Check for valid match between "ResonantNucleus" from the data 
        //  header and nucleus in Application element
        //  =========================================
        string headerNucleus = "13C"; 
        bool supportedHeaderNucleus = false; 
        if ( this->GetImageDataInput(0) != NULL ) { 
            headerNucleus = this->GetImageDataInput(0)->GetDcmHeader()->GetStringValue("ResonantNucleus"); 
        } 
        for (int i = 0; i < numApplicationElements; i++ ) {
            applicationElement = this->mrsXML->GetNestedElement(i); 
            string applicationNucleus = applicationElement->GetAttribute("nucleus"); 
            if( applicationNucleus.compare(headerNucleus) == 0 ) {
                supportedHeaderNucleus = true; 
                break; 
            }
        }
        if ( !supportedHeaderNucleus ) {
            headerNucleus.assign("DEFAULT"); 
        }


        //  Now find the correct application element: 
        for (int i = 0; i < numApplicationElements; i++ ) {

            //  Anatomy 
            string anatomy = svkTypes::GetAnatomyTypeString( this->anatomyType);

            applicationElement = this->mrsXML->GetNestedElement(i); 
            string applicationNucleus = applicationElement->GetAttribute("nucleus"); 
            string applicationAnatomy = "";
            const char* anatomyCharArray = applicationElement->GetAttribute("anatomy");
            if( anatomyCharArray != NULL ) {
                applicationAnatomy = applicationElement->GetAttribute("anatomy") ;

            }
            if( applicationNucleus.compare(headerNucleus) == 0 ) {
               // Check if the anatomy matches OR if the anatomy is blank meaning its the default case
                if ( applicationAnatomy.empty() || applicationAnatomy.compare(anatomy) == 0 ) {
                    bestApplication = applicationElement;
                    if( this->GetDebug() ) {
                        cout << "TARGET: " << anatomy << " " << headerNucleus << endl;
                        bestApplication->PrintXML(cout, vtkIndent());
                    }
                    // If this was the default case (anatomy is NULL) keep searching for a specific anatomy, otherwise break.
                    if( !applicationAnatomy.empty() ) {
                        break;
                    }
                }
            }
        }
        this->mrsXML = bestApplication;

    } 

    vtkXMLDataElement* regionXML;

    while ( parseRegions ) {

        ostringstream ossIndex;
        ossIndex << regionIndex;
        std::string regionIndexString(ossIndex.str());

        regionXML = this->mrsXML->FindNestedElementWithNameAndId("REGION", regionIndexString.c_str());

        if (regionXML != NULL ) {

            std::string regionName( regionXML->GetAttributeValue( 1 ) );
            regionName = this->ReplaceSlashesAndWhiteSpace( regionName );
            string peakPPM( regionXML->GetAttribute("peak_ppm") );
            string widthPPM( regionXML->GetAttribute("width_ppm") );

            //cout << "XX peak pos  : " << peakPPM << endl;
            //cout << "XX peak width: " << widthPPM << endl;

            std::vector < std::string > peak;
            peak.push_back(regionName); 
            peak.push_back(peakPPM); 
            peak.push_back(widthPPM); 

            this->regionVector.push_back(peak);  

        } else {
            parseRegions = false; 

        }
        regionIndex++; 
    }

    return this->regionVector; 
}


/*!
 *  Modify a region's peak and width. 
 */
void svkQuantifyMetabolites::ModifyRegion( int regionID, float peakPPM, float widthPPM )
{

    //  If necessary, read xml from file before modifying content:
    if ( this->mrsXML == NULL ) {
        this->mrsXML = vtkXMLUtilities::ReadElementFromFile( this->xmlFileName.c_str() );
        if( this->GetDebug() ) {
			this->mrsXML->PrintXML(cout, vtkIndent());
        }
        //  Get the application specific sub-element (i.e. 1H Brain, 13C prostate, etc.)
        string nucleus = "13C"; 
        if ( this->GetImageDataInput(0) != NULL ) { 
            nucleus = this->GetImageDataInput(0)->GetDcmHeader()->GetStringValue("ResonantNucleus"); 
        } 
        if ( nucleus.compare( "13C" ) == 0 ) {
            this->mrsXML = this->mrsXML->FindNestedElementWithNameAndAttribute("APPLICATION", "nucleus", "13C");
        } else {
            this->mrsXML = this->mrsXML->FindNestedElementWithNameAndAttribute("APPLICATION", "nucleus", "1H");
        }
        if( this->GetDebug() ) {
			this->mrsXML->PrintXML(cout, vtkIndent());
        }
    }

    vtkXMLDataElement* regionXML;

    ostringstream ossIndex;
    ossIndex << regionID;
    std::string regionIndexString(ossIndex.str());

    regionXML = this->mrsXML->FindNestedElementWithNameAndId("REGION", regionIndexString.c_str());

    if (regionXML != NULL ) {

        //  Modify regionXML entry: 
        regionXML->SetFloatAttribute("peak_ppm", peakPPM);
        regionXML->SetFloatAttribute("width_ppm", widthPPM);

    } else {
        vtkWarningWithObjectMacro(this, "Could not find region " + regionIndexString + " in XML config");
    }

    //regionXML->PrintXML(cout, vtkIndent()); 
    //this->mrsXML->PrintXML(cout, vtkIndent()); 

}


/*!
 *  set the path/name to xml file.   
 */
void svkQuantifyMetabolites::SetXMLFileName( string xmlFileName )
{
    this->xmlFileName = xmlFileName;  
    // Now we have remove the old xml file
    this->ClearXMLFile();
}


/*! Sets the current XML data to NULL
 *  so the file will be re-read.
 */
void  svkQuantifyMetabolites::ClearXMLFile( )
{
    if( this->mrsXML != NULL ) {
        this->mrsXML->Delete();
        this->mrsXML = NULL;
    }

}


/*!
 *  Get the path to the current XML file
 */
string svkQuantifyMetabolites::GetXMLFileName( )
{
    return this->xmlFileName;
}


/*!
 *
 */
void svkQuantifyMetabolites::UpdateProvenance()
{
    vtkDebugMacro(<<this->GetClassName()<<"::UpdateProvenance()");
}


/*!
 *  Write the integrals for each voxel to stdout. Default is false.  
 */
void svkQuantifyMetabolites::SetVerbose( bool isVerbose )
{
    this->isVerbose = isVerbose; 
}


/*!
 *
 */
int svkQuantifyMetabolites::FillInputPortInformation( int vtkNotUsed(port), vtkInformation* info )
{
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "svkMrsImageData");
    return 1;
}


/*!
 *  Output from this algo is an svkMriImageData object. 
 */
int svkQuantifyMetabolites::FillOutputPortInformation( int vtkNotUsed(port), vtkInformation* info )
{
    info->Set( vtkDataObject::DATA_TYPE_NAME(), "svkMriImageData"); 
    return 1;
}


/*!
 *  gets the default xml quantification template filename 
 *  $HOME/.SIVICQuant.xml
 */
string svkQuantifyMetabolites::GetDefaultXMLFileName()
{
    string fileName = svkUtils::GetHomeDirectory(); 
    fileName.append("/.SIVICQuantrc.xml" );
    return fileName;
}


/*!
 *  Print out a template xml config file:  
 *  if fileName is "", then use default filename ( $HOME/.SIVICQuant.xml)
 *  if clober is false, will not overwrite existing file.
 */
void svkQuantifyMetabolites::WriteDefaultXMLTemplate( string fileName, bool clobber )
{

    //  if no filename, use default
    if ( fileName.size() == 0 ) { 
        fileName =  svkQuantifyMetabolites::GetDefaultXMLFileName(); 
    }

    //  first check if the file exists (returns 0):
    struct stat buf;
    bool fileExists; 
    if (stat(fileName.c_str(), &buf) == 0) {
        fileExists = true;
    } else {
        fileExists = false;
    }

    //  if the file doesn't exist, or clobber is true then create it
    if ( ( fileExists && clobber == true ) || fileExists == false ) {

        ofstream xmlOut( fileName.c_str() );
        if( !xmlOut ) {
            throw runtime_error("Cannot open xml file for writing: " + fileName );
        }

        xmlOut << ""
        << "<!-- " << endl
        << "   Copyright © 2009-2017 The Regents of the University of California." << endl
        << "   All Rights Reserved." << endl
        << " " << endl
        << "   Redistribution and use in source and binary forms, with or without" << endl
        << "   modification, are permitted provided that the following conditions are met:" << endl
        << "   •   Redistributions of source code must retain the above copyright notice," << endl
        << "       this list of conditions and the following disclaimer." << endl
        << "   •   Redistributions in binary form must reproduce the above copyright notice," << endl
        << "       this list of conditions and the following disclaimer in the documentation" << endl
        << "       and/or other materials provided with the distribution." << endl
        << "   •   None of the names of any campus of the University of California, the name" << endl
        << "       \"The Regents of the University of California,\" or the names of any of its" << endl
        << "       contributors may be used to endorse or promote products derived from this" << endl
        << "       software without specific prior written permission." << endl
        << " " << endl
        << "   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS \"AS IS\" AND" << endl
        << "   ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED" << endl
        << "   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED." << endl
        << "   IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT," << endl
        << "   INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT" << endl
        << "   NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR" << endl
        << "   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY," << endl
        << "   WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)" << endl
        << "   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY" << endl
        << "   OF SUCH DAMAGE." << endl
        << " " << endl
        << " " << endl
        << "  $URL$" << endl
        << "  $Rev$" << endl
        << "   $Author$" << endl
        << "   $Date$" << endl
        << " " << endl
        << "   Authors:" << endl
        << "       Jason C. Crane, Ph.D." << endl
        << "       Beck Olson" << endl
        << "-->" << endl
        << " "  << endl
        << " <SVK_MRS_QUANTIFICATION version=\""<<  string(SVK_RELEASE_VERSION) <<"\">" << endl
        << " " << endl
        << " -- 1H Brain " << endl
        << "  <APPLICATION nucleus=\"1H\" anatomy=\"brain\"> " << endl
        << " " << endl
        << "     <REGION id=\"0\" name=\"CHOLINE\"   peak_ppm=\"3.1455\" width_ppm=\".1758\">" << endl
        << "     </REGION>" << endl
        << "     <REGION id=\"1\" name=\"CREATINE\"  peak_ppm=\"3.06\"  width_ppm=\".18\">" << endl
        << "     </REGION>" << endl
        << "     <REGION id=\"2\" name=\"NAA\"       peak_ppm=\"1.9833\"  width_ppm=\".13\">" << endl
        << "     </REGION>" << endl
        << "     <REGION id=\"3\" name=\"LIPID_LAC\" peak_ppm=\"1.36\"  width_ppm=\".17\">" << endl
        << "     </REGION>" << endl
        << " " << endl
        << "     -- cho peak ht" << endl
        << "     <QUANT id=\"0\" region=\"0\">" << endl
        << "         <ALGO name=\"PEAK_HT\">" << endl
        << "         </ALGO>" << endl
        << "     </QUANT>" << endl
        << " " << endl
        << "     -- creatine peak ht" << endl
        << "     <QUANT id=\"1\" region=\"1\">" << endl
        << "         <ALGO name=\"PEAK_HT\">" << endl
        << "         </ALGO>" << endl
        << "     </QUANT>" << endl
        << " " << endl
        << "     -- naa peak ht" << endl
        << "     <QUANT id=\"2\" region=\"2\">" << endl
        << "         <ALGO name=\"PEAK_HT\">" << endl
        << "         </ALGO>" << endl
        << "     </QUANT>" << endl
        << " " << endl
        << "     -- lip/lac peak ht " << endl
        << "     <QUANT id=\"3\" region=\"3\">" << endl
        << "         <ALGO name=\"PEAK_HT\">" << endl
        << "         </ALGO>" << endl
        << "     </QUANT>" << endl
        << " " << endl
        << "     -- cho integrated area" << endl
        << "     <QUANT id=\"4\" region=\"0\">" << endl
        << "         <ALGO name=\"INTEGRATE\">" << endl
        << "         </ALGO>" << endl
        << "     </QUANT>" << endl
        << " " << endl
        << "     -- creatine integrated area" << endl
        << "     <QUANT id=\"5\" region=\"1\">" << endl
        << "         <ALGO name=\"INTEGRATE\">" << endl
        << "         </ALGO>" << endl
        << "     </QUANT>" << endl
        << " " << endl
        << "     -- NAAintegrated area" << endl
        << "     <QUANT id=\"6\" region=\"2\">" << endl
        << "         <ALGO name=\"INTEGRATE\">" << endl
        << "         </ALGO>" << endl
        << "     </QUANT>" << endl
        << " " << endl
        << "     -- lip/lac integrated area " << endl
        << "     <QUANT id=\"7\" region=\"3\">" << endl
        << "         <ALGO name=\"INTEGRATE\">" << endl
        << "         </ALGO>" << endl
        << "     </QUANT>" << endl
        << " " << endl
        << "     -- ratio of choline to naa peak ht" << endl
        << "     <RATIO id=\"0\" name=\"CHO/NAA_PEAK_HT\">" << endl
        << "         <NUMERATOR quant_id=\"0\">" << endl
        << "         </NUMERATOR>" << endl
        << "         <DENOMINATOR quant_id=\"2\">" << endl
        << "         </DENOMINATOR>" << endl
        << "     </RATIO>" << endl
        << " " << endl
        << "     -- ratio of choline + creatine to naa peak ht" << endl
        << "     <RATIO id=\"2\" name=\"CHO+CRE/NAA INTEGRATE\">" << endl
        << "         <NUMERATOR quant_id=\"4\">" << endl
        << "         </NUMERATOR>" << endl
        << "         <NUMERATOR quant_id=\"5\">" << endl
        << "         </NUMERATOR>" << endl
        << "         <DENOMINATOR quant_id=\"6\">" << endl
        << "         </DENOMINATOR>" << endl
        << "     </RATIO>" << endl
        << " " << endl
        << "     -- choline/naa peak ht index (z-score)" << endl
        << "     <ZSCORE id=\"0\" name=\"CNI PEAK HT\">" << endl
        << "         <NUMERATOR quant_id=\"0\">" << endl
        << "         </NUMERATOR>" << endl
        << "         <DENOMINATOR quant_id=\"2\">" << endl
        << "         </DENOMINATOR>" << endl
        << "     </ZSCORE>" << endl
        << " " << endl
        << "  </APPLICATION> " << endl
        << " " << endl
        << " " << endl
        << " -- 1H Prostate  " << endl
        << "  <APPLICATION nucleus=\"1H\" anatomy=\"prostate\"> " << endl
        << " " << endl
        << "     <REGION id=\"0\" name=\"CHOLINE\"   peak_ppm=\"3.1455\" width_ppm=\".1758\">" << endl
        << "     </REGION>" << endl
        << "     <REGION id=\"1\" name=\"CREATINE\"  peak_ppm=\"3.06\"  width_ppm=\".18\">" << endl
        << "     </REGION>" << endl
        << "     <REGION id=\"2\" name=\"CITRATE\"   peak_ppm=\"1.9833\"  width_ppm=\".13\">" << endl
        << "     </REGION>" << endl
        << " " << endl
        << "     -- cho peak ht" << endl
        << "     <QUANT id=\"0\" region=\"0\">" << endl
        << "         <ALGO name=\"PEAK_HT\">" << endl
        << "         </ALGO>" << endl
        << "     </QUANT>" << endl
        << " " << endl
        << "     -- creatine peak ht" << endl
        << "     <QUANT id=\"1\" region=\"1\">" << endl
        << "         <ALGO name=\"PEAK_HT\">" << endl
        << "         </ALGO>" << endl
        << "     </QUANT>" << endl
        << " " << endl
        << "     -- cit peak ht" << endl
        << "     <QUANT id=\"2\" region=\"2\">" << endl
        << "         <ALGO name=\"PEAK_HT\">" << endl
        << "         </ALGO>" << endl
        << "     </QUANT>" << endl
        << " " << endl
        << "     -- cho integrated area" << endl
        << "     <QUANT id=\"3\" region=\"0\">" << endl
        << "         <ALGO name=\"INTEGRATE\">" << endl
        << "         </ALGO>" << endl
        << "     </QUANT>" << endl
        << " " << endl
        << "     -- creatine integrated area" << endl
        << "     <QUANT id=\"4\" region=\"1\">" << endl
        << "         <ALGO name=\"INTEGRATE\">" << endl
        << "         </ALGO>" << endl
        << "     </QUANT>" << endl
        << " " << endl
        << "     -- citrate integrated area" << endl
        << "     <QUANT id=\"5\" region=\"2\">" << endl
        << "         <ALGO name=\"INTEGRATE\">" << endl
        << "         </ALGO>" << endl
        << "     </QUANT>" << endl
        << " " << endl
        << "     -- ratio of choline to cit peak ht" << endl
        << "     <RATIO id=\"0\" name=\"CHO/CIT_PEAK_HT\">" << endl
        << "         <NUMERATOR quant_id=\"0\">" << endl
        << "         </NUMERATOR>" << endl
        << "         <DENOMINATOR quant_id=\"2\">" << endl
        << "         </DENOMINATOR>" << endl
        << "     </RATIO>" << endl
        << " " << endl
        << "     -- ratio of choline + creatine to cit peak ht" << endl
        << "     <RATIO id=\"1\" name=\"CHO+CRE/CIT INTEGRATE\">" << endl
        << "         <NUMERATOR quant_id=\"4\">" << endl
        << "         </NUMERATOR>" << endl
        << "         <NUMERATOR quant_id=\"5\">" << endl
        << "         </NUMERATOR>" << endl
        << "         <DENOMINATOR quant_id=\"6\">" << endl
        << "         </DENOMINATOR>" << endl
        << "     </RATIO>" << endl
        << " " << endl
        << "  </APPLICATION> " << endl
        << " " << endl
        << " " << endl
        << " -- 13C Prostate  " << endl
        << "  <APPLICATION nucleus=\"13C\">      " << endl
        << " " << endl
        << "     <REGION id=\"0\" name=\"LACTATE\"  peak_ppm=\"186\"  width_ppm=\"3\"> " << endl
        << "     </REGION> " << endl
        << "     <REGION id=\"1\" name=\"ALANINE\"  peak_ppm=\"178\"  width_ppm=\"2\"> " << endl
        << "     </REGION> " << endl
        << "     <REGION id=\"2\" name=\"PYRUVATE\" peak_ppm=\"173\"  width_ppm=\"2\"> " << endl
        << "     </REGION> " << endl
        << "     <REGION id=\"3\" name=\"UREA\"     peak_ppm=\"164\"  width_ppm=\"2\"> " << endl
        << "     </REGION> " << endl
        << " " << endl
        << "     -- lac peak ht " << endl
        << "     <QUANT id=\"0\" region=\"0\"> " << endl
        << "         <ALGO name=\"PEAK_HT\"> " << endl
        << "         </ALGO> " << endl
        << "     </QUANT> " << endl
        << " " << endl
        << "     -- ala peak ht " << endl
        << "     <QUANT id=\"1\" region=\"1\"> " << endl
        << "         <ALGO name=\"PEAK_HT\"> " << endl
        << "         </ALGO> " << endl
        << "     </QUANT> " << endl
        << " " << endl
        << "     -- pyr peak ht " << endl
        << "     <QUANT id=\"2\" region=\"2\"> " << endl
        << "         <ALGO name=\"PEAK_HT\"> " << endl
        << "         </ALGO> " << endl
        << "     </QUANT> " << endl
        << " " << endl
        << "     -- urea peak ht  " << endl
        << "     <QUANT id=\"3\" region=\"3\"> " << endl
        << "         <ALGO name=\"PEAK_HT\"> " << endl
        << "         </ALGO> " << endl
        << "     </QUANT> " << endl
        << " " << endl
        << "     -- lac integrated area " << endl
        << "     <QUANT id=\"4\" region=\"0\"> " << endl
        << "         <ALGO name=\"INTEGRATE\"> " << endl
        << "         </ALGO> " << endl
        << "     </QUANT> " << endl
        << " " << endl
        << "     -- ala integrated area " << endl
        << "     <QUANT id=\"5\" region=\"1\"> " << endl
        << "         <ALGO name=\"INTEGRATE\"> " << endl
        << "         </ALGO> " << endl
        << "     </QUANT> " << endl
        << " " << endl
        << "     -- pyr integrated area " << endl
        << "     <QUANT id=\"6\" region=\"2\"> " << endl
        << "         <ALGO name=\"INTEGRATE\"> " << endl
        << "         </ALGO> " << endl
        << "     </QUANT> " << endl
        << " " << endl
        << "     -- urea integrated area  " << endl
        << "     <QUANT id=\"7\" region=\"3\"> " << endl
        << "         <ALGO name=\"INTEGRATE\"> " << endl
        << "         </ALGO> " << endl
        << "     </QUANT> " << endl
        << " " << endl
        << "     -- lac magnitude  " << endl
        << "     <QUANT id=\"8\" region=\"0\"> " << endl
        << "         <ALGO name=\"MAG_PEAK_HT\"> " << endl
        << "         </ALGO> " << endl
        << "     </QUANT> " << endl
        << " " << endl
        << "     -- ala magnitude  " << endl
        << "     <QUANT id=\"9\" region=\"1\"> " << endl
        << "         <ALGO name=\"MAG_PEAK_HT\"> " << endl
        << "         </ALGO> " << endl
        << "     </QUANT> " << endl
        << " " << endl
        << "     -- pyr magnitude  " << endl
        << "     <QUANT id=\"10\" region=\"2\"> " << endl
        << "         <ALGO name=\"MAG_PEAK_HT\"> " << endl
        << "         </ALGO> " << endl
        << "     </QUANT> " << endl
        << " " << endl
        << "     -- urea magnitude  " << endl
        << "     <QUANT id=\"11\" region=\"3\"> " << endl
        << "         <ALGO name=\"MAG_PEAK_HT\"> " << endl
        << "         </ALGO> " << endl
        << "     </QUANT> " << endl
        << " " << endl
        << "     -- lac magnitude integrated area " << endl
        << "     <QUANT id=\"12\" region=\"0\"> " << endl
        << "         <ALGO name=\"MAG_INTEGRATE\"> " << endl
        << "         </ALGO> " << endl
        << "     </QUANT> " << endl
        << " " << endl
        << "     -- ala magnitude integrated area " << endl
        << "     <QUANT id=\"13\" region=\"1\"> " << endl
        << "         <ALGO name=\"MAG_INTEGRATE\"> " << endl
        << "         </ALGO> " << endl
        << "     </QUANT> " << endl
        << " " << endl
        << "     -- pyr magnitude integrated area " << endl
        << "     <QUANT id=\"14\" region=\"2\"> " << endl
        << "         <ALGO name=\"MAG_INTEGRATE\"> " << endl
        << "         </ALGO> " << endl
        << "     </QUANT> " << endl
        << " " << endl
        << "     -- urea magnitude integrated area  " << endl
        << "     <QUANT id=\"15\" region=\"3\"> " << endl
        << "         <ALGO name=\"MAG_INTEGRATE\"> " << endl
        << "         </ALGO> " << endl
        << "     </QUANT> " << endl
        << " " << endl
        << " " << endl
        << "     -- ratio of lac to pyr peak ht " << endl
        << "     <RATIO id=\"0\" name=\"LAC/PYR_PEAK_HT\"> " << endl
        << "         <NUMERATOR quant_id=\"0\"> " << endl
        << "         </NUMERATOR> " << endl
        << "         <DENOMINATOR quant_id=\"2\"> " << endl
        << "         </DENOMINATOR> " << endl
        << "     </RATIO> " << endl
        << " " << endl
        << " " << endl
        << "   -- ratio of lac to pyr integrated area  " << endl
        << "     <RATIO id=\"1\" name=\"LAC/PYR_INTEGRATED\"> " << endl
        << "         <NUMERATOR quant_id=\"4\"> " << endl
        << "         </NUMERATOR> " << endl
        << "         <DENOMINATOR quant_id=\"6\"> " << endl
        << "         </DENOMINATOR> " << endl
        << "     </RATIO> " << endl
        << " " << endl
        << "     -- ratio of lac to pyr magnitude  " << endl
        << "     <RATIO id=\"2\" name=\"LAC/PYR_MAG\"> " << endl
        << "         <NUMERATOR quant_id=\"8\"> " << endl
        << "         </NUMERATOR> " << endl
        << "         <DENOMINATOR quant_id=\"10\"> " << endl
        << "         </DENOMINATOR> " << endl
        << "     </RATIO> " << endl
        << " " << endl
        << "   </APPLICATION> " << endl
        << " " << endl
        << " " << endl
        << " -- 31P " << endl
        << "  <APPLICATION nucleus=\"31P\">      " << endl
        << " " << endl
        << "     <REGION id=\"0\" name=\"PCr\"  peak_ppm=\"10\"  width_ppm=\"10\"> " << endl
        << "     </REGION> " << endl
        << " " << endl
        << "     -- Pcr peak ht " << endl
        << "     <QUANT id=\"0\" region=\"0\"> " << endl
        << "         <ALGO name=\"PEAK_HT\"> " << endl
        << "         </ALGO> " << endl
        << "     </QUANT> " << endl
        << " " << endl
        << "     -- Pcr integrated area " << endl
        << "     <QUANT id=\"1\" region=\"0\"> " << endl
        << "         <ALGO name=\"INTEGRATE\"> " << endl
        << "         </ALGO> " << endl
        << "     </QUANT> " << endl
        << " " << endl
        << "     -- Pcr magnitude pk ht " << endl
        << "     <QUANT id=\"2\" region=\"0\"> " << endl
        << "         <ALGO name=\"MAG_PEAK_HT\"> " << endl
        << "         </ALGO> " << endl
        << "     </QUANT> " << endl
        << " " << endl
        << "     -- Pcr magnitude integrated area " << endl
        << "     <QUANT id=\"3\" region=\"0\"> " << endl
        << "         <ALGO name=\"MAG_INTEGRATE\"> " << endl
        << "         </ALGO> " << endl
        << "     </QUANT> " << endl
        << " " << endl
        << "   </APPLICATION> " << endl
        << " " << endl
        << " " << endl
        << " -- 19F " << endl
        << "  <APPLICATION nucleus=\"19F\">      " << endl
        << " " << endl
        << "     <REGION id=\"0\" name=\"19FMet\"  peak_ppm=\"0\"  width_ppm=\"10\"> " << endl
        << "     </REGION> " << endl
        << " " << endl
        << "     -- Pcr peak ht " << endl
        << "     <QUANT id=\"0\" region=\"0\"> " << endl
        << "         <ALGO name=\"PEAK_HT\"> " << endl
        << "         </ALGO> " << endl
        << "     </QUANT> " << endl
        << " " << endl
        << "     -- Pcr integrated area " << endl
        << "     <QUANT id=\"1\" region=\"0\"> " << endl
        << "         <ALGO name=\"INTEGRATE\"> " << endl
        << "         </ALGO> " << endl
        << "     </QUANT> " << endl
        << " " << endl
        << "     -- Pcr magnitude pk ht " << endl
        << "     <QUANT id=\"2\" region=\"0\"> " << endl
        << "         <ALGO name=\"MAG_PEAK_HT\"> " << endl
        << "         </ALGO> " << endl
        << "     </QUANT> " << endl
        << " " << endl
        << "     -- Pcr magnitude integrated area " << endl
        << "     <QUANT id=\"3\" region=\"0\"> " << endl
        << "         <ALGO name=\"MAG_INTEGRATE\"> " << endl
        << "         </ALGO> " << endl
        << "     </QUANT> " << endl
        << " " << endl
        << "   </APPLICATION> " << endl
        << "  <APPLICATION nucleus=\"DEFAULT\">      " << endl
        << " " << endl
        << "     <REGION id=\"0\" name=\"Met1\"  peak_ppm=\"0\"  width_ppm=\"10000\"> " << endl
        << "     </REGION> " << endl
        << " " << endl
        << "     -- met peak ht " << endl
        << "     <QUANT id=\"0\" region=\"0\"> " << endl
        << "         <ALGO name=\"PEAK_HT\"> " << endl
        << "         </ALGO> " << endl
        << "     </QUANT> " << endl
        << " " << endl
        << "     -- met integrated area " << endl
        << "     <QUANT id=\"1\" region=\"0\"> " << endl
        << "         <ALGO name=\"INTEGRATE\"> " << endl
        << "         </ALGO> " << endl
        << "     </QUANT> " << endl
        << " " << endl
        << "     -- met magnitude pk ht " << endl
        << "     <QUANT id=\"2\" region=\"0\"> " << endl
        << "         <ALGO name=\"MAG_PEAK_HT\"> " << endl
        << "         </ALGO> " << endl
        << "     </QUANT> " << endl
        << " " << endl
        << "     -- met magnitude integrated area " << endl
        << "     <QUANT id=\"3\" region=\"0\"> " << endl
        << "         <ALGO name=\"MAG_INTEGRATE\"> " << endl
        << "         </ALGO> " << endl
        << "     </QUANT> " << endl
        << " " << endl
        << "   </APPLICATION> " << endl
        << " " << endl
        << " " << endl
        << " </SVK_MRS_QUANTIFICATION> " << endl; 

            xmlOut.close();
    }

}


/*
 *  Generates z-score maps (metabolite indices) for the quantities specified in 
 *  the input xml configuration file.  
 */
bool svkQuantifyMetabolites::ShouldUpgradeXML()
{
    bool shouldUpdate = false; 
    string v1; 
    string v2; 
    string v3; 
    svkQuantifyMetabolites::GetCurrentXMLVersion(&v1, &v2, &v3);

    int upgradeV1 = 0; 
    int upgradeV2 = 9; 
    int upgradeV3 = 10; 
    if ( svkTypeUtils::StringToInt( v1 )  <= upgradeV1 
            && svkTypeUtils::StringToInt( v2 )  <= upgradeV2 
            && svkTypeUtils::StringToInt( v3 )  < upgradeV3 ) { 
        shouldUpdate = true; 
    }

    return shouldUpdate;     

}


void svkQuantifyMetabolites::GetCurrentXMLVersion(string* v1, string* v2, string* v3)
{
    //  first check if the file exists (returns 0):
    struct stat buf;
    bool fileExists;
    if (stat(svkQuantifyMetabolites::GetDefaultXMLFileName( ).c_str(), &buf) == 0) {
        vtkXMLDataElement* xml = vtkXMLUtilities::ReadElementFromFile(
                svkQuantifyMetabolites::GetDefaultXMLFileName( ).c_str()
        );
        string xmlVersion( xml->GetAttributeValue( 0 ) );
        //  Parse into 3 components:
        size_t delim;
        delim = xmlVersion.find_first_of('.');
        *v1 = xmlVersion.substr(0, delim );
        xmlVersion.assign( xmlVersion.substr( delim + 1 ));

        delim = xmlVersion.find_first_of('.');
        *v2 = xmlVersion.substr( 0, delim );
        xmlVersion.assign( xmlVersion.substr( delim +1 ));

        delim = xmlVersion.find_first_of('.');
        *v3 = xmlVersion.substr( 0, delim );
    } else {
        *v1 = "0";
        *v2 = "0";
        *v3 = "0";
    }
}


/*!
 */
void svkQuantifyMetabolites::SaveOldVersion()
{
    svkUtils::CopyFile( 
        svkQuantifyMetabolites::GetDefaultXMLFileName( ).c_str(),
        svkQuantifyMetabolites::GetOldVersionName().c_str()
    ); 
}


/*!
 */
string svkQuantifyMetabolites::GetOldVersionName()
{
    string v1; 
    string v2; 
    string v3; 
    svkQuantifyMetabolites::GetCurrentXMLVersion(&v1, &v2, &v3);

    string backupName =  svkQuantifyMetabolites::GetDefaultXMLFileName(); 
    backupName.append(".");
    backupName.append(v1);
    backupName.append(".");
    backupName.append(v2);
    backupName.append(".");
    backupName.append(v3);
    backupName.append(".bak");
    return backupName; 

}
