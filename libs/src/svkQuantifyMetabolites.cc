/*
 *  Copyright © 2009-2011 The Regents of the University of California.
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


#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkStreamingDemandDrivenPipeline.h>
#include <vtkXMLDataElement.h>
#include <vtkXMLUtilities.h>
#include <vtkImageMathematics.h>

#include <svkQuantifyMetabolites.h>
#include <svkMetaboliteMap.h>
#include <svkImageCopy.h>
#include <svkMetaboliteRatioZScores.h>


using namespace svk;


vtkCxxRevisionMacro(svkQuantifyMetabolites, "$Rev$");
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
    this->xmlFileName = "/home/jasonc/svn/surbeck/brain/sivic/trunk/libs/src/mrs.xml"; 
    this->useSelectedVolumeFraction = 0;

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

    vtkXMLDataElement* mrsXML = vtkXMLUtilities::ReadElementFromFile( this->xmlFileName.c_str() ); 
    this->GenerateRegionMaps( mrsXML ); 
    this->GenerateRatioMaps( mrsXML ); 
    this->GenerateZScoreMaps( mrsXML ); 
    //mrsXML->PrintXML(cout, vtkIndent()); 

}


/*
 *  Retruns a pointer to the metabolite map vector
 */
vtkstd::vector< svkMriImageData* >* svkQuantifyMetabolites::GetMetMaps()
{
    return &(this->metMapVector);        
}


/*
 *
 */
void svkQuantifyMetabolites::GenerateRegionMaps( vtkXMLDataElement* mrsXML )
{

    //  
    //  Loop over the regions to quantify:
    //  
    bool parseRegions = true; 
    int regionIndex = 0; 
    vtkXMLDataElement* regionXML;
    svkMetaboliteMap* mapGen = svkMetaboliteMap::New();
    mapGen->SetInput( this->GetImageDataInput(0) ); 

    while ( parseRegions ) {

        ostringstream ossIndex;
        ossIndex << regionIndex;
        vtkstd::string regionIndexString(ossIndex.str());

        regionXML = mrsXML->FindNestedElementWithNameAndId("REGION", regionIndexString.c_str());

        if (regionXML != NULL ) {

            //cout << "region id = " << regionIndexString << endl;
            //regionXML->PrintXML( cout, vtkIndent() ); 

            //  Get algo type, quant params: peak, width and generate metabolite 
            //  image to store in a vtkstd::vector of svkMriImageData objects. 
             
            vtkstd::string regionName( regionXML->GetAttributeValue( 1 ) ); 
            regionName = this->ReplaceSlashesAndWhiteSpace( regionName ); 
            vtkstd::string algoName( regionXML->GetAttributeValue( 2 ) ); 
            algoName = this->ReplaceSlashesAndWhiteSpace( algoName ); 
            float peakPPM;
            regionXML->GetScalarAttribute("peak_ppm", peakPPM); 
            float widthPPM;
            regionXML->GetScalarAttribute("width_ppm", widthPPM); 

            cout << "NAME      : " << regionName << endl;
            cout << "ALGO      : " << algoName << endl;
            cout << "peak pos  : " << peakPPM << endl;
            cout << "peak width: " << widthPPM << endl;

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

            parseRegions = false; 

        }

        regionIndex++; 
    }

    mapGen->Delete();
 
}


/*
 *  Generates the ratio maps for the quantities specified in the input xml 
 *  configuration file. 
 *
 *  If the computation is masked, e.g. by selection box, 
 *  then set division by zero to 
 *  zero, otherwise use actual computed value    
 */
