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


#include <svkLCModelCoordReader.h>
#include <svkImageReaderFactory.h>
#include <svkMrsZeroFill.h>
#include <svkString.h>
#include <svkTypeUtils.h>
#include <svkFileUtils.h>
#include <svkSpecPoint.h>
#include </usr/include/vtk/vtkDebugLeaks.h>
#include </usr/include/vtk/vtkByteSwap.h>
#include </usr/include/vtk/vtkSmartPointer.h>
#include </usr/include/vtk/vtkDelimitedTextReader.h>
#include </usr/include/vtk/vtkStringToNumeric.h>
#include </usr/include/vtk/vtkVariantArray.h>

#include <sys/stat.h>


using namespace svk;


//vtkCxxRevisionMacro(svkLCModelCoordReader, "$Rev$");
vtkStandardNewMacro(svkLCModelCoordReader);


/*!
 *  
 */
svkLCModelCoordReader::svkLCModelCoordReader()
{

#if VTK_DEBUG_ON
    this->DebugOn();
    vtkDebugLeaks::ConstructClass("svkLCModelCoordReader");
#endif

    vtkDebugMacro( << this->GetClassName() << "::" << this->GetClassName() << "()" );

    //  3 required input ports: 
    this->SetNumberOfInputPorts(1);

    this->dataStartDelimiter = "fit to the data follow"; 
    this->numCoordFreqPoints = 0; 
}


/*!
 *
 */
svkLCModelCoordReader::~svkLCModelCoordReader()
{
    vtkDebugMacro( << this->GetClassName() << "::~" << this->GetClassName() << "()" );
}



/*!
 *  Check to see if the extension indicates a UCSF IDF file.  If so, try 
 *  to open the file for reading.  If that works, then return a success code. 
 *  Return Values: 1 if can read the file, 0 otherwise.
 */
int svkLCModelCoordReader::CanReadFile(const char* fname)
{

    std::string fileToCheck(fname);

    if( fileToCheck.size() > 4 ) {

        // Also see "vtkImageReader2::GetFileExtensions method" 
        if ( 
            fileToCheck.substr( fileToCheck.size() - 6 ) == ".coord"  
        )  {
            FILE* fp = fopen(fname, "rb");
            if (fp) {
                fclose(fp);
                vtkDebugMacro(<< this->GetClassName() << "::CanReadFile(): It's an LCModel Coord File: " << fileToCheck);
                return 1;
            }
        } else {
            vtkDebugMacro(<< this->GetClassName() << "::CanReadFile(): It's NOT a LCModel Coord File: " << fileToCheck);
            return 0;
        }
    } else {
        vtkDebugMacro(<< this->GetClassName() << "::CanReadFile(): s NOT a valid file: " << fileToCheck);
        return 0;
    }
}



/*!
 *  Side effect of Update() method.  Used to load pixel data and initialize vtkImageData
 *  Called after ExecuteInformation()
 */
void svkLCModelCoordReader::ExecuteDataWithInformation(vtkDataObject* output, vtkInformation* outInfo)
{

    vtkDebugMacro( << this->GetClassName() << "::ExecuteData()" );

    svkImageData* data = svkMrsImageData::SafeDownCast( this->AllocateOutputData(output, outInfo) );

    // Make sure the output data has the correct dimmensions: 
    this->GlobFileNames();
    this->InitNumFreqPointsFromCoordFile(); 
    this->InitPPMRangeFromCoordFile(); 
    this->CheckOutputPtsPerPPM(); 

    //  Create the template data object for fitted spectra to be written to
    this->GetOutput()->DeepCopy( this->GetImageDataInput(0) );     

    //  initialize all output arrays to zero: 
    svkDcmHeader::DimensionVector dimVec = this->GetOutput()->GetDcmHeader()->GetDimensionIndexVector();
    int numCells = svkDcmHeader::GetNumberOfCells( &dimVec );

    double zeroTuple[2];
    zeroTuple[0] = 0; 
    zeroTuple[1] = 0; 
    svkMrsImageData* mrsData = svkMrsImageData::SafeDownCast( this->GetOutput() );
    for (int cellID = 0; cellID < numCells; cellID++ ) {
        vtkFloatArray* spectrumOut  = static_cast<vtkFloatArray*>( mrsData->GetSpectrum( cellID ) );
        for (int i = 0; i < this->numCoordFreqPoints; i++ ) {
            spectrumOut->SetTuple(i, zeroTuple);
        }
    }

    this->ParseCoordFiles(); 

    //  LCModel output is in frequency domain so be sure to set output regardless of the input 
    //  template value. 
    string domain("FREQUENCY");
    this->GetOutput()->GetDcmHeader()->SetValue( "SignalDomainColumns", domain );
}


