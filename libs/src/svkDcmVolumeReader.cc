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


#include <svkDcmVolumeReader.h>
#include </usr/include/vtk/vtkDebugLeaks.h>
#include </usr/include/vtk/vtkGlobFileNames.h>
#include </usr/include/vtk/vtkSortFileNames.h>
#include </usr/include/vtk/vtkStringArray.h>


#include <svkMriImageData.h>


#include <sys/stat.h>
#include <cmath>


using namespace svk;


//vtkCxxRevisionMacro(svkDcmVolumeReader, "$Rev$");


/*!
 *
 */
svkDcmVolumeReader::svkDcmVolumeReader()
{
#if VTK_DEBUG_ON
    this->DebugOn();
    vtkDebugLeaks::ConstructClass("svkDcmVolumeReader");
#endif
    vtkDebugMacro(<<this->GetClassName() << "::" << this->GetClassName() << "()");
    this->tmpFileNames = NULL;
    this->numVolumes = 1;
}


/*!
 *
 */
svkDcmVolumeReader::~svkDcmVolumeReader()
{
    vtkDebugMacro(<<this->GetClassName() << "::~" << this->GetClassName() << "()");
}


/*!
 *
 */
svkDcmVolumeReader::ProprietarySOP svkDcmVolumeReader::ContainsProprietaryContent( svkImageData* data )
{

    // Check if it contains GE spectroscopy content:
    ProprietarySOP proprietaryContent = svkDcmVolumeReader::DICOM_STD_SOP;

    if ( data->GetDcmHeader()->ElementExists("Manufacturer") == true && data->GetDcmHeader()->ElementExists("ImagedNucleus") == true ) {
        string mfg = data->GetDcmHeader()->GetStringValue( "Manufacturer" ) ;
        string imagedNucleus = data->GetDcmHeader()->GetStringValue( "ImagedNucleus" ) ;

        string gePSSeq1 = ""; 
        if( data->GetDcmHeader()->ElementExists( "GE_PS_SEQ_1" ) ) {
            gePSSeq1 = data->GetDcmHeader()->GetStringValue( "GE_PS_SEQ_1" ) ;
        }
        string gePSSeq2 = ""; 
        if( data->GetDcmHeader()->ElementExists( "GE_PS_SEQ_2" ) ) {
            gePSSeq2 = data->GetDcmHeader()->GetStringValue( "GE_PS_SEQ_2" ) ;
        }

        if ( mfg == "GE MEDICAL SYSTEMS" ) {
            if ( imagedNucleus == "SPECT" || gePSSeq1 == "PROBE-P" || gePSSeq2 == "presscsi"  ) {
                proprietaryContent = svkDcmVolumeReader::GE_POSTAGE_STAMP_SOP;
            }
        }

        if ( mfg.find("Bruker") != string::npos ) {
            string seqName = data->GetDcmHeader()->GetStringValue( "SequenceName" ) ;
            if ( seqName.find("CSI") != string::npos ) {
                proprietaryContent = svkDcmVolumeReader::BRUKER_MRS_SOP;
            } 
        }
    }

    return proprietaryContent;
}


/*!
 *  Side effect of Update() method.  Used to load pixel data and initialize vtkImageData
 *  Called before ExecuteData()
 */
void svkDcmVolumeReader::ExecuteInformation()
{

    vtkDebugMacro( << this->GetClassName() << "::ExecuteInformation()" );

    if (this->FileName == NULL) {
        return;
    }

    if ( this->FileName ) {

        struct stat fs;
        if ( stat(this->FileName, &fs) ) {
            vtkErrorMacro("Unable to open file " << this->FileName );
            return;
        }
        this->InitDcmHeader();
        this->InitSliceOrder();

        double dcos[3][3];

        this->GetOutput()->GetDcmHeader()->GetDataDcos( dcos );
        this->GetOutput()->SetDcos(dcos);

        //  SetNumberOfIncrements is supposed to call this, but only works if the data has already
        //  been allocated. but that requires the number of components to be specified.
//        this->GetOutput()->GetIncrements();
        this->SetupOutputInformation();

        //rewrite the DimensionIndexSequence if necessary:
        svkDcmHeader::DimensionVector vec = this->GetOutput()->GetDcmHeader()->GetDimensionIndexVector();
        if (this->GetDebug()) {
            svkDcmHeader::PrintDimensionIndexVector(&vec);
        }
        this->GetOutput()->GetDcmHeader()->Redimension(&vec); 
    }

    this->InitPrivateHeader();

    //  This is a workaround required since the vtkImageAlgo executive
    //  for the reder resets the Extent[5] value to the number of files
    //  which is not correct for 3D multislice volume files. So store
    //  the files in a temporary array until after ExecuteData has been
    //  called, then reset the array.
    if (this->FileNames != NULL)  {
        this->tmpFileNames = vtkStringArray::New();
        this->tmpFileNames->DeepCopy(this->FileNames);
        this->FileNames->Delete();
        this->FileNames = NULL;
    }

}


