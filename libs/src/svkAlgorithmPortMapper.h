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


#ifndef SVK_ALGORITHM_PORT_MAPPER_H
#define SVK_ALGORITHM_PORT_MAPPER_H


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
#include <svkXML.h>

namespace svk {


using namespace std;


/*! 
 * This class is a helper class that can be used with any vtkAlgorithm. It provides an interface
 * for using the input ports for a vtkAlgorithm to store all the parameters necessary to execute the
 * algorithm. Additionally an XML file can be supplied with the names given to each input port as
 * tags that surround the data to be given to that port. This way an algorithm can be configured
 * and run using a simple XML file. If the data type given is an image, a filename will be assumed
 * and the output of the appropriate reader will be set into the given port. For example if you had
 * four inputs to an algorithm called svkMyAlgorithm  and the input ports were named INPUT_IMAGE,
 * PARAMETER_ONE, PARAMETER_TWO, and PARAMETER_THREE, then your XML would appear
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
 * </svkMyAlgorithm>
 *
 *
 *
 * This class uses vtkDataObject-wrapped types (svkDouble, svkInt, svkString, svkBool, svkXML) to
 * fill input ports that are not of type vtkDataObject.
 */
class svkAlgorithmPortMapper : public vtkObject
{

    public:

        typedef enum {
            SVK_BOOL = 0,
            SVK_INT,
            SVK_DOUBLE ,
            SVK_STRING,
            SVK_MR_IMAGE_DATA,
            SVK_4D_IMAGE_DATA,
            SVK_XML
        } svkXMLDataType;

        static svkAlgorithmPortMapper* New();
        vtkTypeRevisionMacro( svkAlgorithmPortMapper, vtkObject);

        //! Set the internal algorithm whos input ports are to be set.
        void             SetAlgorithm( vtkAlgorithm* algo );

        //! This method sets up the inputs for FillInputPortInformation. Must be called before FillInputPortInformation
        void             InitializeInputPort( int port, string name, int type, bool required = true, bool repeatable = false );

        //! Parses an XML element and uses it to set the input ports of the algorithm. Converts image filename strings to svkImageData objects.
        void             SetInputPortsFromXML( vtkXMLDataElement* element );

        //! All ports must be initialized with InitializeInputPort BEFORE this method is called.
        int              FillInputPortInformation( int port, vtkInformation* info );

        //! Basic setter. Wraps value in vtkDataObject subclass
        void             SetDoubleInputPortValue( int port, double value );
        svkDouble*       GetDoubleInputPortValue( int port );

        //! Basic setter. Wraps value in vtkDataObject subclass
        void             SetIntInputPortValue( int port, int value );
        //! Basic getter.
        svkInt*          GetIntInputPortValue( int port );

        //! Basic setter. Wraps value in vtkDataObject subclass
        void             SetStringInputPortValue( int port, string value );
        //! Basic getter.
        svkString*       GetStringInputPortValue( int port);

        //! Basic setter. Wraps value in vtkDataObject subclass
        void             SetBoolInputPortValue( int port, bool value );
        //! Basic getter.
        svkBool*         GetBoolInputPortValue( int port);

        //! Basic setter. Wraps value in vtkDataObject subclass
        void             SetXMLInputPortValue( int port, vtkXMLDataElement* value );
        //! Basic getter.
        svkXML*          GetXMLInputPortValue( int port);

        //! Setter that converts a filename into an svkImageData object
        void             SetMRImageInputPortValue( int port, string filename );

        //! Basic getter.
        svkMriImageData* GetMRImageInputPortValue( int port);

        //! Returns string names used to identify the input port.
        string           GetInputPortName( int port );

        //! Returns string names used in XML configuration files for input port.
        string           GetXMLTagForInputPort( int port );

        //! Returns string names used in XML configuration files for input port.
        string           GetXMLTagForAlgorithm( );

        //! Returns true if the port is required.
        bool             GetInputPortRequired( int port );

        //! Returns true if the port is repeatable.
        bool             GetInputPortRepeatable( int port );

        //! Get the prefix used for the port definitions in xml
        string           GetXMLInputPortPrefix( );

        //! Set the prefix used for the port definitions in xml
        void             SetXMLInputPortPrefix( string prefix );

        //! Get the prefix used for the port definitions in xml
        string           GetXMLAlgorithmPrefix( );

        //! Set the prefix used for the port definitions in xml
        void             SetXMLAlgorithmPrefix( string prefix );

        //! Prints all input parameters set.
        void             PrintSelf( ostream &os, vtkIndent indent );

        //! Write the XSD for this port mappper's current initialization
        string           GetXSD( );

        //! Handles getting data object input appropriately.
        virtual vtkDataObject*   GetAlgorithmInputPort( int port, int index = 0 );

        //! Handles setting data object input appropriately.
        virtual vtkDataObject*   SetAlgorithmInputPort( int port, vtkDataObject* input );

    protected:

        svkAlgorithmPortMapper();
        ~svkAlgorithmPortMapper();

        //! Returns string class name for a given type.
        static string    GetClassTypeFromDataType( int type );

        //! Returns the port number for a given parameter string.
        int              GetInputPortNumber( string name );

        //! Gets the type for a given input port
        int              GetInputPortType( int port );

        //! Stores the names for each parameter. Used to search the XML and print the state.
        vector<string>   inputPortNames;

        //! Stores the types for each parameter. Used by FillInputPortInformation to determine types.
        vector<int>      inputPortTypes;

        //! Stores whether or not a port is required. Used by FillInputPortInformation to determine types.
        vector<bool>     inputPortRequired;

        //! Stores whether or not a port is repeatable. Used by FillInputPortInformation to determine types.
        vector<bool>     inputPortRepeatable;

        //! The XML prefix used for the arguments
        string           inputPortPrefix;

        //! The XML prefix used for the algorithm
        string           algorithmPrefix;


    private:



        //! Internal algorithm
        vtkAlgorithm* algo;

};


}   //svk


#endif //SVK_ALGORITHM_PORT_MAPPER_H

