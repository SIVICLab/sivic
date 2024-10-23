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


#include <svkImageWriterFactory.h>
#include </usr/include/vtk/vtkJPEGWriter.h>
#include </usr/include/vtk/vtkTIFFWriter.h>
#include </usr/include/vtk/vtkPostScriptWriter.h>
#include <svkDICOMSCWriter.h>
#include <svkDICOMMRSWriter.h>
#include <svkDICOMEnhancedMRIWriter.h>
#include <svkDICOMMRIWriter.h>
#include <svkIdfVolumeWriter.h>
#include <svkDdfVolumeWriter.h>
#include <svkLCModelRawWriter.h>


using namespace svk;


//vtkCxxRevisionMacro(svkImageWriterFactory, "$Rev$");
vtkStandardNewMacro(svkImageWriterFactory);


int svkImageWriterFactory::seriesNumberOffset = 7700; 


/*! 
 *
 */
svkImageWriterFactory::svkImageWriterFactory()
{
}


/*! 
 *
 */
svkImageWriterFactory::~svkImageWriterFactory()
{
}


/*! 
 *  
 */
vtkImageWriter* svkImageWriterFactory::CreateImageWriter( svkImageWriterFactory::WriterType writerType )
{

    if ( writerType == svkImageWriterFactory::JPEG ) {
        return vtkJPEGWriter::New();
    } else if ( writerType == svkImageWriterFactory::TIFF ) {
        return vtkTIFFWriter::New();
    } else if ( writerType == svkImageWriterFactory::PS ) {
        return vtkPostScriptWriter::New();
    } else if ( writerType == svkImageWriterFactory::DICOM_SC ) {
        return svkDICOMSCWriter::New();
    } else if ( writerType == svkImageWriterFactory::DICOM_MRS ) {
        return svkDICOMMRSWriter::New();
    } else if ( writerType == svkImageWriterFactory::DICOM_ENHANCED_MRI ) {
        return svkDICOMEnhancedMRIWriter::New();
    } else if ( writerType == svkImageWriterFactory::DICOM_MRI ) {
        return svkDICOMMRIWriter::New();
    } else if ( writerType == svkImageWriterFactory::IDF ) {
        return svkIdfVolumeWriter::New();
    } else if ( writerType == svkImageWriterFactory::DDF ) {
        return svkDdfVolumeWriter::New();
    } else if ( writerType == svkImageWriterFactory::LCMODEL ) {
        return svkLCModelRawWriter::New();
    } else {
        return NULL;  
    }

} 


/*!
 * Get the file default writer type for the given filename.
 */
svkImageWriterFactory::WriterType  svkImageWriterFactory::GetDefaultWriterForFilePattern(string newSeriesFilePattern )
{
    WriterType writerType = svkImageWriterFactory::UNDEFINED;
    size_t pos = newSeriesFilePattern.rfind('.');
    if( pos != string::npos ) {
        string extension = newSeriesFilePattern.substr(pos);
        if( extension == ".jpg" || extension == ".jpeg") {
            writerType = svkImageWriterFactory::JPEG;
        } else if( extension == ".tif" || extension == ".tiff") {
            writerType = svkImageWriterFactory::TIFF;
        } else if( extension == ".ps" ) {
            writerType = svkImageWriterFactory::PS;
        } else if( extension == ".dcm") {
            writerType = svkImageWriterFactory::DICOM_ENHANCED_MRI;
        } else if( extension == ".DCM") {
            writerType = svkImageWriterFactory::DICOM_ENHANCED_MRI;
        } else if( extension == ".idf") {
            writerType = svkImageWriterFactory::IDF;
        } else if( extension == ".ddf") {
            writerType = svkImageWriterFactory::DDF;
        } else if( extension == ".raw" || extension == ".control" ) {
            writerType = svkImageWriterFactory::LCMODEL;
        }
    }
    return writerType;
}


/*!
 *  Generates a default file pattern for a new series and returns the new series number: 
 *  EStudyIdSSeriesNumberI
 */
int svkImageWriterFactory::GetNewSeriesFilePattern(svkImageData* imageData, string* newSeriesFilePattern  )
{
    string studyId = imageData->GetDcmHeader()->GetStringValue("StudyID");
    int seriesNumberInt = imageData->GetDcmHeader()->GetIntValue("SeriesNumber");
    int seriesOffset = svkImageWriterFactory::GetNewSeriesNumberOffset();
    seriesNumberInt += seriesOffset;

    ostringstream ossSN;
    ossSN << seriesNumberInt;
    string seriesNumber (ossSN.str());

    newSeriesFilePattern->assign("E");
    newSeriesFilePattern->append( studyId + "S" +  seriesNumber + "I");
    return seriesNumberInt;  
}


/*!
 *  Return incremented series offset 
 */
int svkImageWriterFactory::GetNewSeriesNumberOffset()
{
    return 22 ;
    //return svkImageWriterFactory::seriesNumberOffset+=77 ;
}