/*!
 *  Method to set the slice order in dcos
 */
void svkDcmVolumeReader::InitSliceOrder()
{
    this->numFrames = this->GetOutput()->GetDcmHeader()->GetIntValue( "NumberOfFrames");
    int numSlices = this->GetOutput()->GetDcmHeader()->GetNumberOfSlices();
    if (numSlices > 1) {
        double origin0[3];
        this->GetOutput()->GetDcmHeader()->GetOrigin(origin0, 0);
        double origin1[3];
        this->GetOutput()->GetDcmHeader()->GetOrigin(origin1, numSlices-1); // zero indexed!

        //  Determine whether the data is ordered with or against the slice normal direction.
        double normal[3];
        this->GetOutput()->GetDcmHeader()->GetNormalVector(normal);
   
        //  Get vector from first to last image and get the dot product of that vector with the normal:
        double dcosSliceOrder[3];
        for (int j = 0; j < 3; j++) {
            dcosSliceOrder[j] =  origin1[j] - origin0[j];
        }
   
        //  Use the scalar product to determine whether the data in the .cmplx
        //  file is ordered along the slice normal or antiparalle to it.
        vtkMath* math = vtkMath::New();
        if (math->Dot(normal, dcosSliceOrder) > 0 ) {
            this->dataSliceOrder = svkDcmHeader::INCREMENT_ALONG_POS_NORMAL;
        } else {
            this->dataSliceOrder = svkDcmHeader::INCREMENT_ALONG_NEG_NORMAL;
        }
        math->Delete();
    } else {
        this->dataSliceOrder = svkDcmHeader::INCREMENT_ALONG_POS_NORMAL;
    }

    this->GetOutput()->GetDcmHeader()->SetSliceOrder( this->dataSliceOrder );
}


/*!
 *  Method to set the slice order in dcos
 */
void svkDcmVolumeReader::InitSliceOrder(string fileStart, string fileEnd)
{

    if ( fileStart.compare(fileEnd) != 0 ) {

        double origin0[3];
        svkImageData* tmpImage0 = svkMriImageData::New();
        tmpImage0->GetDcmHeader()->ReadDcmFileHeaderOnly( fileEnd );
        tmpImage0->GetDcmHeader()->GetOrigin(origin0); // zero indexed!
        tmpImage0->Delete();

        double origin1[3];
        svkImageData* tmpImage1 = svkMriImageData::New();
        tmpImage1->GetDcmHeader()->ReadDcmFileHeaderOnly( fileEnd );
        tmpImage1->GetDcmHeader()->GetOrigin(origin1); // zero indexed!
        tmpImage1->Delete();

        //  Determine whether the data is ordered with or against the slice normal direction.
        double normal[3];
        this->GetOutput()->GetDcmHeader()->GetNormalVector(normal);
   
        //  Get vector from first to last image and get the dot product of that vector with the normal:
        double dcosSliceOrder[3];
        for (int j = 0; j < 3; j++) {
            dcosSliceOrder[j] =  origin1[j] - origin0[j];
        }
   
        //  Use the scalar product to determine whether the data in the .cmplx
        //  file is ordered along the slice normal or antiparalle to it.
        vtkMath* math = vtkMath::New();
        if (math->Dot(normal, dcosSliceOrder) > 0 ) {
            this->dataSliceOrder = svkDcmHeader::INCREMENT_ALONG_POS_NORMAL;
        } else {
            this->dataSliceOrder = svkDcmHeader::INCREMENT_ALONG_NEG_NORMAL;
        }

        math->Delete();

    } else {
        this->dataSliceOrder = svkDcmHeader::INCREMENT_ALONG_POS_NORMAL;
    }

    this->GetOutput()->GetDcmHeader()->SetSliceOrder( this->dataSliceOrder );

}



