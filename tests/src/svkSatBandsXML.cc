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
 *
 */


#include <svkSatBandsXML.h>

using namespace svk;

/*
 *  Application for testing data combination algorithms. 
 */
int main (int argc, char** argv)
{

    //  Input file name: 
    string fname(argv[1]);

    svkSatBandsXML* xml = svkSatBandsXML::New();
    xml->SetXMLFileName( fname.c_str() );

    int     satNumber = 1; 
    string  label; 
    float   normalX; 
    float   normalY; 
    float   normalZ; 
    float   thickness; 
    float   distance;
    xml->GetPressBoxSat( satNumber, &label, &normalX, &normalY, &normalZ, &thickness, &distance);  

    cout << "number:    " << satNumber << endl;
    cout << "label:     " << label << endl;
    cout << "normalX:   " << normalX << endl;
    cout << "normalY:   " << normalY << endl;
    cout << "normalZ:   " << normalZ << endl;
    cout << "thickness: " << thickness << endl;
    cout << "distance:  " << distance << endl;

    float pressOrigin[3]; 
    float pressThickness[3]; 
    float pressAngles[3]; 
    xml->GetPRESSBoxParameters(pressOrigin, pressThickness, pressAngles); 

    xml->Delete();

    return 0; 
}

