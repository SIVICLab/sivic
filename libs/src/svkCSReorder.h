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
 *
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
#ifndef SIVIC_SVKCSREORDER_H
#define SIVIC_SVKCSREORDER_H


#include </usr/include/vtk/vtkObjectFactory.h>
#include </usr/include/vtk/vtkDataObject.h>
#include </usr/include/vtk/vtkGlobFileNames.h>
#include <svkDataAcquisitionDescriptionXML.h>
#include <svkTypeUtils.h>
namespace svk {


    using namespace std;

/*!
 *  This class is a container for a primitive bool. It is a subclass of vtkDataObject so it can
 *  be used in input ports of algorithms.
 */
    class svkCSReorder : public vtkObject
    {

    public:

        // vtk type revision macro
        vtkTypeMacro( svkCSReorder, vtkObject );

        // vtk initialization 
        static svkCSReorder* New();

        void SetDADFilename( string dadFilename);
        void ReOrderAndPadData(float* originalData, int numberDataPointsInFIDFile, float*** paddedData);
        void ReOrderData(float *reorderedData, float* originalData);
        void PadData(float*** paddedData, float* reorderedData);

    protected:

        svkCSReorder();
        ~svkCSReorder();

    private:
        svkDataAcquisitionDescriptionXML* dadFile;

    };



}   //svk


#endif //SIVIC_SVKCSREORDER_H
