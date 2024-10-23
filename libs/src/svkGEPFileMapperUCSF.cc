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


#include <svkGEPFileMapperUCSF.h>
#include </usr/include/vtk/vtkDebugLeaks.h>

#include <svkSatBandsXML.h>

#include "svkTypeUtils.h"


using namespace svk;


//vtkCxxRevisionMacro(svkGEPFileMapperUCSF, "$Rev$");
vtkStandardNewMacro(svkGEPFileMapperUCSF);


/*!
 *
 */
svkGEPFileMapperUCSF::svkGEPFileMapperUCSF()
{

#if VTK_DEBUG_ON
    this->DebugOn();
    vtkDebugLeaks::ConstructClass("svkGEPFileMapperUCSF");
#endif

    vtkDebugMacro( << this->GetClassName() << "::" << this->GetClassName() << "()" );

}


/*!
 *
 */
svkGEPFileMapperUCSF::~svkGEPFileMapperUCSF()
{
    vtkDebugMacro( << this->GetClassName() << "::~" << this->GetClassName() << "()" );
}


/*!
 *  Determine the number of sampled k-space points in the data set. 
 *  This may differ from the number of voxels in the rectalinear grid, 
 *  for example if elliptical or another non rectangular acquisition 
 *  sampling strategy was employed.  GE product sequences pad the
 *  reduced k-space data with zeros so the number of k-space points 
 *  is the same as the number of voxels, but that may not be true for
 *  custom sequences.  
 */
int svkGEPFileMapperUCSF::GetNumKSpacePoints()
{

    int numKSpacePts;

    //  Image user21 indicates that an elliptical k-space sampling radius 
    //  was used.  If true, then number of k-space points differs from numVoxels
    //  in rectaliniear grid. 
    if (this->GetHeaderValueAsInt( "rhi.user21" ) != 0 ) { 

        float ellipticalRad =  this->GetHeaderValueAsFloat( "rhi.user22" ); 
        numKSpacePts = 0; 

        int numVoxels[3]; 
        this->GetNumVoxels( numVoxels ); 

        for( int z = 1; z <= numVoxels[2]; z++ ) {

            float zCorner = 2.0 * z - numVoxels[2] - 1.0;
            if (zCorner > 0) {
                zCorner += 1.0;
            } else {
                zCorner -= 1.0;
            }

            for( int y = 1; y <= numVoxels[1]; y++ ) {

                float yCorner = 2.0 * y - numVoxels[1] - 1.0;
                if (yCorner > 0) {
                    yCorner += 1.0;
                } else {
                    yCorner -= 1.0;
                }

                for( int x = 1; x <= numVoxels[0]; x++ ) {

                    float xCorner = 2.0 * x - numVoxels[0] - 1.0;
                    if (xCorner > 0) {
                        xCorner += 1.0;
                    } else {
                        xCorner -= 1.0;
                    }

                    float test = 
                        ( ( xCorner * xCorner ) / (float)( numVoxels[0] * numVoxels[0] ) )
                      + ( ( yCorner * yCorner ) / (float)( numVoxels[1] * numVoxels[1] ) ) 
                      + ( ( zCorner * zCorner ) / (float)( numVoxels[2] * numVoxels[2] ) );

                    // is it inside ellipse?
                    if ( test <= ellipticalRad ) {
                        numKSpacePts++;
                    }
                }     // for xstep
            }     // for ystep
        }     // for zstep

    } else {
        numKSpacePts = GetNumVoxelsInVol(); 
    }

    if ( this->GetDebug() ) {
        cout << "NUM KSPACE PTS: " << numKSpacePts << endl; 
    }

    return numKSpacePts; 
}


/*!
 *  Determines whether a voxel (index) was sampled, or not, i.e. was it within 
 *  the elliptical sampling volume if reduced k-space elliptical sampling
 *  was used.  Could be extended to support other sparse sampling 
 *  trajectories.       
 */
