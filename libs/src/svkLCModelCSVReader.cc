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


#include <svkLCModelCSVReader.h>
#include <vtkDebugLeaks.h>
#include <vtkByteSwap.h>
#include <vtkSmartPointer.h>
#include <vtkDelimitedTextReader.h>
#include <vtkTable.h>

#include <sys/stat.h>


using namespace svk;


vtkCxxRevisionMacro(svkLCModelCSVReader, "$Rev$");
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

    vtkstd::string fileToCheck(fname);

    if( fileToCheck.size() > 4 ) {

        // Also see "vtkImageReader2::GetFileExtensions method" 
        if ( 
            fileToCheck.substr( fileToCheck.size() - 4 ) == ".byt"  ||
            fileToCheck.substr( fileToCheck.size() - 5 ) == ".int2" || 
            fileToCheck.substr( fileToCheck.size() - 5 ) == ".real" || 
            fileToCheck.substr( fileToCheck.size() - 4 ) == ".idf" 
        )  {
            FILE *fp = fopen(fname, "rb");
            if (fp) {
                fclose(fp);
                vtkDebugMacro(<< this->GetClassName() << "::CanReadFile(): It's a UCSF Volume File: " << fileToCheck);
                return 1;
            }
        } else {
            vtkDebugMacro(<< this->GetClassName() << "::CanReadFile(): It's NOT a UCSF Volume File: " << fileToCheck);
            return 0;
        }
    } else {
        vtkDebugMacro(<< this->GetClassName() << "::CanReadFile(): s NOT a valid file: " << fileToCheck);
        return 0;
    }
}



/*!
 *  Reads the IDf data file (.byt, .int2, .real)
 */
void svkLCModelCSVReader::ReadVolumeFile()
{

    vtkDebugMacro( << this->GetClassName() << "::ReadVolumeFile()" );

    svkImageData* data = this->GetOutput(); 

    for (int fileIndex = 0; fileIndex < this->GetFileNames()->GetNumberOfValues(); fileIndex++) {

        vtkstd::string volFileName = this->GetFileRoot( this->GetFileNames()->GetValue( fileIndex ) );
        int dataUnitSize; 
        vtkDataArray* array;
        array =  vtkFloatArray::New();
        volFileName.append( ".real" );
        dataUnitSize = 4;

        // We can now get rid of of our local reference to the array
        array->Delete();

    }
}



/*!
 *  Side effect of Update() method.  Used to load pixel data and initialize vtkImageData
 *  Called after ExecuteInformation()
 */
void svkLCModelCSVReader::ExecuteData(vtkDataObject* output)
{

    this->FileNames = vtkStringArray::New();
    this->FileNames->DeepCopy(this->tmpFileNames);
    this->tmpFileNames->Delete();
    this->tmpFileNames = NULL;

    vtkDebugMacro( << this->GetClassName() << "::ExecuteData()" );

    svkImageData* data = svkImageData::SafeDownCast( this->AllocateOutputData(output) );

    if ( this->GetFileNames()->GetNumberOfValues() ) {

        vtkDebugMacro( << this->GetClassName() << " FileName: " << FileName );
        struct stat fs;
        if ( stat(this->GetFileNames()->GetValue(0), &fs) ) {
            vtkErrorMacro("Unable to open file " << vtkstd::string(this->GetFileNames()->GetValue(0)) );
            return;
        }

        this->ReadVolumeFile();

        this->GetOutput()->SetDataRange( data->GetScalarRange(), svkImageData::REAL );
        double imaginaryRange[2] = {0,0}; 

        // Imaginary values are zeroes-- since images only have real components
        this->GetOutput()->SetDataRange( imaginaryRange, svkImageData::IMAGINARY );

        // Magnitudes are the same as the reals since the imaginaries are zero
        this->GetOutput()->SetDataRange( data->GetScalarRange(), svkImageData::MAGNITUDE );
    }

    /* 
     * We need to make a shallow copy of the output, otherwise we would have it
     * registered twice to the same reader which would cause the reader to never delete.
     */
    double dcos[3][3];
    this->GetOutput()->GetDcmHeader()->GetDataDcos( dcos );
    this->GetOutput()->SetDcos(dcos); 

    //  SetNumberOfIncrements is supposed to call this, but only works if the data has already
    //  been allocated. but that requires the number of components to be specified. 
    this->GetOutput()->GetIncrements(); 
    this->GetOutput()->GetDcmHeader()->GetDimensionIndexVector();

}


