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


#include </usr/include/vtk/vtkInformation.h>
#include </usr/include/vtk/vtkInformationVector.h>
#include </usr/include/vtk/vtkStreamingDemandDrivenPipeline.h>
#include <svkImageMathematics.h>

#include <svkDCEQuantify.h>
#include <svkDCEBasicFit.h>
#include <svkImageCopy.h>

#include <time.h>
#include <sys/stat.h>


using namespace svk;


//vtkCxxRevisionMacro(svkDCEQuantify, "$Rev$");
vtkStandardNewMacro(svkDCEQuantify);


/*!
 *
 */
svkDCEQuantify::svkDCEQuantify()
{
#if VTK_DEBUG_ON
    this->DebugOn();
#endif

    vtkDebugMacro(<< this->GetClassName() << "::" << this->GetClassName() << "()");
    this->SetNumberOfInputPorts(3);
    bool repeatable = true;
    bool required   = true;
    this->GetPortMapper()->InitializeInputPort(INPUT_IMAGE, "INPUT_IMAGE", svkAlgorithmPortMapper::SVK_MR_IMAGE_DATA, required, !repeatable);
    this->GetPortMapper()->InitializeInputPort(START_TIME_PT, "START_TIME_PT", svkAlgorithmPortMapper::SVK_INT, !required);
    this->GetPortMapper()->InitializeInputPort(END_TIME_PT, "END_TIME_PT", svkAlgorithmPortMapper::SVK_INT, !required);

    this->SetNumberOfOutputPorts(6);
    this->GetPortMapper()->InitializeOutputPort(BASE_HT_MAP, "BASE_HT_MAP", svkAlgorithmPortMapper::SVK_MR_IMAGE_DATA);
    this->GetPortMapper()->InitializeOutputPort(PEAK_HT_MAP, "PEAK_HT_MAP", svkAlgorithmPortMapper::SVK_MR_IMAGE_DATA);
    this->GetPortMapper()->InitializeOutputPort(PEAK_TIME_MAP, "PEAK_TIME_MAP", svkAlgorithmPortMapper::SVK_MR_IMAGE_DATA);
    this->GetPortMapper()->InitializeOutputPort(UP_SLOPE_MAP, "UP_SLOPE_MAP", svkAlgorithmPortMapper::SVK_MR_IMAGE_DATA);
    this->GetPortMapper()->InitializeOutputPort(WASHOUT_SLOPE_MAP, "WASHOUT_SLOPE_MAP", svkAlgorithmPortMapper::SVK_MR_IMAGE_DATA);
    this->GetPortMapper()->InitializeOutputPort(WASHOUT_SLOPE_POS_MAP, "WASHOUT_SLOPE_POS_MAP", svkAlgorithmPortMapper::SVK_MR_IMAGE_DATA);
}


/*!
 *
 */
svkDCEQuantify::~svkDCEQuantify()
{
    vtkDebugMacro(<<this->GetClassName()<<"::~"<<this->GetClassName());
}


/*!
 * Utility setter for input port: Timepoint Start
 */
void svkDCEQuantify::SetTimepointStart(int startPt)
{
    this->GetPortMapper()->SetIntInputPortValue(START_TIME_PT, startPt);
}


/*!
 * Utility getter for input port: Timepoint Start
 */
svkInt* svkDCEQuantify::GetTimepointStart()
{
    return this->GetPortMapper()->GetIntInputPortValue(START_TIME_PT);
}


/*!
 * Utility setter for input port: Timepoint End
 */
void svkDCEQuantify::SetTimepointEnd(int endPt)
{
    this->GetPortMapper()->SetIntInputPortValue(END_TIME_PT, endPt);
}


/*!
 * Utility getter for input port: Timepoint End
 */
svkInt* svkDCEQuantify::GetTimepointEnd()
{
    return this->GetPortMapper()->GetIntInputPortValue(END_TIME_PT);
}


/*!
 *  Top level method to call specific map generators.  Could ultimately be a configurable set of algorithms (XML cfg?). 
 */
void svkDCEQuantify::Quantify()
{

    this->GenerateDCEMaps();
    //not sure if this is needed: 
    //this->GenerateNormalizedMaps(); 

}


/*!
 *
 */
void svkDCEQuantify::GenerateDCEMaps()
{
    //  Calculate DCE maps:
    svkDCEBasicFit* dcePkHt = svkDCEBasicFit::New();
    // dcePkHt->SetInput1(this->GetImageDataInput(0));
    dcePkHt->SetInputData(0, this->GetInput(0));
    dcePkHt->SetTimepointStart(this->GetTimepointStart()->GetValue());
    dcePkHt->SetTimepointEnd(this->GetTimepointEnd()->GetValue());
    dcePkHt->SetSeriesDescription("DCE Params");
    dcePkHt->Update();

    this->GetOutput(0)->DeepCopy(dcePkHt->GetOutput(0));
    this->GetOutput(1)->DeepCopy(dcePkHt->GetOutput(1));
    this->GetOutput(2)->DeepCopy(dcePkHt->GetOutput(2));
    this->GetOutput(3)->DeepCopy(dcePkHt->GetOutput(3));
    this->GetOutput(4)->DeepCopy(dcePkHt->GetOutput(4));
    this->GetOutput(5)->DeepCopy(dcePkHt->GetOutput(5));

    dcePkHt->Delete();
}


/*!
 *  Generates the normalized DCE maps.  Maps are normalized
 *  to the normal appearing white matter values from the 
 *  specific mask. 
 */
void svkDCEQuantify::GenerateNormalizedMaps()
{
    //  Calculate DCE Peak Ht map:
    svkDCEBasicFit* dcePkHt = svkDCEBasicFit::New();
    dcePkHt->SetInputData(this->GetInput(0));
    dcePkHt->SetTimepointStart(this->GetTimepointStart()->GetValue());
    dcePkHt->SetTimepointEnd(this->GetTimepointEnd()->GetValue());    
    dcePkHt->SetSeriesDescription("Normalized Peak Height");
    dcePkHt->SetNormalize();
    dcePkHt->Update();
    svkMriImageData* image = svkMriImageData::New();
    image->DeepCopy(dcePkHt->GetOutput());
    dcePkHt->Delete();
}


/*!
 *  Resets the origin and extent for correct initialization of output svkMriImageData object from input 
 *  svkMrsImageData object. 
 */
int svkDCEQuantify::RequestInformation(vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector)
{
    return 1;
}


/*!
 *  Copy the Dcm Header and Provenance from the input to the output. 
 */
int svkDCEQuantify::RequestData(vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector)
{
    this->Quantify();
    return 1; 
};


/*!
 *
 */
void svkDCEQuantify::UpdateProvenance()
{
    vtkDebugMacro(<<this->GetClassName()<<"::UpdateProvenance()");
}

