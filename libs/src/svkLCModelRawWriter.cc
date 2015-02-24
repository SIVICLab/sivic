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



#include <svkLCModelRawWriter.h>
#include <svk4DImageData.h>
#include <svkDcmHeader.h>
#include <vtkErrorCode.h>
#include <vtkCellData.h>
#include <vtkExecutive.h>
#include <vtkFloatArray.h>
#include <vtkByteSwap.h>
#include <svkUtils.h>
#include "svkTypeUtils.h"


using namespace svk;


vtkCxxRevisionMacro(svkLCModelRawWriter, "$Rev$");
vtkStandardNewMacro(svkLCModelRawWriter);


/*!
 *
 */
svkLCModelRawWriter::svkLCModelRawWriter()
{

#if VTK_DEBUG_ON
    this->DebugOn();
#endif

    vtkDebugMacro( << this->GetClassName() << "::" << this->GetClassName() << "()" );
}


/*!
 *
 */
svkLCModelRawWriter::~svkLCModelRawWriter()
{
    vtkDebugMacro( << this->GetClassName() << "::~" << this->GetClassName() << "()" );
}


/*!
 *  Write the LCModel Raw data file.  Should support multiple coils (files) and multi-time point data 
 */
void svkLCModelRawWriter::Write()
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
void svkLCModelRawWriter::SetProvenance()
{
    this->GetImageDataInput(0)->GetProvenance()->AddAlgorithm( this->GetClassName() ); 

}


/*!
 *  Write the image data pixels and header to the LCModel Raw data file (.raw) and control file
 */
void svkLCModelRawWriter::WriteFiles()
{
    vtkDebugMacro( << this->GetClassName() << "::WriteData()" );

    string rawExtension     = ".raw"; 
    string controlExtension = ".control"; 

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

    int numberTimePointsPerFile = 1; 
    int dataLengthPerFile = cols * rows * slices * specPts * numComponents * numberTimePointsPerFile; 
    int coilOffset = cols * rows * slices * numTimePts;     //number of spectra per coil
    int timePtOffset = cols * rows * slices; 

    // write out one coil per ddf file
    float* specData = new float [ dataLengthPerFile ];

    string fileName;

    svkDcmHeader::DimensionVector origDimensionVector = hdr->GetDimensionIndexVector(); 
    svkDcmHeader::DimensionVector dimensionVector = origDimensionVector; 
    svkDcmHeader::DimensionVector loopVector = dimensionVector; 


    //  In this case we only want to loop over dimensions other than the time dimension, so set 
    //  time to a single value in this vector:

    int numFrames = hdr->GetNumberOfFrames(&dimensionVector); 
    for (int frame = 0; frame < numFrames; frame++) {

        fileName = this->GetFileRootName(fileRoot, &dimensionVector, frame);
        //cout << "RAW FILE NAMEb: " << frame << " = " << fileName << endl;

        ofstream cmplxOut( ( fileName + rawExtension ).c_str(), ios::binary);
        ofstream rawOut(   ( fileName + rawExtension ).c_str() );
        if( !cmplxOut || !rawOut ) {
            throw runtime_error("Cannot open .ddf or .cmplx file for writing");
        }

        //  Initialize meta data in .raw file 
        this->InitRawHeader( &rawOut, fileName);

        //  Append spec data to .raw file 
        hdr->GetDimensionVectorIndexFromFrame(&dimensionVector, &loopVector, frame);
        this->InitSpecData(&rawOut, specData, &dimensionVector, &loopVector); 


        //  cmplx files are by definition big endian:
        #ifndef VTK_WORDS_BIGENDIAN
            vtkByteSwap::SwapVoidRange((void*)specData, dataLengthPerFile, sizeof(float));
        #endif

        //cmplxOut.write( reinterpret_cast<char *>(specData), dataLengthPerFile * dataWordSize);
        //cmplxOut.close();
        rawOut.close(); 
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
void svkLCModelRawWriter::InitSpecData(ofstream* out, float* specData, svkDcmHeader::DimensionVector* dimensionVector, svkDcmHeader::DimensionVector* indexVector) 
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

                int offsetOut = ( cols * rows * z ) + ( cols * y ) + x; 
                
                int cellID = svkDcmHeader::GetCellIDFromDimensionVectorIndex(dimensionVector, indexVector); 
                fa =  vtkFloatArray::SafeDownCast( cellData->GetArray( cellID) );

                for (int i = 0; i < specPts; i++) {

                    fa->GetTupleValue(i, dataTuple);

                    *out << " " ;
                    for (int j = 0; j < numComponents; j++) {
                        specData[ (offsetOut * specPts * numComponents) + (i * numComponents) + j ] = dataTuple[j];
                        *out << fixed << setw(18) << setprecision(3) <<  dataTuple[j];
                    }
                    *out << endl;
                }
            }
        }
    }

    delete[] dataTuple; 

}


/*!
 *  initializes the ofstream header content for the LCModel Raw header.
 *
 *      $SEQPAR
 *      ECHOT=30
 *      HZPPM=298.064
 *      SEQ=SE
 *      $END
 *      $NMID
 *      ID='t8002_comb_cor_sum_inv2'
 *      FMTDAT='(f16.3)'
 *      VOLUME=1.000
 *      TRAMP=67.471
 *      $END
 */
