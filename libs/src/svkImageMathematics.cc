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


//include </usr/include/vtk/vtkInstantiator.h>
#include </usr/include/vtk/vtkInformation.h>
#include </usr/include/vtk/vtkDataSetAttributes.h>
#include </usr/include/vtk/vtkImageCast.h>

#include <svkImageMathematics.h>
#include <svkMriImageData.h>


using namespace svk;


//vtkCxxRevisionMacro(svkImageMathematics, "$Rev$");
vtkStandardNewMacro(svkImageMathematics);


/*!
 *
 *  Default input type is svkImageData base class. Override with a specific concrete type in
 *  sub class if necessary.
 *  Port 0 -> input 1 image to operate on
 *       1 -> input 2 image to operate on (optional)
 *       2 -> mask (optional)
 */
svkImageMathematics::svkImageMathematics()
{

#if VTK_DEBUG_ON
    this->DebugOn();
#endif
    this->portMapper = NULL;

    vtkDebugMacro(<<this->GetClassName() << "::" << this->GetClassName() << "()");

    vtkInstantiator::RegisterInstantiator("svkMriImageData", svkMriImageData::NewObject);

    this->SetNumberOfInputPorts(17);
    bool required = true;
    this->GetPortMapper()->InitializeInputPort( INPUT_IMAGE_1, "INPUT_IMAGE_1", svkAlgorithmPortMapper::SVK_MR_IMAGE_DATA);
    this->GetPortMapper()->InitializeInputPort( INPUT_IMAGE_2, "INPUT_IMAGE_2", svkAlgorithmPortMapper::SVK_MR_IMAGE_DATA, !required );
    this->GetPortMapper()->InitializeInputPort( MASK, "MASK", svkAlgorithmPortMapper::SVK_MR_IMAGE_DATA, !required);
    this->GetPortMapper()->InitializeInputPort( MASK_FOR_MEDIAN, "MASK_FOR_MEDIAN", svkAlgorithmPortMapper::SVK_MR_IMAGE_DATA, !required);
    this->GetPortMapper()->InitializeInputPort( ADD, "ADD", svkAlgorithmPortMapper::SVK_BOOL, !required);
    this->GetPortMapper()->InitializeInputPort( MULTIPLY, "MULTIPLY", svkAlgorithmPortMapper::SVK_BOOL, !required);
    this->GetPortMapper()->InitializeInputPort( SUBTRACT, "SUBTRACT", svkAlgorithmPortMapper::SVK_BOOL, !required);
    this->GetPortMapper()->InitializeInputPort( MULTIPLY_BY_SCALAR , "MULTIPLY_BY_SCALAR", svkAlgorithmPortMapper::SVK_DOUBLE, !required);
    this->GetPortMapper()->InitializeInputPort( ADD_SCALAR , "ADD_SCALAR", svkAlgorithmPortMapper::SVK_DOUBLE, !required);
    this->GetPortMapper()->InitializeInputPort( SQUARE_ROOT, "SQUARE_ROOT", svkAlgorithmPortMapper::SVK_BOOL, !required);
    this->GetPortMapper()->InitializeInputPort( MULTIPLY_IMAGE_1_BY_IMAGE_2_MEDIAN , "MULTIPLY_IMAGE_1_BY_IMAGE_2_MEDIAN", svkAlgorithmPortMapper::SVK_BOOL, !required);
    this->GetPortMapper()->InitializeInputPort( NUM_BINS_FOR_HISTOGRAM , "NUM_BINS_FOR_HISTOGRAM", svkAlgorithmPortMapper::SVK_INT, !required);
    this->GetPortMapper()->InitializeInputPort( BIN_SIZE_FOR_HISTOGRAM , "BIN_SIZE_FOR_HISTOGRAM", svkAlgorithmPortMapper::SVK_DOUBLE, !required);
    this->GetPortMapper()->InitializeInputPort( START_BIN_FOR_HISTOGRAM , "START_BIN_FOR_HISTOGRAM", svkAlgorithmPortMapper::SVK_DOUBLE, !required);
    this->GetPortMapper()->InitializeInputPort( SMOOTH_BINS_FOR_HISTOGRAM , "SMOOTH_BINS_FOR_HISTOGRAM", svkAlgorithmPortMapper::SVK_INT, !required);
    this->GetPortMapper()->InitializeInputPort( OUTPUT_SERIES_DESCRIPTION, "OUTPUT_SERIES_DESCRIPTION", svkAlgorithmPortMapper::SVK_STRING, !required);
    this->GetPortMapper()->InitializeInputPort( OUTPUT_TYPE, "OUTPUT_TYPE", svkAlgorithmPortMapper::SVK_INT, !required);

    this->SetNumberOfOutputPorts(1);
    this->GetPortMapper()->InitializeOutputPort( 0, "MATH_OUTPUT", svkAlgorithmPortMapper::SVK_MR_IMAGE_DATA);

}


