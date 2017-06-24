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


#include <svkImageReaderFactory.h>
#include <string>


using namespace svk;


//vtkCxxRevisionMacro(svkImageReaderFactory, "$Rev$");
vtkStandardNewMacro(svkImageReaderFactory);


/*!
 *
 */
svkImageReaderFactory::svkImageReaderFactory() 
{

#if VTK_DEBUG_ON
    this->DebugOn();
#endif

    this->dcmMriVolReader           = svkDcmMriVolumeReader::New();
    this->dcmPETVolReader           = svkDcmPETVolumeReader::New();
    this->dcmMrsVolReader           = svkDcmMrsVolumeReader::New();
    this->dcmEnhancedVolReader      = svkDcmEnhancedVolumeReader::New();
    this->idfVolReader              = svkIdfVolumeReader::New();
    this->ddfVolReader              = svkDdfVolumeReader::New();
    this->fdfVolReader              = svkFdfVolumeReader::New();
    this->fidVolReader              = svkVarianFidReader::New();
    this->niiVolReader              = svkNIFTIVolumeReader::New();
    this->sdbmVolReader             = svkSdbmVolumeReader::New();
    this->rdaVolReader              = svkSiemensRdaReader::New();
    this->gePFileReader             = svkGEPFileReader::New();
    this->geSigna5XReader           = svkGESigna5XReader::New();
    this->geSignaLX2Reader          = svkGESignaLX2Reader::New();
    this->gePostageStampReader      = svkGEPostageStampReader::New();
    this->brukerDCMMRSReader        = svkBrukerDCMMRSReader::New();
    this->brukerRawMRSReader        = svkBrukerRawMRSReader::New();
    this->philipsSReader            = svkPhilipsSReader::New();
    this->dcmRawDataReader          = svkDcmRawDataReader::New();
    this->dcmSegmentationVolReader  = svkDcmSegmentationVolumeReader::New();
    this->lcmodelCSVReader          = svkLCModelCSVReader::New();
    this->lcmodelTableReader        = svkLCModelTableReader::New();

    vtkImageReader2Factory::RegisterReader( this->dcmMriVolReader );
    vtkImageReader2Factory::RegisterReader( this->dcmPETVolReader );
    vtkImageReader2Factory::RegisterReader( this->dcmMrsVolReader );
    vtkImageReader2Factory::RegisterReader( this->dcmEnhancedVolReader );
    vtkImageReader2Factory::RegisterReader( this->niiVolReader );
    vtkImageReader2Factory::RegisterReader( this->idfVolReader );
    vtkImageReader2Factory::RegisterReader( this->ddfVolReader );
    vtkImageReader2Factory::RegisterReader( this->fdfVolReader );
    vtkImageReader2Factory::RegisterReader( this->fidVolReader );
    vtkImageReader2Factory::RegisterReader( this->sdbmVolReader );
    vtkImageReader2Factory::RegisterReader( this->rdaVolReader );
    vtkImageReader2Factory::RegisterReader( this->gePFileReader );
    vtkImageReader2Factory::RegisterReader( this->geSigna5XReader );
    vtkImageReader2Factory::RegisterReader( this->geSignaLX2Reader );
    vtkImageReader2Factory::RegisterReader( this->gePostageStampReader );
    vtkImageReader2Factory::RegisterReader( this->brukerDCMMRSReader );
    vtkImageReader2Factory::RegisterReader( this->brukerRawMRSReader );
    vtkImageReader2Factory::RegisterReader( this->philipsSReader );
    vtkImageReader2Factory::RegisterReader( this->dcmRawDataReader);
    vtkImageReader2Factory::RegisterReader( this->dcmSegmentationVolReader );
    vtkImageReader2Factory::RegisterReader( this->lcmodelCSVReader );
    vtkImageReader2Factory::RegisterReader( this->lcmodelTableReader );

    //  this can be used if only need to check file type for example.
    this->quickParse = false; 
}


/*!
 *
 */
svkImageReaderFactory::~svkImageReaderFactory()
{
    vtkDebugMacro(<<"svkImageReaderFactory::~svkImageReaderFactory()");
    

    if (this->idfVolReader != NULL) {
        this->idfVolReader->Delete();
        this->idfVolReader = NULL;
    }

    if (this->ddfVolReader != NULL) {
        this->ddfVolReader->Delete();
        this->ddfVolReader = NULL;
    }

    if (this->dcmMriVolReader != NULL) {
        this->dcmMriVolReader->Delete();
        this->dcmMriVolReader = NULL;
    }

    if (this->dcmPETVolReader != NULL) {
        this->dcmPETVolReader->Delete();
        this->dcmPETVolReader = NULL;
    }

    if (this->dcmMrsVolReader != NULL) {
        this->dcmMrsVolReader->Delete();
        this->dcmMrsVolReader = NULL;
    }

    if (this->dcmEnhancedVolReader != NULL) {
        this->dcmEnhancedVolReader->Delete();
        this->dcmEnhancedVolReader = NULL;
    }

    if (this->niiVolReader != NULL) {
        this->niiVolReader->Delete();
        this->niiVolReader = NULL;
    }

    if (this->fdfVolReader != NULL) {
        this->fdfVolReader->Delete();
        this->fdfVolReader = NULL;
    }

    if (this->fidVolReader != NULL) {
        this->fidVolReader->Delete();
        this->fidVolReader = NULL;
    }

    if (this->sdbmVolReader != NULL) {
        this->sdbmVolReader->Delete();
        this->sdbmVolReader = NULL;
    }

    if (this->rdaVolReader != NULL) {
        this->rdaVolReader->Delete();
        this->rdaVolReader = NULL;
    }

    if (this->gePFileReader != NULL) {
        this->gePFileReader->Delete();
        this->gePFileReader = NULL;
    }

    if (this->geSigna5XReader != NULL) {
        this->geSigna5XReader->Delete();
        this->geSigna5XReader = NULL;
    }

    if (this->geSignaLX2Reader != NULL) {
        this->geSignaLX2Reader->Delete();
        this->geSignaLX2Reader = NULL;
    }

    if (this->gePostageStampReader != NULL) {
        this->gePostageStampReader->Delete();
        this->gePostageStampReader = NULL;
    }

    if (this->brukerDCMMRSReader != NULL) {
        this->brukerDCMMRSReader->Delete();
        this->brukerDCMMRSReader = NULL;
    }

    if (this->brukerRawMRSReader != NULL) {
        this->brukerRawMRSReader->Delete();
        this->brukerRawMRSReader = NULL;
    }

    if (this->philipsSReader != NULL) {
        this->philipsSReader->Delete();
        this->philipsSReader = NULL;
    }

    if (this->dcmRawDataReader != NULL) {
        this->dcmRawDataReader->Delete();
        this->dcmRawDataReader = NULL;
    }


    if (this->dcmSegmentationVolReader != NULL) {
        this->dcmSegmentationVolReader->Delete();
        this->dcmSegmentationVolReader = NULL;
    }

    if (this->lcmodelCSVReader != NULL) {
        this->lcmodelCSVReader->Delete();
        this->lcmodelCSVReader = NULL;
    }

    if (this->lcmodelTableReader != NULL) {
        this->lcmodelTableReader->Delete();
        this->lcmodelTableReader = NULL;
    }
}


