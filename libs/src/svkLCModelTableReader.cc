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


#include <svkLCModelTableReader.h>
#include <svkImageReaderFactory.h>
#include <svkString.h>
#include <svkTypeUtils.h>
#include <vtkDebugLeaks.h>
#include <vtkByteSwap.h>
#include <vtkSmartPointer.h>
#include <vtkDelimitedTextReader.h>
#include <vtkTable.h>
#include <vtkStringToNumeric.h>

#include <sys/stat.h>


using namespace svk;


vtkStandardNewMacro(svkLCModelTableReader);


/*!
 *  
 */
svkLCModelTableReader::svkLCModelTableReader()
{

#if VTK_DEBUG_ON
    this->DebugOn();
    vtkDebugLeaks::ConstructClass("svkLCModelTableReader");
#endif

    vtkDebugMacro( << this->GetClassName() << "::" << this->GetClassName() << "()" );

    //  3 required input ports: 
    this->SetNumberOfInputPorts(1);

}


/*!
 *
 */
svkLCModelTableReader::~svkLCModelTableReader()
{
    vtkDebugMacro( << this->GetClassName() << "::~" << this->GetClassName() << "()" );
}



/*!
 *  Check to see if the extension indicates a UCSF IDF file.  If so, try 
 *  to open the file for reading.  If that works, then return a success code. 
 *  Return Values: 1 if can read the file, 0 otherwise.
 */
