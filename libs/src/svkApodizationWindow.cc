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


#include <svkApodizationWindow.h>
#include </usr/include/vtk/vtkGlobFileNames.h>

using namespace svk;

//vtkCxxRevisionMacro(svkApodizationWindow, "$Rev$");
vtkStandardNewMacro(svkApodizationWindow);

//! Constructor
svkApodizationWindow::svkApodizationWindow()
{
}


//! Destructor
svkApodizationWindow::~svkApodizationWindow()
{
}


/*!
 *  Creates a lorentzian window using the equation:
 *  f(t) = e^(-fwhh * PI * dt) from t = 0 to t = N where N the number of tuples in the input array.
 *
 *  NOTE: This window has the properties specified for fwhh in the FREQUENCY DOMAIN not in the
 *        time domain.
 *
 *  \param window    Pre-allocated array that will be populated with the window. 
 *                   The number of tuples allocated determines the number of points in the window.
 *
 *  \param fwhh      Defines the shape of the Lorentzian, also know as the line broadening parameter. 
 *                   Value in Hz. 
 *
 *  \param dt        Temporal resolution of the window in seconds.
 *
 */
void svkApodizationWindow::GetLorentzianWindow( vector < vtkFloatArray* >* window,  float fwhh, float dt )
{
    if( window != NULL ) {

        int numPoints = (*window)[0]->GetNumberOfTuples();
        int numComponents = (*window)[0]->GetNumberOfComponents();

        for( int i = 0; i < numPoints; i++ ) {
            // NOTE: fabs is used here in case we want to alter the center of the window in the future.
            float value = exp( -fwhh * vtkMath::Pi()* fabsf( dt * i ) );
            for( int j = 0; j < numComponents; j++ ) {
				(*window)[0]->SetComponent( i, j, value );
            }
        }
    } 
}


/*!
 *  Creates a lorentzian window using the equation:
 *  f(t) = e^(-fwhh * PI * dt) from t = 0 to t = N where N the number of tuples in the input array.
 *  Extracts parameters from the DICOM header to determine the dt parameter.
 *
 *  \param window    Array that will be populated with the window.
 *                   The number of tuples allocated determines the number of points in the window.
 *
 *  \param data      The data set to generate the window for. Based on the type the number of points
 *                   and dt are determined.
 *
 *  \param fwhh      Defines the shape of the Lorentzian, also know as the line broadening parameter.
 *                   Value in Hz.
 *
 */
void svkApodizationWindow::GetLorentzianWindow( vector < vtkFloatArray*>* window, svkImageData* data, float fwhh )
{
	float dt = 0;
	if( data != NULL ) {
		dt = svkApodizationWindow::GetWindowResolution(data);
	}
    if( data->IsA("svkMrsImageData") && window != NULL && dt != 0 ) {
        if ( window->size() == 0 ) { 
            vtkFloatArray* window1D = vtkFloatArray::New(); 
            window->push_back( window1D ); 
        }    
    	svkApodizationWindow::InitializeWindowSpectral( window, data );
        svkApodizationWindow::GetLorentzianWindow( window, fwhh, svkApodizationWindow::GetWindowResolution( data ) );
    } else {
        cout << "WINDOW: " << window << endl;
         vtkErrorWithObjectMacro(data, "Could not generate Lorentzian window for give data type!");
    }
}


/*!
 *  Creates a gaussian window using the equation:
 *  f(t) = e^( -0.5 * (fwhh * Pi* ( t - center ))/sqrt(2*log(2)))^2 ) from t = 0 to t = N where N the number of tuples in the input array.
 *
 *  NOTE: This window has the properties specified for fwhh in the FREQUENCY DOMAIN not in the
 *        time domain. The center is defined in ms in the time domain.
 *
 *  \param window    Pre-allocated array that will be populated with the window.
 *                   The number of tuples allocated determines the number of points in the window.
 *
 *  \param fwhh      Defines the shape of the gaussian, also know as the line broadening parameter.
 *                   Value in Hz.
 *
 *  \param dt        Temporal resolution of the window in seconds.
 *
 *  \param center    The center point for the window in ms.
 *
 */
