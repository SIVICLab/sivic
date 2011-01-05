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


#include <svkDataValidator.h>


using namespace svk;


vtkCxxRevisionMacro(svkDataValidator, "$Rev$");
vtkStandardNewMacro(svkDataValidator);


//! Constructor
svkDataValidator::svkDataValidator()
{
#if VTK_DEBUG_ON
    this->DebugOn();
#endif

    this->resultInfo = "";
}


//! Destructor
svkDataValidator::~svkDataValidator()
{
}


/*!
 *  Check to see if two datasets are compatible. The resultInfo will be an empty string if
 *  they are compatible, otherwise it will contain an error message.
 *
 *  SIDE EFFECT: resultInfo member variable will be changed.
 *
 *  \param data1 the first dataset to be compared
 *  \param data2 the second dataset to be compared
 *
 *  \return bool (false, not compatible) 
 */
bool svkDataValidator::AreDataCompatible( svkImageData* data1, svkImageData* data2 )
{
    double dcos1[3][3];
    double dcos2[3][3];
    int tol = 100;
    this->resultInfo = "";
    this->status.clear(); 

    // First lets check the dcos. Parallel and anti-parallel are acceptible.
    if( data1 != NULL && data2 != NULL ) {

        if (! this->AreDataOrientationsSame( data1, data2 )) {
            this->status.insert(svkDataValidator::INVALID_DATA_ORIENTATION);
            this->resultInfo = "\tDcos matrices are not aligned!\n";
        }

        // Now lets check to see if the patient id's match
        if( strcmp( data1->GetDcmHeader()->GetStringValue("PatientID").c_str(), 
            data2->GetDcmHeader()->GetStringValue("PatientID").c_str() ) != 0 ) {
            this->status.insert(svkDataValidator::INVALID_DATA_PATIENT_ID);
            this->resultInfo += "\tPatient ID does not match between datasets!\n";
        }

    } else {
        this->status.insert(svkDataValidator::INVALID_DATA_CORRUPTED);
        this->resultInfo += "\tAt least one dataset is corrupt (null)!\n";
    } 

    if ( status.empty() ) {
        return true; 
    } else {
        return false; 
    }
}


/*!
 * Check to see if two datasets have the same orientation 
 *
 *  \param data1 the first dataset to be compared
 *  \param data2 the second dataset to be compared
 *
 */
bool svkDataValidator::AreDataOrientationsSame( svkImageData* data1, svkImageData* data2 )
{
    this->resultInfo = "";
    ValidationErrorStatus areDataIncompatible = svkDataValidator::VALID_DATA;
    double dcos1[3][3];
    double dcos2[3][3];
    int tol = 100;

    bool areOrientationsSame = true; 

    // First lets check the dcos. Parallel and anti-parallel are acceptible.
    if( data1 != NULL && data2 != NULL ) {
        data1->GetDcmHeader()->GetDataDcos( dcos1 );
        data2->GetDcmHeader()->GetDataDcos( dcos2 );
        for( int i = 0; i < 3; i++ ) {
            for( int j = 0; j < 3; j++ ) {
                // Check the absolute value, with a tolerance in case of rounding errors
                if( floor(fabs(tol*dcos1[i][j])) != floor(fabs(tol*dcos2[i][j])) ) {
                    this->resultInfo = "Orientations do not match.";
                    return false;
                }
            }
        }

    } 

    return areOrientationsSame; 
}


/*!
 * Check to see if two datasets have the same geometry (extent, spacing, dcos, origin) 
 *
 *  \param data1 the first dataset to be compared
 *  \param data2 the second dataset to be compared
 *
 */
bool svkDataValidator::AreDataGeometriesSame( svkImageData* data1, svkImageData* data2 )
{
    this->resultInfo = "";
    bool geometryMatch = true;
    string compoundResult = this->resultInfo;
    bool extentMatch = AreDataExtentsSame( data1, data2 );
    if( !extentMatch ) {
        compoundResult.append(this->resultInfo);
        compoundResult.append(" ");
        geometryMatch = false;
    }
    bool spacingMatch = AreDataSpacingsSame( data1, data2 );
    if( !spacingMatch ) {
        compoundResult.append(this->resultInfo);
        compoundResult.append(" ");
        geometryMatch = false;
    }
    bool originMatch = AreDataOriginsSame( data1, data2 );
    if( !originMatch ) {
        compoundResult.append(this->resultInfo);
        compoundResult.append(" ");
        geometryMatch = false;
    }
    bool orientationMatch = AreDataOrientationsSame( data1, data2 );
    if( !orientationMatch ) {
        compoundResult.append(this->resultInfo);
        compoundResult.append(" ");
        geometryMatch = false;
    }
    this->resultInfo = compoundResult;
    return geometryMatch;
}


