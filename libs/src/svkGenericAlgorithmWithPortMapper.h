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


#ifndef SVK_GENERIC_ALGORITHM_WITH_PORT_MAPPER_H
#define SVK_GENERIC_ALGORITHM_WITH_PORT_MAPPER_H


#include </usr/include/vtk/vtkObject.h>
#include </usr/include/vtk/vtkObjectFactory.h>
#include </usr/include/vtk/vtkInformation.h>

#include <svkImageData.h>
#include <svkMriImageData.h>
#include <svkImageAlgorithm.h>
#include <svkDcmHeader.h>
#include <svkUtils.h>
#include <svkDouble.h>
#include <svkString.h>
#include <svkInt.h>
#include <svkImageReaderFactory.h>
#include <svkImageReader2.h>
#include <svkAlgorithmPortMapper.h>

namespace svk {


using namespace std;


/*! 
 * This baseclass is used to support the use of the svkAlgorithmPortMapper class. All parameters
 * that control the execution of this algorithm are stored in input ports that can be set using
 * the XML input to the svkAlgorithmPortMapper class.
 */
class svkGenericAlgorithmWithPortMapper : public vtkAlgorithm
{

    public:

        vtkTypeMacro( svkGenericAlgorithmWithPortMapper, vtkAlgorithm);

        //! Uses port mapper to parse an XML element and convert it into input port data objects.
        void                    SetInputPortsFromXML( vtkXMLDataElement* element );

        //! Get the internal XML interpreter
        svkAlgorithmPortMapper* GetPortMapper();

        //! Prints all input parameters set.
        void                    PrintSelf( ostream &os, vtkIndent indent );

    protected:

        svkGenericAlgorithmWithPortMapper();
        ~svkGenericAlgorithmWithPortMapper();

        //! All ports must be initialized using the svkAlgorithmPortMapper BEFORE this method is called.
        virtual int             FillInputPortInformation( int port, vtkInformation* info );

        //! All ports must be initialized using the svkAlgorithmPortMapper BEFORE this method is called.
        virtual int             FillOutputPortInformation( int port, vtkInformation* info );

        //! Calls RequestData when the correct request comes through
        virtual int ProcessRequest(vtkInformation* request,
                                              vtkInformationVector** inputVector,
                                              vtkInformationVector* outputVector);

        //! Must be implemented in sub-class
        virtual int RequestData(
                       vtkInformation* request,
                       vtkInformationVector** inputVector,
                       vtkInformationVector* outputVector ) = 0;


        //! The port mapper used to set the input port parameters.
        svkAlgorithmPortMapper* portMapper;

    private:

};


}   //svk


#endif //SVK_GENERIC_ALGORITHM_WITH_PORT_MAPPER_H

