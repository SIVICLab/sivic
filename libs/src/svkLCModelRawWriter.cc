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



#include <svkLCModelRawWriter.h>
#include <svkDcmHeader.h>
#include </usr/include/vtk/vtkErrorCode.h>
#include </usr/include/vtk/vtkCellData.h>
#include </usr/include/vtk/vtkExecutive.h>
#include </usr/include/vtk/vtkFloatArray.h>
#include <svkImageReader2.h>
#include <svkTypeUtils.h>


using namespace svk;


//vtkCxxRevisionMacro(svkLCModelRawWriter, "$Rev$");
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
    this->basisFileName = ""; 
    this->quantificationMask = NULL;
    this->singleRawFile = true;

    this->gridTaskIDCounter = 0;
    this->gridTasks = ""; 
    this->gridArch = "lx24-amd64"; 
    this->gridQueue = "city.q@novato.radiology.ucsf.edu"; 
}



/*!
 *
 */
svkLCModelRawWriter::~svkLCModelRawWriter()
{
    vtkDebugMacro( << this->GetClassName() << "::~" << this->GetClassName() << "()" );
}


/*!
 *  Set name of basis file
 */
void svkLCModelRawWriter::SetBasisFileName( string fileName )
{
    this->basisFileName = fileName; 
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


    svkDcmHeader* hdr = this->GetImageDataInput(0)->GetDcmHeader(); 
    string representation = hdr->GetStringValue( "DataRepresentation" );

    int numComponents = 1;
    if (representation.compare("COMPLEX") == 0) {
        numComponents = 2;
    }

    // write out one coil per ddf file
    string fileName;

    svkDcmHeader::DimensionVector dimensionVector = hdr->GetDimensionIndexVector(); 
    svkDcmHeader::DimensionVector loopVector = dimensionVector; 

    //  In this case we only want to loop over dimensions other than the time dimension, so set 
    //  time to a single value in this vector:

    this->InitQuantificationMask();

    ofstream controlOut; 
    ofstream rawOut; 
    string rawExtension     = ".raw"; 
    string controlExtension = ".control"; 
    string fileRoot = string(this->InternalFileName);  

    //  ====================================================
    //  write out all data in 4D object to a single raw 
    //  file.  Assumes ony 4 dim data.   
    //  ====================================================
    rawOut.open(   ( fileRoot + rawExtension ).c_str() );
    if( !rawOut ) {
        throw runtime_error("Cannot open .raw file for writing");
    }
    this->InitRawHeader( &rawOut, fileRoot );
    this->InitSpecData(&rawOut, &dimensionVector, &loopVector); 
    rawOut.close(); 


    //  ====================================================
    //  Always have separate control files, 1 per voxel. 
    //  Iterate over each cell: 
    //  ====================================================
    int numCells = svkDcmHeader::GetNumberOfCells( &dimensionVector );
    for ( int cellID = 0; cellID < numCells; cellID++ )  {
        if ( this->quantificationMask[cellID] == 1 ) {

            int cellIDInt = cellID + 1 ; 
            string cellIDString = svkTypeUtils::IntToString( cellIDInt ); 
            string fileRootCell = fileRoot + "_" + cellIDString; 
            string controlFileName = fileRootCell + controlExtension; 
            this->CreateGridTask( controlFileName ); 

            controlOut.open( ( fileRootCell + controlExtension ).c_str() );
            if( !controlOut ) {
                throw runtime_error("Cannot open .control file for writing");
            }

            //  ========================================
            //  Initialize the .control file 
            //  ========================================
            this->InitControlHeader( &controlOut, fileRootCell, fileRoot, cellID);

            controlOut.close();
       }
    }

    this->WriteGridSubmissionFile( fileRoot ); 
}


/*!
 *  Initializes and creates the grid submission file. 
 *  This should get reimplemented in DRMAA
 */
void svkLCModelRawWriter::WriteGridSubmissionFile( string fileRoot ) 
{

    string submitScriptName = fileRoot + ".grid"; 
    if( this->submitScript.is_open() == false) {
        //cout << this->submitScript << endl;
        this->submitScript.open( submitScriptName.c_str() );
    }

    if( this->submitScript.is_open() ) {

        this->submitScript << "#!/bin/csh" << endl;
        this->submitScript << "" << endl;
        this->submitScript << "#$ -sync yes " << endl;
        this->submitScript << "#$ -N lcmodel_batch" << endl; 
        this->submitScript << "#$ -l arch=" << this->gridArch << endl;
        this->submitScript << "#$ -q " << this->gridQueue << endl;
        this->submitScript << "#$ -t 1-" << this->gridTaskIDCounter << endl;
        this->submitScript << "#$ -cwd " << endl;
        this->submitScript << "" << endl;
        this->submitScript << this->gridTasks << endl;
        this->submitScript.close(); 

    } else {
        cout << "ERROR: could not open LCModel grid script: " << submitScriptName << endl;
        exit(1); 
    }
}


