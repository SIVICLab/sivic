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



#include <svkSatBandsXML.h>
#include <svkUtils.h>
#include <svkTypeUtils.h>
#include <vtkXMLUtilities.h>
#include <vtkXMLDataParser.h>
#include <vtkMath.h>

using namespace svk;


vtkCxxRevisionMacro(svkSatBandsXML, "$Rev$");
vtkStandardNewMacro(svkSatBandsXML);


/*!
 *
 */
svkSatBandsXML::svkSatBandsXML()
{
#if VTK_DEBUG_ON
    this->DebugOn();
#endif

    vtkDebugMacro(<< this->GetClassName() << "::" << this->GetClassName() << "()");

    this->isVerbose = false; 
    this->satBandsXML = NULL; 
    this->versionElement = NULL;     
    this->pressBoxElement = NULL;     
    this->autoSatsElement = NULL; 
}


/*!
 *
 */
svkSatBandsXML::~svkSatBandsXML()
{
    vtkDebugMacro(<<this->GetClassName()<<"::~"<<this->GetClassName());
}


/*!
 *  set the path/name to xml file.   
 */
int svkSatBandsXML::SetXMLFileName( string xmlFileName )
{
    this->xmlFileName = xmlFileName;  
    // Now we have to remove the old xml file
    this->ClearXMLFile();
    this->satBandsXML = vtkXMLUtilities::ReadElementFromFile( this->xmlFileName.c_str() );
    if (this->satBandsXML == NULL ) { 
        cout << "ERROR, could not parse element from " << this->xmlFileName << endl;
        return 1; 
    }

    // parse the 3 top level elements: 
    this->versionElement = this->satBandsXML->FindNestedElementWithName("version");
    this->pressBoxElement = this->satBandsXML->FindNestedElementWithName("press_box");
    this->autoSatsElement = this->satBandsXML->FindNestedElementWithName("auto_sats");

    if( this->GetDebug() ) {
        //this->satBandsXML->PrintXML(cout, vtkIndent());
        //this->versionElement->PrintXML(cout, vtkIndent());
        //this->pressBoxElement->PrintXML(cout, vtkIndent());
        //this->autoSatsElement->PrintXML(cout, vtkIndent());
    }

    return 0; 

}


/*! Sets the current XML data to NULL
 *  so the file will be re-read.
 */
void  svkSatBandsXML::ClearXMLFile( )
{
    if( this->satBandsXML != NULL ) {
        this->satBandsXML->Delete();
        this->satBandsXML = NULL;
    }

}


/*!
 *  Get the path to the current XML file
 */
string svkSatBandsXML::GetXMLFileName( )
{
    return this->xmlFileName;
}


/*!
 *  Write the integrals for each voxel to stdout. Default is false.  
 */
void svkSatBandsXML::SetVerbose( bool isVerbose )
{
    this->isVerbose = isVerbose; 
}


/*!
 *  Get the specified sat band definition: 
 */
void svkSatBandsXML::GetPressBoxSat(int satNumber, string* label, float *normalX, float *normalY, float* normalZ, float* thickness, float* distance)
{

    vtkXMLDataElement* satBandElement; 
    satBandElement = this->pressBoxElement->FindNestedElementWithNameAndId(
                "sat_band", 
                svkTypeUtils::IntToString(satNumber).c_str() 
            ); 

    this->InitSatBandInfo( satBandElement, label, normalX, normalY, normalZ, thickness, distance); 

}


/*!
 *  Get the specified sat band definition: 
 */
int svkSatBandsXML::GetAutoSat(int satNumber, string* label, float *normalX, float *normalY, float* normalZ, float* thickness, float* distance)
{
    int numSats = this->GetNumberOfAutoSats(); 
    if ( satNumber > numSats || satNumber < 0 ) {
        cerr << "ERROR, satNumber is out of range" << endl;
        return 1; 
    }
    vtkXMLDataElement* satBandElement; 
    satBandElement = this->autoSatsElement->FindNestedElementWithNameAndId(
                "sat_band", 
                svkTypeUtils::IntToString(satNumber).c_str() 
            ); 

    this->InitSatBandInfo( satBandElement, label, normalX, normalY, normalZ, thickness, distance); 
    return 0; 

}


/*
 */
