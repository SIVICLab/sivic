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
 *  $URL$
 *  $Rev$
 *  $Author$
 *  $Date$
 *
 *  Authors:
 *      Jason C. Crane, Ph.D.
 *      Beck Olson
 */



#define DEBUG 0


#include <svkDataValidator.h>


using namespace svk;


vtkCxxRevisionMacro(svkDataValidator, "$Rev$");
vtkStandardNewMacro(svkDataValidator);


//! Constructor
svkDataValidator::svkDataValidator()
{
    resultInfo = "";
}


//! Destructor
svkDataValidator::~svkDataValidator()
{
}

/*!
 * Check to see if two datasets are compatible. The resultInfo  will be an empty string if
 * they are compatible, otherwise it will contain an error message.
 *
 * SIDE EFFECT: resultInfo member variable will be changed.
 *
 *  \param data1 the first dataset to be compared
 *  \param data2 the second dataset to be compared
 *
 */
svkDataValidator::ValidationErrorStatus svkDataValidator::AreDataIncompatible( svkImageData* data1, svkImageData* data2 )
{
    ValidationErrorStatus areDataIncompatible = svkDataValidator::VALID_DATA;
    double dcos1[3][3];
    double dcos2[3][3];
    int tol = 100;
    resultInfo = "";

    // First lets check the dcos. Parallel and anti-parallel are acceptible.
    if( data1 != NULL && data2 != NULL ) {
        data1->GetDcos( dcos1 );
        data2->GetDcos( dcos2 );
        for( int i = 0; i < 3; i++ ) {
            for( int j = 0; j < 3; j++ ) {
                // Check the absolute value, with a tolerance in case of rounding errors
                if( fabs(floor(tol*dcos1[i][j])) != fabs(floor(tol*dcos2[i][j])) ) {
                    areDataIncompatible = svkDataValidator::INVALID_DATA_ORIENTATION;
                    resultInfo = "Dcos matrices are not aligned!  ";
                }
            }
        }

        // Now lets check to see if the patient id's match
        if( strcmp( data1->GetDcmHeader()->GetStringValue("PatientID").c_str(), 
            data2->GetDcmHeader()->GetStringValue("PatientID").c_str() ) != 0 ) {
            areDataIncompatible = svkDataValidator::INVALID_DATA_PATIENT_ID;
            resultInfo += "Patient ID does not match between datasets!  ";
        }

    } else {
        areDataIncompatible = svkDataValidator::INVALID_DATA_CORRUPTED;
        resultInfo += "At least one dataset is corrupt (null)!  ";
    } 
cout << " VALID: " << areDataIncompatible << endl;

    return areDataIncompatible; 
}