/*!
 *  Side effect of Update() method.  Used to initialize the svkDcmHeader member of 
 *  the target svkImageData object and uses the header to set up the Output Informatin.
 *  Called before ExecuteData()
 */
void svkLCModelCSVReader::ExecuteInformation()
{
    vtkDebugMacro( << this->GetClassName() << "::ExecuteInformation()" );

    if (this->FileName == NULL) {
        return;
    }

    if ( this->FileName ) {
        vtkDebugMacro( << this->GetClassName() << " FileName: " << FileName );

        vtkDelimitedTextReader* csvReader = vtkDelimitedTextReader::New();
        cout << "FN: " << this->FileName << endl;
        csvReader->SetFileName(this->FileName); 
        csvReader->SetFieldDelimiterCharacters(", ");
        //csvReader->SetStringDelimiter(' ');
        csvReader->SetHaveHeaders(true);
        csvReader->DetectNumericColumnsOn();
        csvReader->Update();
        vtkTable* table = csvReader->GetOutput();
        //table->Dump();
        int numCols = table->GetNumberOfColumns() ; 
        int numRows = table->GetNumberOfRows() ; 
        cout << "NC: " << numCols  << endl; 
        cout << "NR: " << numRows  << endl; 
        for ( int i = 0; i < numCols; i++ ) {
            cout << "COL NAME: " << table->GetColumnName(i) << endl;
        }

        string colName = "NAA"; 
        vtkFloatArray* array = static_cast<vtkFloatArray*>( table->GetColumnByName( colName.c_str() ) ); 
        if ( array == NULL ) {
            cout << "ERROR:  no column " << colName << endl;
            exit(1); 
        }
        cout << "NUM TUPS: " << array->GetNumberOfTuples() << endl;; 
        for ( int i = 0; i < numRows; i++ ) {
            cout << "Row val: " << array->GetTuple1(i) << endl;
        }

        csvReader->Print(cout); 

        struct stat fs;
        if ( stat(this->FileName, &fs) ) {
            vtkErrorMacro("Unable to open file " << this->FileName );
            return;
        }
exit(0);
        this->InitDcmHeader();
        this->SetupOutputInformation();
    }

    //  This is a workaround required since the vtkImageAlgo executive
    //  for the reder resets the Extent[5] value to the number of files
    //  which is not correct for 3D multislice volume files. So store
    //  the files in a temporary array until after ExecuteData has been
    //  called, then reset the array.
    this->tmpFileNames = vtkStringArray::New();
    this->tmpFileNames->DeepCopy(this->FileNames);
    this->FileNames->Delete();
    this->FileNames = NULL;

}


/*!
 *  *  Returns the file root without extension
 *   */
svkDcmHeader::DcmPixelDataFormat svkLCModelCSVReader::GetFileType()
{

    return svkDcmHeader::SIGNED_FLOAT_4;

}


void svkLCModelCSVReader::InitDcmHeader()
{
}


/*
 *  Read IDF header fields into a string STL map for use during initialization 
 *  of DICOM header by Init*Module methods. 
 */
void svkLCModelCSVReader::ParseCSV()
{
   
    this->GlobFileNames();


    try { 

        int fileIndex = 0;
        //vtkstd::string currentIdfFileName = this->GetFileRoot( this->GetFileNames()->GetValue( fileIndex ) ) + ".idf";


    } catch (const exception& e) {
        cerr << "ERROR opening or reading volume file (" << this->GetFileName() << "): " << e.what() << endl;
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


