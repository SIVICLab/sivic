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


#ifndef SVK_LOOKUP_TABLE_H
#define SVK_LOOKUP_TABLE_H


#include </usr/include/vtk/vtkObjectFactory.h>
#include </usr/include/vtk/vtkObject.h>
#include </usr/include/vtk/vtkLookupTable.h>


namespace svk{


using namespace std;


class svkLookupTable : public vtkLookupTable 
{ 

    public:

        typedef enum {
            COLOR = 0, 
            REVERSE_COLOR,
            GREY_SCALE, 
            HURD,
            CYAN_HOT,
            FIRE,
            CNI_FIXED,
            CBF_FIXED,
            GREEN_SCALE,
            RED_SCALE,
            HURD_CNI_FIXED,
            NONE
        } svkLookupTableType;

        static svkLookupTable* New();
        vtkTypeMacro( svkLookupTable, vtkLookupTable);


        //  Methods
        virtual void       SetTableRange(double min, double max);

        void               SetAlphaThreshold(double thresholdPercentage);
        double             GetAlphaThreshold();

        // Returns the actual value
        double             GetAlphaThresholdValue();

        void               PrintLUT();
        bool               IsLUTFixed();
        void               SetLUTType(svkLookupTableType type);
        svkLookupTableType GetLUTType();
        


    protected:

        svkLookupTable(); 
        ~svkLookupTable(); 


    private:

        svkLookupTableType type;
        double             alphaThresholdPercentage;

        void               ConfigureAlphaThreshold();
        bool               reverseThreshold;

        static const int NUM_COLORS;

};


}   //svk 
                

#endif //SVK_LOOKUP_TABLE_H
