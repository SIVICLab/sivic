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



#include <svkDdfVolumeWriter.h>
#include <svk4DImageData.h>
#include <svkDcmHeader.h>
#include </usr/include/vtk/vtkErrorCode.h>
#include </usr/include/vtk/vtkCellData.h>
#include </usr/include/vtk/vtkExecutive.h>
#include </usr/include/vtk/vtkFloatArray.h>
#include </usr/include/vtk/vtkByteSwap.h>
#include <svkUtils.h>
#include "svkTypeUtils.h"


using namespace svk;


//vtkCxxRevisionMacro(svkDdfVolumeWriter, "$Rev$");
vtkStandardNewMacro(svkDdfVolumeWriter);


const int svkDdfVolumeWriter::ALL_TIME_PTS_IN_FILE = -1;


/*!
 *
 */
svkDdfVolumeWriter::svkDdfVolumeWriter()
{

#if VTK_DEBUG_ON
    this->DebugOn();
#endif

    vtkDebugMacro( << this->GetClassName() << "::" << this->GetClassName() << "()" );

    //default value to indicate undefined, gets initialized in Write method  
    this->numTimePtsPerFile = 0;    
    this->useDescriptiveFileNames = false;  
}


/*!
 *
 */
svkDdfVolumeWriter::~svkDdfVolumeWriter()
{
    vtkDebugMacro( << this->GetClassName() << "::~" << this->GetClassName() << "()" );
}


/*!
 *  Write the UCSF DDF spectroscopy file.  Should support multiple coils (files) and multi-time point data 
 */
void svkDdfVolumeWriter::Write()
{

    vtkDebugMacro( << this->GetClassName() << "::Write()" );
    this->SetErrorCode(vtkErrorCode::NoError);

    //  initialize number of time points per file (all time points in each file by default): 
    if (numTimePtsPerFile == 0 ) { 
        this->numTimePtsPerFile = this->GetImageDataInput(0)->GetDcmHeader()->GetNumberOfTimePoints(); 
   }

    if (! this->FileName ) {
        vtkErrorMacro(<<"Write:Please specify either a FileName or a file prefix and pattern");
        this->SetErrorCode(vtkErrorCode::NoFileNameError);
        return;
    }

    // Make sure the file name is allocated
    this->InternalFileName =
        new char[(this->FileName ? strlen(this->FileName) : 1) +
            (this->FilePrefix ? strlen(this->FilePrefix) : 1) +
            (this->FilePattern ? strlen(this->FilePattern) : 1) + 10];

    this->FileNumber = 0;
    this->MinimumFileNumber = this->FileNumber;
    this->FilesDeleted = 0;
    this->UpdateProgress(0.0);

    // based on number of coils of data:
    this->MaximumFileNumber = this->FileNumber;

    // determine the name
    if (this->FileName) {
        sprintf(this->InternalFileName,"%s",this->FileName);
    } else {
        if (this->FilePrefix) {
            sprintf(this->InternalFileName, this->FilePattern,
                this->FilePrefix, this->FileNumber);
        } else {
            sprintf(this->InternalFileName, this->FilePattern,this->FileNumber);
        }
    } 

    this->SetProvenance(); 
    this->WriteFiles();

    if (this->ErrorCode == vtkErrorCode::OutOfDiskSpaceError) {
        vtkErrorMacro("Ran out of disk space; deleting file(s) already written");
        this->DeleteFiles();
        return;
    }

    delete [] this->InternalFileName;
    this->InternalFileName = NULL;
}


/*!
 *  Appends algo info to provenance record.  
 */
void svkDdfVolumeWriter::SetProvenance()
{
    this->GetImageDataInput(0)->GetProvenance()->AddAlgorithm( this->GetClassName() ); 

    if (  !this->AllTimePointsInEachFile() ) {
        this->GetImageDataInput(0)->GetProvenance()->AddAlgorithmArg( 
            this->GetClassName(),  
            0, 
            "OneTimePointPerFile", 
            true
        ); 
    }

}


/*!
 *  Write the image data pixels and header to the DDF data file (.cmplx) 
 *  and .ddf header. 
 */