bool svkGEPFileMapperUCSF::WasIndexSampled(int indexX, int indexY, int indexZ)
{

    bool wasSampled = true;

    //  Image user21 indicates that an elliptical k-space sampling radius 
    //  was used.  If true, then number of k-space points differs from numVoxels
    //  in rectaliniear grid. 
    if (this->GetHeaderValueAsInt( "rhi.user21" ) != 0 ) { 
    
        float ellipticalRad =  this->GetHeaderValueAsFloat( "rhi.user22" ); 

        //  if ellipse is defined by bounding rectangle that 
        //  defines k-space grid, then

        int numVoxels[3]; 
        this->GetNumVoxels( numVoxels ); 

        /*  Get the origin of the ellipse
         *  and length of principle axes defined by 
         *  bounding box (acquisition grid)of ellipse
         *  Also get the exterior corner of the voxel in question 
         *  to see if it is 100% within the sampling ellipse 
         *  radius. 
         */
        float ellipseOrigin[3]; 
        float ellipseRadius[3]; 
        float voxelCorner[3]; 
        voxelCorner[0] = indexX; 
        voxelCorner[1] = indexY; 
        voxelCorner[2] = indexZ; 
        for (int i = 0; i < 3; i++) {

            ellipseOrigin[i] = ( static_cast<float>( numVoxels[i] ) - 1 ) / 2; 
            ellipseRadius[i] =   static_cast<float>( numVoxels[i] ) / 2; 

            if ( voxelCorner[i] < ellipseOrigin[i] ) {
                voxelCorner[i] -= .5; 
            } else {
                voxelCorner[i] += .5; 
            }

        }

        float voxelExteriorRadius = 0.;  
        for (int i = 0; i < 3; i++) {
            voxelExteriorRadius += pow( 
                    ( (voxelCorner[i] - ellipseOrigin[i]) / ellipseRadius[i] ), 
                    2 
                ); 
        }

        /*  See if the exterior corner of the voxel is within the sampled elliptical radius. 
         *   
         *  The eqn of the ellips in a bounding box defined by the MRSI acquisition grid    
         *  with center c (ellipseOrigin) and radius r (ellipseRadius) is if the size of the axes
         *  are ordered correctly such that rx > ry > rz > 0:
         *      (x-cx)^2 + (y-cy)^2 + (z-cz)^2
         *      --------   --------   --------   
         *        rx^2       ry^2       rz^2 
         */
        if ( voxelExteriorRadius <= ellipticalRad) {
            wasSampled = true;  
        } else { 
            wasSampled = false;  
        }

    } 

    return wasSampled; 
}


/*!
 *  Add additional sat bands from XML dat file: 
 *      if read_sats == 1, init the bands from *_box_satbands_atlasbased.dat
 *      if oct_sats == 1, then init the oct sats
 */
