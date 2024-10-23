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



#include <svkSatBandsXML.h>
#include <svkTypeUtils.h>
#include <svkFileUtils.h>
#include </usr/include/vtk/vtkXMLUtilities.h>
#include </usr/include/vtk/vtkXMLDataParser.h>
#include </usr/include/vtk/vtkMath.h>
#include <stdexcept>

using namespace svk;



//vtkCxxRevisionMacro(svkSatBandsXML, "$Rev$");
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
    
    this->versionNumber = 1.0;
}


/*!
 *
 */
svkSatBandsXML::~svkSatBandsXML()
{
    vtkDebugMacro(<<this->GetClassName()<<"::~"<<this->GetClassName());
}


/*!
 *  set the path/name to xml file.  Parses the thre elements: 
 *      versionElement, pressBoxElement, autoSatsElement 
 *      
 */
int svkSatBandsXML::SetXMLFileName( string xmlFileName )
{

    this->xmlFileName = xmlFileName;  
    return this->ParseXML( vtkXMLUtilities::ReadElementFromFile( this->xmlFileName.c_str() ));
}


/*
 *  Tries to parse an XML RX file. 
 */
int svkSatBandsXML::ParseXML( vtkXMLDataElement* satBandsElement )
{
    // Now we have to remove the old xml file
    this->ClearXMLFile();
    this->satBandsXML = satBandsElement;
    if (this->satBandsXML == NULL ) { 
        cout << "ERROR, could not parse element from " << this->xmlFileName << endl;
        return 1; 
    } 

    // parse the 3 top level elements: 
    this->versionElement = this->satBandsXML->FindNestedElementWithName("version");
    this->pressBoxElement = this->satBandsXML->FindNestedElementWithName("press_box");
    this->autoSatsElement = this->satBandsXML->FindNestedElementWithName("auto_sats");

    this->versionNumber = this->GetFloatElementData(this->versionElement);

    if( this->GetDebug() ) {
        this->satBandsXML->PrintXML(cout, vtkIndent());
        this->versionElement->PrintXML(cout, vtkIndent());
        this->pressBoxElement->PrintXML(cout, vtkIndent());
        this->autoSatsElement->PrintXML(cout, vtkIndent());
    }

    return 0; 

}

/*
 *  For backwards compatibility, if no xml version of 
 *  prescription, then check for legacy .dat version of 
 *  press_box and sat_bands and 
 *  convert to the new XML format: 
 *      returns 0 if successful, or if no .dat files found. 
 *      Not every acquisition has a dat file, so this may be OK
 */
int svkSatBandsXML::ConvertDatToXML2( string rootName )
{
    int status = 0; 

    //  If the dat files were parsed, then Create an XML structure in 
    //  memory only: 
    this->satBandsXML     = vtkXMLDataElement::New(); 
    this->satBandsXML->SetName("svk_sat_bands"); 

    vtkXMLDataElement* version = vtkXMLDataElement::New();
    version->SetName("version"); 
    version->SetCharacterData("2.0", 3); 
    this->satBandsXML->AddNestedElement( version ); 

    this->pressBoxElement = vtkXMLDataElement::New();
    this->pressBoxElement->SetName("press_box"); 

    this->autoSatsElement = vtkXMLDataElement::New();
    this->autoSatsElement->SetName("auto_sats"); 

    status = this->InitPressBoxFromDat2( rootName ); 
    if ( status != 0 ) {
        return status; 
    }

    status = this->InitSatsFromDat( rootName ); 
    if ( status != 0 ) {
        return status; 
    }

    this->satBandsXML->AddNestedElement( this->pressBoxElement ); 
    this->satBandsXML->AddNestedElement( this->autoSatsElement ); 

    return status; 
}


/*
 *  For backwards compatibility, if no xml version of 
 *  prescription, then check for legacy .dat version of 
 *  press_box and sat_bands and 
 *  convert to the new XML format: 
 *      returns 0 if successful, or if no .dat files found. 
 *      Not every acquisition has a dat file, so this may be OK
 */
int svkSatBandsXML::ConvertDatToXML( string rootName )
{
    int status = 0; 

    //  If the dat files were parsed, then Create an XML structure in 
    //  memory only: 
    this->satBandsXML     = vtkXMLDataElement::New(); 
    this->satBandsXML->SetName("svk_sat_bands"); 

    vtkXMLDataElement* version = vtkXMLDataElement::New();
    version->SetName("version"); 
    version->SetCharacterData("1.0", 3); 
    this->satBandsXML->AddNestedElement( version ); 

    this->pressBoxElement = vtkXMLDataElement::New();
    this->pressBoxElement->SetName("press_box"); 

    this->autoSatsElement = vtkXMLDataElement::New();
    this->autoSatsElement->SetName("auto_sats"); 

    status = this->InitPressBoxFromDat( rootName ); 
    if ( status != 0 ) {
        return status; 
    }

    status = this->InitSatsFromDat( rootName ); 
    if ( status != 0 ) {
        return status; 
    }

    this->satBandsXML->AddNestedElement( this->pressBoxElement ); 
    this->satBandsXML->AddNestedElement( this->autoSatsElement ); 

    return status; 
}