/*!
 *  Side effect of Update() method.  Used to load pixel data and initialize vtkImageData
 *  Called after ExecuteInformation()
 */
void svkDcmVolumeReader::ExecuteDataWithInformation(vtkDataObject* output, vtkInformation* outInfo)
{

    vtkDebugMacro( << this->GetClassName() << "::ExecuteData()" );

    if (this->tmpFileNames != NULL)  {
        this->FileNames = vtkStringArray::New();
        this->FileNames->DeepCopy(this->tmpFileNames);
        this->tmpFileNames->Delete();
        this->tmpFileNames = NULL;
    }


    svkImageData* data = svkImageData::SafeDownCast( this->AllocateOutputData(output, outInfo) );

    if ( this->FileName ) {

        struct stat fs;
        if ( stat(this->FileName, &fs) ) {
            vtkErrorMacro("Unable to open file " << this->FileName );
            return;
        }

    }

    this->LoadData(data);

}


/*!
 *  Parse through the DICOM images and determine which ones belong to the same series as the 
 *  specified input file instances (StudyInstanceUID, SeriesInstanceUID), then also verify that 
 *  each image has the  same orientation.  The orientation check is useful for cases when there
 *  are multiple orientations in the same series (e.g. 3 plane localizer).  Since there may be 
 *  slight diffs between the values in ImageOrientationPatient the check is performed to within
 *  a tolerance.  
 *       
 *  Currently only supports single volume data, i.e. only one image at each location 
 *  (ImagePositionPatient). 
 */
