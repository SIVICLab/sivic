/*
 *  Copyright © 2009-2012 The Regents of the University of California.
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


#ifndef SVK_DCMTK_ADAPTER_H
#define SVK_DCMTK_ADAPTER_H


#include <vtkObjectFactory.h>
#include <vtkObject.h>
#include <svkDcmHeader.h>
#include <svkDcmtkIod.h>
#include <dcmtk/dcmdata/dctk.h>
#include <dcmtk/dcmdata/dctag.h>


namespace svk {


using namespace svk;


/*!
 *  Adapter Class to make a DCMTK DcmFileFormat look like a generic svkDcmHeader object.  
 */
class svkDcmtkAdapter: public svkDcmHeader 
{

    public:

        static svkDcmtkAdapter* New();
        vtkTypeRevisionMacro( svkDcmtkAdapter, svkDcmHeader);


    protected:

        svkDcmtkAdapter();
        ~svkDcmtkAdapter();

        //  Methods:
        virtual void    CreateIOD(DcmIodType iodType);
        virtual void    SetSOPClassUID(DcmIodType iodType);

        virtual void    PrintDcmHeader();
        virtual void    PrintDcmHeader(ostream& os);

        virtual void    InsertEmptyElement(const char* name);
        virtual void    InsertUniqueUID(const char* name);
        vtkstd::string  GenerateUniqueUID();

        virtual void    SetValue(const char* name, int value);
        virtual void    SetValue(const char* name, float value);
        virtual void    SetValue(const char* name, double value);
        virtual void    SetValue(const char* name, string value);
        virtual void    SetValue(const char* name, unsigned char* values, int numValues);
        virtual void    SetValue(const char* name, unsigned short* values, int numValues);
        virtual void    SetValue(const char* name, signed short* values, int numValues);
        virtual void    SetValue(const char* name, float* values, int numValues);
        virtual void    ModifyValueRecursive(const char* name, string value); 


        virtual void    GetShortValue(const char* name, short* values, long unsigned int numValues); 
        virtual int     GetIntValue(const char *name);
        virtual float   GetFloatValue(const char *name);
        virtual void    GetFloatValue(const char* name, float* values, long unsigned int numValues); 
        virtual double  GetDoubleValue(const char *name);
        virtual string  GetStringValue(const char *name);
        virtual string  GetStringValue(const char *name, int pos);

        virtual void    AddSequenceItemElement(
                            const char* parentSeqName, 
                            int         parentSeqItemPosition, 
                            const char* elementName
                        );
        virtual void    AddSequenceItemElement(
                            const char* seqName, 
                            int         seqItemPosition, 
                            const char* elementName, 
                            string      value, 
                            const char* parentSeqName = NULL, 
                            int         parentSeqItemPosition = 0
                        );
        virtual void    AddSequenceItemElement(
                            const char* seqName,
                            int seqItemPosition,
                            const char* elementName,
                            char*       values, 
                            int         numValues, 
                            const char* parentSeqName = NULL,
                            int parentSeqItemPosition = 0
                        );
        virtual void    AddSequenceItemElement(
                            const char*     seqName,
                            int             seqItemPosition,
                            const char*     elementName,
                            unsigned short* values, 
                            int             numValues, 
                            const char*     parentSeqName = NULL,
                            int             parentSeqItemPosition = 0
                        );
        virtual void    AddSequenceItemElement(
                            const char*     seqName,
                            int             seqItemPosition,
                            const char*     elementName,
                            unsigned int*   values,
                            int             numValues, 
                            const char*     parentSeqName = NULL,
                            int             parentSeqItemPosition = 0
                        );
        virtual void    AddSequenceItemElement(
                            const char* seqName,
                            int         seqItemPosition,
                            const char* elementName,
                            float*      values,
                            unsigned long int numValues, 
                            const char* parentSeqName = NULL,
                            int         parentSeqItemPosition = 0
                        );
        virtual void    AddSequenceItemElement(
                            const char* seqName, 
                            int         seqItemPosition, 
                            const char* elementName, 
                            int         value, 
                            const char* parentSeqName = NULL, 
                            int         parentSeqItemPosition = 0
                        );
        virtual void    AddSequenceItemElement(
                            const char* seqName, 
                            int         seqItemPosition, 
                            const char* elementName, 
                            long int    value, 
                            const char* parentSeqName = NULL, 
                            int         parentSeqItemPosition = 0
                        );
        virtual void    AddSequenceItemElement(
                            const char* seqName, 
                            int         seqItemPosition, 
                            const char* elementName, 
                            float       value, 
                            const char* parentSeqName = NULL, 
                            int         parentSeqItemPosition = 0
                        );
        virtual void    AddSequenceItemElement(
                            const char* seqName,
                            int         seqItemPosition,
                            const char* elementName,
                            double      value,
                            const char* parentSeqName = NULL,
                            int         parentSeqItemPosition = 0
                        );
                        

