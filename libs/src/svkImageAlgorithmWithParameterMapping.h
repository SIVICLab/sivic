/*
 *  Copyright © 2009-2014 The Regents of the University of California.
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


#ifndef SVK_IMAGE_ALGORITHM_WITH_PARAMETER_MAPPING_H
#define SVK_IMAGE_ALGORITHM_WITH_PARAMETER_MAPPING_H


#include <vtkObject.h>
#include <vtkObjectFactory.h>
#include <vtkInformation.h>

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

//! vtk types are defined this way, so we'll keep with the tradition to add our types
#define SVK_MR_IMAGE_DATA  1000
#define SVK_4D_IMAGE_DATA  1001

namespace svk {


using namespace std;


/*! 
 * This class defines a generic interface for using both vtkDataObjects and primitive types stored
 * as subclasses of vtkDataObject as inputs to algorithms. Additionally it has a method
 * (svkImageAlgorithmWithParameterMapping::SetParametersFromXML) to parse an XML tag and extract any
 * elements in it that correspond to known input ports initialized by the method,
 * svkImageAlgorithmWithParameterMapping::InitializeInputPort which must be
 * called in the subclass's constructor. Input images are assumed to be stored as filename strings
 * in the XML and will be read in by this class to svkImageData objects.
 *
 * vtkDataObject wrapped primitive types: svkDouble, svkInt, svkString
 */
class svkImageAlgorithmWithParameterMapping : public svkImageAlgorithm
{

    public:


        //! Parses an XML element and converts it into input port parameters. Converts image filename strings to svkImageData objects.
        virtual void           SetInputPortsFromXML( vtkXMLDataElement* element );

        //! Prints all input parameters set.
        void                   PrintSelf( ostream &os, vtkIndent indent );

    protected:

        svkImageAlgorithmWithParameterMapping();
        ~svkImageAlgorithmWithParameterMapping();

        //! Returns string class name for a given type.
        static string            GetClassTypeFromDataType( int type );

        //! Returns string names used in XML configuration files for input port parameters.
        virtual string           GetInputPortName( int port );

        //! All ports must be initialized with InitializeInputPort BEFORE this method is called.
        virtual int              FillInputPortInformation( int port, vtkInformation* info );

        //! Returns the port number for a given parameter string.
        virtual int              GetInputPortNumber( string name );

        //! This method sets up the inputs for FillInputPortInformation. MUST BE CALLED IN CONSTRUCTOR
        virtual void             InitializeInputPort( int port, string name, int type );
        virtual int              GetInputPortType( int port );

        virtual void             SetDoubleParameter( int port, double value );
        virtual double           GetDoubleParameter( int port );

        virtual void             SetIntParameter( int port, int value );
        virtual int              GetIntParameter( int port );

        virtual void             SetStringParameter( int port, string value );
        virtual string           GetStringParameter( int port);

        //! Setter that converts a filename into an svkImageData object
        virtual void             SetMRImageParameter( int port, string filename );
        virtual svkMriImageData* GetMRImageParameter( int port);

        //! Stores the names for each parameter. Used to search the XML and print the state.
        vector<string> inputPortNames;

        //! Stores the types for each parameter. Used by FillInputPortInformation to determine types.
        vector<int>    inputPortTypes;

    private:

};


}   //svk


#endif //SVK_IMAGE_ALGORITHM_WITH_PARAMETER_MAPPING_H