void svkDcmVolumeReader::InitFileNames()
{
    string dcmFileName( this->GetFileName() );
    string dcmFilePath( this->GetFilePath( this->GetFileName() ) );  

    //  Get all files in the directory of the specified input image file:
    vtkGlobFileNames* globFileNames = vtkGlobFileNames::New();

    vtkSortFileNames* sortFileNames = vtkSortFileNames::New();
    if ( this->readOneInputFile == true) {
        globFileNames->AddFileNames( this->GetFileName() ); 
    } else {
        globFileNames->AddFileNames( string( dcmFilePath + "/*").c_str() );
    }
    sortFileNames->SetInputFileNames( globFileNames->GetFileNames() );
    sortFileNames->NumericSortOn();
    sortFileNames->SkipDirectoriesOn();
    sortFileNames->Update();


    //  Get the reference file's SeriesInstanceUID, ImageOrientationPatient and slice normal.
    //  These are used for parsing the glob'd files.  
    svkImageData* tmp = svkMriImageData::New(); 
    tmp->GetDcmHeader()->ReadDcmFileHeaderOnly(  this->GetFileName() );
    string imageOrientationPatient( tmp->GetDcmHeader()->GetStringValue("ImageOrientationPatient"));
    string seriesInstanceUID( tmp->GetDcmHeader()->GetStringValue("SeriesInstanceUID"));
    double referenceNormal[3]; 
    tmp->GetDcmHeader()->GetNormalVector( referenceNormal );
    tmp->Delete();
    tmp = NULL;

    vtkStringArray* fileNames =  sortFileNames->GetFileNames();

    /*  
     *  1.  Select only DICOM images from glob.  Non-DICOM image parsing will cause problems
     *  2.  Ensure that the list of files all belong to the same series (have the same 
     *      SeriesInstanceUID) as the selected input image. 
     *  3.  Run some checks to ensure that the series comprises data from a single volume: 
     *          - all have same ImageOrientationPatient.
     *          - no duplicated ImagePositionPatient values
     *  4.  Sort the resulting list of files by location (ascending/descending). 
     */


    /*  
     *  create a container to hold file names and dcm attributes, so 
     *  DCM parsing is only done once. The vector contains the following fields: 
     *      0 => fileName, 
     *      1 => SeriesInstanceUID, 
     *      2 => ImageOrientationPatient, 
     *      3 => normalL
     *      4 => normalP
     *      5 => normalS
     *      6 => ImagePositionPatient, 
     *      7 => projectionOnNormal 
     *      8 => instanceNumber 
     *
     *  Note: the projection onto the normal of the input image orientation is 
     *  used for sorting slice order.
     */
    vector < vector< string > > dcmSeriesAttributes; 

    for (int i = 0; i < fileNames->GetNumberOfValues(); i++) {

        //
        //  1.  only include DICOM files 
        //
        if (  svkDcmHeader::IsFileDICOM( fileNames->GetValue(i) ) ) {

            if (this->GetDebug()) {
                cout << "FN: " << fileNames->GetValue(i) << endl;
            }

            vector< string > dcmFileAttributes;  
            if( tmp != NULL ) {
                tmp->Delete();
                tmp = NULL;
            }
            tmp = svkMriImageData::New(); 
            tmp->GetDcmHeader()->ReadDcmFileHeaderOnly( fileNames->GetValue(i) );
            dcmFileAttributes.push_back( fileNames->GetValue(i) ); 

            if( tmp->GetDcmHeader()->ElementExists( "SeriesInstanceUID", "top" ) ) {
                dcmFileAttributes.push_back( tmp->GetDcmHeader()->GetStringValue( "SeriesInstanceUID" ) );
            } else {
                dcmFileAttributes.push_back( "" ); 
            }

            if( tmp->GetDcmHeader()->ElementExists( "ImageOrientationPatient", "top" ) ) {
                dcmFileAttributes.push_back( tmp->GetDcmHeader()->GetStringValue( "ImageOrientationPatient" ) );

                //  Add the 3 components of the normal vector:
                double normal[3]; 
                tmp->GetDcmHeader()->GetNormalVector( normal );
                for (int n = 0; n < 3; n++) {
                    ostringstream oss;
                    oss << normal[n];
                    dcmFileAttributes.push_back( oss.str() ); 
                }

            } else {
                dcmFileAttributes.push_back( "" ); 

                //  Add the 3 components of the normal vector:
                dcmFileAttributes.push_back( "" ); 
                dcmFileAttributes.push_back( "" ); 
                dcmFileAttributes.push_back( "" ); 
            }

            if( tmp->GetDcmHeader()->ElementExists( "ImagePositionPatient", "top" ) ) {
                dcmFileAttributes.push_back( tmp->GetDcmHeader()->GetStringValue( "ImagePositionPatient" ) );
            } else {
                dcmFileAttributes.push_back( "" ); 
            }

            double position[3];   
            if( tmp->GetDcmHeader()->ElementExists( "ImagePositionPatient", "top" ) ) {
                for (int i = 0; i < 3; i++ ) {
                    string pos( tmp->GetDcmHeader()->GetStringValue("ImagePositionPatient", i));
                    std::istringstream positionInString(pos);
                    positionInString >> position[i];
                }
            } else {
                position[0] = -1; 
                position[1] = -1; 
                position[2] = -1; 
            }

            //  Project posLPS onto the normal vector through the
            //  origin of world coordinate space (this is the same
            //  reference for both data sets (e.g MRI and MRS).
            double projectionOnNormal = vtkMath::Dot( position, referenceNormal );
            ostringstream projectionOss;
            projectionOss << projectionOnNormal;
            dcmFileAttributes.push_back( projectionOss.str() ); 

            // Add the instance number to the attributes:
            dcmFileAttributes.push_back( tmp->GetDcmHeader()->GetStringValue( "InstanceNumber" ) );

            //  add the file attributes to the series attribute vector:
            dcmSeriesAttributes.push_back( dcmFileAttributes ); 

            if( i % 2 == 0 ) { // Only update progress every other iteration
				ostringstream progressStream;
				progressStream <<"Reading DICOM Header " << i << " of " <<  fileNames->GetNumberOfValues();
				this->SetProgressText( progressStream.str().c_str() );
				this->UpdateProgress( i/((double)fileNames->GetNumberOfValues() ) );
            }
        }
    }
	this->SetProgressText( "Done Reading DICOM Headers." );
	this->UpdateProgress( 1 );

    //  ======================================================================
    //  Now validate that the DICOM files comprise a single volumetric series
    //  wrt seriesUID and orientation.  Pop files from vector that do not belong: 
    //  ======================================================================
    vector < vector< string > >::iterator seriesIt; 
    seriesIt = dcmSeriesAttributes.begin(); 

    while ( seriesIt != dcmSeriesAttributes.end() ) { 

        string tmpSeriesInstanceUID = (*seriesIt)[1]; 

        //  if seriesUID doesnt match, remove it:
        if ( tmpSeriesInstanceUID != seriesInstanceUID ) {
            vtkWarningWithObjectMacro(
                this, 
                "SeriesInstanceUID is not the same for all slices in directory, removing file from series."
            );
            seriesIt = dcmSeriesAttributes.erase( seriesIt ); 
        } else {
            seriesIt++; 
        }
    }

    seriesIt = dcmSeriesAttributes.begin(); 
    while ( seriesIt != dcmSeriesAttributes.end() ) { 

        bool isOrientationOK = true; 

        string tmpImageOrientationPatient = (*seriesIt)[2]; 

        //  Check the orientation.  If the orientation differs check by how much, permiting a small 
        //  variance to within a tolerance: 
        if ( tmpImageOrientationPatient != imageOrientationPatient ) {

            // is the difference within a small tolerance? compare normal vectors:

            double normal[3];            
            for (int n = 0; n < 3; n++) {
                istringstream* normalString = new istringstream();
                normalString->str( (*seriesIt)[n + 3] ); 
                *normalString >> normal[n];
                delete normalString;
            }
            //  If the normals are the same the dot product should be 1.  
            //  Permit a small variance:
            double dotNormals = vtkMath::Dot( normal, referenceNormal );
            if ( dotNormals < .9999) {
                isOrientationOK = false; 
                //cout << "DOT NORMAL: " << dotNormals << endl;
            }
        }
        if ( isOrientationOK ) {
            seriesIt++; 
        } else {
            //  Orientations don't match, set series to one input file
            vtkWarningWithObjectMacro(this, "ImageOrientationPatient is not the same for all slices, removing file from series");
            seriesIt = dcmSeriesAttributes.erase( seriesIt ); 
        }
    }

    //  ============================
    //  Is Data multi-volumetric (multiple 
    //  instances of same ImagePositionPatient):
    //  ============================
    set < string > uniqueSlices;
    seriesIt = dcmSeriesAttributes.begin(); 
    while ( seriesIt != dcmSeriesAttributes.end() ) { 
        uniqueSlices.insert( (*seriesIt)[6] ); 
        seriesIt++ ;
    }                        

    this->CleanAttributes( &uniqueSlices ); 

    //  there is one entry in the dcmSeriesAttributes vector for each DICOM file:
    dcmSeriesAttributes.size(); 
    int numDcmFiles = dcmSeriesAttributes.size();
    this->numVolumes = numDcmFiles  / uniqueSlices.size();
    if ( numDcmFiles % uniqueSlices.size() != 0 ) {
        //  different number of slices for each volume:
        vtkWarningWithObjectMacro(this, "different number of slices in volumes"); 
        this->OnlyReadInputFile(); 
        return; 
    }

    if ( this->CheckForMultiVolume() ) {
        if ( dcmSeriesAttributes.size() > uniqueSlices.size() ) {
            //  More than one file per slice:  
            vtkWarningWithObjectMacro(this, "Multi-volumetric data, repeated slice locations"); 
            this->OnlyReadInputFile(); 
            return; 
        }
    }

    //  ======================================================================
    //  Now sort the files according to ImagePositionPatient.
    //  ======================================================================
    this->SortFilesByImagePositionPatient( dcmSeriesAttributes, false); 

    //  ======================================================================
    //  And then by InstanceNumber
    //  outter loop should be position, inner should be volume: 
    //      for (pos = 0; pos < numPositions; pos++ ) { 
    //          for (vol= 0; vol < numVolumes; vol++)  { 
    //  i.e. all images from a given slice are consecutive
    //  ======================================================================
    this->SortFilesByInstanceNumber( dcmSeriesAttributes, uniqueSlices.size(), true); 

    // Finally, repopulate the file list.
    fileNames->SetNumberOfValues( dcmSeriesAttributes.size() );
    for ( int i = 0; i < dcmSeriesAttributes.size(); i++ ) { 
        if (this->GetDebug()) {
            cout << "FNS in series: " << dcmSeriesAttributes[i][0] << endl; 
        }
        fileNames->SetValue( i, dcmSeriesAttributes[i][0] ); 
    }

    // Calling this method will set the DataExtents for the slice direction
    //  should set these from the sorted dcmSeriesAttributes: 
    this->SetFileNames( sortFileNames->GetFileNames() );

    //  Set Pixel Spacing, which can't necessarily be determined from data in a single DICOM object, for example if 
    //  the slice thickness differs from distance between samples.
    this->SetSliceSpacing( tmp->GetDcmHeader(), uniqueSlices.size(), dcmSeriesAttributes );

    if( tmp != NULL ) {
        tmp->Delete();
        tmp = NULL;
    }
    globFileNames->Delete();
    sortFileNames->Delete();

    if (  this->onlyGlobFiles == true ) {
        for (int i = 0; i < this->GetFileNames()->GetNumberOfValues(); i++) {
            cout << "FN: " << this->GetFileNames()->GetValue(i) << endl;
        }
        cout << "Just Glob Files, exiting now" << endl;
        exit(0); 
    }

} 


