/*
 *  Copyright © 2009-2010 The Regents of the University of California.
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


using namespace svk;


vtkCxxRevisionMacro(svkDcmtkAdapter, "$Rev$");
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

    this->SetPrivateDictionaryElements(); 

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

    //  defined terms specify whether k-space was sampled 
    //  symmetrically or not.  E.g., for an even number of 
    //  phase encodes, if even sampling, then k=0 was not 
    //  sampled . 
    privateDic->addEntry( new DcmDictEntry(
            0x7777, 0x1016, EVR_CS, 
            "SVK_KSpaceSymmetry", 
            1, 1, "private", OFFalse, "SVK_PRIVATE_CREATOR" 
        )
    );

    //  Specifies (True/False) if acquired data was chopped
    privateDic->addEntry( new DcmDictEntry(
            0x7777, 0x1017, EVR_CS, 
            "SVK_AcquisitionChop", 
            1, 1, "private", OFFalse, "SVK_PRIVATE_CREATOR" 
        )
    );

    dcmDataDict.unlock();
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
        this->dcmFile->getDataset()->putAndInsertString(
            DCM_SOPClassUID, 
            UID_MRImageStorage
        );
    } else if ( iodType == ENHANCED_MR_IMAGE) {
        this->dcmFile->getDataset()->putAndInsertString(
            DCM_SOPClassUID, 
            UID_EnhancedMRImageStorage
        );
    } else if ( iodType == MR_SPECTROSCOPY ) {
        this->dcmFile->getDataset()->putAndInsertString(
            DCM_SOPClassUID, 
            UID_MRSpectroscopyStorage
        );
    } else if ( iodType == SECONDARY_CAPTURE ) {
        this->dcmFile->getDataset()->putAndInsertString(
            DCM_SOPClassUID, 
            UID_SecondaryCaptureImageStorage
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
 *
 */
void svkDcmtkAdapter::InsertUniqueUID(const char* name)
{
    char uid[100];
    this->dcmFile->setValue( 
        GetDcmTag(name), 
        dcmGenerateUniqueIdentifier(uid, SITE_INSTANCE_UID_ROOT)
    ); 
    this->Modified();
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
    this->dcmFile->setValue( GetDcmTag(name), value ); 
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
 */
void svkDcmtkAdapter::SetValue(const char* name, string value)
{
    this->dcmFile->setValue( GetDcmTag(name), value ); 
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
void svkDcmtkAdapter::GetShortValue(const char* name, short* values, long unsigned int numValues)
{
    DcmElement* of;
    Uint16* shortArray;
    OFCondition status = this->dcmFile->getDataset()->findAndGetElement( GetDcmTagKey( name ), of);
    if (status.bad()) {
        cerr << "Error: cannot get element(" << status.text() << ")" << endl;
    }
    of->getUint16Array( shortArray );

    memcpy( values, shortArray, 2 * numValues );

    this->Modified();
}




/*!
 * Gets the value of a given tag as a double.
 *
 * \param name the name of the tag whose value you wish to get
 *
 * \return the double value of the tag
 */
double svkDcmtkAdapter::GetDoubleValue(const char* name)
{
    return this->dcmFile->getDoubleValue( GetDcmTagKey(name) ); 
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
    } catch (const exception& e) {
        cerr << "ERROR: " << e.what() << endl;
        return string(""); 
    }

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
    return this->dcmFile->getStringValue( GetDcmTagKey(name), pos ); 
}


/*!
 *  Add the nested sequence within the specified item of the parent sequence.
 *
 *  \param parentSeqName the string name of the parent sequence
 *  \param parentSeqItemPosition the position of the item in that sequence 
 *  \param elementName the string name of the parent sequence
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
    Uint32 dummyVal = 99; 
    newItem->putAndInsertUint32( GetDcmTag( elementName), dummyVal) ;

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
 *  Removes all items from the specified sequence
 *
 *  \param seqName the string name of the parent sequence to clear
 */
void svkDcmtkAdapter::ClearSequence(const char* seqName) 
{
    DcmSequenceOfItems* seq; 
    this->dcmFile->getDataset()->findAndGetSequence(  GetDcmTagKey( seqName ), seq, true);
    seq->clear();
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
        dataset == NULL; 
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

    const DcmDictEntry *dicEnt = privateDic->findEntry( name );
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

    const DcmDictEntry *dicEnt = this->privateDic->findEntry( name );

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
        cout << "Sequence Not Found" << seqName << endl;
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
 *   Read the DICOM file to the specified file name
 *
 *   \param fileName  name of the output file root (no extension).
 */
int svkDcmtkAdapter::ReadDcmFile(string fileName, int maxLength) 
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

    this->Modified();
    return 0; 
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
    dynamic_cast<svkDcmtkAdapter*>(headerCopy)->dcmFile = static_cast<svkDcmtkIod*>( tmpHeader->clone() );
    
    cout << "REMEMBER TO Set the creation time/date in the header!" << endl;
}


/*!
 *  Test for existence of a particular element. 
 */
bool svkDcmtkAdapter::ElementExists(const char* elementName, const char* parentSeqName)
{
    DcmItem* dataset = this->dcmFile->getDataset();
    bool elementExists = 0;
    DcmElement* tmpElement;

    if( parentSeqName == NULL ) {
        if ( dataset->findAndGetElement( GetDcmTagKey(elementName), tmpElement, OFTrue) == EC_Normal ) {

            elementExists = 1;

        }
    } else {

        DcmStack* searchStack = new DcmStack();
        DcmSequenceOfItems* sequence;  
        
        if( dataset->findAndGetSequence( GetDcmTagKey(parentSeqName), sequence, OFTrue ) == EC_Normal ) {
            DcmTagKey elementTag(GetDcmTagKey(elementName));
            if( sequence->search( elementTag, *searchStack, ESM_fromHere, OFTrue ) == EC_Normal ) {
                elementExists = 1;
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
