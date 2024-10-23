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


#include <svkDcmtkAdapter.h>

#include <svkUtils.h>
#include <svkTypeUtils.h>
#include </mnt/nfs/rad/apps/netopt/versions/dcmtk/latest/include/dcmtk/dcmdata/dcrleerg.h>
#include </mnt/nfs/rad/apps/netopt/versions/dcmtk/latest/include/dcmtk/dcmdata/dcrledrg.h> 



using namespace svk;


bool svkDcmtkAdapter::privateElementsAdded = false;
//vtkCxxRevisionMacro(svkDcmtkAdapter, "$Rev$");
vtkStandardNewMacro(svkDcmtkAdapter);


/*!
 *  Constructor.
 */
svkDcmtkAdapter::svkDcmtkAdapter()
{

#if VTK_DEBUG_ON
    this->DebugOn();
#endif

    vtkDebugMacro(<<this->GetClassName()<<":svkDcmtkAdapter");
    this->dcmFile = new svkDcmtkIod();
    this->replaceOldElements = OFFalse; 

    if( !svkDcmtkAdapter::privateElementsAdded ) {
		this->SetPrivateDictionaryElements();

		this->SetGEPrivateDictionaryElements();
		svkDcmtkAdapter::privateElementsAdded = true;
    }

    // The only way to get a pointer to the global dictionary is by locking it.
	this->privateDic = &( dcmDataDict.wrlock() );

	// We don't want to hold the  lock so lets unlock it.
	dcmDataDict.wrunlock();

    this->pixelDataElement = NULL;


}


/*!
 *  Destructor.
 */
svkDcmtkAdapter::~svkDcmtkAdapter()
{

    vtkDebugMacro(<<this->GetClassName()<<"~svkDcmtkAdapter");

    if (this->dcmFile != NULL) {
        delete this->dcmFile;
        this->dcmFile = NULL;
    }

}


/*!
 *  These may not be reliable, but were determined by trial/error 
 *  elements to it. 
 */
void svkDcmtkAdapter::SetGEPrivateDictionaryElements()
{
    // ===================================================
    //  get existing dictionary and append private entries to it:
    // ===================================================
    this->privateDic = &( dcmDataDict.wrlock() );

    privateDic->addEntry( new DcmDictEntry(
            0x0019, 0x10b2, EVR_FL, 
            "GE_PS_SEL_CENTER_R", 
            1, 1, "private", OFFalse, "SVK_GEMS_PRIVATE_CREATOR" 
        )
    );


    privateDic->addEntry( new DcmDictEntry(
            0x0019, 0x10b3, EVR_FL, 
            "GE_PS_SEL_CENTER_A", 
            1, 1, "private", OFFalse, "SVK_GEMS_PRIVATE_CREATOR" 
        )
    );

    privateDic->addEntry( new DcmDictEntry(
            0x0019, 0x10b4, EVR_FL, 
            "GE_PS_SEL_CENTER_S", 
            1, 1, "private", OFFalse, "SVK_GEMS_PRIVATE_CREATOR" 
        )
    );

    privateDic->addEntry( new DcmDictEntry(
            0x0019, 0x10af, EVR_FL, 
            "GE_PS_SEL_BOX_SIZE_1", 
            1, 1, "private", OFFalse, "SVK_GEMS_PRIVATE_CREATOR" 
        )
    );

    privateDic->addEntry( new DcmDictEntry(
            0x0019, 0x10b0, EVR_FL, 
            "GE_PS_SEL_BOX_SIZE_2", 
            1, 1, "private", OFFalse, "SVK_GEMS_PRIVATE_CREATOR" 
        )
    );

    privateDic->addEntry( new DcmDictEntry(
            0x0019, 0x10b1, EVR_FL, 
            "GE_PS_SEL_BOX_SIZE_3", 
            1, 1, "private", OFFalse, "SVK_GEMS_PRIVATE_CREATOR" 
        )
    );

    privateDic->addEntry( new DcmDictEntry(
            0x0043, 0x1038, EVR_FL, 
            "GE_PS_SAT_BANDS", 
            1, 1, "private", OFFalse, "SVK_GEMS_PRIVATE_CREATOR" 
        )
    );

    privateDic->addEntry( new DcmDictEntry(
            0x0019, 0x109c, EVR_LO, 
            "GE_PS_SEQ_1", 
            1, 1, "private", OFFalse, "SVK_GEMS_PRIVATE_CREATOR" 
        )
    );

    privateDic->addEntry( new DcmDictEntry(
            0x0019, 0x109e, EVR_LO, 
            "GE_PS_SEQ_2", 
            1, 1, "private", OFFalse, "SVK_GEMS_PRIVATE_CREATOR" 
        )
    );


    dcmDataDict.wrunlock();

}


/*!
 *  Gets the global DICOM dictionary and appends SIVIC private  
 *  elements to it. 
 */
