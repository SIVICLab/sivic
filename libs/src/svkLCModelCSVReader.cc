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


#include <svkLCModelCSVReader.h>
#include <svkImageReaderFactory.h>
#include <svkString.h>
#include <svkTypeUtils.h>
#include </usr/include/vtk/vtkDebugLeaks.h>
#include </usr/include/vtk/vtkByteSwap.h>
#include </usr/include/vtk/vtkSmartPointer.h>
#include </usr/include/vtk/vtkDelimitedTextReader.h>
#include </usr/include/vtk/vtkTable.h>
#include </usr/include/vtk/vtkStringToNumeric.h>

#include <sys/stat.h>


using namespace svk;


//vtkCxxRevisionMacro(svkLCModelCSVReader, "$Rev$");
vtkStandardNewMacro(svkLCModelCSVReader);


/*!
 *  
 */
svkLCModelCSVReader::svkLCModelCSVReader()
{

#if VTK_DEBUG_ON
    this->DebugOn();
    vtkDebugLeaks::ConstructClass("svkLCModelCSVReader");
#endif

    vtkDebugMacro( << this->GetClassName() << "::" << this->GetClassName() << "()" );

    //  3 required input ports: 
    this->SetNumberOfInputPorts(1);

}


/*!
 *
 */
svkLCModelCSVReader::~svkLCModelCSVReader()
{
    vtkDebugMacro( << this->GetClassName() << "::~" << this->GetClassName() << "()" );
}



/*!
 *  Check to see if the extension indicates a UCSF IDF file.  If so, try 
 *  to open the file for reading.  If that works, then return a success code. 
 *  Return Values: 1 if can read the file, 0 otherwise.
 */