/*
 *  Sets the slice spacing.  If multi slice, from actual spacing between slices, or for single slice, from 
 *  DCM SliceThickness attribute. 
 */ 
void svkDcmVolumeReader::SetSliceSpacing( svkDcmHeader* hdr, int numSlices, vector< vector< string> >& dcmSeriesAttributes )
{
    if ( dcmSeriesAttributes.size() > 1 ) {
        int index1 = 1;
        if (this->numVolumes > 1 && numSlices > 1) {
            index1 = 1 * this->numVolumes; 
        } 
        this->sliceSpacing = abs( 
            svkDcmVolumeReader::GetFloatValAttribute7( dcmSeriesAttributes[ index1 ] ) - svkDcmVolumeReader::GetFloatValAttribute7( dcmSeriesAttributes[ 0 ]) 
        ); 
    } else {
        //  doing this to take advantage of the searchInto option for the double version 
        this->sliceSpacing = static_cast<float>( hdr->GetDoubleValue("SliceThickness", true) ); 
    }
}


/*
 *  Gets the slice spacing.  If multi slice, from actual spacing between slices, or for single slice, from 
 *  DCM SliceThickness attribute. 
 */ 
float svkDcmVolumeReader::GetSliceSpacing() 
{
    return this->sliceSpacing; 
}


