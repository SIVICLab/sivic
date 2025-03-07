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



#include <svkMrsImageFFT.h>


using namespace svk;


//vtkCxxRevisionMacro(svkMrsImageFFT, "$Rev$");
vtkStandardNewMacro(svkMrsImageFFT);


svkMrsImageFFT::svkMrsImageFFT()
{

#if VTK_DEBUG_ON
    this->DebugOn();
#endif

    vtkDebugMacro(<<this->GetClassName() << "::" << this->GetClassName() << "()");

    //  Start index for update extent
    this->updateExtent[0] = -1;  
    this->updateExtent[1] = -1;  
    this->updateExtent[2] = -1;  
    this->updateExtent[3] = -1;  
    this->updateExtent[4] = -1;  
    this->updateExtent[5] = -1;  
    this->domain = SPECTRAL;
    this->mode   = FORWARD;
    this->preCorrectCenter = false;
    this->postCorrectCenter = false;
    this->voxelShift[0] = 0;
    this->voxelShift[1] = 0;
    this->voxelShift[2] = 0;
    this->onlyUseSelectionBox = false;
    this->selectionBoxMask = NULL;
    this->normalizeTransform = false;
}


svkMrsImageFFT::~svkMrsImageFFT()
{
    if ( this->selectionBoxMask != NULL ) { 
        delete [] this->selectionBoxMask;
        this->selectionBoxMask = NULL;
    }
}


void svkMrsImageFFT::NormalizeTransform()
{
    this->normalizeTransform = true;
}



/*!
 *  Method for converting between vtkDataArrays and vtkImageComplex.
 */
void svkMrsImageFFT::ConvertArrayToImageComplex( vtkDataArray* spectrum, vtkImageComplex* imageComplexSpectrum ) 
{
    int numTuples = spectrum->GetNumberOfTuples();
    for( int i = 0; i < spectrum->GetNumberOfTuples(); i++ ) {
        double tuple[2];
        spectrum->GetTuple( i, tuple );
        imageComplexSpectrum[i].Real = tuple[0]; 
        imageComplexSpectrum[i].Imag = tuple[1]; 
    }
}


/*! 
 *
 */
int svkMrsImageFFT::RequestInformation( vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector )
{

    int* wholeExtent = this->GetOutputInformation(0)->Get(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT());
    
    bool useWholeExtent = false; 
    //  If the specified update extent is outside the whole extent, just use
    //  the whole extent:

    //  Lower bounds of extent:
    for (int i = 0; i < 6; i+=2) {
        if ( updateExtent[i] < wholeExtent[i] ) {
            useWholeExtent = true;
        }
    }

    //  upper bounds of extent:
    for (int i = 1; i < 6; i+=2) {
        if ( updateExtent[i] > wholeExtent[i] ) {
            useWholeExtent = true;
        }
    }

    if (useWholeExtent) {
        wholeExtent[1] =  wholeExtent[1] - 1;  
        wholeExtent[3] =  wholeExtent[3] - 1;  
        wholeExtent[5] =  wholeExtent[5] - 1;  
        for (int i = 0; i < 6; i++) {
            vtkDebugMacro(<<this->GetClassName() << " Whole Extent " << wholeExtent[i]);
            this->updateExtent[i] = wholeExtent[i];  
        }
    }
    return 1;
}

/*! 
 *
 */
int svkMrsImageFFT::RequestData( vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector )
{
    int returnValue;
    vtkImageFourierFilter* fourierFilter = NULL;

    //  Validate request: 
    this->ValidateRequest(); 

    if( this->mode == REVERSE ) {
        fourierFilter = vtkImageRFFT::New();
    } else {
        fourierFilter = vtkImageFFT::New();
    }
    if( this->domain == SPECTRAL ) {
        fourierFilter->SetDimensionality(1);
        returnValue = RequestDataSpectral( request, inputVector, outputVector, fourierFilter ); 
    } else if ( this->domain == SPATIAL ) {
        fourierFilter->SetDimensionality(3);
        returnValue = RequestDataSpatial( request, inputVector, outputVector, fourierFilter ); 
    }
    fourierFilter->Delete();
    return returnValue;
}


/*! 
 *
 */
