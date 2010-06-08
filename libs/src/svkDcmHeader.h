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


#ifndef SVK_DCM_HEADER_H
#define SVK_DCM_HEADER_H


#include <vtkObjectFactory.h>
#include <vtkObject.h>
#include <vtkMath.h>

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <set>


namespace svk {

using namespace std;


/*!
 *  Class that represents an svkDcmHeaderObjet.  
 */
class svkDcmHeader: public vtkObject
{

    public:

        vtkTypeRevisionMacro( svkDcmHeader, vtkObject );

        //  Members:

        /*!
         *  Supported DICOM IOD types.
         */
        typedef enum {
            MR_IMAGE = 0,
            ENHANCED_MR_IMAGE,
            MR_SPECTROSCOPY,
            SECONDARY_CAPTURE,      // Deprecated
            MULTI_FRAME_BYTE_SC,    // 8 bit gray scale multi-frame SC image
            MULTI_FRAME_WORD_SC     // >8 bit gray scale multi-frame SC image
        } DcmIodType;

        enum {
            DCMTK_API = 0
        };

        typedef enum {
            UNDEFINED = -1,
            UNSIGNED_INT_1 = 0,
            UNSIGNED_INT_2, 
            SIGNED_INT_2, 
            SIGNED_FLOAT_4, 
            SIGNED_FLOAT_8 
        } DcmPixelDataFormat;

        typedef enum {
            UNKNOWN = -1,
            AXIAL      = 0,
            CORONAL, 
            SAGITTAL 
        } Orientation;

        

        typedef enum{
            SLICE_ORDER_UNDEFINED = -1,
            INCREMENT_ALONG_POS_NORMAL = 0,
            INCREMENT_ALONG_NEG_NORMAL = 1
        } DcmDataOrderingDirection;


#ifdef SVK_ADAPT_DCMTK
        static const int adapter_type = DCMTK_API;  
#endif    


        //  Methods:

        /*!
         *  Factory method with specific imlementation in adapter sub-class. 
         *  create the implementation specific adaptee type (e.g. DcmFileFormat for DCMTK).  
         */
        virtual void        CreateIOD(DcmIodType iodType) = 0;
        virtual void        SetSOPClassUID(DcmIodType iodType) = 0;
        virtual void        PrintDcmHeader() = 0;

        /*! 
         *  Method to set a DICOM tag by specifying it's name and value. The name should be 
         *  the string representation of the field in the DICOM dictionary being used. 
         */
        virtual void    InsertEmptyElement(const char* name) = 0;

        /*! 
         *  Method to generate a new unique UID and insert it into the specified DICOM tag 
         *  by specifying it's name and value. The name should be 
         *  the string representation of the field in the DICOM dictionary being used. 
         */
        virtual void    InsertUniqueUID(const char* name) = 0;

        /*! 
         *  Method to set a DICOM tag by specifying it's name and value. The name should be 
         *  the string representation of the field in the DICOM dictionary being used. 
         */
        virtual void    SetValue(const char *name, int value) = 0;

        /*! 
         *  Method to set a DICOM tag by specifying it's name and value. The name should be 
         *  the string representation of the field in the DICOM dictionary being used. 
         */
        virtual void    SetValue(const char *name, float value) = 0;

        /*! 
         *  Method to set a DICOM tag by specifying it's name and value. The name should be 
         *  the string representation of the field in the DICOM dictionary being used. 
         */
        virtual void    SetValue(const char *name, double value) = 0;

        /*! 
         *  Method to set a DICOM tag by specifying it's name and value. The name should be 
         *  the string representation of the field in the DICOM dictionary being used. 
         */
        virtual void    SetValue(const char *name, string value) = 0;

        /*!
        *   Sets the array of value for a given tag.
        *
        *   \param name the name of the tag whose value you wish to set
        *
        *   \param values the pointer to the array of values you wish the tag to have 
        *
        *   \param numValues the number of elements in the array of values 
        */
        virtual void    SetValue(const char* name, unsigned char* values, int numValues) = 0;

