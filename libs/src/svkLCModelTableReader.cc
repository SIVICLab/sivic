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


#include <svkLCModelTableReader.h>
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


vtkStandardNewMacro(svkLCModelTableReader);

/*!
 *  
 */
svkLCModelTableReader::svkLCModelTableReader() {

#if VTK_DEBUG_ON
    this->DebugOn();
    vtkDebugLeaks::ConstructClass("svkLCModelTableReader");
#endif

    vtkDebugMacro( << this->GetClassName() << "::" << this->GetClassName() << "()");

    //  3 required input ports: 
    this->SetNumberOfInputPorts(1);

    this->validFields.push_back("FWHM");
    this->validFields.push_back("S/N");
    this->validFields.push_back("Data shift");
    this->validFields.push_back("Ph");

}

/*!
 *
 */
svkLCModelTableReader::~svkLCModelTableReader() {
    vtkDebugMacro( << this->GetClassName() << "::~" << this->GetClassName() << "()");
}

/*!
 *  Check to see if the extension indicates a UCSF IDF file.  If so, try 
 *  to open the file for reading.  If that works, then return a success code. 
 *  Return Values: 1 if can read the file, 0 otherwise.
 */
int svkLCModelTableReader::CanReadFile(const char* fname) {

    std::string fileToCheck(fname);

    if (fileToCheck.size() > 4) {

        // Also see "vtkImageReader2::GetFileExtensions method" 
        if (
                fileToCheck.substr(fileToCheck.size() - 6) == ".table"
                ) {
            FILE* fp = fopen(fname, "rb");
            if (fp) {
                fclose(fp);
                vtkDebugMacro( << this->GetClassName() << "::CanReadFile(): It's an LCModel Table File: " << fileToCheck);
                return 1;
            }
        } else {
            vtkDebugMacro( << this->GetClassName() << "::CanReadFile(): It's NOT an LCModel Table File: " << fileToCheck);
            return 0;
        }
    } else {
        vtkDebugMacro( << this->GetClassName() << "::CanReadFile(): s NOT a valid file: " << fileToCheck);
        return 0;
    }
}


/*!
 *  Side effect of Update() method.  Used to load pixel data and initialize vtkImageData
 *  Called after ExecuteInformation()
 */
void svkLCModelTableReader::ExecuteDataWithInformation(vtkDataObject* output, vtkInformation* outInfo) {

    vtkDebugMacro( << this->GetClassName() << "::ExecuteData()");

    this->IsMetNameValid();

    svkImageData* data = svkImageData::SafeDownCast(this->AllocateOutputData(output, outInfo));

    //  Create the template data object by  
    //  extractng an svkMriImageData from the input svkMrsImageData object
    //  Use an arbitrary point for initialization of scalars.  Actual data 
    //  will be overwritten by algorithm. 
    svkDcmHeader::DimensionVector dimVector = svkImageData::SafeDownCast(this->GetImageDataInput(0))
            ->GetDcmHeader()->GetDimensionIndexVector();
    int numDims = dimVector.size();
    // start from 3, since we just want to set the non spatial indices to 0; 
    for (int dim = 3; dim < numDims; dim++) {
        svkDcmHeader::SetDimensionVectorValue(&dimVector, dim, 0);
    }

    svkMrsImageData::SafeDownCast(this->GetImageDataInput(0))->GetImage(
            svkMriImageData::SafeDownCast(this->GetOutput()),
            0,
            &dimVector,
            0,
            this->GetSeriesDescription(),
            VTK_DOUBLE
            );

    this->ParseTableFiles();

}

/*!
 *  Validate that the specified met name exists in the table file
 */
bool svkLCModelTableReader::IsMetNameValid() 
{

    bool isValid = false;
    for (int i = 0; i < this->validFields.size(); i++) {
        if (this->metName.compare(this->validFields[i]) == 0) {
            isValid = true;
            break;
        }
    }
    if (!isValid) {
        cout << "ERROR: the specified field can not be parsed from the .table files: " << this->metName << endl;
        cout << "The following are valid fields: " << endl;
        for (int i = 0; i < this->validFields.size(); i++) {
            cout << "    " << this->validFields[i] << endl;
        }
        exit(1);
    }
    return isValid;
}

/*
 *  Read Table file fields into a string STL map for use during initialization 
 *  of DICOM header by Init*Module methods. 
 */