/*!
 *
 */
svkImageMathematics::~svkImageMathematics()
{
    if( this->portMapper != NULL ) {
        this->portMapper->Delete();
        this->portMapper = NULL;
    }
}


/*!
 * Pass through method to the internal svkAlgorithmPortMapper
 */
void svkImageMathematics::SetInputPortsFromXML( )
{
    if(this->GetPortMapper()->GetBoolInputPortValue(ADD) && this->GetPortMapper()->GetBoolInputPortValue(ADD)->GetValue()){
        this->SetOperationToAdd();
    }
    if( this->GetPortMapper()->GetDoubleInputPortValue( MULTIPLY_BY_SCALAR ) != NULL ) {
        double k = this->GetPortMapper()->GetDoubleInputPortValue( MULTIPLY_BY_SCALAR )->GetValue();
        this->SetConstantK(k);
        this->SetOperationToMultiplyByK();
    }
    if(this->GetPortMapper()->GetBoolInputPortValue(MULTIPLY) && this->GetPortMapper()->GetBoolInputPortValue(MULTIPLY)->GetValue()){
        this->SetOperationToMultiply();
    }
    if(this->GetPortMapper()->GetBoolInputPortValue(SUBTRACT) && this->GetPortMapper()->GetBoolInputPortValue(SUBTRACT)->GetValue()){
        this->SetOperationToSubtract();
    }
    if(this->GetPortMapper()->GetBoolInputPortValue(MULTIPLY_IMAGE_1_BY_IMAGE_2_MEDIAN) && this->GetPortMapper()->GetBoolInputPortValue(MULTIPLY_IMAGE_1_BY_IMAGE_2_MEDIAN)->GetValue()){
        this->SetOperationToMultiplyByK();
    }
    if(this->GetPortMapper()->GetBoolInputPortValue(SQUARE_ROOT) ) {
        this->SetOperationToSquareRoot();
    }
}


/*!
 * Returns the port mapper. Performs lazy initialization.
 */
svkAlgorithmPortMapper* svkImageMathematics::GetPortMapper()
{
    if( this->portMapper == NULL ) {
        this->portMapper = svkAlgorithmPortMapper::New();
        this->portMapper->SetAlgorithm( this );
    }
    return this->portMapper;
}


/*!
 * Utility setter for input port: Output float
 */
void svkImageMathematics::SetOutputType(int outputType)
{
    this->GetPortMapper()->SetIntInputPortValue(OUTPUT_TYPE, outputType);
}


/*!
 * Utility getter for input port: Output float
 */
svkInt* svkImageMathematics::GetOutputType()
{
    return this->GetPortMapper()->GetIntInputPortValue(OUTPUT_TYPE);
}


/*!
 *  PrintSelf method calls parent class PrintSelf, then prints all parameters using the port mapper.
 *
 */
void svkImageMathematics::PrintSelf( ostream &os, vtkIndent indent )
{
    Superclass::PrintSelf( os, indent );
    this->GetPortMapper()->PrintSelf( os, indent );
}


/*!
 *  If 2 inputs, they must have the same data types.  IF the data types differ, upcast the smaller data type 
 */