/*!
 * Adds a task to the grid file: 
 */
void svkLCModelRawWriter::CreateGridTask( string controlFileName ) 
{
    this->gridTaskIDCounter++; 
    this->gridTasks.append("if ($SGE_TASK_ID == " + svkTypeUtils::IntToString( this->gridTaskIDCounter ) + " ) then \n"); 
    this->gridTasks.append("    lcmodel < " +  controlFileName + "\n"); 
    this->gridTasks.append("endif \n\n"); 
}


/*!
 *  Loops over x,y,z spatial indices to initialize the specData buffer from data in the 
 *  svkImageData object.  The specData buffer is what gets written to the
 *  cmplx file.  With the exception of the offsetOut, this is the same for both blocks of 
 *  WriteData().  The dimensionVector argument is the current set of indices to write, not necessarily
 *  the data dimensionality. 
 */
void svkLCModelRawWriter::InitSpecData(ofstream* out, svkDcmHeader::DimensionVector* dimensionVector, svkDcmHeader::DimensionVector* indexVector) 
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

                if ( this->quantificationMask[cellID] == 1 ) {
                    //cout << "fit: " << x << " " << y << " " << z << endl;
                }

                for (int i = 0; i < specPts; i++) {

                    if ( this->quantificationMask[cellID] == 1 ) {
                        fa->GetTypedTuple(i, dataTuple);
                        //cout << "cellID(" << cellID << ") tup: " << i << " " << dataTuple[0] << endl;
                    } else {
                        dataTuple[0] = 0; 
                        dataTuple[1] = 0; 
                    }

                    //*out << " " ;
                    for (int j = 0; j < numComponents; j++) {
                        //*out << fixed << setw(18) << setprecision(3) <<  dataTuple[j];
                        *out << " " ;
                        *out << fixed << setw(18) << setprecision(3) <<  dataTuple[j] << endl;
                    }
                    //*out << endl;
                }
            }
        }
    }

    delete[] dataTuple; 

}


/*!
 */
void svkLCModelRawWriter::InitQuantificationMask()
{
    //  Determines binary mask (quantificationMask) indicating whether a given voxel 
    //  should be quantified (1) or not (0). Usually this is based on whether a specified 
    //  fraction of the voxel inside the selected volume. 
    if ( this->quantificationMask == NULL ) {
        int numVoxels[3];
        this->GetImageDataInput(0)->GetNumberOfVoxels(numVoxels);
        int totalVoxels = numVoxels[0] * numVoxels[1] * numVoxels[2];
        //cout << " TOTAL VOXELS: " << totalVoxels << endl;
        this->quantificationMask = new short[totalVoxels];

        float selectedVolumeFraction = 0.5; 
        svkMrsImageData::SafeDownCast( this->GetImageDataInput(0) )->GetSelectionBoxMask(
            this->quantificationMask,
            selectedVolumeFraction
        );

    }
}


/*!
 *
 *  NVOXSK = 22
 *  ICOLSK(1) = 4
 *  IROWSK(1) = 18
 *
 *  ICOLSK(2) = 4
 *  IROWSK(2) = 19
 *  ICOLSK(3) = 5
 *  IROWSK(3) = 18
 *  ICOLSK(4) = 5
 *  IROWSK(4) = 19
 *  ICOLSK(5) = 6
 *  IROWSK(5) = 18
 *  ICOLSK(6) = 6
 *  IROWSK(6) = 19
 *  ....   
 *  
 *      cellID is the zero indexed cellID for the current spectrum: 
 */