void svkApodizationWindow::GetGaussianWindow( vector < vtkFloatArray* >* window, float fwhh, float dt, float center )
{
    if( window != NULL ) {
        int numPoints = (*window)[0]->GetNumberOfTuples();
        int numComponents = (*window)[0]->GetNumberOfComponents();
        for( int i = 0; i < numPoints; i++ ) {
            float value = exp( -0.5 * pow((fwhh * vtkMath::Pi()* ( dt * i - center/1000 ))/pow(2*log(2.),0.5),2) );
            for( int j = 0; j < numComponents; j++ ) {
				(*window)[0]->SetComponent( i, j, value );
            }
        }
    }
}


/*!
 *  Creates a gaussian window using the equation:
 *  f(t) = e^((-(t-center)^2 *4*ln(2))/(fwhh(Hz) * dt)^2  ) from t = 0 to t = N where N the number of tuples in the input array.
 *
 *  \param window    Pre-allocated array that will be populated with the window.
 *                   The number of tuples allocated determines the number of points in the window.
 *
 *  \param fwhh      Defines the shape of the gaussian, also know as the line broadening parameter.
 *                   Value in Hz.
 *
 *  \param dt        Temporal resolution of the window in seconds.
 *
 *  \param center    The center point for the peak of the Gaussian.
 *
 */
void svkApodizationWindow::GetGaussianWindow( vector < vtkFloatArray* >* window, svkImageData* data, float fwhh, float center )
{
	float dt = 0;
	if( data != NULL ) {
		dt = svkApodizationWindow::GetWindowResolution(data);
	}
    if( data->IsA("svkMrsImageData") && window != NULL && dt != 0 ) {
        if ( window->size() == 0 ) { 
            vtkFloatArray* window1D = vtkFloatArray::New(); 
            window->push_back( window1D ); 
        }    
    	svkApodizationWindow::InitializeWindowSpectral( window, data );
        svkApodizationWindow::GetGaussianWindow( window, fwhh, svkApodizationWindow::GetWindowResolution( data ), center );
    } else {
         vtkErrorWithObjectMacro(data, "Could not generate Gaussian window for give data type!");
    }
}


/*!
 *  Creates a vector of 3 Hamming windows using the equation, 1 for each of the 3 spatial dimensions:
 *
 *  H(k) = .54 - .46 * cos( 2*PI(k/K-1), where the maximum is at k=0 and K is the number of 
 *          kspace points in a dimension. 
 *
 *  \param window    vector ocated array that will be populated with the window.
 *                   The number of tuples allocated determines the number of points in the window.
 *
 */
void  svkApodizationWindow::GetHammingWindow( vector < vtkFloatArray* >* window, svkImageData* data, svkApodizationWindow::Dimension dimension )
{
    if( data->IsA("svkMrsImageData") && window != NULL ) {

        //  TODO: Verify that the data is in k-space, all 3 dimensions, otherwise exit
        cout << "===============================================" << endl;
        cout << "TODO:  Verify that data is in k-space" << endl;
        cout << "===============================================" << endl;

        if ( window->size() == 0 ) {
            //  create 3 1D filters, one for each spatial dimension
            for ( int i = 0; i < 3; i++ ) {
                vtkFloatArray* window1D = vtkFloatArray::New();
                window->push_back( window1D );
            }
        }
        svkApodizationWindow::InitializeWindowSpatial( window, data );
        svkApodizationWindow::GetHammingWindowData( window, data, dimension );
    } else {
        cout << "WINDOW: " << window << endl;
        vtkErrorWithObjectMacro(data, "Could not generate Hamming window for give data type!");
    }
}


/*!
 *  Creates a vector of 3 Hamming windows using the equation, 1 for each of the 3 spatial dimensions:
 *
 *  H(k) = .54 - .46 * cos(( 2*PI)/(k/K-1)), where the maximum is at k=0 and K is the number of 
 *          kspace points in a dimension. 
 *
 *  \param window    Pre-allocated array that will be populated with the window.
 *                   The number of tuples allocated determines the number of points in the window.
 */
 