void svkLCModelRawWriter::InitRawHeader(ofstream* out, string fileName)
{

    svkDcmHeader* hdr = this->GetImageDataInput(0)->GetDcmHeader(); 

    //  ===========================================================
    //  ECHOT (real) (“echo time”) the echo time (in ms) used for this data.
    //  ===========================================================
    float TE = hdr->GetFloatSequenceItemElement(
        "MREchoSequence",
        0,
        "EffectiveEchoTime",
        "SharedFunctionalGroupsSequence",
        0
    );

    //  ===========================================================
    //  HZPPPM (real) (“Hz per ppm”) the field strength, in terms of the proton resonance
    //  frequency in MHz, i.e., HZPPPM = 42.58B0, with B0 in Tesla. You should
    //  input it with an accuracy of at least four significant figures.
    //      transmitter freq  B0 * gyromag
    //  ===========================================================
    float transmitterFrequency = hdr->GetFloatValue( "TransmitterFrequency" );
    
    //  ===========================================================
    //  SEQ: 
    //  LCModel docs say this should be either PRESS or STEAM, but Yan uses SEQ=SE
    //  ===========================================================
    string sequenceName = hdr->GetStringValue( "PulseSequenceName" ); 
    sequenceName = "SE"; 

    //  ===========================================================
    //  ID (character*20) a string that you can use to identify the data. It appears
    //  in the so-called Detailed Output and in the plot of the data with the
    //  program PlotRaw. It is useful for documentation, but it is optional; if you
    //  leave it out of the Namelist input, then a blank ID is output.
    //  ===========================================================
    string id = fileName;  
    
    //  ===========================================================
    //  FMTDAT (character*80) the Fortran format specification for your raw timedomain
    //  data, which must immediately follow Namelist NMID. (See also
    //  Sec 5.2.3 below.) This has no default; it must be input.
    //  ===========================================================
    string fmtDat = "'(2f18.3)'"; 
    //string fmtDat = "'(f16.3)'"; 

    //  ===========================================================
    //  VOLUME: 
    //  (real) the voxel size (always in the same units, e.g., mL).
    //  Default: VOLUME = 1.0
    //  ===========================================================
    double voxelSpacing[3]; 
    hdr->GetPixelSpacing( voxelSpacing );  
    double voxelVolume = voxelSpacing[0] * voxelSpacing[1] * voxelSpacing[2] / (10 * 10 * 10); 

    //  ===========================================================
    //  TRAMP (real) The data are multiplied by the factor TRAMP/VOLUME to scale the
    //  data consistently with the Basis Set, as discussed in Sec 10.1.1. VOLUME
    //  and TRAMP do not affect the concentration ratios; they only need to be
    //  input for absolute concentrations. Default: TRAMP = 1.0.
    //  ===========================================================
    float tramp = 67.471;   // value Yan is using ?? 

    *out << " $SEQPAR"<<  endl;
    *out << " ECHOT=" << fixed << setprecision(0) << TE <<  endl;
    *out << " HZPPM=" << setprecision(3) << transmitterFrequency <<  endl;
    *out << " SEQ=" << sequenceName <<  endl;
    *out << " $END" << endl;

    *out << " $NMID"<<  endl;
    *out << " ID=" << id <<  endl;
    *out << " FMTDAT=" << fmtDat << endl;
    *out << " VOLUME=" << fixed << setprecision(3) << voxelVolume <<  endl;
    *out << " TRAMP=" << setprecision(3) << tramp <<  endl;
    *out << " $END" <<  endl;

    return; 
}


/*!
 *  Takes a file root name and appends the necessary numerical extension to 
 *  indicate time_pt or coil number for multi-file output of dataset, e.g. 
 *  each coil written to a separate ddf/cmplx file pair. 
 */
string svkLCModelRawWriter::GetFileRootName(string fileRoot, svkDcmHeader::DimensionVector* dimensionVector, int frame ) 
{

    svkDcmHeader::DimensionVector loopVector = *dimensionVector;     
    svkDcmHeader::GetDimensionVectorIndexFromFrame(dimensionVector, &loopVector, frame);

    //  See if any non time dimension has length > 1: 
    string dimLabel = ""; 
    string extraDimLabel = ""; 
    int numDimsToRepresent = 0; 
    int implicitDimensionIndex = 1; 



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

    fileNum = (coilNum * numTimePts) + timePt; 

    if ( fileNum >= 0 ) {
       string coilString;
       ostringstream oss;
       //  Add 1 to the file number so indexing doesn't start at 0.
       oss << fileNum + 1;
       fileRoot.assign( fileRoot + "_" + oss.str() ) ;
    } else if ( dimLabel.size() > 0 ) {
       fileRoot.assign( fileRoot + "_" + dimLabel ) ;
    } else {
       fileRoot.assign( fileRoot ); 
    }

    return fileRoot;
}


/*!
 *
 */
int svkLCModelRawWriter::FillInputPortInformation( int vtkNotUsed(port), vtkInformation* info )
{
    info->Set(svkLCModelRawWriter::INPUT_REQUIRED_DATA_TYPE(), "svkMrsImageData");
    return 1;
}


/*!
 *
 */
vtkDataObject* svkLCModelRawWriter::GetInput(int port)
{
    return this->GetExecutive()->GetInputData(port, 0);
}


/*!
 *
 */
svkImageData* svkLCModelRawWriter::GetImageDataInput(int port)
{
    return svkImageData::SafeDownCast( this->GetInput(port) );
}