/*
 *  Init the number of points in the output data from coord file: 
 */
void svkLCModelCoordReader::InitNumFreqPointsFromCoordFile()
{

    //  Loop through files until the num points on ppm scale can be detected.  For 
    //  error voxels, this info isn't written to the file, so it may be necessar to try 
    //  a few. 
    bool foundNumFreqPoints = false; 
    for (int fileIndex = 0; fileIndex < this->GetFileNames()->GetNumberOfValues(); fileIndex++) {

        string coordFileName = this->GetFileNames()->GetValue( fileIndex ); 

        struct stat fs;
        if ( stat( coordFileName.c_str(), &fs) ) {
            vtkErrorMacro("Unable to open file " << coordFileName );
            return;
        }
        vtkDelimitedTextReader* coordReader = vtkDelimitedTextReader::New();
        coordReader->SetFieldDelimiterCharacters("  ");
        coordReader->SetStringDelimiter(' ');
        coordReader->SetMergeConsecutiveDelimiters(true); 
        coordReader->SetHaveHeaders(true);
        coordReader->SetFileName( coordFileName.c_str() ); 
        coordReader->Update();

        vtkTable* table = coordReader->GetOutput();
        int numCols = table->GetNumberOfColumns() ; 
        int numRows = table->GetNumberOfRows() ; 

        //  parse through the rows to find specific pieces of data; 
        int rowID;  
        string rowString = ""; 

        int numFreqPoints;
        for ( rowID = 0; rowID < numRows; rowID++ ) {

            //  Get a row: 
            this->GetDataRow(table, rowID, &rowString);  

            //  First, find the number of frequency points in fitted spectra: 
            string pointsDelimiter = "points on ppm-axis"; 
            size_t foundPtsPos = rowString.find( pointsDelimiter ); 
            if ( foundPtsPos != string::npos) {
                //  num points is the first word on this line: 
                numFreqPoints = table->GetRow(rowID)->GetValue(0).ToInt(); 
                //cout << "ROW: " << rowString << endl;
                cout << "COORD POINTS: " << numFreqPoints << endl;
                this->numCoordFreqPoints = numFreqPoints; 
                foundNumFreqPoints = true;
            }
        }
        coordReader->Delete(); 
        if ( foundNumFreqPoints == true ) {
            break;     
        }
    }
    if ( foundNumFreqPoints == false ) {
        cout << "ERROR(svkLCModelCoordReader): Could not determine the number of points on ppm-axis" << endl;
        exit(1); 
    }
}


/*
 *  Parse PPM range from coord file: 
 */
void svkLCModelCoordReader::InitPPMRangeFromCoordFile()
{
    //  Loop through files until the num points on ppm scale can be detected.  For 
    //  error voxels, this info isn't written to the file, so it may be necessar to try 
    //  a few. 
    bool foundPPMRangeEnd   = false; 
    bool foundPPMRangeStart = false; 
    for (int fileIndex = 0; fileIndex < this->GetFileNames()->GetNumberOfValues(); fileIndex++) {

        string fileName = this->GetFileNames()->GetValue( fileIndex ); 

        ifstream* input = new ifstream();
        input->exceptions( ifstream::eofbit | ifstream::failbit | ifstream::badbit );
        input->open( fileName.c_str(), ifstream::in );

        if ( ! input->is_open() ) {
            throw runtime_error( "Could not open coord file: " + fileName );
        }

        istringstream* iss = new istringstream();
        bool keepReading = true; 
        size_t pos; 
        while ( keepReading ) {

            try {
                svkFileUtils::ReadLine(input, iss); 
                string aLine = iss->str(); 

                if ( aLine.find("PPMEND") != string::npos) { 
                    size_t posVal = aLine.find("="); 
                    if ( posVal != string::npos ) {
                        //cout << "aLine: found: " << aLine << endl;
                        //cout << "sub" <<  aLine.substr(posVal+2) << endl; 
                        this->ppmEnd = svkTypeUtils::StringToFloat( aLine.substr(posVal+2) ); 
                        foundPPMRangeEnd   = true; 
                    }
                } else if ( aLine.find("PPMST") != string::npos) { 
                    size_t posVal = aLine.find("="); 
                    if ( posVal != string::npos ) {
                        //cout << "aLine: found: " << aLine << endl;
                        //cout << "sub" <<  aLine.substr(posVal+2) << endl; 
                        this->ppmStart = svkTypeUtils::StringToFloat( aLine.substr(posVal+2) ); 
                        foundPPMRangeStart = true; 
                    }
                }
            } catch (const exception& e) {
                keepReading = false; 
            }
        }

        delete iss; 
        input->close(); 
        delete input; 
        if ( foundPPMRangeStart && foundPPMRangeEnd ) {
            break; 
        }
        cout << "ST: " << this->ppmStart << " - " << this->ppmEnd << endl; 
    }
}