        /*!
        *   Sets the array of value for a given tag.
        *
        *   \param name the name of the tag whose value you wish to set
        *
        *   \param values the pointer to the array of values you wish the tag to have 
        *
        *   \param numValues the number of elements in the array of values 
        */
        virtual void    SetValue(const char* name, unsigned short* values, int numValues) = 0;

        /*!
        *   Sets the array of value for a given tag.
        *
        *   \param name the name of the tag whose value you wish to set
        *
        *   \param values the pointer to the array of values you wish the tag to have 
        *
        *   \param numValues the number of elements in the array of values 
        */
        virtual void    SetValue(const char* name, float* values, int numValues) = 0;

        /*! 
         *  Method to get a DICOM tag by specifying it's name and value. The name should be 
         *  the string representation of the field in the DICOM dictionary being used. 
         */
        virtual void    GetShortValue(const char* name, short* values, long unsigned int numValues) = 0;

        /*! 
         *  Method to get a DICOM tag by specifying it's name and value. The name should be 
         *  the string representation of the field in the DICOM dictionary being used. 
         */
        virtual int     GetIntValue(const char *name) = 0;

        /*! 
         *  Method to get a DICOM tag by specifying it's name and value. The name should be 
         *  the string representation of the field in the DICOM dictionary being used. 
         */
        virtual float   GetFloatValue(const char *name) = 0;

        /*! 
         *  Method to get a DICOM tag by specifying it's name and value. The name should be 
         *  the string representation of the field in the DICOM dictionary being used. 
         */
        virtual void    GetFloatValue(const char* name, float* values, long unsigned int numValues) = 0;


        /*! 
         *  Method to get a DICOM tag by specifying it's name and value. The name should be 
         *  the string representation of the field in the DICOM dictionary being used. 
         */
        virtual double  GetDoubleValue(const char *name) = 0;

        /*! 
         *  Method to get a DICOM tag by specifying it's name and value. The name should be 
         *  the string representation of the field in the DICOM dictionary being used. 
         */
        virtual string  GetStringValue(const char *name) = 0;

        /*! 
         *  Method to get a DICOM tag by specifying it's name and value. The name should be 
         *  the string representation of the field in the DICOM dictionary being used. 
         */
        virtual string  GetStringValue(const char *name, int pos) = 0;

        /*! 
         *  Method to add a nested SQ element in the specified  
         *  item of the parent sequence. 
         */
        virtual void    AddSequenceItemElement(
                            const char* parentSeqName, 
                            int parentSeqItemPosition, 
                            const char* elementName
                        ) = 0;

        /*! 
         *  Method to add an item to a DICOM Sequence by specifying the SQ name and
         *  the item's position in the sequence. Optionally, for nested sequences
         *  specify the parent SQ and item number. 
         */
        virtual void    AddSequenceItemElement(
                            const char* seqName,
                            int seqItemPosition,
                            const char* elementName,
                            string value, 
                            const char* parentSeqName = NULL,
                            int parentSeqItemPosition = 0
                        ) = 0;


        /*! 
         *  Method to add a multi-valued item to a DICOM Sequence by specifying the SQ 
         *  name and the item's position in the sequence. Optionally, for nested sequences
         *  specify the parent SQ and item number. 
         */
        virtual void    AddSequenceItemElement(
                            const char* seqName,
                            int seqItemPosition,
                            const char* elementName,
                            unsigned int* values, 
                            int numValues, 
                            const char* parentSeqName = NULL,
                            int parentSeqItemPosition = 0
                        ) = 0;

        /*! 
         *  Method to add an item to a DICOM Sequence by specifying the SQ name and
         *  the item's position in the sequence. Optionally, for nested sequences
         *  specify the parent SQ and item number. 
         */
        virtual void    AddSequenceItemElement(
                            const char* seqName,
                            int seqItemPosition,
                            const char* elementName,
                            int value, 
                            const char* parentSeqName = NULL,
                            int parentSeqItemPosition = 0 
                        ) = 0;