void svkGEPFileMapperUCSF::InitSatBandsFromXML()
{

    int readSats = -1; 
    int octSats = -1; 
    this->GetCVS(&readSats, &octSats); 
    if ( this->GetDebug() ) {
        cout << "AUTO SATS: " << readSats << " OCT SATS: " << octSats << endl;
    }


    if ( octSats > 0 ) {

        //  Unlik the auto sats, the oct sats don't require any 
        //  XML files, just the definition of the PRESS box  
        //      procedure:
        //      1.  get selection box center, size, and  orientation (x, y, z normals)  
        //      2.  use these to generate 5 bands with cosine modulated with (cmw) of 40mm
        //      3.  for each of the bands in step 2 with cmw != 0, also generate a second band displaced
        //          in opposite direction 
        float selBoxSize[3];
        float selBoxCenter[3]; 
        float selBoxOrientation[3][3];
        if( this->dcmHeader->ElementExists( "VolumeLocalizationSequence" ) ) {

            for (int i = 0; i < 3; i++) {

                selBoxSize[i] = this->dcmHeader->GetFloatSequenceItemElement(
                    "VolumeLocalizationSequence",
                    i,
                    "SlabThickness"
                );

                selBoxCenter[i] = this->dcmHeader->GetFloatSequenceItemElement(
                    "VolumeLocalizationSequence",
                    0,
                    "MidSlabPosition",
                    NULL,
                    0,
                    i
                );
    
            }
            for (int i = 0; i < 3; i++) {
                for (int j = 0; j < 3; j++) {
                    selBoxOrientation[i][j] = this->dcmHeader->GetFloatSequenceItemElement(
                        "VolumeLocalizationSequence",
                        i,
                        "SlabOrientation",
                        NULL,
                        0,   
                        j
                    );
                }
            }
        }

        //  Generate 2 additional oct vectors for diagonals: 
        float octDiagVec1[3]; 
        float octDiagVec2[3]; 
        for (int i = 0; i < 3; i++) {
            octDiagVec1[i] = selBoxOrientation[1][i] * selBoxSize[0] + selBoxOrientation[0][i] * selBoxSize[1];
            octDiagVec2[i] = selBoxOrientation[1][i] * selBoxSize[0] - selBoxOrientation[0][i] * selBoxSize[1];
        }

        float thickness = 40.; 
        //  axial bands
        //  front-back
        //  left-right
        this->InitOctBand( selBoxOrientation[2], selBoxCenter, thickness, selBoxSize[2] + thickness ); 
        this->InitOctBand( selBoxOrientation[1], selBoxCenter, thickness, selBoxSize[1] + thickness ); 
        this->InitOctBand( selBoxOrientation[0], selBoxCenter, thickness, selBoxSize[0] + thickness ); 

        //  diagonals
        //  for direction order of dimensions is swapped because we are talking about a normal
        this->InitOctBand( octDiagVec1, selBoxCenter, thickness, ((selBoxSize[0] + selBoxSize[1])/2) + thickness ); 
        this->InitOctBand( octDiagVec2, selBoxCenter, thickness, ((selBoxSize[0] + selBoxSize[1])/2) + thickness ); 
         
    }

    if ( readSats > 0 ) {
        // Init auto-sats from XML file: 

        string xmlFileName = this->pfileName; 
        xmlFileName.append("_box_satbands_atlasbased.dat"); 

        svkSatBandsXML* xml = svkSatBandsXML::New();
        int status = xml->SetXMLFileName( xmlFileName.c_str() );
        //  If no xml file, then check for legacy .dat format 
        if ( status == 1 ) {
            //  double check for legacy version of .dat file: 
            status = xml->ConvertDatToXML(this->pfileName); 
        }
        if ( status != 0 ) {
            return; 
        }

        int numAutoSats = xml->GetNumberOfAutoSats();      
        string label; 
        float normalLPS[3]; 
        float normalRAS[3]; 
        float thickness; 
        float distance; 

        for (int satNumber = 1; satNumber <= numAutoSats; satNumber++ ) {

            xml->GetAutoSat(satNumber, &label, normalLPS, &thickness, &distance); 

            //  ===================================================
            //  Now for a few modifications to get it to conform 
            //  to definition of sats in PFile and 
            //  expected by InitSatBand: 
            //      1.  normal with length equal to sat band thickness
            //      2.  normal in RAS rather than LPS
            //      3.  Distance is to far edge of sat, so distance + 1/2 thickness
            //  ===================================================
            vtkMath::Normalize( normalLPS ); 
            for ( int i = 0; i < 3; i++ ) {
                normalLPS[i] *= thickness; 
            }
            normalRAS[0] = -1. * normalLPS[0]; 
            normalRAS[1] = -1. * normalLPS[1]; 
            normalRAS[2] = normalLPS[2]; 
            distance += (thickness/2.);
            this->InitSatBand(normalRAS, distance);
        }
        xml->Delete();
    }


    return; 
}


/*!
 *  Insert the oct bands into the header  
 *      if cmw != 0, then insert a second band as well. 
 */