/*!  
 *  LCMode can produce output spectra with different PPM/Pt resolution than the inpub data. 
 *  Get the LCModel output ranges and compare with the input template to determine if the
 *  template needs to be zero filled to accommodate the LCMode results Hz/pt
 */
void svkLCModelCoordReader::CheckOutputPtsPerPPM()
{
    svkDcmHeader* hdr = svkMrsImageData::SafeDownCast(this->GetImageDataInput(0))->GetDcmHeader();
    svkSpecPoint* point = svkSpecPoint::New();
    point->SetDcmHeader( hdr ); 

    // Get ppm range and num freq points of input template data
    int numSpecPtsIn = hdr->GetIntValue( "DataPointColumns" );
    float startPPMIn   = point->ConvertPosUnits(0, svkSpecPoint::PTS, svkSpecPoint::PPM);
    float endPPMIn     = point->ConvertPosUnits(numSpecPtsIn - 1, svkSpecPoint::PTS, svkSpecPoint::PPM);
    if ( this->GetDebug() ) {
        cout << "PPM IN: " << startPPMIn << " " << endPPMIn << endl; 
    }

    //  Does the input template need to be zero filled? Compare the input and outupt
    //  pts/ppm ratio:         
    float outputPtsPerPPMRatio = this->numCoordFreqPoints / ( this->ppmStart - this->ppmEnd ); 
    float inputPtsPerPPMRatio  = numSpecPtsIn / ( startPPMIn - endPPMIn ); 

    int zeroFillFactor = vtkMath::Round( outputPtsPerPPMRatio/inputPtsPerPPMRatio ); 
    cout << "pts/PPM LCMODEL        : " << outputPtsPerPPMRatio << endl;
    cout << "pts/PPM Input template : " << inputPtsPerPPMRatio << endl;
    cout << "Zero Fill Factor       : " << zeroFillFactor << endl;

    if ( zeroFillFactor > 1 ) {
        svkMrsZeroFill* zeroFill = svkMrsZeroFill::New();
        zeroFill->SetInputData( this->GetImageDataInput(0) );
        zeroFill->SetNumberOfSpecPoints( numSpecPtsIn * zeroFillFactor ); 
        zeroFill->Update(); 
        zeroFill->Delete();
    }
    point->Delete(); 
}


/*!
 *  Parse the intensity values from the current coord file and initialze a float array of values. 
 */
void svkLCModelCoordReader::ParseIntensityValuesFromCoord( vtkTable* coordDataTable, int rowID, float* coordIntensities )
{ 

    //  ===================================================================
    //  Init an array for the intensity values: 
    //  The intensity values follow and are delimited by a string that ends in "follow", e.g. 
    //
    //  NY points of the fit to the data follow  
    //  -1.58078E+08 -1.63007E+08 -1.68433E+08 -1.70633E+08 -1.71649E+08 -1.72759E+08 
    //  -2.08652E+08 -2.15280E+08 -2.18571E+08 -2.16981E+08 -2.11360E+08 -2.04654E+08 
    //  -1.85580E+08
    //  NY phased data points follow
    //  ===================================================================
    int numCols = coordDataTable->GetNumberOfColumns(); 
    int numRows = coordDataTable->GetNumberOfRows(); 

    int freqIndex = 0;  // total of numCoordFreqPoints values
    for ( int rowIDData = rowID+1; rowIDData < numRows; rowIDData++ ) {

        //cout << "rowID: " << rowIDData << endl;
        //cout << "ROW: " << rowString << endl;
        string rowString = ""; 
        this->GetDataRow(coordDataTable, rowIDData, &rowString);  

        int numValuesInRow = coordDataTable->GetRow(rowIDData)->GetNumberOfValues();
        //cout << "DEREF: " << *coordDataTable->GetRow(rowIDData) << endl;
        bool isValid; 
        for ( int word = 0; word < numValuesInRow; word++ ) {
            float intensity = coordDataTable->GetRow(rowIDData)->GetValue(word).ToFloat(&isValid); 
            if ( isValid ) {
                coordIntensities[freqIndex] = coordDataTable->GetRow(rowIDData)->GetValue(word).ToFloat(); 
                //cout << "word: " << word << " " << coordIntensities[freqIndex] << endl;
                freqIndex++; 
            } else {
                break; 
            }
        }
        if ( freqIndex >= this->numCoordFreqPoints ) {
            break; 
        }
        //cout << "freqIndex: " << freqIndex << endl;
    }
}


