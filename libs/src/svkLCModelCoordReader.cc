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


#include <svkLCModelCoordReader.h>
#include <svkImageReaderFactory.h>
#include <svkString.h>
#include <svkTypeUtils.h>
#include <svkFileUtils.h>
#include <svkSpecPoint.h>
#include <vtkDebugLeaks.h>
#include <vtkByteSwap.h>
#include <vtkSmartPointer.h>
#include <vtkDelimitedTextReader.h>
#include <vtkStringToNumeric.h>
#include <vtkVariantArray.h>

#include <sys/stat.h>


using namespace svk;


vtkCxxRevisionMacro(svkLCModelCoordReader, "$Rev$");
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

    vtkstd::string fileToCheck(fname);

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
void svkLCModelCoordReader::ExecuteData(vtkDataObject* output)
{

    vtkDebugMacro( << this->GetClassName() << "::ExecuteData()" );

    svkImageData* data = svkMrsImageData::SafeDownCast( this->AllocateOutputData(output) );


    //  Create the template data object for fitted spectra to be written to
    this->GetOutput()->DeepCopy( this->GetImageDataInput(0) );     

    svkMrsImageData* mrsData = svkMrsImageData::SafeDownCast( this->GetOutput() );

    //  initialize all arrays to zero: 
    svkDcmHeader::DimensionVector dimVec = this->GetOutput()->GetDcmHeader()->GetDimensionIndexVector();
    int numCells = svkDcmHeader::GetNumberOfCells( &dimVec );
    svkDcmHeader* hdr = this->GetOutput()->GetDcmHeader();
    int numTimePoints = hdr->GetIntValue( "DataPointColumns" );
    double zeroTuple[2];
    zeroTuple[0] = 0; 
    zeroTuple[1] = 0; 

    for (int cellID = 0; cellID < numCells; cellID++ ) {
        vtkFloatArray* spectrumOut  = static_cast<vtkFloatArray*>( mrsData->GetSpectrum( cellID ) );
        for (int i = 0; i < numTimePoints; i++ ) {
            spectrumOut->SetTuple(i, zeroTuple);
        }
    }

    this->ParseCoordFiles(); 
}


/*
 *  Parse PPM range from coord file: 
 */
void svkLCModelCoordReader::ParsePPMFromFile( string fileName )
{

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
                }
            } else if ( aLine.find("PPMST") != string::npos) { 
                size_t posVal = aLine.find("="); 
                if ( posVal != string::npos ) {
                    //cout << "aLine: found: " << aLine << endl;
                    //cout << "sub" <<  aLine.substr(posVal+2) << endl; 
                    this->ppmStart = svkTypeUtils::StringToFloat( aLine.substr(posVal+2) ); 
                }
            }
        } catch (const exception& e) {
            keepReading = false; 
        }
    }
    input->close(); 
    cout << "ST: " << this->ppmStart << " - " << this->ppmEnd << endl; 
}


/*
 *  Parse LCModel coord file 
 */