int svkMrsImageFFT::RequestDataSpatial( vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector, vtkImageFourierFilter* fourierFilter )
{

    svkMrsImageData* data = svkMrsImageData::SafeDownCast(this->GetImageDataInput(0));

    //  First check to see if the transform is required. If not just return: 
    string domainCol = data->GetDcmHeader()->GetStringValue( "SVK_ColumnsDomain");
    string domainRow = data->GetDcmHeader()->GetStringValue( "SVK_ColumnsDomain");
    string domainSlice = data->GetDcmHeader()->GetStringValue( "SVK_ColumnsDomain"); 
    double kZeroShiftWindow[3] = {0,0,0};
    if( this->mode == REVERSE ) {
        // REVERSE should take to space domain, so if the data is already in space then skip the FT 
        if (  ( domainCol.compare("SPACE")   == 0 ) 
           || ( domainRow.compare("SPACE")   == 0 ) 
           || ( domainSlice.compare("SPACE") == 0 ) 
        ){
            cout << "svkMrsImageFFT: Already in target domain, not transforming " << endl; 
            return 1; 
        }; 
        //  if K0 not sampled, e.g. even number of samples, even symmetry then apply half voxel shift.  
        string k0Sampled = data->GetDcmHeader()->GetStringValue( "SVK_K0Sampled");
        if ( k0Sampled.compare( "NO" ) == 0 ) { 
            for( int i = 0; i < 3; i++ ) {
                kZeroShiftWindow[i] = -0.5;
            }
        }
    } else {
        // FORWARD should take to kspace domain, so if the data is already in kspace then skip the FT 
        if (  ( domainCol.compare("KSPACE")   == 0 )  
           || ( domainRow.compare("KSPACE")   == 0 )  
           || ( domainSlice.compare("KSPACE") == 0 )
        ){
            cout << "svkMrsImageFFT: Already in target domain, not transforming " << endl; 
            return 1; 
        }; 
    }

    //  If single voxel do nothing: 
    svkDcmHeader::DimensionVector dimensionVector = data->GetDcmHeader()->GetDimensionIndexVector();
    int numVoxels = 1;
    for ( int dim = 0; dim < 3; dim++) {
        int dimSize = svkDcmHeader::GetDimensionVectorValue ( &dimensionVector, dim ) + 1;
        numVoxels *= dimSize;
    }
    if ( numVoxels == 1 ) {
        //  Update the DICOM header to reflect the spatial domain changes:
        if( this->mode == REVERSE ) {
            data->GetDcmHeader()->SetValue( "SVK_ColumnsDomain", "SPACE" );
            data->GetDcmHeader()->SetValue( "SVK_RowsDomain",    "SPACE" );
            data->GetDcmHeader()->SetValue( "SVK_SliceDomain",   "SPACE" );
            data->GetDcmHeader()->RemoveElement("SVK_K0Sampled"); 
        } else {
            data->GetDcmHeader()->SetValue( "SVK_ColumnsDomain", "KSPACE" );
            data->GetDcmHeader()->SetValue( "SVK_RowsDomain",    "KSPACE" );
            data->GetDcmHeader()->SetValue( "SVK_SliceDomain",   "KSPACE" );
            data->GetDcmHeader()->SetValue( "SVK_K0Sampled",    "YES");
        }
        return 1; 
    }


    vtkImageData* currentData = NULL;

    svkImageLinearPhase* preKZeroShifter = svkImageLinearPhase::New();
    svkImageLinearPhase* postKZeroShifter = svkImageLinearPhase::New();
    svkImageLinearPhase* voxelShifter = svkImageLinearPhase::New();

    svkImageFourierCenter* preIfc = svkImageFourierCenter::New(); 
    svkImageFourierCenter* postIfc = svkImageFourierCenter::New();
    double progress = 0;

    int numberOfPoints = data->GetCellData()->GetArray(0)->GetNumberOfTuples();
    int numChannels  = data->GetDcmHeader()->GetNumberOfCoils();
    int numTimePoints  = data->GetDcmHeader()->GetNumberOfTimePoints();

    //  Get the Dimension Index and index values  
    //  GetNumber of cells in the image:

    //  Only loop over non spatial voxels: 
    svkDcmHeader::SetDimensionVectorValue(&dimensionVector, svkDcmHeader::COL_INDEX,   0);
    svkDcmHeader::SetDimensionVectorValue(&dimensionVector, svkDcmHeader::ROW_INDEX,   0);
    svkDcmHeader::SetDimensionVectorValue(&dimensionVector, svkDcmHeader::SLICE_INDEX, 0);
    svkDcmHeader::DimensionVector loopVector = dimensionVector;
    int numCells = svkDcmHeader::GetNumberOfCells( &dimensionVector );

    svkMriImageData* pointImage = svkMriImageData::New();
    int lastPrint = -1; 
    for (int cellID = 0; cellID < numCells; cellID++ ) {

        //  Get the dimensionVector index for current cell -> loopVector: 
        //  since dimensionVector is spatiall all zeros, this is initialized to the non -spatial 
        //  volumes
        svkDcmHeader::GetDimensionVectorIndexFromCellID( &dimensionVector, &loopVector, cellID );

        ostringstream progressStream;
        progressStream <<"Executing Spatial Recon for cell " << cellID + 1 << "/" << "/" << numCells;
        this->SetProgressText( progressStream.str().c_str() );

        int completed = 100 * ( static_cast<float>(cellID) / static_cast<float>(numCells) ); 
        //cout << completed << " " << cellID << " " << numCells << " " 
            //  << static_cast<float>(cellID) / static_cast<float>(numCells) << endl;
        if( completed % 5 == 0 && completed != lastPrint ) {
            cout << "spatial fft vols " << completed << " %" << endl;
            lastPrint = completed; 
        }

        for( int point = 0; point < numberOfPoints; point++ ) {
            if( point%16==0 ) {
                progress = (point+1)/((double)numberOfPoints);
                this->UpdateProgress( progress );
            }

            //  ImageFourierCenter required data type VTK_DOUBLE
            data->GetImage( pointImage, point, &loopVector, 2, "", VTK_DOUBLE);

            currentData = pointImage;

            // Lets apply a phase shift....
            if( kZeroShiftWindow[0] != 0
                && kZeroShiftWindow[1] != 0
                && kZeroShiftWindow[2] != 0 ) {
                if (this->GetDebug()) {
                    double pixelSpacing[3] = {0,0,0};
                    data->GetDcmHeader()->GetPixelSpacing( pixelSpacing ); 
                    cout << " SHIFT WINDOW(MRSFFT):          " << kZeroShiftWindow[0] 
                        << " " << kZeroShiftWindow[1] << " " << kZeroShiftWindow[2] << endl;
                    cout << " SHIFT WINDOW(MRSFFT) (PixSize):" << pixelSpacing[0] 
                        << " " << pixelSpacing[1] << " " << pixelSpacing[2] << endl;
                }
                preKZeroShifter->SetShiftWindow( kZeroShiftWindow );
                preKZeroShifter->SetInputData( currentData );
                preKZeroShifter->Update( );
                currentData = preKZeroShifter->GetOutput();
            }

            // Lets apply a voxel shift....
            if( (  this->voxelShift[0] != 0
                || this->voxelShift[1] != 0
                || this->voxelShift[2] != 0 )
                && this->mode == REVERSE ) {
                voxelShifter->SetShiftWindow( this->voxelShift );
                voxelShifter->SetInputData( currentData );
                voxelShifter->Update( );
                currentData = voxelShifter->GetOutput();
            }

            // And correct the center....
            if( this->preCorrectCenter ) {
                preIfc->SetReverseCenter( true );
                preIfc->SetInputData(currentData);
                preIfc->Update();
                currentData = preIfc->GetOutput();
            }

            // Do the Fourier Transform
            fourierFilter->SetInputData(currentData);
            currentData = fourierFilter->GetOutput();
            fourierFilter->Update();

            // And correct the center....
            if( this->postCorrectCenter ) {
                postIfc->SetInputData(currentData);
                postIfc->Update();
                currentData = postIfc->GetOutput();
            }


            if( kZeroShiftWindow[0] != 0
                && kZeroShiftWindow[1] != 0
                && kZeroShiftWindow[2] != 0 ) {
                postKZeroShifter->SetShiftWindow( kZeroShiftWindow );
                postKZeroShifter->SetInputData( currentData );
                postKZeroShifter->Update( );
                currentData = postKZeroShifter->GetOutput();
            }

            // Lets apply a voxel shift....
            if( (  this->voxelShift[0] != 0
                || this->voxelShift[1] != 0
                || this->voxelShift[2] != 0 )
                && this->mode == FORWARD ) {
                voxelShifter->SetShiftWindow( this->voxelShift );
                voxelShifter->SetInputData( currentData );
                voxelShifter->Update( );
                currentData = voxelShifter->GetOutput();
            }

            //  if normalize if specified: 
            if ( this->normalizeTransform == true) {
                vtkDataArray* metMapArray = currentData->GetPointData()->GetArray(0);
                for ( int v = 0; v < numVoxels; v++) {
                    double tuple[2];
                    currentData->GetPointData()->GetArray(0)->GetTuple(v, tuple);
                    tuple[0] *= numVoxels; 
                    tuple[1] *= numVoxels; 
                    currentData->GetPointData()->GetArray(0)->SetTuple(v, tuple);
                }
            }
            data->SetImage( currentData, point, &loopVector ); 
        }
    }
    if( preIfc != NULL ) {
        preIfc->Delete(); 
        preIfc = NULL; 
    }
    if( postIfc != NULL ) {
        postIfc->Delete(); 
        postIfc = NULL; 
    }
    if( preKZeroShifter != NULL ) {
        preKZeroShifter->Delete();
        preKZeroShifter = NULL;
    }
    if( postKZeroShifter != NULL ) {
        postKZeroShifter->Delete();
        postKZeroShifter = NULL;
    }
    if( voxelShifter != NULL ) {
        voxelShifter->Delete();
        voxelShifter = NULL;
    }
    pointImage->Delete();

    //  Update the DICOM header to reflect the spatial domain changes:
    if( this->mode == REVERSE ) {
        data->GetDcmHeader()->SetValue( "SVK_ColumnsDomain", "SPACE" );
        data->GetDcmHeader()->SetValue( "SVK_RowsDomain",    "SPACE" );
        data->GetDcmHeader()->SetValue( "SVK_SliceDomain",   "SPACE" );
        data->GetDcmHeader()->RemoveElement("SVK_K0Sampled"); 
    } else {
        data->GetDcmHeader()->SetValue( "SVK_ColumnsDomain", "KSPACE" );
        data->GetDcmHeader()->SetValue( "SVK_RowsDomain",    "KSPACE" );
        data->GetDcmHeader()->SetValue( "SVK_SliceDomain",   "KSPACE" );
        data->GetDcmHeader()->SetValue( "SVK_K0Sampled",    "YES");
    }

    //  Update Origin and Per Frame Functional Groups for voxel shift:
    this->UpdateOrigin();

    //  Trigger observer update via modified event:
    this->GetInput()->Modified();
    return 1; 
}