/*!
 * Check to see if two datasets have the same extent
 *
 *  \param data1 the first dataset to be compared
 *  \param data2 the second dataset to be compared
 *
 */
bool svkDataValidator::AreDataExtentsSame( svkImageData* data1, svkImageData* data2 )
{
    this->resultInfo = "";
    if( data1 == NULL || data2 == NULL ) {
        resultInfo="One Data Set Is NULL. Extents do not match.";
        return false;
    }
    int* extent1 = data1->GetExtent();
    int* extent2 = data2->GetExtent();

    // Spectra should have a larger extent to match due to cell vs point data
    if( data1->IsA("svkMrsImageData") && data2->IsA("svkMriImageData") ) {
        if( extent2[0] == extent1[0] 
         && extent2[1] == extent1[1] - 1 
         && extent2[2] == extent1[2] 
         && extent2[3] == extent1[3] - 1
         && extent2[4] == extent1[4] 
         && extent2[5] == extent1[5] - 1) {
            return true;
        } else {
            resultInfo="Extents do not match.";
        }
    } else if( data2->IsA("svkMrsImageData") && data1->IsA("svkMriImageData") ) {
        if( extent1[0] == extent2[0]
         && extent1[1] == extent2[1] - 1 
         && extent1[2] == extent2[2]
         && extent1[3] == extent2[3] - 1
         && extent1[4] == extent2[4]
         && extent1[5] == extent2[5] - 1) {
            return true;
        } else {
            resultInfo="Extents do not match.";
        }
    } else {
        if( extent1[0] == extent2[0]
         && extent1[1] == extent2[1]           
         && extent1[2] == extent2[2]           
         && extent1[3] == extent2[3]           
         && extent1[4] == extent2[4]           
         && extent1[5] == extent2[5] ) {
            return true;
        } else {
            resultInfo="Extents do not match.";
        }
    }
    return false;
}


/*!
 * Check to see if two datasets have the same spacing
 *
 *  \param data1 the first dataset to be compared
 *  \param data2 the second dataset to be compared
 *
 */
bool svkDataValidator::AreDataSpacingsSame( svkImageData* data1, svkImageData* data2 )
{
    this->resultInfo = "";
    if( data1 == NULL || data2 == NULL ) {
        this->resultInfo="One Data Set Is NULL.";
        return false;
    }
    int tol = 1000; // Again due to precision differences we need a tolerance
    double* spacing1 = data1->GetSpacing();
    double* spacing2 = data2->GetSpacing();
    if( floor(1000*spacing1[0]) == floor(1000*spacing2[0])
     && floor(1000*spacing1[1]) == floor(1000*spacing2[1])           
     && floor(1000*spacing1[2]) == floor(1000*spacing2[2]) ) {
        return true;
    } else {
        this->resultInfo="Spacing does not match.";
    }

    return false;
}


/*!
 * Check to see if two datasets have the same origin
 *
 *  \param data1 the first dataset to be compared
 *  \param data2 the second dataset to be compared
 *
 */
bool svkDataValidator::AreDataOriginsSame( svkImageData* data1, svkImageData* data2 )
{
    this->resultInfo = "";
    if( data1 == NULL || data2 == NULL ) {
        this->resultInfo="One Data Set Is NULL.";
        return false;
    }
    int tol = 1000; // Again due to precision differences we need a tolerance
    double origin1[3] = {0,0,0};
    double origin2[3] = {0,0,0};
    data1->GetDcmHeader()->GetOrigin( origin1, 0 );
    data2->GetDcmHeader()->GetOrigin( origin2, 0 );

    if( floor(1000*origin1[0]) == floor(1000*origin2[0])
     && floor(1000*origin1[1]) == floor(1000*origin2[1])           
     && floor(1000*origin1[2]) == floor(1000*origin2[2]) ) {
        return true;
    } else {
        this->resultInfo="Origin does not match.";
    }
    return false;

}


/*!
 *  Returns true if the data sets are incompatible wrt the specified error check
 *  (svkDataValidator::ValidationErrorStatus).
 */
bool svkDataValidator::IsInvalid( svkDataValidator::ValidationErrorStatus  error )
{
    if ( this->status.find( error ) != status.end() ){
        return true; 
    } else {
        return false;
    }
}