void svkLCModelCoordReader::ParseCoordFiles()
{
   
    this->GlobFileNames();

    svkMrsImageData* mrsData = svkMrsImageData::SafeDownCast( this->GetOutput() );

    svkDcmHeader* hdr = this->GetOutput()->GetDcmHeader();

    int numTimePoints = hdr->GetIntValue( "DataPointColumns" );

    svkDcmHeader::DimensionVector dimVector = hdr->GetDimensionIndexVector(); 
    int voxels[3];  
    hdr->GetSpatialDimensions( &dimVector, voxels ); 
    int numVoxels = svkDcmHeader::GetNumSpatialVoxels(&dimVector); 

    this->ParsePPMFromFile( this->GetFileNames()->GetValue(0) ); 
    svkSpecPoint* point = svkSpecPoint::New();
    point->SetDcmHeader( svkMrsImageData::SafeDownCast(this->GetImageDataInput(0))->GetDcmHeader() );
    int startPt = static_cast<int>(point->ConvertPosUnits(this->ppmStart, svkSpecPoint::PPM, svkSpecPoint::PTS));
    int endPt = static_cast<int>(point->ConvertPosUnits(this->ppmEnd, svkSpecPoint::PPM, svkSpecPoint::PTS));
    cout << "Start->End " << startPt << " " << endPt << endl;
    point->Delete();

    for (int fileIndex = 0; fileIndex < this->GetFileNames()->GetNumberOfValues(); fileIndex++) {

        string coordFileName = this->GetFileNames()->GetValue( fileIndex );
        cout << "Coord NAME: " << fileIndex << " " << coordFileName << endl;

        // ==========================================
        // Parse the col, row and slice from the file name: 
        // *_cCol#_rRow#_sSlice#, e.g. 
        // fileRoot_c10_r8_s4_... csv
        //
        // test_1377_c9_r11_s4.csv
        // ==========================================
        int col; 
        int row; 
        int slice; 
        this->GetVoxelIndexFromFileName(coordFileName, &col, &row, &slice); 

        cout << "col " << col << " row " << row << " slice " << slice << endl;
 
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


        //vtkStringToNumeric* numeric = vtkStringToNumeric::New();
        //numeric->SetInputConnection ( coordReader->GetOutputPort());
        //numeric->Update();
        //vtkTable* table = vtkTable::SafeDownCast(numeric->GetOutput());

        vtkTable* table = coordReader->GetOutput();
        //cout << *table << endl;
        //cout << "==========================================" << endl;
        //table->Dump();
        //cout << "==========================================" << endl;

        int numCols = table->GetNumberOfColumns() ; 
        int numRows = table->GetNumberOfRows() ; 

        //  parse through the rows to find specific pieces of data; 
        int rowID;  
        string rowString = ""; 
        int numFreqPoints;
        for ( rowID = 0; rowID < numRows; rowID++ ) {

            //  Get a row: 
            this->GetDataRow(table, rowID, &rowString);  

            //cout << "ROW: " << rowString << endl;

            //  First, find the number of frequency points in fitted spectra: 
            string pointsDelimiter = "points on ppm-axis"; 
            size_t foundPtsPos = rowString.find( pointsDelimiter ); 
            if ( foundPtsPos != string::npos) {
                //  num points is the first word on this line: 
                numFreqPoints = table->GetRow(rowID)->GetValue(0).ToInt(); 
                //cout << "ROW: " << rowString << endl;
                //cout << "POINTS: " << numFreqPoints << endl;
            }
        
            //  Now read on until the start of the intensity values to parse: 
            string dataFitDelimiter = this->dataStartDelimiter;
            size_t foundDataPos = rowString.find( dataFitDelimiter ); 
            if ( foundDataPos != string::npos) {
                break; 
            }
        }
        //cout << "ROW: " << rowString << endl;

        //  
        //  Create an array for the intensity values: 
        //  The intensity values follow and are delimited by a string that ends in "follow", e.g. 
        //
        //  NY points of the fit to the data follow  
        //  -1.58078E+08 -1.63007E+08 -1.68433E+08 -1.70633E+08 -1.71649E+08 -1.72759E+08 
        //  -2.08652E+08 -2.15280E+08 -2.18571E+08 -2.16981E+08 -2.11360E+08 -2.04654E+08 
        //  -1.85580E+08
        //  NY phased data points follow
        //
        float* coordIntensities = new float[numFreqPoints];     
        float tuple[2]; 
        tuple[1] = 0; //imaginary component; 
        int freqIndex = 0;  // total of numFreqPoints values
        for ( int rowIDData = rowID+1; rowIDData < numRows; rowIDData++ ) {
            //cout << "rowID: " << rowIDData << endl;

            this->GetDataRow(table, rowIDData, &rowString);  
            //cout << "ROW: " << rowString << endl;

            int numValuesInRow = table->GetRow(rowIDData)->GetNumberOfValues();
            //cout << "DEREF: " << *table->GetRow(rowIDData) << endl;
            bool isValid; 
            for ( int word = 0; word < numValuesInRow; word++ ) {
                float intensity = table->GetRow(rowIDData)->GetValue(word).ToFloat(&isValid); 
                if ( isValid ) {
                    coordIntensities[freqIndex] = table->GetRow(rowIDData)->GetValue(word).ToFloat(); 
                    //cout << "word: " << word << " " << coordIntensities[freqIndex] << endl;
                    freqIndex++; 
                } else {
                    break; 
                }
            }
            if ( freqIndex >= numFreqPoints ) {
                break; 
            }
            //cout << "freqIndex: " << freqIndex << endl;
        }

        //  Set the values back into the output data cell: 
        svkDcmHeader::DimensionVector indexVector = dimVector; 
        svkDcmHeader::SetDimensionVectorValue(&indexVector, svkDcmHeader::COL_INDEX, col);
        svkDcmHeader::SetDimensionVectorValue(&indexVector, svkDcmHeader::ROW_INDEX, row);
        svkDcmHeader::SetDimensionVectorValue(&indexVector, svkDcmHeader::SLICE_INDEX, slice);
        int cellID = svkDcmHeader::GetCellIDFromDimensionVectorIndex( &dimVector, &indexVector);
        vtkFloatArray* spectrumOut  = static_cast<vtkFloatArray*>( mrsData->GetSpectrum( cellID ) );


        for ( int freqIndex = 0; freqIndex < numFreqPoints; freqIndex++ ) {

            //  =============================
            //  Now set these intensity values into the spectrum for this fitted voxel (cellID) 
            //  create an index dimVector and set the values of slice, row, and col from the values in the coord 
            //  once these are set, then determine the voxel index for those values and set the values of 
            //  the spectrum for that CellIndex.  
            //  =============================
            //cout << "FI: " << freqIndex << endl; 
            tuple[0] = coordIntensities[freqIndex]; //imaginary component; 
            spectrumOut->SetTuple( freqIndex, tuple );
            //spectrumOut->SetTuple( (startPt + freqIndex - 1), tuple );
        }
        delete [] coordIntensities; 
    } 
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