/*! 
 *  Given a fractional voxel shift (voxelShift) along each of the axes,
 *  calculate a new TOPLC and reset the PerFrameFunctionalGroups.
 */
void svkMrsImageFFT::UpdateOrigin() 
{

    svkImageData* data = this->GetImageDataInput(0);

    int numSlices = data->GetDcmHeader()->GetNumberOfSlices();
    int numTimePts = data->GetDcmHeader()->GetNumberOfTimePoints();
    int numCoils = data->GetDcmHeader()->GetNumberOfCoils();

    double dcos[3][3]={{0,0,0},{0,0,0},{0,0,0}}; 

    data->GetDcmHeader()->GetDataDcos( dcos ); 

    double pixelSpacing[3] = {0,0,0};
    data->GetDcmHeader()->GetPixelSpacing( pixelSpacing ); 

    double toplc[3] = {0,0,0}; 
    data->GetDcmHeader()->GetOrigin( toplc, 0 ); 

    //  Calculate new toplc
    for (int i = 0; i < 3; i++ ){
        for(int j = 0; j < 3; j++ ){
            toplc[i] -= ( dcos[j][i] * this->voxelShift[j] * pixelSpacing[j] );
        }
    }
    //cout << "Voxel Shifto: " << this->voxelShift[0] << " " << this->voxelShift[1] << " " << this->voxelShift[2]<< endl;
    //cout << "PSo        : " << pixelSpacing[0] << " " << pixelSpacing[1] << " " << pixelSpacing[2] << endl;
    //cout << "dcoso      : " << dcos[0][0] << " " << dcos[0][1] << " " << dcos[0][2] << endl;
    //cout << "dcoso      : " << dcos[1][0] << " " << dcos[1][1] << " " << dcos[1][2] << endl;
    //cout << "dcoso      : " << dcos[2][0] << " " << dcos[2][1] << " " << dcos[2][2] << endl;
    //cout << "TOPLC      : " << toplc[0] << " " << toplc[1] << " " << toplc[2]<< endl;

    svkDcmHeader::DimensionVector dimensionVector = data->GetDcmHeader()->GetDimensionIndexVector();
    svkDcmHeader::SetDimensionVectorValue(&dimensionVector, svkDcmHeader::SLICE_INDEX, numSlices-1);
    svkDcmHeader::SetDimensionVectorValue(&dimensionVector, svkDcmHeader::TIME_INDEX, numTimePts-1);
    svkDcmHeader::SetDimensionVectorValue(&dimensionVector, svkDcmHeader::CHANNEL_INDEX, numCoils-1);

    data->GetDcmHeader()->InitPerFrameFunctionalGroupSequence(
        toplc, pixelSpacing, dcos, &dimensionVector
    );

    //  Now displace by 1/2 voxel to get Cell corner for svkImageDataOrigin:
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            toplc[i] -= ( pixelSpacing[j]/2 ) * dcos[j][i];
        }
    }
    data->SetOrigin( toplc );
    double cen[3];
    data->GetImageCenter(cen);
    cout << "center?: " << cen[0] << " " << cen[1] << " " << cen[2]<< endl;
}


