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
//#include </usr/include/vtk/svkImageMathematics.h>

#include <svkDSCQuantify.h>
#include <svkDSCPeakHeight.h>
#include <svkDSCRecovery.h>
#include <svkImageCopy.h>

#include <time.h>
#include <sys/stat.h>


using namespace svk;


//vtkCxxRevisionMacro(svkDSCQuantify, "$Rev$");
vtkStandardNewMacro(svkDSCQuantify);


/*!
 *
 */
svkDSCQuantify::svkDSCQuantify()
{
#if VTK_DEBUG_ON
    this->DebugOn();
#endif

    vtkDebugMacro(<< this->GetClassName() << "::" << this->GetClassName() << "()");
}


/*!
 *
 */
svkDSCQuantify::~svkDSCQuantify()
{
    vtkDebugMacro(<<this->GetClassName()<<"::~"<<this->GetClassName());
}


/* 
 *  Parses the XML quantification parameters and generate set of metabolite maps.  
 */
void svkDSCQuantify::Quantify()
{

    this->GenerateDSCMaps();
    this->GenerateNormalizedMaps(); 

}


/*
 *  Retruns a pointer to the metabolite map vector
 */
std::vector< svkMriImageData* >* svkDSCQuantify::GetDSCMaps()
{
    return &(this->dscMapVector);        
}


/*
 *
 */
void svkDSCQuantify::GenerateDSCMaps()
{

    //  Calculate DSC Peak Ht map:
    svkDSCPeakHeight* dscPkHt = svkDSCPeakHeight::New();
    dscPkHt->SetInputData( this->GetImageDataInput(0) ); 
    dscPkHt->SetSeriesDescription( "Peak Height" );
    dscPkHt->Update();
    svkMriImageData* htImage = svkMriImageData::New();
    htImage->DeepCopy( dscPkHt->GetOutput() );

    this->dscMapVector.push_back( htImage );     

    dscPkHt->Delete();


    //  Calculate DSC % recovery map:
    svkDSCRecovery* dscRecov = svkDSCRecovery::New();
    dscRecov->SetInputData( this->GetImageDataInput(0) ); 
    dscRecov->SetSeriesDescription( "Percent Recovery" );
    dscRecov->Update();
    svkMriImageData* recovImage = svkMriImageData::New();
    recovImage->DeepCopy( dscRecov->GetOutput() );

    this->dscMapVector.push_back( recovImage );     

    dscRecov->Delete();
 
}


/*
 *
 */
int svkDSCQuantify::GetIntFromString(std::string stringVal ) 
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
float svkDSCQuantify::GetFloatFromString(std::string stringVal ) 
{
 
    istringstream* iss = new istringstream();
    float floatVal;
    iss->str( stringVal ); 
    *iss >> floatVal;
    delete iss;
    return floatVal; 
}


/*
 *  Generates the normalized DSC maps.  Maps are normalized
 *  to the normal appearing white matter values from the 
 *  specific mask. 
 */
void svkDSCQuantify::GenerateNormalizedMaps()
{

    //  Calculate DSC Peak Ht map:
    svkDSCPeakHeight* dscPkHt = svkDSCPeakHeight::New();
    dscPkHt->SetInputData( this->GetImageDataInput(0) ); 
    dscPkHt->SetSeriesDescription( "normalized Peak Height" );
    dscPkHt->SetNormalize();
    dscPkHt->Update();
    svkMriImageData* image = svkMriImageData::New();
    image->DeepCopy( dscPkHt->GetOutput() );

    this->dscMapVector.push_back( image );     

    dscPkHt->Delete();

}


/*!
 *  Resets the origin and extent for correct initialization of output svkMriImageData object from input 
 *  svkMrsImageData object. 
 */
int svkDSCQuantify::RequestInformation( vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector )
{
    return 1;
}


/*!
 *  Copy the Dcm Header and Provenance from the input to the output. 
 */
int svkDSCQuantify::RequestData( vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector )
{

    this->dscMapVector.clear();
    this->Quantify();

    return 1; 
};


/*
 *  Replace ('/') slash character and white space in name attributes with "_div_" 
 *  and "_" respectively so we end up with valid file names. 
 */
string svkDSCQuantify::ReplaceSlashesAndWhiteSpace( std::string inString)
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
 *
 */
void svkDSCQuantify::UpdateProvenance()
{
    vtkDebugMacro(<<this->GetClassName()<<"::UpdateProvenance()");
}


/*!
 *
 */
int svkDSCQuantify::FillInputPortInformation( int vtkNotUsed(port), vtkInformation* info )
{
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "svkMriImageData");
    return 1;
}


/*!
 *  Output from this algo is an svkMriImageData object. 
 */
int svkDSCQuantify::FillOutputPortInformation( int vtkNotUsed(port), vtkInformation* info )
{
    info->Set( vtkDataObject::DATA_TYPE_NAME(), "svkMriImageData"); 
    return 1;
}

