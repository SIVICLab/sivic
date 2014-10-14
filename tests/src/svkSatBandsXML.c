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
#include <stdio.h>

 
int main(const int argc, const char **argv)
{
    int   status; 
    void* xml = svkSatBandsXML_New( argv[1], &status );
    char* outputfile = argv[2]; 

    int numberPressBoxSats = svkSatBandsXML_GetNumberOfPressBoxSats( xml ); 
    printf("Number of PressBox Sats = %d\n", numberPressBoxSats); 

    int numberAutoSats = svkSatBandsXML_GetNumberOfAutoSats( xml ); 
    printf("Number of Auto Sats = %d\n", numberAutoSats); 


    int     satNumber; 
    float   normalX;
    float   normalY;
    float   normalZ;
    float   thickness;
    float   distance;

    for ( satNumber = 1; satNumber <= numberPressBoxSats; satNumber++ ) {

        svkSatBandsXML_GetPressBoxSat( xml, satNumber, &normalX, &normalY, &normalZ, &thickness, &distance ); 

        printf("number:    %d\n", satNumber ); 
        printf("normalX:   %f\n", normalX ); 
        printf("normalY:   %f\n", normalY ); 
        printf("normalZ:   %f\n", normalZ); 
        printf("thickness: %f\n", thickness); 
        printf("distance:  %f\n", distance ); 
        printf("\n\n"); 
    }

    for ( satNumber = 1; satNumber <= numberAutoSats; satNumber++ ) {

        svkSatBandsXML_GetAutoSat( xml, satNumber, &normalX, &normalY, &normalZ, &thickness, &distance ); 

        printf("number:    %d\n", satNumber ); 
        printf("normalX:   %f\n", normalX ); 
        printf("normalY:   %f\n", normalY ); 
        printf("normalZ:   %f\n", normalZ); 
        printf("thickness: %f\n", thickness); 
        printf("distance:  %f\n", distance ); 
        printf("\n\n"); 
    }

    FILE* f = fopen( outputfile, "w" ); 
    if (f == NULL) {
        printf("Error opening file!\n");
        exit(1);
    }


    float boxCenter[3];
    float boxThickness[3];
    float boxAngles[3];
    svkSatBandsXML_GetPRESSBoxParameters(xml, boxCenter, boxThickness, boxAngles);

    printf("\n\n");
    printf("PRESS: origin:   %f %f %f\n", boxCenter[0], boxCenter[1], boxCenter[2]);
    printf("PRESS: size:     %f %f %f\n", boxThickness[0], boxThickness[1], boxThickness[2]);
    printf("PRESS: angles:   %f %f %f\n", boxAngles[0], boxAngles[1], boxAngles[2]);
    fprintf(f, "PRESS: origin:   %f %f %f\n", boxCenter[0], boxCenter[1], boxCenter[2]);
    fprintf(f, "PRESS: size:     %f %f %f\n", boxThickness[0], boxThickness[1], boxThickness[2]);
    fprintf(f, "PRESS: angles:   %f %f %f\n", boxAngles[0], boxAngles[1], boxAngles[2]);


    printf("\n\n");
    for ( satNumber = 1; satNumber <= numberAutoSats; satNumber++ ) {

        float angles[3];
        float xmlthickness;
        float xmldistance;
        svkSatBandsXML_GetAutoSatParameters(xml, satNumber, angles, &xmlthickness, &xmldistance);
        printf("SVK(SATBANDS): a = %f, b = %f, d = %f t = %f w = %f\n", 
                angles[2], angles[1], xmldistance, xmlthickness, 0);
        fprintf(f, "SVK(SATBANDS): a = %f, b = %f, d = %f t = %f w = %f\n", 
                angles[2], angles[1], xmldistance, xmlthickness, 0);
    }
    
    fclose(f);

    return (0);
}