/*!
 *
 */
int svkMrsImageFFT::RequestDataSpectral( vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector, vtkImageFourierFilter* fourierFilter )
{
    //  Iterate through spectral data from all cells.  Eventually for performance I should do this by visible

    svkImageData* data = this->GetImageDataInput(0);

    //  Extent initially, and catch up with invisible extents after rerendering (modified update).
    int spatialDims[3];
    data->GetDimensions( spatialDims );
    spatialDims[0] -= 1;
    spatialDims[1] -= 1;
    spatialDims[2] -= 1;
    int numFrequencyPoints = data->GetCellData()->GetNumberOfTuples();
    int numComponents = data->GetCellData()->GetNumberOfComponents();
    double progress = 0;
    int ranges[3];
    ranges[0] = this->updateExtent[1]-this->updateExtent[0]+1;
    ranges[1] = this->updateExtent[3]-this->updateExtent[2]+1;
    ranges[2] = this->updateExtent[5]-this->updateExtent[4]+1;
    int denominator = ranges[2] * ranges[0] * ranges[1] + ranges[1] * ranges[0] + ranges[0];


    //  Get the Dimension Index and index values
    svkDcmHeader::DimensionVector dimensionVector = data->GetDcmHeader()->GetDimensionIndexVector();
    svkDcmHeader::DimensionVector indexVector = dimensionVector;

    //  GetNumber of cells in the image:
    int numCells = svkDcmHeader::GetNumberOfCells( &dimensionVector );

    for (int cellID = 0; cellID < numCells; cellID++ ) {

        //  Get the dimensionVector index for current cell -> indexVector:
        svkDcmHeader::GetDimensionVectorIndexFromCellID( &dimensionVector, &indexVector, cellID );

        if ( this->onlyUseSelectionBox == true ) {
            //  Get the 3D spatial index for comparing if a given cell is in the spatial selectin box maks:
            int spatialCellIndex = svkDcmHeader::GetSpatialCellIDFromDimensionVectorIndex( &dimensionVector, &indexVector);
            if ( this->selectionBoxMask[spatialCellIndex] == 0 ) {
                continue;
            }
        }

        ostringstream progressStream;
        progressStream <<"Executing FFT for cell " << cellID + 1 << "/" << numCells ;
        this->SetProgressText( progressStream.str().c_str() );
        int slice = svkDcmHeader::GetDimensionVectorValue( &indexVector, svkDcmHeader::SLICE_INDEX);
        progress = (((slice-this->updateExtent[4]) * (ranges[0]) * (ranges[1]) ) )/((double)denominator);
        this->UpdateProgress( progress );

        if ( ! svk4DImageData::IsIndexInExtent( this->updateExtent, &indexVector) ) {
            continue;
        }

        vtkFloatArray* spectrum = static_cast<vtkFloatArray*>(
            svkMrsImageData::SafeDownCast(data)->GetSpectrum( cellID )
        );

        vtkImageComplex* imageComplexTime      = new vtkImageComplex[ numFrequencyPoints ];
        vtkImageComplex* imageComplexFrequency = new vtkImageComplex[ numFrequencyPoints ];
        vtkImageComplex* imageOut;

        //  time to frequency:
        if ( this->mode == FORWARD ) {

            this->ConvertArrayToImageComplex( spectrum, imageComplexTime );

            fourierFilter->ExecuteFft( imageComplexTime, imageComplexFrequency, numFrequencyPoints );

            // For a FORWARD FFT, shift the frequency data to put 0 frequency at the center index point.
            this->FFTShift( imageComplexFrequency, numFrequencyPoints );

            imageOut = imageComplexFrequency;

        } else if (this->mode == REVERSE ) {

            this->ConvertArrayToImageComplex( spectrum, imageComplexFrequency);

            //  For a Reverse FFT, shift the frequency data to put 0 frequency at the first index point
            //  prior to RFFT.
            this->IFFTShift( imageComplexFrequency, numFrequencyPoints );

            fourierFilter->ExecuteRfft( imageComplexFrequency, imageComplexTime, numFrequencyPoints );

            imageOut = imageComplexTime;

        }

        for (int i = 0; i < numFrequencyPoints; i++) {
            spectrum->SetTuple2( i, imageOut[i].Real, imageOut[i].Imag );
        }

        delete[] imageComplexTime;
        delete[] imageComplexFrequency;

    }

    //  Update the DICOM header to reflect the spectral domain changes:
    if( this->mode == REVERSE ) {
        string domain("TIME");
        data->GetDcmHeader()->SetValue( "SignalDomainColumns", domain );
    } else {
        string domain("FREQUENCY");
        data->GetDcmHeader()->SetValue( "SignalDomainColumns", domain );
    }

    //  Trigger observer update via modified event:
    this->GetInput()->Modified();

    return 1;
}


