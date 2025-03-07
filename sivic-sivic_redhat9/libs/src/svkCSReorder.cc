//
// Created by bolson on 9/21/18.
//

#include "svkCSReorder.h"
using namespace svk;

//vtkCxxRevisionMacro(svkBool, "$Rev$");
vtkStandardNewMacro(svkCSReorder);

//! Constructor
svkCSReorder::svkCSReorder()
{
    this->dadFile = NULL;
}


//! Destructor
svkCSReorder::~svkCSReorder()
{

}

void svkCSReorder::ReOrderAndPadData(float* originalData, int numberDataPointsInFIDFile, float*** paddedData)
{
    float localReorderedData[ numberDataPointsInFIDFile ];
    this->ReOrderData(localReorderedData, originalData);
    this->PadData(paddedData, localReorderedData);
}


/*!
 *  Reorder FID data encoding
 */
void svkCSReorder::ReOrderData(float* reorderedData, float* originalData)
{

    svkDataAcquisitionDescriptionXML* dadFile = this->dadFile;
    //  Phase encoding order for kx, ky:
    int numKxEncodes = this->dadFile->GetEncodedMatrixSizeDimensionValue(0);
    int numKyEncodes = this->dadFile->GetEncodedMatrixSizeDimensionValue(1);
    int numKzEncodes = this->dadFile->GetEncodedMatrixSizeDimensionValue(2);
    int lobes = this->dadFile->GetEncodedMatrixSizeDimensionValue(3);      //  echoes
    int numEchoTrains = 0;

    int samplingMask[numKxEncodes * numKyEncodes];
    int samplingMaskIndicies[numKxEncodes * numKyEncodes];
    int samplingIndicies[numKxEncodes * numKyEncodes];



    dadFile->GetSamplingMask(samplingMask);
    int i = 0;
    numEchoTrains = 0;
    for (int s = 0; s < numKxEncodes; s++) {
        for (int f = 0; f < numKyEncodes; f++) {
            samplingMaskIndicies[i] = samplingMask[i] * (i + 1);             //  flatten 2D=>1D array
            if( samplingMask[i] > 0 ) {
                numEchoTrains++;
            }
            i++;
        }
    }
    dadFile->GetSamplingIndicies(samplingIndicies);

    int sampledIndicies[numKxEncodes * numKyEncodes];
    int j = 0;
    for ( int i = 0; i < numKxEncodes * numKyEncodes; i++ ) {   //  note only 76 echotrains acquired, not 256
        int index = samplingIndicies[i];      //  full centric acqn order
        if ( samplingMaskIndicies[index] > 0) {          //  check if data acquired
            //  acqn order; no zeros; 76 entries;
            //  sampledIndicies: 121   120   136   137   138   122   106   105...
            sampledIndicies[j] = index;
            j++;
        }
    }


    int reorderedIndicies[numKxEncodes * numKyEncodes];
    j = 0;
    for ( int i = 0; i < numKxEncodes * numKyEncodes; i++ ) {   //  note only 76 echotrains acquired, not 256
        if (samplingMaskIndicies[i] > 0) {               //  check if data acquired
            //  linear acqn order; no zeros; 76 entries;
            //  reorderedIndicies: 1     3     5     7     9    11    13    15    33...
            reorderedIndicies[j] = samplingMaskIndicies[i];
            j++;
        }
    }

    int np = numKzEncodes * 2;            //  re+im
    int fidSize = lobes * np;   //  float words re+im; z+f points; etl*np*4 bytes

    int out;
    int offsetIn;
    int offsetOut;
    for (int i = 0; i < numEchoTrains; i++) {
        out = reorderedIndicies[i];                            //  output index
        for (int k = 0; k < numEchoTrains; k++) {
            // TODO: Check if this + 1 is just a bug from the original translation
            if ( sampledIndicies[k] + 1 == out) {
                offsetIn  = k * fidSize;     //  offset into input data
                offsetOut = i * fidSize;              //  offset to output data
                for (int j = 0; j < fidSize; j++ ) {
                    reorderedData[offsetOut] = originalData[offsetIn];
                    offsetOut++;
                    offsetIn++;
                }
            }
        }
    }
}