/*!
 *  Set this for quick parsing of header info.  For GEPFiles, 
 *  this permits a header dump regardless of whether a mapper
 *  is available. 
 */
void svkImageReaderFactory::QuickParse()
{
    this->quickParse = true; 
    this->gePFileReader->OnlyReadHeader(true); 
    //this->gePFileReader->OnlyReadOneInputFile(); 
}


/*!
 *
 */
svkImageReader2* svkImageReaderFactory::CreateImageReader2(const char* path)
{
    // The vtkGESignaReader conflicts with the newly added svkGESigna5XReader.
    // If the Superclass returns GESigna, then return svkGESigna5XReader instead.
    svkImageReader2* ret = static_cast<svkImageReader2*>( this->Superclass::CreateImageReader2( path ) );
    if ( ret != NULL ) {
        if ( ret->IsA("vtkGESignaReader") ) {
            ret->Delete();
            ret = static_cast<svkImageReader2*>( svkGESigna5XReader::New() );
        }
    }
    return ret;
}


/*!
 *
 */
svkImageReader2* svkImageReaderFactory::CreateImageReader2( svkImageReader2::ReaderType readerType)
{

    if ( readerType == svkImageReader2::DICOM_MRS) {
        return svkDcmMrsVolumeReader::New(); 
    } else if ( readerType == svkImageReader2::DICOM_MRI) {
        return svkDcmMriVolumeReader::New();
    } else if ( readerType == svkImageReader2::DICOM_PET) {
        return svkDcmPETVolumeReader::New();
    } else if ( readerType == svkImageReader2::DICOM_ENHANCED_MRI) {
        return svkDcmEnhancedVolumeReader::New(); 
    } else if ( readerType == svkImageReader2::DICOM_SEGMENTATION) {
        return svkDcmSegmentationVolumeReader::New(); 
    } else if ( readerType == svkImageReader2::DICOM_RAW) {
        return svkDcmRawDataReader::New(); 
    } else if ( readerType == svkImageReader2::SIEMENS_RDA) {
        return svkSiemensRdaReader::New(); 
    } else if ( readerType == svkImageReader2::VARIAN_FID) {
        return svkVarianFidReader::New(); 
    } else if ( readerType == svkImageReader2::VARIAN_FDF) {
        return svkFdfVolumeReader::New(); 
    } else if ( readerType == svkImageReader2::GE_PFILE) {
        return svkGEPFileReader::New(); 
    } else if ( readerType == svkImageReader2::GE_SDBM) {
        return svkSdbmVolumeReader::New(); 
    } else if ( readerType == svkImageReader2::GE_POSTAGE_STAMP) {
        return svkGEPostageStampReader::New(); 
    } else if ( readerType == svkImageReader2::GE_SIGNA_LX2) {
        return svkGESignaLX2Reader::New(); 
    } else if ( readerType == svkImageReader2::GE_SIGNA_5X) {
        return svkGESigna5XReader::New(); 
    } else if ( readerType == svkImageReader2::LC_MODEL_COORD) {
        //return svkLCModelCoordReader::New(); 
    } else if ( readerType == svkImageReader2::LC_MODEL_CSV) {
        return svkLCModelCSVReader::New(); 
    } else if ( readerType == svkImageReader2::LC_MODEL_TABLE ) {
        return svkLCModelTableReader::New(); 
    } else if ( readerType == svkImageReader2::BRUKER_MRS) {
        return svkBrukerDCMMRSReader::New(); 
    } else if ( readerType == svkImageReader2::BRUKER_RAW_MRS) {
        return svkBrukerRawMRSReader::New(); 
    } else if ( readerType == svkImageReader2::PHILIPS_S ) {
        return svkPhilipsSReader::New(); 
    } else if ( readerType == svkImageReader2::DDF) {
        return svkDdfVolumeReader::New(); 
    } else if ( readerType == svkImageReader2::IDF) {
        return svkIdfVolumeReader::New(); 
    } else if ( readerType == svkImageReader2::NIFTI) {
        return svkNIFTIVolumeReader::New();
    } else {
        return NULL;
    }

}
