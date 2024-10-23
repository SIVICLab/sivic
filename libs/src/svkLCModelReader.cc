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


#include <svkLCModelReader.h>
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


//vtkCxxRevisionMacro(svkLCModelReader, "$Rev$");


/*!
 *  
 */
svkLCModelReader::svkLCModelReader()
{

#if VTK_DEBUG_ON
    this->DebugOn();
    vtkDebugLeaks::ConstructClass("svkLCModelReader");
#endif

    vtkDebugMacro( << this->GetClassName() << "::" << this->GetClassName() << "()" );

    //  3 required input ports: 
    this->SetNumberOfInputPorts(1);

}


/*!
 *
 */
svkLCModelReader::~svkLCModelReader()
{
    vtkDebugMacro( << this->GetClassName() << "::~" << this->GetClassName() << "()" );
}


/*!
 *  Side effect of Update() method.  Used to initialize the svkDcmHeader member of 
 *  the target svkImageData object and uses the header to set up the Output Informatin.
 *  Called before ExecuteData()
 */
void svkLCModelReader::ExecuteInformation()
{
    vtkDebugMacro( << this->GetClassName() << "::ExecuteInformation()" );

    if (this->FileName == NULL) {
        return;
    }
}


/*!
 *  Construct a series descrition for the file being loaded
 */
string svkLCModelReader::GetSeriesDescription()
{
    string seriesDescription = "LCModel Out: "; 
    seriesDescription.append( "fitted spectra" ); 
    return seriesDescription; 
}


/*!
 *  returns the data size. 
 */
svkDcmHeader::DcmPixelDataFormat svkLCModelReader::GetFileType()
{
    return svkDcmHeader::SIGNED_FLOAT_4;
}


/*!
 *  Parse voxel col, row and slice coords out of LCMODEL csv or coord file output generated 
 *  form SIVIC generated control files.  
 *      test_1377_c9_r11_s4.csv
 *      test_1377_c9_r11_s4_sl4_11-9.coord 
 */
void svkLCModelReader::GetVoxelIndexFromFileName(string lcmodelFileName, int* col, int* row, int* slice)
{
    string fileRoot = svkImageReader2::GetFileNameWithoutPath( lcmodelFileName.c_str() );

    size_t posCol   = fileRoot.rfind( "_c" );
    size_t posRow   = fileRoot.find( "_r", posCol );
    size_t posSlice = fileRoot.find( "_s", posRow );
    //  find either the dot or for .coord files the location of the _sl# string: 
    size_t posDot   = fileRoot.find( ".csv" );
    size_t posSL    = fileRoot.find( "_sl" );
    size_t posEnd; 
    if ( posDot != string::npos ) {
        posEnd = posDot; 
    } else if ( posSL != string::npos ) {
        posEnd = posSL; 
    }

    if ( posCol != string::npos && posRow != string::npos && posSlice != string::npos && posEnd != string::npos ) {
        string colStr   = fileRoot.substr(posCol   + 2, posRow   - posCol   - 2);
        string rowStr   = fileRoot.substr(posRow   + 2, posSlice - posRow   - 2);
        string sliceStr = fileRoot.substr(posSlice + 2, posEnd   - posSlice - 2);
        //cout << "VOXEL COLUMN: " << colStr << " " << rowStr << " " << sliceStr << endl;
        *col   = svkTypeUtils::StringToInt(colStr) - 1; 
        *row   = svkTypeUtils::StringToInt(rowStr) - 1; 
        *slice = svkTypeUtils::StringToInt(sliceStr) - 1; 
    } else {
        cout << "ERROR, could not parse voxel index from lcmode output file name: " << lcmodelFileName << endl;
        exit(1); 
    }

}


/*!
 *
 */
void svkLCModelReader::SetMRSFileName( string mrsFileName )
{
    this->mrsFileName = mrsFileName; 
   
    svkImageReaderFactory* readerFactory = svkImageReaderFactory::New();
    svkImageReader2* mrsReader = readerFactory->CreateImageReader2( mrsFileName.c_str() );

    if ( mrsReader == NULL ) {
        cerr << "Can not determine appropriate reader for: " << mrsFileName << endl;
        exit(1);
    }

    mrsReader->SetFileName( mrsFileName.c_str() );
    mrsReader->Update();
    this->SetInputConnection( 0, mrsReader->GetOutputPort() );

    readerFactory->Delete(); 
    mrsReader->Delete(); 
}


/*!
 *
 */
void svkLCModelReader::InitDcmHeader( )
{
}


/*!
 *
 */
void svkLCModelReader::SetMetName( string metName )
{
    this->metName = metName; 
}


/*!
 *  *  input ports 0 - 2 are required. All input ports are MRI data. 
 *      0:  MRS template (required)
 *      1:  CSV file name (required) 
 *      2:  met name (required) 
 */
int svkLCModelReader::FillInputPortInformation( int port, vtkInformation* info )
{
    //  MRS Object
    if ( port == 0  ) {
        info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "svkMrsImageData");
    }

    /*
    //  CSV Root Name
    if ( port == 1  ) {
        info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "svkString");
    }

    //  Met Name
    if ( port == 2  ) {
        info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "svkString");
    }
    */
    return 1;
}


/*!
 *  get the series description for this file for globbing purposes to ensure all
 *  files have the same series description.  For the LCModel reader this isn't 
 *  relevant so just return an empty string.  
 */
string svkLCModelReader::GetFileSeriesDescription( string fileName )
{
    return ""; 
}