void svkCSReorder::PadData(float*** paddedData, float* specDataReordered)
{

    int lengthX = this->dadFile->GetEncodedMatrixSizeDimensionValue(0);
    int lengthY = this->dadFile->GetEncodedMatrixSizeDimensionValue(1);
    int lengthZ = this->dadFile->GetEncodedMatrixSizeDimensionValue(2);
    int lengthF = this->dadFile->GetEncodedMatrixSizeDimensionValue(3);
    int**  encodeMatrix = new int*[lengthX];
    for (int i = 0; i < lengthX; i++ ) {
        encodeMatrix[i] = new int[lengthY];
    }
    //  init to 0
    for (int x = 0; x < lengthX; x++ ) {
        for (int y = 0; y < lengthY; y++ ) {
            encodeMatrix[x][y] = 0;
        }
    }



    int A1DBinary[lengthX * lengthY];
    dadFile->GetSamplingMask(A1DBinary);
    int linearCounter = 0;
    for (int y = 0; y < lengthY; y++) {
        for (int x = 0; x < lengthX; x++) {
            encodeMatrix[x][y] = A1DBinary[linearCounter];
            linearCounter++;
        }
    }
    


    //==================================
    //  Zero pad the data
    //==================================
    //  X = X(16:length(X));
    //  Allocate space for a complex array of  lengthY * lengthX * specPts:
    //  59 samples, 16 flyback phase encodes:
    int specPts = lengthF*lengthZ;

    float*** paddedDataTmp = new float**[lengthY];
    int*** xBlips          = new int**[lengthY];
    int*** yBlips          = new int**[lengthY];
    for (int y = 0; y < lengthY; y++ ) {
        paddedData[y] = new float*[lengthX];
        paddedDataTmp[y]    = new float*[lengthX];
        xBlips[y]           = new int*[lengthX];
        yBlips[y]           = new int*[lengthX];
        for (int x = 0; x < lengthX; x++ ) {
            paddedData[y][x] = new float[ specPts * 2 ];
            paddedDataTmp[y][x]    = new float[ specPts * 2 ];
            xBlips[y][x]           = new int[ lengthF ];
            yBlips[y][x]           = new int[ lengthF ];
        }
    }

    for (int y = 0; y < lengthY; y++ ) {
        for (int x = 0; x < lengthX; x++ ) {
            for (int s = 0; s < specPts * 2; s++ ) {
                paddedData[y][x][s] = 0.;
                paddedDataTmp[y][x][s]    = 0.;
            }
            for (int s = 0; s < lengthF; s++ ) {
                xBlips[y][x][s]    = 0;
                yBlips[y][x][s]    = 0;
            }
        }
    }


    //  Check, can I write out zero data array
    //  next, fill in with real values: 
    int startIndex = 0;
    int numSkip = 0;
    int counter = 0;

    for (int y = 0; y < lengthY; y++) {
        for (int x = 0; x < lengthX; x++) {
            if ( encodeMatrix[x][y] > 0) {

                //  loop over 59 lobes:
                for (int f = 0; f < lengthF; f++) {

                    //  each of the 59 lobe cycles has a length of (lengthZ)
                    int padIndStart = f * ( lengthZ * 2);             // target index of zero padded matrix
                    int dataIndStart = (counter * 2 * lengthZ * lengthF) + (f * 2 * lengthZ);

                    int counter2 = dataIndStart;

                    for (int s = padIndStart; s < padIndStart + lengthZ * 2; s += 2) {
                        //  real and imaginary values: 
                        paddedDataTmp[y][x][s]       =  specDataReordered[ counter2 ];
                        paddedDataTmp[y][x][ s + 1 ] =  specDataReordered[ counter2 + 1];
                        counter2 = counter2 + 2;
                    }
                }
                counter++;
            }
        }
    }


    /*! 
     *  The next block of code is reimplemented from Simon Hu's func_reorder_blipped_data.m
     *  Takes blipped data, presumably something like (16*59)x16x16 flyback data,
     *  and based on the blips used, puts the data into the correct view
     *  locations.
     *  Note that unlike the matlab implementation, this version does not artificially zero
     *  zero fill the flyback plateau to 43 points.   
     *
     *  This just reorders where the x and y phase encodes locations, but leaves the 
     *  flyback ordering as is. 
     *  matlab function: ordered_data = 
     *      orderData(data, specpts, xlen, ylen, flen, pts_lobe, 
     *                encodeMatrix, xblips, yblips, blip_phase_correct, phi_x, phi_y);
     */


    int linearIndex = 0;
    int index = 0;
    for (int y = 0; y < lengthY; y++) {
        for (int x = 0; x < lengthX; x++) {
            if ( encodeMatrix[x][y] > 0) {

                int prevStateX = 0;
                int prevStateY = 0;

                int numComponents = 2;
                int xBlipsDad[lengthF];
                int yBlipsDad[lengthF];

                this->dadFile->GetBlips(linearIndex, "x", xBlipsDad);
                this->dadFile->GetBlips(linearIndex, "y", yBlipsDad);

                for (int f = 0; f < lengthF; f++) {
                    int currStateX = 0;
                    int currStateY = 0;

                    currStateX = prevStateX - xBlipsDad[f];
                    currStateY = prevStateY - yBlipsDad[f];


                    index =  f * (lengthZ * numComponents);

                    for (int s = index; s < (index + (lengthZ * numComponents)) ; s += 2) {

                        paddedData[y + currStateY][x + currStateX][s] =
                                paddedDataTmp[y][x][s];
                        paddedData[y + currStateY][x + currStateX][s+1] =
                                paddedDataTmp[y][x][s+1];

                    }

                    prevStateX = currStateX;
                    prevStateY = currStateY;

                }
            }
            linearIndex++;
        }
    }

    //++++++++++++++++++++++++++++++++++++

    for (int i = 0; i < lengthX; i++ ) {
        delete [] encodeMatrix[i];
    }
    delete [] encodeMatrix;




    for (int y = 0; y < lengthY; y++ ) {
        for (int x = 0; x < lengthX; x++ ) {
            delete [] paddedDataTmp[y][x];
            delete [] xBlips[y][x];
            delete [] yBlips[y][x];
        }
        delete [] paddedDataTmp[y];
        delete [] xBlips[y];
        delete [] yBlips[y];
    }

    delete [] paddedDataTmp;
    delete [] xBlips;
    delete [] yBlips;

}


void svkCSReorder::SetDADFilename(string dadFilename) {
    if( this->dadFile == NULL ) {
        string stringFileName = svkUtils::GetPathFromFilename(dadFilename);
        vtkGlobFileNames* glob = vtkGlobFileNames::New();
        stringFileName.append("*.xml");
        glob->AddFileNames(stringFileName.c_str());
        if( glob->GetNumberOfFileNames() == 1 ) {
            this->dadFile = svkDataAcquisitionDescriptionXML::New();
            this->dadFile->SetXMLFileName( glob->GetNthFileName(0));
        }

    }
}