void svkDdfVolumeWriter::WriteFiles()
{
    vtkDebugMacro( << this->GetClassName() << "::WriteData()" );

    string dataExtension = ".cmplx"; 
    string hdrExtension = ".ddf"; 

    string fileRoot = 
        string(this->InternalFileName).substr( 0, string(this->InternalFileName).rfind(".") );

    svkDcmHeader* hdr = this->GetImageDataInput(0)->GetDcmHeader(); 
    int dataWordSize = 4; 
    int cols       = hdr->GetIntValue( "Columns" );
    int rows       = hdr->GetIntValue( "Rows" );
    int slices     = hdr->GetNumberOfSlices();  
    int numCoils   = hdr->GetNumberOfCoils();
    int numTimePts = hdr->GetNumberOfTimePoints();  
    int specPts    = hdr->GetIntValue( "DataPointColumns" );
    string representation = hdr->GetStringValue( "DataRepresentation" );

    int numComponents = 1;
    if (representation.compare("COMPLEX") == 0) {
        numComponents = 2;
    }

    vtkCellData* cellData = this->GetImageDataInput(0)->GetCellData();

    int dataLengthPerFile = cols * rows * slices * specPts * numComponents * this->GetNumberTimePointsPerFile(); 
    int coilOffset = cols * rows * slices * numTimePts;     //number of spectra per coil
    int timePtOffset = cols * rows * slices; 

    // write out one coil per ddf file
    float* specData = new float [ dataLengthPerFile ];

    string fileName;

    svkDcmHeader::DimensionVector origDimensionVector = hdr->GetDimensionIndexVector(); 
    svkDcmHeader::DimensionVector dimensionVector = origDimensionVector; 
    svkDcmHeader::DimensionVector loopVector = dimensionVector; 

    //  looping varies depending on how many time points go into each file:
    if ( this->AllTimePointsInEachFile() ) {

        //  In this case we only want to loop over dimensions other than the time dimension, so set 
        //  time to a single value in this vector:
        svkDcmHeader::SetDimensionVectorValue(&dimensionVector, svkDcmHeader::TIME_INDEX, 0);

        //  Squash the slice frame to a single value.  The slice frame dimension will get looped 
        //  over inside InitSpecData.  numFrames, will only account for other dimensions.  
        svkDcmHeader::DimensionVector noSliceVector = dimensionVector; 
        svkDcmHeader::SetDimensionVectorValue(&noSliceVector, svkDcmHeader::SLICE_INDEX, 0);

        int numFrames = hdr->GetNumberOfFrames(&noSliceVector); 

        for (int frame = 0; frame < numFrames; frame++) {

            fileName = this->GetFileRootName(fileRoot, &noSliceVector, frame);
            //svkDcmHeader::PrintDimensionIndexVector(&noSliceVector);
            //cout << "DDF FILE NAMEa: " << frame << " = " << fileName << endl;

            ofstream cmplxOut( ( fileName + dataExtension ).c_str(), ios::binary);
            ofstream hdrOut(   ( fileName + hdrExtension ).c_str() );
            if( !cmplxOut || !hdrOut ) {
                throw runtime_error("Cannot open .ddf or .cmplx file for writing");
            }

            for (int timePt = 0; timePt < numTimePts; timePt++) {
                hdr->GetDimensionVectorIndexFromFrame(&noSliceVector, &loopVector, frame);
                svkDcmHeader::SetDimensionVectorValue(&loopVector, svkDcmHeader::TIME_INDEX, timePt);
                //svkDcmHeader::PrintDimensionIndexVector(&loopVector); 
                this->InitSpecData(specData, &origDimensionVector, &loopVector); 
            }

            this->InitHeader( &hdrOut, fileName ); 

            //  cmplx files are by definition big endian:
            #ifndef VTK_WORDS_BIGENDIAN
                vtkByteSwap::SwapVoidRange((void*)specData, dataLengthPerFile, sizeof(float));
            #endif

            cmplxOut.write( reinterpret_cast<char *>(specData), dataLengthPerFile * dataWordSize);
            cmplxOut.close();
            hdrOut.close(); 

        }

    } else if ( ! this->AllTimePointsInEachFile() ) {

        //  In this case we only want to loop over dimensions other than the time dimension, so set 
        //  time to a single value in this vector:

        int numFrames = hdr->GetNumberOfFrames(&dimensionVector); 
        for (int frame = 0; frame < numFrames; frame++) {

            fileName = this->GetFileRootName(fileRoot, &dimensionVector, frame);
            //cout << "DDF FILE NAMEb: " << frame << " = " << fileName << endl;

            ofstream cmplxOut( ( fileName + dataExtension ).c_str(), ios::binary);
            ofstream hdrOut(   ( fileName + hdrExtension ).c_str() );
            if( !cmplxOut || !hdrOut ) {
                throw runtime_error("Cannot open .ddf or .cmplx file for writing");
            }

            hdr->GetDimensionVectorIndexFromFrame(&dimensionVector, &loopVector, frame);
            this->InitSpecData(specData, &dimensionVector, &loopVector); 

            this->InitHeader( &hdrOut, fileName );

            //  cmplx files are by definition big endian:
            #ifndef VTK_WORDS_BIGENDIAN
                vtkByteSwap::SwapVoidRange((void*)specData, dataLengthPerFile, sizeof(float));
            #endif

            cmplxOut.write( reinterpret_cast<char *>(specData), dataLengthPerFile * dataWordSize);
            cmplxOut.close();
            hdrOut.close(); 
        }
    }

    delete [] specData;

}


/*!
 *  Loops over x,y,z spatial indices to initialize the specData buffer from data in the 
 *  svkImageData object.  The specData buffer is what gets written to the
 *  cmplx file.  With the exception of the offsetOut, this is the same for both blocks of 
 *  WriteData().  The dimensionVector argument is the current set of indices to write, not necessarily
 *  the data dimensionality. 
 */
void svkDdfVolumeWriter::InitSpecData(float* specData, svkDcmHeader::DimensionVector* dimensionVector, svkDcmHeader::DimensionVector* indexVector) 
{

    svkDcmHeader* hdr = this->GetImageDataInput(0)->GetDcmHeader(); 
    int cols       = hdr->GetIntValue( "Columns" );
    int rows       = hdr->GetIntValue( "Rows" );
    int slices     = hdr->GetNumberOfSlices();  
    int numTimePts = hdr->GetNumberOfTimePoints();  
    int specPts    = hdr->GetIntValue( "DataPointColumns" );
    int timePt     = svkDcmHeader::GetDimensionVectorValue(indexVector, svkDcmHeader::TIME_INDEX);

    int numComponents = 1;
    string representation = hdr->GetStringValue( "DataRepresentation" );
    if (representation.compare("COMPLEX") == 0) {
        numComponents = 2;
    }

    int coilOffset = cols * rows * slices * numTimePts;     //number of spectra per coil
    int timePtOffset = cols * rows * slices; 

    vtkFloatArray* fa;
    float* dataTuple = new float[numComponents];

    vtkCellData* cellData = this->GetImageDataInput(0)->GetCellData();

    for (int z = 0; z < slices; z++) {
        for (int y = 0; y < rows; y++) {
            for (int x = 0; x < cols; x++) {

                svkDcmHeader::SetDimensionVectorValue(indexVector, svkDcmHeader::COL_INDEX, x);
                svkDcmHeader::SetDimensionVectorValue(indexVector, svkDcmHeader::ROW_INDEX, y);
                svkDcmHeader::SetDimensionVectorValue(indexVector, svkDcmHeader::SLICE_INDEX, z);

                //  If all time points written to a single ddf file, then add the time
                //  offset in the output file here, otherwise time doesn't 
                //  affect the offset as each time point goes to a separate file
                //  However, the input offset does get incremented
                int offsetOut = ( cols * rows * z ) + ( cols * y ) + x; 
                if (  this->AllTimePointsInEachFile() ) {
                    offsetOut += ( timePt * timePtOffset );
                }

                //int offset = offsetOut + coilNum * coilOffset;
                //if (  ! this->AllTimePointsInEachFile() ) {
                 //   offset += timePt * timePtOffset;
                //}
                //cout << "DV: " << endl;
                //svkDcmHeader::PrintDimensionIndexVector(dimensionVector); 
                //cout << "IV: " << endl;
                //svkDcmHeader::PrintDimensionIndexVector(indexVector); 

                int cellID = svkDcmHeader::GetCellIDFromDimensionVectorIndex(dimensionVector, indexVector); 
                //svkDcmHeader::PrintDimensionIndexVector(indexVector);
                fa =  vtkFloatArray::SafeDownCast( cellData->GetArray( cellID) );

                for (int i = 0; i < specPts; i++) {

                    fa->GetTypedTuple(i, dataTuple);

                    for (int j = 0; j < numComponents; j++) {
                        specData[ (offsetOut * specPts * numComponents) + (i * numComponents) + j ] = dataTuple[j];
                    }
                }
            }
        }
    }

    delete[] dataTuple; 

}


/*!
 *  initializes the ofstream header content for the DDF header.
 */
