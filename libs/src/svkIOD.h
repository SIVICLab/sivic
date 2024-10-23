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


#ifndef SVK_IOD_H
#define SVK_IOD_H


#include </usr/include/vtk/vtkObjectFactory.h>
#include <svkDcmHeader.h>


namespace svk {


using namespace std;


/*! 
 *  Base class of static methods for default IOD initialization 
 *  using svkDcmHeader adapter interface. 
 */
class svkIOD : public vtkObject 
{

    public:

        vtkTypeMacro( svkIOD, vtkObject);

        //  Methods:
        virtual void  InitDcmHeader() = 0;
        void          SetDcmHeader(svkDcmHeader* header);  
        void          SetReplaceOldElements( bool replaceElements );  

    protected:

        svkIOD();
        ~svkIOD();

        //  Methods:

        /*!
         *  Returns string version of SOP class modality.     
         */
        virtual string  GetModality();
        virtual string  GetManufacturer();

        //  Patient IE
        virtual void    InitPatientModule(); 

        //  Study IE
        virtual void    InitGeneralStudyModule(); 

        //  Series IE
        virtual void    InitGeneralSeriesModule(); 
        virtual void    InitMRSeriesModule();

        //  FrameOfReference IE
        virtual void    InitFrameOfReferenceModule(); 

        //  Equipment IE
        virtual void    InitGeneralEquipmentModule(); 
        virtual void    InitEnhancedGeneralEquipmentModule();

        //  Image IE
        virtual void    InitGeneralImageModule(); 
        virtual void    InitImagePixelModule(); 

        //  Modules found in multiple IEs:
        virtual void    InitMultiFrameFunctionalGroupsModule();
        virtual void    InitMultiFrameDimensionModule();
        virtual void    InitAcquisitionContextModule();
        void            InitFrameAnatomyMacro();
        virtual void    InitSOPCommonModule() = 0; 


        //  Members:  
        svkDcmHeader*       dcmHeader;
        static const string NA_STRING;
        static const string NA_DATE_STRING;
        static const string NA_TIME_STRING;
        
};


}   //svk


#endif //SVK_IOD_H