        virtual void    CopySequence( svkDcmHeader* target, const char* seqName );

        virtual void    ClearSequence( const char* seqName ); 

        virtual void    ClearElement(const char* elementName); 

        virtual void    RemoveElement(const char* elementName); 

        virtual int     GetIntSequenceItemElement(
                            const char* seqName, 
                            int         seqItemPosition, 
                            const char* elementName,
                            const char* parentSeqName = NULL, 
                            int         parentSeqItemPosition = 0, 
                            int         pos = 0    
                        );
        virtual long int GetLongIntSequenceItemElement(
                            const char* seqName, 
                            int         seqItemPosition, 
                            const char* elementName,
                            const char* parentSeqName = NULL, 
                            int         parentSeqItemPosition = 0, 
                            int         pos = 0    
                        );
        virtual float   GetFloatSequenceItemElement(
                            const char* seqName, 
                            int         seqItemPosition, 
                            const char* elementName,
                            const char* parentSeqName = NULL, 
                            int         parentSeqItemPosition = 0, 
                            int         pos = 0    
                        );
        virtual void    GetFloatSequenceItemElement(
                            const char* seqName,
                            int         seqItemPosition,
                            const char* elementName,
                            float*      values,
                            int         numValues,
                            const char* parentSeqName = NULL,
                            int         parentSeqItemPosition = 0
                        );
        virtual double  GetDoubleSequenceItemElement(
                            const char* seqName, 
                            int         seqItemPosition, 
                            const char* elementName,
                            const char* parentSeqName = NULL, 
                            int         parentSeqItemPosition = 0
                        );
        virtual string  GetStringSequenceItemElement(
                            const char* seqName, 
                            int         seqItemPosition, 
                            const char* elementName,
                            const char* parentSeqName = NULL, 
                            int         parentSeqItemPosition = 0
                        );
        virtual string  GetStringSequenceItemElement(
                            const char* seqName, 
                            int         seqItemPosition, 
                            const char* elementName, 
                            int         pos,
                            const char* parentSeqName = NULL, 
                            int         parentSeqItemPosition = 0
                        );

        virtual int     GetNumberOfElements( const char* elementName );
        virtual int     GetNumberOfItemsInSequence( const char* seqName ); 

        virtual void    WriteDcmFile(string fileName);
        virtual int     ReadDcmFile(string fileName, int maxLength );

        virtual void    CopyDcmHeader(svkDcmHeader* headerCopy);

        virtual bool    ElementExists(const char* elementName, const char* parentSeqName); 

        virtual void    ReplaceOldElements( bool replaceElements );

        virtual void    HandleTagNotFoundException( const svkTagNotFound& e);


    private:

        //  Methods:
        const DcmDictEntry* FindEntry( const char* name );
        DcmTag              GetDcmTag(const char* name);
        DcmTagKey           GetDcmTagKey(const char* name);
        DcmItem*            GetDcmItem(DcmItem* dataset, const char* seqName, int itemPosition);
        DcmSequenceOfItems* GetDcmSequence(const char* seqName); 
        void                SetPrivateDictionaryElements(); 
        void                SetGEPrivateDictionaryElements();


        //  Members:
        svkDcmtkIod*                     dcmFile;
        OFBool                           replaceOldElements;
        DcmDataDictionary*               privateDic;
        // For performance we store some entries from the dictionary.
        map<string, const DcmDictEntry*> foundEntries;

        // Static Members:
        static bool         privateElementsAdded;



};


}   //svk


#endif //SVK_DCMTK_ADAPTER_H

