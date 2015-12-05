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
    const char* trajectoryType = "cartesian";
    const char* trajectoryID = "cartesian2342";
    const char* trajectoryComment = "This is an undersampled dataset..";
    svkDataAcquisitionDescriptionXML_SetTrajectory(trajectoryType, trajectoryID, trajectoryComment, dadXml );

    //Set a long parameter
    long myLongParam = 5;
    svkDataAcquisitionDescriptionXML_SetTrajectoryLongParameter("myLongParam", myLongParam, dadXml );

    //Create a double parameter
    svkDataAcquisitionDescriptionXML_SetTrajectoryDoubleParameter("myDoubleParam", 0, dadXml );

    //Set a double parameter
    double myDoubleParam = 5.56;
    svkDataAcquisitionDescriptionXML_SetTrajectoryDoubleParameter("myDoubleParam", myDoubleParam, dadXml );

    //Get the trajectory type
    const char* trajectoryTypeTest = svkDataAcquisitionDescriptionXML_GetTrajectoryType( dadXml );
    if(strcmp( trajectoryType, trajectoryTypeTest ) != 0 ){
    	printf("ERROR: Trajectory type could not be set/get.\n");
    	status = -1;
    	return status;
    }
    printf("Trajectory Type   : %s\n", trajectoryTypeTest);

    // Get a trajectory comment
    const char* trajectoryIDTest = svkDataAcquisitionDescriptionXML_GetTrajectoryID( dadXml );
    if(strcmp( trajectoryID, trajectoryIDTest ) != 0 ){
    	printf("ERROR: Trajectory ID could not be set/get.\n");
    	status = -1;
    	return status;
    }
    printf("Trajectory ID     : %s\n", trajectoryIDTest);

    // Get a trajectory comment
    const char* trajectoryCommentTest = svkDataAcquisitionDescriptionXML_GetTrajectoryComment( dadXml );
    if(strcmp( trajectoryComment, trajectoryCommentTest ) != 0 ){
    	printf("ERROR: Trajectory comment could not be set/get.\n");
    	status = -1;
    	return status;
    }
    printf("Trajectory Comment: %s\n", trajectoryCommentTest);

    // Get a lang user parameter
    long myLongParamTest = svkDataAcquisitionDescriptionXML_GetTrajectoryLongParameter("myLongParam", dadXml );
    if( myLongParam != myLongParamTest ){
    	printf("ERROR: Trajectory long parameter could not be set/get.\n");
    	status = -1;
    	return status;
    }
    printf("Trajectory Long   : %d\n", myLongParamTest);

    // Get a double user parameter
    double myDoubleParamTest = svkDataAcquisitionDescriptionXML_GetTrajectoryDoubleParameter("myDoubleParam", dadXml );
    if( myDoubleParam != myDoubleParamTest ){
    	printf("ERROR: Trajectory double parameter could not be set/get.\n");
    	status = -1;
    	return status;
    }
    printf("Trajectory Double : %f\n", myDoubleParamTest);

    // Use the generic setting to modify a variable using its path
    const char* newID = "MyNewID";
    int result = svkDataAcquisitionDescriptionXML_SetDataWithPath(dadXml, "encoding/trajectoryDescription/identifier", newID );

    // Use the generic getter to get data at a specific path
    const char* currentID = svkDataAcquisitionDescriptionXML_GetDataWithPath(dadXml, "encoding/trajectoryDescription/identifier" );
    if(strcmp( newID, currentID ) != 0 ){
    	printf("ERROR: Identifier could not be set.");
    	status = -1;
    	return status;
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
    	return status;
    }
    // Write the XML File out
    svkDataAcquisitionDescriptionXML_WriteXMLFile( argv[1], dadXml );

    return status;
}