// Use press_box.dat to obtain the PRESS parameters for XML 2
int svkSatBandsXML::InitPressBoxFromDat2( string rootName ) 
{

    int status = 0; 
   
    string suffix = "_press_box.dat";  
    string datFileName = rootName; 
    datFileName.append(suffix); 
    if ( ! svkFileUtils::FilePathExists( datFileName.c_str() ) ) {
        //  No legacy .dat file, so just return 0; 
        cout << "No legacy sat_bands.dat file:  " <<  datFileName << endl;
        return status; 
    } else {
        cout << "Found legacy " << suffix << " file: " <<  datFileName << endl;
    }

    try { 

        ifstream* datFile = new ifstream();
        datFile->exceptions( ifstream::eofbit | ifstream::failbit | ifstream::badbit );
        datFile->open( datFileName.c_str(), ifstream::in );
        if ( ! datFile->is_open() ) {
            throw runtime_error( "Could not open PFile .dat file: " + datFileName);
        } 

        long datFileSize = svkFileUtils::GetFileSize( datFile );

        istringstream* iss = new istringstream();
        string keyString;

        try {

            size_t  position; 
            string  tmp; 
            float   normals[3][3]; 
            float   boxCenter[3]; 
            float   boxSize[3];
            string  planeLabels[3] = {"sagittal", "coronal", "axial"}; 
            string normalString; 
            string labelString;
            string thicknessString;
            string distanceString;
            float distance;
            if ( datFile->tellg() < datFileSize - 1 ) {

                // parse 3 lines of press_box.dat: 

                for ( int i = 0; i < 3; i++ ) {

                    svkFileUtils::ReadLine( datFile, iss); 
                    //cout << "iss: " << iss->str() << endl;
                    tmp.assign( iss->str() ); 

                    // parse 3 values from each line
                    string values[3]; 
                    for ( int j = 0; j < 3; j++ ) {
                        //cout << "TMP: " << tmp << endl;
                        position = tmp.find_first_of(' ');
                        if (position != string::npos) {
                            values[j].assign( tmp.substr(0, position) );
                        } else {   
                            values[j].assign( tmp );
                        }
                        tmp.assign( tmp.substr(position + 1) ); 
                    } 
                    if ( this->GetDebug() )  {
                        cout << "PARSE PB DAT: " << values[0] << " " << values[1] << " " << values[2] << endl;        
                    }

                    //  center of box
                    if ( i == 0 ) {
                        boxCenter[0] = svkTypeUtils::StringToFloat( values[0] ); 
                        boxCenter[1] = svkTypeUtils::StringToFloat( values[1] ); 
                        boxCenter[2] = svkTypeUtils::StringToFloat( values[2] ); 
                    }

                    //  size of box
                    if ( i == 1 ) {
                        boxSize[0] = svkTypeUtils::StringToFloat( values[0] ); 
                        boxSize[1] = svkTypeUtils::StringToFloat( values[1] ); 
                        boxSize[2] = svkTypeUtils::StringToFloat( values[2] ); 
                    }

                    //  orientation of box
                    if ( i == 2 ) {

                        //  Convention from Slaubaugh and  svkSatBandsXML::RotationMatrixToEulerAngles    
                        //  inverse of RotationMatrixToEulerAngles
                        //  angle1 = psi 
                        //  angle2 = theta 
                        //  angle3 = phi 

                        float psi   = svkTypeUtils::StringToFloat( values[0] ); 
                        float theta = svkTypeUtils::StringToFloat( values[1] ); 
                        float phi   = svkTypeUtils::StringToFloat( values[2] ); 
                        // I hate this mystery factor
                        phi -= vtkMath::Pi(); // Added during generation of DAT file (needs to be subtracted to get the XML).
   
                        //  row 0:  
                        normals[0][0] = cos(theta) * cos(phi); 
                        normals[1][0] = sin(psi) * sin(theta) * cos(phi) - cos(psi) * sin(phi); 
                        normals[2][0] = cos(psi) * sin(theta) * cos(phi) + sin(psi) * sin(phi);

                        //  row 1:  
                        normals[0][1] = cos(theta) * sin(phi); 
                        normals[1][1] = sin(psi) * sin(theta) * sin(phi) + cos(psi) * cos(phi); 
                        normals[2][1] = cos(psi) * sin(theta) * sin(phi) - sin(psi) * cos(phi); 

                        //  row 2:  
                        normals[0][2] = -1 * sin(theta); 
                        normals[1][2] = sin(psi) * cos(theta);
                        normals[2][2] = cos(psi) * cos(theta);

                    }
                } 

                if ( this->GetDebug() ) {
                    //  Print normals from euler angles
                    cout << normals[0][0] << " " << normals[1][0] << " " << normals[2][0] << endl;    
                    cout << normals[0][1] << " " << normals[1][1] << " " << normals[2][1] << endl;    
                    cout << normals[0][2] << " " << normals[1][2] << " " << normals[2][2] << endl;    
                }

                for ( int kk = 0; kk<3; kk++){

                    distance = vtkMath::Dot(normals[kk], boxCenter); 

                    // Add these inversions to be conform with MATLAB code generating the normals.
                    if (distance < 0){
	                    if ( kk == 0){
		                    normals[kk][0] = -1* normals[kk][0];
	                    }
	                    else{
		                    normals[kk][1] = -1*normals[kk][1];
		                    normals[kk][2] = -1*normals[kk][2];
	                    }
	                    distance = -1*distance;
                    }

                    //  Use this info to initialize a sat band element in an XML file:
                    vtkXMLDataElement* satBandElement = vtkXMLDataElement::New();
                    satBandElement->SetName("sat_band");
                    satBandElement->SetAttribute( "id", svkTypeUtils::IntToString( kk+1 ).c_str() );
                    satBandElement->SetId( svkTypeUtils::IntToString( kk+1 ).c_str() );
                    satBandElement->SetAttribute("label", planeLabels[kk].c_str() );
                    this->pressBoxElement->AddNestedElement( satBandElement );

                    vtkXMLDataElement* normalXElement = vtkXMLDataElement::New();
                    normalXElement->SetName("normal_x");
                    normalString = svkTypeUtils::DoubleToString(normals[kk][0]);
                    normalXElement->SetCharacterData(normalString.c_str(), normalString.length() );
                    satBandElement->AddNestedElement( normalXElement );

                    vtkXMLDataElement* normalYElement = vtkXMLDataElement::New();
                    normalYElement->SetName("normal_y");
                    normalString = svkTypeUtils::DoubleToString(normals[kk][1]);
                    normalYElement->SetCharacterData(normalString.c_str(), normalString.length() );
                    satBandElement->AddNestedElement( normalYElement );

                    vtkXMLDataElement* normalZElement = vtkXMLDataElement::New();
                    normalZElement->SetName("normal_z");
                    normalString = svkTypeUtils::DoubleToString(normals[kk][2]);
                    normalZElement->SetCharacterData(normalString.c_str(), normalString.length() );
                    satBandElement->AddNestedElement( normalZElement );

                    vtkXMLDataElement* thicknessElement = vtkXMLDataElement::New();
                    thicknessString = svkTypeUtils::DoubleToString(boxSize[kk]);
                    thicknessElement->SetCharacterData(thicknessString.c_str(), thicknessString.length());
                    thicknessElement->SetName("thickness");
                    satBandElement->AddNestedElement( thicknessElement );

                    vtkXMLDataElement* distanceElement = vtkXMLDataElement::New();
                    distanceString = svkTypeUtils::DoubleToString( distance );
                    distanceElement->SetName("distance_from_origin");
                    distanceElement->SetCharacterData(distanceString.c_str(), distanceString.length());
                    satBandElement->AddNestedElement( distanceElement );
                }

            } else {                     
                datFile->seekg(0, ios::end);     
            }

        } catch (const exception& e) {
            if (this->GetDebug()) {
                cout <<  "ERROR reading line: " << e.what() << endl;
            }
            status = -1;  
        }

        datFile->close();

    } catch (const exception& e) {
        cerr << "ERROR opening or reading PFile .dat file: " << e.what() << endl;
    }
    return status; 

}