        /*! 
         *  Method to add an item to a DICOM Sequence by specifying the SQ name and
         *  the item's position in the sequence. Optionally, for nested sequences
         *  specify the parent SQ and item number. 
         */
        virtual void    AddSequenceItemElement(
                            const char* seqName,
                            int seqItemPosition,
                            const char* elementName,
                            float value, 
                            const char* parentSeqName = NULL,
                            int parentSeqItemPosition = 0
                        ) = 0;


        /*! 
         *  Method to clear all items from a sequence of items 
         */
        virtual void    ClearSequence( const char* parentSeqName ) = 0; 


        /*! 
         *  Method to clear all items from a sequence of items 
         */
        virtual void    ClearElement( const char* elementName ) = 0;


        /*! 
         *  Method to get an item's element from a DICOM Sequence by specifying the SQ 
         *  name and the item's position in the sequence. Optionally, for nested sequences 
         *  specify the parent SQ and item number.
         *  \param seqName  camel case string name of DICOM field  
         *  \param seqItemPosition  int value for sequence item in which element is found 
         *  \param elementName camel case string name of dicom field  
         *  \param parentSeqName optional camel case string name of parent DICOM sequence. 
         *  \param parentSeqItemPosition optional int value for parent sequence item. 
         */
        virtual int     GetIntSequenceItemElement(
                            const char* seqName, 
                            int seqItemPosition, 
                            const char* elementName,
                            const char* parentSeqName = NULL,
                            int parentSeqItemPosition = 0, 
                            int pos = 0
                        ) = 0;

        /*! 
         *  Method to get an item's element from a DICOM Sequence by specifying the SQ 
         *  name and the item's position in the sequence. Optionally, for nested sequences 
         *  specify the parent SQ and item number.
         */
        virtual float   GetFloatSequenceItemElement(
                            const char* seqName,
                            int seqItemPosition,
                            const char* elementName,
                            const char* parentSeqName = NULL,
                            int parentSeqItemPosition = 0,
                            int pos = 0
                        ) = 0;

        /*! 
         *  Method to get an item's element from a DICOM Sequence by specifying the SQ 
         *  name and the item's position in the sequence. Optionally, for nested sequences 
         *  specify the parent SQ and item number.
         */
        virtual double  GetDoubleSequenceItemElement(
                            const char* seqName,
                            int seqItemPosition,
                            const char* elementName,
                            const char* parentSeqName = NULL,
                            int parentSeqItemPosition = 0
                        ) = 0;

        /*! 
         *  Method to get an item's element from a DICOM Sequence by specifying the SQ 
         *  name and the item's position in the sequence. Optionally, for nested sequences 
         *  specify the parent SQ and item number.
         */
        virtual string  GetStringSequenceItemElement(
                            const char* seqName,
                            int seqItemPosition,
                            const char* elementName,
                            const char* parentSeqName = NULL,
                            int parentSeqItemPosition = 0
                        ) = 0;

        /*! 
         *  Method to get an  item's element from a DICOM Sequence by specifying the SQ 
         *  name and the item's position in the sequence. Optionally, for nested sequences 
         *  specify the parent SQ and item number.  
         */
        virtual string  GetStringSequenceItemElement(
                            const char* seqName, 
                            int seqItemPosition, 
                            const char* elementName, 
                            int pos,
                            const char* parentSeqName = NULL,
                            int parentSeqItemPosition = 0
                        ) = 0;


        /*! 
         *  Returns the number of elements in the data set matching the specified elementName.
         */
        virtual int     GetNumberOfElements( const char* elementName ) = 0;


        /*! 
         *  Returns the number of items in the specified sequence. 
         */
        virtual int     GetNumberOfItemsInSequence( const char* seqName ) = 0;

        /*!
         *  Writes the DICOM file to the specified file name  
         *
         *  \param fileName  name of the output file root (no extension). 
         */
        virtual void  WriteDcmFile(string fileName) = 0;