void svkSatBandsXML::InitSatBandInfo( vtkXMLDataElement* satBandElement, string* label, float *normalX, float *normalY, float* normalZ, float* thickness, float* distance) 
{

    *label = string( satBandElement->GetAttribute("label") ); 

    *normalX   = this->GetFloatElementData( satBandElement->
                    FindNestedElementWithName( "normal_x" ) ); 
    *normalY   = this->GetFloatElementData( satBandElement->
                    FindNestedElementWithName( "normal_y" ) ); 
    *normalZ   = this->GetFloatElementData( satBandElement->
                    FindNestedElementWithName( "normal_z" ) ); 
    *thickness = this->GetFloatElementData( satBandElement->
                    FindNestedElementWithName( "thickness" ) ); 
    *distance  = this->GetFloatElementData( satBandElement->
                    FindNestedElementWithName( "distance_from_origin" ) ); 
            
    if( this->GetDebug() ) {
        //satBandElement->PrintXML(cout, vtkIndent());
    }   

}


/*!
 * 
 */
float svkSatBandsXML::GetFloatElementData( vtkXMLDataElement* element )
{
    return svkTypeUtils::StringToFloat(
            string( element->GetCharacterData() )
            ); 
}


/*!
 * 
 */
int svkSatBandsXML::GetNumberOfPressBoxSats() 
{
    int numberOfSats = this->pressBoxElement->GetNumberOfNestedElements(); 
    return numberOfSats; 
}


/*!
 * 
 */
int svkSatBandsXML::GetNumberOfAutoSats() 
{
    int numberOfSats = this->autoSatsElement->GetNumberOfNestedElements(); 
    return numberOfSats; 
}


/*
 *  Which 2 angles take the Z vector (+S) to the specified normal vector? 
 *      1.  first rotate about the x axis (rotation in the sagital yz plane)
 *          This angle is denoted as Beta and is written 2nd in the .dat file.  
 *      2.  next rotate about the z axis
 *          This angle is denoted as gamma and is written fist in the .dat file.  
 */
int svkSatBandsXML::GetAutoSatParameters( int satNumber, float angles[3], float* thickness, float* distance) 
{
    string label; 
    float normal[3]; 
    int status = this->GetAutoSat(satNumber, &label, &normal[0], &normal[1], &normal[2], thickness, distance); 
    if (status) {
        return status; 
    }

    //cout << "NORMAL: " << normal[0] << " " << normal[1] << " " << normal[2] << endl;
    //normal[0] = -1 * normal[0]; 
    //normal[1] = -1 * normal[1]; 
    //normal[2] = -1 * normal[2]; 

    //  Which 2 angles take the Z vector (+S) to the specified normal vector? 

    //  First determine the angle listed in .dat file for use by PSD to 
    //  rotation 100 vector about the x axis (this rotates the Z/S vector 100 
    //  toward the p (+y) vector and is given by the atan(y/z)

    //  Next determine the angle listed in .dat file for use by PSD to 
    //  rotation the vector about the z axis, this is written fist in the dat file 
    //  just to be confusing.
    angles[0] = 0; 
    angles[1] = -1 * acos( normal[2]);
    angles[2] = -1 * atan2( normal[1] , -1 *  normal[0] ); 

    //cout << "ANGLES: " << angles[2] << " " << angles[1] << endl;
    return status; 
}

/*!
 *  Given the xml press box definition, extract the 
 *  location, thickness and orientation  of the box for use by the PSD.  
 */