/*
 *  Attempt to open and parse legacy press_box.dat file and initialize the 
 *  intermal XML data structure from the info in the file.  Sats are required by 
 *  psd to be ordered according to the following convention: 
 *      //  0- anterior  (AP)  
 *      //  1- posterior (AP)
 *      //  2- superior  (SI)
 *      //  3- inferior  (SI)
 *      //  4- left      (LR)
 *      //  5- right     (LR)
 */
int svkSatBandsXML::InitPressBoxFromDat( string rootName ) 
{

    int status = 0; 
   
    string suffix = "_press_box.dat";  
    string datFileName = rootName; 
    datFileName.append(suffix); 
    if ( ! svkFileUtils::FilePathExists( datFileName.c_str() ) ) {
        //  No legacy .dat file, so just return 0; 
        cout << "No legacy sat_bands.dat file:  " <<  datFileName << endl;
        return status; 
    } else {
        cout << "Found legacy " << suffix << " file: " <<  datFileName << endl;
    }

    try { 

        ifstream* datFile = new ifstream();
        datFile->exceptions( ifstream::eofbit | ifstream::failbit | ifstream::badbit );
        datFile->open( datFileName.c_str(), ifstream::in );
        if ( ! datFile->is_open() ) {
            throw runtime_error( "Could not open PFile .dat file: " + datFileName);
        } 

        long datFileSize = svkFileUtils::GetFileSize( datFile );

        istringstream* iss = new istringstream();
        string keyString;

        try {

            size_t  position; 
            string  tmp; 
            float   normals[3][3]; 
            float   boxCenter[3]; 
            float   boxSize[3]; 

            if ( datFile->tellg() < datFileSize - 1 ) {

                // parse 3 lines of press_box.dat: 

                for ( int i = 0; i < 3; i++ ) {

                    svkFileUtils::ReadLine( datFile, iss); 
                    //cout << "iss: " << iss->str() << endl;
                    tmp.assign( iss->str() ); 

                    // parse 3 values from each line
                    string values[3]; 
                    for ( int j = 0; j < 3; j++ ) {
                        //cout << "TMP: " << tmp << endl;
                        position = tmp.find_first_of(' ');
                        if (position != string::npos) {
                            values[j].assign( tmp.substr(0, position) );
                        } else {   
                            values[j].assign( tmp );
                        }
                        tmp.assign( tmp.substr(position + 1) ); 
                    } 
                    if ( this->GetDebug() )  {
                        cout << "PARSE PB DAT: " << values[0] << " " << values[1] << " " << values[2] << endl;        
                    }

                    //  center of box
                    if ( i == 0 ) {
                        boxCenter[0] = svkTypeUtils::StringToFloat( values[0] ); 
                        boxCenter[1] = svkTypeUtils::StringToFloat( values[1] ); 
                        boxCenter[2] = svkTypeUtils::StringToFloat( values[2] ); 
                    }

                    //  size of box
                    if ( i == 1 ) {
                        boxSize[0] = svkTypeUtils::StringToFloat( values[0] ); 
                        boxSize[1] = svkTypeUtils::StringToFloat( values[1] ); 
                        boxSize[2] = svkTypeUtils::StringToFloat( values[2] ); 
                    }

                    //  orientation of box
                    if ( i == 2 ) {

                        //  Convention from Slaubaugh and  svkSatBandsXML::RotationMatrixToEulerAngles    
                        //  inverse of RotationMatrixToEulerAngles
                        //  angle1 = psi 
                        //  angle2 = theta 
                        //  angle3 = phi 

                        float psi   = svkTypeUtils::StringToFloat( values[0] ); 
                        float theta = svkTypeUtils::StringToFloat( values[1] ); 
                        float phi   = svkTypeUtils::StringToFloat( values[2] ); 
                        // I hate this mystery factor
                        phi += vtkMath::Pi(); 
   
                        //  row 0:  
                        normals[0][0] = cos(theta) * cos(phi); 
                        normals[1][0] = sin(psi) * sin(theta) * cos(phi) - cos(psi) * sin(phi); 
                        normals[2][0] = cos(psi) * sin(theta) * cos(phi) + sin(psi) * sin(phi);

                        //  row 1:  
                        normals[0][1] = cos(theta) * sin(phi); 
                        normals[1][1] = sin(psi) * sin(theta) * sin(phi) + cos(psi) * cos(phi); 
                        normals[2][1] = cos(psi) * sin(theta) * sin(phi) - sin(psi) * cos(phi); 

                        //  row 2:  
                        normals[0][2] = -1 * sin(theta); 
                        normals[1][2] = sin(psi) * cos(theta);
                        normals[2][2] = cos(psi) * cos(theta); 

                    }
                } 

                if ( this->GetDebug() ) {
                    //  Print normals from euler angles
                    cout << normals[0][0] << " " << normals[1][0] << " " << normals[2][0] << endl;    
                    cout << normals[0][1] << " " << normals[1][1] << " " << normals[2][1] << endl;    
                    cout << normals[0][2] << " " << normals[1][2] << " " << normals[2][2] << endl;    
                }

                // Put array rows in RL, AP, SI order
                this->SortNormalArrayRLAPSI(normals); 

                float satNormal[3]; 
                float lpsVector[3]; 
                float size; 
                float distance; 
                int sign;  
                string thicknessString = "0"; 
                string distanceString; 
                
                string normalString; 
                string labelString;
                for ( int i = 0; i < 6; i++ ) {

                    //  Now convert this representation (boxCenter, boxSize, boxDcos) into a 
                    //  normal, center and distance from origin for each of the 6 faces.   
                    //  for each of the 6 faces derive the location, thickness (0) and normal
                    //  0- anterior  (AP)  
                    //  1- posterior (AP)
                    //  2- superior  (SI)
                    //  3- inferior  (SI)
                    //  4- left      (LR)
                    //  5- right     (LR)
                    if ( i == 0 || i == 1 ) {
                        //  size in AP direction: 
                        size = boxSize[1]/2;  

                        for ( int j = 0; j < 3; j++ ) {
                            satNormal[j] = normals[1][j];   //AP
                        }

                        //  id = 0 is more Anterior
                        //  id = 1 is more Posterior 
                        //  in LPS coords:      
                        lpsVector[0] = 0; 
                        lpsVector[1] = 1; 
                        lpsVector[2] = 0; 
                        if ( vtkMath::Dot(satNormal, lpsVector) > 0) {
                            //satNormal points to P
                            if ( i == 0 ) {
                                sign = -1; 
                            } else {
                                sign = 1; 
                            }
                        } else {
                            if ( i == 0 ) {
                                sign = 1; 
                            } else {
                                sign = -1; 
                            }
                        }
                        if ( i == 0) { 
                            labelString = "Anterior"; 
                        } else {
                            labelString = "Posterior"; 
                        }
                    }

                    if ( i == 2 || i == 3 ) {
                        //  SI size: 
                        size = boxSize[2]/2;  

                        for ( int j = 0; j < 3; j++ ) {
                            satNormal[j] = normals[2][j];   //SI
                        }
                        //  id = 2 is more Superior
                        //  id = 3 is more Inferior  
                        //  in LPS coords:      
                        lpsVector[0] = 0; 
                        lpsVector[1] = 0; 
                        lpsVector[2] = 1; 
                        if ( vtkMath::Dot(satNormal, lpsVector) > 0) {
                            //satNormal points to S
                            if ( i == 2 ) {
                                sign = 1; 
                            } else {
                                sign = -1; 
                            }
                        } else {
                            if ( i == 2 ) {
                                sign = -1; 
                            } else {
                                sign = 1; 
                            }
                        }
                        if ( i == 2) { 
                            labelString = "Superior"; 
                        } else {
                            labelString = "Inferior"; 
                        }
                    }

                    if ( i == 4 || i == 5 ) {
                        //  LR size: 
                        size = boxSize[0]/2;  

                        for ( int j = 0; j < 3; j++ ) {
                            satNormal[j] = normals[0][j];   //LR
                        }
                        //  id = 4 is more Left 
                        //  id = 5 is more Right     
                        //  in LPS coords:      
                        lpsVector[0] = 1; 
                        lpsVector[1] = 0; 
                        lpsVector[2] = 0; 
                        if ( vtkMath::Dot(satNormal, lpsVector) > 0) {
                            //satNormal points to S
                            if ( i == 4 ) {
                                sign = 1; 
                            } else {
                                sign = -1; 
                            }
                        } else {
                            if ( i == 4 ) {
                                sign = -1; 
                            } else {
                                sign = 1; 
                            }
                        }
                        if ( i == 4) { 
                            labelString = "Left"; 
                        } else {
                            labelString = "Right"; 
                        }
                    }

                    //  Compute distance of plane from origin: 
                    //  1.  compute points on planes a distance, size, +/- from the box
                    //      origin along the normal
                    //  2.  The vector from the origin to this point (s) is projected
                    //      along the normal vector.  The magnitude of this projection is 
                    //      the desired distance
                    //  each point is defined by 3 coordinates: 
                    float point[3];  
                    for (int k = 0; k < 3; k++ ) {
                        point[k] = boxCenter[k] + sign * satNormal[k] * size; 
                    }
                    distance = vtkMath::Dot(satNormal, point); 

                    //  Use this info to initialize a sat band element in an XML file: 
                    vtkXMLDataElement* satBandElement = vtkXMLDataElement::New(); 
                    satBandElement->SetName("sat_band"); 
                    satBandElement->SetAttribute( "id", svkTypeUtils::IntToString( i+1 ).c_str() );
                    satBandElement->SetId( svkTypeUtils::IntToString( i+1 ).c_str() );
                    satBandElement->SetAttribute("label", labelString.c_str() );
                    this->pressBoxElement->AddNestedElement( satBandElement ); 

                    vtkXMLDataElement* normalXElement = vtkXMLDataElement::New();
                    normalXElement->SetName("normal_x"); 
                    normalString = svkTypeUtils::DoubleToString(satNormal[0]); 
                    normalXElement->SetCharacterData(normalString.c_str(), normalString.length() ); 
                    satBandElement->AddNestedElement( normalXElement ); 

                    vtkXMLDataElement* normalYElement = vtkXMLDataElement::New();
                    normalYElement->SetName("normal_y"); 
                    normalString = svkTypeUtils::DoubleToString(satNormal[1]); 
                    normalYElement->SetCharacterData(normalString.c_str(), normalString.length() ); 
                    satBandElement->AddNestedElement( normalYElement ); 

                    vtkXMLDataElement* normalZElement = vtkXMLDataElement::New();
                    normalZElement->SetName("normal_z"); 
                    normalString = svkTypeUtils::DoubleToString(satNormal[2]); 
                    normalZElement->SetCharacterData(normalString.c_str(), normalString.length() ); 
                    satBandElement->AddNestedElement( normalZElement ); 

                    vtkXMLDataElement* thicknessElement = vtkXMLDataElement::New();
                    thicknessElement->SetCharacterData(thicknessString.c_str(), thicknessString.length()); 
                    thicknessElement->SetName("thickness"); 
                    satBandElement->AddNestedElement( thicknessElement ); 

                    vtkXMLDataElement* distanceElement = vtkXMLDataElement::New();
                    distanceString = svkTypeUtils::DoubleToString( distance ); 
                    distanceElement->SetName("distance_from_origin"); 
                    distanceElement->SetCharacterData(distanceString.c_str(), distanceString.length()); 
                    satBandElement->AddNestedElement( distanceElement ); 

                    if ( this->GetDebug() )  {
                        cout << "PB RAW INFO: " << satNormal[0] << " " << satNormal[1] << " " << satNormal[2] << " " << thicknessString << " " << distance << endl;        
                    }
                }
            
            } else {                     
                datFile->seekg(0, ios::end);     
            }

        } catch (const exception& e) {
            if (this->GetDebug()) {
                cout <<  "ERROR reading line: " << e.what() << endl;
            }
            status = -1;  
        }

        datFile->close();

        if (this->GetDebug()) {
            cout << " PRESS BOX XML " << endl;
            this->pressBoxElement->PrintXML( cout, vtkIndent() );
            cout << " done: PRESS BOX XML " << endl;
        }

    } catch (const exception& e) {
        cerr << "ERROR opening or reading PFile .dat file: " << e.what() << endl;
    }
    return status; 
}