/*
 *  Debugging
 */
void svkMrsImageFFT::PrintSpectrum( vtkImageComplex* data, int numPoints, std::string msg )
{
    for( int i = 0; i < numPoints; i++ ) {
        float abs = ( data[i].Real * data[i].Real )
                  + ( data[i].Imag * data[i].Imag );
        cout << msg << i << " = " << data[i].Real << " " << data[i].Imag << " -> " << abs << endl;
    }
}


/*!
 *  Shifts the zero frequency component in output of FFT operation to center of the spectrum.
 *  Behaves differently for even and odd data lengths.
 */
void svkMrsImageFFT::FFTShift( vtkImageComplex* dataIn, int numPoints )
{

    float origin =  static_cast<float>(numPoints - 1) / 2. ;
    int shiftSize = static_cast<int>( ceil( origin ) );
    int oddCorrection = numPoints%2;

    vtkImageComplex* dataTmp = new vtkImageComplex[ numPoints ];

    for (int i = 0; i < numPoints; i++) {
        if( i > origin ) {
            dataTmp[i - shiftSize - oddCorrection].Real = dataIn[i].Real;
            dataTmp[i - shiftSize - oddCorrection].Imag = dataIn[i].Imag;
        } else {
            dataTmp[i + shiftSize].Real = dataIn[i].Real;
            dataTmp[i + shiftSize].Imag = dataIn[i].Imag;
        }
    }
    for (int i = 0; i < numPoints; i++) {
        dataIn[i].Real = dataTmp[i].Real;
        dataIn[i].Imag = dataTmp[i].Imag;
    }

    delete [] dataTmp;
}


/*!
 *  Shifts the zero frequency component from center to origin, for example
 *  in preparation for an IFFT.
 *  Behaves differently for even and odd data lengths.
 *  Assumes that the data is ordered from low to high frequency.
 */