void svkSatBandsXML::GetPRESSBoxParameters( float pressOrigin[3], float pressThickness[3], float pressAngles[3] ) 
{

    float normals[3][3]; 
    this->InitPressBoxNormals(normals); 

    float distances[3][2]; 
    this->InitPressDistances(normals, distances); 


    //  ===========================================================
    //  Now, use the normals and distances to get the location of the  
    //  center of the box.  
    //      Find the center of each of the normals and add up the 3 three vectors
    //      to find the center of the box. 
    //  ===========================================================
    //  first get the corresponding lengths: 
    for ( int i = 0; i < 3; i++ ) {
        pressOrigin[i] = 0.; 
    }
    for ( int i = 0; i < 3; i++ ) {
        for ( int j = 0; j < 3; j++ ) {
            float tmp = normals[i][j] * (distances[i][0] + distances[i][1]); 
            tmp /= 2;  
            pressOrigin[j] += tmp;  
        }
    }
    cout << "PRESS ORIGIN: " << pressOrigin[0] << " " << pressOrigin[1] << " " << pressOrigin[2] << endl;        


    //  ===========================================================
    //  Now, use the normals and distances to get the thickness. 
    //  Get the distance between the 2 vectors (pythagorean distance)
    //  ===========================================================

    //  3 components of vector representing the difference between the 2 antiparallel 
    //  normals: 
    float differenceVector[3]; 
    float differenceVectorLength; 
    for ( int i = 0; i < 3; i++ ) {
        differenceVectorLength = 0.; 
        for ( int j = 0; j < 3; j++ ) {
            differenceVector[j] = normals[i][j] * (distances[i][0] - distances[i][1]); 
            differenceVectorLength += differenceVector[j] * differenceVector[j]; 
        }
        pressThickness[i] = pow(differenceVectorLength, .5);  
    }
    cout << "PRESS Thickness: " << pressThickness[0] << " " 
            << pressThickness[1] << " " << pressThickness[2] << endl;        


    //  ===========================================================
    //  Finally, get the 3 Euler angles from the normals array (DCM). 
    //  http://en.wikipedia.org/wiki/Rotation_formalisms_in_three_dimensions#Rotation_matrix_.E2.86.94_Euler_angles
    //      Note here that the rotation matrix, DCM = AzAyAx, so 
    //          Ax-> Psi is rotation about L 
    //          Ay-> Theta is rotation about P 
    //          Az-> Phi is rotation about S
    //  ===========================================================
    for ( int i = 0; i < 3; i++ ) {
        cout << "NORMALS ANGLES: " << normals[i][0] << " " 
            << normals[i][1] << " " << normals[i][2] << endl;        
    }

    this->LPSToRAS(normals); 
    this->RotationMatrixToEulerAngles( normals, pressAngles ); 
    
    cout << "PRESS EULERS: " << pressAngles[0] << " " << pressAngles[1] << " " << pressAngles[2] << endl;        

    return; 
}


/*
 *  Compute the Euler angles from the rotation matrix (i.e. dcos, or matrix of normals)
 *  Based on the formulas in the following article that assumes Psi is rotation around X, 
 *  theta about y and phi about z:
 *      Computing Euler angles from a rotation matrix
 *      Gregory G. Slabaugh
 *      Retrieved from Google Scholar Citations on August 6, 2000
 *  Return angle0 -> psi   = rotation about X
 *  Return angle1 -> theta = rotation about Y
 *  Return angle2 -> phi   = rotation about Z 
 */
void svkSatBandsXML::RotationMatrixToEulerAngles( float normals[3][3], float eulerAngles[3]  ) 
{

    float psi;
    float theta; 
    float phi; 

    if ( fabs( normals[2][0] ) != 1 ) {


        cout << "EULER one " << endl;
        float psi1; 
        float theta1; 
        float phi1; 
        float psi2; 
        float theta2; 
        float phi2; 
        theta1 = -1 * asin( normals[2][0] ); 
        theta2 = vtkMath::Pi() - theta1; 
        float cosTheta1 = cos( theta1 ); 
        float cosTheta2 = cos( theta2 ); 
        //cout << "n21 / n22 " << normals[2][1] / cosTheta1 << " " <<  normals[2][2]/cosTheta1 << endl;
        //cout << "n21 / n22 " << normals[2][1] / cosTheta2 << " " <<  normals[2][2]/cosTheta2 << endl;
        psi1   = atan2( ( normals[2][1]/cosTheta1 ) , ( normals[2][2]/cosTheta1 ) ); 
        psi2   = atan2( ( normals[2][1]/cosTheta2 ) , ( normals[2][2]/cosTheta2 ) ); 
        phi1   = atan2( ( normals[1][0]/cosTheta1 ) , ( normals[0][0]/cosTheta1 ) ); 
        phi2   = atan2( ( normals[1][0]/cosTheta2 ) , ( normals[0][0]/cosTheta2 ) ); 

        //  Here use the first set of Euler angles from the above citation
        psi = psi1; 
        theta = theta1; 
        phi = phi1; 
        //psi = psi2; 
        //theta = theta2; 
        //phi = phi2; 

        //cout << "psi1 " << psi1 << " psi2 " << psi2 << endl;
        //cout << "phi1 " << phi1 << " phi2 " << phi2 << endl;
        //cout << "theta1 " << theta1 << " theta2 " << theta2 << endl;

    } else {

        cout << "EULER two" << endl;
        phi = vtkMath::Pi(); 
        if ( normals[2][0] == -1 ) {
            theta = vtkMath::Pi()/2; 
            psi = phi + atan2( normals[0][1] , normals[0][2] ); 
        } else {
            theta = -1 * vtkMath::Pi()/2; 
            psi = -1 * phi + atan2( (-1 * normals[0][1] ) , (-1 * normals[0][2]) ); 
        }

    }

    eulerAngles[0] = psi; 
    eulerAngles[1] = theta; 
    eulerAngles[2] = phi; 

}

