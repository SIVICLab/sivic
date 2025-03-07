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


#include <svkMultiWindowToImageFilter.h>

using namespace svk;


//vtkCxxRevisionMacro(svkMultiWindowToImageFilter, "$Rev$");
vtkStandardNewMacro(svkMultiWindowToImageFilter);


/*!
 *
 */
svkMultiWindowToImageFilter::svkMultiWindowToImageFilter()
{

#if VTK_DEBUG_ON
    this->DebugOn();
#endif

    this->SetNumberOfOutputPorts(1);
    this->padConstant = 0;
}


/*!
 *
 */
svkMultiWindowToImageFilter::~svkMultiWindowToImageFilter()
{

}


/*!
 *
 */
void svkMultiWindowToImageFilter::SetInput(vtkRenderWindow* renWin, int indX, int indY, int magnification)
{
    int i;

    //  Are there enough rows? If not, allocate a new vector for each required row:
    if (indY >=  vtkRenderWindowArray.size() ) {
        for (i = vtkRenderWindowArray.size(); i <= indY; i++) { 
            vtkRenderWindowArray.push_back(vector<vtkRenderWindow*>()) ;
            magnificationVector.push_back(vector<int>());
        }    
    }

    int colSize = vtkRenderWindowArray[indY].size();  

    if (indX >=  colSize ) {
        for (i = colSize; i < indX; i++) { 
            vector<vtkRenderWindow*>::iterator it = vtkRenderWindowArray[indY].begin();
            vtkRenderWindowArray[indY].insert( it + i,  NULL) ;
            vector<int>::iterator magIt = magnificationVector[indY].begin();
            magnificationVector[indY].insert( magIt + i,  0) ;
        }
        vector<vtkRenderWindow*>::iterator it = vtkRenderWindowArray[indY].begin();
        vtkRenderWindowArray[indY].insert(it + indX, renWin) ;
        vector<int>::iterator magIt = magnificationVector[indY].begin();
        magnificationVector[indY].insert(magIt + indX, magnification) ;
    } else {
        vector<vtkRenderWindow*>::iterator it = vtkRenderWindowArray[indY].begin();
        vtkRenderWindowArray[indY].insert(it + indX, renWin) ;
        vector<int>::iterator magIt = magnificationVector[indY].begin();
        magnificationVector[indY].insert(magIt + indX, magnification) ;
    }

    // DEBUG
    if (0) {
        for (i = 0; i < vtkRenderWindowArray.size(); i++) { 
            cout << "row " << i << " => " ;
            for (int j = 0; j < vtkRenderWindowArray[i].size(); j++) {
                cout <<  vtkRenderWindowArray[i][j] << " " << magnification << " " ; 
            } 
            cout << endl;
        }
        cout << endl << endl;
    }
}



int svkMultiWindowToImageFilter::ProcessRequest(vtkInformation* request, vtkInformationVector** inputVector,  vtkInformationVector* outputVector)
{

    vtkDebugMacro(<<"svkMultiWindowToImageFilter::ProcessRequest()");


    // generate the data
    if( request->Has( vtkDemandDrivenPipeline::REQUEST_DATA() ) ) {
        this->RequestData(request, inputVector, outputVector);
        return 1;
    }

    // execute information
    if( request->Has( vtkDemandDrivenPipeline::REQUEST_INFORMATION() ) ) {
        this->RequestInformation(request, inputVector, outputVector);
        return 1;
    }

    return this->Superclass::ProcessRequest(request, inputVector, outputVector);

}


/*!
 *
 */
int svkMultiWindowToImageFilter::RequestData(vtkInformation* request, vtkInformationVector** inputVector,  vtkInformationVector* outputVector)
{

    vtkDebugMacro(<<"svkMultiWindowToImageFilter::RequestData()");

    vtkInformation* outInfo = outputVector->GetInformationObject(0);
    vtkImageData* outImage = vtkImageData::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));
    outImage->SetExtent( outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_EXTENT()));
    outImage->AllocateScalars();

    // DEBUG
    if (0) {
        cout << "check outputVector: " << *outputVector<< endl;
        cout << "check outInfo " << *outInfo << endl;
        cout << "check outImage: " << *outImage << endl;
    }


    /*
     *  Fill output vtkImageData object with output from individual 
     *  vtkWindowToImageFilter outputs: 
     *  Loop over each input window, raster across x, then y rows
     *  appending in X direction, pad each individual window to the max ht from that row
     *  i.e. rowHeightVector[y] and max width for that column (colWidthVector[x]).  
     */
    vtkImageConstantPad* pad;

    vtkImageAppend* rowAppend= vtkImageAppend::New();
    rowAppend->SetAppendAxis(1);


    for (int y = 0; y < vtkRenderWindowArray.size(); y++) { 
        vtkImageAppend* colAppend= vtkImageAppend::New();
        colAppend->SetAppendAxis(0);
        for (int x = 0; x < vtkRenderWindowArray[y].size(); x++) {
            pad = vtkImageConstantPad::New();
            pad->SetConstant(padConstant);
            if (vtkRenderWindowArray[y][x] != NULL) {
                vtkWindowToImageFilter* w2if = vtkWindowToImageFilter::New();
                vtkRenderLargeImage* largeImageRenderer = vtkRenderLargeImage::New();
                largeImageRenderer->SetInput( vtkRenderWindowArray[y][x]->GetRenderers()->GetFirstRenderer() );
                largeImageRenderer->SetMagnification(magnificationVector[y][x]);
                //w2if->SetInput( static_cast<vtkWindow*>( vtkRenderWindowArray[y][x]) );
                //w2if->Update();
                //pad->SetInput( w2if->GetOutput() );
                pad->SetInput( largeImageRenderer->GetOutput() );
                //w2if->Delete();
            } else {
                //  If no window was provided for this index, fill with a blank tile
                vtkImageData* blank = vtkImageData::New();
                blank->SetDimensions(1,1,1);
                blank->SetSpacing(1,1,1);
                blank->SetNumberOfScalarComponents(3);
                blank->SetScalarType(3);
                blank->Update();
                pad->SetInput(blank); 
                blank->Delete();
            }
            //pad to max 
            pad->SetOutputWholeExtent( 0, colWidthVector[x]-1, 0, rowHeightVector[y]-1, 0, 0 );
            pad->Update();
            colAppend->AddInput( pad->GetOutput() );
            colAppend->Update();
            pad->Delete();
        } 
        rowAppend->AddInput(colAppend->GetOutput());
        colAppend->Delete();
    }
    rowAppend->Update();
    vtkImageFlip* flip = vtkImageFlip::New();
    flip->SetFilteredAxis(1);
    flip->RemoveAllInputs();
    flip->SetInput( rowAppend->GetOutput() );
    flip->Update();

    outImage->DeepCopy(flip->GetOutput());

    rowAppend->Delete();
    flip->Delete();
    
    return 0; 
}