void svkDcmtkAdapter::SetPrivateDictionaryElements()
{

    // ===================================================
    //  get existing dictionary and append private entries to it:
    // ===================================================
    this->privateDic = &( dcmDataDict.wrlock() );

    privateDic->addEntry( new DcmDictEntry(
            0x7777, 0x0010, DcmVR("LO"), "SVK_PRIVATE_TAG", 1, 1, "private", OFFalse, "SVK_PRIVATE_CREATOR" 
        )
    );

    // ===================================================
    //  Private Additions to:  
    //  MR Spectroscopy Module
    //
    //  If the center frequence and therefore chemical shift 
    //  reference are offset from center. 
    // ===================================================
    privateDic->addEntry( new DcmDictEntry(
            0x7777, 0x1000, EVR_DS, 
            "SVK_FrequencyOffset", 
            1, 1, "private", OFFalse, "SVK_PRIVATE_CREATOR" 
        )
    );


    // ===================================================
    //  Private Additions to:  
    //  MR Spectroscopy Data Module  
    //  (DICOM PT 3: C.8.14.3)
    //  
    //  Current data Domain indicators for spatial 
    //  coordinates (SPACE, KSPACE, ETC.)
    // ===================================================
    privateDic->addEntry( new DcmDictEntry(
            0x7777, 0x1001, EVR_CS, 
            "SVK_ColumnsDomain", 
            1, 1, "private", OFFalse, "SVK_PRIVATE_CREATOR" 
        )
    );
    privateDic->addEntry( new DcmDictEntry(
            0x7777, 0x1002, EVR_CS, 
            "SVK_RowsDomain", 
            1, 1, "private", OFFalse, "SVK_PRIVATE_CREATOR" 
        )
    );
    privateDic->addEntry( new DcmDictEntry(
            0x7777, 0x1003, EVR_CS, 
            "SVK_SliceDomain", 
            1, 1, "private", OFFalse, "SVK_PRIVATE_CREATOR" 
        )
    );


    // ===================================================
    //  Private Additions to:  
    //  MR Spectroscopy FOV/Geometry Macro Attributes 
    //  (DICOM PT 3: C.8.14.3.2)
    //
    //  Attributes that describe the acquisition or any data
    //  reordering required for acquisition interpretation
    // ===================================================
    privateDic->addEntry( new DcmDictEntry(
            0x7777, 0x1004, EVR_DS, 
            "SVK_SpectroscopyAcquisitionTLC", 
            1, 1, "private", OFFalse, "SVK_PRIVATE_CREATOR" 
        )
    );
    privateDic->addEntry( new DcmDictEntry(
            0x7777, 0x1005, EVR_DS, 
            "SVK_SpectroscopyAcquisitionPixelSpacing", 
            1, 1, "private", OFFalse, "SVK_PRIVATE_CREATOR" 
        )
    );
    privateDic->addEntry( new DcmDictEntry(
            0x7777, 0x1006, EVR_DS, 
            "SVK_SpectroscopyAcquisitionSliceThickness", 
            1, 1, "private", OFFalse, "SVK_PRIVATE_CREATOR" 
        )
    );
    privateDic->addEntry( new DcmDictEntry(
            0x7777, 0x1007, EVR_DS,
            "SVK_SpectroscopyAcquisitionOrientation", 
            1, 1, "private", OFFalse, "SVK_PRIVATE_CREATOR" 
        )
    );
    privateDic->addEntry( new DcmDictEntry(
            0x7777, 0x1008, EVR_UL, 
            "SVK_SpectroscopyAcqReorderedDataColumns", 
            1, 1, "private", OFFalse, "SVK_PRIVATE_CREATOR" 
        )
    );
    privateDic->addEntry( new DcmDictEntry(
            0x7777, 0x1009, EVR_UL, 
            "SVK_SpectroscopyAcqReorderedPhaseRows", 
            1, 1, "private", OFFalse, "SVK_PRIVATE_CREATOR" 
        )
    );
    privateDic->addEntry( new DcmDictEntry(
            0x7777, 0x1010, EVR_UL, 
            "SVK_SpectroscopyAcqReorderedPhaseColumns", 
            1, 1, "private", OFFalse, "SVK_PRIVATE_CREATOR" 
        )
    );
    privateDic->addEntry( new DcmDictEntry(
            0x7777, 0x1011, EVR_UL, 
            "SVK_SpectroscopyAcqReorderedOutOfPlanePhaseSteps", 
            1, 1, "private", OFFalse, "SVK_PRIVATE_CREATOR" 
        )
    );
    privateDic->addEntry( new DcmDictEntry(
            0x7777, 0x1012, EVR_DS, 
            "SVK_SpectroscopyAcqReorderedTLC", 
            1, 1, "private", OFFalse, "SVK_PRIVATE_CREATOR" 
        )
    );
    privateDic->addEntry( new DcmDictEntry(
            0x7777, 0x1013, EVR_DS, 
            "SVK_SpectroscopyAcqReorderedPixelSpacing", 
            1, 1, "private", OFFalse, "SVK_PRIVATE_CREATOR" 
        )
    );
    privateDic->addEntry( new DcmDictEntry(
            0x7777, 0x1014, EVR_DS, 
            "SVK_SpectroscopyAcqReorderedSliceThickness", 
            1, 1, "private", OFFalse, "SVK_PRIVATE_CREATOR" 
        )
    );
    privateDic->addEntry( new DcmDictEntry(
            0x7777, 0x1015, EVR_DS, 
            "SVK_SpectroscopyAcqReorderedOrientation", 
            1, 1, "private", OFFalse, "SVK_PRIVATE_CREATOR" 
        )
    );


    // ===================================================
    //  Private Additions to:  
    //  MR Spectroscopy Pulse Sequence Module
    //  (DICOM PT 3: C.8.14.2)
    // ===================================================

    //  defined terms specify whether K=0 is sampled.  YES/NO
    //  Explanation:
    //  for GE and possibly other vendors k-space may be sampled 
    //  symmetrically or not.  E.g., for an even number of 
    //  phase encodes, if even sampling, then k=0 was not 
    //  sampled . 
    //  Permutations (assuming that SVK_KSpaceCentered is TRUE):  
    //  for zero index arrays:
    //  
    //      symmetry,   numpts
    //      ======================================
    //      EVEN,       even number of points -> origin not sampled
    //      e.g, 8 points, index 4 is k 0.5
    //  
    //      EVEN,       odd  number of points -> origin is sampled
    //      e.g, 9 points, index 4 is k 0.0
    //  
    //      ODD ,       even number of points -> origin is sampled
    //      e.g, 8 points, index 4 is k 0.0
    //
    //      ODD ,       odd  number of points -> origin not sampled
    //      e.g, 9 points, index 4 is k 0.5
    privateDic->addEntry( new DcmDictEntry(
            0x7777, 0x1016, EVR_CS, 
            "SVK_K0Sampled", 
            1, 1, "private", OFFalse, "SVK_PRIVATE_CREATOR" 
        )
    );

    //  DEFINED TERMS indicating whether the k-space origin is
    //  at the image center, or corner.  
    //      Values: TRUE, FALSE
    //privateDic->addEntry( new DcmDictEntry(
            //0x7777, 0x1017, EVR_CS, 
            //"SVK_KSpaceCentered", 
            //1, 1, "private", OFFalse, "SVK_PRIVATE_CREATOR" 
        //)
    //);

    //  Specifies (YES/NO) if acquired data was chopped
    privateDic->addEntry( new DcmDictEntry(
            0x7777, 0x1017, EVR_CS, 
            "SVK_AcquisitionChop", 
            1, 1, "private", OFFalse, "SVK_PRIVATE_CREATOR" 
        )
    );

    // ===================================================
    //  Private Additions to:  
    //  Raw Data Module
    //  (DICOM PT 3: C.19.1)
    // ===================================================

    privateDic->addEntry( new DcmDictEntry(
            0x7777, 0x1018, EVR_SQ, 
            "SVK_FILE_SET_SEQUENCE", 
            1, 1, "private", OFFalse, "SVK_PRIVATE_CREATOR" 
        )
    );

    privateDic->addEntry( new DcmDictEntry(
            0x7777, 0x1019, EVR_LO, 
            "SVK_FILE_TYPE", 
            1, 1, "private", OFFalse, "SVK_PRIVATE_CREATOR" 
        )
    );

    privateDic->addEntry( new DcmDictEntry(
            0x7777, 0x1020, EVR_LT, 
            "SVK_FILE_NAME", 
            1, 1, "private", OFFalse, "SVK_PRIVATE_CREATOR" 
        )
    );

    privateDic->addEntry( new DcmDictEntry(
            0x7777, 0x1021, EVR_LO, 
            "SVK_FILE_SHA1_DIGEST", 
            1, 1, "private", OFFalse, "SVK_PRIVATE_CREATOR" 
        )
    );


    privateDic->addEntry( new DcmDictEntry(
            0x7777, 0x1022, EVR_UL, 
            "SVK_FILE_NUM_BYTES", 
            1, 1, "private", OFFalse, "SVK_PRIVATE_CREATOR" 
        )
    );

    privateDic->addEntry( new DcmDictEntry(
            0x7777, 0x1022, EVR_LO, 
            "SVK_FILE_NUM_BYTES_LONG", 
            1, 1, "private", OFFalse, "SVK_PRIVATE_CREATOR" 
        )
    );

    //  Block containing a GE PFile. 
    privateDic->addEntry( new DcmDictEntry(
            0x7777, 0x1023, EVR_OF, 
            "SVK_FILE_CONTENTS", 
            1, 1, "private", OFFalse, "SVK_PRIVATE_CREATOR" 
        )
    );

    privateDic->addEntry( new DcmDictEntry(
            0x7777, 0x1028, EVR_SQ, 
            "SVK_FILE_CONTENT_SEQUENCE", 
            1, 1, "private", OFFalse, "SVK_PRIVATE_CREATOR" 
        )
    );

    privateDic->addEntry( new DcmDictEntry(
            0x7777, 0x1029, EVR_LO, 
            "SVK_ZLIB_INFLATED_SIZE", 
            1, 1, "private", OFFalse, "SVK_PRIVATE_CREATOR" 
        )
    );

    //  Set point position of echo center 
    privateDic->addEntry( new DcmDictEntry(
            0x7777, 0x1024, EVR_DS, 
            "SVK_ECHO_CENTER_PT", 
            1, 1, "private", OFFalse, "SVK_PRIVATE_CREATOR" 
        )
    );

    // ===================================================
    //  Private Additions For SIVIC Voxel Tagging
    // ===================================================
    privateDic->addEntry( new DcmDictEntry(
            0x7777, 0x1025, EVR_SQ,
            "SVK_VOXEL_TAGGING_SEQUENCE",
            1, 1, "private", OFFalse, "SVK_PRIVATE_CREATOR"
        )
    );

    privateDic->addEntry( new DcmDictEntry(
            0x7777, 0x1026, EVR_SH,
            "SVK_VOXEL_TAG_NAME",
            1, 1, "private", OFFalse, "SVK_PRIVATE_CREATOR"
        )
    );

    privateDic->addEntry( new DcmDictEntry(
            0x7777, 0x1027, EVR_US,
            "SVK_VOXEL_TAG_VALUE",
            1, 1, "private", OFFalse, "SVK_PRIVATE_CREATOR"
        )
    );

    //  Next element number should be 30 (not 28!). 

    dcmDataDict.wrunlock();
}


/*!
 *  Factory method implementation
 */
void svkDcmtkAdapter::CreateIOD(DcmIodType iodType)
{
    vtkDebugMacro(<<this->GetClassName() << "::CreateIOD " << iodType );
    vtkDebugMacro(<< "PROBABLY Deprecated, remove method " << iodType );
}


/*!
 *  Set the SOP CLass UID.
 */
void svkDcmtkAdapter::SetSOPClassUID(DcmIodType iodType)
{
    vtkDebugMacro(<<this->GetClassName() << "::SetSOPClassUID" << iodType );

    if ( iodType == MR_IMAGE ) {          
        this->dcmFile->getMetaInfo()->putAndInsertString(
            DCM_MediaStorageSOPClassUID, 
            UID_MRImageStorage
        );
        this->dcmFile->getDataset()->putAndInsertString(
            DCM_SOPClassUID, 
            UID_MRImageStorage
        );
    } else if ( iodType == ENHANCED_MR_IMAGE) {
        this->dcmFile->getMetaInfo()->putAndInsertString(
            DCM_MediaStorageSOPClassUID, 
            UID_EnhancedMRImageStorage
        );
        this->dcmFile->getDataset()->putAndInsertString(
            DCM_SOPClassUID, 
            UID_EnhancedMRImageStorage
        );
    } else if ( iodType == MR_SPECTROSCOPY ) {
        this->dcmFile->getMetaInfo()->putAndInsertString(
            DCM_MediaStorageSOPClassUID, 
            UID_MRSpectroscopyStorage
        );
        this->dcmFile->getDataset()->putAndInsertString(
            DCM_SOPClassUID, 
            UID_MRSpectroscopyStorage
        );
    } else if ( iodType == SECONDARY_CAPTURE ) {
        this->dcmFile->getMetaInfo()->putAndInsertString(
            DCM_MediaStorageSOPClassUID, 
            UID_SecondaryCaptureImageStorage
        );
        this->dcmFile->getDataset()->putAndInsertString(
            DCM_SOPClassUID, 
            UID_SecondaryCaptureImageStorage
        );
    } else if ( iodType == RAW_DATA ) {
        this->dcmFile->getMetaInfo()->putAndInsertString(
            DCM_MediaStorageSOPClassUID, 
            UID_RawDataStorage
        );
        this->dcmFile->getDataset()->putAndInsertString(
            DCM_SOPClassUID, 
            UID_RawDataStorage
        );
    } else { 
        cout << "WARNING: UNRECOGNIZED SOP CLASS" << endl;
    }
}