/*
 *  Move file pointer to start of fitted data intensities as delimited by 
 *  the "dataStartDelimiter" string.  
 *  Returns the rowID in the table 
 */ 
int svkLCModelCoordReader::FindStartOfIntensityData( vtkTable* coordDataTable )
{ 
    int numCols = coordDataTable->GetNumberOfColumns() ; 
    int numRows = coordDataTable->GetNumberOfRows() ; 

    int rowID;  
    string rowString = ""; 
    bool foundDelimiter = false; 
    for ( rowID = 0; rowID < numRows; rowID++ ) {

        //  Get a row: 
        this->GetDataRow(coordDataTable, rowID, &rowString);  
        //cout << "ROW: " << rowString << endl;

        //  Now read on until the start of the intensity values to parse: 
        string dataFitDelimiter = this->dataStartDelimiter;
        size_t foundDataPos = rowString.find( dataFitDelimiter ); 
        if ( foundDataPos != string::npos) {
            foundDelimiter = true; 
            break; 
        }
    }
    if ( foundDelimiter == false ) {
        cout << "WARNING, could not find the coord file delimiter " << this->dataStartDelimiter << endl;
    }
    return rowID; 
}


/*
 *  Parse LCModel coord file 
 */
void svkLCModelCoordReader::ParseCoordFiles()
{
   
    svkMrsImageData* mrsData = svkMrsImageData::SafeDownCast( this->GetOutput() );
    svkDcmHeader* hdr = this->GetOutput()->GetDcmHeader();

    svkDcmHeader::DimensionVector dimVector = hdr->GetDimensionIndexVector(); 
    int voxels[3];  
    hdr->GetSpatialDimensions( &dimVector, voxels ); 


    //  Get the point range of the fitted LCModel PPM range
    svkSpecPoint* point = svkSpecPoint::New();
    point->SetDcmHeader( svkMrsImageData::SafeDownCast(this->GetImageDataInput(0))->GetDcmHeader() );
    int startPt = static_cast<int>(point->ConvertPosUnits(this->ppmStart, svkSpecPoint::PPM, svkSpecPoint::PTS));
    int endPt = static_cast<int>(point->ConvertPosUnits(this->ppmEnd, svkSpecPoint::PPM, svkSpecPoint::PTS));
    cout << "Start->End " << startPt << " " << endPt << endl;
    point->Delete();
    float* coordIntensities = new float[this->numCoordFreqPoints];     
    for (int fileIndex = 0; fileIndex < this->GetFileNames()->GetNumberOfValues(); fileIndex++) {

        string coordFileName = this->GetFileNames()->GetValue( fileIndex );
        vtkDebugMacro( << this->GetClassName() << " Coord NAME: " << fileIndex << " " << coordFileName);
        if ( this->GetDebug() ) {
            cout  << " Coord NAME: " << fileIndex << " " << coordFileName << endl;
        }
 
        struct stat fs;
        if ( stat( coordFileName.c_str(), &fs) ) {
            vtkErrorMacro("Unable to open file " << coordFileName );
            return;
        }
        vtkDelimitedTextReader* coordReader = NULL; 
        coordReader = vtkDelimitedTextReader::New();
        if ( coordReader == NULL ) {
            cout << "ERROR: could not allocate coord reader" << endl;
            exit(1);    
        }
        coordReader->SetFieldDelimiterCharacters("  ");
        coordReader->SetStringDelimiter(' ');
        coordReader->SetMergeConsecutiveDelimiters(true); 
        coordReader->SetHaveHeaders(true);
        coordReader->SetFileName( coordFileName.c_str() ); 
        coordReader->Update();

        vtkTable* table = coordReader->GetOutput();
        //cout << *table << endl;
        //cout << "==========================================" << endl;
        //table->Dump();
        //cout << "==========================================" << endl;

        //  parse through the rows to find the start of the intensity data; 
        int rowID; 
        rowID = this->FindStartOfIntensityData( table ); 

        //  parse the coord file starting at this rowID and initialize an array of 
        //  fitted intensities
        //  for the voxels encoded in this coord file: 
        this->ParseIntensityValuesFromCoord( table, rowID, coordIntensities ); 
      

        // ==========================================
        //  Set the values back into the output data cell: 
        // ==========================================
        
        // Parse the col, row and slice from the file name: 
        // *_cCol#_rRow#_sSlice#, e.g. 
        // fileRoot_c10_r8_s4_... csv
        // Example: test_1377_c9_r11_s4.csv
        int col; 
        int row; 
        int slice; 
        this->GetVoxelIndexFromFileName(coordFileName, &col, &row, &slice); 
        cout << "col " << col << " row " << row << " slice " << slice << endl;
        svkDcmHeader::DimensionVector indexVector = dimVector; 
        svkDcmHeader::SetDimensionVectorValue(&indexVector, svkDcmHeader::COL_INDEX, col);
        svkDcmHeader::SetDimensionVectorValue(&indexVector, svkDcmHeader::ROW_INDEX, row);
        svkDcmHeader::SetDimensionVectorValue(&indexVector, svkDcmHeader::SLICE_INDEX, slice);
        int cellID = svkDcmHeader::GetCellIDFromDimensionVectorIndex( &dimVector, &indexVector);
        vtkFloatArray* spectrumOut  = static_cast<vtkFloatArray*>( mrsData->GetSpectrum( cellID ) );
        //cout << "SO: " << *spectrumOut << endl; exit(1); 

        float tuple[2]; 
        tuple[1] = 0; //imaginary component; 

        svkMrsImageData* mrsData = svkMrsImageData::SafeDownCast( this->GetOutput() );
        svkDcmHeader* hdr = this->GetOutput()->GetDcmHeader();
        int numSpecPtsIn = hdr->GetIntValue( "DataPointColumns" );

        float fftNormalizationFactor = pow(this->numCoordFreqPoints, .5);  
        fftNormalizationFactor = pow(numSpecPtsIn, .5);  
        fftNormalizationFactor = 1; 

        for ( int freqIndex = 0; freqIndex < this->numCoordFreqPoints; freqIndex++ ) {

            //  =============================
            //  Now set these intensity values into the spectrum for this fitted voxel (cellID) 
            //  create an index dimVector and set the values of slice, row, and col from the values in the coord 
            //  once these are set, then determine the voxel index for those values and set the values of 
            //  the spectrum for that CellIndex.  
            //  =============================
            //cout << "FI: " << freqIndex << endl; 
            tuple[0] = coordIntensities[freqIndex]; //imaginary component; 
            //  Scale tuple for FFT normalization factor: 
            tuple[0] *= fftNormalizationFactor; 
            //spectrumOut->SetTuple( freqIndex, tuple );
            spectrumOut->SetTuple( (startPt + freqIndex - 1), tuple );
        }
        coordReader->Delete(); 
    } 

    delete [] coordIntensities; 
}


/*!
 *  Set the string to search for within the output "coord" files. The data to load will follow the 
 *  specfied string, e.g. 
 *       "phased data points follow"
 *       "fit to the data follow"
 */
void svkLCModelCoordReader::SetDataStartDelimiter( string delimiterString )
{
    this->dataStartDelimiter = delimiterString; 
}


/*
 *
 */
void svkLCModelCoordReader::GetDataRow(vtkTable* table, int rowID, string* rowString)
{
    *rowString = ""; 
    int numValuesInRow = table->GetRow(rowID)->GetNumberOfValues();
    //cout << " NVIR: " << numValuesInRow << endl;
    for ( int word = 0; word < numValuesInRow; word++ ) {
        *rowString += table->GetRow(rowID)->GetValue(word).ToString() + " ";
    }
}


/*!
 *  Output is fitted spectra
 */
int svkLCModelCoordReader::FillOutputPortInformation( int vtkNotUsed(port), vtkInformation* info )
{
    info->Set(vtkDataObject::DATA_TYPE_NAME(), "svkMrsImageData");
    return 1;
}