/*
 *  Initialized a set of normal vectors from the press box description in the XML file. 
 *  The returned array consists of 3 normal vectors, 1 in each row, and ordered from 
 *  Rmajor, Amajor, Smajor. 
 *      normals[0][0-2]  -> normal vector primarily along +/-L vector
 *      normals[1][0-2]  -> normal vector primarily along +/-P vector
 *      normals[2][0-2]  -> normal vector primarily along +/-S vector
 */
void svkSatBandsXML::InitPressBoxNormals( float normals[3][3] ) 
{
    //  ===========================================================
    //  First, the press box sides need to be sorted into pairs 
    //  with parallel/antiparallel normals: 
    //      - get a set of 3 unique normal vectors: 
    //  ===========================================================
    for ( int i = 0; i < 3; i++ ) {
        for ( int j = 0; j < 3; j++ ) {
            normals[i][j] = 0;        
        }
    }

    int numNormalsInitialized = 0; 
    for ( int satNumber = 1; satNumber <= 6; satNumber++ ) {

        string label;     
        float normal[3]; 
        float thickness; 
        float distance; 
        this->GetPressBoxSat(
                    satNumber, 
                    &label, 
                    &normal[0], 
                    &normal[1], 
                    &normal[2], 
                    &thickness, 
                    &distance
                ); 

        if ( this->IsNormalUnique( normal, normals) )  {
            normals[numNormalsInitialized][0] = normal[0]; 
            normals[numNormalsInitialized][1] = normal[1]; 
            normals[numNormalsInitialized][2] = normal[2]; 
            numNormalsInitialized++; 
        }

    }

    if ( numNormalsInitialized != 3 ) {
        cerr << "PRESS BOX NORMALS NOT INITIALIZED" << endl;
        exit(1); 
    } 

    //  finally, sort the normals into order: 
    //      0, - predominantly RL direction
    //      1, - predominantly AP direction
    //      2, - predominantly SI direction
    //  dot product along each axis(LPS), 100, 010, 001
    float normalsTmp[3][3]; 
    //  dotLPS: 
    //      indicates which axis the normal vector (row) 
    //      is along,  L(0), P(1), or S(2). 
    int dotLPS[3];  
    float lpsNormals[3][3]; 
    lpsNormals[0][0] = 1; 
    lpsNormals[0][1] = 0;  
    lpsNormals[0][2] = 0;  
    lpsNormals[1][0] = 0;  
    lpsNormals[1][1] = 1;  
    lpsNormals[1][2] = 0;  
    lpsNormals[2][0] = 0;  
    lpsNormals[2][1] = 0;  
    lpsNormals[2][2] = 1;  
    for ( int i = 0; i < 3; i++ ) { // loop over L, P, S vector
        dotLPS[i] = 0;     
        float maxDot = 0.; 
        for ( int j = 0; j < 3; j++ ) { // loop over 3 normal vectors
            float tmp = fabs( vtkMath::Dot( lpsNormals[i], normals[j] ) ) ;
            if ( tmp >= maxDot ) {
                maxDot = tmp;
                dotLPS[i] = j; 
            }
        }
    }
    //  Now sort the normals row into the correct order, Lmajor, Pmajor, Smajor: 
    for ( int i = 0; i < 3; i++ ) {
        normalsTmp[i][0] = normals[ dotLPS[i] ][0]; 
        normalsTmp[i][1] = normals[ dotLPS[i] ][1]; 
        normalsTmp[i][2] = normals[ dotLPS[i] ][2]; 
    }
    for ( int i = 0; i < 3; i++ ) {
        normals[i][0] = normalsTmp[i][0]; 
        normals[i][1] = normalsTmp[i][1]; 
        normals[i][2] = normalsTmp[i][2]; 
    }


    if( this->GetDebug() ) {
        for ( int i = 0; i < 3; i++ ) {
            cout << "NORMALS LPS: " << normals[i][0] << " " 
                        << normals[i][1] << " " << normals[i][2] << endl;        
        }
    }

}