void svkQuantifyMetabolites::GenerateRatioMaps( vtkXMLDataElement* mrsXML )
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
        vtkstd::string ratioIndexString(ossIndex.str());

        ratioXML = mrsXML->FindNestedElementWithNameAndId("RATIO", ratioIndexString.c_str());

        if (ratioXML != NULL ) {

            //  Get images contributing to numerator and  denominator for this ratio
             
            cout << "RXML: " << ratioXML->GetAttributeValue( 1 ) << endl;; 
            vtkstd::string ratioName( ratioXML->GetAttributeValue( 1 ) ); 
            ratioName = this->ReplaceSlashesAndWhiteSpace( ratioName ); 
            cout << "RATIO NAME : " << ratioName << endl;
            
            //  Initialize the ratio with zeros:
            svkImageData* numeratorImage   = svkMriImageData::New();
            svkImageData* denominatorImage = svkMriImageData::New();
            numeratorImage->DeepCopy( this->metMapVector[0] ); 
            denominatorImage->DeepCopy( this->metMapVector[0] ); 

            //  Get the numerator and denominator values for this ratio
            this->GetNumeratorAndDenominatorImages( ratioXML, numeratorImage, denominatorImage); 

            vtkImageMathematics* mathR = vtkImageMathematics::New(); 
            mathR->SetOperationToDivide();
            //  If the computation is masked, e.g. by selection box, 
            //  then set division by zero to 
            //  zero, otherwise use actual computed value    
            if ( this->useSelectedVolumeFraction ) { 
                mathR->SetConstantC(0);
                mathR->DivideByZeroToCOn();
            }
            mathR->SetInput1( numeratorImage ); 
            mathR->SetInput2( denominatorImage ); 
            mathR->Update();

            //  Create a new image a copy from a correctly 
            //  instantiated object, then copy over the new pixels
            //  from the vtkImageData object
            svkMriImageData* ratioImage = svkMriImageData::New();
            ratioImage->DeepCopy( this->metMapVector[0] );
            ratioImage->DeepCopy( mathR->GetOutput() );

            //  set the header fields
            vtkstd::string seriesDescription( ratioName );
            svkDcmHeader* hdr = ratioImage->GetDcmHeader();
            hdr->SetValue("SeriesDescription", ratioName );

            this->metMapVector.push_back( ratioImage ); 

        } else {

            parseRatios = false; 

        }

        ratioIndex++; 
    }

}


/*
 *  Sums metabolite imgages contributing to numerator and denominator of image ratios or z-scores.   
 */
void svkQuantifyMetabolites::GetNumeratorAndDenominatorImages( vtkXMLDataElement* ratioXML, svkImageData* numeratorImage, svkImageData* denominatorImage)
{

    vtkImageMathematics* mathN = vtkImageMathematics::New(); 
    vtkImageMathematics* mathD = vtkImageMathematics::New(); 
    mathN->SetOperationToAdd();
    mathD->SetOperationToAdd();
    mathN->SetInput1( numeratorImage );     
    mathD->SetInput1( denominatorImage );     

    svkImageData* inputImage = svkMrsImageData::New();
    inputImage->ZeroCopy( this->metMapVector[0] );

    bool elementsExist = true; 
    int elementID = 0; 
    while ( elementsExist ) {

        vtkXMLDataElement* nestedXML = ratioXML->GetNestedElement(elementID); 
        if ( nestedXML != NULL ) {

            //  Is it part of numerator or denominator?
            vtkstd::string regionType (nestedXML->GetName() ); 
            vtkstd::string regionNumString = nestedXML->GetAttributeValue(0);
            istringstream* iss = new istringstream();
            int regionNum;
            iss->str( regionNumString ); 
            *iss >> regionNum;
            delete iss;
            elementID++;    
            if ( regionType.compare("NUMERATOR") == 0 ) {
                inputImage->DeepCopy( this->metMapVector[regionNum] );
                mathN->SetInput2( inputImage ); 
                mathN->Update();
                numeratorImage->DeepCopy( mathN->GetOutput() ); 
                mathN->SetInput1( numeratorImage ); 
            } else if ( regionType.compare("DENOMINATOR") == 0 ) {
                mathD->SetInput2( this->metMapVector[regionNum] ); 
                mathD->Update();
                denominatorImage->DeepCopy( mathD->GetOutput() ); 
                mathD->SetInput1( denominatorImage ); 
            }
            
        } else {

            elementsExist = false; 
        }
    }

    mathN->Delete();
    mathD->Delete();
}