/*!
 *  Print the header information to standard out.
 */
void svkDcmtkAdapter::PrintDcmHeader()
{
	this->dcmFile->print(cout);
}


/*!
 *  Print the header information to standard the given stream.
 */
void svkDcmtkAdapter::PrintDcmHeader(ostream& os)
{
	this->dcmFile->print(os);
}


/*!
 * Sets the value of a given tag.
 *
 * \param name the name of the tag whose value you wish to set
 *
 * \param value the integer value you wish the tag to have 
 */
void svkDcmtkAdapter::InsertEmptyElement(const char* name)
{
    this->dcmFile->getDataset()->insertEmptyElement( GetDcmTag(name), this->replaceOldElements ); 
    this->Modified();
}


/*!
 *  Generate and insert a unique UID as the value of the specified tag.
 *
 *  \param name the name of the tag whose value you wish to set
 *  \param whether to set the UID in the dataset or meta information. 
 *
 */
void svkDcmtkAdapter::InsertUniqueUID(const char* name)
{
    char uid[100];
    string newUID = this->GenerateUniqueUID(); 

    this->dcmFile->setValue( 
        GetDcmTag(name), 
        newUID
    ); 
    this->Modified();
}

/*! 
 *  Static utility method to generate a new unique DICOM UID. 
 */
string svkDcmtkAdapter::GenerateUniqueUID()
{
    char uid[100];
    string newUID( dcmGenerateUniqueIdentifier(uid, SITE_INSTANCE_UID_ROOT) ); 
    return newUID; 
}



/*!
 * Sets the value of a given tag.
 *
 * \param name the name of the tag whose value you wish to set
 *
 * \param value the integer value you wish the tag to have 
 */
void svkDcmtkAdapter::SetValue(const char* name, int value)
{
    this->dcmFile->setValue( GetDcmTag(name), value); 
    this->Modified();
}


/*!
 * Sets the value of a given tag.
 *
 * \param name the name of the tag whose value you wish to set
 *
 * \param value the float value you wish the tag to have 
 */
void svkDcmtkAdapter::SetValue(const char* name, float value)
{
    this->dcmFile->setValue( GetDcmTag(name), value ); 
    this->Modified();
}


/*!
 * Sets the value of a given tag.
 *
 * \param name the name of the tag whose value you wish to set
 *
 * \param value the double value you wish the tag to have 
 */
void svkDcmtkAdapter::SetValue(const char* name, double value)
{
    this->dcmFile->setValue( GetDcmTag(name), value ); 
    this->Modified();
}


/*!
 * Sets the value of a given tag.
 *
 * \param name the name of the tag whose value you wish to set
 *
 * \param value the string value you wish the tag to have 
 * \param whether to set the value in the dataset or metaInfo
 */
void svkDcmtkAdapter::SetValue(const char* name, string value, bool setMetaInfo)
{
    this->dcmFile->setValue( GetDcmTag(name), value, setMetaInfo ); 
    this->Modified();
}


/*!
 * Sets the array of value for a given tag.
 *
 * \param name the name of the tag whose value you wish to set
 *
 * \param values the pointer to the array of values you wish the tag to have 
 *
 * \param numValues the number of elements in the array of values
 */
void svkDcmtkAdapter::SetValue(const char* name, unsigned char* values, int numValues)
{
    this->dcmFile->getDataset()->putAndInsertUint8Array(
        GetDcmTag(name), 
        values,
        numValues
    );
    this->Modified();
}


/*!
 * Sets the array of value for a given tag.
 *
 * \param name the name of the tag whose value you wish to set
 *
 * \param values the pointer to the array of values you wish the tag to have 
 *
 * \param numValues the number of elements in the array of values
 */
void svkDcmtkAdapter::SetValue(const char* name, unsigned short* values, int numValues)
{
    this->dcmFile->getDataset()->putAndInsertUint16Array(
        GetDcmTag(name), 
        values,
        numValues
    );
    this->Modified();
}


/*!
 * Sets the array of value for a given tag.
 *
 * \param name the name of the tag whose value you wish to set
 *
 * \param values the pointer to the array of values you wish the tag to have 
 *
 * \param numValues the number of elements in the array of values
 */
void svkDcmtkAdapter::SetValue(const char* name, short* values, int numValues)
{
    this->dcmFile->getDataset()->putAndInsertSint16Array(
        GetDcmTag(name), 
        values,
        numValues
    );
    this->Modified();
}


/*!
 * Sets the array of value for a given tag.
 *
 * \param name the name of the tag whose value you wish to set
 *
 * \param values the pointer to the array of values you wish the tag to have 
 *
 * \param numValues the number of elements in the array of values
 */
void svkDcmtkAdapter::SetValue(const char* name, float* values, int numValues)
{
    DcmElement* of;
    OFCondition status = this->dcmFile->getDataset()->findAndGetElement( GetDcmTagKey( name ), of); 
    if (status.bad()) {
        cerr << "Error: cannot get element(" << status.text() << ")" << endl;
    }
    status = of->putFloat32Array(values, numValues);
    if (status.bad()) {
        cerr << "Error: cannot get element(" << status.text() << ")" << endl;
    }
    this->Modified();
}


/*!
 * Sets the array of value for a given tag.
 *
 * \param name the name of the tag whose value you wish to set
 *
 * \param values the pointer to the array of values you wish the tag to have 
 *
 * \param numValues the number of elements in the array of values
 */
void svkDcmtkAdapter::ModifyValueRecursive(const char* name, string value)
{
    DcmStack stack; 
    OFCondition status = this->dcmFile->getDataset()->findAndGetElements( GetDcmTagKey( name ), stack); 
    if (status.bad()) {
        cerr << "Error: cannot get element(" << status.text() << ")" << endl;
    }

    while ( !stack.empty() ) {
        DcmElement* of = static_cast<DcmElement*>(stack.pop());  
        status = of->putString(value.c_str());
        if (status.bad()) {
            cerr << "Error: cannot get element(" << status.text() << ")" << endl;
        }
    }
    this->Modified();
}

/*!
 * Gets the value of a given tag as an integer.
 *
 * \param name the name of the tag whose value you wish to get
 *
 * \return the integer value of the tag
 */
int svkDcmtkAdapter::GetIntValue(const char* name)
{
    return this->dcmFile->getIntValue( GetDcmTagKey(name) ); 
}


/*!
 * Gets the value of a given tag as a float.
 *
 * \param name the name of the tag whose value you wish to get
 *
 * \return the float value of the tag
 */
float svkDcmtkAdapter::GetFloatValue(const char* name)
{
    return this->dcmFile->getFloatValue( GetDcmTagKey(name) ); 
}


/*!
 * Gets the array of values for a given tag.
 *
 * \param name the name of the tag whose value you wish to set
 *
 * \param values the pointer to the array of values you wish the tag to have
 *
 * \param numValues the number of elements in the array of values
 */
void svkDcmtkAdapter::GetFloatValue(const char* name, float* values, long unsigned int numValues)
{
    DcmElement* of;
    float* floatArray;
    OFCondition status = this->dcmFile->getDataset()->findAndGetElement( GetDcmTagKey( name ), of);
    if (status.bad()) {
        cerr << "Error: cannot get element(" << status.text() << ")" << endl;
    }
    of->getFloat32Array( floatArray );
    memcpy( values, floatArray, 4 * numValues );

    this->Modified();
} 


/*!
 * Gets the array of values for a given tag.
 *
 * \param name the name of the tag whose value you wish to set
 *
 * \param values the pointer to the array of values you wish the tag to have
 *
 * \param numValues the number of elements in the array of values
 */
void svkDcmtkAdapter::GetByteValue(const char* name, char* values, long unsigned int numValues)
{
    DcmElement* of;
    Uint8* byteArray;
    OFCondition status = this->dcmFile->getDataset()->findAndGetElement( GetDcmTagKey( name ), of);
    if (status.bad()) {
        cerr << "Error: cannot get element(" << status.text() << ")" << endl;
    }
    status = of->getUint8Array( byteArray );
    if (status.bad()) {
        cerr << "Error: cannot get Uint8Array for(" << status.text() << ")" << endl;
    }

    int sizeOfByte = 1; 
    memcpy( values, byteArray, sizeOfByte * numValues );

    this->Modified();
}