void svkDdfVolumeWriter::InitHeader(ofstream* out, string fileName)
{

    svkDcmHeader* hdr = this->GetImageDataInput(0)->GetDcmHeader(); 

    string deidMethod = hdr->GetStringValue( "DeidentificationMethod" );
    bool deidentified = false; 
    if (  deidMethod.compare("DEIDENTIFIED") == 0 ){ 
        deidentified = true; 
    }

    *out << "DATA DESCRIPTOR FILE" << endl;
    *out << "version: 6.1" << endl;
    *out << "object type: MR Spectroscopy" << endl;
    *out << "patient id: " << setw(19) << left << hdr->GetStringValue( "PatientID" ) << endl;
    *out << "patient name: " << setw(63) << left << this->GetDDFPatientName( hdr->GetStringValue( "PatientName" ) ) << endl;
    *out << "patient code: " << endl;


    string dob = hdr->GetStringValue( "PatientBirthDate" ); 

    if ( deidentified ) { 
        *out << "date of birth: " << dob << endl; 
    } else {
        if ( dob.length() == 0 ) {
            *out << "date of birth: " << endl; 
        } else {
            *out << "date of birth: " << 
                dob[4] << dob[5] << "/" << dob[6] << dob[7] << "/" << dob[0] << dob[1] << dob[2] << dob[3] << endl;
        }
    }

    *out << "sex: " << hdr->GetStringValue( "PatientSex" ) <<  endl;
    *out << "study id: " << hdr->GetStringValue( "StudyID" ) <<  endl;
    *out << "study code: " << "" <<  endl;

    string date = hdr->GetStringValue( "StudyDate" );
    if (  deidentified ) { 
        *out << "study date: " << date << endl;
    } else {
        if ( date.length() == 0 ) {
            *out << "study date: " << endl;
        } else {
            *out << "study date: " << 
            date[4] << date[5] << "/" << date[6] << date[7] << "/" << date[0] << date[1] << date[2] << date[3] << endl;
        }
    }

    *out << "accession number: " << hdr->GetStringValue( "AccessionNumber" ) <<  endl;
    *out << "root name: " << setw(7) << fileName <<  endl;
    *out << "series number: " << hdr->GetStringValue( "SeriesNumber" ) <<  endl;
    *out << "series description: " << hdr->GetStringValue( "SeriesDescription" ) <<  endl;
    *out << "comment: " << " " << endl;

    *out << "patient entry: ";
    string position_string = hdr->GetStringValue( "PatientPosition" );
    if ( position_string.substr(0,2) == string( "HF" ) ){
        *out << "head first";
    } else if ( position_string.substr(0,2) == string( "FF" ) ) {
        *out << "feet first";
    } else {
        *out << "UNKNOWN";
    }
    *out << endl;

    *out << "patient position: ";
    if (position_string.length() >= 2 ) {
        if ( position_string.substr(2) == string( "S" ) ) {
            *out << "supine" << endl;
        } else if ( position_string.substr(2) == string( "P" ) ) {
            *out << "prone" << endl;
        } else if ( position_string.substr(2) == string( "DL" ) ) {
            *out << "decubitus left" << endl;
        } else if ( position_string.substr(2) == string( "DR" ) ) {
            *out << "decubitus right" << endl;
        } else {
            *out << "UNKNOWN" << endl;
        }
    } else {
        *out << "UNKNOWN" << endl;
    }

    double orientation[2][3];
    hdr->GetOrientation(orientation);
    string orientationString; 

    if ( ( fabs( orientation[0][0] ) == 1 && fabs( orientation[1][1] ) == 1 ) ||
        ( fabs(orientation[0][1]) ==1 && fabs( orientation[1][0] ) == 1 ) ) {
        orientationString.assign( "axial" ); 
    } else if ( ( fabs( orientation[0][1] ) == 1 && fabs( orientation[1][2] ) == 1 ) ||
        ( fabs( orientation[0][2] ) == 1 && fabs( orientation[1][1] ) == 1 ) ) {
        orientationString.assign( "sagittal" ); 
    } else if ( ( fabs( orientation[0][0] ) == 1 && fabs( orientation[1][2] ) == 1) ||
        ( fabs( orientation[0][2] ) == 1 && fabs( orientation[1][0] ) == 1 ) ) {
        orientationString.assign( "coronal" ); 
    } else {
        orientationString.assign( "oblique" ); 
    }

    *out << "orientation: " << orientationString << endl;
    *out << "data type: floating point" << endl;

    int numComponents = 0;      
    if ( hdr->GetStringValue("DataRepresentation").compare("COMPLEX") == 0 ) {
        numComponents = 2;     
    }
    *out << "number of components: " << numComponents << endl; 

    *out << "source description: " << endl;

    int numDims = 4; 

    // If all time points are written to a single file, then add the time dimension.
    if (  this->AllTimePointsInEachFile() ) {

        if ( hdr->GetNumberOfTimePoints()  > 1 ) { 
            numDims = 5; 
        }

    }
    *out << "number of dimensions: " << numDims << endl; 

    string specDomain = this->GetDimensionDomain( hdr->GetStringValue( "SignalDomainColumns") ); 

    string spatialDomains[3];  
    spatialDomains[0] = this->GetDimensionDomain( hdr->GetStringValue( "SVK_ColumnsDomain") ); 
    spatialDomains[1] = this->GetDimensionDomain( hdr->GetStringValue( "SVK_RowsDomain") ); 
    spatialDomains[2] = this->GetDimensionDomain( hdr->GetStringValue( "SVK_SliceDomain") ); 

    int numVoxels[3];  
    numVoxels[0] = hdr->GetIntValue( "Columns" ); 
    numVoxels[1] = hdr->GetIntValue( "Rows" ); 
    numVoxels[2] = hdr->GetNumberOfSlices(); 
    
    double voxelSpacing[3]; 
    hdr->GetPixelSpacing( voxelSpacing );  
    *out << "dimension 1: type: " << specDomain << " npoints: " << setw(5) << left << hdr->GetIntValue( "DataPointColumns" ) << endl;
    *out << "dimension 2: type: " << spatialDomains[0] << " npoints: " << numVoxels[0] << 
        " pixel spacing(mm): " << fixed << left << setw(9) << setprecision(6) << voxelSpacing[0] << endl;
    *out << "dimension 3: type: " << spatialDomains[1] << " npoints: " << numVoxels[1] << 
        " pixel spacing(mm): " << fixed << left << setw(9) << setprecision(6) << voxelSpacing[1] << endl;
    *out << "dimension 4: type: " << spatialDomains[2] << " npoints: " << numVoxels[2] << 
        " pixel spacing(mm): " << fixed << left << setw(9) << setprecision(6) << voxelSpacing[2] << endl;

    // If all time points are written to a single file, then add the time dimension.
    if (  this->AllTimePointsInEachFile() ) {
        if ( numDims == 5 ) {    
            *out << "dimension 5: type: time npoints: " << hdr->GetNumberOfTimePoints() << endl ; 
        }
    }
    
    float center[3];
    this->GetDDFCenter( center );
    *out << "center(lps, mm): " << fixed << right << setw(14) << setprecision(5) << center[0]
         << setw(14) << center[1] << setw(14) << center[2] << endl;

    double positionFirst[3]; 
    hdr->GetOrigin(positionFirst, 0);
    *out << "toplc(lps, mm):  " << fixed << right << setw(14) << setprecision(5)
         << positionFirst[0]
         << setw(14) << positionFirst[1]
         << setw(14) << positionFirst[2] <<endl;

    double dcos[3][3];
    this->GetImageDataInput(0)->GetDcos(dcos);
    *out << "dcos0: " << fixed << setw(14) << setprecision(5) << dcos[0][0] << setw(14) << dcos[0][1]
         << setw(14) << dcos[0][2] << endl;
    *out << "dcos1: " << fixed << setw(14) << setprecision(5) << dcos[1][0]
         << setw(14) << dcos[1][1] << setw(14) << dcos[1][2] << endl;
    *out << "dcos2: " << fixed << setw(14) << setprecision(5) << dcos[2][0] << setw(14)
         << dcos[2][1] << setw(14) << dcos[2][2] << endl;

    *out << "===================================================" << endl; 
    *out << "MR Parameters" << endl; 

    string coilName = hdr->GetStringSequenceItemElement(
        "MRReceiveCoilSequence",
        0,
        "ReceiveCoilName",
        "SharedFunctionalGroupsSequence",
        0
    );
    *out << "coil name: " << fixed << left << setw(15) << coilName << endl; 

    double voxelSize[3]; 
    hdr->GetPixelSize( voxelSize );  
    *out << "slice gap(mm): " << voxelSpacing[2] - voxelSize[2] << endl;

    float TE = hdr->GetFloatSequenceItemElement(
        "MREchoSequence",
        0,
        "EffectiveEchoTime",
        "SharedFunctionalGroupsSequence",
        0
    );
    *out << "echo time(ms): " << fixed << setprecision(6) << TE << endl; 

    float TR = hdr->GetFloatSequenceItemElement(
        "MRTimingAndRelatedParametersSequence",
        0,
        "RepetitionTime",
        "SharedFunctionalGroupsSequence",
        0
    );
    *out << "repetition time(ms): " << fixed << setprecision(6) << TR << endl;

    *out << "inversion time(ms): " << fixed << setprecision(6) << endl; 
    
    float flipAngle =  hdr->GetFloatSequenceItemElement(
        "MRTimingAndRelatedParametersSequence",
        0,
        "FlipAngle",
        "SharedFunctionalGroupsSequence",
        0
    );
    *out << "flip angle: " << fixed << setprecision(6) << flipAngle << endl;

    *out << "pulse sequence name: " << fixed << setw(15) << hdr->GetStringValue( "PulseSequenceName" ) << endl;
    *out << "transmitter frequency(MHz): " << hdr->GetFloatValue( "TransmitterFrequency" ) << endl; 
    *out << "isotope: " << hdr->GetStringValue( "ResonantNucleus" ) << endl;
    *out << "field strength(T): " <<  hdr->GetFloatValue( "MagneticFieldStrength" ) << endl; 
    int numSatBands = hdr->GetNumberOfItemsInSequence("MRSpatialSaturationSequence");
    *out << "number of sat bands: " << numSatBands << endl;
    for ( int satBand = 0; satBand < numSatBands; satBand++ ) {

        float thickness = hdr->GetFloatSequenceItemElement( 
            "MRSpatialSaturationSequence", satBand, "SlabThickness", "SharedFunctionalGroupsSequence"
        );
            
        float pos0 = hdr->GetFloatSequenceItemElement(
            "MRSpatialSaturationSequence", satBand, "MidSlabPosition", "SharedFunctionalGroupsSequence", 0, 0
        );
        float pos1 = hdr->GetFloatSequenceItemElement(
            "MRSpatialSaturationSequence", satBand, "MidSlabPosition", "SharedFunctionalGroupsSequence", 0, 1
        );
        float pos2 = hdr->GetFloatSequenceItemElement(
            "MRSpatialSaturationSequence", satBand, "MidSlabPosition", "SharedFunctionalGroupsSequence", 0, 2
        );

        float norm0 = hdr->GetFloatSequenceItemElement(
            "MRSpatialSaturationSequence", satBand, "SlabOrientation", "SharedFunctionalGroupsSequence", 0, 0
        );
        float norm1 = hdr->GetFloatSequenceItemElement(
            "MRSpatialSaturationSequence", satBand, "SlabOrientation", "SharedFunctionalGroupsSequence", 0, 1
        );
        float norm2 = hdr->GetFloatSequenceItemElement(
            "MRSpatialSaturationSequence", satBand, "SlabOrientation", "SharedFunctionalGroupsSequence", 0, 2
        );

        *out << "sat band " << satBand + 1 << " thickness(mm): " << fixed << setprecision(6) << thickness << endl; 
        *out << "sat band " << satBand + 1 << " orientation: " << fixed << right << setw(21) << setprecision(5) << norm0 
                                                          << fixed << right << setw(14) << setprecision(5) << norm1 
                                                          << fixed << right << setw(14) << setprecision(5) << norm2 
                                                          << endl;  

        *out << "sat band " << satBand + 1 << " position(lps, mm): " << fixed << right << setw(15) << setprecision(5) << pos0 
                                                                << fixed << right << setw(14) << setprecision(5) << pos1 
                                                                << fixed << right << setw(14) << setprecision(5) << pos2 
                                                                << endl;  

    }

    float freqOffset = 0; 
    if ( hdr->ElementExists( "SVK_FrequencyOffset" ) ) {
        freqOffset = hdr->GetFloatValue( "SVK_FrequencyOffset" ); 
    }
                                   
    *out << "===================================================" << endl; 
    *out << "Spectroscopy Parameters" << endl; 

    *out << "localization type: " << hdr->GetStringValue( "VolumeLocalizationTechnique" ) << endl;
    *out << "center frequency(MHz): " << fixed << setprecision(6) << hdr->GetFloatValue( "TransmitterFrequency" ) << endl;
    *out << "ppm reference: " << fixed << setprecision(6) << hdr->GetFloatValue( "ChemicalShiftReference" ) << endl;
    *out << "sweepwidth(Hz): " << fixed << setprecision(6) << hdr->GetFloatValue( "SpectralWidth" ) << endl;
    *out << "dwelltime(ms): " << fixed << setprecision(6) << 1000/( hdr->GetFloatValue( "SpectralWidth" ) ) << endl;
    *out << "frequency offset(Hz): " << fixed << setprecision(6) << freqOffset  << endl;
    string onH20 = "yes"; 
    //  if this is not H1, then set to no:
    if ( hdr->GetStringValue( "ResonantNucleus" ).compare("1H") != 0 ) {
        onH20.assign( "" ); 
    }
    if ( freqOffset != 0 ) {
        onH20.assign( "no" ); 
    }
    *out << "centered on water: " << onH20 << endl;
    *out << "suppression technique: " << endl;
    *out << "residual water:" << endl;
   
    int numAcqs = static_cast< int > (   
        hdr->GetFloatSequenceItemElement(
            "MRAveragesSequence", 0, "NumberOfAverages", 
            "SharedFunctionalGroupsSequence", 0
        )
    ); 
    *out << "number of acquisitions: " << numAcqs << endl; 

    string chop = hdr->GetStringValue( "SVK_AcquisitionChop" );
    if ( chop.compare( "YES" ) == 0 ) {
        chop = "yes"; 
    } else {
        chop = "no"; 
    }
    *out << "chop: " << chop << endl;


    string evenSymmetry = ""; 
    if ( hdr->ElementExists( "SVK_K0Sampled" ) ) {
        string k0Sampled = hdr->GetStringValue( "SVK_K0Sampled" );
        if ( k0Sampled.compare( "YES" ) == 0 ) {
            //  if odd dims
            if (  (numVoxels[0] > 1 && numVoxels[0] % 2 )
               || (numVoxels[1] > 1 && numVoxels[1] % 2 )
               || (numVoxels[2] > 1 && numVoxels[2] % 2 ) ) {
                evenSymmetry = "yes"; 
            } else {
                evenSymmetry = "no"; 
            }
        } else if ( k0Sampled.compare( "NO" ) == 0 ) {
            if ( (numVoxels[0] > 1 && numVoxels[0] % 2 )
              || (numVoxels[1] > 1 && numVoxels[1] % 2 )
              || (numVoxels[2] > 1 && numVoxels[2] % 2 ) ) {
                evenSymmetry = "no"; 
            } else {
                evenSymmetry = "yes"; 
            }
        }
    }
    *out << "even symmetry: " << evenSymmetry << endl;
    *out << "data reordered: " << endl;

    float acqTLC[3];
    if( hdr->ElementExists( "SVK_SpectroscopyAcquisitionTLC" ) ) {
        for (int i = 0; i < 3; i++) {
            acqTLC[i] = hdr->GetFloatSequenceItemElement(
                "MRSpectroscopyFOVGeometrySequence", 0, "SVK_SpectroscopyAcquisitionTLC", 
                "SharedFunctionalGroupsSequence", 0, i
            );
        }
    } else {
        for (int i = 0; i < 3; i++ ) {
            acqTLC[i] = positionFirst[i]; 
        }
    }

    *out << "acq. toplc(lps, mm):  " << fixed << right << setw(13) << setprecision(5)
         << acqTLC[0]
         << setw(14) << acqTLC[1]
         << setw(14) << acqTLC[2] << endl;

    float acqSpacing[3];
    if( hdr->ElementExists( "SVK_SpectroscopyAcquisitionPixelSpacing" ) ) {
        for (int i = 0; i < 2; i++) {
            acqSpacing[i] = hdr->GetFloatSequenceItemElement(
                "MRSpectroscopyFOVGeometrySequence", 0, "SVK_SpectroscopyAcquisitionPixelSpacing", 
                "SharedFunctionalGroupsSequence", 0, i
            );
        }
        acqSpacing[2] = hdr->GetFloatSequenceItemElement(
            "MRSpectroscopyFOVGeometrySequence", 0, "SVK_SpectroscopyAcquisitionSliceThickness", 
            "SharedFunctionalGroupsSequence", 0
        );
    } else {
        for (int i = 0; i < 3; i++ ) {
            acqSpacing[i] = voxelSpacing[i]; 
        }
    }

    *out << "acq. spacing(mm):  " << fixed << right << setw(16) << setprecision(5)
         << acqSpacing[0]
         << setw(14) << acqSpacing[1]
         << setw(14) << acqSpacing[2] << endl;

    *out << "acq. number of data points: " << 
        hdr->GetIntSequenceItemElement(
            "MRSpectroscopyFOVGeometrySequence", 0, "SpectroscopyAcquisitionDataColumns", 
            "SharedFunctionalGroupsSequence", 0 
        )
    << endl;

    int acqPts1 = hdr->GetIntSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence", 0, "SpectroscopyAcquisitionPhaseColumns", 
        "SharedFunctionalGroupsSequence", 0 
    );
    int acqPts2 = hdr->GetIntSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence", 0, "SpectroscopyAcquisitionPhaseRows", 
        "SharedFunctionalGroupsSequence", 0 
    );
    int acqPts3 = hdr->GetIntSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence", 0, "SpectroscopyAcquisitionOutOfPlanePhaseSteps", 
        "SharedFunctionalGroupsSequence", 0 
    );
    *out << "acq. number of points: " << acqPts1 << " " << acqPts2 << " " << acqPts3 << endl;

    float acqDcos[9]; 
    if( hdr->ElementExists( "SVK_SpectroscopyAcquisitionOrientation" ) ) {
        for (int i = 0; i < 9; i++) {
            acqDcos[i] = hdr->GetFloatSequenceItemElement(
                "MRSpectroscopyFOVGeometrySequence", 0, "SVK_SpectroscopyAcquisitionOrientation", 
                "SharedFunctionalGroupsSequence", 0, i
            );
        }
    } else {
        acqDcos[0] = dcos[0][0]; 
        acqDcos[1] = dcos[0][1]; 
        acqDcos[2] = dcos[0][2]; 
        acqDcos[3] = dcos[1][0]; 
        acqDcos[4] = dcos[1][1]; 
        acqDcos[5] = dcos[1][2]; 
        acqDcos[6] = dcos[2][0]; 
        acqDcos[7] = dcos[2][1]; 
        acqDcos[8] = dcos[2][2]; 
    }

    *out << "acq. dcos1: " << fixed << setw(14) << setprecision(5) << acqDcos[0] << setw(14) << acqDcos[1]
         << setw(14) << acqDcos[2] << endl;
    *out << "acq. dcos2: " << fixed << setw(14) << setprecision(5) << acqDcos[3]
         << setw(14) << acqDcos[4] << setw(14) << acqDcos[5] << endl;
    *out << "acq. dcos3: " << fixed << setw(14) << setprecision(5) << acqDcos[6] << setw(14)
         << acqDcos[7] << setw(14) << acqDcos[8] << endl;

    float selBoxSize[3]; 
    float selBoxCenter[3];
    selBoxSize[0] = 0.;
    selBoxSize[1] = 0.;
    selBoxSize[2] = 0.;
    selBoxCenter[0] = 0.;
    selBoxCenter[1] = 0.;
    selBoxCenter[2] = 0.;

    if( hdr->ElementExists( "VolumeLocalizationSequence" ) ) {

        int numberOfItems = hdr->GetNumberOfItemsInSequence("VolumeLocalizationSequence");

        for (int i = 0; i < numberOfItems; i++) {
            selBoxSize[i] = hdr->GetFloatSequenceItemElement(
                "VolumeLocalizationSequence",
                i,
                "SlabThickness"
            );
    
            selBoxCenter[i] = hdr->GetFloatSequenceItemElement(
                "VolumeLocalizationSequence",
                0,
                "MidSlabPosition", 
                NULL,      
                0,
                i 
            );

        }
    }

    *out << "selection center(lps, mm): " << fixed << setw(14) << setprecision(5) << selBoxCenter[0] 
                                          << fixed << setw(14) << setprecision(5) << selBoxCenter[1]  
                                          << fixed << setw(14) << setprecision(5) << selBoxCenter[2] 
                                          << endl;  

    *out << "selection size(mm): " << fixed << setw(21) << setprecision(5) << selBoxSize[0] 
                                   << fixed << setw(14) << setprecision(5) << selBoxSize[1]  
                                   << fixed << setw(14) << setprecision(5) << selBoxSize[2] 
                                   << endl;  


    float selBoxOrientation[3][3]; 
    selBoxOrientation[0][0] = 0; 
    selBoxOrientation[0][1] = 0; 
    selBoxOrientation[0][2] = 0; 
    selBoxOrientation[1][0] = 0; 
    selBoxOrientation[1][1] = 0; 
    selBoxOrientation[1][2] = 0; 
    selBoxOrientation[2][0] = 0; 
    selBoxOrientation[2][1] = 0; 
    selBoxOrientation[2][2] = 0; 
    if( hdr->ElementExists( "VolumeLocalizationSequence" ) ) {

        int numberOfItems = hdr->GetNumberOfItemsInSequence("VolumeLocalizationSequence");
        for (int i = 0; i < numberOfItems; i++) {
            for (int j = 0; j < 3; j++) {

                selBoxOrientation[i][j] = hdr->GetFloatSequenceItemElement(
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

    *out << "selection dcos1: " << fixed << setw(14) << setprecision(5) << selBoxOrientation[0][0] 
                                << fixed << setw(14) << setprecision(5) << selBoxOrientation[0][1]  
                                << fixed << setw(14) << setprecision(5) << selBoxOrientation[0][2] 
                                << endl;  

    *out << "selection dcos2: " << fixed << setw(14) << setprecision(5) << selBoxOrientation[1][0] 
                                << fixed << setw(14) << setprecision(5) << selBoxOrientation[1][1]  
                                << fixed << setw(14) << setprecision(5) << selBoxOrientation[1][2] 
                                << endl;  

    *out << "selection dcos3: " << fixed << setw(14) << setprecision(5) << selBoxOrientation[2][0] 
                                << fixed << setw(14) << setprecision(5) << selBoxOrientation[2][1]  
                                << fixed << setw(14) << setprecision(5) << selBoxOrientation[2][2] 
                                << endl;  

    float reorderedTLC[3];
    if( hdr->ElementExists( "SVK_SpectroscopyAcqReorderedTLC" ) ) {
        for (int i = 0; i < 3; i++) {
            reorderedTLC[i] = hdr->GetFloatSequenceItemElement(
                "MRSpectroscopyFOVGeometrySequence", 0, "SVK_SpectroscopyAcqReorderedTLC",
                "SharedFunctionalGroupsSequence", 0, i
            );
        }
    } else {
        for (int i = 0; i < 3; i++ ) {
            reorderedTLC[i] = positionFirst[i]; 
        }
    }

    *out << "reordered toplc(lps, mm):  " << fixed << right << setw(14) << setprecision(5)
         << reorderedTLC[0]
         << setw(14) << reorderedTLC[1]
         << setw(14) << reorderedTLC[2] << endl;

    float reorderedCenter[3]; 
    this->GetDDFCenter( reorderedCenter, "reordered" );
    *out << "reordered center(lps, mm): " << fixed << setw(14) << setprecision(5) << reorderedCenter[0] 
                                          << fixed << setw(14) << setprecision(5) << reorderedCenter[1]  
                                          << fixed << setw(14) << setprecision(5) << reorderedCenter[2] 
                                          << endl;  

    float reorderedSpacing[3];
    if( hdr->ElementExists( "SVK_SpectroscopyAcqReorderedPixelSpacing" ) ) {
        for (int i = 0; i < 2; i++) {
            reorderedSpacing[i] = hdr->GetFloatSequenceItemElement(
                "MRSpectroscopyFOVGeometrySequence", 0, "SVK_SpectroscopyAcqReorderedPixelSpacing",
                "SharedFunctionalGroupsSequence", 0, i
            );
        }
        reorderedSpacing[2] = hdr->GetFloatSequenceItemElement(
            "MRSpectroscopyFOVGeometrySequence", 0, "SVK_SpectroscopyAcqReorderedSliceThickness",
            "SharedFunctionalGroupsSequence", 0
        );
    } else {
        for (int i = 0; i < 3; i++ ) {
            reorderedSpacing[i] = voxelSpacing[i]; 
        }
    }

    *out << "reordered spacing(mm):  " << fixed << right << setw(17) << setprecision(5)
         << reorderedSpacing[0]
         << setw(14) << reorderedSpacing[1]
         << setw(14) << reorderedSpacing[2] << endl;

    int reorderedPts1;
    int reorderedPts2;
    int reorderedPts3; 
    if( hdr->ElementExists( "SVK_SpectroscopyAcqReorderedPhaseColumns" ) ) {
        reorderedPts1 = hdr->GetIntSequenceItemElement(
            "MRSpectroscopyFOVGeometrySequence", 0, "SVK_SpectroscopyAcqReorderedPhaseColumns",
            "SharedFunctionalGroupsSequence", 0
        );
        reorderedPts2 = hdr->GetIntSequenceItemElement(
            "MRSpectroscopyFOVGeometrySequence", 0, "SVK_SpectroscopyAcqReorderedPhaseRows",
            "SharedFunctionalGroupsSequence", 0
        );
        reorderedPts3 = hdr->GetIntSequenceItemElement(
            "MRSpectroscopyFOVGeometrySequence", 0, "SVK_SpectroscopyAcqReorderedOutOfPlanePhaseSteps",
            "SharedFunctionalGroupsSequence", 0
        );
    } else {
        reorderedPts1 =  hdr->GetIntValue("Columns");
        reorderedPts2 =  hdr->GetIntValue("Rows");
        reorderedPts3 =  hdr->GetNumberOfSlices();
    }
    *out << "reordered number of points: " << reorderedPts1 << " " << reorderedPts2 << " " << reorderedPts3 << endl;

    float reorderedDcos[9];
    if( hdr->ElementExists( "SVK_SpectroscopyAcqReorderedOrientation" ) ) {
        for (int i = 0; i < 9; i++) {
            reorderedDcos[i] = hdr->GetFloatSequenceItemElement(
                "MRSpectroscopyFOVGeometrySequence", 0, "SVK_SpectroscopyAcqReorderedOrientation",
                "SharedFunctionalGroupsSequence", 0, i
            );
        }
    } else {
        reorderedDcos[0] = dcos[0][0]; 
        reorderedDcos[1] = dcos[0][1]; 
        reorderedDcos[2] = dcos[0][2]; 
        reorderedDcos[3] = dcos[1][0]; 
        reorderedDcos[4] = dcos[1][1]; 
        reorderedDcos[5] = dcos[1][2]; 
        reorderedDcos[6] = dcos[2][0]; 
        reorderedDcos[7] = dcos[2][1]; 
        reorderedDcos[8] = dcos[2][2]; 
    }

    *out << "reordered dcos1: " << fixed << setw(14) << setprecision(5) << reorderedDcos[0] << setw(14) << reorderedDcos[1]
         << setw(14) << reorderedDcos[2] << endl;
    *out << "reordered dcos2: " << fixed << setw(14) << setprecision(5) << reorderedDcos[3]
         << setw(14) << reorderedDcos[4] << setw(14) << reorderedDcos[5] << endl;
    *out << "reordered dcos3: " << fixed << setw(14) << setprecision(5) << reorderedDcos[6] << setw(14)
         << reorderedDcos[7] << setw(14) << reorderedDcos[8] << endl;

    *out << "===================================================" << endl; 

    this->GetImageDataInput(0)->GetProvenance()->PrintXML(*out); 

}


/*!
 *   
 */
void svkDdfVolumeWriter::GetDDFCenter(float center[3], string centerType)
{

    svkDcmHeader* hdr = this->GetImageDataInput(0)->GetDcmHeader(); 

    double pixelSpacing[3];
    hdr->GetPixelSpacing(pixelSpacing);

    double slicePositionFirst[3];
    hdr->GetOrigin(slicePositionFirst, 0);

    double dcos[3][3];
    this->GetImageDataInput(0)->GetDcos(dcos);

    int numPix[3]; 
    if ( centerType.compare( "reordered" ) == 0 ) {
        if( hdr->ElementExists( "SVK_SpectroscopyAcqReorderedPhaseColumns" ) ) {
            numPix[0] = hdr->GetIntSequenceItemElement(
                "MRSpectroscopyFOVGeometrySequence", 0, "SVK_SpectroscopyAcqReorderedPhaseColumns",
                "SharedFunctionalGroupsSequence", 0
            );
            numPix[1] = hdr->GetIntSequenceItemElement(
                "MRSpectroscopyFOVGeometrySequence", 0, "SVK_SpectroscopyAcqReorderedPhaseRows",
                "SharedFunctionalGroupsSequence", 0
            );
            numPix[2] = hdr->GetIntSequenceItemElement(
                "MRSpectroscopyFOVGeometrySequence", 0, "SVK_SpectroscopyAcqReorderedOutOfPlanePhaseSteps",
                "SharedFunctionalGroupsSequence", 0
            );
        } else {
            numPix[0] =  hdr->GetIntValue("Columns");
            numPix[1] =  hdr->GetIntValue("Rows");
            numPix[2] =  hdr->GetNumberOfSlices();
        }
    } else if ( centerType.compare( "current" ) == 0 ) {
        numPix[0] =  hdr->GetIntValue("Columns");
        numPix[1] =  hdr->GetIntValue("Rows");
        numPix[2] =  hdr->GetNumberOfSlices();
    }

    float fov[3]; 
    for(int i = 0; i < 3; i++) {
        fov[i] = (numPix[i] - 1) * pixelSpacing[i];  
    }

    for (int i = 0; i < 3; i++ ){
        center[i] = slicePositionFirst[i];
        for(int j = 0; j < 3; j++ ){
            center[i] += ( dcos[j][i] * fov[j]/2. );
        }
    }

    if (this->GetDebug()) {
        cout << "space: " << pixelSpacing[0] << " " << pixelSpacing[1] << " " << pixelSpacing[2] << endl;
        cout << "center: " << center[0] << " " << center[1] << " " << center[2] << endl;
    }
}


/*!
 *   
 */
string svkDdfVolumeWriter::GetDDFPatientName(string PatientName)
{

    //  Remove DICOM delimiters:
    for (int i = 0; i < PatientName.size(); i++) {
        if ( PatientName[i] == '^') {
            PatientName[i] = ' ';
        }
    }

    //  Remove multiple spaces:
    size_t pos; 
    while ( (pos = PatientName.find("  ")) != string::npos) {
        PatientName.erase(pos, 1);     
    }

    return PatientName;
}


/*!
 *  Converts the DICOM dimension domain type to a ddf dimension type string:
 */
string svkDdfVolumeWriter::GetDimensionDomain( string dimensionDomainString )
{
    string domain;
    if ( dimensionDomainString.compare("TIME") == 0 )  {
        domain.assign("time");
    } else if ( dimensionDomainString.compare("FREQUENCY") == 0 )  {
        domain.assign("frequency");
    } else if ( dimensionDomainString.compare("SPACE") == 0 )  {
        domain.assign("space");
    } else if ( dimensionDomainString.compare("KSPACE") == 0 )  {
        domain.assign("k-space");
    }
    return domain;
}


/*!
 *  Use descriptive file names to indicate dimension indices. 
 */
void svkDdfVolumeWriter::UseDescriptiveFileNames() 
{
    this->useDescriptiveFileNames = true; 
}


/*!
 *  Takes a file root name and appends the necessary numerical extension to 
 *  indicate time_pt or coil number for multi-file output of dataset, e.g. 
 *  each coil written to a separate ddf/cmplx file pair. 
 */
string svkDdfVolumeWriter::GetFileRootName(string fileRoot, svkDcmHeader::DimensionVector* dimensionVector, int frame ) 
{

    svkDcmHeader::DimensionVector loopVector = *dimensionVector;     
    svkDcmHeader::GetDimensionVectorIndexFromFrame(dimensionVector, &loopVector, frame);

    //  See if any non time dimension has length > 1: 
    string dimLabel = ""; 
    string extraDimLabel = ""; 
    int numDimsToRepresent = 0; 
    int implicitDimensionIndex = 1; 


    if ( this->useDescriptiveFileNames == true ) {
        for ( int i = 3; i < dimensionVector->size(); i++) {
            int dimSize = svkDcmHeader::GetDimensionVectorValue(dimensionVector, i); 
            if ( dimSize > 0 ) {
                numDimsToRepresent++; 
                if ( numDimsToRepresent == 1) {
                    implicitDimensionIndex = svkDcmHeader::GetDimensionVectorValue(&loopVector, i) + 1; 
                    dimLabel.assign( svkTypeUtils::IntToString( implicitDimensionIndex ) ); 
                }

                if ( numDimsToRepresent > 1 ) {
                    extraDimLabel.append("_"); 
                    string type = this->GetImageDataInput(0)->GetDcmHeader()->GetDimensionIndexLabel(i-2);
                    //cout << "TYPE: " << type << endl;
                    extraDimLabel.append(type); 
                    int dimValue = svkDcmHeader::GetDimensionVectorValue(&loopVector, i) + 1; 
                    extraDimLabel.append( svkTypeUtils::IntToString(dimValue) ); 
                }
            }
        }
        //  construct file number.  By default this reflects the coil number, 
        //  but dependeing on numTimePtsPerFile, may also reflect time point.
        if ( numDimsToRepresent >=1 ) {
            dimLabel.append(extraDimLabel); 
            fileRoot.assign( fileRoot + "_" + dimLabel ) ;
        } else {
            fileRoot.assign( fileRoot );  
        }

    } else if ( this->useDescriptiveFileNames == false ) {

        for ( int i = 3; i < dimensionVector->size(); i++) {
            int dimSize = svkDcmHeader::GetDimensionVectorValue(dimensionVector, i); 
            if ( dimSize > 0 ) {
                numDimsToRepresent++; 
                if ( numDimsToRepresent == 1) {
                    implicitDimensionIndex = svkDcmHeader::GetDimensionVectorValue(&loopVector, i) + 1; 
                    dimLabel.assign( svkTypeUtils::IntToString( implicitDimensionIndex ) ); 
                }
            }
        }
   
        //  construct file number.  By default this reflects the coil number,
        //  but dependeing on numTimePtsPerFile, may also reflect time point.
        int fileNum = -1; 
        int numCoils   = svkDcmHeader::GetDimensionVectorValue(dimensionVector, svkDcmHeader::CHANNEL_INDEX) + 1; 
        int numTimePts = svkDcmHeader::GetDimensionVectorValue(dimensionVector, svkDcmHeader::TIME_INDEX) + 1; 
        int coilNum    = svkDcmHeader::GetDimensionVectorValue(&loopVector, svkDcmHeader::CHANNEL_INDEX); 
        int timePt     = svkDcmHeader::GetDimensionVectorValue(&loopVector, svkDcmHeader::TIME_INDEX); 
        int acq        = svkDcmHeader::GetDimensionVectorValue(&loopVector, svkDcmHeader::EPSI_ACQ_INDEX); 
        //cout << "coils: " << numCoils << " tp: " << numTimePts << " cindex " << coilNum << " tindex: "  << timePt << endl;
        if ( numCoils > 1 &&  this->AllTimePointsInEachFile() ) {
            fileNum = coilNum; 
            //cout << "c1: " << fileNum << endl;
        } else if ( 
            (numCoils > 1 || numTimePts > 1 ) 
            && ! this->AllTimePointsInEachFile() ) 
        {
            fileNum = (coilNum * numTimePts) + timePt; 
            //cout << "c2: " << fileNum << endl;
        } 

        if ( fileNum >= 0 ) {
           ostringstream oss;
           //  Add 1 to the file number so indexing doesn't start at 0.
           oss << fileNum + 1;
           fileRoot.assign( fileRoot + "_" + oss.str() ) ;
        } else if ( dimLabel.size() > 0 ) {
           fileRoot.assign( fileRoot + "_" + dimLabel ) ;
        } else {
           fileRoot.assign( fileRoot ); 
        }
        //cout << "ddf FN: " << fileNum << " -> " << fileRoot << endl;

           ostringstream acqoss;
           //  Add 1 to the file number so indexing doesn't start at 0.
           acqoss << acq;
            
           //fileRoot.assign( fileRoot + "_" + acqoss.str() ) ;

    }



    return fileRoot;
}


/*!
 *  Currently only 1 time pt or all time points is supported. 
 */
void svkDdfVolumeWriter::SetOneTimePointsPerFile()
{
    this->SetNumberTimePointsPerFile(1) ; 
}


/*!
 *  Currently only 1 time pt or all time points is supported. 
 */
void svkDdfVolumeWriter::SetNumberTimePointsPerFile(int numTimePts)
{
    this->numTimePtsPerFile = numTimePts; 
}


/*!
 *
 */
int svkDdfVolumeWriter::GetNumberTimePointsPerFile()
{
    return this->numTimePtsPerFile;
}


/*!
 *  Method to check weather all time points are written to each file. 
 *  If only one time point then return true. 
 */
bool svkDdfVolumeWriter::AllTimePointsInEachFile()
{
    int numTimePts = this->GetImageDataInput(0)->GetDcmHeader()->GetNumberOfTimePoints();
    if ( this->GetNumberTimePointsPerFile() == numTimePts ) {
        return true;  
    } else {
        return false; 
    }
}


/*!
 *
 */
int svkDdfVolumeWriter::FillInputPortInformation( int vtkNotUsed(port), vtkInformation* info )
{
    info->Set(svkDdfVolumeWriter::INPUT_REQUIRED_DATA_TYPE(), "svkMrsImageData");
    return 1;
}


/*!
 *
 */
vtkDataObject* svkDdfVolumeWriter::GetInput(int port)
{
    return this->GetExecutive()->GetInputData(port, 0);
}


/*!
 *
 */
svkImageData* svkDdfVolumeWriter::GetImageDataInput(int port)
{
    return svkImageData::SafeDownCast( this->GetInput(port) );
}