void svkLCModelRawWriter::InitControlHeader(ofstream* out, string fileRootName, string rawRootName, int cellID)
{

    // initialize the DimensionVector that represents the current cellID: 
    svkDcmHeader* hdr = this->GetImageDataInput(0)->GetDcmHeader(); 
    svkDcmHeader::DimensionVector dimensionVector = hdr->GetDimensionIndexVector(); 
    svkDcmHeader::DimensionVector loopVector = dimensionVector; 
    svkDcmHeader::GetDimensionVectorIndexFromCellID( &dimensionVector, &loopVector, cellID );

    //  this is a 1 indexed array
    int voxelIndex[3];
    svkDcmHeader::GetSpatialDimensions( &loopVector,  voxelIndex );


    //  ===========================================================
    //  NDCOLS (integer) (“Number of data columns”) The number of columns in the
    //  CSI data set.
    //  Default: NDCOLS = 1
    //  ===========================================================
    int numCols = hdr->GetIntValue( "Columns" );
    
    //  ===========================================================
    //  NDROWS (integer) (“Number of data rows”) The number of rows in the CSI data
    //  set.
    //  Default: NDROWS = 1
    //  ===========================================================
    int numRows = hdr->GetIntValue( "Rows" );
    
    //  ===========================================================
    //  NDSLIC (integer) (“Number of data slices”) The number of slices in the CSI data
    //  set.
    //  I believe this is the number of slices in the raw file, which for us will be 1.
    //  Default: NDSLIC = 1
    //  ===========================================================
    int numSlices;
    numSlices = hdr->GetNumberOfSlices();  

    int currentSlice; 
    currentSlice = voxelIndex[2]; 
 
    //  ===========================================================
    //  You specify the range (rectangular subset) of voxels to be analyzed by LCModel with:
    //  ICOLST & ICOLEN (integer). Voxels in columns ICOLST through ICOLEN will be
    //  analyzed.
    //  Default: ICOLST = ICOLEN = 1
    //  ===========================================================
    //  for now just set to be all voxels, but will reduce this later 
    int colStart = voxelIndex[0]; 
    int colEnd = voxelIndex[0]; 
    
    //  ===========================================================
    //  IROWST & IROWEN (integer). Voxels in rows IROWST through IROWEN will be analyzed.
    //  Default: IROWST = IROWEN = 1
    //  ===========================================================
    int rowStart = voxelIndex[1]; 
    int rowEnd = voxelIndex[1]; 

    string colString = svkTypeUtils::IntToString( voxelIndex[0]); 
    string rowString = svkTypeUtils::IntToString( voxelIndex[1]); 
    string sliceString = svkTypeUtils::IntToString( voxelIndex[2]); 
    string indexSuffix = "_c" + colString + "_r" + rowString + "_s" + sliceString; 
    
    //  ===========================================================
    //  ISLICE (integer). Voxels in slice ISLICE will be analyzed.
    //  Default: ISLICE = 1
    //  ===========================================================
    
    
    //  ===========================================================
    //  NVOXSK (integer) the number of voxels to be skipped. Default: NVOXSK = 0
    //  ===========================================================
    int  numVoxSkipped = 0;  
    
    //  ===========================================================
    //  ICOLSK (integer(64)) ICOLSK(J) specifies the column of the J’th voxel to be skipped.
    //  ===========================================================
    
    //  ===========================================================
    //  IROWSK (integer(64)) IROWSK(J) specifies the row of the J’th voxel to be skipped.
    //  ===========================================================
 
    //  ===========================================================
    //  NUNFIL (integer) (“N unfilled”) the number of complex pairs of data points in
    //  one scan, without zero-filling. Default: NUNFIL = 2048 
    //  ===========================================================
    int numSpecPts = hdr->GetIntValue( "DataPointColumns" );

    //  ===========================================================
    //  DELTAT (real) (“δt”) the time (in s) between two consecutive real points (or two
    //  consecutive imaginary points). This is just the sample time or “dwell time”
    //  or the reciprocal of the “bandwidth.” Default: DELTAT = 0.0005
    //  ===========================================================
    float dwellTimeSecs = 1./hdr->GetFloatValue( "SpectralWidth" ); 

    //  ===========================================================
    //  HZPPPM (real) (“Hz per ppm”) the field strength, in terms of the proton resonance
    //  frequency in MHz, i.e., HZPPPM = 42.58B0, with B0 in Tesla. You should
    //  input it with an accuracy of at least four significant figures.
    //  Default: HZPPPM = 84.47 (1.984 T; so, you will almost certainly have to
    //  input your own value.)
    //  ===========================================================
    float transmitterFrequency = hdr->GetFloatValue( "TransmitterFrequency" );

    //  ===========================================================
    //  PPMEND (real) (“ppm end”) the lower limit of the ppm-range; i.e., the right edge of
    //  the Analysis Window (with the usual convention of the ppm-axis decreasing
    //  toward the right).
    //  Defaults: See Sec 3.4.1.
    //  ===========================================================
    float ppmEnd = 1.8;    
 
    //  ===========================================================
    //  PPMST (real) (“ppm start”) the upper limit of the ppm-range; i.e., the left edge
    //  of the Analysis Window.
    //  Default: PPMST = 4.0
    //  ===========================================================
    float ppmStart = 4.1;  


    //  ===========================================================
    //  The .PS and .TABLE files contain the essential graphic and numeric information from
    //  the analysis. The .CSV file contains the concentrations and can be imported to
    //  spreadsheet programs. With LCMgui, you can select any of these to be output using
    //  Fig 7.3.
    //  5.3.7.1 .TABLE File
    //  The .TABLE file contains all the information in the tables of the One-Page Output
    //  as a self-documented text (ASCII) file. You can archive these files (particularly if
    //  you have assigned them unique pathnames with FILTAB) into a database (e.g., for
    //  computing means or other statistics over a group of spectra). For this, you must
    //  input the following:
    //  FILTAB (character*255) (“table file”) the pathname of the .TABLE file to be
    //  output. (e.g., ’/home/user1/.lcmodel/saved/pat 777 77/table’).
    //  LTABLE (integer)
    //  LTABLE = 0 (the default) will suppress creation of this file;
    //  LTABLE = 7 will make this file.
    //  ===========================================================
    int lTable = 7;  

    //  ===========================================================
    //  NEACH (integer) the number of metabolites for which individual plots are to be
    //  made.
    //  Default: NEACH = 0
    //  ===========================================================
    int nEach = 30;  
    
    //  ===========================================================
    //  NUSE1 (integer) the number of Basis Spectra that are to be used in the Preliminary
    //  Analysis.
    //  Default: NUSE1 = 5.
    //  ===========================================================
    int nUse1 = 5;  

    //  ===========================================================
    //  CHUSE1 (character(300)*6) the first NUSE1 elements contain the Metabolite Names
    //  of the spectra to be used. Default: CHUSE1 = ’NAA’, ’Cr’, ’GPC’, ’Glu’,
    //  ’Ins’.
    //  ===========================================================
    string chuse1 = "'NAA','GPC','Cr','mI','Glu'";

    //  ===========================================================
    //  NSIMUL (integer) is the number of Basis Spectra that you will simulate.
    //  Default: NSIMUL = 13.
    //  ===========================================================
    int nSimul = 13;  
   
    //  ===========================================================
    //  Normally all Basis Spectra in the .BASIS file are used by LCModel. If some of these
    //  are very unlikely to be detectable in your spectrum, then you should exclude these
    //  from this analysis, as follows:
    //  ===========================================================
    
    //  ===========================================================
    //  NOMIT (integer) the number of metabolites to be excluded. You can exclude
    //  Basis Spectra in the .BASIS file, as well as simulated spectra.
    //  Default: NOMIT = 0.
    //  ===========================================================
    int noMit = 9;  
    
    //  ===========================================================
    //  CHOMIT (character(100)*6) the first NOMIT elements contain the Metabolite
    //  Names of the spectra to be excluded.
    //  ===========================================================
    vector < string > chomitArray;   
    chomitArray.push_back( "'Lip13a'" ); 
    chomitArray.push_back( "'Lip13b'" ); 
    chomitArray.push_back( "'Lip13c'" ); 
    chomitArray.push_back( "'Lip13d'" ); 
    chomitArray.push_back( "'Lip09'" ); 
    chomitArray.push_back( "'MM09'" ); 
    chomitArray.push_back( "'MM12'" ); 
    chomitArray.push_back( "'MM14'" ); 
    chomitArray.push_back( "'MM17'" ); 
    
    //  ===========================================================
    //  NKEEP (integer) the number of metabolites that are to be kept in the current
    //  analysis, regardless of the ppm-range of the Analysis Window. You can
    //  include Basis Spectra in the .BASIS file, as well as simulated spectra.
    //  Default: NKEEP = 0.
    //  ===========================================================
    int nKeep = 2;  

    vector < string > chKeepArray;   
    chKeepArray.push_back( "'Lip20'" );  
    chKeepArray.push_back( "'MM20'" );  

    //  ===========================================================
    //  ECHOT (real) The echo time (in ms). If ECHOT ≥ 100.0, then the default becomes
    //  NUSE1 = 4, because of the weak Ins at long T E. However, you can override this
    //  by simply inputting your own value for NUSE1.
    //  ===========================================================
    string fileRoot = svkImageReader2::GetFileNameWithoutPath( fileRootName.c_str() ); 

    string title = "'" + fileRoot + "'";
    
    //  ===========================================================
    //  FILRAW (character*255) the pathname of the .RAW file to be input.
    //  ===========================================================
    string fileRaw = "'" + rawRootName + ".raw'";  

    //  ===========================================================
    //  FILBAS (character*255) the pathname of the .BASIS file to be input.
    //  Note: On most platforms, you cannot use symbols like ∼ or $HOME to
    //  denote your home directory with any Control Parameters.
    //  ===========================================================
    string fileBas = "'" + this->basisFileName + "'"; 

    //  ===========================================================
    //  FILPS (character*255) the pathname of the .PS file that will contain the
    //  PostScript One-Page Output.
    //  ===========================================================
    string filePS  = "'" + fileRootName + indexSuffix + ".ps'";  
    string fileCSV = "'" + fileRootName + indexSuffix + ".csv'";  
    string fileCOO = "'" + fileRootName + indexSuffix + ".coord'";  
    string fileTAB = "'" + fileRootName + indexSuffix + ".table'";  

    //  ===========================================================
    //  ===========================================================
    //   $LCMODL
    //  ===========================================================
    //  ===========================================================
    *out << " $LCMODL" << endl;
    *out << " PPMST     = " << fixed << setprecision(1) << ppmStart             << endl;
    *out << " PPMEND    = " << fixed << setprecision(1) << ppmEnd               << endl;
    *out << " HZPPPM    = " << fixed << setprecision(4) << transmitterFrequency << endl;
    *out << " NUNFIL    = " << fixed << setprecision(0) << numSpecPts           << endl;
    *out << " DELTAT    = " << fixed << setprecision(6) << dwellTimeSecs        << endl;
    //  =====================================================
    *out << " TITLE     = "   << title << endl;
    *out << " LPS       = 8"  << endl;
    *out << " LCSV      = 11" << endl;
    *out << " LCOORD    = 9"  << endl;
    *out << " FILBAS    = "   << fileBas << endl;
    *out << " LTABLE    = "   << lTable  << endl;
    *out << " FILTAB    = "   << fileTAB << endl;   
    *out << " FILRAW    = "   << fileRaw << endl;
    *out << " FILPS     = "   << filePS  << endl;
    *out << " FILCSV    = "   << fileCSV << endl;
    *out << " FILCOO    = "   << fileCOO << endl;
    *out << " NEACH     = "   << nEach    << endl;
    *out << " NUSE1     = "   << nUse1    << endl;
    *out << " CHUSE1    = "   << chuse1   << endl;
    *out << " NSIMUL    = "   << nSimul   << endl;
    *out << " NOMIT     = "   << noMit    << endl;
    for ( int i = 0; i < chomitArray.size(); i++ ) { 
        *out << " CHOMIT(" << i + 1 << ") = " << chomitArray[i] << endl;
    }
    *out << " NKEEP     = "   << nKeep << endl;
    for ( int i = 0; i < chKeepArray.size(); i++ ) { 
        *out << " CHKEEP(" << i + 1 << ") = " << chKeepArray[i] << endl;
    }
    //  =====================================================
    *out << " ISLICE    = "   << currentSlice << endl;
    *out << " NDSLIC    = "   << setprecision(0) << numSlices     <<  endl;
    *out << " NDROWS    = "   << setprecision(0) << numRows       <<  endl;
    *out << " NDCOLS    = "   << setprecision(0) << numCols       <<  endl;
    *out << " ICOLST    = "   << setprecision(0) << colStart      <<  endl;
    *out << " ICOLEN    = "   << setprecision(0) << colEnd        <<  endl;
    *out << " IROWST    = "   << setprecision(0) << rowStart      <<  endl;
    *out << " IROWEN    = "   << setprecision(0) << rowEnd        <<  endl;
    *out << " NVOXSK    = "   << setprecision(0) << numVoxSkipped <<  endl;
    *out << " $END" << endl;
    

}


/*!
 *  initializes the ofstream header content for the LCModel Raw header.
 *
 *      $SEQPAR
 *      ECHOT=30
 *      HZPPPM=298.064
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
    string fmtDat = "'(f18.3)'"; 
    //string fmtDat = "'(2f18.3)'"; 
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
    *out << " HZPPPM=" << setprecision(3) << transmitterFrequency <<  endl;
    *out << " SEQ=" << sequenceName <<  endl;
    *out << " $END" << endl;

    *out << " $NMID"<<  endl;
    *out << " ID='" << id <<  "'" << endl;
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


void svkLCModelRawWriter::UseSingleRawFile( bool useSingleRawFile ) 
{
    this->singleRawFile = useSingleRawFile;
}


/*!
 *
 */
svkImageData* svkLCModelRawWriter::GetImageDataInput(int port)
{
    return svkImageData::SafeDownCast( this->GetInput(port) );
}