/*
 *  Attempt to open and parse legacy sat_bands.dat file and initialize the 
 *  intermal XML data structure from the info in the file. 
 */
int svkSatBandsXML::InitSatsFromDat( string rootName ) 
{
    
    int status = 0; 
    string datFileName = rootName; 
    datFileName.append("_sat_bands.dat"); 
    if ( ! svkFileUtils::FilePathExists( datFileName.c_str() ) ) {
        //  No legacy .dat file, so just return 0; 
        cout << "No legacy sat_bands.dat file:  " <<  datFileName << endl;
        return status; 
    } 
    
    cout << "Found legacy sat_bands.dat file: " <<  datFileName << endl;

    try { 

        ifstream* datFile = new ifstream();
        datFile->exceptions( ifstream::eofbit | ifstream::failbit | ifstream::badbit );
        datFile->open( datFileName.c_str(), ifstream::in );
        if ( ! datFile->is_open() ) {
            throw runtime_error( "Could not open PFile .dat file: " + datFileName);
        } 

        long datFileSize = svkFileUtils::GetFileSize( datFile );

        istringstream* iss = new istringstream();
        string keyString;

        try {

            //  Read the first line to get the number of sat bands in the file: 
            svkFileUtils::ReadLine( datFile, iss); 

            size_t  position; 
            string  tmp; 

            if ( datFile->tellg() < datFileSize - 1 ) {

                    
                //  find first white space position after "num sats" string: 
                int numSats = svkTypeUtils::StringToInt( iss->str()); 

                for ( int i = 0; i < numSats; i++ ) { 

                    vtkXMLDataElement* satBandElement = vtkXMLDataElement::New(); 
                    satBandElement->SetName("sat_band"); 
                    satBandElement->SetAttribute( "id", svkTypeUtils::IntToString( i+1 ).c_str() );
                    satBandElement->SetId( svkTypeUtils::IntToString( i+1 ).c_str() );
                    string labelString = "label_"; 
                    labelString.append( svkTypeUtils::IntToString(i+1)); 
                    satBandElement->SetAttribute("label", labelString.c_str() );
                    this->autoSatsElement->AddNestedElement( satBandElement ); 

                    svkFileUtils::ReadLine( datFile, iss); 

                    position = iss->str().find_first_of(' ');
                    string angle1; 
                    if (position != string::npos) {
                        angle1.assign( iss->str().substr(0, position) );
                    }    
                    
                    tmp.assign( iss->str().substr(position + 1) ); 

                    position = tmp.find_first_of(' ');
                    string angle2; 
                    if (position != string::npos) {
                        angle2.assign( tmp.substr(0, position) );
                    } 
                    tmp.assign( tmp.substr(position + 1) ); 

                    position = tmp.find_first_of(' ');
                    string distance; 
                    if (position != string::npos) {
                        distance.assign( tmp.substr(0, position) );
                    }
                    tmp.assign( tmp.substr(position + 1) ); 

                    position = tmp.find_first_of(' ');
                    string thickness; 
                    if (position != string::npos) {
                      thickness.assign( tmp.substr(0, position) );
                    }
                    
                    //cout << "a1: " << angle1 << " a2: " << angle2 << " dist: " << distance << " thck: " << thickness << endl;
                    //  Convert these into a normal, distance and thickness
                    float satNormal[3]; 
                    this->PSDAutSatAnglesToNormal( 
                                svkTypeUtils::StringToFloat(angle1),  
                                svkTypeUtils::StringToFloat(angle2), 
                                satNormal 
                            ) ; 

                    string normalString; 

                    //  Use this info to initialize a sat band element in an XML file: 
                    vtkXMLDataElement* normalXElement = vtkXMLDataElement::New();
                    normalXElement->SetName("normal_x"); 
                    normalString = svkTypeUtils::DoubleToString(satNormal[0]); 
                    normalXElement->SetCharacterData(normalString.c_str(), normalString.length() ); 
                    satBandElement->AddNestedElement( normalXElement ); 

                    vtkXMLDataElement* normalYElement = vtkXMLDataElement::New();
                    normalYElement->SetName("normal_y"); 
                    normalString = svkTypeUtils::DoubleToString(satNormal[1]); 
                    normalYElement->SetCharacterData(normalString.c_str(), normalString.length() ); 
                    normalYElement->SetCharacterData(normalString.c_str(), normalString.length() ); 
                    satBandElement->AddNestedElement( normalYElement ); 

                    vtkXMLDataElement* normalZElement = vtkXMLDataElement::New();
                    normalZElement->SetName("normal_z"); 
                    normalString = svkTypeUtils::DoubleToString(satNormal[2]); 
                    normalZElement->SetCharacterData(normalString.c_str(), normalString.length() ); 
                    normalZElement->SetCharacterData(normalString.c_str(), normalString.length() ); 
                    satBandElement->AddNestedElement( normalZElement ); 

                    vtkXMLDataElement* thicknessElement = vtkXMLDataElement::New();
                    thicknessElement->SetCharacterData(thickness.c_str(), thickness.length()); 
                    thicknessElement->SetName("thickness"); 
                    satBandElement->AddNestedElement( thicknessElement ); 

                    vtkXMLDataElement* distanceElement = vtkXMLDataElement::New();
                    distanceElement->SetName("distance_from_origin"); 
                    distanceElement->SetCharacterData(distance.c_str(), distance.length()); 
                    satBandElement->AddNestedElement( distanceElement ); 
                
                }

            } else {                     
                datFile->seekg(0, ios::end);     
            }

        } catch (const exception& e) {
            if (this->GetDebug()) {
                cout <<  "ERROR reading line: " << e.what() << endl;
            }
            status = -1;  
        }

        datFile->close();

        if (this->GetDebug()) {
            cout << " SAT BAND XML " << endl;
            this->autoSatsElement->PrintXML( cout, vtkIndent() );
            cout << " done: SAT BAND XML " << endl;
        }


    } catch (const exception& e) {
        cerr << "ERROR opening or reading PFile .dat file: " << e.what() << endl;
    }
    return status; 
}