/*!
 * Gets the array of values for a given tag.
 *
 * \param name the name of the tag whose value you wish to set
 *
 * \param values the pointer to the array of values you wish the tag to have
 *
 * \param numValues the number of elements in the array of values
 */
void svkDcmtkAdapter::GetShortValue(const char* name, short* values, long unsigned int numValues)
{
    DcmElement* of;
    Uint16* shortArray;
    OFCondition status = this->dcmFile->getDataset()->findAndGetElement( GetDcmTagKey( name ), of);
    if (status.bad()) {
        cerr << "Error: cannot get element(" << status.text() << ")" << endl;
    }
    status = of->getUint16Array( shortArray );
    if (status.bad()) {
        cerr << "Error: cannot get Uint16Array for(" << status.text() << ")" << endl;
    }

    memcpy( values, shortArray, 2 * numValues );

    this->Modified();
}


/*!
 * Gets the value at a given position for a given tag.
 *
 * \param name the name of the tag whose value you wish to set
 *
 * \param position the position in the array of the value
 *
 */
unsigned short svkDcmtkAdapter::GetShortValue(const char* name, long unsigned int position)
{
    DcmElement* of;
    Uint16 shortValue;
    OFCondition status = this->dcmFile->getDataset()->findAndGetElement( GetDcmTagKey( name ), of);
    if (status.bad()) {
        cerr << "Error: cannot get element(" << status.text() << ")" << endl;
    }
    of->getUint16( shortValue, position );
    return static_cast<unsigned short> ( shortValue );

}


/*!
 *  Return one short word at the specified index from the PixelData field.  
 */
unsigned short svkDcmtkAdapter::GetPixelValue( long unsigned int position )
{

    OFCondition status; 

    DcmTagKey pixelDataTag; 
    pixelDataTag.setGroup(0x7fe0); 
    pixelDataTag.setElement(0x0010); 
    if ( this->pixelDataElement == NULL ) {
        status = this->dcmFile->getDataset()->findAndGetElement( pixelDataTag, this->pixelDataElement);
        if (status.bad()) {
            cerr << "Error: cannot get element(" << status.text() << ")" << endl;
        }
    }

    Uint16 shortValue;
    this->pixelDataElement->getUint16( shortValue, position );
    return static_cast<unsigned short> ( shortValue );

}


/*!
 * Gets the value of a given tag as a double.
 *
 * \param name the name of the tag whose value you wish to get
 *
 * \return the double value of the tag
 */
double svkDcmtkAdapter::GetDoubleValue(const char* name, bool searchInto)
{
    if ( searchInto == true ) {
        return this->dcmFile->getDoubleValue( GetDcmTagKey(name), searchInto ); 
    } else {    
        return this->dcmFile->getDoubleValue( GetDcmTagKey(name) ); 
    }
}


/*!
 *  Gets the value of a given tag as a string.
 *
 *  \param name the name of the tag whose value you wish to get
 *
 *  \return the string value of the tag
 */
string svkDcmtkAdapter::GetStringValue(const char* name)
{
    try {
        return this->dcmFile->getStringValue( GetDcmTagKey(name) ); 
    } catch (const svkTagNotFound& e) {
        this->HandleTagNotFoundException(e);
    } catch (const exception& e) {
        cerr << "ERROR: " << e.what() << endl;
    }
	return string("");

}


/*!
 *  Gets the value of a given tag, for a given position as a string.
 *
 *  \param name the name of the tag whose value you wish to get
 *  \param pos the position of the element in the string whose value you wish to get
 *
 *  \return the string value of the tag, for the given position
 */
string svkDcmtkAdapter::GetStringValue(const char* name, int pos)
{
    try {
        return this->dcmFile->getStringValue( GetDcmTagKey(name), pos ); 
    } catch (const svkTagNotFound& e) {
        this->HandleTagNotFoundException(e);
    } catch (const exception& e) {
        cerr << "ERROR: " << e.what() << endl;
    }
	return string("");
}


/*!
 *  Add the nested sequence within the specified item of the parent sequence.
 *
 *  \param parentSeqName the string name of the parent sequence
 *  \param parentSeqItemPosition the position of the item in that sequence 
 *  \param elementName the string name of the item in the sequence
 */
void svkDcmtkAdapter::AddSequenceItemElement(const char* parentSeqName, int parentSeqItemPosition, const char* elementName)
{
    DcmSequenceOfItems* seq; 
    this->dcmFile->getDataset()->findAndGetSequence(  GetDcmTagKey( parentSeqName ), seq, true);
    DcmItem* item = NULL;
    if ( (item = seq->getItem(parentSeqItemPosition)) == NULL) {
        item = new DcmItem();  
        seq->insert(item, parentSeqItemPosition);
    }

    item->insertEmptyElement( GetDcmTag(elementName), this->replaceOldElements ); 
    this->Modified();
    return ;
}


/*!
 *  Add an item to the specified item in the specified sequence(seqName), where the nestedSequence is 
 *  nested within a specific parent sequence(parentSeqName) and item(parentSeqItemPosition).
 *
 *  \param seqName the string name of the parent sequence
 *  \param seqItemPosition the position of the item in that sequence 
 *  \param elementName the name of the element being added to the specified item
 *  \param value the value of the element being added to the specified item ("EMPTY_ELEMENT") inserts an empty element. 
 *  \param parentSeqName the string name of the parent sequence
 *  \param parentSeqItemPosition the position of the item in that sequence 
 */
void svkDcmtkAdapter::AddSequenceItemElement(const char* seqName, int seqItemPosition, const char* elementName, string value, const char* parentSeqName, int parentSeqItemPosition)
{
    DcmItem* dataset = this->dcmFile->getDataset(); 

    if (parentSeqName != NULL) {
        DcmSequenceOfItems* seq = GetDcmSequence(parentSeqName);
        if (seq != NULL ) {
            dataset = seq->getItem(parentSeqItemPosition);
        }
    }

    if (value == "EMPTY_ELEMENT") {
        this->GetDcmItem(dataset, seqName, seqItemPosition)->insertEmptyElement( GetDcmTag(elementName), this->replaceOldElements );    
    } else {
        svkDcmtkUtils::setValue(
            this->GetDcmItem(dataset, seqName, seqItemPosition),    
            GetDcmTag(elementName), 
            value
        );
    }
    this->Modified();
}


/*!
 *  Add an item to the specified item in the specified sequence(seqName), where the sequence is 
 *  nested within a specific parent sequence(parentSeqName) and item(parentSeqItemPosition).
 *
 *  \param seqName the string name of the parent sequence
 *  \param seqItemPosition the position of the item in that sequence 
 *  \param elementName the name of the element being added to the specified item
 *  \param value the value of the element being added to the specified item
 *  \param parentSeqName the string name of the parent sequence
 *  \param parentSeqItemPosition the position of the item in that sequence 
 */
void svkDcmtkAdapter::AddSequenceItemElement(const char* seqName, int seqItemPosition, const char* elementName, int value, const char* parentSeqName, int parentSeqItemPosition)
{
    DcmItem* dataset = this->dcmFile->getDataset(); 

    if (parentSeqName != NULL) {
        DcmSequenceOfItems* seq = GetDcmSequence(parentSeqName);
        if (seq != NULL ) {
            dataset = seq->getItem(parentSeqItemPosition);
        }
    }

    svkDcmtkUtils::setValue(
        this->GetDcmItem(dataset, seqName, seqItemPosition),    
        GetDcmTag(elementName), 
        value 
    );

    this->Modified();
}


/*!
 *  Add an item to the specified item in the specified sequence(seqName), where the sequence is 
 *  nested within a specific parent sequence(parentSeqName) and item(parentSeqItemPosition).
 *
 *  \param seqName the string name of the parent sequence
 *  \param seqItemPosition the position of the item in that sequence 
 *  \param elementName the name of the element being added to the specified item
 *  \param value the value of the element being added to the specified item
 *  \param parentSeqName the string name of the parent sequence
 *  \param parentSeqItemPosition the position of the item in that sequence 
 */
void svkDcmtkAdapter::AddSequenceItemElement(const char* seqName, int seqItemPosition, const char* elementName, long int value, const char* parentSeqName, int parentSeqItemPosition)
{
    DcmItem* dataset = this->dcmFile->getDataset(); 

    if (parentSeqName != NULL) {
        DcmSequenceOfItems* seq = GetDcmSequence(parentSeqName);
        if (seq != NULL ) {
            dataset = seq->getItem(parentSeqItemPosition);
        }
    }

    svkDcmtkUtils::setValue(
        this->GetDcmItem(dataset, seqName, seqItemPosition),    
        GetDcmTag(elementName), 
        value 
    );

    this->Modified();
}