/*!
 *  Scales an array of unsigned short pixel values into to a floating point
 *  array according to the given center and window. The minimum value of the
 *  output will be the center - window/2 and the max will be center + window/2.
 */
void svkDcmVolumeReader::GetVOILUTScaledPixels( float* floatPixels, unsigned short* shortPixels, float center, float window, int numberOfValues )
{

	float inputRangeMin = VTK_UNSIGNED_SHORT_MAX;
	float inputRangeMax = VTK_UNSIGNED_SHORT_MIN;

	// Get the min/max of the input array
	for( int i = 0; i < numberOfValues; i++ ) {
		if( shortPixels[i] < inputRangeMin ) {
			inputRangeMin = shortPixels[i];
		}
		if( shortPixels[i] > inputRangeMax ) {
			inputRangeMax = shortPixels[i];
		}

	}

	float deltaRangeIn = inputRangeMax - inputRangeMin;
	float outputRangeMin = center - window / 2.0;
	float outputRangeMax = center + window / 2.0;
	float deltaRangeOut = outputRangeMax - outputRangeMin;
	float slope = 0;
    if ( deltaRangeIn != 0 ) {
	    slope = deltaRangeOut/deltaRangeIn;
    }
	float intercept = outputRangeMin - inputRangeMin * ( slope );

	for( int i = 0; i < numberOfValues; i++ ) {
		floatPixels[i] = slope*shortPixels[i] + intercept;
	}
}



/*
 *  If true, then make sure only one data volume
 *  is present in file list
 */
bool svkDcmVolumeReader::CheckForMultiVolume() {
    return true; 
}


/*!
 *  Reset file names to only the single input file name
 */
void svkDcmVolumeReader::OnlyReadInputFile()
{
    vtkStringArray* tmpFileNames = vtkStringArray::New();
    tmpFileNames->SetNumberOfValues(1);
    tmpFileNames->SetValue(0, this->GetFileName() );

    // Calling this method will set the DataExtents for the slice direction
    this->SetFileNames( tmpFileNames );

    tmpFileNames->Delete();
}


