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
#include <vtkDebugLeaks.h>
#include <vtkByteSwap.h>
#include <vtkSmartPointer.h>
#include <vtkDelimitedTextReader.h>
#include <vtkTable.h>
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
 *  Read IDF header fields into a string STL map for use during initialization 
 *  of DICOM header by Init*Module methods. 
 */
void svkLCModelCoordReader::ParseCoordFiles()
{
   
    this->GlobFileNames();

    vtkDataArray* metMapArray = this->GetOutput()->GetPointData()->GetArray(0);

    svkDcmHeader* hdr = this->GetOutput()->GetDcmHeader();
    svkDcmHeader::DimensionVector dimVector = hdr->GetDimensionIndexVector(); 
    int voxels[3];  
    hdr->GetSpatialDimensions( &dimVector, voxels ); 
    int numVoxels = svkDcmHeader::GetNumSpatialVoxels(&dimVector); 

    for (int fileIndex = 0; fileIndex < this->GetFileNames()->GetNumberOfValues(); fileIndex++) {

        string coordFileName = this->GetFileNames()->GetValue( fileIndex );
        cout << "Coord NAME: " << coordFileName << endl;

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
        cout << *table << endl;
        cout << "==========================================" << endl;
        table->Dump();
        cout << "==========================================" << endl;

        int numCols = table->GetNumberOfColumns() ; 
        int numRows = table->GetNumberOfRows() ; 

        //  parse through the rows to find specific pieces of data; 
        for ( int rowID = 0; rowID < numRows; rowID++ ) {

            //  Get a row: 
            string rowString = ""; 
            int numValuesInRow = table->GetRow(rowID)->GetNumberOfValues();
            for ( int word = 0; word < numValuesInRow; word++ ) {
                rowString += table->GetRow(rowID)->GetValue(word).ToString() + " ";
            }

            //  First, find the number of frequency points in fitted spectra: 
            string pointsDelimiter = "points on ppm-axis"; 
            size_t foundPtsPos = rowString.find( pointsDelimiter ); 
            if ( foundPtsPos != string::npos) {
                //  num points is the first word on this line: 
                int numFreqPoints = table->GetRow(rowID)->GetValue(0).ToInt(); 
                cout << "ROW: " << rowString << endl;
                cout << "POINTS: " << numFreqPoints << endl;
            }
        
            //  Now read on until the start of the intensity values to parse: 
            string dataFitDelimiter = "fit to the data follow"; 
            size_t foundDataPos = rowString.find( dataFitDelimiter ); 
            if ( foundDataPos != string::npos) {
                break; 
            }
        }

        for ( int rowIDData = rowID; rowIDData < numRows; rowIDData++ ) {

            cout << "ROW: " << rowString << endl;

            //  Now the intensity values follow and are delimited by a string that ends in "follow", e.g. 
            //
            //  NY points of the fit to the data follow  
            //  -1.58078E+08 -1.63007E+08 -1.68433E+08 -1.70633E+08 -1.71649E+08 -1.72759E+08 
            //  -2.08652E+08 -2.15280E+08 -2.18571E+08 -2.16981E+08 -2.11360E+08 -2.04654E+08 
            //  -1.85580E+08
            //  NY phased data points follow
            
            //  =============================
            //  Now set these intensity values into the spectrum for this fitted voxel (cellID) 
            //  create an index dimVector and set the values of slice, row, and col from the values in the coord 
            //  once these are set, then determine the voxel index for those values and set the values of 
            //  the spectrum for that CellIndex.  
            //  =============================
            svkDcmHeader::DimensionVector indexVector = dimVector; 
            svkDcmHeader::SetDimensionVectorValue(&indexVector, svkDcmHeader::COL_INDEX, col);
            svkDcmHeader::SetDimensionVectorValue(&indexVector, svkDcmHeader::ROW_INDEX, row);
            svkDcmHeader::SetDimensionVectorValue(&indexVector, svkDcmHeader::SLICE_INDEX, slice);
            int cellID = svkDcmHeader::GetCellIDFromDimensionVectorIndex( &dimVector, &indexVector);
            //cout << "VV: " << cellID << " " << colIndex << " " << rowIndex << " " << " " << sliceIndex << " " << voxelValue  << endl;
            vtkFloatArray* spectrumOut  = static_cast<vtkFloatArray*>( mrsData->GetSpectrum( cellID ) );
            for (int i = 0; i < numTimePoints; i++ ) {
                spectrumOut->SetTuple(i, 0.);
            }
        }
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



