/*
 *  Copyright © 2009-2012 The Regents of the University of California.
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



#include <svkEPSIReorder.h>
#include <svkMrsLinearPhase.h> 


using namespace svk;


vtkCxxRevisionMacro(svkEPSIReorder, "$Rev$");
vtkStandardNewMacro(svkEPSIReorder);


/*!
 *  Constructor.  Initialize any member variables. 
 */
svkEPSIReorder::svkEPSIReorder()
{

#if VTK_DEBUG_ON
    this->DebugOn();
#endif

    vtkDebugMacro(<<this->GetClassName() << "::" << this->GetClassName() << "()");

    //  Initialize any member variables
    this->numEPSIkRead = 0;
    this->epsiAxis = -1;
    this->epsiOrigin = -1;

}


/*!
 *  Clean up any allocated member variables. 
 */
svkEPSIReorder::~svkEPSIReorder()
{
}


/*!
 *  Set the number of k-space samples along the EPSI encoding 
 *  direction (number of samples per lobe). This is the number
 *  of samples per echo in the EPSI acquisition trajetory (not 
 *  necessarily the final k-space dimensionality). 
 */
void svkEPSIReorder::SetNumEPSIkRead( int numKspacePoints )
{
    this->numEPSIkRead = numKspacePoints;
}


/*!
 *  Set the axis index corresponding to the EPSI encoding (0,1 or 2). 
 */
void svkEPSIReorder::SetEPSIAxis( int epsiAxis)
{
    this->epsiAxis = epsiAxis;
}


/*!
 *  Set the origin index along the EPSI encoding axis 
 *  default = (numEPSIkRead-1)/2. See notes for 
 *  GetEPSIOrigin.  
 */
void svkEPSIReorder::SetEPSIOrigin( float epsiOrigin )
{
    this->epsiOrigin = epsiOrigin;
}


/*!
 *  Get the origin index along the EPSI encoding axis 
 *  default = (numEPSIkRead-1)/2. This is the c-lang  
 *  index, thus the -1, e.g.: if numEPSIkRead is 8,  
 *  and data index varies from 0-7, the default origin 
 *  index is 3.5.  However, should depend on whether 
 *  k=0 was sampled or not.  If not, (default, then origin
 *  is .5 higher. 
 */
float svkEPSIReorder::GetEPSIOrigin()
{
    bool k0Sampled = false; 
    if ( this->epsiOrigin < 0 ) { 
        if (k0Sampled) { 
            this->epsiOrigin = (this->numEPSIkRead - 1) / 2.;
        } else { 
            this->epsiOrigin = (this->numEPSIkRead) / 2.;
        }
    }
    return this->epsiOrigin; 
}


/*! 
 *  This method is called during pipeline execution.  Calculates the 2D array of linear phase correction factors, which
 *  are a function of the epsi k-space index, as well as the spectral index.  Applies these to all spectra in data set
 *  to generate a rectilinear spectral/spatial (k-space, time-domain) data set.  
 */
