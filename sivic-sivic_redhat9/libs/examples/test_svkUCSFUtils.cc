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

#include <svkUCSFUtils.h>
#include <string>

using namespace svk;

int main ( int argc, char** argv )
{
    string metaboliteName( "CRE (area)" );
    string postfixName( "r02" );
    string spectraFileName("t5083_fbcomb_dif_cp_cor_back.ddf");
    string spectraWithPath("/home/bolson/sivic_data/t5083/spectra/t5083_fbcomb_dif_cp_cor_back.ddf");
    string metaboliteFileName("t5083_fbcomb_dif_cp_corr01.idf");
    string metaboliteWithPath("/home/bolson/sivic_data/t5083/spectra/t5083_fbcomb_dif_cp_cor_met/t5083_fbcomb_dif_cp_corr01.idf");
    bool includePath = true;
    cout <<"----------------------------------------------------------------------"<< endl;
    cout <<"The postfix for: " << metaboliteName << " is: " <<  svkUCSFUtils::GetMetabolitePostfix( metaboliteName ) << endl;
    cout <<"----------------------------------------------------------------------"<< endl << endl;

    cout <<"----------------------------------------------------------------------"<< endl;
    cout <<"The metabolite for postfix: " << postfixName << " is: " 
         <<  svkUCSFUtils::GetMetaboliteFromPostfix( postfixName ) << endl;
    cout <<"----------------------------------------------------------------------"<< endl << endl;

    cout <<"----------------------------------------------------------------------"<< endl;
    cout <<"The directory for: " << spectraFileName << " is: " 
         <<  svkUCSFUtils::GetMetaboliteDirectoryName( spectraFileName ) << endl;
    cout <<"----------------------------------------------------------------------"<< endl << endl;

    cout <<"----------------------------------------------------------------------"<< endl;
    cout <<"The file for: " << spectraFileName << " is: " 
         <<  svkUCSFUtils::GetMetaboliteDirectoryName( spectraFileName ) << endl;
    cout <<"----------------------------------------------------------------------"<< endl << endl;

    cout <<"----------------------------------------------------------------------"<< endl;
    cout <<"The metabolite filename for " << endl << spectraFileName << " for " << metaboliteName << " is: " << endl
         << svkUCSFUtils::GetMetaboliteFileName( spectraFileName, metaboliteName ) << endl; 
    cout <<"----------------------------------------------------------------------"<< endl << endl;

    cout <<"----------------------------------------------------------------------"<< endl;
    cout <<"The metabolite filename for " << endl << spectraFileName << " for " << metaboliteName << " with path is: " << endl
         << svkUCSFUtils::GetMetaboliteFileName( spectraFileName, metaboliteName, includePath ) << endl; 
    cout <<"----------------------------------------------------------------------"<< endl << endl;

    cout <<"----------------------------------------------------------------------"<< endl;
    cout <<"The root for " << endl << spectraWithPath << " is: " << endl
         << svkUCSFUtils::GetMetaboliteRoot( spectraWithPath ) << endl; 
    cout <<"----------------------------------------------------------------------"<< endl << endl;

    cout <<"----------------------------------------------------------------------"<< endl;
    cout <<"The metabolite filename for " << endl << spectraWithPath << " for " << metaboliteName << " is: " << endl
         << svkUCSFUtils::GetMetaboliteFileName( spectraWithPath, metaboliteName ) << endl; 
    cout <<"----------------------------------------------------------------------"<< endl << endl;

    cout <<"----------------------------------------------------------------------"<< endl;
    cout <<"The metabolite filename for " << endl << spectraWithPath << " for " << metaboliteName << " with path is: " << endl
         << svkUCSFUtils::GetMetaboliteFileName( spectraWithPath, metaboliteName, includePath ) << endl; 
    cout <<"----------------------------------------------------------------------"<< endl << endl;

    cout <<"----------------------------------------------------------------------"<< endl;
    cout <<"The metabolite for " << endl << metaboliteFileName << " is: " << endl
         << svkUCSFUtils::GetMetaboliteName( metaboliteFileName ) << endl; 
    cout <<"----------------------------------------------------------------------"<< endl << endl;

    cout <<"----------------------------------------------------------------------"<< endl;
    cout <<"The metabolite for " << endl << metaboliteWithPath << " is: " << endl
         << svkUCSFUtils::GetMetaboliteName( metaboliteWithPath ) << endl; 
    cout <<"----------------------------------------------------------------------"<< endl << endl;
    return 0;
}
