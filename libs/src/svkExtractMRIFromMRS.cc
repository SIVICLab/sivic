/*
 *  Copyright © 2009-2010 The Regents of the University of California.
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
 *  $URL: https://sivic.svn.sourceforge.net/svnroot/sivic/trunk/libs/src/svkExtractMRIFromMRS.cc $
 *  $Rev: 76 $
 *  $Author: jccrane $
 *  $Date: 2010-01-26 11:05:15 -0800 (Tue, 26 Jan 2010) $
 *
 *  Authors:
 *      Jason C. Crane, Ph.D.
 *      Beck Olson
 */


#include <svkExtractMRIFromMRS.h>
#include <svkSpecPoint.h>


using namespace svk;


vtkCxxRevisionMacro(svkExtractMRIFromMRS, "$Rev: 76 $");
vtkStandardNewMacro(svkExtractMRIFromMRS);


/*!
 *
 */
svkExtractMRIFromMRS::svkExtractMRIFromMRS()
{
#if VTK_DEBUG_ON
    this->DebugOn();
#endif

    vtkDebugMacro(<< this->GetClassName() << "::" << this->GetClassName() << "()");

    this->dataType = svkDcmHeader::UNDEFINED; 
    this->newSeriesDescription = ""; 
    this->zeroCopy = false; 
    this->iod = NULL;
}


/*!
 *
 */
svkExtractMRIFromMRS::~svkExtractMRIFromMRS()
{
    vtkDebugMacro(<<this->GetClassName()<<"::~"<<this->GetClassName());

    if ( this->iod != NULL )  {
        iod->Delete();
        this->iod = NULL;
    }

}


/*!
 *  Set the series description for the DICOM header of the copy.  
 */
void svkExtractMRIFromMRS::SetSeriesDescription( vtkstd::string newSeriesDescription )
{
    this->newSeriesDescription = newSeriesDescription;
}


/*!
 *  Optionally sets the output data type to cast the copy to. 
 */
void svkExtractMRIFromMRS::SetOutputDataType(svkDcmHeader::DcmPixelDataFormat dataType)
{
    this->dataType = dataType; 
}


/*!
 *  If set, the copy will have pixels set to 0.  This is also useful for downcasting
 *  a copy. 
 */
void svkExtractMRIFromMRS::SetZeroCopy(bool zeroCopy)
{
    this->zeroCopy = zeroCopy;
}


/*!
 *  Copy the Dcm Header and Provenance from the input to the output. 
 */
int svkExtractMRIFromMRS::RequestData( vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector )
{

    int startPt = 0; 
    int endPt = 0; 
    this->GetIntegrationPtRange(startPt, endPt); 

    cout << " GET POINT: " << static_cast<int>( (endPt+startPt)/2 ) << endl;

    //  Extract svkMriImageData from svkMrsImageData
    svkMrsImageData::SafeDownCast( this->GetImageDataInput(0) )->GetImage(
        this->GetOutput(), 
        static_cast<int>( (endPt + startPt)/2 ), 
        0, 
        0, 
        0 
    );

    this->GetOutput()->CopyDcos( this->GetImageDataInput(0) ); 

    this->UpdateHeader(); 

    int numVoxels[3]; 
    this->GetOutput()->GetNumberOfVoxels(numVoxels);
    int totalVoxels = numVoxels[0] * numVoxels[1] * numVoxels[2]; 

    for (int i = 0; i < totalVoxels; i++ ) {

        vtkFloatArray* spectrum = vtkFloatArray::SafeDownCast( 
            svkMrsImageData::SafeDownCast(this->GetImageDataInput(0))->GetSpectrumFromID(i) 
        ); 
        float* specPtr = spectrum->GetPointer(0);

        double integral = 0;     
        for ( int pt = startPt; pt <= endPt; pt ++ ) {
            integral += specPtr[2*pt]; 
        }
        this->GetOutput()->GetPointData()->GetScalars()->SetTuple1(i, integral);
    }

    return 1; 
}


/*!
 *  Set the chemical shift of the peak position to integrate over.
 */
void svkExtractMRIFromMRS::SetPeakPosPPM( float centerPPM )
{
    this->peakCenterPPM = centerPPM;
}


/*!
 *  Set the chemical shift range to integrate over.  Integration will be +/- 1/2 this
 *  width about the peak position.
 */
void svkExtractMRIFromMRS::SetPeakWidthPPM( float widthPPM )
{
    this->peakWidthPPM = widthPPM;
}


/*!
 *
 */
void svkExtractMRIFromMRS::GetIntegrationPtRange(int& startPt, int& endPt) 
{

    //  Get the integration range in points:
    svkSpecPoint* point = svkSpecPoint::New();
    point->SetDcmHeader( this->GetImageDataInput(0)->GetDcmHeader() );
    startPt =  static_cast< int > (
                        point->ConvertPosUnits(
                            this->peakCenterPPM + (this->peakWidthPPM/2), svkSpecPoint::PPM, svkSpecPoint::PTS )
                      );
    endPt   =  static_cast< int > (
                        point->ConvertPosUnits(
                            this->peakCenterPPM - (this->peakWidthPPM/2), svkSpecPoint::PPM, svkSpecPoint::PTS )
                      );  

    point->Delete();
}