int svkLCModelCSVReader::CanReadFile(const char* fname)
{

    std::string fileToCheck(fname);

    if( fileToCheck.size() > 4 ) {

        // Also see "vtkImageReader2::GetFileExtensions method" 
        if ( 
            fileToCheck.substr( fileToCheck.size() - 4 ) == ".csv"  
        )  {
            FILE* fp = fopen(fname, "rb");
            if (fp) {
                fclose(fp);
                vtkDebugMacro(<< this->GetClassName() << "::CanReadFile(): It's an LCModel CSV File: " << fileToCheck);
                return 1;
            }
        } else {
            vtkDebugMacro(<< this->GetClassName() << "::CanReadFile(): It's NOT a LCModel CSV File: " << fileToCheck);
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
void svkLCModelCSVReader::ExecuteDataWithInformation(vtkDataObject* output, vtkInformation* outInfo)
{

    vtkDebugMacro( << this->GetClassName() << "::ExecuteData()" );

    svkImageData* data = svkImageData::SafeDownCast( this->AllocateOutputData(output, outInfo) );

    //  Create the template data object by  
    //  extractng an svkMriImageData from the input svkMrsImageData object
    //  Use an arbitrary point for initialization of scalars.  Actual data 
    //  will be overwritten by algorithm. 
    svkMriImageData::SafeDownCast( this->GetOutput() ); 
    
    svkMrsImageData::SafeDownCast( this->GetImageDataInput(0) )->GetImage(
                svkMriImageData::SafeDownCast( this->GetOutput() ),
                0,
                0,
                0,
                0,
                this->GetSeriesDescription(),
                VTK_DOUBLE
    );

    //  Now map the csv info into the pixel data and done.
    //this->GetOutput()->GetDcmHeader()->PrintDcmHeader();  
    
    this->ParseCSVFiles(); 

}


/*
 *  Read IDF header fields into a string STL map for use during initialization 
 *  of DICOM header by Init*Module methods. 
 */
void svkLCModelCSVReader::ParseCSVFiles()
{
   
    this->GlobFileNames();

    vtkDataArray* metMapArray = this->GetOutput()->GetPointData()->GetArray(0);
    //cout << "MMA: " << metMapArray << endl;
    //cout << "MMA: " << *metMapArray << endl;
    //cout << "this OUTPUT" << *(this->GetOutput()) << endl;

    svkDcmHeader* hdr = this->GetOutput()->GetDcmHeader();
    svkDcmHeader::DimensionVector dimVector = hdr->GetDimensionIndexVector(); 
    int voxels[3];  
    hdr->GetSpatialDimensions( &dimVector, voxels ); 
    //cout << "NV: " << voxels[0] << " " << voxels[1] << " " << voxels[2] << endl;
    int numVoxels = svkDcmHeader::GetNumSpatialVoxels(&dimVector); 
    for (int i = 0; i < numVoxels; i++ ) {
        metMapArray->SetTuple1(i, 0);
    }


    for (int fileIndex = 0; fileIndex < this->GetFileNames()->GetNumberOfValues(); fileIndex++) {

        string csvFileName = this->GetFileNames()->GetValue( fileIndex );
        cout << "CSV NAME: " << csvFileName << endl;

        // ==========================================
        // Parse the col, row and slice from the file name: 
        // *_cCol#_rRow#_sSlice#, e.g. 
        // fileRoot_c10_r8_s4_... csv
        //
        // test_1377_c9_r11_s4.csv
        // ==========================================
        vtkDebugMacro( << this->GetClassName() << " FileName: " << csvFileName );

        int col; 
        int row; 
        int slice; 
        this->GetVoxelIndexFromFileName(csvFileName, &col, &row, &slice); 
 
        struct stat fs;
        if ( stat( csvFileName.c_str(), &fs) ) {
            vtkErrorMacro("Unable to open file " << csvFileName );
            return;
        }

        vtkDelimitedTextReader* csvReader = vtkDelimitedTextReader::New();
        csvReader->SetFieldDelimiterCharacters(",");
        csvReader->SetMergeConsecutiveDelimiters(true); 
        csvReader->SetHaveHeaders(true);
        csvReader->SetFileName( csvFileName.c_str() ); 
        csvReader->Update();

        vtkStringToNumeric* numeric = vtkStringToNumeric::New();
        numeric->SetInputConnection ( csvReader->GetOutputPort());
        numeric->Update();
        vtkTable* table = vtkTable::SafeDownCast(numeric->GetOutput());
        
        //cout << "==========================================" << endl;
        //table->Dump();
        //cout << "==========================================" << endl;
        int numCols = table->GetNumberOfColumns() ; 
        int numRows = table->GetNumberOfRows() ; 

        for ( int i = 0; i < numCols; i++ ) {
            string colName( table->GetColumn(i)->GetName() ); 
            table->GetColumn(i)->SetName( this->StripWhite( colName ).c_str() ); 
            //cout << "COL NAME: |" << table->GetColumnName(i) << "|" << endl;
            //cout << table->GetColumn(i)->GetName() << endl; 
            //cout << *table->GetColumn(i) << endl; 
        }

        //  =============================
        //  initialize the row vals 
        //  =============================
        if (!vtkIntArray::SafeDownCast(table->GetColumnByName( "Row"  )) ) {
            cout << "Warning: no column Row" <<  endl;
            //exit(1); 
            continue; 
        } else {
            this->csvRowIndex = vtkIntArray::SafeDownCast(table->GetColumnByName( "Row"  )); 
        }

        //  =============================
        //  initialize the col vals
        //  =============================
        if (!vtkIntArray::SafeDownCast(table->GetColumnByName( "Col"  )) ) {
            cout << "Warning:  no column: Col" <<  endl;
            //exit(1); 
            continue; 
        } else {
            this->csvColIndex = vtkIntArray::SafeDownCast(table->GetColumnByName( "Col"  )); 
        }


        //  =============================
        //  initialize the pixel values
        //  =============================
        string colName = this->metName; 
        if ( !table->GetColumnByName(colName.c_str() ) ) {
            cout << "Warning:  no column " << colName << endl;
            //exit(1); 
            continue; 
        } else {
            this->csvPixelValues = table->GetColumnByName(colName.c_str() ) ; 
        }
        int csvPixelDataType = this->csvPixelValues->GetDataType();  

        

        //  =============================
        //  initialize a slice index from 
        //  the csv file name: 
        //  =============================
        int sliceIndex = slice;  
        //cout << "NUM TUPS: " << this->csvPixelValues->GetNumberOfTuples() << endl;; 
        //for ( int i = 0; i < numRows; i++ ) {
            //cout << "Row val: " << this->csvPixelValues->GetTuple1(i) << endl;
        //}

        //  =============================

        //  create an index dimVector and set the values of slice, row, and col from the values in teh csv
        //  once these are set, then determine the voxel index for those values and set the value of 
        //  the metMapArray for that index.  


        float voxelValue;
        svkDcmHeader::DimensionVector indexVector = dimVector; 
        //cout << "NUM ROWS: " << numRows << endl;
        for (int i = 0; i < numRows; i++ ) {
            int rowIndex     = this->csvRowIndex->GetTuple1(i) - 1;
            int colIndex     = this->csvColIndex->GetTuple1(i) - 1;
            if ( csvPixelDataType == VTK_DOUBLE ) {
                voxelValue = vtkDoubleArray::SafeDownCast( this->csvPixelValues )->GetTuple1(i) ;
            } else if ( csvPixelDataType == VTK_INT ) {
                voxelValue = vtkIntArray::SafeDownCast( this->csvPixelValues )->GetTuple1(i) ;
            }
            //cout << "PV: " << voxelValue << endl;
            svkDcmHeader::SetDimensionVectorValue(&indexVector, svkDcmHeader::COL_INDEX, colIndex);
            svkDcmHeader::SetDimensionVectorValue(&indexVector, svkDcmHeader::ROW_INDEX, rowIndex);
            svkDcmHeader::SetDimensionVectorValue(&indexVector, svkDcmHeader::SLICE_INDEX, sliceIndex);
            int cellID = svkDcmHeader::GetCellIDFromDimensionVectorIndex( &dimVector, &indexVector);
            //cout << "VV: " << cellID << " " << colIndex << " " << rowIndex << " " << " " << sliceIndex << " " << voxelValue  << endl;
            metMapArray->SetTuple1(cellID, voxelValue);
        }

    }
}


/*!
 *
 */
int svkLCModelCSVReader::FillOutputPortInformation( int vtkNotUsed(port), vtkInformation* info )
{
    info->Set(vtkDataObject::DATA_TYPE_NAME(), "svkMriImageData");
    return 1;
}



