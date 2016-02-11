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
 *  Utility application for converting between legacy and XML PSD prescription file formats. 
 *
 */

#ifdef WIN32
extern "C" {
#include <getopt.h>
}
#else
#include <getopt.h>
#include <unistd.h>
#endif
#include <svkUtils.h>
#include <svkSatBandsXML.h>
#include <vtkXMLUtilities.h>
#include <vtkIndent.h>


using namespace svk;


int main (int argc, char** argv)
{

    string usemsg("\n") ; 
    usemsg += "Version " + string(SVK_RELEASE_VERSION) + "\n";   
    usemsg += "svk_convert_xml_format -i input_prescription_xml -o xmlver20output      \n"; 
    usemsg += "                             [ -vh ]                                                     \n";
    usemsg += "                                                                                         \n";  
    usemsg += "   -i            input_prescription_root     Root name of input prescription file.       \n"; 
    usemsg += "                                             this is the prefix to:                      \n"; 
    usemsg += "                                                 _press_box.dat or _sat_bands.dat        \n"; 
    usemsg += "   -o            output_prescription_root    Root name of output prescriptin file.       \n";  
    usemsg += "   -x            version_of_xml              Numerical version of the XML file:          \n"; 
    usemsg += "                                             1: 6 plane representation of the PRESS box  \n";
    usemsg += "                                             2: 3 plane representation of the PRESS box  \n"; 
    usemsg += "   -v                                        Verbose output.                             \n";
    usemsg += "   -h                                        Print help mesage.                          \n";  
    usemsg += "                                                                                         \n";  
    usemsg += "Converts the input prescription file to an XML prescription.                             \n";  
    usemsg += "                                                                                         \n";  
    usemsg += "EXAMPLE: svk_psd_prescription_convert -i P12345 -o P12345 -x 1                           \n";  
    usemsg += "this would read in P12345_press_box.dat and P12345_sat_bands.dat and export              \n";  
    usemsg += "P12345_box_satbands_atlasbased.xml defining the PRESS box with 6 planes.                  \n";  
    usemsg += "                                                                                         \n";  

    string  input; 
    string  output; 

    enum FLAG_NAME {
    };


    static struct option long_options[] =
    {
        {0, 0, 0, 0}
    };



    /*
    *   Process flags and arguments
    */
    int i;
    int option_index = 0; 
    while ((i = getopt_long(argc, argv, "i:o:h", long_options, &option_index)) != EOF) {
        switch (i) {
            case 'i':
                input.assign( optarg );
                break;
            case 'o':
                output.assign(optarg);
                break;
           
            default:
                ;
        }
    }

    argc -= optind;
    argv += optind;

    /*
     *  Validate required input: 
     */
    if ( argc != 0 || input.length() == 0 || output.length() == 0 ) { 
        cout << usemsg << endl;
        exit(1); 
    }

    cout << "hhereee1" << endl;

    svkSatBandsXML* xml = svkSatBandsXML::New();

    int status = xml->SetXMLFileName( input.c_str() );
    cout << "hhereee" << endl;
    xml->WriteXMLFile( output.c_str() ); 

    xml->Delete();

    return 0; 
}

