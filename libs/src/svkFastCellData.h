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
 *
 *  Authors:
 *      Jason C. Crane, Ph.D.
 *      Beck Olson
 */


#ifndef SVK_FAST_CELL_DATA
#define SVK_FAST_CELL_DATA


#include </usr/include/vtk/vtkObject.h>
#include </usr/include/vtk/vtkObjectFactory.h>
#include </usr/include/vtk/vtkCellData.h>
#include </usr/include/vtk/vtkDataArray.h>

namespace svk {


/*! 
 *  The purpose of this class is to create a fast method for adding arrays to cell data.
 *  VTK's implementation is an order N operation to add arrays so adding N arrays becomes
 *  N^2 and is slow for large data sets. To get around this svkFastCellData permits the
 *  adding of arrays without checking for duplicates and without updating the number of
 *  components. The method FinishAddArray is used to complete the process and updates
 *  the components.
 *
 *  This object also overrides the DeepCopy method to use the Fast Adding methodology.
 */
class svkFastCellData : public vtkCellData
{

    public:

        // vtk type revision macro
        vtkTypeMacro( svkFastCellData,vtkCellData );
   
        static svkFastCellData*       New();
        
        svkFastCellData();
        ~svkFastCellData();

        //! A fast method that does not require name lookup. Follow with FinshFastAdd when all arrays are added.
        virtual int  FastAddArray( vtkAbstractArray* array );

        //! A fast method that does not update the number of components.Follow with FinishFastAdd when all arrays are set.
        virtual void FastSetArray(int i, vtkAbstractArray *data);

        //! Method must be called after using FastAddArray
        virtual void FinishFastAdd();
        virtual void DeepCopy(vtkFieldData *fd);

        //  Description:
        //  reimplement the vtk version from vtkFieldData: 
        //  Copy a field by reference counting the data arrays.
        virtual void ShallowCopy(vtkFieldData *da);


};


}   //svk


#endif //SVK_FAST_CELL_DATA