void svkMrsImageFFT::IFFTShift( vtkImageComplex* dataIn, int numPoints )
{

    float origin =  static_cast<float>(numPoints - 1)/ 2. ;
    int shiftSize = static_cast<int>( ceil( origin ) );
    int oddCorrection = numPoints%2;

    vtkImageComplex* dataTmp = new vtkImageComplex[ numPoints ];

    for (int i = 0; i < numPoints; i++) {
        if( i >= origin ) {
            dataTmp[i - shiftSize].Real = dataIn[i].Real;
            dataTmp[i - shiftSize].Imag = dataIn[i].Imag;
        } else {
            dataTmp[i + shiftSize + oddCorrection].Real = dataIn[i].Real;
            dataTmp[i + shiftSize + oddCorrection].Imag = dataIn[i].Imag;
        }
    }
    for (int i = 0; i < numPoints; i++) {
        dataIn[i].Real = dataTmp[i].Real;
        dataIn[i].Imag = dataTmp[i].Imag;
    }

    delete [] dataTmp;

}


/*!
 *
 */
void svkMrsImageFFT::SetFFTDomain( FFTDomain domain )
{
    this->domain = domain;
}


/*!
 *
 */
void svkMrsImageFFT::SetFFTMode( FFTMode mode )
{
    this->mode = mode;
}


/*!
 *  Should we correct for an offset center before FT?
 */
void svkMrsImageFFT::SetPreCorrectCenter( bool preCorrectCenter )
{
    this->preCorrectCenter = preCorrectCenter;
}


/*!
 *  Should we correct for an offset center after FT?
 */
void svkMrsImageFFT::SetPostCorrectCenter( bool postCorrectCenter )
{
    this->postCorrectCenter = postCorrectCenter;
}


/*!
 * Set the fractional voxel shift along cols, rows, slices
 * @param voxelShift
 */
void svkMrsImageFFT::SetVoxelShift( double voxelShift[3] )
{
    this->NormalizePhaseShift( voxelShift );
    this->voxelShift[0] = voxelShift[0];
    this->voxelShift[1] = voxelShift[1];
    this->voxelShift[2] = voxelShift[2];
    cout << "Normalized Voxel Shift: " 
        << this->voxelShift[0] << " " << this->voxelShift[1] << " " << this->voxelShift[2] << endl;
}


/*!
 *  This method will normalize the input shift to map to a +0.5 to -0.5
 *  range. This is the only type of phase shift supported by this class.
 *  @param shift
 */
void svkMrsImageFFT::NormalizePhaseShift( double shift[3] )
{
    for( int i = 0; i < 3; i ++ ) {
        shift[i] = fmod(shift[i], 1.0);
        if( shift[i] > 0.5 ) {
            shift[i] -= 1;
        } else if ( shift[i] < -0.5 ) {
            shift[i] += 1;
        }
    }
}


/*!
 *  Sets the extent over which the phasing should be applied.
 *  Takes 2 sets of x,y,z indices that specify the extent range
 *  in 3D.
 */
void svkMrsImageFFT::SetUpdateExtent(int* start, int* end)
{
    this->updateExtent[0] =  start[0];
    this->updateExtent[1] =  end[0];
    this->updateExtent[2] =  start[1];
    this->updateExtent[3] =  end[1];
    this->updateExtent[4] =  start[2];
    this->updateExtent[5] =  end[2];

    /*
     *  set modified time so that subsequent calls to Update() call RequestInformation()
     *  and refresh the extent
     */
    this->Modified();
}


/*
 *  For spectral only transforms, limit to selection box.  This will be ignored if
 *  the data is in KSPACE, or if a spatial transform was requested.
 */
void svkMrsImageFFT::OnlyUseSelectionBox()
{
    this->onlyUseSelectionBox = true;
}


/*
 *  For spectral only transforms, limit to selection box.  This will be ignored if
 *  the data requires a spatial transform.
 */
void svkMrsImageFFT::ValidateRequest()
{
    //  If the data is in k-space or the transform is spatial then ignore use selection box setting:
    if ( this->onlyUseSelectionBox == true ) {
        svkMrsImageData* data = svkMrsImageData::SafeDownCast(this->GetImageDataInput(0));
        string domainCol = data->GetDcmHeader()->GetStringValue( "SVK_ColumnsDomain");
        string domainRow = data->GetDcmHeader()->GetStringValue( "SVK_ColumnsDomain");
        string domainSlice = data->GetDcmHeader()->GetStringValue( "SVK_ColumnsDomain");
        if ( !domainCol.compare("KSPACE") || !domainRow.compare("KSPACE") || !domainSlice.compare("KSPACE") ) {
            cout << "Ignore request to only transform selection box for kspace data" << endl;
            this->onlyUseSelectionBox = false;
            return;
        }
        if ( this->domain == SPATIAL ) {
            cout << "Ignore request to only transform selection box for spatial transform" << endl;
            this->onlyUseSelectionBox = false;
            return;
        }

        //  If validated, then initialize the mask:
        svkDcmHeader::DimensionVector dimVec = data->GetDcmHeader()->GetDimensionIndexVector();
        int numSpatialVoxels = svkDcmHeader::GetNumSpatialVoxels(&dimVec);

        float tolerance = .5;
        this->selectionBoxMask = new short[numSpatialVoxels];
        data->GetSelectionBoxMask(this->selectionBoxMask, tolerance);
    }
}