/*! Sets the current XML data to NULL
 *  so the file will be re-read.
 */
void svkSatBandsXML::ClearXMLFile( )
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
 *  
 */
void svkSatBandsXML::SetVerbose( bool isVerbose )
{
    this->isVerbose = isVerbose; 
}


/*!
 */
void svkSatBandsXML::WriteXMLFile( string xmlFileName )
{
    vtkIndent indent;
    vtkXMLUtilities::WriteElementToFile( this->satBandsXML, xmlFileName.c_str(), &indent );
}



/*!
 *  Get the specified sat band definition: 
 */
void svkSatBandsXML::GetPressBoxSat(int satNumber, string* label, float *normalX, float *normalY, float* normalZ, float* thickness, float* distance)
{
    cout << "INSIDE GetPressBoxSat nr: " << satNumber<< endl;
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
int svkSatBandsXML::GetAutoSat(int satNumber, string* label, float normal[3], float* thickness, float* distance)
{
    return this->GetAutoSat(
                satNumber, 
                label, 
                &normal[0], 
                &normal[1], 
                &normal[2], 
                thickness, 
                distance
            ); 
}


/*!
 *  Get the specified sat band definition: 
 *      Normal 
 *      thickness of band 
 *      distance along normal from origin to center of plane 
 *  SatNumber starts at 1 (element ID)
 */
int svkSatBandsXML::GetAutoSat(int satNumber, string* label, float* normalX, float* normalY, float* normalZ, float* thickness, float* distance)
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
    //this->autoSatsElement->PrintXML( cout, vtkIndent() );
    //satBandElement->PrintXML( cout, vtkIndent() );
    //cout << "ID NUMBER: " << satBandElement->GetId() << endl;

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
int svkSatBandsXML::GetXMLVersion() 
{
    return this->versionNumber; 
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
 *  Determine the auto sat parameters as used by the prose PSD code.  This is a different
 *  representation of the same information as encoded into the XML file 
 * 
 *  Determines the 2 angles that take the Z vector (+S) to the specified normal vector? 
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

    //  Which 2 angles take the Z vector (+S) to the specified normal vector? 

    //  First determine the angle listed in .dat file for use by PSD to 
    //  rotation 100 vector about the x axis (this rotates the Z/S vector 100 
    //  toward the p (+y) vector and is given by the atan(y/z)

    //  Next determine the angle listed in legacy .dat file for use by PSD to 
    //  rotation the vector about the z axis, this is written fist in the dat file 
    //  just to be confusing.
    angles[0] = 0; 
    angles[1] = -1 * acos( normal[2]);
    //  ====================================================
    //  note: 
    //      (1) -1 * atan (x) = atan (-x)  
    //          therefore -1*atan(n1/-1*n0) = atan(n1/n0);
    //  and the following 2 representations of angles[2] are equivalent: 
    //      angles[2] =  atan2( normal[1] , normal[0] );  
    //      OR:    
    //  ====================================================
    angles[2] = -1 * atan2( normal[1] , -1 *  normal[0] );  

    //cout << "ANGLES: " << angles[2] << " " << angles[1] << endl;
    return status; 
}


/*!
 *  Performs the inverse of GetAutoSatParameters
 *  Output is the normal (LPS) derived from the 2 input angles    
 */
void svkSatBandsXML::PSDAutSatAnglesToNormal( float angle1, float angle2, float normal[3] ) 
{
    normal[0] = cos(angle1) * sin (angle2); 
    normal[1] = sin(angle1) * sin (angle2); 
    normal[2] = cos (angle2); 
}

/*!
 *  Given the xml press box definition, extract the 
 *  location, thickness and orientation  of the box for use by the PSD.  
 *  Since this is the only difference between the versions, no new classes are used.
 */
void svkSatBandsXML::GetPRESSBoxParameters( float pressOrigin[3], float pressThickness[3], float pressAngles[3] ) 
{
    cout << "VERSION OF THE XML FILE: " << this->versionNumber << endl;    
    if (this->versionNumber == 2.0)
    {
        this->GetPRESSBoxParametersVer20(pressOrigin, pressThickness, pressAngles);
    }
    else
    {
        this->GetPRESSBoxParametersVer10(pressOrigin, pressThickness, pressAngles);
    }

}


/*!
 *  Given the xml press box definition, extract the 
 *  location, thickness and orientation  of the box for use by the PSD. Uses 3 plane     
 *  representaiton  
 */
void svkSatBandsXML::GetPRESSBoxParametersVer20( float pressOrigin[3], float pressThickness[3], float pressAngles[3] ) 
{
    float normals[3][3];
    float distances[3];
    string labels[3];

    for ( int satNumber = 1; satNumber <= 3; satNumber++ ) {
    
        this->GetPressBoxSat(
                    satNumber, 
                    &labels[satNumber-1], 
                    &normals[satNumber-1][0], 
                    &normals[satNumber-1][1], 
                    &normals[satNumber-1][2], 
                    &pressThickness[satNumber-1], 
                    &distances[satNumber-1]
                ); 
    }

    //  ===========================================================
    //  Now, use the normals and distances to get the location of the  
    //  center of the box.  
    //  ===========================================================
    //  first get the corresponding lengths: 

    for ( int i = 0; i < 3; i++ ) {
        pressOrigin[i] = 0.; 
    }

    for ( int i = 0; i < 3; i++ ) {
        for ( int j = 0; j < 3; j++ ) {
            float tmp = normals[i][j] * (distances[i]); 
            pressOrigin[j] += tmp;  
        }
    }

    // Invert the second normal in Version 2.0 because of compatibility issues
     if (normals[0][0] < 0){
        normals[0][0] = - normals[0][0];    
    }

    if (normals[1][1] < 0){    
        normals[1][1] = -normals[1][1];
        normals[1][2] = -normals[1][2];
    }
    if (normals[2][2] < 0){    
        normals[2][1] = -normals[2][1];
        normals[2][2] = -normals[2][2];
    }
/*   else{
        vtkMath::Cross(normals[0], normals[2], normals[1]);
    }*/

    
    cout << "PRESS ORIGIN ver 2.0: " << pressOrigin[0] << " " << pressOrigin[1] << " " << pressOrigin[2] << endl;    
    //  ===========================================================
    //  The PRESS Thickness is directly loaded from the XML in Version 2.0
    //  ===========================================================
    cout << "PRESS Thickness ver 2.0: " << pressThickness[0] << " " 
            << pressThickness[1] << " " << pressThickness[2] << endl;    

    //  ===========================================================
    //  Finally, get the 3 Euler angles from the normals array (DCM). 
    //  http://en.wikipedia.org/wiki/Rotation_formalisms_in_three_dimensions#Rotation_matrix_.E2.86.94_Euler_angles
    //      Note here that the rotation matrix, DCM = AzAyAx, so 
    //          Ax-> Psi is rotation about L 
    //          Ay-> Theta is rotation about P 
    //          Az-> Phi is rotation about S
    //  ===========================================================
    this->RotationMatrixToEulerAngles( normals, pressAngles ); 

    //  ADD PI to the 3rd angle.. this is a bit of a black box operation, but required for the PSD
    //  to get the correct prescription
    pressAngles[2] += vtkMath::Pi();
   
    cout << "PRESS EULERS ver 2.0: " << pressAngles[0] << " " << pressAngles[1] << " " << pressAngles[2] << endl;        

}

/*!
 *  Given the xml press box definition, extract the 
 *  location, thickness and orientation  of the box for use by the PSD. Uses 6 plane
 *  representation  
 */
void svkSatBandsXML::GetPRESSBoxParametersVer10( float pressOrigin[3], float pressThickness[3], float pressAngles[3] ) 
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
        pressThickness[i] = pow((float)differenceVectorLength, (float)0.5);  
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

    this->RotationMatrixToEulerAngles( normals, pressAngles ); 

    //  ADD PI to the 3rd angle.. this is a bit of a black box operation, but required for the PSD
    //  to get the correct prescription
    pressAngles[2] += vtkMath::Pi();

    
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

    //  these equations are defined for the inverse transformation, so transpose the normals matrix when 
    //  solvinig for 3 angles: 
    this->TransposeNormals( normals );
    for ( int i = 0; i < 3; i++ ) {
        cout << "NORMALS: " << normals[i][0] << " " 
            << normals[i][1] << " " << normals[i][2] << endl;        
    }

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
 *
 */
void svkSatBandsXML::TransposeNormals( float normals[3][3] ) {
    //  Transpose rotation matrix: 
    float transposed[3][3]; 
    vtkMath::Transpose3x3( normals, transposed); 
    for ( int i=0; i < 3; i++) {
        for ( int j=0; j < 3; j++) {
            normals[i][j] = transposed[i][j]; 
        }
    }
    //  Transpose rotation matrix END 
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
cout << "satnum: " << satNumber << endl;
        if ( this->IsNormalUnique( normal, normals) && this->IsConventionalNormal( normal ) )  {
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

    this->SortNormalArrayRLAPSI(normals); 

}


/*!
 *  Sorts 3x3 normal array into conventional order RL, AP, SI rows.  
 *   *      xNormal is defined by 5th element  (points to Left)
 *   *      yNormal is defined by 2nd element  (points to Posterior)
 *   *      zNormal is defined by 3nd element  (points to Superior)
 *
 */
void svkSatBandsXML::SortNormalArrayRLAPSI( float normals[3][3] ) 
{
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
 *  By current convention (terrible!!!) in sat band xml (LPS positivie), 
 *      xNormal is defined by 5th element  (points to Left)
 *      yNormal is defined by 2nd element  (points to Posterior)
 *      zNormal is defined by 3nd element  (points to Superior)
 */
bool svkSatBandsXML::IsConventionalNormal( float normalIn[3]  ) 
{

    bool isConventionalElement = false; 

    string label;     
    float normal[3]; 
    float thick; 
    float dist; 

    float dot; 

    this->GetPressBoxSat( 2, &label, &normal[0], &normal[1], &normal[2], &thick, &dist); 
    dot = vtkMath::Dot( normalIn, normal );
    float dotThreshold = .98; 
    if (dot > dotThreshold ) { 
         isConventionalElement = true; 
    }
        
    this->GetPressBoxSat( 3, &label, &normal[0], &normal[1], &normal[2], &thick, &dist); 
    dot = vtkMath::Dot( normalIn, normal );
    if (dot > dotThreshold ) { 
         isConventionalElement = true; 
    }

    this->GetPressBoxSat( 5, &label, &normal[0], &normal[1], &normal[2], &thick, &dist); 
    dot = vtkMath::Dot( normalIn, normal );
    if (dot > dotThreshold ) { 
         isConventionalElement = true; 
    }

    return isConventionalElement; 
    
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
 *          5 => most Left  
 *          6 => most Right 
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
            float dotThreshold = .98; 
            if ( dot  >= dotThreshold || dot <= -1 * dotThreshold ) {

                int distanceIndex = 0; 
                if ( distances[normalIndex][0] != VTK_FLOAT_MAX ) {    
                    distanceIndex = 1; 
                }
                
                if ( dot  >= -1 * dotThreshold ) {
                    distances[normalIndex][distanceIndex] = distance; 
                    numDistancesInitialized++;     
                } else if ( dot  <= -1 * dotThreshold ) {
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
        if ( fabs( dot ) >= .98 ) {
            isUnique = false; 
        }
    }
    return isUnique; 
}


vtkXMLDataElement* svkSatBandsXML::GetXMLDataElement()
{
    return this->satBandsXML; 
}


/*! 
 * External C interface: 
 */
void* svkSatBandsXML_New(const char* xmlFileName, int *status)
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
    return NULL;
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