void svkImageMathematics::SetDatatypes()
{
    // By default, output datatype is the greater of the input datatypes
    // With two integer inputs, you must explicitly request float output
    svk::svkDcmHeader::DcmPixelDataFormat svkOutputType = svkDcmHeader::UNDEFINED;
    if (this->GetPortMapper()->GetIntInputPortValue(OUTPUT_TYPE)) {
        int outputType = this->GetOutputType()->GetValue();
        switch (outputType) {
            case UNDEFINED:
                break;
            case UNSIGNED_INT_2:
                svkOutputType = svkDcmHeader::UNSIGNED_INT_2;
                break;
            case SIGNED_FLOAT_4:
                svkOutputType = svkDcmHeader::SIGNED_FLOAT_4;
                break;
            default:
                break;
        }
    }

    svkMriImageData::SafeDownCast(this->GetOutput())->DeepCopy( this->GetImageDataInput(0), svkOutputType );
}


/*!
 *  This method loops over all volumes and calls the VTK super class update method on each to 
 *  perform the specified calculation.  If necessary the output will be masked by an input mask.
 */
int svkImageMathematics::RequestData( vtkInformation* request,
                                            vtkInformationVector** inputVector,
                                            vtkInformationVector* outputVector )
{

    // if there are 2 inputs of different types, upcast to the larger: 
    this->SetDatatypes();  

    //  Determine how many vtkPointData arrays are in the input data
    //  Require the first input to have at least as many volumes as the 2nd input if a binary 
    //  operation.     
    this->GetImageDataInput(0); 
    this->GetImageDataInput(0)->GetPointData(); 
    int numVolumes0 = this->GetImageDataInput(0)->GetPointData()->GetNumberOfArrays();
    int numVolumes1 = 0;
    if (this->GetInput(1)) {
        numVolumes1 = this->GetImageDataInput(1)->GetPointData()->GetNumberOfArrays();
        if ( numVolumes0 != numVolumes1 ) {
            if ( numVolumes1 == 1 ) {
            } else {
                cout << "ERROR, svkImageMathematics:  number of volumes is not compatible: " << numVolumes0 << " " << numVolumes1 << endl;
                exit(1); 
            }
        }
    }

    int numVolumes = ( numVolumes1 > numVolumes0 ) ? numVolumes1:numVolumes0;

    //  Set the active scalars for each relevant input and output arrays:    
    //  Determine number of input ports for this particular operation (1 or 2).    
    this->SetInputPortsFromXML();

    //cout << "OUTPUT DATA: " << *this->GetOutput(); 

    for ( int vol = 0; vol < numVolumes; vol++ ) {


        //  input 1 scalar data volume
        string arrayName0;  
        if (this->GetInput(0)) {
            arrayName0 = this->GetImageDataInput(0)->GetPointData()->GetArray(vol)->GetName(); 
            this->GetImageDataInput(0)->GetPointData()->SetActiveScalars( 
                //this->GetImageDataInput(0)->GetPointData()->GetArray(vol)->GetName()
                arrayName0.c_str() 
            );
        }    

        //  input 2 scalar data volume
        string arrayName1;  
        if (this->GetInput(1)) {
            int vol1; 
            if ( numVolumes1 == 1 ) {
                vol1 = 0 ;
            } else {
                vol1 = vol; 
            }
            arrayName1 = this->GetImageDataInput(1)->GetPointData()->GetArray(vol1)->GetName(); 
            this->GetImageDataInput(1)->GetPointData()->SetActiveScalars( 
                //this->GetImageDataInput(1)->GetPointData()->GetArray(vol1)->GetName()
                arrayName1.c_str() 
            );
            if(this->GetPortMapper()->GetBoolInputPortValue(MULTIPLY_IMAGE_1_BY_IMAGE_2_MEDIAN) && this->GetPortMapper()->GetBoolInputPortValue(MULTIPLY_IMAGE_1_BY_IMAGE_2_MEDIAN)->GetValue()){
                //Default values from historical data
                double binSize = 10;
                int numBins = 1000;
                int smoothBins = 21;
                double startBin = 10;
                if( this->GetPortMapper()->GetDoubleInputPortValue(BIN_SIZE_FOR_HISTOGRAM) != NULL ) {
                    binSize = this->GetPortMapper()->GetDoubleInputPortValue(BIN_SIZE_FOR_HISTOGRAM)->GetValue();
                }
                if( this->GetPortMapper()->GetDoubleInputPortValue(START_BIN_FOR_HISTOGRAM) != NULL ) {
                    startBin = this->GetPortMapper()->GetDoubleInputPortValue(START_BIN_FOR_HISTOGRAM)->GetValue();
                }
                if( this->GetPortMapper()->GetIntInputPortValue(NUM_BINS_FOR_HISTOGRAM) != NULL ) {
                    numBins = this->GetPortMapper()->GetIntInputPortValue(NUM_BINS_FOR_HISTOGRAM)->GetValue();
                }
                if( this->GetPortMapper()->GetIntInputPortValue(SMOOTH_BINS_FOR_HISTOGRAM) != NULL ) {
                    smoothBins = this->GetPortMapper()->GetIntInputPortValue(SMOOTH_BINS_FOR_HISTOGRAM)->GetValue();
                }
                vtkDataArray* pixelsImageOne = this->GetImageDataInput(0)->GetPointData()->GetScalars();
                vtkDataArray* pixelsImageTwo = this->GetImageDataInput(1)->GetPointData()->GetScalars();
                if (this->GetInput( svkImageMathematics::MASK_FOR_MEDIAN )) {
                    svkMriImageData* medianMask = svkMriImageData::SafeDownCast(this->GetImageDataInput(svkImageMathematics::MASK_FOR_MEDIAN) );
                    pixelsImageOne = svkStatistics::GetMaskedPixels(svkMriImageData::SafeDownCast(this->GetImageDataInput(0)), medianMask);
                    pixelsImageTwo = svkStatistics::GetMaskedPixels(svkMriImageData::SafeDownCast(this->GetImageDataInput(1)), medianMask);
                }
                vtkDataArray* histogramImageOne = svkStatistics::GetHistogram(pixelsImageOne, binSize, startBin, numBins, smoothBins);
                vector<double> imageOneQuantiles = svkStatistics::ComputeQuantilesFromHistogram(20, histogramImageOne, binSize, startBin, numBins, smoothBins);
                vtkDataArray* histogramImageTwo = svkStatistics::GetHistogram(pixelsImageTwo, binSize, startBin, numBins, smoothBins);
                vector<double> imageTwoQuantiles = svkStatistics::ComputeQuantilesFromHistogram(20, histogramImageTwo, binSize, startBin, numBins, smoothBins);
                // We are using floats here for consistency with historical results.
                this->SetConstantK(((float)(imageTwoQuantiles[10]))/((float)(imageOneQuantiles[10])));
            }

        }    

        //this->Superclass::Update(); 
        if (this->GetOutput()) {

            this->GetOutput()->GetPointData()->SetActiveScalars( 
                //this->GetImageDataInput(0)->GetPointData()->GetArray(vol)->GetName()
                arrayName0.c_str() 
            );
        }    

        int numVoxels[3];
        svkMriImageData::SafeDownCast(this->GetOutput(0))->GetNumberOfVoxels(numVoxels);
        int totalVoxels = numVoxels[0] * numVoxels[1] * numVoxels[2];
        vtkDataArray* in1Array = this->GetImageDataInput(0)->GetPointData()->GetScalars();    // returns a vtkDataArray
        vtkDataArray* in2Array;
        if (this->GetInput(1)) {
            in2Array = this->GetImageDataInput(1)->GetPointData()->GetScalars();    // returns a vtkDataArray
        }
        vtkDataArray* outArray = this->GetOutput()->GetPointData()->GetArray(arrayName0.c_str() );    // returns a vtkDataArray
        int numComponents = outArray->GetNumberOfComponents(); 
        for ( int i = 0; i < totalVoxels; i++ ) {
            if ( this->Operation == VTK_ADD ) {
                outArray->SetTuple1( 
                    i,  
                    in1Array->GetTuple1(i) + in2Array->GetTuple1(i) 
                ); 
            } else if ( this->Operation == VTK_SUBTRACT ) {
                outArray->SetTuple1( 
                    i,  
                    in1Array->GetTuple1(i) - in2Array->GetTuple1(i) 
                ); 
            } else if ( this->Operation == VTK_DIVIDE ) {
                if ( in2Array->GetTuple1(i) == 0 ) {
                    outArray->SetTuple1( 
                        i,  
                        0
                    ); 
                } else {
                    outArray->SetTuple1( 
                        i,  
                        (in1Array->GetTuple1(i)) / in2Array->GetTuple1(i) 
                    ); 
                }
            } else if ( this->Operation == VTK_MULTIPLY ) {
                //  rewrote this to support complex valued image tuples
                for ( int c = 0; c < numComponents; c++ ) {
                    outArray->SetComponent(
                        i, 
                        c, 
                        in1Array->GetComponent(i, c) * in2Array->GetComponent(i, c)
                    );
                }
            } else if ( this->Operation == VTK_MULTIPLYBYK ) {
                outArray->SetTuple1( 
                    i,  
                    in1Array->GetTuple1(i) * this->GetConstantK()  
                ); 
            } else if ( this->Operation == VTK_ADDC) {
                outArray->SetTuple1( 
                    i,  
                    in1Array->GetTuple1(i) + this->GetConstantC()  
                ); 
            } else if ( this->Operation == VTK_SQRT ) {
                outArray->SetTuple1( 
                    i,  
                    sqrt( static_cast<double>(in1Array->GetTuple1(i)) ) 
                ); 
            }
        }

        //cout << "SPOUT post: " << *this->GetOutput(); 
        
        //  Mask the data if a mask was provided.  Preferably the masking would be applied by 
        //  limiting the extent fo the vtkImagetMathematics operation, but I'll just zero out the 
        //  results here for now. 
        if (this->GetInput( svkImageMathematics::MASK )) {
            
            vtkDataArray* outArray = this->GetOutput()->GetPointData()->GetScalars();    // returns a vtkDataArray

            svkMriImageData* maskImage = svkMriImageData::SafeDownCast(this->GetImageDataInput(svkImageMathematics::MASK) );
            unsigned short* mask = static_cast<vtkUnsignedShortArray*>( 
                        maskImage->GetPointData()->GetArray(0) )->GetPointer(0) ; 

            int numVoxels[3];
            svkMriImageData::SafeDownCast(this->GetOutput(0))->GetNumberOfVoxels(numVoxels);
            int totalVoxels = numVoxels[0] * numVoxels[1] * numVoxels[2];

            for ( int i = 0; i < totalVoxels; i++ ) {
                if ( mask[i] == 0 ) { 
                    outArray->SetTuple1( i, 0 );    // set value to 0 outside mask: 
                }
            }
        }

        //  debug
        /*
        vtkDataArray* in1Array0 = this->GetImageDataInput(0)->GetPointData()->GetScalars();    // returns a vtkDataArray
        vtkDataArray* in2Array0 = this->GetImageDataInput(1)->GetPointData()->GetScalars();    // returns a vtkDataArray
        vtkDataArray* outArray0 = this->GetOutput()->GetPointData()->GetArray(arrayName0.c_str() );    // returns a vtkDataArray
        for (int k = 0; k < 1; k++ ) {
            cout << "K: " << k << endl;
            //outArray0 = this->GetOutput()->GetPointData()->GetArray(k);
            cout << "ARRAY 1 " << *in1Array0 << endl;
            cout << "ARRAY 2 " << *in2Array0 << endl;
            cout << "ARRAY o0 " << *outArray0 << endl;
            for ( int i = 1543; i < 1550; i++ ) {
                cout<<"TUPLE Init: " << i << " " << in1Array0->GetTuple1(i) 
                    << " op " << in2Array0->GetTuple1(i) << " " <<outArray0->GetTuple1( i ) << endl; 
            }
        }
        */
        
    }

    //  Now copy the multi-volume output results back into the  algorithm's output object. 
    if( this->GetPortMapper()->GetStringInputPortValue( OUTPUT_SERIES_DESCRIPTION ) != NULL ) {
        string description = this->GetPortMapper()->GetStringInputPortValue( OUTPUT_SERIES_DESCRIPTION )->GetValue();
        svkMriImageData::SafeDownCast(this->GetOutput())->GetDcmHeader()->SetValue("SeriesDescription", description );
    }
    return 1;
}


/*!
 * Pass through method to the internal svkAlgorithmPortMapper
 */
int svkImageMathematics::FillOutputPortInformation( int port, vtkInformation* info )
{
    this->GetPortMapper()->FillOutputPortInformation(port, info );

    return 1;
}


/*!
 * Pass through method to the internal svkAlgorithmPortMapper
 */
int svkImageMathematics::FillInputPortInformation( int port, vtkInformation* info )
{
    this->GetPortMapper()->FillInputPortInformation(port, info );

    return 1;
}