/*!
 *
 */
int svkMultiWindowToImageFilter::RequestInformation(vtkInformation* request, vtkInformationVector** inputVector,  vtkInformationVector* outputVector)
{

    vtkDebugMacro(<<"svkMultiWindowToImageFilter::RequestInformation()");

    int sizeX = 0; 
    int sizeY = 0; 
    int numRowsMax = 0; 
    int numColsMax = 0;     // Max number of columns in a window in a particular colunm of windows
    int longestRow = 0;     //  Row with most windows


    //  Pad Array out to a rectangle in case windows weren't set:
    int numWindowRows = vtkRenderWindowArray.size(); 
    int numWindowColumns = 0; 
    for (int y = 0; y < numWindowRows; y++) { 
        if (vtkRenderWindowArray[y].size() > numWindowColumns) {
            numWindowColumns = vtkRenderWindowArray[y].size(); 
        }    
    }    
    for (int y = 0; y < numWindowRows; y++) { 
        if (vtkRenderWindowArray[y].size() < numWindowColumns) {
            vector<vtkRenderWindow*>::iterator it = vtkRenderWindowArray[y].begin();
            vtkRenderWindowArray[y].insert( 
                it + vtkRenderWindowArray[y].size(),  
                numWindowColumns - vtkRenderWindowArray[y].size(), 
                NULL
            ) ;
        }    
    }    
            

    /* 
     *  Iterate over each column, one row at a time.    
     *  Y size is sum of largest y size from each row 
     *  X size is sum of largest x size from each column 
     *  Set the rowHeight of each row to the max ht of any individual window in that row:
     */ 
    for (int y = 0; y < vtkRenderWindowArray.size(); y++) { 
        numRowsMax = 0; 
        for (int x = 0; x < vtkRenderWindowArray[y].size(); x++) {
            if ( vtkRenderWindowArray[y].size() > longestRow ) {
                longestRow = vtkRenderWindowArray[y].size(); 
            }
            if (vtkRenderWindowArray[y][x] != NULL) {
                if ( (vtkRenderWindowArray[y][x]->GetSize())[1] * magnificationVector[y][x]  > numRowsMax) {
                    numRowsMax = (vtkRenderWindowArray[y][x]->GetSize())[1] * magnificationVector[y][x] ; 
                }
            }
        } 
        rowHeightVector.push_back(numRowsMax);
        sizeY += numRowsMax ;
    }


    //  For each column in window array, iterate through the rows to find one with maximum width (cols)
    for (int x = 0; x < longestRow;  x++) {
        numColsMax = 0; 

        for (int y = 0; y < vtkRenderWindowArray.size(); y++) { 
            if ( x < vtkRenderWindowArray[y].size() ) {
                if (vtkRenderWindowArray[y][x] != NULL) {
                    if ( (vtkRenderWindowArray[y][x]->GetSize())[0] * magnificationVector[y][x]  > numColsMax) {
                        numColsMax = (vtkRenderWindowArray[y][x]->GetSize())[0] * magnificationVector[y][x] ; 
                    }
                }
            }
        } 
        colWidthVector.push_back(numColsMax);
        sizeX += numColsMax ;
    }

    if (0) {
        cout << " size X Y: " << sizeX << " " << sizeY << endl;
    }

    int wExtent[6];
    wExtent[0] = 0;
    wExtent[1] = sizeX - 1;
    wExtent[2] = 0;
    wExtent[3] = sizeY - 1;
    wExtent[4] = 0;
    wExtent[5] = 0;

    // get the info objects
    vtkInformation* outInfo = outputVector->GetInformationObject(0);

    outInfo->Set(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(), wExtent, 6);
    return 0;

}



/*!
 *
 */
int svkMultiWindowToImageFilter::FillOutputPortInformation(int vtkNotUsed(port), vtkInformation* info)
{   

    vtkDebugMacro(<<"svkMultiWindowToImageFilter::FillOutputPortInformation()");
    // now add our info
    info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkImageData");

    return 1;
}


/*!
 *
 */
vtkImageData* svkMultiWindowToImageFilter::GetOutput()
{
    vtkDebugMacro(<<"svkMultiWindowToImageFilter::GetOutput()");

    return vtkImageData::SafeDownCast(this->GetOutputDataObject(0));

}


/*!
 *  Sets the constant value used for padding.
 */
void svkMultiWindowToImageFilter::SetPadConstant( int padConstant)
{
    this->padConstant = padConstant;

}
