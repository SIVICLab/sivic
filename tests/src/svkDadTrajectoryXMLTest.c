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
#include <svkDataAcquisitionDescriptionXML.h>
#include <stdio.h>
#include <stdlib.h>     

 
int main(const int argc, const char **argv)
{
    int   status = 0;

    // Create a new data acquisition description object
    void* dadXml = svkDataAcquisitionDescriptionXML_New( );

    // Set the trajectory type, id, and comment
    svkDataAcquisitionDescriptionXML_SetTrajectory("cartesian", "cartesian2342", "This is an undersampled dataset..", dadXml ); 

    //Set a long parameter
    svkDataAcquisitionDescriptionXML_SetTrajectoryLongParameter("myLongParam", 5, dadXml ); 

    //Set a double parameter
    svkDataAcquisitionDescriptionXML_SetTrajectoryDoubleParameter("myDoubleParam", 5.56, dadXml ); 

    //Get the trajectory type
    const char* trajectoryType = svkDataAcquisitionDescriptionXML_GetTrajectoryType(dadXml );
    printf("Trajectory Type   : %s\n", trajectoryType);

    // Get a trajectory comment
    const char* trajectoryID = svkDataAcquisitionDescriptionXML_GetTrajectoryID(dadXml );
    printf("Trajectory ID     : %s\n", trajectoryID);

    // Get a trajectory comment
    const char* trajectoryComment = svkDataAcquisitionDescriptionXML_GetTrajectoryComment(dadXml );
    printf("Trajectory Comment: %s\n", trajectoryComment);

    // Get a lang user parameter
    long myLongParam = svkDataAcquisitionDescriptionXML_GetTrajectoryLongParameter("myLongParam", dadXml );
    printf("Trajectory Long   : %d\n", myLongParam);

    // Get a double user parameter
    double myDoubleParam = svkDataAcquisitionDescriptionXML_GetTrajectoryDoubleParameter("myDoubleParam", dadXml );
    printf("Trajectory Double : %f\n", myDoubleParam);

    // Use the generic setting to modify a variable using its path
    const char* newID = "MyNewID";
    int result = svkDataAcquisitionDescriptionXML_SetDataWithPath(dadXml, "encoding/trajectoryDescription/identifier", newID );

    // Use the generic getter to get data at a specific path
    const char* currentID = svkDataAcquisitionDescriptionXML_GetDataWithPath(dadXml, "encoding/trajectoryDescription/identifier" );
    if(strcmp( newID, currentID ) == 0 ){
    	printf("ERROR: Identified could not be set.");
    	status = -1;
    }

    // Write the XML File out
    svkDataAcquisitionDescriptionXML_WriteXMLFile( argv[1], dadXml );

    // Free memory
    svkDataAcquisitionDescriptionXML_Delete( dadXml );
    dadXml = NULL;

    // Read the dad file back in
    status = -1;
    dadXml =  svkDataAcquisitionDescriptionXML_Read(argv[1], &status);
    svkDataAcquisitionDescriptionXML_AddElementWithParentPath( dadXml, "encoding", "dummy_one" );
    svkDataAcquisitionDescriptionXML_AddElementWithParentPath( dadXml, "encoding", "dummy_two" );
    status = svkDataAcquisitionDescriptionXML_RemoveElementWithParentPath( dadXml, "encoding", "dummy_one" );
    if( status != 0 ) {
    	printf("Could not remove element\n");
    }
    // Write the XML File out
    svkDataAcquisitionDescriptionXML_WriteXMLFile( argv[1], dadXml );

    return status;
}