/*!
 *  Add an item to the specified item in the specified sequence(seqName), where the sequence is 
 *  nested within a specific parent sequence(parentSeqName) and item(parentSeqItemPosition).
 *
 *  \param seqName the string name of the parent sequence
 *  \param seqItemPosition the position of the item in that sequence 
 *  \param elementName the name of the element being added to the specified item
 *  \param values array of value of the element being added to the specified item
 *  \param numValues  number of values in values array. 
 *  \param parentSeqName the string name of the parent sequence
 *  \param parentSeqItemPosition the position of the item in that sequence 
 */
void svkDcmtkAdapter::AddSequenceItemElement(const char* seqName, int seqItemPosition, const char* elementName, char* values, int numValues, const char* parentSeqName, int parentSeqItemPosition)
{
    OFCondition status; 
    DcmItem* dataset = this->dcmFile->getDataset(); 

    if (parentSeqName != NULL) {
        DcmSequenceOfItems* seq = GetDcmSequence(parentSeqName);
        if (seq != NULL ) {
            dataset = seq->getItem(parentSeqItemPosition);
        }
    }

    DcmItem* newItem = this->GetDcmItem(dataset, seqName, seqItemPosition); 

    //  Insert a dummy val. This was the only way I was able to get this to work
    //  for inserting multiple vals.  I'm probably missing something, but it works. 
    status = newItem->insertEmptyElement( GetDcmTag( elementName)) ;
    if (status.bad()) {
        cerr << "Error: cannot insert dummy val(" << status.text() << ")" << endl;
    }

    //  Now get the element and add the array to it:    
    DcmElement* element;
    status = newItem->findAndGetElement( GetDcmTagKey( elementName), element);
    if (status.bad()) {
        cerr << "Error: cannot get element(" << status.text() << ")" << endl;
    }

    Uint8* inputArray = new Uint8[numValues]; 
    for (int i = 0; i < numValues; i++) {
        inputArray[i] = (Uint8)values[i]; 
    }
    status = element->putUint8Array(inputArray, numValues);
    if (status.bad()) {
        cerr << "Error: could not insert element(" << status.text() << ")" << endl;
    }
    delete[] inputArray;
    this->Modified();
}


/*!
 *  Add an item to the specified item in the specified sequence(seqName), where the sequence is 
 *  nested within a specific parent sequence(parentSeqName) and item(parentSeqItemPosition).
 *
 *  \param seqName the string name of the parent sequence
 *  \param seqItemPosition the position of the item in that sequence 
 *  \param elementName the name of the element being added to the specified item
 *  \param values array of value of the element being added to the specified item
 *  \param numValues  number of values in values array. 
 *  \param parentSeqName the string name of the parent sequence
 *  \param parentSeqItemPosition the position of the item in that sequence 
 */
void svkDcmtkAdapter::AddSequenceItemElement(const char* seqName, int seqItemPosition, const char* elementName, unsigned short* values, int numValues, const char* parentSeqName, int parentSeqItemPosition)
{
    OFCondition status; 
    DcmItem* dataset = this->dcmFile->getDataset(); 

    if (parentSeqName != NULL) {
        DcmSequenceOfItems* seq = GetDcmSequence(parentSeqName);
        if (seq != NULL ) {
            dataset = seq->getItem(parentSeqItemPosition);
        }
    }

    DcmItem* newItem = this->GetDcmItem(dataset, seqName, seqItemPosition); 

    //  Insert a dummy val. This was the only way I was able to get this to work
    //  for inserting multiple vals.  I'm probably missing something, but it works. 
    status = newItem->insertEmptyElement( GetDcmTag( elementName)) ;
    if (status.bad()) {
        cerr << "Error: cannot insert dummy val(" << status.text() << ")" << endl;
    }

    //  Now get the element and add the array to it:    
    DcmElement* element;
    status = newItem->findAndGetElement( GetDcmTagKey( elementName), element);
    if (status.bad()) {
        cerr << "Error: cannot get element(" << status.text() << ")" << endl;
    }

    Uint16* inputArray = new Uint16[numValues]; 
    for (int i = 0; i < numValues; i++) {
        inputArray[i] = (Uint16)values[i]; 
    }

    //  If this is a DcmAttributeTag VR, then the short array represents the two fields of the element, but this is treated 
    //  as a single 2 element array, so divide
    //  numValues by 2 (see DcmAttributeTag: DcmAttributeTag::putUint16Array).  
    //  There is already a 2* in the put even for numUints = 1. Very confusing
    //  errorFlag = putValue(uintVals, 2 * sizeof(Uint16) * Uint32(numUints));
    if ( element->ident() == EVR_AT ) {
        //cout << "EVR_AT, divide numValues by 2"<< endl;
        numValues = numValues/2; 
    }

    status = element->putUint16Array(inputArray, numValues);
    if (status.bad()) {
        cerr << "Error: could not insert element(" << status.text() << ")" << endl;
    }
    delete[] inputArray;
    this->Modified();
}



/*!
 *  Add an item to the specified item in the specified sequence(seqName), where the sequence is 
 *  nested within a specific parent sequence(parentSeqName) and item(parentSeqItemPosition).
 *
 *  \param seqName the string name of the parent sequence
 *  \param seqItemPosition the position of the item in that sequence 
 *  \param elementName the name of the element being added to the specified item
 *  \param values array of value of the element being added to the specified item
 *  \param numValues  number of values in values array. 
 *  \param parentSeqName the string name of the parent sequence
 *  \param parentSeqItemPosition the position of the item in that sequence 
 */
void svkDcmtkAdapter::AddSequenceItemElement(const char* seqName, int seqItemPosition, const char* elementName, unsigned int* values, int numValues, const char* parentSeqName, int parentSeqItemPosition)
{
    OFCondition status; 
    DcmItem* dataset = this->dcmFile->getDataset(); 

    if (parentSeqName != NULL) {
        DcmSequenceOfItems* seq = GetDcmSequence(parentSeqName);
        if (seq != NULL ) {
            dataset = seq->getItem(parentSeqItemPosition);
        }
    }

    DcmItem* newItem = this->GetDcmItem(dataset, seqName, seqItemPosition); 

    //  Insert a dummy val. This was the only way I was able to get this to work
    //  for inserting multiple vals.  I'm probably missing something, but it works. 
    //this->AddSequenceItemElement(seqName, seqItemPosition, elementName, 0, parentSeqName, seqItemPosition);

    Uint32 dummyVal = 99; 
    status = newItem->putAndInsertUint32( GetDcmTag( elementName), dummyVal) ;
    //status = newItem->insertEmptyElement( GetDcmTag( elementName)) ;
    if (status.bad()) {
        cerr << "Error: cannot insert dummy val(" << status.text() << ")" << endl;
    }
    //this->PrintDcmHeader();
    
    //  Now get the element and add the array to it:    
    DcmElement* element;
    status = newItem->findAndGetElement( GetDcmTagKey( elementName), element);
    if (status.bad()) {
        cerr << "Error: cannot get element(" << status.text() << ")" << endl;
    }

    Uint32* inputArray = new Uint32[numValues]; 
    for (int i = 0; i < numValues; i++) {
        inputArray[i] = (Uint32)values[i]; 
    }
    status = element->putUint32Array(inputArray, numValues);
    if (status.bad()) {
        cerr << "Error: could not insert element(" << status.text() << ")" << endl;
    }
    delete[] inputArray;
    this->Modified();
}


/*!
 *  Add an item to the specified item in the specified sequence(seqName), where the sequence is 
 *  nested within a specific parent sequence(parentSeqName) and item(parentSeqItemPosition).
 *
 *  \param seqName the string name of the parent sequence
 *  \param seqItemPosition the position of the item in that sequence 
 *  \param elementName the name of the element being added to the specified item
 *  \param values array of value of the element being added to the specified item
 *  \param numValues  number of values in values array. 
 *  \param parentSeqName the string name of the parent sequence
 *  \param parentSeqItemPosition the position of the item in that sequence 
 */
void svkDcmtkAdapter::AddSequenceItemElement(const char* seqName, int seqItemPosition, const char* elementName, float* values, unsigned long int numValues, const char* parentSeqName, int parentSeqItemPosition)
{
    OFCondition status; 
    DcmItem* dataset = this->dcmFile->getDataset(); 

    if (parentSeqName != NULL) {
        DcmSequenceOfItems* seq = GetDcmSequence(parentSeqName);
        if (seq != NULL ) {
            dataset = seq->getItem(parentSeqItemPosition);
        }
    }

    DcmItem* newItem = this->GetDcmItem(dataset, seqName, seqItemPosition); 

    //  Insert a dummy val. This was the only way I was able to get this to work
    //  for inserting multiple vals.  I'm probably missing something, but it works. 
    Float32 dummyVal = 99.;  
    newItem->putAndInsertFloat32( GetDcmTag( elementName), dummyVal) ;

    //  Now get the element and add the array to it:    
    DcmElement* element;
    status = newItem->findAndGetElement( GetDcmTagKey( elementName), element);
    if (status.bad()) {
        cerr << "Error: cannot get element(" << status.text() << ")" << endl;
    }

    status = element->putFloat32Array(values, numValues);
    if (status.bad()) {
        cerr << "Error: could not insert element(" << status.text() << ")" << endl;
    }
    this->Modified();
}


