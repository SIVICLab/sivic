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


#ifndef SVK_DATA_VALIDATOR_H
#define SVK_DATA_VALIDATOR_H


#include </usr/include/vtk/vtkObjectFactory.h>
#include <svkImageData.h>
#include <svkUtils.h>
#include <set>


namespace svk {


/*! 
 *   This class is used to validate datasets and dataset compatibility.
 *   Results of comparisons are stored in the resultInfo member variable.
 *   If the result yielded no errors the variable will be empty.
 *
 *   NOTE: It might be better to return a string instead of using the member
 *         variable, but this would make using the method a little more
 *         cumbersome and less intuitive in if statements. It would however
 *         allow the method to be static...
 */
class svkDataValidator : public vtkObject
{

    public:

        // vtk type revision macro
        vtkTypeMacro( svkDataValidator,vtkObject );
   
        static svkDataValidator*       New();  
        
        svkDataValidator();
        ~svkDataValidator();


        /*!
         *  Supported DICOM IOD types.
         */
        enum ValidationErrorStatus {
            VALID_DATA = 0,
            INVALID_DATA_ORIENTATION = 1,
            INVALID_DATA_PATIENT_ID = 2,
            INVALID_DATA_CORRUPTED = 3,
            INVALID_DATA_ACCESSION_NUMBER = 4
        };
        
        //! Check to see if two datasets are from the same scan
        bool    AreDataCompatible( svkImageData* data1, svkImageData* data2 );
        bool    AreDataOrientationsSame( svkImageData* data1, svkImageData* data2 ); 
        bool    AreDataGeometriesSame( svkImageData* data1, svkImageData* data2 ); 
        bool    AreCellDataArrayStructureSame( svkImageData* data1, svkImageData* data2 );
        bool    AreDataExtentsSame( svkImageData* data1, svkImageData* data2 ); 
        bool    AreDataSpacingsSame( svkImageData* data1, svkImageData* data2 ); 
        bool    AreDataOriginsSame( svkImageData* data1, svkImageData* data2 ); 
        bool    IsInvalid( svkDataValidator::ValidationErrorStatus  error ); 
        bool    IsOnlyError( svkDataValidator::ValidationErrorStatus  error );
        bool    AreValuesClose( double valueA, double valueB, int percentTolerance  );

        //! Holds the result of the last validation
        string resultInfo;

    private: 
        set <svkDataValidator::ValidationErrorStatus> status; 
};


}   //svk


#endif //SVK_DATA_VALIDATOR_H
