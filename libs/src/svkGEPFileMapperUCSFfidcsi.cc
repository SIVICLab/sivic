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


#include <svkGEPFileMapperUCSFfidcsi.h>
#include </usr/include/vtk/vtkDebugLeaks.h>
#include <svkDataAcquisitionDescriptionXML.h>
#include <svkTypeUtils.h>


using namespace svk;


//vtkCxxRevisionMacro(svkGEPFileMapperUCSFfidcsi, "$Rev$");
vtkStandardNewMacro(svkGEPFileMapperUCSFfidcsi);


/*!
 *
 */
svkGEPFileMapperUCSFfidcsi::svkGEPFileMapperUCSFfidcsi()
{

#if VTK_DEBUG_ON
    this->DebugOn();
    vtkDebugLeaks::ConstructClass("svkGEPFileMapperUCSFfidcsi");
#endif

    vtkDebugMacro( << this->GetClassName() << "::" << this->GetClassName() << "()" );

}


/*!
 *
 */
svkGEPFileMapperUCSFfidcsi::~svkGEPFileMapperUCSFfidcsi()
{
    vtkDebugMacro( << this->GetClassName() << "::~" << this->GetClassName() << "()" );
}


/*
 *
 */
void svkGEPFileMapperUCSFfidcsi::GetSelBoxCenter( float selBoxCenter[3] )
{

    selBoxCenter[0] = -1 * this->GetHeaderValueAsFloat( "rhi.ctr_R" );
    selBoxCenter[1] = -1 * this->GetHeaderValueAsFloat( "rhi.ctr_A" );
    selBoxCenter[2] = this->GetHeaderValueAsFloat( "rhi.ctr_S" );

} 


/*
 *  Gets the center of the acquisition grid.  May vary between sequences.
 */
void svkGEPFileMapperUCSFfidcsi::GetCenterFromRawFile( double* center )
{

    center[0] = -1 * this->GetHeaderValueAsFloat( "rhi.ctr_R" );
    center[1] = -1 * this->GetHeaderValueAsFloat( "rhi.ctr_A" );
    center[2] = this->GetHeaderValueAsFloat( "rhi.ctr_S" );

} 


/*
 *  Returns the volume localization type = NONE. 
 */
string  svkGEPFileMapperUCSFfidcsi::GetVolumeLocalizationTechnique()
{
    string localizationType("NONE");
    return localizationType;
}


/*!
 *  Get the voxel spacing in 3D. Note that the slice spacing may 
 *  include a skip. 
 *  Swaps the FOV if necessary based on freq_dir setting. 
 */
void svkGEPFileMapperUCSFfidcsi::GetFOV( float fov[3] )
{
    //  read from DAD file
    cout << "Question: " << endl;
    cout << "Is SWAP still relevant, or would that have been reflected already in the DAD content? " << endl;
    cout << "see parent class GetFOV logic" << endl;


    int status = this->InitAcqDad(); 

    if ( status == 1 ) {
        this->Superclass::GetFOV( fov ); 
        return; 
    } else {
        fov[0] = svkTypeUtils::StringToFloat( this->acqDad->GetDataWithPath(
                "encoding/encodedSpace/fieldOfView_mm/x")); 
        fov[1] = svkTypeUtils::StringToFloat( this->acqDad->GetDataWithPath(
                "encoding/encodedSpace/fieldOfView_mm/y")); 
        fov[2] = svkTypeUtils::StringToFloat( this->acqDad->GetDataWithPath(
                "encoding/encodedSpace/fieldOfView_mm/z")); 
        if ( this->GetDebug() ) {
            cout << "FOV( " << fov[0] << ", " << fov[1] << ", " << fov[2] << ")" <<  endl;
        }
    }

}


void svkGEPFileMapperUCSFfidcsi::GetVoxelSpacing( double voxelSpacing[3] )
{
    float fov[3]; 
    this->GetFOV( fov );

    int status = this->InitAcqDad(); 

    if ( status == 1 ) {

        this->Superclass::GetVoxelSpacing( voxelSpacing ); 
        return; 

    } else {

        float fov[3]; 
        this->GetFOV( fov ); 

        float numVoxels[3]; 
        numVoxels[0] = svkTypeUtils::StringToInt( this->acqDad->GetDataWithPath(
                "encoding/encodedSpace/matrixSize/x")); 
        numVoxels[1] = svkTypeUtils::StringToInt( this->acqDad->GetDataWithPath(
                "encoding/encodedSpace/matrixSize/y")); 
        numVoxels[2] = svkTypeUtils::StringToInt( this->acqDad->GetDataWithPath(
                "encoding/encodedSpace/matrixSize/z")); 

        for ( int i = 0; i < 3; i++ ) {
            voxelSpacing[i] = fov[i]/numVoxels[i]; 
        }
    }

    if ( this->GetDebug() ) {
        cout << "VOX SIZE( " << voxelSpacing[0] << ", " << voxelSpacing[1] << ", " << voxelSpacing[2] << ")" <<  endl;
    }
}


/*!
 *  Initialize the acqDad object
 *  return 
 *      0 OK, DAD was initialized
 *      1 NO DAD available
 */
int svkGEPFileMapperUCSFfidcsi::InitAcqDad( )
{
    int status = 0; 
    if ( this->acqDad == NULL ) {
        this->acqDad = svkDataAcquisitionDescriptionXML::New();
        string acqDadFileName = this->GetAcqDADFileName(); 
        status = this->acqDad->SetXMLFileName( acqDadFileName ); 
        if (status != 0)  {
            cout << "WARNING: Could not find DAD file: " << acqDadFileName << endl;
            cout << "using legacy GetFOV method. " << endl;
            this->acqDad->Delete();
            this->acqDad = NULL; 
        }
    }
    return status; 
}


//
string svkGEPFileMapperUCSFfidcsi::GetAcqDADFileName( )
{
    cout << "Issue: " << endl;
    cout << "xml file name is difficult to systematically determine" << endl;
    cout << "should be something like: run_num_acq_dad.xml" << endl;
    string dadFileName  = this->pfileName;
    string sequenceName = this->GetHeaderValueAsString( "rhi.psdname" );
    //dadFileName.append("_dad_" + sequenceName + ".xml");
    dadFileName.append("_dad_fidcsi_ucsf.xml");
    return dadFileName; 
}