int svkLCModelTableReader::CanReadFile(const char* fname)
{

    vtkstd::string fileToCheck(fname);

    if( fileToCheck.size() > 4 ) {

        // Also see "vtkImageReader2::GetFileExtensions method" 
        if ( 
            fileToCheck.substr( fileToCheck.size() - 4 ) == ".table"  
        )  {
            FILE* fp = fopen(fname, "rb");
            if (fp) {
                fclose(fp);
                vtkDebugMacro(<< this->GetClassName() << "::CanReadFile(): It's an LCModel Table File: " << fileToCheck);
                return 1;
            }
        } else {
            vtkDebugMacro(<< this->GetClassName() << "::CanReadFile(): It's NOT a LCModel Table File: " << fileToCheck);
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
void svkLCModelTableReader::ExecuteDataWithInformation(vtkDataObject* output, vtkInformation* outInfo)
{

    vtkDebugMacro( << this->GetClassName() << "::ExecuteData()" );

    svkImageData* data = svkImageData::SafeDownCast( this->AllocateOutputData(output, outInfo) );

    //  Create the template data object by  
    //  extractng an svkMriImageData from the input svkMrsImageData object
    //  Use an arbitrary point for initialization of scalars.  Actual data 
    //  will be overwritten by algorithm. 
    svkDcmHeader::DimensionVector dimVector = data->GetDcmHeader()->GetDimensionIndexVector();
    int numDims = dimVector.size();
    // start from 3, since we just want to set the non spatial indices to 0; 
    for ( int dim = 3; dim < numDims; dim++) {
        svkDcmHeader::SetDimensionVectorValue( &dimVector, dim, 0);
    }
    
    svkMrsImageData::SafeDownCast( this->GetImageDataInput(0) )->GetImage(
        svkMriImageData::SafeDownCast( this->GetOutput() ),
        0,
        &dimVector,
        0,
        this->GetSeriesDescription(),
        VTK_DOUBLE
    );

    this->ParseTableFiles(); 

}


/*
 *  Read IDF header fields into a string STL map for use during initialization 
 *  of DICOM header by Init*Module methods. 
 */
void svkLCModelTableReader::ParseTableFiles()
{
   
    this->GlobFileNames();

    vtkDataArray* metMapArray = this->GetOutput()->GetPointData()->GetArray(0);

    svkDcmHeader* hdr = this->GetOutput()->GetDcmHeader();
    svkDcmHeader::DimensionVector dimVector = hdr->GetDimensionIndexVector(); 
    int voxels[3];  
    hdr->GetSpatialDimensions( &dimVector, voxels ); 

    int numVoxels = svkDcmHeader::GetNumSpatialVoxels(&dimVector); 
    for (int i = 0; i < numVoxels; i++ ) {
        metMapArray->SetTuple1(i, 0);
    }

    for (int fileIndex = 0; fileIndex < this->GetFileNames()->GetNumberOfValues(); fileIndex++) {

        string tableFileName = this->GetFileNames()->GetValue( fileIndex );
        cout << "Table NAME: " << tableFileName << endl;

        // ==========================================
        // Parse the col, row and slice from the file name: 
        // *_cCol#_rRow#_sSlice#, e.g. 
        // fileRoot_c10_r8_s4_... table
        //
        // test_1377_c9_r11_s4.table
        // ==========================================
        vtkDebugMacro( << this->GetClassName() << " FileName: " << tableFileName );

        int col; 
        int row; 
        int slice; 
        this->GetVoxelIndexFromFileName(tableFileName, &col, &row, &slice); 
 
        struct stat fs;
        if ( stat( tableFileName.c_str(), &fs) ) {
            vtkErrorMacro("Unable to open file " << tableFileName );
            return;
        }

        try {

            ifstream* tableFile = new ifstream();
            tableFile->exceptions( ifstream::eofbit | ifstream::failbit | ifstream::badbit );

            tableFile->open( tableFileName.c_str(), ifstream::in );
            if ( ! tableFile->is_open() ) {
                throw runtime_error( "Could not open file: " + tableFileName);
            }

            long tableFileSize = this->GetFileSize( tableFile );

            tableFile->clear();
            tableFile->seekg( 0, ios_base::beg );

            while (! tableFile->eof() ) {
                this->GetSparKeyValuePair(); 
            }

            tableFile->close();

        } catch (const exception& e) {
            cerr << "ERROR opening or reading LCModel file( " << tableFileName << ": " << e.what() << endl;
            exit(1); 
        }
        /*
        //  =============================
        //  initialize the pixel values
        //  =============================
        string colName = this->metName; 
        if ( !table->GetColumnByName(colName.c_str() ) ) {
            cout << "Warning:  no column " << colName << endl;
            //exit(1); 
            continue; 
        } else {
            this->tablePixelValues = table->GetColumnByName(colName.c_str() ) ; 
        }
        int tablePixelDataType = this->tablePixelValues->GetDataType();  

        

        //  =============================
        //  initialize a slice index from 
        //  the table file name: 
        //  =============================
        int sliceIndex = slice;  
        //  =============================
        //  create an index dimVector and set the values of slice, row, and col from the values in teh table
        //  once these are set, then determine the voxel index for those values and set the value of 
        //  the metMapArray for that index.  

        float voxelValue;
        svkDcmHeader::DimensionVector indexVector = dimVector; 
        //cout << "NUM ROWS: " << numRows << endl;
        for (int i = 0; i < numRows; i++ ) {
            int rowIndex     = this->tableRowIndex->GetTuple1(i) - 1;
            int colIndex     = this->tableColIndex->GetTuple1(i) - 1;
            if ( tablePixelDataType == VTK_DOUBLE ) {
                voxelValue = vtkDoubleArray::SafeDownCast( this->tablePixelValues )->GetTuple1(i) ;
            } else if ( tablePixelDataType == VTK_INT ) {
                voxelValue = vtkIntArray::SafeDownCast( this->tablePixelValues )->GetTuple1(i) ;
            }
            //cout << "PV: " << voxelValue << endl;
            svkDcmHeader::SetDimensionVectorValue(&indexVector, svkDcmHeader::COL_INDEX, colIndex);
            svkDcmHeader::SetDimensionVectorValue(&indexVector, svkDcmHeader::ROW_INDEX, rowIndex);
            svkDcmHeader::SetDimensionVectorValue(&indexVector, svkDcmHeader::SLICE_INDEX, sliceIndex);
            int cellID = svkDcmHeader::GetCellIDFromDimensionVectorIndex( &dimVector, &indexVector);
            metMapArray->SetTuple1(cellID, voxelValue);
        }
        */

    }
}


/*!
 *
 */
int svkLCModelTableReader::FillOutputPortInformation( int vtkNotUsed(port), vtkInformation* info )
{
    info->Set(vtkDataObject::DATA_TYPE_NAME(), "svkMriImageData");
    return 1;
}