/*!
 *  This method tries to get the most number of voxels with > .5 inside box
 *  Reimplement logic from set_parameters_v6 
 *      1. numVoxInBox = selected volume size / pixel size
 *      2. nearestNumVoxInBox =  round that to the nearest int
 *      3.  if nearestNumInBox > 2 * (int)nearestNumVoxInBox/2     
 *              shift by 1/2 voxel
 *          else 
 *              do not shift 
 */
void svkMrsImageFFT::MaximizeVoxelsInSelectionBox()
{
    //  For each dimension divide the selected volume by the pixel size in that 
    //  dimension.  That's the number of complete voxels in the box.
    //  add the remainder and

    svkMrsImageData* data = svkMrsImageData::SafeDownCast(this->GetImageDataInput(0));
    double pixelSpacing[3] = {0,0,0};
    data->GetDcmHeader()->GetPixelSpacing( pixelSpacing );

    double selBoxSpacing[3];
    data->GetSelectionBoxSpacing( selBoxSpacing );

    double selBoxCenter[3];
    data->GetSelectionBoxCenter( selBoxCenter );

    /* 
    //  Get the index of the voxel at the center of the press box:
    int boxCenterVoxelIndex[3];
    data->GetIndexFromPosition( selBoxCenter, boxCenterVoxelIndex );

    //  Get the LPS center of the voxel containing the selectionBox center (from it's cell index)
    double boxCenterVoxelLPS[3];
    data->GetPositionFromIndex( boxCenterVoxelIndex, boxCenterVoxelLPS );

    //  Get the distance along the col, row or slice from the voxel center to the center of the selection box:
    //  LPS to row,col,slice space ( convert LPS to cols, rows slices displacement)
    double displacementAlongLPSAxes[3];
    for (int i = 0; i < 3; i++) {
        displacementAlongLPSAxes[i] = boxCenterVoxelLPS[i] - selBoxCenter[i];
    }
    // this is the displacemnt from the center of the voxel containing the sel box center to the center of the sel box
    double displacementAlongDataAxes[3];
    double dcos[3][3];
    data->GetDcos(dcos);
    for (int i = 0; i < 3; i++) {
        displacementAlongDataAxes[i] = 0.;
        for (int j = 0; j < 3; j++) {
            displacementAlongDataAxes[i] -= dcos[i][j] * displacementAlongLPSAxes[j];
        }
    }

    //  loop over data dimension (cols, rows slices)
    double voxelShift[3];
    for ( int dim = 0; dim < 3; dim++ ) {

        //  Get max number of whole voxels in the box
        int numWholeVoxels = static_cast<int>(selBoxSpacing[dim] / pixelSpacing[dim]);

        // Even number of whole voxels: box center is between two voxels
        // Odd number of whole voxels:  box center is at center of voxel
        //      compare boxCenterLPS  and boxelCenterLPS and figure out if it's
        //      at a boundry or center.
        if ( numWholeVoxels % 2 == 0 ) {
            // displacement should be 1/2 pixel spacing
            if ( abs( displacementAlongDataAxes[dim] ) != pixelSpacing[dim] /2. ) {
                double voxelShiftAbsolute = 0.5 - displacementAlongDataAxes[dim];
                voxelShift[dim] = voxelShiftAbsolute / pixelSpacing[dim];
            }
        } else if ( numWholeVoxels % 2 == 1 ) {
            if ( abs( displacementAlongDataAxes[dim] ) != 0. ) {
                double voxelShiftAbsolute = -1 * displacementAlongDataAxes[dim];
                voxelShift[dim] = voxelShiftAbsolute / pixelSpacing[dim];
            }
        }
    }
    */


    double voxelShift[3];
    for ( int dim = 0; dim < 3; dim++ ) {

        //  Get max number of whole voxels in the box
        float numVoxelsInBox = selBoxSpacing[dim] / pixelSpacing[dim];
        if (this->GetDebug()) {
            cout << "vox in box : " << numVoxelsInBox << endl;
        }
        int nearestNumVoxelsInBox = static_cast<int>( roundf( numVoxelsInBox ) ); 
        if (this->GetDebug()) {
            cout << "nn in box : " << nearestNumVoxelsInBox << endl;
        }
        if ( 
            ( nearestNumVoxelsInBox > 1 ) && 
            ( nearestNumVoxelsInBox > 2 * static_cast<int>( nearestNumVoxelsInBox / 2 ) )
        ) {
            voxelShift[dim] = 0.5 * pixelSpacing[dim];  
        } else {
            voxelShift[dim] = 0.0;  
        }
    }
    if (this->GetDebug()) {
        cout << "Voxel shift to maximize: " << voxelShift[0] << " " << voxelShift[1] << " " << voxelShift[2] << endl;
    }

    //  Get the center of the toplc voxel:
    double toplc[3];
    data->GetDcmHeader()->GetOrigin(toplc, 0); 

    double dcos[3][3];
    data->GetDcos(dcos);

    //  LPS to XYZ (cols,rows,slices)
    //  LPS distance from centerLPS of the selBox to toplcLPS in terms of cols, rows, slices frame (XYZ)
    //      calculate the distance from the topLC to the origin in LPS coordinates in terms of the 
    //      xyz(cols,rows,slices) data frame by projecting the distance in the cols, rows slices 
    //      farme (selBoxCtr - toplc) to LPS:
    float temp; 
    double toplcToSelBoxCenterXYZ[3];
    for (int i = 0; i < 3; i++) {
        temp = 0.0; 
        for (int j = 0; j < 3; j++) {
            temp = temp + dcos[i][j] * ( selBoxCenter[j] - toplc[j] ); 
        }
        toplcToSelBoxCenterXYZ[i] = temp; 
    }
    if (this->GetDebug()) {
        cout << "MAX VS(xyzcenter): " << toplcToSelBoxCenterXYZ[0] << " " 
            << toplcToSelBoxCenterXYZ[1] << " " << toplcToSelBoxCenterXYZ[2] << endl;
    }
    double shiftedDistanceToplcToSelBoxCenterXYZ[3];
    for (int i = 0; i < 3; i++) {
        shiftedDistanceToplcToSelBoxCenterXYZ[i] = voxelShift[i] + toplcToSelBoxCenterXYZ[i];
    }

    //  Convert the shifted distance to the center in xyz(col,row,slices) back to LPS frame and 
    //  add to the toplc:  
    //  temp = shiftedDistanceToplcToSelBoxCenterLPS; 
    double centerLPS[3];
    for (int i = 0; i < 3; i++) {
        temp = 0.0; 
        for (int j = 0; j < 3; j++) {
            temp = temp + dcos[j][i] * ( shiftedDistanceToplcToSelBoxCenterXYZ[j] ); 
        }
        centerLPS[i] = toplc[i] + temp; 
    }

    ////  This is the target spatial center and should be subtracte from 
    ////  the original center to get the voxel shift size
    //for (int i = 0; i < 3; i++) {
        //voxelShift[i] = centerLPS[i]; 
    //}
    //cout << "MAX VS(lpscenter): " << centerLPS[0] << " " << centerLPS[1] << " " << centerLPS[2] << endl;

    //  center data at this location
    this->SetVolumeCenter( centerLPS );

}


