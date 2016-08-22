/*
 *  Copyright © 2009-2014 The Regents of the University of California.
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
#include <vtkGlobFileNames.h>

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
void svkApodizationWindow::GetLorentzianWindow( vtkFloatArray* window,  float fwhh, float dt )
{
    if( window != NULL ) {

        int numPoints = window->GetNumberOfTuples();
        int numComponents = window->GetNumberOfComponents();

        for( int i = 0; i < numPoints; i++ ) {
            // NOTE: fabs is used here in case we want to alter the center of the window in the future.
            float value = exp( -fwhh * vtkMath::Pi()* fabsf( dt * i ) );
            for( int j = 0; j < numComponents; j++ ) {
				window->SetComponent( i, j, value );
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
void svkApodizationWindow::GetLorentzianWindow( vtkFloatArray* window, svkImageData* data, float fwhh )
{
	float dt = 0;
	if( data != NULL ) {
		dt = svkApodizationWindow::GetWindowResolution(data);
	}
    if( data->IsA("svkMrsImageData") && window != NULL && dt != 0 ) {
    	svkApodizationWindow::InitializeWindow( window, data );
        svkApodizationWindow::GetLorentzianWindow( window, fwhh, svkApodizationWindow::GetWindowResolution( data ) );
    } else {
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
void svkApodizationWindow::GetGaussianWindow( vtkFloatArray* window, float fwhh, float dt, float center )
{
    if( window != NULL ) {
        int numPoints = window->GetNumberOfTuples();
        int numComponents = window->GetNumberOfComponents();
        for( int i = 0; i < numPoints; i++ ) {
            float value = exp( -0.5 * pow((fwhh * vtkMath::Pi()* ( dt * i - center/1000 ))/pow(2*log(2.),0.5),2) );
            for( int j = 0; j < numComponents; j++ ) {
				window->SetComponent( i, j, value );
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
void svkApodizationWindow::GetGaussianWindow( vtkFloatArray* window, svkImageData* data, float fwhh, float center )
{
	float dt = 0;
	if( data != NULL ) {
		dt = svkApodizationWindow::GetWindowResolution(data);
	}
    if( data->IsA("svkMrsImageData") && window != NULL && dt != 0 ) {
    	svkApodizationWindow::InitializeWindow( window, data );
        svkApodizationWindow::GetGaussianWindow( window, fwhh, svkApodizationWindow::GetWindowResolution( data ), center );
    } else {
         vtkErrorWithObjectMacro(data, "Could not generate Gaussian window for give data type!");
    }
}


/*!
 * Determines the number of components and number of points appropriate for the window array.
 *
 *  \param data The data for which you wish to get the required window resolution
 *
 *  \return the resolution, 0 is for failure
 *
 */
void  svkApodizationWindow::InitializeWindow( vtkFloatArray* window, svkImageData* data )
{
    if( data->IsA("svkMrsImageData") && window != NULL ) {

        // Lets determine the number of points in our array
        int numPoints     = data->GetDcmHeader()->GetIntValue( "DataPointColumns" );
		int numComponents = 1;

		// And the number of components
		vtkstd::string representation =  data->GetDcmHeader()->GetStringValue( "DataRepresentation" );
		if (representation.compare( "COMPLEX" ) == 0 ) {
			numComponents = 2;
		}

        // Lets set the number of components and the number of tuples
        window->SetNumberOfComponents ( numComponents );
        window->SetNumberOfTuples( numPoints );
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