/*
 *  Generates z-score maps (metabolite indices) for the quantities specified in 
 *  the input xml configuration file.  
 */
void svkQuantifyMetabolites::GenerateZScoreMaps( vtkXMLDataElement* mrsXML )
{

    //  
    //  Loop over the zscores to quantify (while loop):
    //  
    bool parseZScores= true; 
    int zscoreIndex = 0; 
    vtkXMLDataElement* zscoreXML;

    while ( parseZScores) {

        ostringstream ossIndex;
        ossIndex << zscoreIndex;
        vtkstd::string zscoreIndexString(ossIndex.str());

        zscoreXML = mrsXML->FindNestedElementWithNameAndId("ZSCORE", zscoreIndexString.c_str());

        if (zscoreXML != NULL ) {

            //  Get images contributing to numerator and  denominator for this index 
             
            cout << "ZSXML: " << zscoreXML->GetAttributeValue( 1 ) << endl;; 
            vtkstd::string zscoreName( zscoreXML->GetAttributeValue( 1 ) ); 
            zscoreName = this->ReplaceSlashesAndWhiteSpace( zscoreName ); 
            cout << "ZSCORE NAME : " << zscoreName << endl;
            
            //  Initialize the z-score maps with zeros:
            svkImageData* numeratorImage   = svkMriImageData::New();
            svkImageData* denominatorImage = svkMriImageData::New();
            numeratorImage->DeepCopy( this->metMapVector[0] ); 
            denominatorImage->DeepCopy( this->metMapVector[0] ); 

            //  Get the numerator and denominator images for 
            //  this ratio:
            this->GetNumeratorAndDenominatorImages( zscoreXML, numeratorImage, denominatorImage); 

            svkMetaboliteRatioZScores* zscore = svkMetaboliteRatioZScores::New(); 
            if ( this->useSelectedVolumeFraction ) { 
                zscore->LimitToSelectedVolume(); 
            }

            //  Now generate the z-score form the 
            //  numerator/denominator images
            zscore->SetInputNumerator( numeratorImage ); 
            zscore->SetInputDenominator( denominatorImage ); 
            zscore->SetInputMrsData( this->GetImageDataInput(0) ); 
            zscore->Update();

            //  Create a new image a copy from a correctly 
            //  instantiated object, then copy over the new pixels
            //  from the vtkImageData object
            svkMriImageData* zscoreImage = svkMriImageData::New();
            zscoreImage->DeepCopy( zscore->GetOutput() );

            //  set the header fields
            vtkstd::string seriesDescription( zscoreName );
            svkDcmHeader* hdr = zscoreImage->GetDcmHeader();
            hdr->SetValue("SeriesDescription", zscoreName );

            this->metMapVector.push_back( zscoreImage ); 

        } else {

            parseZScores = false; 

        }

        zscoreIndex++; 
    }

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

    this->Quantify();

    return 1; 
};


/*
 *  Replace ('/') slash character and white space in name attributes with "_div_" 
 *  and "_" respectively so we end up with valid file names. 
 */
string svkQuantifyMetabolites::ReplaceSlashesAndWhiteSpace( vtkstd::string inString)
{
   
    vtkstd::string outString( inString ) ;

    size_t slashPos;
    slashPos = outString.find_first_of("/");

    while ( slashPos != vtkstd::string::npos ) {
        outString.assign( outString.replace(slashPos, 1, "_div_") );
        slashPos = outString.find_first_of("/");
    }

    
    size_t whitePos;
    whitePos = outString.find_first_of(" ");

    while ( whitePos != vtkstd::string::npos ) {
        outString.assign( outString.replace(whitePos, 1, "_") );
        whitePos = outString.find_first_of(" ");
    }

    return outString; 

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