/*!
 *  Set the center of the volume to the specified LPS value.
 *  This method sets the member voxelShif variable required to achieve the shift to the
 *  centerLPS value specified.
 *  @param centerLPS    This is the target LPS value for the data after applying a voxel shift
 *
 */
void svkMrsImageFFT::SetVolumeCenter( double centerLPS[3] )
{
    // Get the current volume center and voxel size.
    // Determine the shift in terms of voxel units
    svkMrsImageData* data = svkMrsImageData::SafeDownCast(this->GetImageDataInput(0));
    double currentCenterLPS[3];
    data->GetImageCenter(currentCenterLPS);
    double pixelSize[3];
    data->GetDcmHeader()->GetPixelSize(pixelSize);

    double dcos[3][3];
    data->GetDcos(dcos);

    //  shift must be applied in the data frame, which can be oblique to the LPS frame
    //  the target is in the LPS frame and the Deltas in LPS must be projected into the
    //  the data frame defined by the dcos.
    double lpsShift[3];
    double voxelShiftAbsolute[3];

    cout << "Current center: " << currentCenterLPS[0] << " " << currentCenterLPS[1] << " " << currentCenterLPS[2] << endl;
    for (int i = 0; i < 3; i++) {
        lpsShift[i] = -(currentCenterLPS[i] - centerLPS[i]);
        cout << "LPSSHIFT: "  << lpsShift[i] << endl;
        cout << "center: "  << currentCenterLPS[i] << endl;
    }
    //  LPS to row,col,slice space
    for (int i = 0; i < 3; i++) {
        voxelShiftAbsolute[i] = 0.;
        for (int j = 0; j < 3; j++) {
            voxelShiftAbsolute[i] -=  dcos[i][j] * lpsShift[j];
        }
        //  now convert the absolute voxel shift into fractions of voxels
        this->voxelShift[i] =  voxelShiftAbsolute[i] / pixelSize[i];
    }
    //cout << "Voxel Shift: " << this->voxelShift[0] << " " << this->voxelShift[1] << " " << this->voxelShift[2]<< endl;
    //cout << "PS         : " << pixelSize[0] << " " << pixelSize[1] << " " << pixelSize[2] << endl;
    //cout << "dcos       : " << dcos[0][0] << " " << dcos[0][1] << " " << dcos[0][2] << endl;
    //cout << "dcos       : " << dcos[1][0] << " " << dcos[1][1] << " " << dcos[1][2] << endl;
    //cout << "dcos       : " << dcos[2][0] << " " << dcos[2][1] << " " << dcos[2][2] << endl;
}


/*!
 *
 */
int svkMrsImageFFT::FillInputPortInformation( int vtkNotUsed(port), vtkInformation* info )
{
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "svkMrsImageData"); 
    return 1;
}