/*!
 *  Copies a top-level sequence from this header to the target header.
 */
void svkDcmtkAdapter::CopySequence( svkDcmHeader* target, const char* seqName )
{
    DcmSequenceOfItems* seq;
    OFCondition status = this->dcmFile->getDataset()->findAndGetSequence(  GetDcmTagKey( seqName ), seq, true);
    if ( seq == NULL || status != EC_Normal ) {
    	if( this->GetDebug() ) {
			cout << "WARNING: Sequence Not Found--" << seqName << endl;
    	}
    } else {
		if (dynamic_cast<svkDcmtkAdapter*>(target)->dcmFile != NULL) {
			DcmElement* newElement;

            DcmElement *elem = NULL;
            // find the element 
            OFCondition status = this->dcmFile->getDataset()->findAndGetElement( GetDcmTagKey( seqName ), elem );
            if (status.good()) {
                // create copy of element 
                newElement = OFstatic_cast(DcmElement *, elem->clone());
                dynamic_cast<svkDcmtkAdapter*>(target)->dcmFile->getDataset()->insert(newElement, OFTrue, OFTrue);
            } 

		}

    }
}


/*!
 *  Removes all items from the specified sequence
 *
 *  \param seqName the string name of the parent sequence to clear
 */
void svkDcmtkAdapter::ClearSequence(const char* seqName) 
{
    DcmSequenceOfItems* seq; 
    OFCondition status = this->dcmFile->getDataset()->findAndGetSequence(  GetDcmTagKey( seqName ), seq, true);
    if ( seq == NULL || status != EC_Normal ) {
    	if( this->GetDebug() ) {
			cout << "Sequence Not Found" << seqName << endl;
    	}
    } else {
        seq->clear();
    }

    return; 
}


/*!
 *  Clears the specified element 
 *
 *  \param seqName the string name of the parent sequence to clear
 */
void svkDcmtkAdapter::ClearElement(const char* elementName) 
{
    DcmElement* element;  
    this->dcmFile->getDataset()->findAndGetElement(  GetDcmTagKey( elementName ), element, true);
    element->clear();
}


/*!
 *  Removes the specified element 
 *
 *  \param seqName the string name of the element to remove 
 */
void svkDcmtkAdapter::RemoveElement(const char* elementName) 
{
    this->dcmFile->getDataset()->remove(  GetDcmTagKey( elementName ));
}


/*!
 *  Add an item to the specified item in the specified sequence(seqName), where the nestedSequence is 
 *  nested within a specific parent sequence(parentSeqName) and item(parentSeqItemPosition).
 *
 *  \param seqName the string name of the parent sequence
 *  \param seqItemPosition the position of the item in that sequence 
 *  \param elementName the name of the element being added to the specified item
 *  \param value the value of the element being added to the specified item
 *  \param parentSeqName the string name of the parent sequence
 *  \param parentSeqItemPosition the position of the item in that sequence 
 */
void svkDcmtkAdapter::AddSequenceItemElement(const char* seqName, int seqItemPosition, const char* elementName, float value, const char* parentSeqName, int parentSeqItemPosition)
{
    DcmItem* dataset = this->dcmFile->getDataset(); 

    if (parentSeqName != NULL) {
        DcmSequenceOfItems* seq = GetDcmSequence(parentSeqName);
        if (seq != NULL ) {
            dataset = seq->getItem(parentSeqItemPosition);
        }
    }

    svkDcmtkUtils::setValue(
        this->GetDcmItem(dataset, seqName, seqItemPosition),    
        GetDcmTag(elementName), 
        value
    );
    this->Modified();
}


/*!
 *  Add an item to the specified item in the specified sequence(seqName), where the nestedSequence is 
 *  nested within a specific parent sequence(parentSeqName) and item(parentSeqItemPosition).
 *
 *  \param seqName the string name of the parent sequence
 *  \param seqItemPosition the position of the item in that sequence 
 *  \param elementName the name of the element being added to the specified item
 *  \param value the value of the element being added to the specified item
 *  \param parentSeqName the string name of the parent sequence
 *  \param parentSeqItemPosition the position of the item in that sequence 
 */
void svkDcmtkAdapter::AddSequenceItemElement(const char* seqName, int seqItemPosition, const char* elementName, double value, const char* parentSeqName, int parentSeqItemPosition)
{
    DcmItem* dataset = this->dcmFile->getDataset();

    if (parentSeqName != NULL) {
        DcmSequenceOfItems* seq = GetDcmSequence(parentSeqName);
        if (seq != NULL ) {
            dataset = seq->getItem(parentSeqItemPosition);
        }
    }

    svkDcmtkUtils::setValue(
        this->GetDcmItem(dataset, seqName, seqItemPosition),
        GetDcmTag(elementName),
        value
    );
    this->Modified();
}


/*!
 *  Get an item from the specified item in the specified sequence.
 *
 *  \param seqName the string name of the sequence
 *  \param itemPosition the position of the item in that sequence 
 *  \param elementName the name of the element being added to the specified item
 *
 *  \return integer value of the item element 
 */
int svkDcmtkAdapter::GetIntSequenceItemElement(const char* seqName, int seqItemPosition, const char* elementName,  const char* parentSeqName, int parentSeqItemPosition, int pos)
{

    DcmItem* dataset = this->dcmFile->getDataset(); 

    if (parentSeqName != NULL) {
        DcmSequenceOfItems* seq = GetDcmSequence(parentSeqName);
        if (seq != NULL ) {
            dataset = seq->getItem(parentSeqItemPosition);
        }
    }

    return svkDcmtkUtils::getIntValue(
        this->GetDcmItem(dataset, seqName, seqItemPosition),
        GetDcmTagKey(elementName), 
        pos
    );
}


/*!
 *  Get an item from the specified item in the specified sequence.
 *
 *  \param seqName the string name of the sequence
 *  \param itemPosition the position of the item in that sequence 
 *  \param elementName the name of the element being added to the specified item
 *
 *  \return integer value of the item element 
 */
long int svkDcmtkAdapter::GetLongIntSequenceItemElement(const char* seqName, int seqItemPosition, const char* elementName,  const char* parentSeqName, int parentSeqItemPosition, int pos)
{

    DcmItem* dataset = this->dcmFile->getDataset(); 

    if (parentSeqName != NULL) {
        DcmSequenceOfItems* seq = GetDcmSequence(parentSeqName);
        if (seq != NULL ) {
            dataset = seq->getItem(parentSeqItemPosition);
        }
    }

    string longString = svkDcmtkUtils::getStringValue(
        this->GetDcmItem(dataset, seqName, seqItemPosition),
        GetDcmTagKey(elementName), 
        pos
    );
    return svkTypeUtils::StringToLInt(longString); 
    //return svkDcmtkUtils::getLongIntValue(
     //   this->GetDcmItem(dataset, seqName, seqItemPosition),
      //  GetDcmTagKey(elementName), 
       // pos
    //);
}

/*!
 *  Get an item from the specified item in the specified sequence.
 *
 *  \param seqName the string name of the sequence
 *  \param itemPosition the position of the item in that sequence 
 *  \param elementName the name of the element being added to the specified item
 *
 *  \return float value of the item element 
 */
float svkDcmtkAdapter::GetFloatSequenceItemElement(const char* seqName, int seqItemPosition, const char* elementName, const char* parentSeqName, int parentSeqItemPosition, int pos)
{

    DcmItem* dataset = this->dcmFile->getDataset(); 

    if (parentSeqName != NULL) {
        DcmSequenceOfItems* seq = GetDcmSequence(parentSeqName);
        if (seq != NULL ) {
            dataset = seq->getItem(parentSeqItemPosition);
        }
    }

    return svkDcmtkUtils::getFloatValue(
        this->GetDcmItem(dataset, seqName, seqItemPosition),
        GetDcmTagKey(elementName), 
        pos
    );
}


/*!
 *  Get an item from the specified item in the specified sequence.
 *
 *  \param seqName the string name of the sequence
 *  \param itemPosition the position of the item in that sequence 
 *  \param elementName the name of the element being added to the specified item
 *
 *  \return array of float values for the item element 
 */
void svkDcmtkAdapter::GetFloatSequenceItemElement(const char* seqName, int seqItemPosition, const char* elementName, float* values, int numValues, const char* parentSeqName, int parentSeqItemPosition) 
{

    DcmItem* dataset = this->dcmFile->getDataset(); 

    if (parentSeqName != NULL) {
        DcmSequenceOfItems* seq = GetDcmSequence(parentSeqName);
        if (seq != NULL ) {
            dataset = seq->getItem(parentSeqItemPosition);
        }
    }

    dataset = this->GetDcmItem(dataset, seqName, seqItemPosition);

    DcmElement* of;
    float* floatArray;
    OFCondition status = dataset->findAndGetElement( GetDcmTagKey( elementName ), of);
    if (status.bad()) {
        cerr << "Error: cannot get element(" << status.text() << ")" << endl;
    }

    of->getFloat32Array( floatArray );
    memcpy( values, floatArray, 4 * numValues );

    this->Modified();
}