void svkApodizationWindow::GetHammingWindowData( vector < vtkFloatArray* >* window, svkImageData* data, svkApodizationWindow::Dimension dimension )
{

    if( window != NULL ) {

        // loop over each dimension. 
        for ( int dim = 0; dim < 3; dim++ ) {

            int numVoxels = (*window)[dim]->GetNumberOfTuples();
            int numComponents = (*window)[dim]->GetNumberOfComponents();
            
            for( int i = 0; i < numVoxels; i++ ) {

                //  Only set hamming window in requested dimensions, others set to 1: 
                float hamming = 1;  
                if ( (dim == dimension || dimension == svkApodizationWindow::THREE_D) && numVoxels > 1) {  
                    float N = svkApodizationWindow::GetWindowExpansion( data, numVoxels);
                    hamming = 0.54 - 0.46 * (cos ((2 * vtkMath::Pi() * i)/(N - 1)));
                }
                
                for( int j = 0; j < numComponents; j++ ) {
                    (*window)[dim]->SetComponent( i, j, hamming);
                }
            }
        }
    }
}


/*!
 *  Below is a truth table that determines the size of the window under different conditions.
 */

float svkApodizationWindow::GetWindowExpansion( svkImageData* data, int numVoxels )
{
    string k0Sampled = data->GetDcmHeader()->GetStringValue( "SVK_K0Sampled" );

    float N;

    if ( k0Sampled.compare("YES") == 0 ) {
        if (numVoxels%2 == 0) {
	        N = numVoxels + 1;
        } else {
	        N = numVoxels;
        }
    } else {
        if (numVoxels%2 == 0) {
	        N = numVoxels;
        } else {
            cout << "Error: svkApodizationWindow::GetWindowExpansion, numVoxels is too small." << endl;
            exit(1);
        }
    }
    return N;
}




/*!
 * Determines the number of components and number of points appropriate for the window array.
 *
 *  \param data The data for which you wish to get the required window resolution
 *
 *  \return the resolution, 0 is for failure
 *
 */
void  svkApodizationWindow::InitializeWindowSpectral( vector < vtkFloatArray* >* window, svkImageData* data )
{
    if( data->IsA("svkMrsImageData") && window != NULL ) {

        // Lets determine the number of points in our array
        int numPoints     = data->GetDcmHeader()->GetIntValue( "DataPointColumns" );
		int numComponents = 1;

		// And the number of components
		string representation =  data->GetDcmHeader()->GetStringValue( "DataRepresentation" );
		if (representation.compare( "COMPLEX" ) == 0 ) {
			numComponents = 2;
		}

        // Lets set the number of components and the number of tuples
        (*window)[0]->SetNumberOfComponents ( numComponents );
        (*window)[0]->SetNumberOfTuples( numPoints );
    }
}



/*!
 * Determines the spatial extent of image and allocates appropriate size window arrays.
 *
 *  \param data The data for which you wish to get the required window resolution
 *
 *  \return the resolution, 0 is for failure
 *
 */
void  svkApodizationWindow::InitializeWindowSpatial( vector < vtkFloatArray* >* window, svkImageData* data )
{
    if( data->IsA("svkMrsImageData") && window != NULL ) {

        // Determine the number of voxels in each of 3 spatial dimensions. 
        int numVoxels[3]; 
        data->GetNumberOfVoxels( numVoxels ); 


        // And the number of components
        int numComponents = 1;
        string representation =  data->GetDcmHeader()->GetStringValue( "DataRepresentation" );
        if (representation.compare( "COMPLEX" ) == 0 ) {
            numComponents = 2;
        }

        // Lets set the number of components and the number of tuples
        for ( int dim = 0; dim < 3; dim++ ) {
            (*window)[dim]->SetNumberOfComponents ( numComponents );
            (*window)[dim]->SetNumberOfTuples( numVoxels[dim] );
        }
    }
}


/*!
 * Determines the window resolution from the input dataset.
 *
 *  \param data The data for which you wish to get the required window resolution
 *
 *  \return the resolution, 0 is for failure
 *
 */
float svkApodizationWindow::GetWindowResolution( svkImageData* data )
{
	float windowResolution = 0;
    if( data->IsA("svkMrsImageData") ) {
        // Lets determine the point resolution for the window
        float spectralWidth = data->GetDcmHeader()->GetFloatValue( "SpectralWidth" );
        windowResolution = 1.0/spectralWidth;
    }
    return windowResolution;
}




