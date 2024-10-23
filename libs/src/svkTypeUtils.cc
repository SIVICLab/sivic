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


#include <svkTypeUtils.h>
#include </usr/include/vtk/vtkGlobFileNames.h>

using namespace svk;

//vtkCxxRevisionMacro(svkTypeUtils, "$Rev$");
vtkStandardNewMacro(svkTypeUtils);

//! Constructor
svkTypeUtils::svkTypeUtils()
{
}


//! Destructor
svkTypeUtils::~svkTypeUtils()
{
}


/*!
 *  Converts a string to a double.
 *
 * @param doubleString
 * @return
 */
double svkTypeUtils::StringToDouble( string doubleString )
{
    istringstream* iss = new istringstream();
    double value = 0.0;
    iss->str( doubleString );
    *iss >> value;
    delete iss;
    return value;
}


/*!
 *
 * @param doubleString
 * @return
 */
float svkTypeUtils::StringToFloat( string doubleString )
{
    istringstream* iss = new istringstream();
    float value = 0.0;
    iss->str( doubleString );
    *iss >> value;
    delete iss;
    return value;
}


/*!
 *
 * @param intString
 * @return
 */
int svkTypeUtils::StringToInt( string intString )
{
    istringstream* iss = new istringstream();
    int value;
    iss->str( intString );
    *iss >> value;
    delete iss;
    return value;
}


/*!
 *
 * @param intString
 * @return
 */
long int svkTypeUtils::StringToLInt( string longIntString )
{
    istringstream* iss = new istringstream();
    long int value;
    iss->str( longIntString );
    *iss >> value;
    delete iss;
    return value;
}


/*!
 *  @param intVal
 *  @return string equivalent
 */
string svkTypeUtils::IntToString( long intVal )
{
    ostringstream intStream;
    intStream << intVal;
    return intStream.str();
}


/*!
 *  @param doubleVal
 *  @return string equivalent
 */
string svkTypeUtils::DoubleToString( double doubleVal, int precision )
{
    ostringstream intStream;
    if( precision > 0 ) {
        intStream << setprecision(precision);
    }
    intStream << doubleVal;
    return intStream.str();
}
