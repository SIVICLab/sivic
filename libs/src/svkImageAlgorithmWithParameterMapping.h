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
#include <svkMrsImageData.h>
#include <svkImageAlgorithm.h>
#include <svkDcmHeader.h>
#include <vtkImageThreshold.h>
#include <svkImageAlgorithmExecuter.h>
#include <vtkCharArray.h>
#include <svkUtils.h>


namespace svk {


using namespace std;


/*! 
 *  Class to manage copying svkImageData as part of a pipeline, 
 *  updating the header and provenance.   
 */
class svkImageAlgorithmWithParameterMapping : public svkImageAlgorithm
{

    public:


        // Initializes the vtkDataObjects and vtkDataArrays to hold the input port parameters.
        virtual void           SetupParameterPorts( ) = 0;

        /*
         * Sets the number of input ports, the first input port parameter, and the number of
         * parameters and then it calls SetupParameterPorts.
         */
        virtual void           SetNumberOfInputAndParameterPorts(int numberOfInputPorts, int numberOfParameters );

        // Parses XML and convert it into input port parameters
        virtual void           SetParametersFromXML( vtkXMLDataElement* element );

        // Returns string names used in XML for input port parameters
        virtual string         GetParameterName( int port );



        void                   PrintSelf( ostream &os, vtkIndent indent );

    protected:

        svkImageAlgorithmWithParameterMapping();
        ~svkImageAlgorithmWithParameterMapping();

        virtual int            FillInputPortInformation( int vtkNotUsed(port), vtkInformation* info );

        // Returns the port number for a given parameter string
        virtual int            GetParameterPort( string parameterName );

        virtual void           InitializeParameterPort( int port, string name, int type );
        virtual int            GetParameterPortType( int port );

        virtual void           SetDoubleParameter( int port, double value );
        virtual double         GetDoubleParameter( int port );

        virtual void           SetIntParameter( int port, int value );
        virtual int            GetIntParameter( int port );

        virtual void           SetStringParameter( int port, string value );
        virtual string         GetStringParameter( int port);

        int            firstParameterPort;
        int            numberOfParameters;
        vector<string> parameterNames;

    private:



};


}   //svk


#endif //SVK_IMAGE_ALGORITHM_WITH_PARAMETER_MAPPING_H