/*!
 *  sort compare utility method.  Gets the 7th element of the vectors 
 *  being compared.
 */
float svkDcmVolumeReader::GetFloatValAttribute7( vector< string > vec ) 
{
    istringstream* posString = new istringstream();

    //  7th element of attributes vector is the projection of the 
    //  ImagePositionPatient on the normal through the origin
    posString->str( vec[7] );    
    float floatPosition; 
    *posString >> floatPosition; 

    delete posString; 

    return floatPosition; 
}


/*!
 *  sort compare utility method.  Gets the 8th element of the vectors 
 *  being compared.
 */
int svkDcmVolumeReader::GetIntValAttribute8( vector< string > vec ) 
{
    istringstream* instanceString = new istringstream();

    //  8th element of attributes vector is the instance number 
    instanceString->str( vec[8] );    
    int instanceNumber; 
    *instanceString >> instanceNumber; 

    delete instanceString; 

    return instanceNumber; 
}


//  sort ascending slice order
bool SortAscendAttribute7( vector< string > first, vector < string> second )
{
    return svkDcmVolumeReader::GetFloatValAttribute7( first ) < svkDcmVolumeReader::GetFloatValAttribute7( second );  
}


//  sort descending slice order
bool SortDescendAttribute7( vector< string > first, vector < string> second )
{
    return svkDcmVolumeReader::GetFloatValAttribute7( first ) > svkDcmVolumeReader::GetFloatValAttribute7( second );  
}


//  sort ascending slice order
bool SortAscendAttribute8( vector< string > first, vector < string> second )
{
    return svkDcmVolumeReader::GetIntValAttribute8( first ) < svkDcmVolumeReader::GetIntValAttribute8( second );  
}


//  sort descending slice order
bool SortDescendAttribute8( vector< string > first, vector < string> second )
{
    return svkDcmVolumeReader::GetIntValAttribute8( first ) > svkDcmVolumeReader::GetIntValAttribute8( second );  
}


/*!
 *  Sort the list of files in either ascending or descending order by ImagePositionPatient
 */
void svkDcmVolumeReader::SortFilesByImagePositionPatient(
        vector< vector< string> >& dcmSeriesAttributes, 
        bool ascending
    )
{

    if ( ascending ) {
        std::sort( dcmSeriesAttributes.begin(), dcmSeriesAttributes.end(), SortAscendAttribute7 ); 
    } else {
        std::sort( dcmSeriesAttributes.begin(), dcmSeriesAttributes.end(), SortDescendAttribute7 ); 
    }

    return;
}


/*!
 *  Sort the list of files in either ascending or descending order by InstanceNumber 
 *  thie input is already sorted by ImagePositionPatient   
 */
void svkDcmVolumeReader::SortFilesByInstanceNumber(
        vector< vector< string> >& dcmSeriesAttributes, 
        int numSlicesPerVol, 
        bool ascending
    )
{


    int counter = -1; 

    //  extract all attributes from corresponding to a single slice
    for ( int slice = 0; slice < numSlicesPerVol; slice++ ) {


        //  Put all instances of this slice into a new vector and sort it 
        vector< vector< string> > dcmSliceAttributes;
        for ( int vol = 0; vol < this->numVolumes; vol++ ) {
            counter++; 
            dcmSliceAttributes.push_back( dcmSeriesAttributes[counter] );
        }

        if ( ascending ) {
            std::sort( dcmSliceAttributes.begin(), dcmSliceAttributes.end(), SortAscendAttribute8 ); 
        } else {
            std::sort( dcmSliceAttributes.begin(), dcmSliceAttributes.end(), SortDescendAttribute8 ); 
        }

        //  Not put the images back in the correc order:
        counter = counter - this->numVolumes; 
        for ( int vol = 0; vol < this->numVolumes; vol++ ) {
            counter++; 
            dcmSeriesAttributes[counter] = dcmSliceAttributes[vol]; 
        }

    }

    return;
}




/*!
 *
 */
void svkDcmVolumeReader::InitDcmHeader()
{
    this->GetOutput()->GetDcmHeader()->ReadDcmFile( this->FileName );
}