void svkGEPFileMapperUCSF::InitOctBand( float normalLPS[3], float selBoxCenter[3], float thickness, float width) 
{
    //cout << "OCT BAND: " <<  thickness << " " << width << endl;

    //  normalize, then scale to proper thickness: 
    vtkMath::Normalize( normalLPS ); 

    //  compute distance from origin to center of the selBox along the specified normalLPS vector: : 
    float distance = vtkMath::Dot(normalLPS, selBoxCenter); 
    //cout << "DISTANCE: " << distance << " " << thickness << " " << width << endl;

    //  ===============
    float point[3];
    for (int k = 0; k < 3; k++ ) {
        point[k] = normalLPS[k] * ( distance + (width/2.) + (thickness/2.) );
    }
    float projection = vtkMath::Dot(normalLPS, point); 
    //cout << "PROJECTION: " << projection << endl;
    //  ===============

    float normalRAS[3]; 
    normalRAS[0] = -1. * normalLPS[0]; 
    normalRAS[1] = -1. * normalLPS[1]; 
    normalRAS[2] = normalLPS[2]; 
    for ( int i = 0; i < 3; i++ ) {
        normalRAS[i] *= thickness;
    }
    this->InitSatBand(normalRAS, projection);

    if( width != 0 ) {
        for (int k = 0; k < 3; k++ ) {
            point[k] = normalLPS[k] * (distance - (width/2.) + (thickness/2.) );
        }
        projection = vtkMath::Dot(normalLPS, point); 
        this->InitSatBand(normalRAS, projection);
    }

}


/*!
 *  Parse PFile .dat file to get value of read_sats and oct_sats. 
 */
void svkGEPFileMapperUCSF::GetCVS(int* readSats, int* octSats)
{

    string datFileName = this->pfileName; 
    datFileName.append(".dat"); 

    // If there is no dat file just return
    if( !svkUtils::FilePathExists(datFileName.c_str() ) ) {
        cout << "No associated dat files found." << endl;
        return;
    }
    try { 

        ifstream* datFile = new ifstream();
        datFile->exceptions( ifstream::eofbit | ifstream::failbit | ifstream::badbit );
        datFile->open( datFileName.c_str(), ifstream::in );
        if ( ! datFile->is_open() ) {
            throw runtime_error( "Could not open PFile .dat file: " + datFileName);
        } 

        long datFileSize = svkImageReader2::GetFileSize( datFile );
        while (! datFile->eof() ) {

            int status = 0; 

            istringstream* iss = new istringstream();
            string keyString;

            try {

                svkUtils::ReadLine( datFile, iss); 

                size_t  position; 
                string  tmp; 

                if ( datFile->tellg() < datFileSize - 1 ) {

                    //  find first white space position after "key" string: 
                    position = iss->str().find_first_of(' ');
                    if (position != string::npos) {
                        keyString.assign( iss->str().substr(0, position) );
                    } 
                    if ( (keyString.compare("oct_sats") == 0) || (keyString.compare("read_sats") == 0) ) {

                        //  Find '='
                        string valueString = iss->str().substr(position+1); 
                        position = valueString.find_first_of('=');
                        if (position != string::npos) {
                            valueString.assign( valueString.substr(position + 1) );
                        } 

                        //  Find value 
                        position = valueString.find_first_not_of(' ');
                        if (position != string::npos) {
                            valueString.assign( valueString.substr(position) );
                        } 
                        if ( keyString.compare("oct_sats") == 0 ) {
                            *octSats = svkTypeUtils::StringToInt(valueString); 
                        } else if ( keyString.compare("read_sats") == 0) {
                            *readSats = svkTypeUtils::StringToInt(valueString); 
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

            if ( status != 0 ) {
                break;
            }
        }

        datFile->close();

    } catch (const exception& e) {
        cerr << "ERROR opening or reading PFile .dat file: " << e.what() << endl;
    }

}


