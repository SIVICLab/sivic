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
 *  Much of the source here is copied from vtkDataSetAttributes. Due to private member
 *  variables and a lack of virtual methods we had to re-implement some functionality
 *  for performance reasons when dealing with a large number of arrays.
 *
 *  Authors:
 *      Jason C. Crane, Ph.D.
 *      Beck Olson
 */


#include <svkFastCellData.h>


using namespace svk;


//vtkCxxRevisionMacro(svkFastCellData, "$Rev$");
vtkStandardNewMacro(svkFastCellData);


//! Constructor
svkFastCellData::svkFastCellData()
{

}


//! Destructor
svkFastCellData::~svkFastCellData()
{
}


/*!
 *  Fast add array does array inserting without check
 *  for duplicate names. It also calls FastSetArray.
 *
 * @param array
 * @return
 */
int svkFastCellData::FastAddArray(  vtkAbstractArray* array )
{
    if (!array) {
        return -1;
    }

    int index = -1;
    // Below is the line commented out for speed. It checks for an array of the same name.
    //this->GetAbstractArray(array->GetName(), index);

    // We are going to not check for the abstract array and assume new
    if (index == -1) {
      index = this->NumberOfActiveArrays;
      this->NumberOfActiveArrays++;
    }
    this->FastSetArray(index, array);
    return index;
}


/*!
 * This is a copy of the vtk implementation. The only change is that
 * the updating of the number of components is not done. FinishFastAdd
 * will do that.
 *
 * @param i
 * @param data
 */
void svkFastCellData::FastSetArray(int i, vtkAbstractArray *data)
{
    if (!data || (i > this->NumberOfActiveArrays)) {
        vtkWarningMacro("Can not set array " << i << " to " << data << endl);
        return;
    }


    if ( i < 0 ) {
        vtkWarningMacro("Array index should be >= 0");
        return;
    } else if (i >= this->NumberOfArrays) {
        this->AllocateArrays(i+1);
        this->NumberOfActiveArrays = i+1;
    }

    if ( this->Data[i] != data ) {
        if ( this->Data[i] != NULL ) {
          this->Data[i]->UnRegister(this);
        }
        this->Data[i] = data;
        if ( this->Data[i] != NULL ) {
            this->Data[i]->Register(this);
        }
    }

    /*
     *  Get number of components is very slow, so we are going to bypass
     *  this call. The user must then call FinishFastAdd which will add
     *  a dummy array and then remove it. This is so that the code
     *  below gets updated appropriately.
     *
     *
#ifndef VTK_LEGACY_REMOVE
   //adjust scratch tuple array
    only if when the final array is being set
  int numComp = this->GetNumberOfComponents();
  if ( numComp != this->TupleSize )
    {
    this->TupleSize = numComp;
    if ( this->Tuple )
      {
      delete [] this->Tuple;
      }
    this->Tuple = new double[this->TupleSize];
    }
#endif
     */

}


/*!
 *
 *  FinishFastAdd is used to complete the fast adding process.
 *  It adds and then removes a dummy array so that we can be
 *  sure that the number of components is updated correctly.
 *
 *  This is implemented this way to get around private
 *  member variables used in the vtkDa
 *
 */
void svkFastCellData::FinishFastAdd()
{
    //  If no arrays, then skip this method:
    if ( this->GetArray(0) )  {
        vtkAbstractArray* toRemove = vtkDataArray::CreateArray( this->GetArray(0)->GetDataType() );
        this->AddArray( toRemove );
        this->RemoveArray( this->GetNumberOfArrays() - 1 );
        toRemove->Delete();
    }
}


/*!
 * Uses fast add array methods to do deep copies. Also uses ShallowCopy to
 * copy field data attributes which are private and cannot be updated
 * in this method directly.
 *
 * @param fd
 */
void svkFastCellData::DeepCopy(vtkFieldData *fd)
{

    this->Initialize(); //free up memory

    vtkDataSetAttributes* dsa = vtkDataSetAttributes::SafeDownCast(fd);
    // If the source is a vtkDataSetAttributes
    if (dsa) {
        int numArrays = fd->GetNumberOfArrays();
        int attributeType, i;
        vtkAbstractArray *data, *newData;

        // Let's do a shallow copy first to get the structure and the field data attributes.
        this->ShallowCopy( fd );

        // Allocate space for numArrays
        this->AllocateArrays(numArrays);
        for (i=0; i < numArrays; i++ ) {
            data = fd->GetAbstractArray(i);
            newData = data->NewInstance(); //instantiate same type of object
            newData->DeepCopy(data);
            newData->SetName(data->GetName());
            this->FastSetArray(i, newData);
            newData->Delete();
        }

        this->FinishFastAdd();
        // Copy the copy flags

        this->CopyFlags(dsa);
    } else {  // If the source is field data, do a field data copy
        this->vtkFieldData::DeepCopy(fd);
    }
}


//----------------------------------------------------------------------------
// Copy a field by reference counting the data arrays.
void svkFastCellData::ShallowCopy(vtkFieldData *f)
{

    this->AllocateArrays(f->GetNumberOfArrays());
    this->NumberOfActiveArrays = 0;

    for ( int i=0; i < f->GetNumberOfArrays(); i++ )
    {
        this->NumberOfActiveArrays++;
        this->FastSetArray(i, f->GetAbstractArray(i));
    }

    this->CopyFlags(f);

    this->FinishFastAdd();
}

