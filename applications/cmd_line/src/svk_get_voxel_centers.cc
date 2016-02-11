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
 *      Stojan Maleschlijski
 *
 *  Get a list of voxel centers in LPS
 *
 */

#include <vtkSmartPointer.h>

#include <svkImageReaderFactory.h>
#include <svkImageReader2.h>
#include <svkDdfVolumeReader.h>
#include <svkDcmVolumeReader.h>
#include <svkImageWriterFactory.h>
#include <svkImageWriter.h>
#include <svkDICOMMRSWriter.h>
#include <svkCorrectDCOffset.h>
#include <svkDcmHeader.h>
#include <svkGEPFileReader.h>
#include <svkGEPFileMapper.h>
#include <svkImageAlgorithm.h>
#ifdef WIN32
extern "C" {
#include <getopt.h>
}
#else
#include <getopt.h>
#endif

#define UNDEFINED_TEMP -1111

using namespace svk;



int main (int argc, char** argv)
{

    string usemsg("\n") ;
    usemsg += "Version " + string(SVK_RELEASE_VERSION) + "\n";
    usemsg += "svk_get_voxel_centers -i input_file_name -o output_file_name \n";
    usemsg += "                                                                         \n";
    usemsg += "   -i  name                Name of the MRS DDF file.                  \n";
    usemsg += "   -r  name                Name of an MRI File to calculate Center of mass (optional) \n";
    usemsg += "   -o name                Name of outputfile (ASCII).                  \n";
    usemsg += "   -v           Print a list of the coordinates in the stdout.       \n";
    usemsg += "   -h           Print this help mesage.                                  \n";
    usemsg += "                                                                         \n";
    usemsg += "Application that outputs the voxel centers in LPS coordinates. \n";
    usemsg += " Optionally last line contains the center of mass of the -r MRI image   \n";
    usemsg += "                                                                         \n";


    string inputFileName;
    string mriFileName;
    string outputFileName;

    bool isVerbose = false;

    string cmdLine = svkProvenance::GetCommandLineString( argc, argv );


    static struct option long_options[] =
    {
        /* This option sets a flag. */
        {0, 0, 0, 0}
    };

    // ===============================================
    //  Process flags and arguments
    // ===============================================
    int i;
    int option_index = 0;
    while ( ( i = getopt_long(argc, argv, "i:r:o:hv", long_options, &option_index) ) != EOF) {
        switch (i) {
            case 'i':
                inputFileName.assign( optarg );
                break;
            case 'r':
                mriFileName.assign( optarg );
                break;
            case 'o':
                outputFileName.assign(optarg);
                break;
            case 'v':
                isVerbose = true;
                break;
            case 'h':
                cout << usemsg << endl;
                exit(1);
                break;
            default:
                ;
        }
    }

    argc -= optind;
    argv += optind;

    // ===============================================
    //  validate that:
    //      an output name was supplied
    //      that not suppresses and unsuppressed were both specified
    //      that only the supported output types was requested.
    //
    // ===============================================
    if (
        argc != 0 ||  inputFileName.length() == 0
    ) {
        cout << usemsg << endl;
        exit(1);
    }


    cout << "file name (input): " << inputFileName << endl;

    if (outputFileName.length()!=0){
    	cout << "file name (output) : " << outputFileName << endl;
    }

    // ===============================================
    //  Use a reader factory to get the correct reader
    //  type .
    // ===============================================
    vtkSmartPointer< svkImageReaderFactory > readerFactory = vtkSmartPointer< svkImageReaderFactory >::New();

    svkImageReader2* testReader = readerFactory->CreateImageReader2(inputFileName.c_str());
    if (testReader == NULL) {
        cerr << "Can not determine appropriate reader for test data: " << inputFileName << endl;
        exit(1);
    }
    testReader->SetFileName( inputFileName.c_str() );
    testReader->Update();
    svkMrsImageData* testData = svkMrsImageData::SafeDownCast( testReader->GetOutput() );


    svkDcmHeader::DimensionVector fullDimensionVector = testData->GetDcmHeader()->GetDimensionIndexVector();
    svkDcmHeader::DimensionVector channelDimensionVector = fullDimensionVector;
    //  analyze one channel at a time:
    svkDcmHeader::SetDimensionVectorValue(&channelDimensionVector, svkDcmHeader::CHANNEL_INDEX, 0);

    int numVoxelsPerChannel = svkDcmHeader::GetNumberOfCells( &channelDimensionVector );
    svkDcmHeader::DimensionVector indexVector = fullDimensionVector;
    double LPS[3];
    int indexV[3];
	if ( isVerbose ) {
		cout << "Voxel centers in LPS coordinates: " << endl;
	}
	FILE* mStream = NULL;

	if (outputFileName.length()!=0){
		mStream = fopen(outputFileName.c_str(), "w");
		fprintf(mStream, "Voxel centers in LPS coordinates: \n");
	}
	for( int cellID = 0; cellID < numVoxelsPerChannel; cellID++ ) {

		//  Get the dimensions for the single channel.  reset the channel index and get the
		//  actual cellID for this channel
		svkDcmHeader::GetDimensionVectorIndexFromCellID(&channelDimensionVector, &indexVector, cellID);
		indexV[0]  = svkDcmHeader::GetDimensionVectorValue(&indexVector, svkDcmHeader::COL_INDEX);
		indexV[1]  = svkDcmHeader::GetDimensionVectorValue(&indexVector, svkDcmHeader::ROW_INDEX);
		indexV[2]  = svkDcmHeader::GetDimensionVectorValue(&indexVector, svkDcmHeader::SLICE_INDEX);
		testData->GetPositionFromIndex(indexV, LPS);
		if ( isVerbose ) {
			cout <<  LPS[0] << ", " <<LPS[1] << ", "<<LPS[2] << endl;
		}
		if (mStream!=NULL){
			fprintf(mStream, "%f, %f, %f\n", LPS[0], LPS[1], LPS[2]);
		}
	}
	if (mStream!=NULL){
		// Show the center of mass as a last argument
		if (mriFileName.length()!=0){
			vtkSmartPointer< svkImageReaderFactory > readerFactoryMRI = vtkSmartPointer< svkImageReaderFactory >::New();
			svkImageReader2* mriReader = readerFactoryMRI->CreateImageReader2(mriFileName.c_str());
			if (mriReader == NULL) {
				cerr << "Can not determine appropriate reader for test data: " << mriFileName << endl;
				exit(1);
			}
			mriReader->SetFileName( mriFileName.c_str() );
			mriReader->Update();
			svkMriImageData* mriData = svkMriImageData::SafeDownCast( mriReader->GetOutput() );
			double centerOfMass[3];
			mriData->GetCenterOfMass(centerOfMass);
			mriReader->Delete();

			fprintf(mStream, "%f, %f, %f\n", centerOfMass[0], centerOfMass[1], centerOfMass[2]);
			if ( isVerbose ) {
				cout <<  centerOfMass[0] << ", " <<centerOfMass[1] << ", "<<centerOfMass[2] << endl;
			}
		}
		fclose(mStream);
	}
    // ===============================================
    //  Clean up:
    // ===============================================

    testReader->Delete();

    return 0;
}

