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


#ifndef SVK_XML_INPUT_INTERPRETER_H
#define SVK_XML_INPUT_INTERPRETER_H


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
#include <svkBool.h>

namespace svk {


using namespace std;


/*! 
 * This class is a helper class that can be used with any vtkAlgorithm. It provides an interface
 * for using the input ports for the vtkAlgorithm to store all the parameters necessary to execute the
 * algorithm. Additionally an XML file can be supplied with the names given to each input port as
 * tags that surround the data to be given to that port. This way an algorithm can be configured
 * and run using a simple XML. If the data type given is an image, a filename will be assumed and
 * and the output of the appropriate reader will be set into the given port. For example if you had
 * five inputs to an algorithm called like svkMyAlgorithm called and the input ports were named
 * INPUT_IMAGE, PARAMETER_ONE, PARAMETER_TWO, and PARAMETER_THREE, then your XML would appear
 * something like this:
 *
 * <svkMyAlgorithm>
 *
 *   <INPUT_IMAGE>image.idf</INPUT_IMAGE>
 *
 *   <PARAMETER_ONE>1</PARAMETER_ONE>
 *
 *   <PARAMETER_TWO>1</PARAMETER_TWO>
 *
 *   <PARAMETER_THREE>Data Mask</PARAMETER_THREE>
 *
 * </MyAlgorithm>
 *
 *
 *
 * This class uses vtkDataObject-wrapped primitive types (svkDouble, svkInt, svkString, svkBool) to
 * fill input ports of these types.
 */
class svkXMLInputInterpreter : public vtkObject
{

    public:

        typedef enum {
            SVK_BOOL = 0,
            SVK_INT,
            SVK_DOUBLE ,
            SVK_STRING,
            SVK_MR_IMAGE_DATA,
            SVK_4D_IMAGE_DATA
        } svkXMLDataType;

        static svkXMLInputInterpreter* New();
        vtkTypeRevisionMacro( svkXMLInputInterpreter, vtkObject);

        //! Set the internal algorithm whos input ports are to be set.
        virtual void             SetAlgorithm( vtkAlgorithm* algo );

        //! This method sets up the inputs for FillInputPortInformation. Must be called before FillInputPortInformation
        virtual void             InitializeInputPort( int port, string name, int type );

        //! Parses an XML element and uses it to set the input ports of the algorithm. Converts image filename strings to svkImageData objects.
        virtual void             SetInputPortsFromXML( vtkXMLDataElement* element );

        //! All ports must be initialized with InitializeInputPort BEFORE this method is called.
        virtual int              FillInputPortInformation( int port, vtkInformation* info );

        virtual void             SetDoubleInputPortValue( int port, double value );
        virtual double           GetDoubleInputPortValue( int port );

        virtual void             SetIntInputPortValue( int port, int value );
        virtual int              GetIntInputPortValue( int port );

        virtual void             SetStringInputPortValue( int port, string value );
        virtual string           GetStringInputPortValue( int port);

        virtual void             SetBoolInputPortValue( int port, bool value );
        virtual bool             GetBoolInputPortValue( int port);

        //! Setter that converts a filename into an svkImageData object
        virtual void             SetMRImageInputPortValue( int port, string filename );
        virtual svkMriImageData* GetMRImageInputPortValue( int port);

        //! Prints all input parameters set.
        void                     PrintSelf( ostream &os, vtkIndent indent );

    protected:

        svkXMLInputInterpreter();
        ~svkXMLInputInterpreter();

        //! Returns string class name for a given type.
        static string            GetClassTypeFromDataType( int type );

        //! Returns string names used in XML configuration files for input port parameters.
        virtual string           GetInputPortName( int port );

        //! Returns the port number for a given parameter string.
        virtual int              GetInputPortNumber( string name );

        virtual int              GetInputPortType( int port );

        virtual int              GetNumberOfInputPorts();

        virtual vtkDataObject*   GetInput( int port );

        virtual vtkDataObject*   SetInput( int port, vtkDataObject* input );

        //! Stores the names for each parameter. Used to search the XML and print the state.
        vector<string> inputPortNames;

        //! Stores the types for each parameter. Used by FillInputPortInformation to determine types.
        vector<int>    inputPortTypes;

    private:
        vtkAlgorithm* algo;

};


}   //svk


#endif //SVK_XML_INPUT_INTERPRETER_H