        /*!
         *  Read the DICOM file of the specified file name  
         *
         *  \param fileName  name of the output file root (no extension). 
         * 
         *  Returns 0 for success, 1 for failure.  
         */
        virtual int   ReadDcmFile(string fileName, int max_length = 0) = 0;


        /*! 
         *  Copies the current DICOM header to the headerCopy, generating new
         *  series and instance UIDs, but retaining the reference header studyUID.  
         *  VTK doesn't support copy constructors, so this is a method.  
         *
         *   \param headerCopy header object to copy current object into. 
         */
        virtual void        CopyDcmHeader(svkDcmHeader* headerCopy) = 0;


        /*! 
         *  Initializes the input svkDcmHeader (the copy) from this.  Also 
         *  generates new seriesUID and instanceUID for the copy. 
         *
         *  VTK doesn't support copy constructors, so this is a method.  
         *
         *   \param headerCopy header object to copy current object into. 
         */
        virtual void        MakeDerivedDcmHeader(svkDcmHeader* headerCopy, string seriesDescription);


        /*!
         *  Check whether an element exists in the DICOM header object. 
         */
        virtual bool        ElementExists(const char* elementName, const char* parentSeqName = NULL) = 0;


        /*!
         *  determines whether insert new element statements replace existing values or not. 
         */
        virtual void        ReplaceOldElements( bool replaceElements ) = 0;



        void                SetDcmPatientsName(string patientsName);
        void                SetPixelDataType(DcmPixelDataFormat dataType);
        int                 GetPixelDataType();

        int                 GetOrigin(double origin[3], int frameNumber = 0);
        void                GetPixelSpacing(double spacing[3]);
        void                GetPixelSize(double size[3]);
        void                GetOrientation(double orientation[2][3]);
        svkDcmHeader::Orientation         GetOrientationType();
        void                GetNormalVector(double normal[3]);
        void                GetDataDcos(
                                double dcos[3][3], 
                                DcmDataOrderingDirection dataSliceOrder = SLICE_ORDER_UNDEFINED 
                            );
        void                SetSliceOrder( DcmDataOrderingDirection sliceOrderVal );
        int                 GetNumberOfCoils(); 
        int                 GetNumberOfSlices(); 
        static int          GetNumberOfDimensionIndices(int numTimePts, int numCoils); 
        static void         SetDimensionIndices(
                                unsigned int* indexValues, 
                                int numFrameIndices, 
                                int sliceNum, 
                                int timePt, 
                                int coilNum, 
                                int numTimePts, 
                                int numCoils
                            ); 
        int                 GetNumberOfTimePoints(); 
        void                UpdateNumTimePoints(); 

        int                 GetDimensionIndexPosition(string indexLabel); 
        int                 GetNumberOfFramesInDimension( int dimensionIndex ); 

        void                InitMultiFrameDimensionModule( int numSlices, int numTimePts, int numCoils ); 
        void                InitPerFrameFunctionalGroupSequence(
                                            double toplc[3], double voxelSpacing[3],
                                            double dcos[3][3], int numSlices, int numTimePts, int numCoils); 


    protected:

        svkDcmHeader();
        ~svkDcmHeader();
        bool                        WasModified();    


    private:
        double                      pixelSize[3]; 
        double                      pixelSpacing[3]; 
        double                      origin0[3]; 
        double                      orientation[2][3]; 
        int                         numTimePts;
        unsigned long               lastUpdateTime; 
        DcmDataOrderingDirection    dataSliceOrder;

        void                        UpdateSpatialParams();
        void                        UpdateOrientation();
        void                        UpdatePixelSpacing();
        void                        UpdatePixelSize();
        void                        UpdateOrigin0();

        void                        InitPlanePositionMacro(
                                             double toplc[3], double voxelSpacing[3],
                                             double dcos[3][3], int numSlices, int numTimePts, int numCoils
                                    ); 

        void                        InitFrameContentMacro( 
                                        int numSlices = -1, 
                                        int numTimePts = -1, 
                                        int numCoils  = -1
                                    ); 


};


}   //svk


#endif //SVK_DCM_HEADER_H