void svkLCModelTableReader::ParseTableFiles() {

    this->GlobFileNames();

    vtkDataArray* metMapArray = this->GetOutput()->GetPointData()->GetArray(0);

    svkDcmHeader* hdr = this->GetOutput()->GetDcmHeader();
    svkDcmHeader::DimensionVector dimVector = hdr->GetDimensionIndexVector();
    int voxels[3];
    hdr->GetSpatialDimensions(&dimVector, voxels);

    int numVoxels = svkDcmHeader::GetNumSpatialVoxels(&dimVector);
    for (int i = 0; i < numVoxels; i++) {
        metMapArray->SetTuple1(i, 0);
    }

    for (int fileIndex = 0; fileIndex < this->GetFileNames()->GetNumberOfValues(); fileIndex++) {

        string tableFileName = this->GetFileNames()->GetValue(fileIndex);
        cout << "Table NAME: " << tableFileName << endl;

        // ==========================================
        // Parse the col, row and slice from the file name: 
        // *_cCol#_rRow#_sSlice#, e.g. 
        // fileRoot_c10_r8_s4_... table
        //
        // test_1377_c9_r11_s4.table
        // ==========================================
        vtkDebugMacro( << this->GetClassName() << " FileName: " << tableFileName);

        int col;
        int row;
        int slice;
        this->GetVoxelIndexFromFileName(tableFileName, &col, &row, &slice);

        struct stat fs;
        if (stat(tableFileName.c_str(), &fs)) {
            vtkErrorMacro("Unable to open file " << tableFileName);
            return;
        }

        try {

            this->tableFile = new ifstream();
            this->tableFile->exceptions(ifstream::eofbit | ifstream::failbit | ifstream::badbit);

            this->tableFile->open(tableFileName.c_str(), ifstream::in);
            if (!this->tableFile->is_open()) {
                throw runtime_error("Could not open file: " + tableFileName);
            }

            this->tableFileSize = this->GetFileSize(this->tableFile);

            this->tableFile->clear();
            this->tableFile->seekg(0, ios_base::beg);

            while (!this->tableFile->eof()) {
                this->GetKeyValuePair();
            }
            this->PrintKeyValuePairs();

            this->tableFile->close();

        } catch (const exception& e) {
            cerr << "ERROR opening or reading LCModel file( " << tableFileName << ": " << e.what() << endl;
            exit(1);
        }

        // Set the dimension indices for this voxel and initialize the metMapArray: 
        svkDcmHeader::DimensionVector indexVector = dimVector;
        svkDcmHeader::SetDimensionVectorValue(&indexVector, svkDcmHeader::COL_INDEX, col );
        svkDcmHeader::SetDimensionVectorValue(&indexVector, svkDcmHeader::ROW_INDEX, row );
        svkDcmHeader::SetDimensionVectorValue(&indexVector, svkDcmHeader::SLICE_INDEX, slice );
        int cellID = svkDcmHeader::GetCellIDFromDimensionVectorIndex(&dimVector, &indexVector);
        metMapArray->SetTuple1(
                cellID,
                svkTypeUtils::StringToFloat(this->tableMap[this->metName])
                );
    }
}

/*! 
 *  Utility function to read key/values from spar file 
 *  and set the delimited key/value pair into the stl map.  
 *  \return  0 if found key value pair, 1 otherwise
 */
int svkLCModelTableReader::GetKeyValuePair() 
{

    int status = 1;

    istringstream* iss = new istringstream();

    string keyString;
    string valueString;

    try {
        if (this->tableFile->tellg() <= this->tableFileSize) {

            char delim = '='; 
            if (this->metName.compare("Ph") == 0 ){
                delim = ':'; 
            }

            status = this->ReadLineKeyValue(this->tableFile, iss, delim, &keyString, &valueString);
            if (status == 0) {
                valueString.erase(remove(valueString.begin(), valueString.end(), '\r'), valueString.end());
                this->ParseSubFields(keyString, valueString);
            }
            //cout << "KEY VAL: " << keyString << " = " << valueString << endl;

        } else {
            this->tableFile->seekg(0, ios::end);
        }

    } catch (const exception& e) {
        if (this->GetDebug()) {
            cout << "ERROR reading line: " << e.what() << endl;
        }
    }

    delete iss;
    return status;
}

/*!
 *  Parse specific sub fields fields from table file rows
 */
void svkLCModelTableReader::ParseSubFields(string keyString, string valueString) {

    if (keyString.compare("FWHM") == 0) {

        //  FWHM = 0.052 ppm    S/N =  36
        //  remove units and parse S/N field
        size_t delimPos = valueString.find_first_of(' ');
        string fwhmValue = valueString.substr(0, delimPos);

        string snValue = valueString.substr(delimPos + 1);
        delimPos = snValue.find_first_of('=');
        snValue = snValue.substr(delimPos + 1);
        delimPos = snValue.find_first_not_of(' ');
        snValue = snValue.substr(delimPos);

        this->tableMap["FWHM"] = fwhmValue;
        this->tableMap["S/N"] = snValue;

    } else if (keyString.compare("Data shift") == 0) {

        //Data shift =-0.055 ppm
        //  remove units 
        size_t delimPos = valueString.find_first_of(' ');
        string shiftValue = valueString.substr(0, delimPos);
        this->tableMap["Data shift"] = shiftValue;

    } else if (keyString.compare("Ph") == 0) {

        //Ph:  78 deg      34.9 deg/ppm
        //  remove units 
        size_t delimPos = valueString.find_first_of(' ');
        string phValue = valueString.substr(0, delimPos);
        this->tableMap["Ph"] = phValue;

    } else {
        this->tableMap[keyString] = valueString;
    }
}

/*!
 *  Prints the key value pairs parsed from the header of a single table file. 
 */
void svkLCModelTableReader::PrintKeyValuePairs() {

    //  Print out key value pairs parsed from header:
    map< string, string >::iterator mapIter;
    for (mapIter = this->tableMap.begin(); mapIter != this->tableMap.end(); ++mapIter) {
        cout << this->GetClassName() << " " << mapIter->first << " = " << mapIter->second << endl;
    }
}

/*!
 *
 */
int svkLCModelTableReader::FillOutputPortInformation(int vtkNotUsed(port), vtkInformation* info) {
    info->Set(vtkDataObject::DATA_TYPE_NAME(), "svkMriImageData");
    return 1;
}