/*
 *  Make the PRESS box normals orthogonal.  Use the convention that 
 *      - the Z normal remains as defined.   
 *      - the X is defined as the cross product of Z and Y. 
 *      - the Y is defined as the cross product of Z and X. 
 *      * note that there are 6 normals, one for each face.  The convention is: 
 *          1 => most Anterior
 *          2 => most Posterior 
 *          3 => most Superior  
 *          4 => most Inferior  
 *          5 => most Right 
 *          6 => most Left  
 */
void svkSatBandsXML::MakePRESSOrthonormal( float normals[3][3] ) 
{
    float orthoNormals[3][3]; 
    //  Z normal is the reference: 
    orthoNormals[2][0] = normals[2][0]; 
    orthoNormals[2][1] = normals[2][1]; 
    orthoNormals[2][2] = normals[2][2]; 

    //  X normal is -1 * Z x Y
    vtkMath::Cross( orthoNormals[2], normals[1], orthoNormals[0] ); 
    for (int i = 0; i < 3; i++ ) {
        orthoNormals[0][i] = -1 * orthoNormals[0][i]; 
    }

    //  X normal is -1 * Z x X
    vtkMath::Cross( orthoNormals[2], normals[0], orthoNormals[1] ); 


    for (int i = 0; i < 3; i++ ) {
        for (int j = 0; j < 3; j++ ) {
            normals[i][j] = orthoNormals[i][j]; 
        }
    }
    
    //% boxPlane_patient(3).normal will be keep as it is
    //normalZ = boxPlane_patient(3).normal; 
    //normalX = -cross(normalZ, boxPlane_patient(2).normal);
    //normalY = cross(normalZ, normalX);

    //boxPlane_patient(2).normal = normalY;
    //if boxPlane_patient(1).normal(2)*boxPlane_patient(2).normal(2) < 0
        //boxPlane_patient(1).normal = -normalY;
    //else
        //boxPlane_patient(1).normal = normalY;
    //end
//
    //if boxPlane_patient(3).normal(3)*boxPlane_patient(4).normal(3) < 0
        //boxPlane_patient(4).normal = -normalZ;
    //else
        //boxPlane_patient(4).normal = normalZ;
    //end
//
    //boxPlane_patient(5).normal = normalX;
    //if boxPlane_patient(5).normal(1)*boxPlane_patient(6).normal(1) < 0
        //boxPlane_patient(6).normal = -normalX;
    //else
        //boxPlane_patient(6).normal = normalX;
    //end 
}


/*
 *  Converts an LPS rotation matrix into an RAS
 *  frame: 
 */
void svkSatBandsXML::LPSToRAS(float normals[3][3])
{
    //  convert from LPS to RAS
    //
    //                      L       P       S
    //                  --------------------------
    //      row_norm     |
    //      col_nor      |
    //      slice_norm   |
    for ( int i = 0; i < 3; i++ ) {
        for ( int j = 0; j < 2; j++ ) {
            if ( normals[i][j] != 0 ) {
                normals[i][j] = -1 * normals[i][j]; 
            }
        }
    }
}

/*
 *  Init an array of distances from the press box xml file: 
 *  Each row is the distance from the origin along the corresponding
 *  normal vector, and antiparallel to the normal vector: 
 *      distances[0][0,1]  -> distance from origin parrallel to Lmajor normal vector, antiparallel to
 *      distances[0][0,1]  -> distance from origin parrallel to Pmajor normal vector, antiparallel to
 *      distances[0][0,1]  -> distance from origin parrallel to Smajor normal vector, antiparallel to
 *      
 */