/*!
 *  Get an item from the specified item in the specified sequence.
 *
 *  \param seqName the string name of the sequence
 *  \param itemPosition the position of the item in that sequence 
 *  \param elementName the name of the element being added to the specified item
 *
 *  \return double value of the item element 
 */
double svkDcmtkAdapter::GetDoubleSequenceItemElement(const char* seqName, int seqItemPosition, const char* elementName, const char* parentSeqName, int parentSeqItemPosition)
{
    DcmItem* dataset = this->dcmFile->getDataset(); 

    if (parentSeqName != NULL) {
        DcmSequenceOfItems* seq = GetDcmSequence(parentSeqName);
        if (seq != NULL ) {
            dataset = seq->getItem(parentSeqItemPosition);
        }
    }

    return svkDcmtkUtils::getDoubleValue(
        this->GetDcmItem(dataset, seqName, seqItemPosition),
        GetDcmTagKey(elementName) 
    );
}


/*!
 *  Get an item from the specified item in the specified sequence.
 *
 *  \param seqName the string name of the sequence
 *  \param itemPosition the position of the item in that sequence 
 *  \param elementName the name of the element being added to the specified item
 *
 *  \return string value of the item element 
 */
string svkDcmtkAdapter::GetStringSequenceItemElement(const char* seqName, int seqItemPosition, const char* elementName, const char* parentSeqName, int parentSeqItemPosition)
{
    DcmItem* dataset = this->dcmFile->getDataset(); 

    if (parentSeqName != NULL) {
        DcmSequenceOfItems* seq = GetDcmSequence(parentSeqName);
        if (seq != NULL ) {
            dataset = seq->getItem(parentSeqItemPosition);
        }
    }

    return svkDcmtkUtils::getStringValue(
        this->GetDcmItem(dataset, seqName, seqItemPosition),
        GetDcmTagKey(elementName) 
    );
}


/*!
 *  Get an item from the specified item in the specified sequence.
 *
 *  \param seqName the string name of the sequence
 *  \param itemPosition the position of the item in that sequence 
 *  \param elementName the name of the element being added to the specified item
 *  \param pos the position within the element     
 *
 *  \return integer value of the item element 
 */
string svkDcmtkAdapter::GetStringSequenceItemElement(const char* seqName, int seqItemPosition, const char* elementName, int pos, const char* parentSeqName, int parentSeqItemPosition)
{

    DcmItem* dataset = this->dcmFile->getDataset(); 

    if (parentSeqName != NULL) {
        dataset = NULL; 
        DcmSequenceOfItems* seq = GetDcmSequence(parentSeqName);
        if (seq != NULL ) {
            dataset = seq->getItem(parentSeqItemPosition);
        }
    }
    if (dataset != NULL) {
        return svkDcmtkUtils::getStringValue(
            this->GetDcmItem(dataset, seqName, seqItemPosition),
            GetDcmTagKey(elementName),
            pos    
        );
    } else {
        return string (""); 
    }
}


/*!
 *  Gets the DcmDictEntry for a given character string. First the foundEntries
 *  map is searched, and if the entry is not found the dcmtk global dictionary
 *  is searched. This is for performance reasons and should have the same
 *  behavior as searching the dcmtk global dictionary every time.
 *
 *  NOTE: If the dictionary is changed while the application is running then
 *        it is possible for the foundEntries map to become out of date.
 *
 *  \param name the name of the DcmDictEntry you wish to get
 *
 *  \return the DcmDictEntry of the input name
 */
const DcmDictEntry* svkDcmtkAdapter::FindEntry( const char* name )
{
    if ( this->foundEntries[name] == NULL  ) {
    	// The hash created an entry on the null check, lets remove it
    	this->foundEntries.erase(name);
    	this->foundEntries[name] = privateDic->findEntry( name );
    }
    return this->foundEntries[name];
}


/*!
 *  Lookup a name from a group/element
 *  Args: groupElementString "(gggg,eeee)"
 */
string svkDcmtkAdapter::GetDcmNameFromTag( string groupElementString ) 
{
    size_t delimit = groupElementString.find_first_of(","); 
    string group   = groupElementString.substr(1, delimit ); 
    string element = groupElementString.substr(delimit+1, 4); 

    istringstream* issG = new istringstream();
    unsigned short hexGroup = 0.0;
    issG->str( group );
    *issG >> hex >>  hexGroup;
    delete issG;

    istringstream* issE = new istringstream();
    unsigned short hexElement = 0.0;
    issE->str( element );
    *issE >> hex >> hexElement;
    delete issE;

    DcmTagKey tagKey;
    tagKey.setGroup( hexGroup ); 
    tagKey.setElement( hexElement ); 

    DcmTag tag = DcmTag(tagKey); 
    string name = tag.getTagName(); 

    return name;  
}


/*!
 *  Gets the DcmTagKey for a given character string
 *
 *  \param name the name of the DcmTagKey you wish to get
 *
 *  \return the DcmTagKey of the input name
 *  Note, this doesn't preserve downstream VR lookups via
 *  private dictionary elements if DcmTag objects are 
 *  initialized form this DcmTagKey return value.  To
 *  obtain the DcmTag with VR for downstreaming value 
 *  setting checks use GetDcmTag instead.  
 */
DcmTagKey svkDcmtkAdapter::GetDcmTagKey(const char* name) 
{

    DcmTag tag;
    const DcmDictEntry *dicEnt = this->FindEntry(name);
    if (dicEnt != NULL) {
        tag.set( dicEnt->getKey() );
    } else {
        if ( this->GetDebug() ) {
            cout << "GetDcmTagKey: TAG KEY NOT FOUND " << name << endl;
        }
    }

    return tag.getXTag();

}


/*!
 *  Gets the DcmTag for a given character string
 *
 *  \param name the name of the DcmTagKey you wish to get
 *
 *  \return the DcmTag (initialized with VR) of the input name
 */
DcmTag svkDcmtkAdapter::GetDcmTag(const char* name) 
{

    DcmTag tag;

    const DcmDictEntry *dicEnt = this->FindEntry(name);
    if (dicEnt != NULL) {
        tag.set( dicEnt->getKey() );
        tag.setVR( dicEnt->getVR() );
    } else {
        if ( this->GetDebug() ) {
            cout << "GetDcmTag: TAG NOT FOUND " << name << endl;
        }
    }

    return tag; 
}


/*!
 *  Creates/Gets the DcmItem in the specified position of the sequence.
 *
 *  \param seqName the name of the sequence in which the item you are looking for exists
 *  \param itemPosition the position in the sequence of the item you are looking for
 *  \return the DcmItem in the given sequence, at the given position
 *  Only searches top level of structure. 
 */
DcmItem* svkDcmtkAdapter::GetDcmItem(DcmItem* dataset, const char* seqName, int itemPosition) 
{
    DcmItem* item = NULL; 
    dataset->findOrCreateSequenceItem( GetDcmTagKey(seqName), item, itemPosition); 
    if (item == NULL) {
        cout << "Item Not Found" << seqName << endl;
        exit(1);    
    }

    return item; 
}



/*!
 *  Returns the DcmSequenceOfItems. 
 *
 *  \param seqName the name of the sequence you are looking for
 *
 *  \return the DcmSequenceOfItem of the given sequence name
 */
DcmSequenceOfItems* svkDcmtkAdapter::GetDcmSequence(const char* seqName) 
{

    DcmSequenceOfItems* sequence = NULL; 
    OFCondition status = this->dcmFile->getDataset()->findAndGetSequence( GetDcmTagKey(seqName), sequence, true ); 
    if ( sequence == NULL || status != EC_Normal ) {
    	if( this->GetDebug() ) {
			cout << "Sequence Not Found" << seqName << endl;
    	}
    }
    
    return sequence; 
}



/*!
 *  Returns the number of elements in the data set with the specified element name. 
 *
 *  \param elementName the name of the tag you wish to know the number of elements in
 *
 *  \return the number of elements in the given tag 
 */
int svkDcmtkAdapter::GetNumberOfElements(const char* elementName) 
{

    DcmStack stack; 
    this->dcmFile->getDataset()->findAndGetElements( GetDcmTagKey(elementName), stack); 
    int numberOfElements = 0; 
    while(stack.elem( numberOfElements ) != NULL) {
        numberOfElements++; 
    }
    return numberOfElements;  

}


/*!
 *  Returns the length of the element 
 *
 *  \param elementName the name of the tag you wish to know the number of elements in
 *
 *  \return the length of elements in the given tag 
 */
