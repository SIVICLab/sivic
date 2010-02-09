/*
 *  Copyright © 2009-2010 The Regents of the University of California.
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


using namespace svk;


vtkCxxRevisionMacro(svkDdfVolumeWriter, "$Rev$");
vtkStandardNewMacro(svkDdfVolumeWriter);


/*!
 *
 */
svkDdfVolumeWriter::svkDdfVolumeWriter()
{

#if VTK_DEBUG_ON
    this->DebugOn();
#endif

    vtkDebugMacro( << this->GetClassName() << "::" << this->GetClassName() << "()" );

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

    this->WriteData();
    this->WriteHeader();

    if (this->ErrorCode == vtkErrorCode::OutOfDiskSpaceError) {
        vtkErrorMacro("Ran out of disk space; deleting file(s) already written");
        this->DeleteFiles();
        return;
    }

    delete [] this->InternalFileName;
    this->InternalFileName = NULL;
}


/*!
 *  Write the image data pixels to the DDF data file (.cmplx)
 */
void svkDdfVolumeWriter::WriteData()
{
    vtkDebugMacro( << this->GetClassName() << "::WriteData()" );

    string extension = ".cmplx"; 

    string fileRoot = string(this->InternalFileName).substr( 0, string(this->InternalFileName).rfind(".") );

    ofstream cmplxOut( ( fileRoot + extension ).c_str(), ios::binary);
    if( !cmplxOut ) {
        throw runtime_error("Cannot open .cmplx file for writing");
    }

    svkDcmHeader* hdr = this->GetImageDataInput(0)->GetDcmHeader(); 
    int dataWordSize = 4; 
    int cols     = hdr->GetIntValue( "Columns" );
    int rows     = hdr->GetIntValue( "Rows" );
    int slices   = hdr->GetNumberOfSlices();  
    int numCoils = hdr->GetNumberOfCoils();
    int numTimePts = hdr->GetNumberOfTimePoints();  
    int specPts  = hdr->GetIntValue( "DataPointColumns" );
    string representation = hdr->GetStringValue( "DataRepresentation" );

    int numComponents = 1;
    if (representation.compare("COMPLEX") == 0) {
        numComponents = 2;
    }

    vtkCellData* cellData = this->GetImageDataInput(0)->GetCellData();

    int dataLengthPerCoil = cols * rows * slices * specPts * numComponents;
    int dataLength = dataLengthPerCoil * numCoils;
    int coilOffset = cols * rows * slices ;

    // write out one coil per ddf file
    float* specData = new float [ dataLengthPerCoil ];

    vtkFloatArray* fa;
    float* dataTuple = new float[numComponents];

    for (int coilNum = 0; coilNum < numCoils; coilNum ++) {
        for (int timePt = 0; timePt < numTimePts; timePt++) {
            for (int z = 0; z < slices; z++) {
                for (int y = 0; y < rows; y++) {
                    for (int x = 0; x < cols; x++) {

                        int offset = (cols * rows * z) + (cols * y) + x + (coilNum * coilOffset);
                        fa =  vtkFloatArray::SafeDownCast( cellData->GetArray( offset ) );

                        for (int i = 0; i < specPts; i++) {

                            fa->GetTupleValue(i, dataTuple);

                            for (int j = 0; j < numComponents; j++) {
                                specData[ (offset * specPts * numComponents) + (i * numComponents) + j ] = dataTuple[j];
                            }
                        }
                    }
                }
            }
        }
    }
    delete [] dataTuple;

    //  cmplx files are by definition big endian:
#if defined (linux) || defined (Darwin)
    svkByteSwap::SwapBufferEndianness( (float*)specData, dataLengthPerCoil );
#endif

    cmplxOut.write( (char *)specData, dataLengthPerCoil * dataWordSize);

}


/*!
 *  Write the DDF header.

 */