int svkEPSIReorder::RequestData( vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector )
{

    if ( this->numEPSIkRead == 0  || this->epsiAxis < 0 ) {
        cout << "ERROR, must specify the epsiAxis and number of sample k-space points per lobe" << endl;
        return 1; 
    }

    //  Get pointer to input data set. 
    svkMrsImageData* mrsData = svkMrsImageData::SafeDownCast(this->GetImageDataInput(0)); 

    //  Get pointer to data's meta-data header (DICOM object). 
    svkDcmHeader* hdr = mrsData->GetDcmHeader();  

    //  Lookup any data set attributes from header required for algorithm (See DICOM IOD for field names):
    int numSpecPts = hdr->GetIntValue( "DataPointColumns" );
    int cols       = hdr->GetIntValue( "Columns" );
    int rows       = hdr->GetIntValue( "Rows" );
    int slices     = hdr->GetNumberOfSlices();
    int numLobes   = hdr->GetNumberOfCoils();  // e.g. symmetric EPSI has pos + neg lobes

    //  Initialize the spatial and spectral factor in the EPSI phase correction: 
    //  One phase factor for each value of k in EPSI axis
    vtkImageComplex** epsiPhaseArray = new vtkImageComplex*[ this->numEPSIkRead ];  
    for (int i = 0; i < this->numEPSIkRead; i++ ) {
        epsiPhaseArray[i] = new vtkImageComplex[ numSpecPts ];  
    }
    this->CreateEPSIReorderionFactors( epsiPhaseArray, numSpecPts ); 

    float cmplxPtIn[2]; 
    float cmplxPtPhased[2]; 
    float epsiPhase[2]; 
    int   epsiIndex; 
    vtkImageComplex* ktCorrection = new vtkImageComplex[2]; 

    //  Inverse Fourier Transform spectral data to frequency domain to 
    //  apply linear phase shift: 
    this->SpectralFFT( svkMrsImageFFT::FORWARD); 

    //  Iterate through 3D spatial locations
    for (int lobe = 0; lobe < numLobes; lobe++) {
        for (int z = 0; z < slices; z++) {
            for (int y = 0; y < rows; y++) {
                for (int x = 0; x < cols; x++) {

                    //  Index along epsiAxis used to get appropriate kPhaseArray values along epsiAxis 
                    if ( this->epsiAxis == 2 ) {
                        epsiIndex = z; 
                    } else if ( this->epsiAxis == 1 ) {
                        epsiIndex = y; 
                    } else if ( this->epsiAxis == 0 ) {
                        epsiIndex = x; 
                    }

                    vtkFloatArray* spectrum = vtkFloatArray::SafeDownCast( mrsData->GetSpectrum( x, y, z, 0, lobe) );

                    //  Iterate over frequency points in spectrum and apply phase correction:
                    for ( int freq = 0; freq < numSpecPts; freq++ ) {
                    
                        spectrum->GetTupleValue(freq, cmplxPtIn);

                        epsiPhase[0] = epsiPhaseArray[epsiIndex][freq].Real; 
                        epsiPhase[1] = epsiPhaseArray[epsiIndex][freq].Imag; 

                        //  data * e(i * X)
                        //cmplxPtPhased[0] = cmplxPtIn[0] * epsiPhase[0] - cmplxPtIn[1] * epsiPhase[1]; 
                        //cmplxPtPhased[1] = cmplxPtIn[1] * epsiPhase[0] + cmplxPtIn[0] * epsiPhase[1]; 

                        //  data * e(-i * X)
                        cmplxPtPhased[0] = cmplxPtIn[0] * epsiPhase[0] + cmplxPtIn[1] * epsiPhase[1]; 
                        cmplxPtPhased[1] = cmplxPtIn[1] * epsiPhase[0] - cmplxPtIn[0] * epsiPhase[1]; 

                        //cout << "spec: " << cmplxPtIn[0] << " " << cmplxPtIn[1] << " -> " << cmplxPtPhased[0] 
                        //  << " " << cmplxPtPhased[1] << endl;

                        spectrum->SetTuple(freq, cmplxPtPhased); 
    
                    }

                }
            }
        }
    }


    //  Forward Fourier Transform spectral data to back to time domain, should now be shifted.   
    this->SpectralFFT( svkMrsImageFFT::REVERSE); 


    //  Trigger observer update via modified event:
    this->GetInput()->Modified();
    this->GetInput()->Update();

    for (int i = 0; i < this->numEPSIkRead; i++ ) {
        delete [] epsiPhaseArray[i]; 
    }
    delete [] epsiPhaseArray; 

    return 1; 
} 


/*!
 *  Reorder the data.  Input EPSI data has one dimension with both time (spectral) and k-space
 *  content.  The time domain data from each k-space point needs to be extracted and put into 
 *  a cell array.  This is done separately for positive and negative lobes.  So, for example, 
 *  a 1380 x 1 x 12 x 10 data set with 60 lobes (23 k-space samples) would get reordered to a
 *  two 30 x 23 x 12 x 10 data sets. Each will be encoded into a separate "channel" indicated
 *  by the dimension index sequence.  this is just a temporarary internal layout.
 */