int svkDcmtkAdapter::GetSequenceItemElementLength(const char* seqName, int seqItemPosition, const char* elementName, const char* parentSeqName, int parentSeqItemPosition) 
{

    int length = 0; 

    DcmItem* dataset = this->dcmFile->getDataset(); 

    if (parentSeqName != NULL) {
        DcmSequenceOfItems* seq = GetDcmSequence(parentSeqName);
        if (seq != NULL ) {
            dataset = seq->getItem(parentSeqItemPosition);
        }
    }

    dataset = this->GetDcmItem(dataset, seqName, seqItemPosition);

    DcmElement* of;
    float* floatArray;
    OFCondition status = dataset->findAndGetElement( GetDcmTagKey( elementName ), of);
    if (status.bad()) {
        cerr << "Error: cannot get element(" << status.text() << ")" << endl;
    }

    length = of->getLength();

    return length; 
}



/*!
 *  Returns the number of items in the specified sequence. 
 *
 *  \param seqName the name of the sequence you wish to know the number of elements in 
 *
 *  \return the number of elements in the given sequence
 */
int svkDcmtkAdapter::GetNumberOfItemsInSequence( const char* seqName) 
{

    int itemCount = 0; 

    DcmSequenceOfItems* sequence = this->GetDcmSequence( seqName ); 

    if (sequence != NULL ) {
    
        while( sequence->getItem(itemCount) != NULL) {   
            itemCount++; 
        }
    }

    return itemCount; 

}


/*!
 *  Returns the number of items in the specified sequence. 
 *
 *  \param seqName the name of the sequence you wish to know the number of elements in 
 *
 *  \return the number of elements in the given sequence
 */
int svkDcmtkAdapter::GetNumberOfItemsInSequence(const char* seqName, const char* parentSeqName, int parentSeqItemPosition)
{
    DcmItem* dataset = this->dcmFile->getDataset(); 

    if (parentSeqName != NULL) {
        DcmSequenceOfItems* seq = GetDcmSequence(parentSeqName);
        if (seq != NULL ) {
            dataset = seq->getItem(parentSeqItemPosition);
        }
    }

    //DcmItem* tmpItem = this->GetDcmItem(dataset, seqName, seqItemPosition);    

    DcmSequenceOfItems* sequence = NULL; 
    OFCondition status = dataset->findAndGetSequence( GetDcmTagKey(seqName), sequence, true ); 

    int itemCount = 0; 
    if (sequence != NULL ) {
        while( sequence->getItem(itemCount) != NULL) {   
            itemCount++; 
        }
    }

    return itemCount; 
}


/*!
 *   Writes the DICOM file to the specified file name
 *
 *   \param fileName  name of the output file root (no extension).
 */
void  svkDcmtkAdapter::WriteDcmFile(string fileName) 
{

    OFCondition status = this->dcmFile->saveFile(fileName.c_str(), EXS_LittleEndianExplicit);

    if (status.bad()) {
        cerr << "Error: cannot write DICOM file (" << status.text() << ")" << endl;
    }

}


/*!
 *   Writes the DICOM file to the specified file name
 *   and using RLE Lossless compression transfer syntax. 
 *
 *   \param fileName  name of the output file root (no extension).
 */
void  svkDcmtkAdapter::WriteDcmFileCompressed(string fileName) 
{
    // register RLE compression codec
    DcmRLEEncoderRegistration::registerCodecs(); 

    OFCondition status = this->dcmFile->chooseRepresentation(EXS_RLELossless, NULL);
    if (status.bad()) {
        cerr << "Error: cannot set RLE Representation (" << status.text() << ")" << endl;
    }

    status = this->dcmFile->saveFile(fileName.c_str(), EXS_RLELossless);
    if (status.bad()) {
        cerr << "Error: cannot write DICOM file (" << status.text() << ")" << endl;
    }
    DcmRLEEncoderRegistration::cleanup();

}


/*!
 *   Read the DICOM file to the specified file name
 *
 *   \param fileName  name of the output file root (no extension).
 */
int svkDcmtkAdapter::ReadDcmFile(string fileName, unsigned int maxLength) 
{

    if (maxLength == 0) { 
        maxLength = DCM_MaxReadLength;
    }

    OFCondition status = this->dcmFile->loadFile( 
            fileName.c_str(), 
            EXS_Unknown, 
            EGL_noChange, 
            maxLength 
         );

    if (status.bad()) {
        cerr << "Error: cannot read DICOM file (" << status.text() << ")" << endl;
        return 1; 
    }

    this->originalXferSyntax =  this->dcmFile->getDataset()->getOriginalXfer(); 
    if( maxLength > HEADER_MAX_READ_LENGTH ) {
        //  get the input transfer syntax.. if RLELossless, then
        //  decode.
        E_TransferSyntax opt_oxfer = EXS_LittleEndianExplicit;
        DcmXfer opt_oxferSyn(opt_oxfer);
        DcmXfer original_xfer(this->dcmFile->getDataset()->getOriginalXfer());
        if ( this->dcmFile->getDataset()->getOriginalXfer() == EXS_RLELossless ) {
            // register RLE compression codec
            DcmRLEDecoderRegistration::registerCodecs();

            status = this->dcmFile->chooseRepresentation(EXS_LittleEndianExplicit, NULL);
            if (status.bad()) {
                cerr << "Error: cannot decode RLE Representation (" << status.text() << ")" << endl;
                return 1;
            }
            DcmRLEDecoderRegistration::cleanup();
        }
    }

    this->Modified();
    return 0; 
}


/*!
 *   Read the DICOM header only to the specified file name
 *
 *   \param fileName  name of the output file root (no extension).
 */
int svkDcmtkAdapter::ReadDcmFileHeaderOnly(string fileName)
{
    return this->ReadDcmFile( fileName, HEADER_MAX_READ_LENGTH);
}



/*!
 *   Return the original transfer syntax of the data set. 
 */
int svkDcmtkAdapter::GetOriginalXFerSyntax()
{
    return this->originalXferSyntax; 
}


/*!
 *  Copies the current DICOM header to the headerCopy, generating new
 *  series and instance UIDs, but retaining the reference header studyUID.  
 *  VTK doesn't support copy constructors, so this is a method.
 *
 *   \param headerCopy header object to copy current object into.
 */
void svkDcmtkAdapter::CopyDcmHeader(svkDcmHeader* headerCopy)
{

    DcmFileFormat* tmpHeader= (static_cast<DcmFileFormat*>( (this->dcmFile) ) );

    if (dynamic_cast<svkDcmtkAdapter*>(headerCopy)->dcmFile != NULL) {
        delete dynamic_cast<svkDcmtkAdapter*>(headerCopy)->dcmFile;
        dynamic_cast<svkDcmtkAdapter*>(headerCopy)->dcmFile = NULL;
    }

    dynamic_cast<svkDcmtkAdapter*>(headerCopy)->dcmFile = static_cast<svkDcmtkIod*>( tmpHeader->clone() );
   
}


/*!
 *  Test for existence of a particular element. If patentSeqName is set to "top" then only 
 *  check top level attributes (i.e. searchIntoSub = false). 
 */
bool svkDcmtkAdapter::ElementExists(const char* elementName, const char* parentSeqName)
{
    DcmItem* dataset = this->dcmFile->getDataset();
    bool elementExists = false;
    DcmElement* tmpElement;

    if( parentSeqName == NULL ) {
        if ( dataset->findAndGetElement( GetDcmTagKey(elementName), tmpElement, OFTrue) == EC_Normal ) {

            elementExists = true;

        }
    } else if( std::string(parentSeqName).compare("top") == 0) {

        if ( dataset->findAndGetElement( GetDcmTagKey(elementName), tmpElement, OFFalse) == EC_Normal ) {

            elementExists = true;

        }
    } else {

        DcmStack* searchStack = new DcmStack();
        DcmSequenceOfItems* sequence;  
        
        if( dataset->findAndGetSequence( GetDcmTagKey(parentSeqName), sequence, OFTrue ) == EC_Normal ) {
            DcmTagKey elementTag(GetDcmTagKey(elementName));
            if( sequence->search( elementTag, *searchStack, ESM_fromHere, OFTrue ) == EC_Normal ) {
                elementExists = true;
            }

        }
        delete searchStack;
    }
    if (this->GetDebug() && !elementExists) {

        vtkWarningWithObjectMacro(this, "could not find element: " << elementName ); 

    }
        
    return elementExists;
}


/*!
*  determines whether insert new element statements replace existing values or not.
*/
void svkDcmtkAdapter::ReplaceOldElements( bool replaceOldElements )
{
    if ( replaceOldElements ) {
        this->replaceOldElements = OFTrue; 
    } else {
        this->replaceOldElements = OFFalse; 
    }
}


/*!
 *  Some exceptions are acceptable so lets filter those out. If in debug mode
 *  report all exceptions.
 */
void svkDcmtkAdapter::HandleTagNotFoundException( const svkTagNotFound& e )
{
	if( this->GetDebug() ) {
        cout << "EXCEPTION: " << e.what() << endl;
	}
}