void svkDdfVolumeWriter::WriteHeader()
{

    //write the ddf file
    string fileRoot = string(this->InternalFileName).substr( 0, string(this->InternalFileName).rfind(".") );

    ofstream out( (  fileRoot + string(".ddf") ).c_str() );
    if(!out) {
        throw runtime_error("Cannot open .ddf file for writing");
    }

    svkDcmHeader* hdr = this->GetImageDataInput(0)->GetDcmHeader(); 

    out << "DATA DESCRIPTOR FILE" << endl;
    out << "version: 6.1" << endl;
    out << "object type: MR Spectroscopy" << endl;
    out << "patient id: " << setw(19) << left << hdr->GetStringValue( "PatientID" ) << endl;
    out << "patient name: " << setw(63) << left << this->GetDDFPatientsName( hdr->GetStringValue( "PatientsName" ) ) << endl;
    out << "patient code: " << endl;
    out << "date of birth: " << hdr->GetStringValue( "PatientsBirthDate" ) <<  endl;
    out << "sex: " << hdr->GetStringValue( "PatientsSex" ) <<  endl;
    out << "study id: " << hdr->GetStringValue( "StudyID" ) <<  endl;
    out << "study code: " << "" <<  endl;

    string date = hdr->GetStringValue( "StudyDate" );
    if ( date.length() == 0 ) {
        date.assign("        ");
    }

    out << "study date: " << 
        date[4] << date[5] << "/" << date[6] << date[7] << "/" << date[0] << date[1] << date[2] << date[3] << endl;


    out << "accession number: " << hdr->GetStringValue( "AccessionNumber" ) <<  endl;
    out << "root name: " << setw(7) << fileRoot <<  endl;
    out << "series number: " << hdr->GetStringValue( "SeriesNumber" ) <<  endl;
    out << "series description: " << hdr->GetStringValue( "SeriesDescription" ) <<  endl;
    out << "comment: " << " " << endl;

    out << "patient entry: ";
    string position_string = hdr->GetStringValue( "PatientPosition" );
    if ( position_string.substr(0,2) == string( "HF" ) ){
        out << "head first";
    } else if ( position_string.substr(0,2) == string( "FF" ) ) {
        out << "feet first";
    } else {
        out << "UNKNOWN";
    }
    out << endl;

    out << "patient position: ";
    if ( position_string.substr(2) == string( "S" ) ) {
        out << "supine" << endl;
    } else if ( position_string.substr(2) == string( "P" ) ) {
        out << "prone" << endl;
    } else if ( position_string.substr(2) == string( "DL" ) ) {
        out << "decubitus left" << endl;
    } else if ( position_string.substr(2) == string( "DR" ) ) {
        out << "decubitus right" << endl;
    } else {
        out << "UNKNOWN" << endl;
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

    out << "orientation: " << orientationString << endl;
    out << "data type: floating point" << endl;
   
    int numComponents = 0;      
    if ( hdr->GetStringValue("DataRepresentation").compare("COMPLEX") == 0 ) {
        numComponents = 2;     
    }
    out << "number of components: " << numComponents << endl; 

    out << "source description: " << endl;

    int numDims = 4; 
    if ( hdr->GetNumberOfTimePoints()  > 1 ) { 
        numDims = 5; 
    }
    out << "number of dimensions: " << numDims << endl; 

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
    out << "dimension 1: type: " << specDomain << " npoints: " << setw(5) << left << hdr->GetIntValue( "DataPointColumns" ) << endl;
    out << "dimension 2: type: " << spatialDomains[0] << " npoints: " << numVoxels[0] << 
        " pixel spacing(mm): " << fixed << left << setw(9) << setprecision(6) << voxelSpacing[0] << endl;
    out << "dimension 3: type: " << spatialDomains[1] << " npoints: " << numVoxels[1] << 
        " pixel spacing(mm): " << fixed << left << setw(9) << setprecision(6) << voxelSpacing[1] << endl;
    out << "dimension 4: type: " << spatialDomains[2] << " npoints: " << numVoxels[2] << 
        " pixel spacing(mm): " << fixed << left << setw(9) << setprecision(6) << voxelSpacing[2] << endl;

    if ( numDims == 5 ) {    
        out << "dimension 5: type: time" << endl ; 
    }

    float center[3];
    this->GetDDFCenter( center );
    out << "center(lps, mm): " << fixed << right << setw(14) << setprecision(5) << center[0]
        << setw(14) << center[1] << setw(14) << center[2] << endl;

    double positionFirst[3]; 
    hdr->GetOrigin(positionFirst, 0);
    out << "toplc(lps, mm):  " << fixed << right << setw(14) << setprecision(5)
        << positionFirst[0]
        << setw(14) << positionFirst[1]
        << setw(14) << positionFirst[2] <<endl;

    double dcos[3][3];
    this->GetImageDataInput(0)->GetDcos(dcos);
    out << "dcos0: " << fixed << setw(14) << setprecision(5) << dcos[0][0] << setw(14) << dcos[0][1]
        << setw(14) << dcos[0][2] << endl;
    out << "dcos1: " << fixed << setw(14) << setprecision(5) << dcos[1][0]
        << setw(14) << dcos[1][1] << setw(14) << dcos[1][2] << endl;
    out << "dcos2: " << fixed << setw(14) << setprecision(5) << dcos[2][0] << setw(14)
        << dcos[2][1] << setw(14) << dcos[2][2] << endl;

    out << "===================================================" << endl; 
    out << "MR Parameters" << endl; 

    string coilName = hdr->GetStringSequenceItemElement(
        "MRReceiveCoilSequence",
        0,
        "ReceiveCoilName",
        "SharedFunctionalGroupsSequence",
        0
    );
    out << "coil name: " << fixed << left << setw(15) << coilName << endl; 

    out << "slice gap(mm): " << endl;

    float TE = hdr->GetFloatSequenceItemElement(
        "MREchoSequence",
        0,
        "EffectiveEchoTime",
        "SharedFunctionalGroupsSequence",
        0
    );
    out << "echo time(ms): " << fixed << setprecision(6) << TE << endl; 

    float TR = hdr->GetFloatSequenceItemElement(
        "MRTimingAndRelatedParametersSequence",
        0,
        "RepetitionTime",
        "SharedFunctionalGroupsSequence",
        0
    );
    out << "repetition time(ms): " << fixed << setprecision(6) << TR << endl;

    out << "inversion time(ms): " << fixed << setprecision(6) << endl; 
    
    float flipAngle =  hdr->GetFloatSequenceItemElement(
        "MRTimingAndRelatedParametersSequence",
        0,
        "FlipAngle",
        "SharedFunctionalGroupsSequence",
        0
    );
    out << "flip angle: " << fixed << setprecision(6) << flipAngle << endl;

    out << "pulse sequence name: " << fixed << setw(15) << hdr->GetStringValue( "PulseSequenceName" ) << endl;
    out << "transmitter frequency(MHz): " << hdr->GetFloatValue( "TransmitterFrequency" ) << endl; 
    out << "isotope: " << hdr->GetStringValue( "ResonantNucleus" ) << endl;
    out << "field strength(T): " <<  hdr->GetFloatValue( "MagneticFieldStrength" ) << endl; 
    int numSatBands = hdr->GetNumberOfItemsInSequence("MRSpatialSaturationSequence");
    out << "number of sat bands: " << numSatBands << endl;
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

        out << "sat band " << satBand + 1 << " thickness(mm): " << fixed << setprecision(6) << thickness << endl; 
        out << "sat band " << satBand + 1 << " orientation: " << fixed << right << setw(21) << setprecision(5) << norm0 
                                                              << fixed << right << setw(14) << setprecision(5) << norm1 
                                                              << fixed << right << setw(14) << setprecision(5) << norm2 
                                                              << endl;  

        out << "sat band " << satBand + 1 << " position(lps, mm): " << fixed << right << setw(15) << setprecision(5) << pos0 
                                                                    << fixed << right << setw(14) << setprecision(5) << pos1 
                                                                    << fixed << right << setw(14) << setprecision(5) << pos2 
                                                                    << endl;  

    }
                                       
    out << "===================================================" << endl; 
    out << "Spectroscopy Parameters" << endl; 

    out << "localization type: " << hdr->GetStringValue( "VolumeLocalizationTechnique" ) << endl;
    out << "center frequency(MHz): " << fixed << setprecision(6) << hdr->GetFloatValue( "TransmitterFrequency" ) << endl;
    out << "ppm reference: " << fixed << setprecision(6) << hdr->GetFloatValue( "ChemicalShiftReference" ) << endl;
    out << "sweepwidth(Hz): " << fixed << setprecision(6) << hdr->GetFloatValue( "SpectralWidth" ) << endl;
    out << "dwelltime(ms): " << fixed << setprecision(6) << 1000/( hdr->GetFloatValue( "SpectralWidth" ) ) << endl;
    out << "frequency offset(Hz): " << fixed << setprecision(6) << hdr->GetFloatValue( "SVK_FrequencyOffset" ) << endl;
    string onH20 = "yes"; 
    if (hdr->GetFloatValue( "SVK_FrequencyOffset" ) != 0 ) {
        onH20.assign( "no" ); 
    }
    out << "centered on water: " << onH20 << endl;
    out << "suppression technique: " << endl;
    out << "residual water:" << endl;
    out << "number of acquisitions: " << endl;
    out << "chop: " << endl;
    out << "even symmetry: " << endl;
    out << "data reordered: " << endl;

    float acqTLC[3];
    for (int i = 0; i < 3; i++) {
        acqTLC[i] = hdr->GetFloatSequenceItemElement(
            "MRSpectroscopyFOVGeometrySequence", 0, "SVK_SpectroscopyAcquisitionTLC", 
            "SharedFunctionalGroupsSequence", 0, i
        );
    }

    out << "acq. toplc(lps, mm):  " << fixed << right << setw(13) << setprecision(5)
        << acqTLC[0]
        << setw(14) << acqTLC[1]
        << setw(14) << acqTLC[2] << endl;

    float acqSpacing[3];
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

    out << "acq. spacing(mm):  " << fixed << right << setw(16) << setprecision(5)
        << acqSpacing[0]
        << setw(14) << acqSpacing[1]
        << setw(14) << acqSpacing[2] << endl;

    out << "acq. number of data points: " << 
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
    out << "acq. number of points: " << acqPts1 << " " << acqPts2 << " " << acqPts3 << endl;

    float acqDcos[9]; 
    for (int i = 0; i < 9; i++) {
        acqDcos[i] = hdr->GetFloatSequenceItemElement(
            "MRSpectroscopyFOVGeometrySequence", 0, "SVK_SpectroscopyAcquisitionOrientation", 
            "SharedFunctionalGroupsSequence", 0, i
        );
    }

    out << "acq. dcos1: " << fixed << setw(14) << setprecision(5) << acqDcos[0] << setw(14) << acqDcos[1]
        << setw(14) << acqDcos[2] << endl;
    out << "acq. dcos2: " << fixed << setw(14) << setprecision(5) << acqDcos[3]
        << setw(14) << acqDcos[4] << setw(14) << acqDcos[5] << endl;
    out << "acq. dcos3: " << fixed << setw(14) << setprecision(5) << acqDcos[6] << setw(14)
        << acqDcos[7] << setw(14) << acqDcos[8] << endl;

    float selBoxSize[3]; 
    float selBoxCenter[3]; 

    for (int i = 0; i < 3; i++) {

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
    out << "selection center(lps, mm): " << fixed << setw(14) << setprecision(5) << selBoxCenter[0] 
                                         << fixed << setw(14) << setprecision(5) << selBoxCenter[1]  
                                         << fixed << setw(14) << setprecision(5) << selBoxCenter[2] 
                                         << endl;  

    out << "selection size(mm): " << fixed << setw(21) << setprecision(5) << selBoxSize[0] 
                                  << fixed << setw(14) << setprecision(5) << selBoxSize[1]  
                                  << fixed << setw(14) << setprecision(5) << selBoxSize[2] 
                                  << endl;  


    float selBoxOrientation[3][3]; 
    for (int i = 0; i < 3; i++) {
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

    out << "selection dcos1: " << fixed << setw(14) << setprecision(5) << selBoxOrientation[0][0] 
                               << fixed << setw(14) << setprecision(5) << selBoxOrientation[0][1]  
                               << fixed << setw(14) << setprecision(5) << selBoxOrientation[0][2] 
                               << endl;  

    out << "selection dcos2: " << fixed << setw(14) << setprecision(5) << selBoxOrientation[1][0] 
                               << fixed << setw(14) << setprecision(5) << selBoxOrientation[1][1]  
                               << fixed << setw(14) << setprecision(5) << selBoxOrientation[1][2] 
                               << endl;  

    out << "selection dcos3: " << fixed << setw(14) << setprecision(5) << selBoxOrientation[2][0] 
                               << fixed << setw(14) << setprecision(5) << selBoxOrientation[2][1]  
                               << fixed << setw(14) << setprecision(5) << selBoxOrientation[2][2] 
                               << endl;  

    float reorderedTLC[3];
    for (int i = 0; i < 3; i++) {
        reorderedTLC[i] = hdr->GetFloatSequenceItemElement(
            "MRSpectroscopyFOVGeometrySequence", 0, "SVK_SpectroscopyAcqReorderedTLC",
            "SharedFunctionalGroupsSequence", 0, i
        );
    }

    out << "reordered toplc(lps, mm):  " << fixed << right << setw(14) << setprecision(5)
        << reorderedTLC[0]
        << setw(14) << reorderedTLC[1]
        << setw(14) << reorderedTLC[2] << endl;

    float reorderedCenter[3]; 
    this->GetDDFCenter( reorderedCenter, "reordered" );
    out << "reordered center(lps, mm): " << fixed << setw(14) << setprecision(5) << reorderedCenter[0] 
                                         << fixed << setw(14) << setprecision(5) << reorderedCenter[1]  
                                         << fixed << setw(14) << setprecision(5) << reorderedCenter[2] 
                                         << endl;  

    float reorderedSpacing[3];
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

    out << "reordered spacing(mm):  " << fixed << right << setw(17) << setprecision(5)
        << reorderedSpacing[0]
        << setw(14) << reorderedSpacing[1]
        << setw(14) << reorderedSpacing[2] << endl;

    int reorderedPts1 = hdr->GetIntSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence", 0, "SVK_SpectroscopyAcqReorderedPhaseColumns",
        "SharedFunctionalGroupsSequence", 0
    );
    int reorderedPts2 = hdr->GetIntSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence", 0, "SVK_SpectroscopyAcqReorderedPhaseRows",
        "SharedFunctionalGroupsSequence", 0
    );
    int reorderedPts3 = hdr->GetIntSequenceItemElement(
        "MRSpectroscopyFOVGeometrySequence", 0, "SVK_SpectroscopyAcqReorderedOutOfPlanePhaseSteps",
        "SharedFunctionalGroupsSequence", 0
    );
    out << "reordered number of points: " << reorderedPts1 << " " << reorderedPts2 << " " << reorderedPts3 << endl;

    float reorderedDcos[9];
    for (int i = 0; i < 9; i++) {
        reorderedDcos[i] = hdr->GetFloatSequenceItemElement(
            "MRSpectroscopyFOVGeometrySequence", 0, "SVK_SpectroscopyAcqReorderedOrientation",
            "SharedFunctionalGroupsSequence", 0, i
        );
    }

    out << "reordered dcos1: " << fixed << setw(14) << setprecision(5) << reorderedDcos[0] << setw(14) << reorderedDcos[1]
        << setw(14) << reorderedDcos[2] << endl;
    out << "reordered dcos2: " << fixed << setw(14) << setprecision(5) << reorderedDcos[3]
        << setw(14) << reorderedDcos[4] << setw(14) << reorderedDcos[5] << endl;
    out << "reordered dcos3: " << fixed << setw(14) << setprecision(5) << reorderedDcos[6] << setw(14)
        << reorderedDcos[7] << setw(14) << reorderedDcos[8] << endl;

    out << "===================================================" << endl; 

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
string svkDdfVolumeWriter::GetDDFPatientsName(string patientsName)
{

    //  Remove DICOM delimiters:
    for (int i = 0; i < patientsName.size(); i++) {
        if ( patientsName[i] == '^') {
            patientsName[i] = ' ';
        }
    }

    //  Remove multiple spaces:
    size_t pos; 
    while ( (pos = patientsName.find("  ")) != string::npos) {
        patientsName.erase(pos, 1);     
    }

    return patientsName;
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