void svkEPSIReorder::ReorderEPSIData( svkImageData* data )
{
    //  Reorder data.  This requires adding data arrays 
    //  to accommodate the spectra at the new k-space points 
    //  points extracted from the EPSI encoding 

    //  Preallocate data arrays. We can't override the original arrays, 
    //  until the reordering is complette, so use these as the 
    //  target arrays. 

    //  Allocate arrays for all How many k-space points were acquired in all?

    //  Allocate arrays for spectra at each phase encode:
    svkDcmHeader* hdr = data->GetDcmHeader();
    vtkstd::string dataRepresentation = hdr->GetStringValue( "DataRepresentation" );

    int numComponents;
    if ( dataRepresentation == "COMPLEX" ) {
        numComponents = 2;
    } else {
        numComponents = 1;
    }

    //===========
    //  Reset hdr value (DataPointColumns)
    //===========
    int numEPSIPts= hdr->GetIntValue( "DataPointColumns" );
    //cout << "Num EPSI Pts: "<< numEPSIPts << endl;

    //  this is the number of lobes in the EPSI sampling. For symmetric 
    //  epsi this is twice the number of frequence points (pos + neg)    
    int numSymmetricEPSILobes = this->GetHeaderValueAsInt( "rhr.rh_user10" );
    int numFreqPts = numSymmetricEPSILobes / 2;     
    //cout << "Num Freq Pts: "<< numFreqPts << endl;


    //============================================================
    //  Read EPSI acquisition params from raw header: 
    //============================================================

    //  dwell time time between k-space points
    int deltaT = this->GetHeaderValueAsInt( "rhr.rh_user12" );

    //  time for plateau encoding (gradient duratation)
    int plateauTime = this->GetHeaderValueAsInt( "rhr.rh_user13" );

    //  time for ramp in one direction
    int rampTime = this->GetHeaderValueAsInt( "rhr.rh_user15" );

    //  number of samples at start
    int numSkip = this->GetHeaderValueAsInt( "rhr.rh_user22" );

    //  number of samples per lobe (ramps + plateau)  
    //  num spectral samples in FID is num time_pts / this value (
    int numRead = (plateauTime + 2 * rampTime)/deltaT;

    //============================================================
    //Q:  is the number of EPSI phase encodes equal to numRead or numEPSIkSpacePoints?
    //  I think it starts at numRead, and gets sampled down to numEPSIkSpacePoints (23-18)
    //  after ramp sampling... i think
    //============================================================
    //===========
    //  Reset hdr value (num cols?)
    //===========
    int numEPSIkSpacePts = this->GetHeaderValueAsInt( "rhr.rh_user9" );

    //  EPSI Axis (user9) defines which axis is epsi encoded.  Swap the value of 
    //  epsi k-space encodes into this field: 
    int epsiAxis = this->GetHeaderValueAsInt( "rhr.rh_user20" ) - 1;

    //  Get the input dimensionality (EPSI)
    int numEPSIVoxels[3]; 
    this->GetNumVoxels( numEPSIVoxels ); 

    //  Set the actual number of k-space points sampled:
    int numVoxels[3]; 
    numVoxels[0] = numEPSIVoxels[0]; 
    numVoxels[1] = numEPSIVoxels[1]; 
    numVoxels[2] = numEPSIVoxels[2]; 
    numVoxels[ epsiAxis ] = numRead; 
    //numVoxels[ epsiAxis ] = numEPSIkSpacePts; 

    //  Set the number of reordered voxels (target dimensionality of this method)
    int reorderedVoxels[3]; 
    reorderedVoxels[0] = numVoxels[0]; 
    reorderedVoxels[1] = numVoxels[1]; 
    reorderedVoxels[2] = numVoxels[2]; 
    reorderedVoxels[ epsiAxis ] = numVoxels[epsiAxis] - 2; //throw out first and last samples

    //cout << "full dimensionality = " << numVoxels[0] << " " << numVoxels[1] << " " << numVoxels[2] << endl;
    int totalVoxels = numVoxels[0] * numVoxels[1] * numVoxels[2]; 

    //============================================================
    //  Reorder the data: 
    //============================================================
    int numTimePoints = 1; 
    int timePoint = 0;
    int numCoils = 2; //lobes
    int coilNum = 0; 

    //============================================================
    //  create a data set to store the new reordered data arrays. 
    //  Remove all the original arrays from it. 
    //============================================================
    svkImageData* reorderedImageData = svkMrsImageData::New();
    reorderedImageData->DeepCopy( data ); 
    this->RemoveArrays( reorderedImageData ); 

    //============================================================
    //  Preallocate data arrays for reordered data. The API only permits dynamic 
    //  assignmet at end of CellData, so for swapped cases where we need to insert 
    //  data out of order they need to be preallocated.
    //============================================================
    int numReorderedVoxels = 1;
    for (int i = 0; i < 3; i++) {
        numReorderedVoxels *= reorderedVoxels[i];  
    }
    numReorderedVoxels *= numTimePoints * numCoils; //2 lobes
    for (int arrayNumber = 0; arrayNumber < numReorderedVoxels; arrayNumber++) {
        vtkDataArray* dataArray = vtkDataArray::CreateDataArray(VTK_FLOAT);
        dataArray->SetNumberOfComponents( numComponents );
        dataArray->SetNumberOfTuples( numFreqPts );
        reorderedImageData->GetCellData()->AddArray(dataArray);
        dataArray->Delete();
    }

    
    for (int zEPSI = 0; zEPSI < numEPSIVoxels[2]; zEPSI++ ) { 
        for (int yEPSI = 0; yEPSI < numEPSIVoxels[1]; yEPSI++ ) { 
            for (int xEPSI = 0; xEPSI < numEPSIVoxels[0]; xEPSI++ ) { 

                vtkFloatArray* epsiSpectrum = static_cast<vtkFloatArray*>(
                        svkMrsImageData::SafeDownCast(data)->GetSpectrum( xEPSI, yEPSI, zEPSI, timePoint, 0) );
    
                //  define range of spatial dimension to expand (i.e. epsiAxis):  
                int rangeMin[3]; 
                rangeMin[0] = xEPSI; 
                rangeMin[1] = yEPSI; 
                rangeMin[2] = zEPSI; 
                int rangeMax[3]; 
                rangeMax[0] = xEPSI; 
                rangeMax[1] = yEPSI;
                rangeMax[2] = zEPSI;
                //  throw out the first and last points (zero crossing)
                rangeMin[epsiAxis] = 0; 
                rangeMax[epsiAxis] = numVoxels[epsiAxis] - 2; 

                
                //============================================================
                //  allocate an array for both the positive and negative lobes of 
                //  the symmetric EPSI encoding:
                //============================================================
                int lobeStride = numRead;  

                //  set the current index along the EPSI dimension
                //  Initialize first point to skip + throw out first point (zero crossing)
                int currentEPSIPt = numSkip + 1;   

                rangeMax[epsiAxis] -= 1;    //  other indices have zero range 
                //  first loop over epsi-kspace points for first lobe, then for 2nd lobe, etc.
                for (int lobe = 0; lobe < 2; lobe++ ) { 
                    for (int z = rangeMin[2]; z < rangeMax[2] + 1; z++ ) { 
                        for (int y = rangeMin[1]; y < rangeMax[1] + 1; y++ ) { 
                            for (int x = rangeMin[0]; x < rangeMax[0] + 1; x++ ) { 

                                //  Add arrays to the new data set at the correct array index: 
                                int index =  x
                                          + ( reorderedVoxels[0] ) * y
                                          + ( reorderedVoxels[0] * reorderedVoxels[1] ) * z
                                          + ( reorderedVoxels[0] * reorderedVoxels[1] * reorderedVoxels[2] ) * lobe 
                                          + ( reorderedVoxels[0] * reorderedVoxels[1] * reorderedVoxels[2] * 2 ) * timePoint;
 
                                //cout << "   target index(xyzlobe): " << x << " " 
                                    //<< y << " " << z << " " << lobe  << " index: " << index << endl;
                                vtkDataArray* dataArray = reorderedImageData->GetCellData()->GetArray(index);

                                char arrayName[30];
                                sprintf(arrayName, "%d %d %d %d %d", x, y, z, timePoint, lobe);
                                //cout << "reordered Array name (" << index << ") = " << arrayName << endl; 
                                dataArray->SetName(arrayName);

                                //  reorder EPSI data, throwng away the first and last 
                                //  point (zero crossing). 
                                float epsiTuple[2]; 
                                float tuple[2]; 
                                int epsiOffset; 
                                for (int i = 0; i < numFreqPts; i++) {
                                    epsiOffset = (lobeStride * 2 * i ) + currentEPSIPt;
                                    epsiSpectrum->GetTupleValue(epsiOffset, epsiTuple); 
                                    tuple[0] = epsiTuple[0]; 
                                    tuple[1] = epsiTuple[1]; 
                                    dataArray->SetTuple( i, tuple );

                                    //cout << "       extract spectrum  epsiOffset: " 
                                    //<< epsiOffset << " target index " << i << endl;
                                    //if ( i <  10 ) { 
                                        //cout << "fist 2 values or val0:  " << epsiTuple[0] << " " << epsiTuple[1] << endl;                                     //}
                                }

                                currentEPSIPt++; 
                            }
                        }
                    }
                    //  between lobes, throw out the last and first point before resuming sampling
                    //  These are the zero crossings in symmetric EPSI. 
                    currentEPSIPt +=2; 
                }
            }
        }
    }


    //  =================================================
    //  Redimension the meta data and set the new arrays:
    //  =================================================
    data->DeepCopy( reorderedImageData ); 
    numVoxels[epsiAxis] = numVoxels[epsiAxis] - 2; //   first and last point were thrown out
    int numVoxelsOriginal[3]; 
    this->GetNumVoxels( numVoxelsOriginal ); 
    this->RedimensionData( data, numVoxelsOriginal, numVoxels, numFreqPts ); 


}


/*!
 *  Set the input data type, e.g. svkMrsImageData for an MRS algorithm.
 */
int svkEPSIReorder::FillInputPortInformation( int vtkNotUsed(port), vtkInformation* info )
{
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "svkMrsImageData"); 
    return 1;
}