/*!
 *
 */
void svkExtractMRIFromMRS::UpdateProvenance()
{
    vtkDebugMacro(<<this->GetClassName()<<"::UpdateProvenance()");
}


/*!
 *
 */
void svkExtractMRIFromMRS::UpdateHeader()
{

    if (this->newSeriesDescription == "") {
        cout << "ERROR:  must set target image series description" << endl;
        exit(1); 
    }

    //  Copy the DICOM header:     
    //  Create an MRI header template to initialize the output object DcmHeader:
    this->iod = svkMRIIOD::New();
    iod->SetDcmHeader( this->GetOutput()->GetDcmHeader() );
    iod->InitDcmHeader();

    this->GetOutput()->GetDcmHeader()->MakeDerivedDcmHeader( 
        this->GetOutput()->GetDcmHeader(), 
        this->newSeriesDescription
    );

    this->ConvertDcmMrsToMri(); 

}


/*!
 *
 */
int svkExtractMRIFromMRS::ConvertDcmMrsToMri()
{
    svkDcmHeader* mrs = this->GetImageDataInput(0)->GetDcmHeader();
    svkDcmHeader* mri = this->GetOutput()->GetDcmHeader(); 

    //
    //  Patient IE requires modification
    //
    mri->InitPatientModule(
        mrs->GetStringValue( "PatientsName" ), 
        mrs->GetStringValue( "PatientID" ), 
        mrs->GetStringValue( "PatientsBirthDate" ), 
        mrs->GetStringValue( "PatientsSex" )
    );


    //
    //  Study IE requires modification
    //
    mri->InitGeneralStudyModule(
        mrs->GetStringValue("StudyDate"), 
        mrs->GetStringValue("StudyTime"), 
        mrs->GetStringValue("ReferringPhysiciansName"), 
        mrs->GetStringValue("StudyID"), 
        mrs->GetStringValue("AccessionNumber") 
    );

    //
    //  General Series Module
    //
    mri->InitGeneralSeriesModule(
        "77", 
        "", 
        mrs->GetStringValue("PatientPosition") 
    ); 

    //
    //  Image Pixel Module 
    //  Set DCM data type based on vtkImageData Scalar type:
    //
    if ( this->GetOutput()->GetScalarType() == VTK_DOUBLE ) {
        dataType = svkDcmHeader::SIGNED_FLOAT_8; 
    } else if ( this->GetOutput()->GetScalarType() == VTK_FLOAT ) {
        dataType = svkDcmHeader::SIGNED_FLOAT_4; 
    } else {
        cout << this->GetClassName() << ": Unsupported ScalarType " << endl;
        exit(1); 
    }
    mri->InitImagePixelModule( 
        mrs->GetIntValue( "Rows"), 
        mrs->GetIntValue( "Columns"), 
        dataType 
    ); 

    //
    //  Per Frame Functinal Groups Module 
    //
    int numSlices = mrs->GetNumberOfSlices();
    double dcos[3][3];
    mrs->GetDataDcos( dcos );

    double pixelSpacing[3];
    mrs->GetPixelSpacing( pixelSpacing );

    double toplc[3];
    mrs->GetOrigin( toplc, 0 );

    mri->InitPerFrameFunctionalGroupSequence( toplc, pixelSpacing, dcos, numSlices, 1, 1 ); 

    mri->InitPlaneOrientationMacro(
        mrs->GetStringSequenceItemElement( 
            "PlaneOrientationSequence",
            0,
            "ImageOrientationPatient",
            "SharedFunctionalGroupsSequence"
        )
    );
 
    //mri->SetSliceOrder( mrs->GetSliceOrder() );

    // Add Pixel Spacing
    vtkstd::string pixelSizes = mrs->GetStringSequenceItemElement ( 
                                        "PixelMeasuresSequence",
                                        0,
                                        "PixelSpacing",
                                        "SharedFunctionalGroupsSequence"
                                    ); 

    vtkstd::string sliceThickness = mrs->GetStringSequenceItemElement ( 
                                        "PixelMeasuresSequence",
                                        0,
                                        "SliceThickness",
                                        "SharedFunctionalGroupsSequence"
                                    ); 

    mri->InitPixelMeasuresMacro(  pixelSizes, sliceThickness ); 

}


/*!
 *
 */
int svkExtractMRIFromMRS::FillInputPortInformation( int vtkNotUsed(port), vtkInformation* info )
{
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "svkMrsImageData");
    return 1;
}


/*!
 *  Output from this algo is an svkMriImageData object. 
 */
int svkExtractMRIFromMRS::FillOutputPortInformation( int vtkNotUsed(port), vtkInformation* info )
{
    info->Set( vtkDataObject::DATA_TYPE_NAME(), "svkMriImageData"); 
    return 1;
}