void svkSatBandsXML::InitPressDistances(float normals[3][3], float distances[3][2]) 
{

    for ( int i = 0; i < 3; i ++ ) {
        for ( int j = 0; j < 2; j++ ) {
            distances[i][j] = VTK_FLOAT_MAX; 
        }
    }

    //  Now match up the corresponding distances to the appropriate row of the normals array. 
    //  Distances are grouped in pairs for each normal vector.  The
    //  first distance in each tuple is the distance along the normal, the 2nd is the 
    //  corresponding antiparallel distance: 
    int numDistancesInitialized = 0; 
    for ( int normalIndex = 0; normalIndex < 3; normalIndex++ ) {
        for ( int satNumber = 1; satNumber <= 6; satNumber++ ) {

            string label;     
            float normal[3]; 
            float thickness; 
            float distance; 
            this->GetPressBoxSat(
                        satNumber, 
                        &label, 
                        &normal[0], 
                        &normal[1], 
                        &normal[2], 
                        &thickness, 
                        &distance
                    ); 
            cout << "CHECK: " << normalIndex << " " << satNumber << " " << distance << endl;
            float dot = vtkMath::Dot( normal, normals[normalIndex] );
            //  some tolerance, but close to 1: 
            //  if parallel, then assign to first of tuple
            if ( dot  >= .99 || dot <= -.99 ) {

                int distanceIndex = 0; 
                if ( distances[normalIndex][0] != VTK_FLOAT_MAX ) {    
                    distanceIndex = 1; 
                }
                
                if ( dot  >= -.99 ) {
                    distances[normalIndex][distanceIndex] = distance; 
                    numDistancesInitialized++;     
                } else if ( dot  <= -.99 ) {
                    distances[normalIndex][distanceIndex] = -1 * distance; 
                    numDistancesInitialized++;     
                }

            }

        }
    }

    if ( numDistancesInitialized != 6 ) {
        cerr << "PRESS BOX DISTANCES NOT INITIALIZED" << numDistancesInitialized << endl;
        exit(1); 
    } else {
        for ( int normalIndex = 0; normalIndex < 3; normalIndex++ ) {
            cout << "Distance " << distances[normalIndex][0] << " -> " << distances[normalIndex][1] << endl;
        }
    }
}


/*!
 *  Compares the dot product of the current normal to the existing normals. 
 *  if it's parallel to an existing normal, then it's not unique and the normal
 *  already exists in the normals array.
 */
bool svkSatBandsXML::IsNormalUnique( float normal[3], float normals[3][3]) 
{        
    bool isUnique = true; 
    for ( int i = 0; i < 3; i++ ) {
        float dot = fabs( vtkMath::Dot( normal, normals[i] ) ) ;
        //  some tolerance, but close to 1: 
        if ( fabs( dot ) >= .99 ) {
            isUnique = false; 
        }
    }
    return isUnique; 
}


/*! 
 * External C interface: 
 */
void* svkSatBandsXML_New(char* xmlFileName, int *status)
{
    svkSatBandsXML* xml = svkSatBandsXML::New();     
    *status = xml->SetXMLFileName(xmlFileName); 
    if (*status == 1 ) {
        xml->Delete(); 
        xml = NULL; 
    }
    return ((void*)xml); 
}; 


void* svkSatBandsXML_Delete( void* xml )                   
{
    ((svkSatBandsXML*)xml)->Delete(); 
}


/*!
 * 
 */
int svkSatBandsXML_GetNumberOfPressBoxSats( void* xml ) 
{
    return ((svkSatBandsXML*)xml)->GetNumberOfPressBoxSats(); 
}


/*!
 * 
 */
int svkSatBandsXML_GetNumberOfAutoSats( void* xml ) 
{
    return ((svkSatBandsXML*)xml)->GetNumberOfAutoSats(); 
}



/*!
 * 
 */
void  svkSatBandsXML_GetPressBoxSat(void* xml, int satNumber, float* normalX, float* normalY, float* normalZ, float* thickness, float* distance ) 
{
    string label; 
    ((svkSatBandsXML*)xml)->GetPressBoxSat( 
            satNumber, 
            &label, 
            normalX, 
            normalY, 
            normalZ, 
            thickness, 
            distance
        );

}


/*!
 * 
 */
int svkSatBandsXML_GetAutoSat(void* xml, int satNumber, float* normalX, float* normalY, float* normalZ, float* thickness, float* distance ) 
{

    string label; 
    int status = ((svkSatBandsXML*)xml)->GetAutoSat( 
            satNumber, 
            &label, 
            normalX, 
            normalY, 
            normalZ, 
            thickness, 
            distance
    );
    return status; 

}


/*!
 * 
 */
void svkSatBandsXML_GetPRESSBoxParameters(void* xml, float* pressOrigin, float* pressThickness, float* pressAngles )    
{
    ((svkSatBandsXML*)xml)->GetPRESSBoxParameters( pressOrigin, pressThickness, pressAngles ); 
}



/*!
 * 
 */
int svkSatBandsXML_GetAutoSatParameters(void* xml, int satNumber, float* normal, float* thickness, float* distance)    
{
    int status = ((svkSatBandsXML*)xml)->GetAutoSatParameters( satNumber, normal, thickness, distance); 
    return status; 
}
