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
#include <string.h>

 
int main(const int argc, const char **argv) {

    if( base_test(argc, argv) != 0 || epsi_test(argc, argv) != 0) {
        return -1;
    } else {
        return 0;
    }
}



int base_test(const int argc, const char **argv) {
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
    struct svkCString trajectoryTypeTest = svkDataAcquisitionDescriptionXML_GetTrajectoryType( dadXml );
    if(strcmp( trajectoryType, trajectoryTypeTest.c_str ) != 0 ){
        printf("ERROR: Trajectory type could not be set/get.\n");
        status = -1;
        return status;
    }
    printf("Trajectory Type   : %s\n", trajectoryTypeTest);

    // Get a trajectory comment
    struct svkCString trajectoryIDTest = svkDataAcquisitionDescriptionXML_GetTrajectoryID( dadXml );
    if(strcmp( trajectoryID, trajectoryIDTest.c_str ) != 0 ){
        printf("ERROR: Trajectory ID could not be set/get.\n");
        status = -1;
        return status;
    }
    printf("Trajectory ID     : %s\n", trajectoryIDTest);

    // Get a trajectory comment
    struct svkCString trajectoryCommentTest = svkDataAcquisitionDescriptionXML_GetTrajectoryComment( dadXml );
    if(strcmp( trajectoryComment, trajectoryCommentTest.c_str ) != 0 ){
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
    struct svkCString currentID = svkDataAcquisitionDescriptionXML_GetDataWithPath(dadXml, "encoding/trajectoryDescription/identifier" );
    if(strcmp( newID, currentID.c_str ) != 0 ){
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

int epsi_test(const int argc, const char **argv) {
    int status = 0;
    printf("This is my new test start\n");
    // Create a new data acquisition description object
    void* dadXml = svkDataAcquisitionDescriptionXML_New( );
    // Set the trajectory type, id, and comment
    const char* trajectoryType = "epsi";
    const char* trajectoryID = "epsi2342";
    const char* trajectoryComment = "This is an epsi dataset..";
    svkDataAcquisitionDescriptionXML_SetTrajectory(trajectoryType, trajectoryID, trajectoryComment, dadXml );

    svkDataAcquisitionDescriptionXML_AddTrajectoryDimension("dim1", "kx","", dadXml);
    svkDataAcquisitionDescriptionXML_AddTrajectoryDimension("dim2", "ky","", dadXml);
    svkDataAcquisitionDescriptionXML_AddTrajectoryDimension("dim3", "kz","", dadXml);
    svkDataAcquisitionDescriptionXML_AddTrajectoryDimension("dim4", "kf","", dadXml);
    svkDataAcquisitionDescriptionXML_AddTrajectoryDimension("dim5", "time_dynamic", "dynamic time points", dadXml);
    svkDataAcquisitionDescriptionXML_AddEncodedMatrixSizeDimension("x", 8, dadXml);
    svkDataAcquisitionDescriptionXML_AddEncodedMatrixSizeDimension("y", 7, dadXml);
    svkDataAcquisitionDescriptionXML_AddEncodedMatrixSizeDimension("z", 6, dadXml);
    svkDataAcquisitionDescriptionXML_AddEncodedMatrixSizeDimension("time_spec", 59, dadXml);
    svkDataAcquisitionDescriptionXML_AddEncodedMatrixSizeDimension("time_dynamic", 10, dadXml);

    svkDataAcquisitionDescriptionXML_SetEPSITypeToSymmetric( dadXml );
    if( svkDataAcquisitionDescriptionXML_GetEPSIType(dadXml) != SYMMETRIC) {
        printf("ERROR: Could not set EPSI Type to flyback. <%d>\n");
        status = -1;
    }

    svkDataAcquisitionDescriptionXML_SetEPSITypeToFlyback( dadXml );
    if( svkDataAcquisitionDescriptionXML_GetEPSIType(dadXml) != FLYBACK) {
        printf("ERROR: Could not set EPSI Type to symmetric. <%d>\n");
        status = -1;
    }

    enum EPSIType expectedEPSIType = INTERLEAVED;
    svkDataAcquisitionDescriptionXML_SetEPSIType( expectedEPSIType, dadXml );
    int expectedNumberOfInterleaves = 11;
    svkDataAcquisitionDescriptionXML_SetEPSINumberOfInterleaves( expectedNumberOfInterleaves, dadXml );
    
    float  expectedGradientAmplitudeEven = 10;
    svkDataAcquisitionDescriptionXML_SetEPSIGradientAmplitude(expectedGradientAmplitudeEven, EVEN, dadXml);
    float expectedGradientAmplitudeOdd = 11;
    svkDataAcquisitionDescriptionXML_SetEPSIGradientAmplitude(expectedGradientAmplitudeOdd, ODD, dadXml);

    float  expectedRampDurationEven = .180;
    svkDataAcquisitionDescriptionXML_SetEPSIRampDuration(expectedRampDurationEven, EVEN, dadXml);
    float expectedRampDurationOdd = .190;
    svkDataAcquisitionDescriptionXML_SetEPSIRampDuration(expectedRampDurationOdd, ODD, dadXml);

    float  expectedPlateauDurationEven = .560;
    svkDataAcquisitionDescriptionXML_SetEPSIPlateauDuration(expectedPlateauDurationEven, EVEN, dadXml);
    float expectedPlateauDurationOdd = .570;
    svkDataAcquisitionDescriptionXML_SetEPSIPlateauDuration(expectedPlateauDurationOdd, ODD, dadXml);
    
    int  expectedNumberOfLobesEven = 53;
    svkDataAcquisitionDescriptionXML_SetEPSINumberOfLobes(expectedNumberOfLobesEven, EVEN, dadXml);
    int expectedNumberOfLobesOdd = 54;
    svkDataAcquisitionDescriptionXML_SetEPSINumberOfLobes(expectedNumberOfLobesOdd, ODD, dadXml);
    
    float expectedSampleSpacing = 0.04;
    svkDataAcquisitionDescriptionXML_SetEPSISampleSpacing(expectedSampleSpacing, dadXml);

    float expectedAcquisitionDelay = 0.004;
    svkDataAcquisitionDescriptionXML_SetEPSIAcquisitionDelay(expectedAcquisitionDelay, dadXml);

    float expectedEchoDelay = 0.01;
    svkDataAcquisitionDescriptionXML_SetEPSIEchoDelay(expectedEchoDelay, dadXml);

    struct svkCString expectedGradientAxisId;
    strcpy(expectedGradientAxisId.c_str, "dim3");
    int expectedGradientAxisIndex = 3;
    svkDataAcquisitionDescriptionXML_SetEPSIGradientAxis( expectedGradientAxisIndex, dadXml );
    
    svkDataAcquisitionDescriptionXML_SetTrajectoryDoubleParameter("myDoubleParam", 0, dadXml );

    svkDataAcquisitionDescriptionXML_WriteXMLFile( argv[2], dadXml );
    svkDataAcquisitionDescriptionXML_Delete( dadXml );
    dadXml = NULL;
    // Read the dad file back in
    int readStatus;
    dadXml =  svkDataAcquisitionDescriptionXML_Read(argv[2], &readStatus);
    status = readStatus != 0 ? -1: status;
    int actualNumDims = svkDataAcquisitionDescriptionXML_GetTrajectoryNumberOfDimensions(dadXml);
    int expectedNumDims = 5;
    if( actualNumDims != expectedNumDims ) {
        printf("Incorrect number of dimension. Actual value <%d> is not equal to expected value <%d>\n",
               actualNumDims, expectedNumDims);
        status = -1;
    }

    int actualMatrixSizeLength = svkDataAcquisitionDescriptionXML_GetEncodedMatrixSizeNumberOfDimensions(dadXml);
    int expectedMatrixSizeLength = 5;
    if( actualMatrixSizeLength != expectedMatrixSizeLength ) {
        printf("Incorrect matrix size length. Actual value <%d> is not equal to expected value <%d>\n",
               actualMatrixSizeLength, expectedMatrixSizeLength);
        status = -1;
    }

    status = checkMatrixSize(dadXml, 0, "x", 8) != 0 ? -1: status;
    status = checkMatrixSize(dadXml, 1, "y", 7) != 0 ? -1: status;
    status = checkMatrixSize(dadXml, 2, "z", 6) != 0 ? -1: status;
    status = checkMatrixSize(dadXml, 3, "time_spec", 59) != 0 ? -1: status;
    status = checkMatrixSize(dadXml, 4, "time_dynamic", 10) != 0 ? -1: status;
    status = checkDimensionDefinition(dadXml, 0, "dim1", "kx", "") != 0 ? -1: status;
    status = checkDimensionDefinition(dadXml, 1, "dim2", "ky", "") != 0 ? -1: status;
    status = checkDimensionDefinition(dadXml, 2, "dim3", "kz", "") != 0 ? -1: status;
    status = checkDimensionDefinition(dadXml, 3, "dim4", "kf", "") != 0 ? -1: status;
    status = checkDimensionDefinition(dadXml, 4, "dim5", "time_dynamic", "dynamic time points") != 0 ? -1: status;
    enum EPSIType actualEPSIType = svkDataAcquisitionDescriptionXML_GetEPSIType(dadXml);
    if(   actualEPSIType != expectedEPSIType) {
        printf("Incorrect EPSI Type. Actual value <%d> is not equal to expected value <%d> \n",
               actualEPSIType, expectedEPSIType);
        status = -1;
    }
    int actualNumberOfInterleaves = svkDataAcquisitionDescriptionXML_GetEPSINumberOfInterleaves(dadXml);
    if( actualNumberOfInterleaves != expectedNumberOfInterleaves ) {
        printf("Incorrect number of interleaves. Actual value <%d> is not equal to expected value <%d> \n",
               actualNumberOfInterleaves, expectedNumberOfInterleaves);
        status = -1;
    }
    
    float actualGradientAmplitudeEven = svkDataAcquisitionDescriptionXML_GetEPSIGradientAmplitude(EVEN, dadXml);
    if(  actualGradientAmplitudeEven != expectedGradientAmplitudeEven ) {
        printf("Incorrect gradient amplitude even. Actual value <%f> is not equal to expected value <%f> \n",
        actualGradientAmplitudeEven, expectedGradientAmplitudeEven);
        status = -1;
    }
    float actualGradientAmplitudeOdd = svkDataAcquisitionDescriptionXML_GetEPSIGradientAmplitude(ODD, dadXml);
    if(  actualGradientAmplitudeOdd != expectedGradientAmplitudeOdd ) {
        printf("Incorrect gradient amplitude odd. Actual value <%f> is not equal to expected value <%f> \n",
               actualGradientAmplitudeOdd, expectedGradientAmplitudeOdd);
        status = -1;
    }
    
    float actualRampDurationEven = svkDataAcquisitionDescriptionXML_GetEPSIRampDuration(EVEN, dadXml);
    if(  actualRampDurationEven != expectedRampDurationEven ) {
        printf("Incorrect ramp duration even. Actual value <%f> is not equal to expected value <%f> \n",
               actualRampDurationEven, expectedRampDurationEven);
        status = -1;
    }
    float actualRampDurationOdd = svkDataAcquisitionDescriptionXML_GetEPSIRampDuration(ODD, dadXml);
    if(  actualRampDurationOdd != expectedRampDurationOdd ) {
        printf("Incorrect ramp duration odd. Actual value <%f> is not equal to expected value <%f> \n",
               actualRampDurationOdd, expectedRampDurationOdd);
        status = -1;
    }

    float actualPlateauDurationEven = svkDataAcquisitionDescriptionXML_GetEPSIPlateauDuration(EVEN, dadXml);
    if(  actualPlateauDurationEven != expectedPlateauDurationEven ) {
        printf("Incorrect plateau duration even. Actual value <%f> is not equal to expected value <%f> \n",
               actualPlateauDurationEven, expectedPlateauDurationEven);
        status = -1;
    }
    float actualPlateauDurationOdd = svkDataAcquisitionDescriptionXML_GetEPSIPlateauDuration(ODD, dadXml);
    if(  actualPlateauDurationOdd != expectedPlateauDurationOdd ) {
        printf("Incorrect plateau duration odd. Actual value <%f> is not equal to expected value <%f> \n",
               actualPlateauDurationOdd, expectedPlateauDurationOdd);
        status = -1;
    }

    int actualNumberOfLobesEven = svkDataAcquisitionDescriptionXML_GetEPSINumberOfLobes(EVEN, dadXml);
    if(  actualNumberOfLobesEven != expectedNumberOfLobesEven ) {
        printf("Incorrect number of lobes even. Actual value <%f> is not equal to expected value <%f> \n",
               actualNumberOfLobesEven, expectedNumberOfLobesEven);
        status = -1;
    }
    int actualNumberOfLobesOdd = svkDataAcquisitionDescriptionXML_GetEPSINumberOfLobes(ODD, dadXml);
    if(  actualNumberOfLobesOdd != expectedNumberOfLobesOdd ) {
        printf("Incorrect number of lobes odd. Actual value <%f> is not equal to expected value <%f> \n",
               actualNumberOfLobesOdd, expectedNumberOfLobesOdd);
        status = -1;
    }
    
    float actualSampleSpacing = svkDataAcquisitionDescriptionXML_GetEPSISampleSpacing(dadXml);
    if(  actualSampleSpacing != expectedSampleSpacing ) {
        printf("Incorrect sample spacing. Actual value <%f> is not equal to expected value <%f> \n",
               actualSampleSpacing, expectedSampleSpacing);
        status = -1;
    }

    float actualAcquisitionDelay = svkDataAcquisitionDescriptionXML_GetEPSIAcquisitionDelay(dadXml);
    if(  actualAcquisitionDelay != expectedAcquisitionDelay ) {
        printf("Incorrect acquisition delay. Actual value <%f> is not equal to expected value <%f> \n",
               actualAcquisitionDelay, expectedAcquisitionDelay);
        status = -1;
    }
    
    float actualEchoDelay = svkDataAcquisitionDescriptionXML_GetEPSIEchoDelay(dadXml);
    if(  actualEchoDelay != expectedEchoDelay ) {
        printf("Incorrect echo delay. Actual value <%f> is not equal to expected value <%f> \n",
               actualEchoDelay, expectedEchoDelay);
        status = -1;
    }

    struct svkCString actualGradientAxisId = svkDataAcquisitionDescriptionXML_GetEPSIGradientAxisId(dadXml);
    if(  strcmp(actualGradientAxisId.c_str, expectedGradientAxisId.c_str) != 0) {
        printf("Incorrect gradient axis id. Actual value <%s> is not equal to expected value <%s> \n",
               actualGradientAxisId.c_str, expectedGradientAxisId.c_str);
        status = -1;
    }

    int actualGradientAxisIndex = svkDataAcquisitionDescriptionXML_GetEPSIGradientAxisIndex(dadXml);
    if( actualGradientAxisIndex != expectedGradientAxisIndex ) {
        printf("Incorrect gradient axis index. Actual value <%d> is not equal to expected value <%d> \n",
               actualGradientAxisIndex, expectedGradientAxisIndex);
        status = -1;
    }
    return status;
}


int checkMatrixSize(const void *dadXml, int dimIndex, const char *expectedDimName, int expectedValue) {
    int status = 0;
    if(dadXml == NULL ) {
        printf("XML is NULL!\n");
        return -1;
    }
    struct svkCString actualDimName = svkDataAcquisitionDescriptionXML_GetEncodedMatrixSizeDimensionName(dimIndex, dadXml);
    if(strcmp(actualDimName.c_str, expectedDimName) != 0 ) {
        printf("Incorrect dimension name for index %d. Actual value <%s> is not equal to expected value <%s> \n", dimIndex,
               actualDimName.c_str, expectedDimName);
        status = -1;
    }
    int actualValue = svkDataAcquisitionDescriptionXML_GetEncodedMatrixSizeDimensionValue(dimIndex, dadXml);
    if(actualValue != expectedValue) {
        printf("Incorrect dimension matrix size for index %d. Actual value %d is not equal to expected value %d \n", dimIndex,
               actualValue, expectedValue );
        status = -1;
    }
    return status;
}


int checkDimensionDefinition(const void *dadXml, int dimIndex, const char *expectedDimId,
                             const char* expectedDimLogical, const char* expectedDimDescription) {
    int status = 0;
    if(dadXml == NULL ) {
        printf("XML is NULL!\n");
        return -1;
    }
    struct svkCString actualDimId = svkDataAcquisitionDescriptionXML_GetTrajectoryDimensionId(dimIndex, dadXml);
    if(strcmp(actualDimId.c_str, expectedDimId) != 0 ) {
        printf("Incorrect dimension ID for index %d. Actual value <%s> is not equal to expected value <%s> \n", dimIndex,
               actualDimId.c_str, expectedDimId);
        status = -1;
    }

    struct svkCString actualDimLogical = svkDataAcquisitionDescriptionXML_GetTrajectoryDimensionLogical(dimIndex, dadXml);
    if(strcmp(actualDimLogical.c_str, expectedDimLogical) != 0 ) {
        printf("Incorrect dimension Logical for index %d. Actual value <%s> is not equal to expected value <%s> \n", dimIndex,
               actualDimLogical.c_str, expectedDimLogical);
        status = -1;
    }

    struct svkCString actualDimDescription = svkDataAcquisitionDescriptionXML_GetTrajectoryDimensionDescription(dimIndex, dadXml);
    if(strcmp(actualDimDescription.c_str, expectedDimDescription) != 0 ) {
        printf("Incorrect dimension Description for index %d. Actual value <%s> is not equal to expected value <%s> \n", dimIndex,
               actualDimDescription.c_str, expectedDimDescription);
        status = -1;
    }
    return status;
}


